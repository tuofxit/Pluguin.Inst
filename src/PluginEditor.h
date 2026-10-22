#pragma once

#include "PluginProcessor.h"

class InstroAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit InstroAudioProcessorEditor(InstroAudioProcessor& processor);
    ~InstroAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void scanLibrary(const juce::File&);
    void refreshInstruments();
    void refreshArticulations();
    void loadSelection();
    struct Entry { juce::String family, instrument, articulation, path; };
    std::vector<Entry> entries;
    juce::ComboBox familySelector, instrumentSelector, articulationSelector;
    juce::Label familyLabel, instrumentLabel, articulationLabel, statusLabel;
    juce::Label naturalInfo;
    juce::ToggleButton naturalToggle { "Usar samples" };
    juce::TextButton folderButton { "Escolher pasta Banks..." };
    juce::TextButton panicButton { "Parar notas" };
    std::unique_ptr<juce::FileChooser> folderChooser;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> naturalAttachment;
    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);

    InstroAudioProcessor& audioProcessor;
    juce::MidiKeyboardComponent keyboard;
    juce::Slider variationSlider, vibratoSlider, levelSlider;
    juce::Label variationLabel, vibratoLabel, levelLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> variationAttachment, vibratoAttachment, levelAttachment;
    juce::ComboBox engineSelector;
    juce::Label titleLabel;
    juce::Label engineLabel;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel, cutoffLabel, driveLabel;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, cutoffSlider, driveSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> engineAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstroAudioProcessorEditor)
};
