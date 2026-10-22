#include "PluginEditor.h"
#include "InstrumentLibrary.h"

InstroAudioProcessorEditor::InstroAudioProcessorEditor(InstroAudioProcessor& processor)
    : AudioProcessorEditor(&processor), audioProcessor(processor),
      keyboard(processor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setSize(940, 620);
    addAndMakeVisible(keyboard);
    keyboard.setAvailableRange(24, 96);
    keyboard.setLowestVisibleKey(48);
    addAndMakeVisible(naturalInfo);
    naturalInfo.setFont(juce::Font(juce::FontOptions(16.0f)));
    naturalInfo.setText("EXPRESSAO DOS INSTRUMENTOS\n\nToque com intensidades diferentes para explorar a dinamica das gravacoes.\nEscolha tecnicas de execucao no menu de variacoes.\nUse o pedal de sustain quando o instrumento permitir.\n\nO volume e ajustado no controle OUTPUT. O teclado abaixo permite experimentar.", juce::dontSendNotification);

    titleLabel.setText("INSTRO NATURAL", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe8f1e8));
    addAndMakeVisible(titleLabel);

    engineLabel.setText("INSTRUMENT", juce::dontSendNotification);
    engineLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9cac9d));
    engineLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    addAndMakeVisible(engineLabel);

    engineSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff202a24));
    engineSelector.setColour(juce::ComboBox::textColourId, juce::Colour(0xffeff7ee));
    engineSelector.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff45574a));
    addAndMakeVisible(engineSelector);

    configureSlider(attackSlider, attackLabel, "ATTACK");
    configureSlider(decaySlider, decayLabel, "DECAY");
    configureSlider(sustainSlider, sustainLabel, "SUSTAIN");
    configureSlider(releaseSlider, releaseLabel, "RELEASE");
    configureSlider(cutoffSlider, cutoffLabel, "TONE");
    configureSlider(driveSlider, driveLabel, "DRIVE");

    configureSlider(variationSlider, variationLabel, "CHARACTER");
    configureSlider(vibratoSlider, vibratoLabel, "VIBRATO");
    configureSlider(levelSlider, levelLabel, "OUTPUT");
    engineSelector.addItemList(InstroLibrary::names(), 1);
    for (auto* component : std::initializer_list<juce::Component*> { &familySelector, &instrumentSelector, &articulationSelector,
             &familyLabel, &instrumentLabel, &articulationLabel, &statusLabel, &naturalToggle, &folderButton, &panicButton })
        addAndMakeVisible(component);
    familyLabel.setText("FAMILIA", juce::dontSendNotification);
    instrumentLabel.setText("INSTRUMENTO", juce::dontSendNotification);
    articulationLabel.setText("VARIACAO / ARTICULACAO", juce::dontSendNotification);
    familySelector.onChange = [this] { refreshInstruments(); };
    instrumentSelector.onChange = [this] { refreshArticulations(); };
    articulationSelector.onChange = [this] { loadSelection(); };
    folderButton.onClick = [this] {
        folderChooser = std::make_unique<juce::FileChooser>("Selecione a pasta Banks", audioProcessor.getBankRoot());
        const juce::Component::SafePointer<InstroAudioProcessorEditor> safe(this);
        folderChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
            [safe] (const juce::FileChooser& chooser) {
                if (safe != nullptr && chooser.getResult().isDirectory()) safe->scanLibrary(chooser.getResult());
            });
    };
    panicButton.onClick = [this] { audioProcessor.samples.panic(); };
    auto& state = audioProcessor.parameters;
    naturalAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "sampleMode", naturalToggle);
    startTimerHz(4);
    auto root = audioProcessor.getBankRoot();
    if (! root.isDirectory()) {
        auto candidate = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory();
        for (int i = 0; i < 7; ++i) {
            if (candidate.getChildFile("Banks/catalog.json").existsAsFile()) { root = candidate.getChildFile("Banks"); break; }
            candidate = candidate.getParentDirectory();
        }
    }
    if (root.isDirectory()) scanLibrary(root);
    variationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "variation", variationSlider);
    vibratoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "vibrato", vibratoSlider);
    levelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "level", levelSlider);
    engineAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, "engine", engineSelector);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "attack", attackSlider);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "decay", decaySlider);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "sustain", sustainSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "release", releaseSlider);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "cutoff", cutoffSlider);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "drive", driveSlider);
    timerCallback();
}

void InstroAudioProcessorEditor::configureSlider(juce::Slider& slider, juce::Label& label,
                                                   const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colour(0xff9cac9d));
    label.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    addAndMakeVisible(label);

    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff8bd28e));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff344238));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff0bb68));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffdce8dc));
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff202a24));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff344238));
    addAndMakeVisible(slider);
}

void InstroAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(0xff141b17));
    graphics.setColour(juce::Colour(0xff8bd28e));
    graphics.fillRect(0, 0, getWidth(), 5);
    graphics.setColour(juce::Colour(0xff202a24));
    graphics.fillRoundedRectangle(20.0f, 284.0f, static_cast<float>(getWidth() - 40), 225.0f, 8.0f);
    graphics.setColour(juce::Colour(0xff344238));
    graphics.drawRoundedRectangle(20.0f, 284.0f, static_cast<float>(getWidth() - 40), 225.0f, 8.0f, 1.0f);
}

