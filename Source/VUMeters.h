#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * VUMeters.h
 * Voice activity meters and level displays
 * 
 * Part of Improvement #19: VU Meters
 */

namespace Metering
{

//==============================================================================
/**
 * Simple VU Meter - Vertical LED-style
 */
class VUMeter : public juce::Component, private juce::Timer
{
public:
    VUMeter()
    {
        startTimerHz(30);
    }
    
    ~VUMeter() override
    {
        stopTimer();
    }
    
    void setLevel(float newLevel)
    {
        targetLevel = juce::jlimit(0.0f, 1.0f, newLevel);
        
        // Track peak
        if (targetLevel > peakLevel)
        {
            peakLevel = targetLevel;
            peakHoldTime = 30; // frames
        }
    }
    
    void setColor(juce::Colour c) { meterColor = c; repaint(); }
    void setShowPeak(bool show) { showPeak = show; repaint(); }
    void setNumSegments(int num) { numSegments = num; repaint(); }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1);
        
        // Background
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, 3);
        
        // Calculate segments
        float segmentHeight = (bounds.getHeight() - (numSegments - 1) * 2) / numSegments;
        int litSegments = static_cast<int>(currentLevel * numSegments);
        int peakSegment = static_cast<int>(peakLevel * numSegments);
        
        for (int i = 0; i < numSegments; i++)
        {
            float y = bounds.getBottom() - (i + 1) * (segmentHeight + 2) + 2;
            auto segBounds = juce::Rectangle<float>(bounds.getX() + 2, y, bounds.getWidth() - 4, segmentHeight);
            
            juce::Colour segColor;
            float segPosition = static_cast<float>(i) / numSegments;
            
            if (segPosition > 0.85f)
                segColor = juce::Colour(0xFFef4444); // Red
            else if (segPosition > 0.7f)
                segColor = juce::Colour(0xFFf59e0b); // Yellow
            else
                segColor = meterColor;
            
            bool isLit = i < litSegments;
            bool isPeak = showPeak && i == peakSegment;
            
            if (isLit)
            {
                g.setColour(segColor);
                g.fillRoundedRectangle(segBounds, 2);
            }
            else if (isPeak)
            {
                g.setColour(segColor.withAlpha(0.8f));
                g.fillRoundedRectangle(segBounds, 2);
            }
            else
            {
                g.setColour(segColor.withAlpha(0.1f));
                g.fillRoundedRectangle(segBounds, 2);
            }
        }
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, 3, 1.0f);
    }
    
private:
    void timerCallback() override
    {
        // Smooth level changes
        float diff = targetLevel - currentLevel;
        if (std::abs(diff) < 0.01f)
            currentLevel = targetLevel;
        else
            currentLevel += diff * 0.3f;
        
        // Decay peak
        if (peakHoldTime > 0)
            peakHoldTime--;
        else
            peakLevel *= 0.95f;
        
        repaint();
    }
    
    float currentLevel = 0.0f;
    float targetLevel = 0.0f;
    float peakLevel = 0.0f;
    int peakHoldTime = 0;
    juce::Colour meterColor = DS::Color::accent;
    bool showPeak = true;
    int numSegments = 12;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};

//==============================================================================
/**
 * Stereo VU Meter
 */
class StereoMeter : public juce::Component
{
public:
    StereoMeter()
    {
        addAndMakeVisible(leftMeter);
        addAndMakeVisible(rightMeter);
        
        leftMeter.setColor(juce::Colour(0xFF6366f1));
        rightMeter.setColor(juce::Colour(0xFF6366f1));
    }
    
    void setLevels(float left, float right)
    {
        leftMeter.setLevel(left);
        rightMeter.setLevel(right);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(2);
        int meterWidth = (bounds.getWidth() - 4) / 2;
        
        leftMeter.setBounds(bounds.removeFromLeft(meterWidth));
        bounds.removeFromLeft(4);
        rightMeter.setBounds(bounds);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Labels
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        
        auto labelBounds = getLocalBounds().removeFromBottom(14);
        g.drawText("L", labelBounds.removeFromLeft(getWidth() / 2), juce::Justification::centred);
        g.drawText("R", labelBounds, juce::Justification::centred);
    }
    
private:
    VUMeter leftMeter;
    VUMeter rightMeter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoMeter)
};

//==============================================================================
/**
 * Voice Activity Display - Shows which voices are active
 */
class VoiceActivityDisplay : public juce::Component, private juce::Timer
{
public:
    static constexpr int NUM_VOICES = 6;  // MKS-70 has 6 voices
    
    VoiceActivityDisplay()
    {
        startTimerHz(15);
    }
    
    ~VoiceActivityDisplay() override
    {
        stopTimer();
    }
    
    void setVoiceActive(int voice, bool active, float level = 1.0f)
    {
        if (voice >= 0 && voice < NUM_VOICES)
        {
            voiceStates[voice].active = active;
            voiceStates[voice].level = level;
            
            if (active)
                voiceStates[voice].decayTime = 15; // frames
        }
    }
    
