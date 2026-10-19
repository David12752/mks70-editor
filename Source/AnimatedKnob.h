/*
  ==============================================================================

    AnimatedKnob.h
    Knob component with smooth value animation

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
    AnimatedKnob - A slider knob with smooth value animation
*/
class AnimatedKnob : public juce::Slider, private juce::Timer
{
public:
    AnimatedKnob()
    {
        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
        setRange(0, 127, 1);
        
        // Animation settings
        animationDuration = 150;  // ms
    }
    
    ~AnimatedKnob() override
    {
        stopTimer();
    }
    
    // Set value with smooth animation
    void setValueAnimated(double newValue)
    {
        if (!enableAnimation)
        {
            setValue(newValue, juce::dontSendNotification);
            return;
        }
        
        targetValue = newValue;
        startValue = getValue();
        animationProgress = 0.0f;
        startTimerHz(60);
    }
    
    // Enable/disable animation
    void setAnimationEnabled(bool enabled) { enableAnimation = enabled; }
    bool isAnimationEnabled() const { return enableAnimation; }
    
    // Set animation duration in milliseconds
    void setAnimationDuration(int durationMs) { animationDuration = durationMs; }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto knobArea = bounds.removeFromTop(bounds.getWidth());
        
        // Draw glow when animating
        if (isTimerRunning())
        {
            float glowIntensity = std::sin(animationProgress * juce::MathConstants<float>::pi);
            auto glowColour = DS::Colors::accent.withAlpha(glowIntensity * 0.3f);
            
            juce::ColourGradient glow(
                glowColour,
                knobArea.getCentreX(), knobArea.getCentreY(),
                juce::Colours::transparentBlack,
                knobArea.getX(), knobArea.getCentreY(),
                true
            );
            g.setGradientFill(glow);
            g.fillEllipse(knobArea.expanded(5));
        }
        
        juce::Slider::paint(g);
        
        // Draw value change indicator arc during animation
        if (isTimerRunning() && enableValueArc)
        {
            float startAngle = valueToAngle(startValue);
            float targetAngle = valueToAngle(targetValue);
            
            g.setColour(DS::Colors::accent.withAlpha(0.5f));
            
            juce::Path arc;
            auto arcBounds = knobArea.reduced(5);
            arc.addCentredArc(arcBounds.getCentreX(), arcBounds.getCentreY(),
                             arcBounds.getWidth() / 2 - 2, arcBounds.getHeight() / 2 - 2,
                             0.0f, startAngle, targetAngle, true);
            
            g.strokePath(arc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
        }
    }
    
    void setValueArcEnabled(bool enabled) { enableValueArc = enabled; }
    
private:
    double targetValue = 0;
    double startValue = 0;
    float animationProgress = 0.0f;
    int animationDuration = 150;
    bool enableAnimation = true;
    bool enableValueArc = true;
    
    void timerCallback() override
    {
        float stepSize = 1.0f / (animationDuration / (1000.0f / 60.0f));
        animationProgress += stepSize;
        
        if (animationProgress >= 1.0f)
        {
            animationProgress = 1.0f;
            stopTimer();
            setValue(targetValue, juce::dontSendNotification);
        }
        else
        {
            // Ease-out cubic interpolation
            float t = 1.0f - std::pow(1.0f - animationProgress, 3.0f);
            double currentValue = startValue + (targetValue - startValue) * t;
            setValue(currentValue, juce::dontSendNotification);
        }
        
        repaint();
    }
    
    float valueToAngle(double value) const
    {
        double normalized = (value - getMinimum()) / (getMaximum() - getMinimum());
        float rotaryStartAngle = juce::MathConstants<float>::pi * 1.2f;
        float rotaryEndAngle = juce::MathConstants<float>::pi * 2.8f;
        return rotaryStartAngle + (float)normalized * (rotaryEndAngle - rotaryStartAngle);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimatedKnob)
};

