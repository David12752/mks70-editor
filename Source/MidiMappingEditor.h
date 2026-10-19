#pragma once

#include <JuceHeader.h>
#include "MidiLearnManager.h"

/**
 * Simple dialog for editing a single MIDI mapping.
 */
class MidiMappingEditor : public juce::Component,
                          private juce::Button::Listener,
                          private juce::Timer
{
public:
    MidiMappingEditor(const juce::String& paramId, MidiLearnManager& mgr);
    ~MidiMappingEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onClose; // owner deletes onClose

private:
    void buttonClicked(juce::Button* b) override;
    void timerCallback() override;

    void loadFromMapping();
    MidiLearnManager::MidiMapping getEdited() const;

    MidiLearnManager& manager;
    juce::String paramId;

    // UI widgets
    juce::Label title;
    juce::ComboBox typeBox;
    juce::Label numberLabel, channelLabel, inputLabel, outputLabel, optionsLabel, smoothLabel, monitorLabel;
    juce::TextEditor numberEdit, inMinEdit, inMaxEdit, outMinEdit, outMaxEdit;
    juce::ComboBox channelBox;
    juce::ToggleButton invertedToggle, relativeToggle;
    juce::Slider smoothingSlider; // 0..1
    juce::Label monitorValue;
    juce::TextButton testButton { "Test" }, okButton { "OK" }, cancelButton { "Cancel" };
};