    void triggerVoice(int voice)
    {
        setVoiceActive(voice, true, 1.0f);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        auto contentBounds = bounds.reduced(DS::Space::sm);
        
        // Title
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText("VOICES", contentBounds.removeFromTop(14), juce::Justification::centred);
        
        contentBounds.removeFromTop(4);
        
        // Voice LEDs
        float ledSize = juce::jmin(contentBounds.getWidth() / NUM_VOICES - 4, contentBounds.getHeight() - 4);
        float totalWidth = NUM_VOICES * (ledSize + 4) - 4;
        float startX = contentBounds.getCentreX() - totalWidth / 2;
        
        for (int i = 0; i < NUM_VOICES; i++)
        {
            float x = startX + i * (ledSize + 4);
            float y = contentBounds.getCentreY() - ledSize / 2;
            
            auto ledBounds = juce::Rectangle<float>(x, y, ledSize, ledSize);
            
            // LED glow
            if (voiceStates[i].displayLevel > 0.1f)
            {
                g.setColour(DS::Color::accent.withAlpha(voiceStates[i].displayLevel * 0.3f));
                g.fillEllipse(ledBounds.expanded(4));
            }
            
            // LED background
            g.setColour(DS::Color::surfaceAlt);
            g.fillEllipse(ledBounds);
            
            // LED lit
            if (voiceStates[i].displayLevel > 0.01f)
            {
                g.setColour(DS::Color::accent.withAlpha(voiceStates[i].displayLevel));
                g.fillEllipse(ledBounds.reduced(2));
            }
            
            // LED border
            g.setColour(DS::Color::border);
            g.drawEllipse(ledBounds, 1.0f);
            
            // Voice number
            g.setColour(DS::Color::textDim);
            g.setFont(DS::Font::label.withHeight(9.0f));
            g.drawText(juce::String(i + 1), ledBounds, juce::Justification::centred);
        }
    }
    
private:
    void timerCallback() override
    {
        for (auto& state : voiceStates)
        {
            if (state.active)
            {
                state.displayLevel = state.level;
            }
            else if (state.decayTime > 0)
            {
                state.decayTime--;
                state.displayLevel = static_cast<float>(state.decayTime) / 15.0f;
            }
            else
            {
                state.displayLevel *= 0.9f;
            }
        }
        repaint();
    }
    
    struct VoiceState
    {
        bool active = false;
        float level = 0.0f;
        float displayLevel = 0.0f;
        int decayTime = 0;
    };
    
    std::array<VoiceState, NUM_VOICES> voiceStates;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceActivityDisplay)
};

//==============================================================================
/**
 * MIDI Activity Indicator
 */
class MIDIActivityLED : public juce::Component, private juce::Timer
{
public:
    MIDIActivityLED()
    {
        startTimerHz(30);
    }
    
    ~MIDIActivityLED() override
    {
        stopTimer();
    }
    
    void triggerIn() { inActivity = 1.0f; }
    void triggerOut() { outActivity = 1.0f; }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        float ledSize = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2 - 8;
        
        // IN LED
        auto inBounds = juce::Rectangle<float>(bounds.getX() + 4, bounds.getCentreY() - ledSize / 2, ledSize, ledSize);
        
        g.setColour(DS::Color::surfaceAlt);
        g.fillEllipse(inBounds);
        
        if (inActivity > 0.01f)
        {
            g.setColour(juce::Colour(0xFF10b981).withAlpha(inActivity));
            g.fillEllipse(inBounds.reduced(2));
        }
        
        g.setColour(DS::Color::border);
        g.drawEllipse(inBounds, 1.0f);
        
        // OUT LED
        auto outBounds = juce::Rectangle<float>(bounds.getRight() - ledSize - 4, bounds.getCentreY() - ledSize / 2, ledSize, ledSize);
        
        g.setColour(DS::Color::surfaceAlt);
        g.fillEllipse(outBounds);
        
        if (outActivity > 0.01f)
        {
            g.setColour(juce::Colour(0xFFf59e0b).withAlpha(outActivity));
            g.fillEllipse(outBounds.reduced(2));
        }
        
        g.setColour(DS::Color::border);
        g.drawEllipse(outBounds, 1.0f);
        
        // Labels
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText("IN", inBounds.translated(0, ledSize + 2), juce::Justification::centred);
        g.drawText("OUT", outBounds.translated(0, ledSize + 2), juce::Justification::centred);
    }
    
private:
    void timerCallback() override
    {
        if (inActivity > 0)
        {
            inActivity *= 0.85f;
            repaint();
        }
        if (outActivity > 0)
        {
            outActivity *= 0.85f;
            repaint();
        }
    }
    
    float inActivity = 0.0f;
    float outActivity = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDIActivityLED)
};

//==============================================================================
/**
 * Output Level Control with Meter
 */
class OutputLevelControl : public juce::Component
{
public:
    OutputLevelControl()
    {
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setRange(0.0, 1.0, 0.01);
        slider.setValue(0.75);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        addAndMakeVisible(slider);
        
        addAndMakeVisible(meter);
    }
    
    void setLevel(float level)
    {
        meter.setLevel(level);
    }
    
    float getGain() const { return static_cast<float>(slider.getValue()); }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(2);
        
        // Title area
        bounds.removeFromTop(16);
        
        // Meter on left
        meter.setBounds(bounds.removeFromLeft(20));
        bounds.removeFromLeft(4);
        
        // Slider
        slider.setBounds(bounds);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        // Label
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText("OUTPUT", bounds.reduced(4).removeFromTop(14), juce::Justification::centred);
    }
    
private:
    juce::Slider slider;
    VUMeter meter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputLevelControl)
};

} // namespace Metering
