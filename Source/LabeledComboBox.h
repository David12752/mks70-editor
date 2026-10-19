#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

class LabeledComboBox : public juce::Component {
public:
    enum Layout { Stacked, Inline, Compact };
    
    LabeledComboBox(const juce::String& name, Layout layout = Stacked)
        : paramName(name), layoutStyle(layout)
    {
        nameLabel.setText(name, juce::dontSendNotification);
        nameLabel.setJustificationType(layout == Inline ? 
            juce::Justification::centredRight : juce::Justification::centred);
        nameLabel.setFont(juce::Font(8.0f, juce::Font::bold));
        nameLabel.setColour(juce::Label::textColourId, DesignSystem::Color::TextSecondary());
        addAndMakeVisible(nameLabel);
        
        comboBox.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(comboBox);
    }
    
    void resized() override {
        using namespace DesignSystem;
        auto bounds = getLocalBounds();
        
        if (layoutStyle == Compact) {
            // Ultra-compact: tiny label above tiny combo
            nameLabel.setBounds(bounds.removeFromTop(10));
            comboBox.setBounds(bounds.withHeight(Size::ComboHeightTiny));
        }
        else if (layoutStyle == Stacked) {
            nameLabel.setBounds(bounds.removeFromTop(12));
            comboBox.setBounds(bounds.withHeight(Size::ComboBoxHeight));
        } else {
            auto labelWidth = juce::jmin(40, bounds.getWidth() / 3);
            nameLabel.setBounds(bounds.removeFromLeft(labelWidth));
            bounds.removeFromLeft(2);
            comboBox.setBounds(bounds.withHeight(Size::ComboBoxHeight)
                                     .withY((bounds.getHeight() - Size::ComboBoxHeight) / 2));
        }
    }
    
    juce::ComboBox& getComboBox() { return comboBox; }
    
    void addItem(const juce::String& text, int itemId) {
        comboBox.addItem(text, itemId);
    }
    
    void addItems(const juce::StringArray& items) {
        int id = 1;
        for (const auto& item : items)
            comboBox.addItem(item, id++);
    }
    
    // Convenience: support initializer lists like {"A", "B"}
    void addItems(std::initializer_list<juce::String> items) {
        int id = comboBox.getNumItems() + 1;
        for (const auto& item : items)
            comboBox.addItem(item, id++);
    }
    
    void setSelectedId(int id, juce::NotificationType notification = juce::sendNotification) {
        comboBox.setSelectedId(id, notification);
    }
    
    int getSelectedId() const { return comboBox.getSelectedId(); }

private:
    juce::String paramName;
    Layout layoutStyle;
    juce::Label nameLabel;
    juce::ComboBox comboBox;
};
