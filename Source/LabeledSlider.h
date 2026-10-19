#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "ValuePopup.h"

class MKS70EditorAudioProcessorEditor;
class MidiLearnManager;

// 3D Minimoog-style LookAndFeel for rotary sliders with glow ring
class MinimoogKnobLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(6);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        
        // === GLOWING POSITION RING (shows current position) ===
        float ringRadius = radius + 4.0f;
        
        // Background ring (dark)
        g.setColour(juce::Colour(0xFF151515));
        g.drawEllipse(centreX - ringRadius, centreY - ringRadius, ringRadius * 2, ringRadius * 2, 3.0f);
        
        // Glowing arc showing position
        juce::Path arcPath;
        arcPath.addCentredArc(centreX, centreY, ringRadius, ringRadius, 0,
                             rotaryStartAngle, angle, true);
        
        // Glow effect (multiple layers)
        g.setColour(DS::Color::Accent::cyan().withAlpha(0.2f));
        g.strokePath(arcPath, juce::PathStrokeType(6.0f));
        g.setColour(DS::Color::Accent::cyan().withAlpha(0.5f));
        g.strokePath(arcPath, juce::PathStrokeType(4.0f));
        g.setColour(DS::Color::Accent::cyan());
        g.strokePath(arcPath, juce::PathStrokeType(2.0f));
        
        // === KNOB BODY ===
        // Outer shadow
        g.setColour(juce::Colour(0x40000000));
        g.fillEllipse(rx + 2, ry + 2, rw, rw);
        
        // 3D knob body - gradient for depth
        juce::ColourGradient gradient(
            juce::Colour(0xFF3A3A3A), centreX - radius * 0.5f, centreY - radius * 0.5f,
            juce::Colour(0xFF0A0A0A), centreX + radius * 0.5f, centreY + radius * 0.5f,
            true);
        g.setGradientFill(gradient);
        g.fillEllipse(rx, ry, rw, rw);
        
        // Highlight on top-left for 3D effect
        g.setColour(juce::Colour(0xFF606060));
        g.fillEllipse(rx + 3, ry + 3, rw * 0.35f, rw * 0.35f);
        
        // Outer ring (metallic)
        g.setColour(juce::Colour(0xFF505050));
        g.drawEllipse(rx, ry, rw, rw, 2.0f);
        
        // Pointer line with glow
        juce::Path pointer;
        auto pointerLength = radius * 0.55f;
        auto pointerThickness = 3.5f;
        pointer.addRectangle(-pointerThickness * 0.5f, -radius + 6, pointerThickness, pointerLength);
        
        // Pointer glow
        g.setColour(DS::Color::Accent::cyan().withAlpha(0.4f));
        g.fillPath(pointer.createPathWithRoundedCorners(1.5f), 
                   juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(DS::Color::Accent::cyan());
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        
        // Center dot (metal look)
        g.setColour(juce::Colour(0xFF707070));
        g.fillEllipse(centreX - 4, centreY - 4, 8, 8);
        g.setColour(juce::Colour(0xFF404040));
        g.drawEllipse(centreX - 4, centreY - 4, 8, 8, 1.0f);
    }
};

// Custom slider that forwards right-click to parent for MIDI Learn
// Now includes value popup on hover and double-click to edit
class MidiLearnableSlider : public juce::Slider {
public:
    std::function<void()> onRightClick;
    std::function<void(bool)> onHoverChanged;
    std::function<void(double)> onDragValue;
    std::function<void()> onDoubleClick;
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isRightButtonDown() && onRightClick) {
            onRightClick();
            return;
        }
        juce::Slider::mouseDown(e);
    }
    
    void mouseDoubleClick(const juce::MouseEvent& e) override {
        if (e.mods.isLeftButtonDown()) {
            if (onDoubleClick) {
                onDoubleClick();
                return;
            }
            // Default: show text editor
            showTextBox();
        }
    }
    
    void mouseEnter(const juce::MouseEvent& e) override {
        juce::Slider::mouseEnter(e);
        if (onHoverChanged) onHoverChanged(true);
    }
    
    void mouseExit(const juce::MouseEvent& e) override {
        juce::Slider::mouseExit(e);
        if (onHoverChanged) onHoverChanged(false);
    }
    
    void mouseDrag(const juce::MouseEvent& e) override {
        juce::Slider::mouseDrag(e);
        if (onDragValue) onDragValue(getValue());
    }
};

class LabeledSlider : public juce::Component {
public:
    enum Style { Vertical, Horizontal, Knob };
    
