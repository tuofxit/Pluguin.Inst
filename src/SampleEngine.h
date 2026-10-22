#pragma once
#include <JuceHeader.h>
#include <sfizz.h>

// Files and engine construction belong to the worker, never the audio callback.
class SampleEngine final : private juce::Thread
{
public:
    SampleEngine();
    ~SampleEngine() override;
    void prepare(double sampleRate);
    void request(const juce::File& file);
    void render(juce::AudioBuffer<float>&, const juce::MidiBuffer&);
    juce::String status() const;
    juce::File selectedFile() const;
    void panic();

private:
    struct Deleter { void operator()(sfizz_synth_t* p) const { if (p) sfizz_free(p); } };
    using Synth = std::unique_ptr<sfizz_synth_t, Deleter>;
    void run() override;
    static constexpr int blockSize = 1024;
    juce::SpinLock engineLock;
    mutable juce::CriticalSection requestLock;
    Synth active;
    juce::File wantedFile;
    juce::File loadedFile;
    juce::String message { "Escolha a pasta Banks para comecar." };
    std::atomic<double> rate { 44100.0 };
    std::atomic<int> generation { 0 };
    juce::AudioBuffer<float> scratch { 2, blockSize };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleEngine)
};
