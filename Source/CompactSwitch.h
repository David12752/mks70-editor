#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

class MKS70EditorAudioProcessorEditor;

//==============================================================================
// CompactSwitch - Small multi-position switch for 2-4 options
//==============================================================================
class CompactSwitch : public juce::Component {
public:
    CompactSwitch(const juce::String& labelText) : label(labelText) {
        for (int i = 0; i < 4; ++i) {
            buttons.add(new juce::TextButton());
            buttons[i]->setClickingTogglesState(true);
            buttons[i]->setRadioGroupId(1000 + (int)(intptr_t)this); // Unique group per instance
            buttons[i]->onClick = [this, i]() { 
                if (buttons[i]->getToggleState()) {
                    currentValue = i;
                    if (onChange) onChange(i);
                }
            };
            addAndMakeVisible(buttons[i]);
        }
        
        // Style buttons
        for (auto* btn : buttons) {
            btn->setColour(juce::TextButton::buttonColourId, DS::Color::bgCard());
            btn->setColour(juce::TextButton::buttonOnColourId, DS::Color::Section::chorus());
            btn->setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
            btn->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        }
    }
    
    void setOptions(const juce::StringArray& opts) {
        options = opts;
        numOptions = juce::jmin(4, options.size());
        
        // Hide unused buttons
        for (int i = 0; i < 4; ++i) {
            buttons[i]->setVisible(i < numOptions);
            if (i < numOptions) {
                buttons[i]->setButtonText(options[i]);
            }
        }
        
        if (numOptions > 0) {
            buttons[0]->setToggleState(true, juce::dontSendNotification);
        }
        
        resized();
    }
    
    void setValue(int value) {
        if (value >= 0 && value < numOptions) {
            currentValue = value;
            buttons[value]->setToggleState(true, juce::dontSendNotification);
        }
    }
    
    int getValue() const { return currentValue; }
    
    void setTooltip(const juce::String& tip) {
        for (auto* btn : buttons)
            if (btn->isVisible()) btn->setTooltip(tip);
    }
    
    std::function<void(int)> onChange;
    
    void resized() override {
        auto bounds = getLocalBounds();
        
        // Label on top
        auto labelArea = bounds.removeFromTop(14);
        
        bounds.removeFromTop(2);
        
        // Buttons in a row
        int buttonW = numOptions > 0 ? bounds.getWidth() / numOptions : 0;
        for (int i = 0; i < numOptions; ++i) {
            buttons[i]->setBounds(bounds.removeFromLeft(buttonW));
        }
    }
    
    void paint(juce::Graphics& g) override {
        // Draw label
        g.setColour(DS::Color::textSecondary());
        g.setFont(juce::Font(11.0f));
        g.drawText(label, 0, 0, getWidth(), 14, juce::Justification::centred);
    }
    
    // Editor integration
    void setEditor(MKS70EditorAudioProcessorEditor* ed) { editor = ed; }
    void setParameterName(const juce::String& name) { paramName = name; }
    
private:
    juce::String label;
    juce::StringArray options;
    juce::OwnedArray<juce::TextButton> buttons;
    int numOptions = 0;
    int currentValue = 0;
    
    MKS70EditorAudioProcessorEditor* editor = nullptr;
    juce::String paramName;
};
