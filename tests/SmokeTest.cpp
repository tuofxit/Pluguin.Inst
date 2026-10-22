#include "PluginProcessor.h"
#include <iostream>
#include <stdexcept>

static void expect(bool condition, const char* message)
{
    if (! condition) throw std::runtime_error(message);
}

static void waitForLoad(InstroAudioProcessor& p)
{
    for (int i = 0; i < 1500; ++i) {
        const auto status = p.samples.status();
        if (status.startsWith("Pronto:")) return;
        if (status.startsWith("Falha")) throw std::runtime_error(status.toStdString());
        juce::Thread::sleep(20);
    }
    throw std::runtime_error("Timed out loading samples");
}

static float play(InstroAudioProcessor& p, int blockSize, int note = 60)
{
    float peak = 0.0f;
    juce::AudioBuffer<float> audio(2, blockSize);
    for (int i = 0; i < 16; ++i) {
        juce::MidiBuffer midi;
        if (i == 0) {
            midi.addEvent(juce::MidiMessage::allSoundOff(1), 0);
            midi.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8) 100), 0);
            midi.addEvent(juce::MidiMessage::noteOn(1, note + 4, (juce::uint8) 80), blockSize / 2);
        }
        if (i == 8) midi.addEvent(juce::MidiMessage::controllerEvent(1, 64, 127), 0);
        if (i == 10) midi.addEvent(juce::MidiMessage::noteOff(1, note), 0);
        if (i == 12) {
            midi.addEvent(juce::MidiMessage::pitchWheel(1, 10000), 0);
            midi.addEvent(juce::MidiMessage::controllerEvent(1, 64, 0), blockSize / 2);
            midi.addEvent(juce::MidiMessage::noteOff(1, note + 4), blockSize - 1);
        }
        p.processBlock(audio, midi);
        for (int ch = 0; ch < 2; ++ch)
            for (int j = 0; j < blockSize; ++j) {
                const float value = audio.getSample(ch, j);
                expect(std::isfinite(value) && std::abs(value) <= 1.0f, "Invalid audio output");
                peak = juce::jmax(peak, std::abs(value));
            }
    }
    return peak;
}

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    try {
        expect(argc == 3 || argc == 4, "Usage: InstroChecks <Banks> <screenshot.png> [relative.sfz]");
        const juce::File banks(juce::String::fromUTF8(argv[1]));
        const auto piano = banks.getChildFile(argc == 4 ? juce::String::fromUTF8(argv[3]) : "Meta-Piano/meta-piano.sfz");
        InstroAudioProcessor p;
        p.setBankRoot(banks);
        p.prepareToPlay(44100, 64);
        p.samples.request(piano);
        waitForLoad(p);
        for (const double sr : { 44100.0, 48000.0 })
            for (const int size : { 64, 1024, 4096 }) {
                p.prepareToPlay(sr, size);
                expect(play(p, size) > 0.00001f, "Natural mode is silent");
                p.samples.panic();
                std::cout << "Natural " << sr << " Hz / " << size << " frames PASS\n";
            }
        juce::MemoryBlock state;
        p.getStateInformation(state);
        InstroAudioProcessor restored;
        restored.prepareToPlay(48000, 512);
        restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        waitForLoad(restored);
        expect(restored.samples.selectedFile() == piano, "Sample path not restored");
        expect(restored.getBankRoot() == banks, "Bank root not restored");
        expect(play(restored, 512) > 0.00001f, "Restored instrument silent");
        std::cout << "State restoration PASS\n";
        restored.parameters.getParameter("sampleMode")->setValueNotifyingHost(0.0f);
        for (int i = 0; i < 80; ++i) {
            auto* engine = restored.parameters.getParameter("engine");
            engine->setValueNotifyingHost(engine->convertTo0to1(static_cast<float>(i)));
            expect(play(restored, 512) > 0.000001f, "Synthetic instrument silent");
        }
        std::cout << "80 synthetic choices PASS\n";
        restored.parameters.getParameter("sampleMode")->setValueNotifyingHost(1.0f);
        restored.samples.request(banks.getChildFile("missing-file.sfz"));
        for (int i = 0; i < 200 && restored.samples.status().startsWith("Carregando"); ++i) juce::Thread::sleep(10);
        expect(restored.samples.selectedFile() == piano, "Failed load lost previous instrument");
        expect(play(restored, 512) > 0.00001f, "Previous instrument lost after failed load");
        std::cout << "Missing file recovery PASS\n";
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        const auto image = editor->createComponentSnapshot(editor->getLocalBounds());
        const juce::File screenshot(juce::String::fromUTF8(argv[2]));
        juce::FileOutputStream stream(screenshot);
        expect(stream.openedOk(), "Cannot write screenshot");
        expect(juce::PNGImageFormat().writeImageToStream(image, stream), "Cannot encode screenshot");
        std::cout << "Editor snapshot PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
