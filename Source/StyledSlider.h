#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
// StyledSlider - סליידר מעוצב עם תווית וצבע אקסנט
//==============================================================================
class StyledSlider : public juce::Component {
public:
    enum Style { Vertical, Horizontal, Knob };
    
    StyledSlider(const juce::String& name, Style style = Vertical, 
                 juce::Colour accent = juce::Colour())
        : paramName(name), sliderStyle(style), accentColor(accent)
    {
        if (!accentColor.isOpaque())
            accentColor = DS::Color::Accent::primary();
        
        // Configure slider
        slider.setColour(juce::Slider::thumbColourId, accentColor);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
        
        switch (style) {
            case Vertical:
                slider.setSliderStyle(juce::Slider::LinearVertical);
                break;
            case Horizontal:
                slider.setSliderStyle(juce::Slider::LinearHorizontal);
                slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 18);
                break;
            case Knob:
                slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
                break;
        }
        addAndMakeVisible(slider);
        
        // Label
        nameLabel.setText(name, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centred);
        nameLabel.setFont(DS::Font::label());
        nameLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        addAndMakeVisible(nameLabel);
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        
        if (sliderStyle == Horizontal) {
            auto labelWidth = juce::jmin(60, bounds.getWidth() / 3);
            nameLabel.setBounds(bounds.removeFromLeft(labelWidth));
            bounds.removeFromLeft(DS::Space::S);
            slider.setBounds(bounds);
        } else if (sliderStyle == Knob) {
            // Fixed knob size - INCREASED from 42 to 52
            constexpr int knobSize = 52;
            constexpr int textboxH = 18;
            nameLabel.setBounds(bounds.removeFromTop(18));
            bounds.removeFromTop(DS::Space::XS);
            
            int sliderW = std::min(bounds.getWidth(), knobSize);
            int sliderH = std::min(bounds.getHeight(), knobSize + textboxH);
            auto sliderBounds = bounds.withSizeKeepingCentre(sliderW, sliderH);
            slider.setBounds(sliderBounds);
        } else {
            nameLabel.setBounds(bounds.removeFromTop(18));
            bounds.removeFromTop(DS::Space::XS);
            slider.setBounds(bounds);
        }
    }
    
    juce::Slider& getSlider() { return slider; }
    const juce::Slider& getSlider() const { return slider; }
    
    void setRange(double min, double max, double interval = 1.0) {
        slider.setRange(min, max, interval);
    }
    
    void setValue(double value, juce::NotificationType notification = juce::sendNotification) {
        slider.setValue(value, notification);
    }
    
    double getValue() const { return slider.getValue(); }
    
    void setAccentColor(juce::Colour color) {
        accentColor = color;
        slider.setColour(juce::Slider::thumbColourId, color);
        repaint();
    }
    
    juce::Colour getAccentColor() const { return accentColor; }
    
    void addListener(juce::Slider::Listener* listener) {
        slider.addListener(listener);
    }
    
    void removeListener(juce::Slider::Listener* listener) {
        slider.removeListener(listener);
    }
    
    // Tooltip support
    void setTooltip(const juce::String& newTooltip) {
        slider.setTooltip(newTooltip);
    }
    
private:
    juce::String paramName;
    Style sliderStyle;
    juce::Colour accentColor;
    juce::Slider slider;
    juce::Label nameLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StyledSlider)
};
