#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * StatusBar - Bottom bar showing MIDI connection status and activity
 * Displays: Connection state, MIDI In/Out indicators with animated LEDs, current patch info
 */
class StatusBar : public juce::Component,
                  public juce::Timer
{
public:
    StatusBar()
    {
        // Connection status label
        addAndMakeVisible(connectionLabel);
        connectionLabel.setText("MIDI: Not Connected", juce::dontSendNotification);
        connectionLabel.setFont(juce::FontOptions(12.0f));
        connectionLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        
        // MIDI In indicator
        addAndMakeVisible(midiInLabel);
        midiInLabel.setText("IN", juce::dontSendNotification);
        midiInLabel.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
        midiInLabel.setJustificationType(juce::Justification::centred);
        midiInLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        
        // MIDI Out indicator
        addAndMakeVisible(midiOutLabel);
        midiOutLabel.setText("OUT", juce::dontSendNotification);
        midiOutLabel.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
        midiOutLabel.setJustificationType(juce::Justification::centred);
        midiOutLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        
        // Patch name label
        addAndMakeVisible(patchLabel);
        patchLabel.setText("Patch: --", juce::dontSendNotification);
        patchLabel.setFont(juce::FontOptions(12.0f));
        patchLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        patchLabel.setJustificationType(juce::Justification::centredRight);
        
        // CPU usage label
        addAndMakeVisible(cpuLabel);
        cpuLabel.setText("CPU: 0%", juce::dontSendNotification);
        cpuLabel.setFont(juce::FontOptions(10.0f));
        cpuLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        cpuLabel.setJustificationType(juce::Justification::centredRight);
        
        startTimerHz(30); // 30Hz for smooth LED animation
    }
    
    ~StatusBar() override
    {
        stopTimer();
    }
    
    void paint(juce::Graphics& g) override
    {
        // Dark background
        g.fillAll(DS::Color::bg().darker(0.3f));
        
        // Top border line
        g.setColour(DS::Color::border());
        g.drawLine(0, 0, (float)getWidth(), 0, 1.0f);
        
        // MIDI activity indicators (animated LED circles)
        auto inBounds = midiInLabel.getBounds().translated(-18, 0);
        auto outBounds = midiOutLabel.getBounds().translated(-18, 0);
        
        float inX = (float)inBounds.getX();
        float inY = (float)inBounds.getCentreY() - 5.0f;
        float outX = (float)outBounds.getX();
        float outY = (float)outBounds.getCentreY() - 5.0f;
        float ledSize = 10.0f;
        
        // MIDI In LED with glow
        drawAnimatedLED(g, inX, inY, ledSize, midiInBrightness, DS::Color::Accent::cyan());
        
        // MIDI Out LED with glow  
        drawAnimatedLED(g, outX, outY, ledSize, midiOutBrightness, DS::Color::Accent::success());
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(8, 2);
        
        // Left side: Connection status
        connectionLabel.setBounds(bounds.removeFromLeft(200));
        
        bounds.removeFromLeft(20);
        
        // MIDI indicators
        bounds.removeFromLeft(12); // Space for circle
        midiInLabel.setBounds(bounds.removeFromLeft(30));
        bounds.removeFromLeft(12); // Space for circle
        midiOutLabel.setBounds(bounds.removeFromLeft(35));
        
        // Right side: CPU
        cpuLabel.setBounds(bounds.removeFromRight(60));
        bounds.removeFromRight(10);
        
        // Patch name takes remaining space
        patchLabel.setBounds(bounds);
    }
    
    // === Public API ===
    
    void setConnected(bool connected)
    {
        isConnected = connected;
        updateConnectionStatus();
    }
    
    void setMidiDeviceName(const juce::String& name)
    {
        midiDeviceName = name;
        updateConnectionStatus();
    }
    
    void setMidiChannel(int channel)
    {
        midiChannel = channel;
        updateConnectionStatus();
    }
    
    void flashMidiIn()
    {
        midiInBrightness = 1.0f;  // Full brightness on trigger
        repaint();
    }
    
    void flashMidiOut()
    {
        midiOutBrightness = 1.0f;
        repaint();
    }
    
    void setPatchName(const juce::String& name)
    {
        patchLabel.setText("Patch: " + name, juce::dontSendNotification);
    }
    
    void setCpuUsage(double percentage)
    {
        cpuLabel.setText("CPU: " + juce::String(percentage, 1) + "%", juce::dontSendNotification);
    }
    
    int getPreferredHeight() const { return 24; }

private:
    void timerCallback() override
    {
        bool needsRepaint = false;
        
        // Smooth fade out for MIDI In LED
        if (midiInBrightness > 0.0f)
        {
            midiInBrightness -= 0.08f;  // Smooth decay
            if (midiInBrightness < 0.0f) midiInBrightness = 0.0f;
            needsRepaint = true;
        }
        
        // Smooth fade out for MIDI Out LED
        if (midiOutBrightness > 0.0f)
        {
            midiOutBrightness -= 0.08f;
            if (midiOutBrightness < 0.0f) midiOutBrightness = 0.0f;
            needsRepaint = true;
        }
        
        if (needsRepaint) repaint();
    }
    
    void drawAnimatedLED(juce::Graphics& g, float x, float y, float size, float brightness, juce::Colour activeColor)
    {
        // Outer glow when active
        if (brightness > 0.1f)
        {
            float glowSize = size + 6.0f * brightness;
            float glowX = x - (glowSize - size) / 2;
            float glowY = y - (glowSize - size) / 2;
            
            // Multi-layer glow
            g.setColour(activeColor.withAlpha(0.15f * brightness));
            g.fillEllipse(glowX - 4, glowY - 4, glowSize + 8, glowSize + 8);
            
            g.setColour(activeColor.withAlpha(0.25f * brightness));
            g.fillEllipse(glowX - 2, glowY - 2, glowSize + 4, glowSize + 4);
            
            g.setColour(activeColor.withAlpha(0.4f * brightness));
            g.fillEllipse(glowX, glowY, glowSize, glowSize);
        }
        
        // LED base (dark when off)
        juce::Colour baseColor = DS::Color::bgInput();
        juce::Colour litColor = activeColor.brighter(0.4f);
        juce::Colour ledColor = baseColor.interpolatedWith(litColor, brightness);
        
        // LED body with gradient
        juce::ColourGradient ledGrad(
            ledColor.brighter(0.2f), x + size/2, y,
            ledColor.darker(0.2f), x + size/2, y + size, false);
        g.setGradientFill(ledGrad);
        g.fillEllipse(x, y, size, size);
        
        // Highlight reflection
        g.setColour(juce::Colours::white.withAlpha(0.15f + 0.2f * brightness));
        g.fillEllipse(x + size * 0.2f, y + size * 0.15f, size * 0.3f, size * 0.25f);
        
        // Border
        g.setColour(DS::Color::border().darker(0.2f));
        g.drawEllipse(x, y, size, size, 1.0f);
    }
    
    void updateConnectionStatus()
    {
        juce::String status;
        juce::Colour color;
        
        if (isConnected && midiDeviceName.isNotEmpty())
        {
            status = "MIDI: " + midiDeviceName;
            if (midiChannel > 0)
                status += " (Ch " + juce::String(midiChannel) + ")";
            color = DS::Color::Accent::success();
        }
        else if (midiDeviceName.isNotEmpty())
        {
            status = "MIDI: " + midiDeviceName + " (Offline)";
            color = DS::Color::Accent::warning();
        }
        else
        {
            status = "MIDI: Not Connected";
            color = DS::Color::textMuted();
        }
        
        connectionLabel.setText(status, juce::dontSendNotification);
        connectionLabel.setColour(juce::Label::textColourId, color);
    }
    
    // State
    bool isConnected = false;
    juce::String midiDeviceName;
    int midiChannel = 0;
    
    // Activity indicators - smooth brightness for animation
    float midiInBrightness = 0.0f;
    float midiOutBrightness = 0.0f;
    
    // Labels
    juce::Label connectionLabel;
    juce::Label midiInLabel;
    juce::Label midiOutLabel;
    juce::Label patchLabel;
    juce::Label cpuLabel;
};