    LabeledSlider(const juce::String& name, Style style = Vertical)
        : paramName(name), sliderStyle(style)
    {
        // Label - small font for knobs
        nameLabel.setText(name, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centred);
        nameLabel.setFont(juce::Font(style == Knob ? 9.0f : 10.0f, juce::Font::bold));
        nameLabel.setColour(juce::Label::textColourId, DesignSystem::Color::TextSecondary());
        addAndMakeVisible(nameLabel);
        
        // Slider with right-click handler and hover popup
        slider.onRightClick = [this]() { showContextMenu(); };
        slider.onHoverChanged = [this](bool hovered) {
            if (hovered) {
                showValuePopup();
            } else {
                hideValuePopup();
            }
        };
        slider.onDragValue = [this](double) {
            updateValuePopup();
        };
        slider.onValueChange = [this]() {
            updateValuePopup();
        };
        
        switch (style) {
            case Vertical:
                slider.setSliderStyle(juce::Slider::LinearVertical);
                slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 45, 14);
                break;
            case Horizontal:
                slider.setSliderStyle(juce::Slider::LinearHorizontal);
                slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 14);
                break;
            case Knob:
                slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
                slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 14);
                // Apply 3D Minimoog-style LookAndFeel for realistic appearance
                slider.setLookAndFeel(&knobLookAndFeel);
                break;
        }
        slider.setColour(juce::Slider::textBoxTextColourId, DesignSystem::Color::TextPrimary());
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(slider);
        
        // Store default value
        defaultValue = 64.0;
        
        // Create value popup
        valuePopup = std::make_unique<ValuePopup>();
    }
    
    ~LabeledSlider() override;
    
    void resized() override {
        using namespace DesignSystem;
        auto bounds = getLocalBounds();
        constexpr int tinyLabel = 11; // Minimal label height for knobs
        
        switch (sliderStyle) {
            case Vertical:
                nameLabel.setBounds(bounds.removeFromTop(Size::LabelHeight));
                slider.setBounds(bounds);
                break;
                
            case Horizontal:
                nameLabel.setBounds(bounds.removeFromLeft(40));
                bounds.removeFromLeft(2);
                slider.setBounds(bounds);
                break;
                
            case Knob: {
                // Label at top - compact
                nameLabel.setBounds(bounds.removeFromTop(12));
                
                // Realistic knob - reduced 30% (91px from 130px)
                constexpr int knobSize = 91;
                constexpr int textboxH = 18;
                int totalH = knobSize + textboxH;
                int sliderW = std::min(bounds.getWidth(), knobSize);
                int sliderH = std::min(bounds.getHeight(), totalH);
                
                auto sliderBounds = bounds.withSizeKeepingCentre(sliderW, sliderH);
                slider.setBounds(sliderBounds);
                break;
            }
        }
    }
    
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    
    juce::Slider& getSlider() { return slider; }
    
    void setRange(double min, double max, double interval = 1.0) {
        slider.setRange(min, max, interval);
    }
    
    void setValue(double value, juce::NotificationType notification = juce::sendNotification) {
        slider.setValue(value, notification);
    }
    
    double getValue() const { return slider.getValue(); }
    
    // For custom value display
    void setValueFormatter(std::function<juce::String(double)> formatter) {
        slider.textFromValueFunction = [formatter](double v) { return formatter(v); };
        if (slider.isTextBoxEditable())
            slider.setValue(slider.getValue(), juce::dontSendNotification);
    }
    
    // MIDI Learn support
    void setEditor(MKS70EditorAudioProcessorEditor* ed) { editor = ed; }
    void setParameterName(const juce::String& name) { midiParamName = name; }
    const juce::String& getParameterName() const { return midiParamName; }
    
    // Set default value for reset
    void setDefaultValue(double val) { defaultValue = val; }
    
    // Value popup support - set unit for display (e.g., "Hz", "ms", "%")
    void setValueUnit(const juce::String& unit) { valueUnit = unit; }
    
    // Copy/Paste support
    static double clipboardValue;
    static bool hasClipboardValue;

private:
    juce::String paramName;
    juce::String midiParamName;
    juce::String valueUnit;
    Style sliderStyle;
    juce::Label nameLabel;
    MidiLearnableSlider slider;
    double defaultValue = 64.0;
    
    // 3D Minimoog-style LookAndFeel for knob mode
    MinimoogKnobLookAndFeel knobLookAndFeel;
    
    // Value popup
    std::unique_ptr<ValuePopup> valuePopup;
    bool popupVisible = false;
    
    void showValuePopup() {
        if (valuePopup == nullptr) return;
        
        juce::String valueStr;
        if (slider.textFromValueFunction) {
            valueStr = slider.textFromValueFunction(slider.getValue());
        } else {
            valueStr = juce::String(slider.getValue(), slider.getInterval() < 1 ? 1 : 0);
        }
        
        valuePopup->show(&slider, valueStr, valueUnit);
        popupVisible = true;
    }
    
    void updateValuePopup() {
        if (valuePopup == nullptr || !popupVisible) return;
        
        juce::String valueStr;
        if (slider.textFromValueFunction) {
            valueStr = slider.textFromValueFunction(slider.getValue());
        } else {
            valueStr = juce::String(slider.getValue(), slider.getInterval() < 1 ? 1 : 0);
        }
        
        valuePopup->updateValue(valueStr, valueUnit);
    }
    
    void hideValuePopup() {
        if (valuePopup != nullptr) {
            valuePopup->hide();
            popupVisible = false;
        }
    }
    
    // MIDI Learn
    MKS70EditorAudioProcessorEditor* editor = nullptr;
    bool midiSinkRegistered = false;
    
    void showContextMenu();
    void showLearnMenu();
    void showMappingEditor();
};
