#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * Floating value popup that appears above sliders/knobs on hover
 * Shows formatted parameter values with unit labels
 */
class ValuePopup : public juce::Component, public juce::Timer {
public:
    ValuePopup() {
        setAlwaysOnTop(true);
        setOpaque(false);
        setInterceptsMouseClicks(false, false);
    }
    
    void show(juce::Component* target, const juce::String& value, const juce::String& unit = "") {
        if (target == nullptr) return;
        
        currentValue = value;
        currentUnit = unit;
        targetComponent = target;
        
        // Calculate size based on text
        auto font = DS::Font::small().boldened();
        int textWidth = font.getStringWidth(value + (unit.isEmpty() ? "" : " " + unit));
        int popupW = juce::jmax(50, textWidth + 24);
        int popupH = 28;
        
        setSize(popupW, popupH);
        
        // Position above the target component
        if (auto* parent = target->getTopLevelComponent()) {
            auto targetBounds = target->getBoundsInParent();
            auto* targetParent = target->getParentComponent();
            while (targetParent != nullptr && targetParent != parent) {
                targetBounds = targetBounds.translated(targetParent->getX(), targetParent->getY());
                targetParent = targetParent->getParentComponent();
            }
            
            int x = targetBounds.getCentreX() - popupW / 2;
            int y = targetBounds.getY() - popupH - 6;
            
            // Keep within parent bounds
            x = juce::jlimit(4, parent->getWidth() - popupW - 4, x);
            y = juce::jmax(4, y);
            
            setBounds(x, y, popupW, popupH);
            
            if (getParentComponent() != parent) {
                parent->addAndMakeVisible(this);
            }
            
            setVisible(true);
            toFront(false);
            
            // Animation
            alpha = 0.0f;
            animating = true;
            startTimerHz(60);
        }
    }
    
    void hide() {
        fadeOut = true;
        if (!isTimerRunning()) startTimerHz(60);
    }
    
    void hideImmediately() {
        stopTimer();
        setVisible(false);
        targetComponent = nullptr;
    }
    
    void updateValue(const juce::String& value, const juce::String& unit = "") {
        currentValue = value;
        currentUnit = unit;
        
        // Resize if needed
        auto font = DS::Font::small().boldened();
        int textWidth = font.getStringWidth(value + (unit.isEmpty() ? "" : " " + unit));
        int popupW = juce::jmax(50, textWidth + 24);
        
        if (popupW != getWidth() && targetComponent != nullptr) {
            // Recenter
            int x = getBounds().getCentreX() - popupW / 2;
            setBounds(x, getY(), popupW, getHeight());
        }
        
        repaint();
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Drop shadow
        g.setColour(juce::Colours::black.withAlpha(0.3f * alpha));
        g.fillRoundedRectangle(bounds.translated(2, 2), 6.0f);
        
        // Background gradient
        juce::ColourGradient bgGrad(
            DS::Color::Accent::cyan().withAlpha(0.95f * alpha), bounds.getCentreX(), bounds.getY(),
            DS::Color::Accent::cyan().darker(0.3f).withAlpha(0.95f * alpha), bounds.getCentreX(), bounds.getBottom(),
            false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, 6.0f);
        
        // Border glow
        g.setColour(DS::Color::Accent::cyan().brighter(0.3f).withAlpha(0.6f * alpha));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
        
        // Text
        juce::String text = currentValue;
        if (currentUnit.isNotEmpty()) text += " " + currentUnit;
        
        g.setColour(juce::Colours::white.withAlpha(alpha));
        g.setFont(DS::Font::small().boldened());
        g.drawText(text, bounds, juce::Justification::centred);
        
        // Small triangle pointing down
        juce::Path triangle;
        float cx = bounds.getCentreX();
        float by = bounds.getBottom();
        triangle.addTriangle(cx - 6, by, cx + 6, by, cx, by + 5);
        g.fillPath(triangle);
    }
    
    void timerCallback() override {
        if (fadeOut) {
            alpha -= 0.15f;
            if (alpha <= 0) {
                alpha = 0;
                fadeOut = false;
                animating = false;
                setVisible(false);
                stopTimer();
            }
        } else if (animating) {
            alpha += 0.2f;
            if (alpha >= 1.0f) {
                alpha = 1.0f;
                animating = false;
                stopTimer();
            }
        }
        repaint();
    }
    
    bool isShowing() const { return isVisible() && alpha > 0.1f; }

private:
    juce::String currentValue;
    juce::String currentUnit;
    juce::Component* targetComponent = nullptr;
    float alpha = 0.0f;
    bool animating = false;
    bool fadeOut = false;
};

/**
 * Mixin class to add value popup functionality to any slider
 * Inherit from this and call the mouse methods
 */
class ValuePopupOwner {
public:
    ValuePopupOwner() {
        popup = std::make_unique<ValuePopup>();
    }
    
    virtual ~ValuePopupOwner() = default;
    
    void setValueUnit(const juce::String& unit) { valueUnit = unit; }
    
protected:
    void showValuePopup(juce::Component* slider, double value, std::function<juce::String(double)> formatter = nullptr) {
        if (popup == nullptr) return;
        
        juce::String valueStr;
        if (formatter) {
            valueStr = formatter(value);
        } else {
            valueStr = juce::String(value, 1);
        }
        
        popup->show(slider, valueStr, valueUnit);
    }
    
    void updateValuePopup(double value, std::function<juce::String(double)> formatter = nullptr) {
        if (popup == nullptr || !popup->isShowing()) return;
        
        juce::String valueStr;
        if (formatter) {
            valueStr = formatter(value);
        } else {
            valueStr = juce::String(value, 1);
        }
        
        popup->updateValue(valueStr, valueUnit);
    }
    
    void hideValuePopup() {
        if (popup != nullptr) popup->hide();
    }
    
    void hideValuePopupImmediately() {
        if (popup != nullptr) popup->hideImmediately();
    }
    
    std::unique_ptr<ValuePopup> popup;
    juce::String valueUnit;
};

/**
 * Slider with built-in value popup on hover
 */
class SliderWithPopup : public juce::Slider, public ValuePopupOwner {
public:
    SliderWithPopup() = default;
    
    void mouseEnter(const juce::MouseEvent& e) override {
        juce::Slider::mouseEnter(e);
        showValuePopup(this, getValue(), textFromValueFunction);
        hovered = true;
    }
    
    void mouseExit(const juce::MouseEvent& e) override {
        juce::Slider::mouseExit(e);
        hideValuePopup();
        hovered = false;
    }
    
    void mouseDrag(const juce::MouseEvent& e) override {
        juce::Slider::mouseDrag(e);
        if (hovered) updateValuePopup(getValue(), textFromValueFunction);
    }
    
    void valueChanged() override {
        juce::Slider::valueChanged();
        if (hovered && popup && popup->isShowing()) {
            updateValuePopup(getValue(), textFromValueFunction);
        }
    }

private:
    bool hovered = false;
};