//==============================================================================
/**
    BouncyKnob - A knob that bounces when value changes
*/
class BouncyKnob : public juce::Slider, private juce::Timer
{
public:
    BouncyKnob()
    {
        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    }
    
    ~BouncyKnob() override
    {
        stopTimer();
    }
    
    void valueChanged() override
    {
        // Trigger bounce animation on value change
        if (bounceEnabled && !isDragging)
        {
            bouncePhase = 0;
            bounceAmplitude = 0.15f;  // Initial bounce amount
            startTimerHz(60);
        }
        
        juce::Slider::valueChanged();
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        isDragging = true;
        juce::Slider::mouseDown(e);
    }
    
    void mouseUp(const juce::MouseEvent& e) override
    {
        isDragging = false;
        juce::Slider::mouseUp(e);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Apply scale transform for bounce effect
        auto bounds = getLocalBounds().toFloat();
        auto centre = bounds.getCentre();
        
        float scale = 1.0f + currentBounce;
        
        g.addTransform(juce::AffineTransform::scale(scale, scale, centre.x, centre.y));
        
        juce::Slider::paint(g);
    }
    
    void setBounceEnabled(bool enabled) { bounceEnabled = enabled; }
    
private:
    bool bounceEnabled = true;
    bool isDragging = false;
    int bouncePhase = 0;
    float bounceAmplitude = 0.0f;
    float currentBounce = 0.0f;
    
    void timerCallback() override
    {
        bouncePhase++;
        
        // Spring physics simulation
        float damping = 0.85f;
        float frequency = 0.4f;
        
        currentBounce = bounceAmplitude * std::sin(bouncePhase * frequency) * std::pow(damping, bouncePhase);
        
        if (bouncePhase > 30 || std::abs(currentBounce) < 0.001f)
        {
            currentBounce = 0;
            stopTimer();
        }
        
        repaint();
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BouncyKnob)
};

//==============================================================================
/**
    GlowKnob - A knob that glows when active/changing
*/
class GlowKnob : public juce::Slider, private juce::Timer
{
public:
    GlowKnob()
    {
        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    }
    
    ~GlowKnob() override
    {
        stopTimer();
    }
    
    void mouseEnter(const juce::MouseEvent& e) override
    {
        targetGlow = 0.5f;
        startTimerHz(60);
        juce::Slider::mouseEnter(e);
    }
    
    void mouseExit(const juce::MouseEvent& e) override
    {
        targetGlow = 0.0f;
        startTimerHz(60);
        juce::Slider::mouseExit(e);
    }
    
    void valueChanged() override
    {
        // Flash on value change
        currentGlow = 1.0f;
        targetGlow = isMouseOver() ? 0.5f : 0.0f;
        startTimerHz(60);
        
        juce::Slider::valueChanged();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto knobArea = bounds.removeFromTop(bounds.getWidth());
        
        // Draw glow
        if (currentGlow > 0.01f)
        {
            auto glowColour = glowColor.withAlpha(currentGlow * 0.4f);
            
            // Multiple glow layers for smooth effect
            for (int i = 3; i >= 1; i--)
            {
                g.setColour(glowColour.withMultipliedAlpha(1.0f / i));
                g.fillEllipse(knobArea.expanded(i * 3.0f));
            }
        }
        
        juce::Slider::paint(g);
    }
    
    void setGlowColor(juce::Colour colour) { glowColor = colour; }
    
private:
    float currentGlow = 0.0f;
    float targetGlow = 0.0f;
    juce::Colour glowColor = DS::Colors::accent;
    
    void timerCallback() override
    {
        // Smooth interpolation towards target
        float speed = 0.2f;
        currentGlow += (targetGlow - currentGlow) * speed;
        
        if (std::abs(currentGlow - targetGlow) < 0.01f)
        {
            currentGlow = targetGlow;
            if (targetGlow == 0.0f)
                stopTimer();
        }
        
        repaint();
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlowKnob)
};