void InstroAudioProcessorEditor::resized()
{
    keyboard.setBounds(24, 535, getWidth() - 48, 65);
    naturalInfo.setBounds(46, 300, 730, 200);
    naturalToggle.setBounds(430, 25, 150, 30);
    folderButton.setBounds(590, 25, 200, 30);
    panicButton.setBounds(800, 25, 115, 30);
    familyLabel.setBounds(24, 110, 210, 22);
    instrumentLabel.setBounds(250, 110, 300, 22);
    articulationLabel.setBounds(565, 110, 350, 22);
    familySelector.setBounds(24, 136, 210, 32);
    instrumentSelector.setBounds(250, 136, 300, 32);
    articulationSelector.setBounds(565, 136, 350, 32);
    statusLabel.setBounds(24, 185, 890, 75);
    titleLabel.setBounds(24, 20, 380, 42);
    engineLabel.setBounds(24, 68, 100, 20);
    engineSelector.setBounds(128, 64, getWidth() - 152, 30);

    juce::Slider* sliders[] = { &attackSlider, &decaySlider, &sustainSlider,
                                &releaseSlider, &cutoffSlider, &driveSlider, &variationSlider, &vibratoSlider, &levelSlider };
    juce::Label* labels[] = { &attackLabel, &decayLabel, &sustainLabel,
                              &releaseLabel, &cutoffLabel, &driveLabel, &variationLabel, &vibratoLabel, &levelLabel };
    const auto columnWidth = (getWidth() - 56) / 9;

    for (int index = 0; index < 9; ++index)
    {
        const auto x = 28 + index * columnWidth;
        labels[index]->setBounds(x, 308, columnWidth - 4, 22);
        sliders[index]->setBounds(x, 334, columnWidth - 4, 150);
    }
}
void InstroAudioProcessorEditor::timerCallback()
{
    const bool natural = audioProcessor.parameters.getRawParameterValue("sampleMode")->load() >= 0.5f;
    engineSelector.setEnabled(! natural);
    engineSelector.setVisible(! natural);
    engineLabel.setVisible(! natural);
    naturalInfo.setVisible(natural);
    for (auto* slider : { &attackSlider, &decaySlider, &sustainSlider, &releaseSlider, &cutoffSlider, &driveSlider, &variationSlider, &vibratoSlider })
        slider->setVisible(! natural);
    for (auto* label : { &attackLabel, &decayLabel, &sustainLabel, &releaseLabel, &cutoffLabel, &driveLabel, &variationLabel, &vibratoLabel })
        label->setVisible(! natural);
    for (auto* selector : { &familySelector, &instrumentSelector, &articulationSelector }) selector->setEnabled(natural);
    statusLabel.setText(natural ? audioProcessor.samples.status() + "\nDinamica: velocity | Expressao: CC1 / CC11 conforme o banco | Sustain: CC64"
                               : "Modo sintetizador: os controles abaixo ajustam os 80 timbres sinteticos.", juce::dontSendNotification);
}

void InstroAudioProcessorEditor::scanLibrary(const juce::File& root)
{
    const auto catalog = juce::JSON::parse(root.getChildFile("catalog.json"));
    if (! catalog.isArray()) {
        statusLabel.setText("Esta pasta precisa conter catalog.json. Selecione a pasta Banks fornecida.", juce::dontSendNotification);
        return;
    }
    audioProcessor.setBankRoot(root);
    entries.clear();
    juce::StringArray families;
    for (const auto& item : *catalog.getArray()) {
        Entry e { item["family"].toString(), item["instrument"].toString(), item["articulation"].toString(), item["path"].toString() };
        if (root.getChildFile(e.path).existsAsFile()) { families.addIfNotAlreadyThere(e.family); entries.push_back(e); }
    }
    familySelector.clear(juce::dontSendNotification);
    familySelector.addItemList(families, 1);
    const auto selected = audioProcessor.samples.selectedFile();
    auto found = std::find_if(entries.begin(), entries.end(), [&] (const Entry& e) { return root.getChildFile(e.path) == selected; });
    if (found != entries.end()) {
        familySelector.setText(found->family, juce::dontSendNotification);
        refreshInstruments();
        instrumentSelector.setText(found->instrument, juce::dontSendNotification);
        refreshArticulations();
        articulationSelector.setText(found->articulation, juce::dontSendNotification);
        loadSelection();
    } else { familySelector.setSelectedId(1, juce::dontSendNotification); refreshInstruments(); }
}

void InstroAudioProcessorEditor::refreshInstruments()
{
    juce::StringArray names;
    for (const auto& e : entries) if (e.family == familySelector.getText()) names.addIfNotAlreadyThere(e.instrument);
    instrumentSelector.clear(juce::dontSendNotification);
    instrumentSelector.addItemList(names, 1);
    instrumentSelector.setSelectedId(1, juce::dontSendNotification);
    refreshArticulations();
}

void InstroAudioProcessorEditor::refreshArticulations()
{
    juce::StringArray names;
    for (const auto& e : entries) if (e.family == familySelector.getText() && e.instrument == instrumentSelector.getText()) names.addIfNotAlreadyThere(e.articulation);
    articulationSelector.clear(juce::dontSendNotification);
    articulationSelector.addItemList(names, 1);
    articulationSelector.setSelectedId(1, juce::dontSendNotification);
    loadSelection();
}

void InstroAudioProcessorEditor::loadSelection()
{
    for (const auto& e : entries)
        if (e.family == familySelector.getText() && e.instrument == instrumentSelector.getText() && e.articulation == articulationSelector.getText()) {
            const auto file = audioProcessor.getBankRoot().getChildFile(e.path);
            if (file != audioProcessor.samples.selectedFile()) audioProcessor.samples.request(file);
            break;
        }
}
