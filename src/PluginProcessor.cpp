#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "InstrumentLibrary.h"

namespace
{
constexpr double twoPi = juce::MathConstants<double>::twoPi;

float sine(double angle) noexcept
{
    return static_cast<float>(std::sin(angle));
}

float saw(double angle) noexcept
{
    const auto cycle = angle / twoPi;
    return static_cast<float>(2.0 * (cycle - std::floor(cycle + 0.5)));
}
}

InstroAudioProcessor::InstroAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("INSTRO_STATE"), createParameterLayout())
{
    for (int voice = 0; voice < 32; ++voice)
        synthesiser.addVoice(new InstrumentVoice(parameters));

    synthesiser.addSound(new InstrumentSound());
}

juce::AudioProcessorValueTreeState::ParameterLayout InstroAudioProcessor::createParameterLayout()
{
    using Choice = juce::AudioParameterChoice;
    using Float = juce::AudioParameterFloat;
    using Range = juce::NormalisableRange<float>;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<Choice>(juce::ParameterID { "engine", 1 }, "Instrument",
        InstroLibrary::names(), 0));
    layout.add(std::make_unique<Float>(juce::ParameterID { "attack", 1 }, "Attack", Range(0.001f, 3.0f, 0.001f), 0.01f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "decay", 1 }, "Decay", Range(0.01f, 4.0f, 0.01f), 0.25f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "sustain", 1 }, "Sustain", Range(0.0f, 1.0f, 0.001f), 0.72f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "release", 1 }, "Release", Range(0.02f, 6.0f, 0.01f), 0.35f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "cutoff", 1 }, "Tone", Range(80.0f, 18000.0f, 1.0f, 0.35f), 14000.0f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "drive", 1 }, "Drive", Range(0.0f, 1.0f, 0.001f), 0.08f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "variation", 1 }, "Character", Range(0.0f, 1.0f), 0.5f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "vibrato", 1 }, "Vibrato", Range(0.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "level", 1 }, "Output", Range(0.0f, 1.0f), 0.7f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "sampleMode", 1 }, "Natural samples", true));
    return layout;
}

void InstroAudioProcessor::prepareToPlay(double sampleRate, int)
{
    synthesiser.setCurrentPlaybackSampleRate(sampleRate);
    samples.prepare(sampleRate);
    outputGain.reset(sampleRate, 0.02);
    outputGain.setCurrentAndTargetValue(parameters.getRawParameterValue("level")->load());
}

void InstroAudioProcessor::releaseResources()
{
}

bool InstroAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

void InstroAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    keyboardState.processNextMidiBuffer(midi, 0, buffer.getNumSamples(), true);
    const bool natural = parameters.getRawParameterValue("sampleMode")->load() >= 0.5f;
    if (natural != previousSampleMode)
    {
        synthesiser.allNotesOff(0, false);
        samples.panic();
        previousSampleMode = natural;
    }
    if (natural) samples.render(buffer, midi);
    else synthesiser.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
    outputGain.setTargetValue(parameters.getRawParameterValue("level")->load());
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float gain = outputGain.getNextValue();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const float value = buffer.getSample(ch, i) * gain;
            buffer.setSample(ch, i, natural ? juce::jlimit(-1.0f, 1.0f, value) : std::tanh(value));
        }
    }
}

juce::AudioProcessorEditor* InstroAudioProcessor::createEditor()
{
    return new InstroAudioProcessorEditor(*this);
}

bool InstroAudioProcessor::hasEditor() const { return true; }
const juce::String InstroAudioProcessor::getName() const { return JucePlugin_Name; }
bool InstroAudioProcessor::acceptsMidi() const { return true; }
bool InstroAudioProcessor::producesMidi() const { return false; }
bool InstroAudioProcessor::isMidiEffect() const { return false; }
double InstroAudioProcessor::getTailLengthSeconds() const { return 30.0; }
int InstroAudioProcessor::getNumPrograms() { return 1; }
int InstroAudioProcessor::getCurrentProgram() { return 0; }
void InstroAudioProcessor::setCurrentProgram(int) {}
const juce::String InstroAudioProcessor::getProgramName(int) { return "Default"; }
void InstroAudioProcessor::changeProgramName(int, const juce::String&) {}

void InstroAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto state = parameters.copyState();
    state.setProperty("bankRoot", getBankRoot().getFullPathName(), nullptr);
    state.setProperty("sampleFile", samples.selectedFile().getFullPathName(), nullptr);
    if (const auto xml = state.createXml()) copyXmlToBinary(*xml, destination);
}

void InstroAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters.state.getType()))
        {
            const auto restored = juce::ValueTree::fromXml(*xml);
            parameters.replaceState(restored);
            const auto rootPath = restored.getProperty("bankRoot").toString();
            const auto samplePath = restored.getProperty("sampleFile").toString();
            setBankRoot(rootPath.isNotEmpty() ? juce::File(rootPath) : juce::File());
            if (samplePath.isNotEmpty()) samples.request(juce::File(samplePath));
        }
}

void InstroAudioProcessor::setBankRoot(const juce::File& root)
{
    const juce::ScopedLock lock(bankRootLock); bankRoot = root;
}
juce::File InstroAudioProcessor::getBankRoot() const
{
    const juce::ScopedLock lock(bankRootLock); return bankRoot;
}

InstroAudioProcessor::InstrumentVoice::InstrumentVoice(juce::AudioProcessorValueTreeState& state)
{
    variationParameter = state.getRawParameterValue("variation");
    vibratoParameter = state.getRawParameterValue("vibrato");
    engineParameter = state.getRawParameterValue("engine");
    attackParameter = state.getRawParameterValue("attack");
    decayParameter = state.getRawParameterValue("decay");
    sustainParameter = state.getRawParameterValue("sustain");
    releaseParameter = state.getRawParameterValue("release");
    cutoffParameter = state.getRawParameterValue("cutoff");
    driveParameter = state.getRawParameterValue("drive");
}

bool InstroAudioProcessor::InstrumentVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<InstrumentSound*>(sound) != nullptr;
}

float InstroAudioProcessor::InstrumentVoice::readParameter(const char* parameterID, float fallback) const noexcept
{
    const auto* parameter = juce::String(parameterID) == "attack" ? attackParameter
        : juce::String(parameterID) == "decay" ? decayParameter
        : juce::String(parameterID) == "sustain" ? sustainParameter
        : juce::String(parameterID) == "release" ? releaseParameter
        : juce::String(parameterID) == "cutoff" ? cutoffParameter
        : juce::String(parameterID) == "drive" ? driveParameter : nullptr;
    return parameter != nullptr ? parameter->load(std::memory_order_relaxed) : fallback;
}

void InstroAudioProcessor::InstrumentVoice::startNote(int midiNoteNumber, float velocity,
                                                       juce::SynthesiserSound*, int pitchWheel)
{
    noteFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    velocityGain = velocity;
    phase = 0.0;
    pitchWheelMoved(pitchWheel);
    noteAge = 0.0;
    filterState = 0.0f;
    noiseFilter = 0.0f;
    selectedEngine = engineParameter != nullptr
        ? juce::roundToInt(engineParameter->load(std::memory_order_relaxed)) : 0;

    envelope.setSampleRate(getSampleRate());
    // Acoustic approximations use a characteristic envelope per family.
    float attackScale = 1.0f, sustainScale = 1.0f;
    if ((selectedEngine >= 12 && selectedEngine <= 18) || (selectedEngine >= 21 && selectedEngine <= 40) || selectedEngine == 48 || selectedEngine >= 71)
        sustainScale = 0.0f;
    if ((selectedEngine >= 43 && selectedEngine <= 47) || (selectedEngine >= 63 && selectedEngine <= 67))
        attackScale = 12.0f;
    envelope.setParameters({ attackScale * readParameter("attack", 0.01f),
                             readParameter("decay", 0.25f),
                             sustainScale * readParameter("sustain", 0.72f),
                             readParameter("release", 0.35f) });
    envelope.noteOn();
}

void InstroAudioProcessor::InstrumentVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        envelope.noteOff();
    else
    {
        envelope.reset();
        clearCurrentNote();
    }
}

void InstroAudioProcessor::InstrumentVoice::pitchWheelMoved(int value)
{
    pitchRatio = std::pow(2.0, ((value - 8192) / 8192.0) * 2.0 / 12.0);
}
void InstroAudioProcessor::InstrumentVoice::controllerMoved(int, int) {}

