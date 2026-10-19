#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
// StyledComboBox - קומבובוקס מעוצב עם תווית
//==============================================================================
class StyledComboBox : public juce::Component {
public:
    enum Layout { Stacked, Inline };
    
    StyledComboBox(const juce::String& name, Layout layout = Stacked)
        : paramName(name), layoutStyle(layout)
    {
        nameLabel.setText(name, juce::dontSendNotification);
        nameLabel.setFont(DS::Font::label());
        nameLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        nameLabel.setJustificationType(layout == Stacked ? 
            juce::Justification::centred : juce::Justification::centredRight);
        addAndMakeVisible(nameLabel);
        
        addAndMakeVisible(comboBox);
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        
        if (layoutStyle == Stacked) {
            nameLabel.setBounds(bounds.removeFromTop(18));
            bounds.removeFromTop(DS::Space::XS);
            comboBox.setBounds(bounds.withHeight(DS::Size::ComboHeight));
        } else {
            auto labelWidth = juce::jmin(80, bounds.getWidth() / 3);
            nameLabel.setBounds(bounds.removeFromLeft(labelWidth));
            bounds.removeFromLeft(DS::Space::S);
            comboBox.setBounds(bounds.withHeight(DS::Size::ComboHeight)
                                     .withCentre({bounds.getCentreX(), bounds.getCentreY()}));
        }
    }
    
    juce::ComboBox& getComboBox() { return comboBox; }
    const juce::ComboBox& getComboBox() const { return comboBox; }
    
    void addItem(const juce::String& text, int id) { 
        comboBox.addItem(text, id); 
    }
    
    void addItems(const juce::StringArray& items) {
        for (int i = 0; i < items.size(); ++i)
            comboBox.addItem(items[i], i + 1);
    }
    
    // Support initializer_list like {"A", "B", "C"}
    void addItems(std::initializer_list<juce::String> items) {
        int id = comboBox.getNumItems() + 1;
        for (const auto& item : items)
            comboBox.addItem(item, id++);
    }
    
    void setSelectedId(int id, juce::NotificationType n = juce::sendNotification) { 
        comboBox.setSelectedId(id, n); 
    }
    
    int getSelectedId() const { return comboBox.getSelectedId(); }
    
    void addListener(juce::ComboBox::Listener* listener) {
        comboBox.addListener(listener);
    }
    
    void removeListener(juce::ComboBox::Listener* listener) {
        comboBox.removeListener(listener);
    }
    
    void clear(juce::NotificationType notification = juce::dontSendNotification) {
        comboBox.clear(notification);
    }
    
    // Tooltip support
    void setTooltip(const juce::String& newTooltip) {
        comboBox.setTooltip(newTooltip);
    }
    
private:
    juce::String paramName;
    Layout layoutStyle;
    juce::Label nameLabel;
    juce::ComboBox comboBox;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StyledComboBox)
};
