#include "SampleEngine.h"

SampleEngine::SampleEngine() : Thread("Instro sample loader") { startThread(); }
SampleEngine::~SampleEngine() { signalThreadShouldExit(); notify(); stopThread(-1); }

void SampleEngine::prepare(double sampleRate)
{
    rate.store(sampleRate);
    const juce::SpinLock::ScopedLockType lock(engineLock);
    if (active) sfizz_set_sample_rate(active.get(), static_cast<float>(sampleRate));
}

void SampleEngine::request(const juce::File& file)
{
    const juce::ScopedLock lock(requestLock);
    wantedFile = file;
    message = "Carregando: " + file.getFileNameWithoutExtension();
    ++generation;
    notify();
}

juce::String SampleEngine::status() const { const juce::ScopedLock lock(requestLock); return message; }
juce::File SampleEngine::selectedFile() const { const juce::ScopedLock lock(requestLock); return wantedFile; }

void SampleEngine::run()
{
    int handled = 0;
    while (! threadShouldExit())
    {
        juce::File file;
        int ticket;
        { const juce::ScopedLock lock(requestLock); ticket = generation.load(); file = wantedFile; }
        if (ticket == handled) { wait(100); continue; }
        handled = ticket;
        Synth replacement(sfizz_create_synth());
        sfizz_set_sample_rate(replacement.get(), static_cast<float>(rate.load()));
        sfizz_set_samples_per_block(replacement.get(), blockSize);
        sfizz_set_num_voices(replacement.get(), 128);
        const bool ok = file.existsAsFile() && sfizz_load_file(replacement.get(), file.getFullPathName().toRawUTF8());
        if (threadShouldExit()) break;
        const juce::ScopedLock requestGuard(requestLock);
        if (ticket != generation.load()) continue;
        if (ok && sfizz_get_num_regions(replacement.get()) > 0)
        {
            { // Old synth is freed after releasing the lock.
                const juce::SpinLock::ScopedLockType lock(engineLock);
                sfizz_set_sample_rate(replacement.get(), static_cast<float>(rate.load()));
                active.swap(replacement);
            }
            message = "Pronto: " + file.getFileNameWithoutExtension();
            loadedFile = file;
        }
        else {
            wantedFile = loadedFile;
            message = "Falha ao abrir SFZ. O instrumento anterior foi mantido.";
        }
    }
}

void SampleEngine::panic()
{
    const juce::SpinLock::ScopedTryLockType lock(engineLock);
    if (lock.isLocked() && active) sfizz_all_sound_off(active.get());
}

void SampleEngine::render(juce::AudioBuffer<float>& output, const juce::MidiBuffer& midi)
{
    const juce::SpinLock::ScopedTryLockType lock(engineLock);
    if (! lock.isLocked() || ! active) return;
    auto* synth = active.get();
    // Fixed-size slices also support host blocks larger than the prepared size.
    auto event = midi.begin();
    for (int offset = 0; offset < output.getNumSamples(); offset += blockSize)
    {
        const auto count = juce::jmin(blockSize, output.getNumSamples() - offset);
        while (event != midi.end() && (*event).samplePosition < offset + count)
        {
            const auto metadata = *event++;
            const auto m = metadata.getMessage();
            const int delay = juce::jlimit(0, count - 1, metadata.samplePosition - offset);
            if (m.isNoteOn()) sfizz_send_note_on(synth, delay, m.getNoteNumber(), m.getVelocity());
            else if (m.isNoteOff()) sfizz_send_note_off(synth, delay, m.getNoteNumber(), m.getVelocity());
            else if (m.isController()) sfizz_send_cc(synth, delay, m.getControllerNumber(), m.getControllerValue());
            else if (m.isPitchWheel()) sfizz_send_pitch_wheel(synth, delay, m.getPitchWheelValue() - 8192);
            else if (m.isChannelPressure()) sfizz_send_channel_aftertouch(synth, delay, m.getChannelPressureValue());
            else if (m.isAftertouch()) sfizz_send_poly_aftertouch(synth, delay, m.getNoteNumber(), m.getAfterTouchValue());
        }
        float* channels[] { scratch.getWritePointer(0), scratch.getWritePointer(1) };
        sfizz_render_block(synth, channels, 2, count);
        if (output.getNumChannels() == 1)
        {
            output.copyFrom(0, offset, channels[0], count, 0.5f);
            output.addFrom(0, offset, channels[1], count, 0.5f);
        }
        else
        {
            output.copyFrom(0, offset, channels[0], count);
            output.copyFrom(1, offset, channels[1], count);
        }
    }
}
