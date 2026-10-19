#include "MidiMappingEditor.h"

MidiMappingEditor::MidiMappingEditor(const juce::String& pid, MidiLearnManager& mgr)
    : manager(mgr), paramId(pid)
{
    setSize(520, 320);
    title.setText("MIDI MAPPING: " + paramId, juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centredLeft);
    title.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    addAndMakeVisible(title);

    typeBox.addItem("CC", 1);
    typeBox.addItem("NRPN", 2);
    typeBox.addItem("NOTE", 3);
    typeBox.addItem("AFTERTOUCH", 4);
    typeBox.addItem("PITCHBEND", 5);
    addAndMakeVisible(typeBox);

    numberLabel.setText("Number:", juce::dontSendNotification);
    addAndMakeVisible(numberLabel);
    addAndMakeVisible(numberEdit);

    channelLabel.setText("Channel:", juce::dontSendNotification);
    addAndMakeVisible(channelLabel);
    channelBox.addItem("Omni", 1);
    for (int i=1;i<=16;++i) channelBox.addItem("Ch " + juce::String(i), i+1);
    addAndMakeVisible(channelBox);

    inputLabel.setText("Input Range:", juce::dontSendNotification);
    addAndMakeVisible(inputLabel);
    addAndMakeVisible(inMinEdit);
    addAndMakeVisible(inMaxEdit);

    outputLabel.setText("Output Range:", juce::dontSendNotification);
    addAndMakeVisible(outputLabel);
    addAndMakeVisible(outMinEdit);
    addAndMakeVisible(outMaxEdit);

    optionsLabel.setText("Options:", juce::dontSendNotification);
    addAndMakeVisible(optionsLabel);
    invertedToggle.setButtonText("Inverted");
    addAndMakeVisible(invertedToggle);
    relativeToggle.setButtonText("Relative");
    addAndMakeVisible(relativeToggle);

    smoothLabel.setText("Smoothing:", juce::dontSendNotification);
    addAndMakeVisible(smoothLabel);
    smoothingSlider.setRange(0.0, 1.0, 0.01);
    smoothingSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    addAndMakeVisible(smoothingSlider);

    monitorLabel.setText("MONITOR:", juce::dontSendNotification);
    addAndMakeVisible(monitorLabel);
    monitorValue.setText("Waiting...", juce::dontSendNotification);
    addAndMakeVisible(monitorValue);

    addAndMakeVisible(testButton);
    addAndMakeVisible(okButton);
    addAndMakeVisible(cancelButton);
    testButton.addListener(this);
    okButton.addListener(this);
    cancelButton.addListener(this);

    loadFromMapping();
    startTimerHz(10);
}

MidiMappingEditor::~MidiMappingEditor() { stopTimer(); }

void MidiMappingEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2b2b2b));
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds());
}

void MidiMappingEditor::resized()
{
    auto r = getLocalBounds().reduced(12);
    title.setBounds(r.removeFromTop(24));
    r.removeFromTop(6);

    auto line1 = r.removeFromTop(28);
    typeBox.setBounds(line1.removeFromLeft(120));
    numberLabel.setBounds(line1.removeFromLeft(70));
    numberEdit.setBounds(line1.removeFromLeft(80));
    channelLabel.setBounds(line1.removeFromLeft(70));
    channelBox.setBounds(line1.removeFromLeft(110));

    r.removeFromTop(6);
    auto line2 = r.removeFromTop(28);
    inputLabel.setBounds(line2.removeFromLeft(90));
    inMinEdit.setBounds(line2.removeFromLeft(60));
    line2.removeFromLeft(10);
    inMaxEdit.setBounds(line2.removeFromLeft(60));

    r.removeFromTop(6);
    auto line3 = r.removeFromTop(28);
    outputLabel.setBounds(line3.removeFromLeft(90));
    outMinEdit.setBounds(line3.removeFromLeft(60));
    line3.removeFromLeft(10);
    outMaxEdit.setBounds(line3.removeFromLeft(60));

    r.removeFromTop(6);
    auto line4 = r.removeFromTop(28);
    optionsLabel.setBounds(line4.removeFromLeft(90));
    invertedToggle.setBounds(line4.removeFromLeft(120));
    relativeToggle.setBounds(line4.removeFromLeft(160));

    r.removeFromTop(6);
    auto line5 = r.removeFromTop(36);
    smoothLabel.setBounds(line5.removeFromLeft(90));
    smoothingSlider.setBounds(line5.removeFromLeft(240));

    r.removeFromTop(6);
    auto line6 = r.removeFromTop(28);
    monitorLabel.setBounds(line6.removeFromLeft(90));
    monitorValue.setBounds(line6);

    r.removeFromTop(12);
    auto bottom = r.removeFromBottom(32);
    testButton.setBounds(bottom.removeFromLeft(80));
    bottom.removeFromLeft(8);
    okButton.setBounds(bottom.removeFromLeft(80));
    bottom.removeFromLeft(8);
    cancelButton.setBounds(bottom.removeFromLeft(80));
}