float InstroAudioProcessor::InstrumentVoice::renderEngineSample() noexcept
{
    const float character = variationParameter->load(std::memory_order_relaxed);
    int model = selectedEngine;
    float colour = 1.0f;
    if (model >= 12) {
        const auto entry = InstroLibrary::extra[juce::jlimit(0, static_cast<int>(std::size(InstroLibrary::extra)) - 1, model - 12)];
        model = entry.engine; colour = entry.colour;
    }
    colour *= 0.5f + character;
    const auto fundamental = sine(phase);
    const auto second = sine(phase * 2.0);
    const auto third = sine(phase * 3.0);
    const auto fourth = sine(phase * 4.0);
    const auto detune = 0.0035;
    const auto decay = static_cast<float>(std::exp(-noteAge * (1.0 + 6.0 * colour)));
    float sample = 0.0f;

    switch (model)
    {
        case 0: sample = 0.72f * saw(phase) + 0.28f * sine(phase * 0.5); break;
        case 1:
            sample = (saw(phase + twoPi * noteFrequency * noteAge * detune * colour)
                    + saw(phase) + saw(phase - twoPi * noteFrequency * noteAge * detune * colour)) / 3.0f;
            break;
        case 2: sample = sine(phase + (0.5 + 3.4 * colour) * sine(phase * 2.0)); break;
        case 3: sample = sine(phase + (1.0 + 4.0 * colour) * decay * sine(phase * 2.76)); break;
        case 4: sample = 0.48f * fundamental + (0.12f + 0.16f * character) * second + 0.16f * third + 0.08f * fourth; break;
        case 5: sample = sine(phase + (0.5 + 3.0 * colour) * decay * sine(phase * 2.0)); break;
        case 6: sample = decay * (0.58f * fundamental + (0.18f * colour) * second + (0.09f * colour) * third); break;
        case 7: sample = (0.48f + 0.15f * colour) * fundamental + 0.25f * sine(phase * 0.5) + (0.08f * colour) * third; break;
        case 8: sample = 0.55f * sine(phase) + 0.45f * sine(phase + twoPi * noteFrequency * noteAge * detune * colour); break;
        case 9: sample = (0.32f + 0.16f * colour) * saw(phase) + 0.25f * sine(phase * 2.0) + (0.1f * colour) * sine(phase * 3.0); break;
        case 10: sample = decay * (0.62f * fundamental + 0.28f * sine(phase * 3.92) + 0.10f * sine(phase * 6.17)); break;
        case 11: sample = fundamental - (0.15f * colour) * sine(phase * 3.0) + (0.08f * colour) * sine(phase * 5.0); break;
        case 12: sample = sine(twoPi * (45.0 * noteAge + 3.5 * (1.0 - std::exp(-noteAge * 35.0)))) * std::exp(-noteAge * (5.0 + colour * 5.0)); break;
        case 13: sample = (noise.nextFloat() * 2.0f - 1.0f) * std::exp(-noteAge * 16.0) * 0.75f + sine(twoPi * 180.0 * noteAge) * std::exp(-noteAge * 25.0) * 0.25f; break;
        case 14: case 15: case 18: case 19:
            sample = (noise.nextFloat() * 2.0f - 1.0f) * std::exp(-noteAge * (model == 15 ? 5.0 : model == 19 ? 28.0 : 55.0));
            sample = sample - noiseFilter; noiseFilter += 0.18f * sample; break;
        case 16: sample = sine(phase + 5.0 * (1.0 - std::exp(-noteAge * 20.0))) * std::exp(-noteAge * 9.0); break;
        case 17: sample = (noise.nextFloat() * 2.0f - 1.0f) * std::exp(-noteAge * 20.0) * (0.5f + 0.5f * sine(twoPi * 95.0 * noteAge)); break;
        case 20: sample = sine(phase + 4.0 * sine(phase * 1.47)) * std::exp(-noteAge * 12.0); break;
        default: sample = fundamental; break;
    }

    return sample;
}

void InstroAudioProcessor::InstrumentVoice::renderNextBlock(juce::AudioBuffer<float>& output,
                                                             int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    const auto sampleRate = getSampleRate();
    const auto phaseStep = twoPi * noteFrequency / sampleRate;
    const auto vibrato = vibratoParameter->load(std::memory_order_relaxed);
    const auto cutoff = juce::jlimit(20.0f, 20000.0f, readParameter("cutoff", 14000.0f));
    const auto filterCoefficient = 1.0f - std::exp(-twoPi * cutoff / static_cast<float>(sampleRate));
    const auto drive = 1.0f + 12.0f * readParameter("drive", 0.08f);

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const auto amplitude = envelope.getNextSample();
        auto value = renderEngineSample() * amplitude * velocityGain;
        filterState += filterCoefficient * (value - filterState);
        value = std::tanh(filterState * drive) / std::tanh(drive);
        value *= 0.22f;

        for (int channel = 0; channel < output.getNumChannels(); ++channel)
            output.addSample(channel, startSample + sampleIndex, value);

        phase += phaseStep * pitchRatio * (1.0 + 0.009 * vibrato * std::sin(twoPi * 5.2 * noteAge));
        noteAge += 1.0 / sampleRate;
    }

    if (! envelope.isActive())
        clearCurrentNote();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new InstroAudioProcessor();
}
