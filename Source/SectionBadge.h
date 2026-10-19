/*
  ==============================================================================

    SectionBadge.h
    Color-coded badges for parameter sections with activity indicators

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
    SectionBadge - A colored badge that shows section info and activity
*/
class SectionBadge : public juce::Component, private juce::Timer
{
public:
    enum class SectionType
    {
        DCO1,
        DCO2,
        Mixer,
        VCF,
        VCA,
        LFO1,
        LFO2,
        ENV1,
        ENV2,
        ENV3,
        ENV4,
        Chorus,
        Patch,
        Custom
    };
    
    SectionBadge(SectionType type = SectionType::Custom, const juce::String& customLabel = "")
        : sectionType(type), label(customLabel)
    {
        setSize(70, 24);
        updateColorFromType();
        
        if (label.isEmpty())
            label = getSectionName(type);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1);
        
        // Background with section color
        auto bgColour = sectionColour.withAlpha(0.2f + activityLevel * 0.3f);
        g.setColour(bgColour);
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Border
        g.setColour(sectionColour.withAlpha(0.5f + activityLevel * 0.5f));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        
        // Activity pulse effect
        if (activityLevel > 0.01f)
        {
            float pulseSize = activityLevel * 3.0f;
            g.setColour(sectionColour.withAlpha(activityLevel * 0.3f));
            g.drawRoundedRectangle(bounds.expanded(pulseSize), 4.0f + pulseSize, 2.0f);
        }
        
        // Icon
        auto iconArea = bounds.removeFromLeft(22).reduced(3);
        drawSectionIcon(g, iconArea, sectionColour.withAlpha(0.8f + activityLevel * 0.2f));
        
        // Label text
        g.setColour(sectionColour.interpolatedWith(DS::Colors::text, 0.4f));
        g.setFont(11.0f);
        g.drawText(label, bounds.reduced(2, 0), juce::Justification::centredLeft);
        
        // Activity indicator dot
        if (showActivityDot && activityLevel > 0.1f)
        {
            g.setColour(sectionColour);
            float dotSize = 6.0f + activityLevel * 2.0f;
            g.fillEllipse(bounds.getRight() - dotSize - 2, bounds.getCentreY() - dotSize/2, dotSize, dotSize);
        }
    }
    
    // Trigger activity flash
    void triggerActivity()
    {
        activityLevel = 1.0f;
        startTimerHz(30);
    }
    
    // Set continuous activity level (0-1)
    void setActivityLevel(float level)
    {
        activityLevel = juce::jlimit(0.0f, 1.0f, level);
        repaint();
    }
    
    void setCustomColour(juce::Colour colour)
    {
        sectionColour = colour;
        repaint();
    }
    
    void setLabel(const juce::String& newLabel)
    {
        label = newLabel;
        repaint();
    }
    
    void setShowActivityDot(bool show)
    {
        showActivityDot = show;
        repaint();
    }
    
