/*
  ==============================================================================

    ParameterRangeIndicator.h
    Visual indicators showing parameter min/max/default values

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
    RangeIndicatorSlider - Slider with visual range markers
*/
class RangeIndicatorSlider : public juce::Slider
{
public:
    RangeIndicatorSlider()
    {
        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    }
    
    void setDefaultValue(double value)
    {
        defaultValue = value;
        hasDefault = true;
        repaint();
    }
    
    void setSweetSpot(double low, double high)
    {
        sweetSpotLow = low;
        sweetSpotHigh = high;
        hasSweetSpot = true;
        repaint();
    }
    
    void setWarningZone(double low, double high)
    {
        warningLow = low;
        warningHigh = high;
        hasWarningZone = true;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        // Get knob area
        auto bounds = getLocalBounds().toFloat();
        auto knobArea = bounds.removeFromTop(bounds.getWidth());
        
        float rotaryStartAngle = juce::MathConstants<float>::pi * 1.2f;
        float rotaryEndAngle = juce::MathConstants<float>::pi * 2.8f;
        
        // Draw range indicators behind the knob
        drawRangeIndicators(g, knobArea, rotaryStartAngle, rotaryEndAngle);
        
        // Draw the actual slider
        juce::Slider::paint(g);
        
        // Draw default marker on top
        if (hasDefault)
        {
            drawDefaultMarker(g, knobArea, rotaryStartAngle, rotaryEndAngle);
        }
    }
    
    void setShowRangeLabels(bool show) { showRangeLabels = show; repaint(); }
    void setShowDefaultMarker(bool show) { showDefaultMarker = show; repaint(); }
    
private:
    double defaultValue = 64;
    bool hasDefault = false;
    
    double sweetSpotLow = 0;
    double sweetSpotHigh = 127;
    bool hasSweetSpot = false;
    
    double warningLow = 0;
    double warningHigh = 127;
    bool hasWarningZone = false;
    
    bool showRangeLabels = true;
    bool showDefaultMarker = true;
    
    void drawRangeIndicators(juce::Graphics& g, juce::Rectangle<float> knobArea,
                            float startAngle, float endAngle)
    {
        auto centre = knobArea.getCentre();
        float outerRadius = knobArea.getWidth() / 2 + 6;
        float innerRadius = knobArea.getWidth() / 2 + 2;
        
        // Draw sweet spot arc
        if (hasSweetSpot)
        {
            float sweetStartAngle = valueToAngle(sweetSpotLow, startAngle, endAngle);
            float sweetEndAngle = valueToAngle(sweetSpotHigh, startAngle, endAngle);
            
            g.setColour(DS::Colors::success.withAlpha(0.3f));
            
            juce::Path sweetArc;
            sweetArc.addCentredArc(centre.x, centre.y, outerRadius, outerRadius,
                                  0, sweetStartAngle, sweetEndAngle, true);
            sweetArc.addCentredArc(centre.x, centre.y, innerRadius, innerRadius,
                                  0, sweetEndAngle, sweetStartAngle, false);
            sweetArc.closeSubPath();
            g.fillPath(sweetArc);
        }
        
        // Draw warning zone arc
        if (hasWarningZone)
        {
            float warnStartAngle = valueToAngle(warningLow, startAngle, endAngle);
            float warnEndAngle = valueToAngle(warningHigh, startAngle, endAngle);
            
            g.setColour(DS::Colors::warning.withAlpha(0.25f));
            
            juce::Path warnArc;
            warnArc.addCentredArc(centre.x, centre.y, outerRadius, outerRadius,
                                 0, warnStartAngle, warnEndAngle, true);
            warnArc.addCentredArc(centre.x, centre.y, innerRadius, innerRadius,
                                 0, warnEndAngle, warnStartAngle, false);
            warnArc.closeSubPath();
            g.fillPath(warnArc);
        }
        
        // Draw range labels
        if (showRangeLabels)
        {
            g.setColour(DS::Colors::textMuted.withAlpha(0.6f));
            g.setFont(9.0f);
            
            // Min label
            float minAngle = startAngle;
            float minX = centre.x + std::cos(minAngle - juce::MathConstants<float>::halfPi) * (outerRadius + 8);
            float minY = centre.y + std::sin(minAngle - juce::MathConstants<float>::halfPi) * (outerRadius + 8);
            g.drawText(juce::String((int)getMinimum()), (int)minX - 10, (int)minY - 6, 20, 12,
                      juce::Justification::centred);
            
            // Max label
            float maxAngle = endAngle;
            float maxX = centre.x + std::cos(maxAngle - juce::MathConstants<float>::halfPi) * (outerRadius + 8);
            float maxY = centre.y + std::sin(maxAngle - juce::MathConstants<float>::halfPi) * (outerRadius + 8);
            g.drawText(juce::String((int)getMaximum()), (int)maxX - 10, (int)maxY - 6, 20, 12,
                      juce::Justification::centred);
        }
    }
    
    void drawDefaultMarker(juce::Graphics& g, juce::Rectangle<float> knobArea,
                          float startAngle, float endAngle)
    {
        if (!showDefaultMarker) return;
        
        auto centre = knobArea.getCentre();
        float radius = knobArea.getWidth() / 2 - 5;
        
        float defAngle = valueToAngle(defaultValue, startAngle, endAngle);
        
        // Draw a small tick mark at the default position
        float innerR = radius - 3;
        float outerR = radius + 3;
        
        float x1 = centre.x + std::cos(defAngle - juce::MathConstants<float>::halfPi) * innerR;
        float y1 = centre.y + std::sin(defAngle - juce::MathConstants<float>::halfPi) * innerR;
        float x2 = centre.x + std::cos(defAngle - juce::MathConstants<float>::halfPi) * outerR;
        float y2 = centre.y + std::sin(defAngle - juce::MathConstants<float>::halfPi) * outerR;
        
        g.setColour(DS::Colors::accent.withAlpha(0.6f));
        g.drawLine(x1, y1, x2, y2, 2.0f);
        
        // Small dot at the marker
        g.fillEllipse(x2 - 2, y2 - 2, 4, 4);
    }
    
