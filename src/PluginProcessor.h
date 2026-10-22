#pragma once

#include <JuceHeader.h>
#include "SampleEngine.h"

class InstroAudioProcessor final : public juce::AudioProcessor
{
public:
    InstroAudioProcessor();
    ~InstroAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destination) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    SampleEngine samples;
    juce::MidiKeyboardState keyboardState;
    void setBankRoot(const juce::File& root);
    juce::File getBankRoot() const;
    juce::AudioProcessorValueTreeState parameters;

private:
    class InstrumentSound final : public juce::SynthesiserSound
    {
    public:
        bool appliesToNote(int) override { return true; }
        bool appliesToChannel(int) override { return true; }
    };

    class InstrumentVoice final : public juce::SynthesiserVoice
    {
    public:
        explicit InstrumentVoice(juce::AudioProcessorValueTreeState& state);
        bool canPlaySound(juce::SynthesiserSound* sound) override;
        void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override;
        void stopNote(float velocity, bool allowTailOff) override;
        void pitchWheelMoved(int newPitchWheelValue) override;
        void controllerMoved(int controllerNumber, int newControllerValue) override;
        void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;

    private:
        float renderEngineSample() noexcept;
        float readParameter(const char* parameterID, float fallback) const noexcept;

        juce::ADSR envelope;
        std::atomic<float>* engineParameter = nullptr;
        std::atomic<float>* attackParameter = nullptr;
        std::atomic<float>* decayParameter = nullptr;
        std::atomic<float>* sustainParameter = nullptr;
        std::atomic<float>* releaseParameter = nullptr;
        std::atomic<float>* cutoffParameter = nullptr;
        std::atomic<float>* driveParameter = nullptr;
        std::atomic<float>* variationParameter = nullptr;
        std::atomic<float>* vibratoParameter = nullptr;
        juce::Random noise;
        float noiseFilter = 0.0f;
        double pitchRatio = 1.0;
        double phase = 0.0;
        double noteFrequency = 440.0;
        double noteAge = 0.0;
        float velocityGain = 0.0f;
        float filterState = 0.0f;
        int selectedEngine = 0;
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::Synthesiser synthesiser;
    mutable juce::CriticalSection bankRootLock;
    juce::File bankRoot;
    bool previousSampleMode = true;
    juce::SmoothedValue<float> outputGain;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstroAudioProcessor)
};
