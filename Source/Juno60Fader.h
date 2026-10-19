#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "MacroControl.h"

class MKS70EditorAudioProcessorEditor;

/**
 * Juno-60 style realistic vertical fader
 * Features:
 * - Brushed aluminum track
 * - Black plastic cap with white line indicator
 * - Subtle 3D lighting effects
 * - Value popup on hover
 * - Macro Learn support
 */
class Juno60Fader : public juce::Component {
public:
    Juno60Fader(const juce::String& name = "")
        : paramName(name)
    {
        // Label
        nameLabel.setText(name, juce::dontSendNotification);
        nameLabel.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
        nameLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        nameLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(nameLabel);
        
        // Slider setup
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 14);
        slider.setColour(juce::Slider::textBoxTextColourId, DS::Color::textPrimary());
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider.setLookAndFeel(&faderLookAndFeel);
        
        // Enable double-click reset to 0 by default (will be overridden by setDefaultValue)
        slider.setDoubleClickReturnValue(true, 0.0);
        
        // Listen for value changes to support Macro Learn
        slider.onValueChange = [this]() { notifyMacroLearn(); };
        
        addAndMakeVisible(slider);
    }
    
    ~Juno60Fader() override {
        slider.setLookAndFeel(nullptr);
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        nameLabel.setBounds(bounds.removeFromTop(12));
        bounds.removeFromBottom(2);
        slider.setBounds(bounds);
    }
    
    juce::Slider& getSlider() { return slider; }
    
    void setRange(double min, double max, double interval = 1.0) {
        slider.setRange(min, max, interval);
    }
    
    void setValue(double value, juce::NotificationType notification = juce::sendNotification) {
        slider.setValue(value, notification);
    }
    
    double getValue() const { return slider.getValue(); }
    
    void setDefaultValue(double val) { 
        defaultValue = val; 
        slider.setDoubleClickReturnValue(true, val);
    }
    
    // Enable double-click reset to center (for sliders without specific default)
    void enableDoubleClickReset(double centerValue = 64.0) {
        slider.setDoubleClickReturnValue(true, centerValue);
    }
    
    // MIDI Learn support
    void setEditor(MKS70EditorAudioProcessorEditor* ed) { editor = ed; }
    void setParameterName(const juce::String& name) { midiParamName = name; }
    
    // Macro Learn support
    void setMacroManager(MacroManager* mgr) { macroManager = mgr; }
    
    void notifyMacroLearn() {
        if (macroManager != nullptr && macroManager->isLearning()) {
            juce::String paramId = midiParamName.isEmpty() ? paramName : midiParamName;
            if (paramId.isNotEmpty()) {
                macroManager->addLearnedParameter(paramId, static_cast<float>(slider.getValue()));
            }
        }
    }