void MidiMappingEditor::buttonClicked(juce::Button* b)
{
    if (b == &cancelButton)
    {
        if (onClose) onClose();
        return;
    }
    if (b == &okButton)
    {
        auto m = getEdited();
        manager.addMapping(m);
        if (onClose) onClose();
        return;
    }
    if (b == &testButton)
    {
        auto m = getEdited();
        // Send mid-range test value
        int testIn = (m.minIn + m.maxIn) / 2;
        int out = juce::jlimit(m.minOut, m.maxOut, int(std::round( m.minOut + (m.maxOut - m.minOut) * 0.5 )));
        manager.onMappedParameterChange ? manager.onMappedParameterChange(m.paramId, out) : void();
    }
}

void MidiMappingEditor::timerCallback()
{
    // Show quick info of last values if available
    if (auto* m = manager.getMappingForParam(paramId))
    {
        juce::String type;
        switch (m->type)
        {
            case MidiLearnManager::MidiMapping::CC: type = "CC"; break;
            case MidiLearnManager::MidiMapping::NRPN: type = "NRPN"; break;
            case MidiLearnManager::MidiMapping::NOTE: type = "NOTE"; break;
            case MidiLearnManager::MidiMapping::AFTERTOUCH: type = "AFTERTOUCH"; break;
            case MidiLearnManager::MidiMapping::PITCHBEND: type = "PITCHBEND"; break;
        }
        monitorValue.setText("Current: " + type + " " + juce::String(m->controller) +
                             " Ch " + (m->channel >= 0 ? juce::String(m->channel + 1) : juce::String("Omni")), juce::dontSendNotification);
    }
}

void MidiMappingEditor::loadFromMapping()
{
    if (auto* m = manager.getMappingForParam(paramId))
    {
        switch (m->type)
        {
            case MidiLearnManager::MidiMapping::CC: typeBox.setSelectedId(1); break;
            case MidiLearnManager::MidiMapping::NRPN: typeBox.setSelectedId(2); break;
            case MidiLearnManager::MidiMapping::NOTE: typeBox.setSelectedId(3); break;
            case MidiLearnManager::MidiMapping::AFTERTOUCH: typeBox.setSelectedId(4); break;
            case MidiLearnManager::MidiMapping::PITCHBEND: typeBox.setSelectedId(5); break;
        }
        numberEdit.setText(juce::String(m->controller));
        channelBox.setSelectedId(m->channel < 0 ? 1 : (m->channel + 2));
        inMinEdit.setText(juce::String(m->minIn));
        inMaxEdit.setText(juce::String(m->maxIn));
        outMinEdit.setText(juce::String(m->minOut));
        outMaxEdit.setText(juce::String(m->maxOut));
        invertedToggle.setToggleState(m->inverted, juce::dontSendNotification);
        relativeToggle.setToggleState(m->relative, juce::dontSendNotification);
        smoothingSlider.setValue(m->smoothing, juce::dontSendNotification);
    }
    else
    {
        typeBox.setSelectedId(1);
        numberEdit.setText("0");
        channelBox.setSelectedId(1);
        inMinEdit.setText("0");
        inMaxEdit.setText("127");
        outMinEdit.setText("0");
        outMaxEdit.setText("127");
        invertedToggle.setToggleState(false, juce::dontSendNotification);
        relativeToggle.setToggleState(false, juce::dontSendNotification);
        smoothingSlider.setValue(0.0, juce::dontSendNotification);
    }
}

MidiLearnManager::MidiMapping MidiMappingEditor::getEdited() const
{
    MidiLearnManager::MidiMapping m;
    m.paramId = paramId;
    switch (typeBox.getSelectedId())
    {
        case 1: m.type = MidiLearnManager::MidiMapping::CC; break;
        case 2: m.type = MidiLearnManager::MidiMapping::NRPN; break;
        case 3: m.type = MidiLearnManager::MidiMapping::NOTE; break;
        case 4: m.type = MidiLearnManager::MidiMapping::AFTERTOUCH; break;
        case 5: m.type = MidiLearnManager::MidiMapping::PITCHBEND; break;
        default: m.type = MidiLearnManager::MidiMapping::CC; break;
    }
    m.controller = numberEdit.getText().getIntValue();
    m.channel = (channelBox.getSelectedId() <= 1) ? -1 : (channelBox.getSelectedId() - 2);
    m.minIn = inMinEdit.getText().getIntValue();
    m.maxIn = inMaxEdit.getText().getIntValue();
    m.minOut = outMinEdit.getText().getIntValue();
    m.maxOut = outMaxEdit.getText().getIntValue();
    m.inverted = invertedToggle.getToggleState();
    m.relative = relativeToggle.getToggleState();
    m.smoothing = float(smoothingSlider.getValue());
    return m;
}