    float valueToAngle(double value, float startAngle, float endAngle)
    {
        double normalized = (value - getMinimum()) / (getMaximum() - getMinimum());
        return startAngle + (float)normalized * (endAngle - startAngle);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangeIndicatorSlider)
};

//==============================================================================
/**
    ParameterTooltip - Floating tooltip showing parameter details
*/
class ParameterTooltip : public juce::Component
{
public:
    ParameterTooltip()
    {
        setSize(150, 80);
        setAlwaysOnTop(true);
    }
    
    void setParameterInfo(const juce::String& name, double current, double min, double max,
                         double defaultVal, const juce::String& unit = "")
    {
        paramName = name;
        currentValue = current;
        minValue = min;
        maxValue = max;
        this->defaultVal = defaultVal;
        this->unit = unit;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background with shadow
        juce::DropShadow shadow(juce::Colours::black.withAlpha(0.4f), 8, {0, 2});
        shadow.drawForRectangle(g, bounds.toNearestInt());
        
        g.setColour(juce::Colour(0xff252525));
        g.fillRoundedRectangle(bounds, 6.0f);
        
        g.setColour(DS::Colors::border);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
        
        // Content
        auto content = bounds.reduced(8);
        
        // Parameter name
        g.setColour(DS::Colors::text);
        g.setFont(juce::Font(12.0f).boldened());
        g.drawText(paramName, content.removeFromTop(16), juce::Justification::centredLeft);
        
        content.removeFromTop(4);
        
        // Current value with progress bar
        auto valueArea = content.removeFromTop(20);
        
        // Progress bar background
        auto barArea = valueArea.removeFromLeft(100);
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(barArea, 3.0f);
        
        // Progress fill
        float progress = (float)((currentValue - minValue) / (maxValue - minValue));
        auto fillArea = barArea;
        fillArea.setWidth(barArea.getWidth() * progress);
        g.setColour(DS::Colors::accent);
        g.fillRoundedRectangle(fillArea, 3.0f);
        
        // Value text
        g.setColour(DS::Colors::text);
        g.setFont(12.0f);
        juce::String valueStr = juce::String(currentValue, 0) + (unit.isNotEmpty() ? " " + unit : "");
        g.drawText(valueStr, valueArea.reduced(5, 0), juce::Justification::centredLeft);
        
        content.removeFromTop(4);
        
        // Range info
        g.setColour(DS::Colors::textMuted);
        g.setFont(10.0f);
        juce::String rangeStr = "Range: " + juce::String(minValue, 0) + " - " + juce::String(maxValue, 0);
        g.drawText(rangeStr, content.removeFromTop(12), juce::Justification::centredLeft);
        
        // Default value
        juce::String defStr = "Default: " + juce::String(defaultVal, 0);
        g.drawText(defStr, content.removeFromTop(12), juce::Justification::centredLeft);
    }
    
private:
    juce::String paramName;
    double currentValue = 0;
    double minValue = 0;
    double maxValue = 127;
    double defaultVal = 64;
    juce::String unit;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterTooltip)
};

//==============================================================================
/**
    RangeBarComponent - Horizontal bar showing value in range
*/
class RangeBarComponent : public juce::Component
{
public:
    RangeBarComponent()
    {
        setSize(100, 20);
    }
    
    void setRange(double min, double max, double defaultVal = -1)
    {
        minValue = min;
        maxValue = max;
        if (defaultVal >= min && defaultVal <= max)
        {
            this->defaultVal = defaultVal;
            hasDefault = true;
        }
        repaint();
    }
    
    void setValue(double val)
    {
        currentValue = val;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background track
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Value fill
        float progress = (float)((currentValue - minValue) / (maxValue - minValue));
        progress = juce::jlimit(0.0f, 1.0f, progress);
        
        auto fillBounds = bounds;
        fillBounds.setWidth(bounds.getWidth() * progress);
        
        // Color based on position
        juce::Colour fillColour = DS::Colors::accent;
        if (progress > 0.9f)
            fillColour = DS::Colors::warning;
        if (progress < 0.1f)
            fillColour = DS::Colors::textMuted;
        
        g.setColour(fillColour);
        g.fillRoundedRectangle(fillBounds, 4.0f);
        
        // Default marker
        if (hasDefault)
        {
            float defPos = (float)((defaultVal - minValue) / (maxValue - minValue));
            float markerX = bounds.getX() + bounds.getWidth() * defPos;
            
            g.setColour(DS::Colors::text.withAlpha(0.7f));
            g.drawLine(markerX, bounds.getY(), markerX, bounds.getBottom(), 1.5f);
        }
        
        // Value text
        g.setColour(DS::Colors::text);
        g.setFont(10.0f);
        g.drawText(juce::String(currentValue, 0), bounds, juce::Justification::centred);
    }
    
private:
    double currentValue = 64;
    double minValue = 0;
    double maxValue = 127;
    double defaultVal = 64;
    bool hasDefault = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangeBarComponent)
};