private:
    SectionType sectionType;
    juce::String label;
    juce::Colour sectionColour = DS::Colors::accent;
    float activityLevel = 0.0f;
    bool showActivityDot = true;
    
    void timerCallback() override
    {
        activityLevel *= 0.9f;  // Decay
        
        if (activityLevel < 0.01f)
        {
            activityLevel = 0;
            stopTimer();
        }
        
        repaint();
    }
    
    void updateColorFromType()
    {
        switch (sectionType)
        {
            case SectionType::DCO1:
            case SectionType::DCO2:
                sectionColour = DS::Colors::sectionDCO;
                break;
            case SectionType::Mixer:
                sectionColour = DS::Colors::sectionMixer;
                break;
            case SectionType::VCF:
                sectionColour = DS::Colors::sectionVCF;
                break;
            case SectionType::VCA:
                sectionColour = DS::Colors::sectionVCA;
                break;
            case SectionType::LFO1:
            case SectionType::LFO2:
                sectionColour = DS::Colors::sectionLFO;
                break;
            case SectionType::ENV1:
            case SectionType::ENV2:
            case SectionType::ENV3:
            case SectionType::ENV4:
                sectionColour = DS::Colors::sectionENV;
                break;
            case SectionType::Chorus:
                sectionColour = juce::Colour(0xff9966ff);
                break;
            case SectionType::Patch:
                sectionColour = DS::Colors::accent;
                break;
            default:
                sectionColour = DS::Colors::textMuted;
                break;
        }
    }
    
    juce::String getSectionName(SectionType type)
    {
        switch (type)
        {
            case SectionType::DCO1:  return "DCO-1";
            case SectionType::DCO2:  return "DCO-2";
            case SectionType::Mixer: return "MIXER";
            case SectionType::VCF:   return "VCF";
            case SectionType::VCA:   return "VCA";
            case SectionType::LFO1:  return "LFO-1";
            case SectionType::LFO2:  return "LFO-2";
            case SectionType::ENV1:  return "ENV-1";
            case SectionType::ENV2:  return "ENV-2";
            case SectionType::ENV3:  return "ENV-3";
            case SectionType::ENV4:  return "ENV-4";
            case SectionType::Chorus: return "CHORUS";
            case SectionType::Patch: return "PATCH";
            default: return "CUSTOM";
        }
    }
    
    void drawSectionIcon(juce::Graphics& g, juce::Rectangle<float> area, juce::Colour colour)
    {
        g.setColour(colour);
        float cx = area.getCentreX();
        float cy = area.getCentreY();
        float r = juce::jmin(area.getWidth(), area.getHeight()) / 2 - 1;
        
        switch (sectionType)
        {
            case SectionType::DCO1:
            case SectionType::DCO2:
                // Waveform icon
                {
                    juce::Path wave;
                    wave.startNewSubPath(area.getX(), cy);
                    wave.lineTo(area.getX() + r * 0.5f, area.getY() + 2);
                    wave.lineTo(area.getX() + r, cy);
                    wave.lineTo(area.getX() + r * 1.5f, area.getBottom() - 2);
                    wave.lineTo(area.getRight(), cy);
                    g.strokePath(wave, juce::PathStrokeType(1.5f));
                }
                break;
                
            case SectionType::VCF:
                // Filter curve icon
                {
                    juce::Path filter;
                    filter.startNewSubPath(area.getX(), cy + r * 0.5f);
                    filter.quadraticTo(cx, cy - r * 0.8f, area.getRight(), cy + r * 0.5f);
                    g.strokePath(filter, juce::PathStrokeType(1.5f));
                }
                break;
                
            case SectionType::VCA:
                // Amp icon (speaker-like)
                g.drawLine(area.getX(), cy - r * 0.5f, area.getX(), cy + r * 0.5f, 1.5f);
                g.drawLine(area.getX(), cy - r * 0.5f, cx, cy - r, 1.5f);
                g.drawLine(area.getX(), cy + r * 0.5f, cx, cy + r, 1.5f);
                g.drawArc(cx - r * 0.3f, cy - r * 0.5f, r * 0.8f, r, 
                         -juce::MathConstants<float>::halfPi * 0.8f,
                         juce::MathConstants<float>::halfPi * 0.8f, true);
                break;
                
            case SectionType::LFO1:
            case SectionType::LFO2:
                // Sine wave icon
                {
                    juce::Path sine;
                    sine.startNewSubPath(area.getX(), cy);
                    for (float x = 0; x <= 1; x += 0.1f)
                    {
                        float y = std::sin(x * juce::MathConstants<float>::twoPi) * r * 0.7f;
                        sine.lineTo(area.getX() + x * area.getWidth(), cy - y);
                    }
                    g.strokePath(sine, juce::PathStrokeType(1.5f));
                }
                break;
                
            case SectionType::ENV1:
            case SectionType::ENV2:
            case SectionType::ENV3:
            case SectionType::ENV4:
                // ADSR shape icon
                {
                    juce::Path adsr;
                    adsr.startNewSubPath(area.getX(), area.getBottom() - 2);
                    adsr.lineTo(area.getX() + r * 0.5f, area.getY() + 2);
                    adsr.lineTo(area.getX() + r, cy);
                    adsr.lineTo(area.getX() + r * 1.5f, cy);
                    adsr.lineTo(area.getRight(), area.getBottom() - 2);
                    g.strokePath(adsr, juce::PathStrokeType(1.5f));
                }
                break;
                
            case SectionType::Mixer:
                // Faders icon
                g.drawLine(area.getX() + 2, area.getY() + 3, area.getX() + 2, area.getBottom() - 3, 1.5f);
                g.fillRect(area.getX(), cy - 2, 4.0f, 4.0f);
                g.drawLine(area.getRight() - 2, area.getY() + 3, area.getRight() - 2, area.getBottom() - 3, 1.5f);
                g.fillRect(area.getRight() - 4, cy + 2, 4.0f, 4.0f);
                break;
                
            case SectionType::Chorus:
                // Swirl/chorus icon
                g.drawEllipse(area.reduced(2), 1.5f);
                g.drawArc(cx - r * 0.4f, cy - r * 0.4f, r * 0.8f, r * 0.8f,
                         0, juce::MathConstants<float>::pi, true);
                break;
                
            default:
                // Generic dot
                g.fillEllipse(area.reduced(4));
                break;
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionBadge)
};