private:
    // Custom LookAndFeel for premium Arturia/NI style fader
    class Juno60FaderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            (void)minSliderPos;
            (void)maxSliderPos;
            (void)style;
            (void)slider;
            
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
            
            // Track dimensions - wider slot with maximum travel range
            const float trackWidth = 12.0f;  // Wider track
            const float trackX = bounds.getCentreX() - trackWidth / 2.0f;
            const float trackY = bounds.getY() + 1.0f;  // Minimal top margin - increased travel!
            const float trackHeight = bounds.getHeight() - 3.0f;  // Maximum travel range - taller slot!
            
            // === SLOT GROOVE BACKGROUND (realistic fader slot) ===
            juce::Rectangle<float> slotBounds(trackX - 4, trackY - 2, trackWidth + 8, trackHeight + 4);
            
            // Outer dark frame
            g.setColour(juce::Colour(0xFF0A0A0A));
            g.fillRoundedRectangle(slotBounds, 3.0f);
            
            // Inner slot shadow gradient (3D depth)
            juce::ColourGradient slotGrad(
                juce::Colour(0xFF151515), slotBounds.getX(), slotBounds.getY(),
                juce::Colour(0xFF2A2A2A), slotBounds.getRight(), slotBounds.getY(),
                false
            );
            g.setGradientFill(slotGrad);
            g.fillRoundedRectangle(slotBounds.reduced(1.5f), 2.5f);
            
            // === SCALE MARKINGS (Arturia style) ===
            g.setColour(DS::Color::textSecondary().withAlpha(0.5f));
            const int numMarks = 11;
            for (int i = 0; i < numMarks; i++) {
                float yPos = trackY + (trackHeight - 4) * (float)i / (numMarks - 1) + 2;
                float markWidth = (i == 0 || i == 5 || i == 10) ? 8.0f : 4.0f;
                float markHeight = (i == 0 || i == 5 || i == 10) ? 1.5f : 1.0f;
                // Left side marks
                g.fillRect(trackX - markWidth - 5, yPos - markHeight/2, markWidth, markHeight);
                // Right side marks
                g.fillRect(trackX + trackWidth + 5, yPos - markHeight/2, markWidth, markHeight);
            }
            
            // === CENTER TRACK GROOVE (visible travel slot) ===
            juce::Rectangle<float> trackBounds(trackX, trackY, trackWidth, trackHeight);
            
            // Deep recessed track groove
            juce::ColourGradient trackGrad(
                juce::Colour(0xFF080808), trackBounds.getCentreX() - 2, trackBounds.getY(),
                juce::Colour(0xFF1A1A1A), trackBounds.getCentreX() + 2, trackBounds.getY(),
                false
            );
            g.setGradientFill(trackGrad);
            g.fillRoundedRectangle(trackBounds.reduced(1, 0), 2.0f);
            
            // Track inner highlight (metal reflection)
            g.setColour(juce::Colour(0xFF303030));
            g.drawLine(trackX + trackWidth/2, trackY + 4, trackX + trackWidth/2, trackY + trackHeight - 4, 1.0f);
            
            // Side rails (raised edges of the slot)
            g.setColour(juce::Colour(0xFF3A3A3A));
            g.fillRect(trackX - 1, trackY, 1.5f, trackHeight);
            g.fillRect(trackX + trackWidth - 0.5f, trackY, 1.5f, trackHeight);
            
            // === VALUE FILL (premium gradient glow) ===
            const float fillTop = sliderPos;
            const float fillBottom = trackY + trackHeight - 2;
            if (fillBottom > fillTop) {
                // Gradient fill with accent color
                juce::ColourGradient fillGradient(
                    DS::Color::Accent::cyan().withAlpha(0.15f), trackX, fillTop,
                    DS::Color::Accent::cyan().withAlpha(0.7f), trackX, fillBottom,
                    false
                );
                g.setGradientFill(fillGradient);
                g.fillRoundedRectangle(trackX, fillTop, trackWidth, fillBottom - fillTop, 1.5f);
                
                // Bright center line
                g.setColour(DS::Color::Accent::cyan().withAlpha(0.9f));
                g.fillRect(trackX + trackWidth/2 - 0.5f, fillTop + 2, 1.0f, fillBottom - fillTop - 4);
            }
            
            // === FADER CAP (Arturia/Prophet style - larger for easier grabbing) ===
            const float capWidth = 32.0f;  // Wider cap
            const float capHeight = 18.0f;  // Taller cap
            const float capX = bounds.getCentreX() - capWidth / 2.0f;
            const float capY = sliderPos - capHeight / 2.0f;
            
            juce::Rectangle<float> capBounds(capX, capY, capWidth, capHeight);
            
            // Cap drop shadow (soft)
            for (int i = 3; i >= 1; i--) {
                g.setColour(juce::Colour(0x10000000 * i));
                g.fillRoundedRectangle(capBounds.translated(0, i + 1).expanded(i * 0.5f), 4.0f);
            }
            
            // Cap body - premium brushed metal gradient
            juce::ColourGradient capGradient(
                juce::Colour(0xFF5A5A5A), capBounds.getX(), capBounds.getY(),
                juce::Colour(0xFF2A2A2A), capBounds.getX(), capBounds.getCentreY(),
                false
            );
            capGradient.addColour(0.5, juce::Colour(0xFF3A3A3A));
            capGradient.addColour(1.0, juce::Colour(0xFF1A1A1A));
            g.setGradientFill(capGradient);
            g.fillRoundedRectangle(capBounds, 4.0f);
            
            // Top highlight (glossy reflection)
            juce::ColourGradient highlightGrad(
                juce::Colour(0x40FFFFFF), capBounds.getX(), capBounds.getY(),
                juce::Colour(0x00FFFFFF), capBounds.getX(), capBounds.getY() + 4,
                false
            );
            g.setGradientFill(highlightGrad);
            g.fillRoundedRectangle(capX + 2, capY + 1, capWidth - 4, 4.0f, 2.0f);
            
            // Side bevels for 3D effect
            g.setColour(juce::Colour(0xFF4A4A4A));
            g.fillRect(capX + 1, capY + 3, 1.5f, capHeight - 6);
            g.setColour(juce::Colour(0xFF151515));
            g.fillRect(capX + capWidth - 2.5f, capY + 3, 1.5f, capHeight - 6);
            
            // Indicator line (white with glow)
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.fillRect(capX + capWidth / 2.0f - 1.5f, capY + 2.0f, 3.0f, capHeight - 4.0f);
            g.setColour(juce::Colours::white);
            g.fillRect(capX + capWidth / 2.0f - 0.75f, capY + 3.0f, 1.5f, capHeight - 6.0f);
            
            // Grip texture (horizontal lines)
            g.setColour(juce::Colour(0x20000000));
            for (float ly = capY + 4; ly < capY + capHeight - 4; ly += 2.5f) {
                g.fillRect(capX + 4.0f, ly, capWidth - 8.0f, 1.0f);
            }
            
            // Cap border with subtle highlight
            g.setColour(juce::Colour(0xFF0D0D0D));
            g.drawRoundedRectangle(capBounds, 4.0f, 1.0f);
        }
    };
    
    Juno60FaderLookAndFeel faderLookAndFeel;
    juce::String paramName;
    juce::String midiParamName;
    juce::Label nameLabel;
    juce::Slider slider;
    double defaultValue = 64.0;
    MKS70EditorAudioProcessorEditor* editor = nullptr;
    MacroManager* macroManager = nullptr;
};