//==============================================================================
/**
    SectionActivityMonitor - Tracks parameter changes per section
*/
class SectionActivityMonitor
{
public:
    static SectionActivityMonitor& getInstance()
    {
        static SectionActivityMonitor instance;
        return instance;
    }
    
    void registerBadge(SectionBadge::SectionType type, SectionBadge* badge)
    {
        badges[type] = badge;
    }
    
    void unregisterBadge(SectionBadge::SectionType type)
    {
        badges.erase(type);
    }
    
    void triggerActivity(SectionBadge::SectionType type)
    {
        auto it = badges.find(type);
        if (it != badges.end() && it->second != nullptr)
        {
            it->second->triggerActivity();
        }
    }
    
    // Trigger activity based on parameter ID
    void triggerActivityForParam(const juce::String& paramId)
    {
        SectionBadge::SectionType type = getSectionForParam(paramId);
        triggerActivity(type);
    }
    
private:
    SectionActivityMonitor() = default;
    
    std::map<SectionBadge::SectionType, SectionBadge*> badges;
    
    SectionBadge::SectionType getSectionForParam(const juce::String& paramId)
    {
        if (paramId.startsWithIgnoreCase("dco1"))
            return SectionBadge::SectionType::DCO1;
        if (paramId.startsWithIgnoreCase("dco2"))
            return SectionBadge::SectionType::DCO2;
        if (paramId.startsWithIgnoreCase("mixer"))
            return SectionBadge::SectionType::Mixer;
        if (paramId.startsWithIgnoreCase("vcf"))
            return SectionBadge::SectionType::VCF;
        if (paramId.startsWithIgnoreCase("vca"))
            return SectionBadge::SectionType::VCA;
        if (paramId.startsWithIgnoreCase("lfo1"))
            return SectionBadge::SectionType::LFO1;
        if (paramId.startsWithIgnoreCase("lfo2"))
            return SectionBadge::SectionType::LFO2;
        if (paramId.startsWithIgnoreCase("env1"))
            return SectionBadge::SectionType::ENV1;
        if (paramId.startsWithIgnoreCase("env2"))
            return SectionBadge::SectionType::ENV2;
        if (paramId.startsWithIgnoreCase("env3"))
            return SectionBadge::SectionType::ENV3;
        if (paramId.startsWithIgnoreCase("env4"))
            return SectionBadge::SectionType::ENV4;
        if (paramId.startsWithIgnoreCase("chorus"))
            return SectionBadge::SectionType::Chorus;
            
        return SectionBadge::SectionType::Custom;
    }
};
