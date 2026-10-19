#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * PerformanceMode.h
 * Performance macros, big knobs, and live-friendly interface
 * 
 * Part of Improvement #10: Performance Mode
 */

namespace Performance
{

//==============================================================================
/**
 * Macro definition - one knob controlling multiple parameters
 */
struct MacroTarget
{
    juce::String parameterId;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float curve = 1.0f;  // 1.0 = linear, <1 = log, >1 = exp
    bool inverted = false;
};

struct Macro
{
    juce::String name = "Macro";
    juce::Colour color = DS::Color::accent;
    float value = 0.0f;
    std::vector<MacroTarget> targets;
    
    void addTarget(const juce::String& paramId, float min = 0.0f, float max = 1.0f)
    {
        targets.push_back({paramId, min, max, 1.0f, false});
    }
    
    void clearTargets()
    {
        targets.clear();
    }
};

//==============================================================================
/**
 * Macro Knob - Big performance knob
 */
class MacroKnob : public juce::Component
{
public:
    MacroKnob(const juce::String& name = "MACRO")
    {
        macroName = name;
        setRepaintsOnMouseActivity(true);
    }
    
    void setColor(juce::Colour c) { knobColor = c; repaint(); }
    void setName(const juce::String& name) { macroName = name; repaint(); }
    void setValue(float v) { value = juce::jlimit(0.0f, 1.0f, v); repaint(); }
    float getValue() const { return value; }
    
    std::function<void(float)> onValueChanged;
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto knobBounds = bounds.reduced(10);
        
        float size = juce::jmin(knobBounds.getWidth(), knobBounds.getHeight());
        auto knobRect = knobBounds.withSizeKeepingCentre(size, size);
        
        // Outer ring
        g.setColour(DS::Color::surfaceAlt);
        g.fillEllipse(knobRect);
        
        // Value arc
        juce::Path arcPath;
        float startAngle = juce::MathConstants<float>::pi * 1.25f;
        float endAngle = startAngle + (value * juce::MathConstants<float>::pi * 1.5f);
        
        arcPath.addCentredArc(knobRect.getCentreX(), knobRect.getCentreY(),
                              size * 0.45f, size * 0.45f,
                              0, startAngle, endAngle, true);
        
        g.setColour(knobColor);
        g.strokePath(arcPath, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, 
                                                    juce::PathStrokeType::rounded));
        
        // Inner circle
        float innerSize = size * 0.6f;
        auto innerRect = knobRect.withSizeKeepingCentre(innerSize, innerSize);
        
        juce::ColourGradient grad(DS::Color::surface, innerRect.getCentreX(), innerRect.getY(),
                                   DS::Color::surfaceAlt, innerRect.getCentreX(), innerRect.getBottom(), false);
        g.setGradientFill(grad);
        g.fillEllipse(innerRect);
        
        // Indicator line
        float angle = startAngle + (value * juce::MathConstants<float>::pi * 1.5f);
        float indicatorLength = innerSize * 0.35f;
        float cx = knobRect.getCentreX();
        float cy = knobRect.getCentreY();
        
        g.setColour(knobColor);
        g.drawLine(cx, cy,
                   cx + std::sin(angle) * indicatorLength,
                   cy - std::cos(angle) * indicatorLength, 3.0f);
        
        // Value text
        g.setFont(DS::Font::heading.withHeight(18.0f));
        g.setColour(DS::Color::text);
        g.drawText(juce::String(static_cast<int>(value * 100)) + "%",
                   innerRect, juce::Justification::centred);
        
        // Name
        g.setFont(DS::Font::label);
        g.setColour(DS::Color::textMuted);
        g.drawText(macroName, bounds.removeFromBottom(18), juce::Justification::centredTop);
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        lastY = e.position.y;
        isDragging = true;
    }
    
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (isDragging)
        {
            float delta = (lastY - e.position.y) / 200.0f;
            setValue(value + delta);
            lastY = e.position.y;
            
            if (onValueChanged)
                onValueChanged(value);
        }
    }
    
    void mouseUp(const juce::MouseEvent&) override
    {
        isDragging = false;
    }
    
    void mouseDoubleClick(const juce::MouseEvent&) override
    {
        setValue(0.5f);
        if (onValueChanged)
            onValueChanged(value);
    }
    
private:
    juce::String macroName;
    juce::Colour knobColor = DS::Color::accent;
    float value = 0.0f;
    float lastY = 0.0f;
    bool isDragging = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroKnob)
};

//==============================================================================
/**
 * Performance Panel - 4 Macro knobs + XY Pad
 */
class PerformancePanel : public juce::Component
{
public:
    PerformancePanel()
    {
        for (int i = 0; i < 4; i++)
        {
            auto knob = std::make_unique<MacroKnob>("MACRO " + juce::String(i + 1));
            knob->setColor(getMacroColor(i));
            knob->onValueChanged = [this, i](float v)
            {
                macros[i].value = v;
                if (onMacroChanged)
                    onMacroChanged(i, v);
            };
            addAndMakeVisible(*knob);
            macroKnobs.push_back(std::move(knob));
        }
    }
    
    Macro& getMacro(int index) { return macros[index]; }
    
    std::function<void(int, float)> onMacroChanged;
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        int knobSize = 120;
        int spacing = DS::Space::md;
        
        // Center the knobs
        int totalWidth = (knobSize * 4) + (spacing * 3);
        bounds = bounds.withSizeKeepingCentre(totalWidth, knobSize + 20);
        
        for (int i = 0; i < 4; i++)
        {
            macroKnobs[i]->setBounds(bounds.removeFromLeft(knobSize));
            if (i < 3)
                bounds.removeFromLeft(spacing);
        }
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::large);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::large, 1.0f);
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading);
        g.drawText("PERFORMANCE", bounds.reduced(DS::Space::md).removeFromTop(24),
                   juce::Justification::centredLeft);
    }
    
private:
    juce::Colour getMacroColor(int index)
    {
        static const juce::Colour colors[] = {
            juce::Colour(0xFF6366f1), // Purple
            juce::Colour(0xFFec4899), // Pink
            juce::Colour(0xFF10b981), // Green
            juce::Colour(0xFFf59e0b)  // Orange
        };
        return colors[index % 4];
    }
    
    std::array<Macro, 4> macros;
    std::vector<std::unique_ptr<MacroKnob>> macroKnobs;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformancePanel)
};

//==============================================================================
/**
 * XY Pad - 2D control surface
 */
class XYPad : public juce::Component
{
public:
    XYPad()
    {
        setRepaintsOnMouseActivity(true);
    }
    
    void setX(float v) { xValue = juce::jlimit(0.0f, 1.0f, v); repaint(); }
    void setY(float v) { yValue = juce::jlimit(0.0f, 1.0f, v); repaint(); }
    float getX() const { return xValue; }
    float getY() const { return yValue; }
    
    void setLabels(const juce::String& xLabel, const juce::String& yLabel)
    {
        xAxisLabel = xLabel;
        yAxisLabel = yLabel;
        repaint();
    }
    
    std::function<void(float, float)> onValueChanged;
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        // Grid
        g.setColour(DS::Color::border.withAlpha(0.3f));
        for (int i = 1; i < 4; i++)
        {
            float x = bounds.getX() + (i / 4.0f) * bounds.getWidth();
            float y = bounds.getY() + (i / 4.0f) * bounds.getHeight();
            g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
            g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
        }
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        // Position indicator
        float px = bounds.getX() + xValue * bounds.getWidth();
        float py = bounds.getBottom() - yValue * bounds.getHeight();
        
        // Crosshairs
        g.setColour(DS::Color::accent.withAlpha(0.5f));
        g.drawHorizontalLine(static_cast<int>(py), bounds.getX(), bounds.getRight());
        g.drawVerticalLine(static_cast<int>(px), bounds.getY(), bounds.getBottom());
        
        // Glow
        g.setColour(DS::Color::accent.withAlpha(0.3f));
        g.fillEllipse(px - 15, py - 15, 30, 30);
        
        // Point
        g.setColour(DS::Color::accent);
        g.fillEllipse(px - 8, py - 8, 16, 16);
        g.setColour(juce::Colours::white);
        g.fillEllipse(px - 4, py - 4, 8, 8);
        
        // Labels
        g.setFont(DS::Font::label);
        g.setColour(DS::Color::textMuted);
        g.drawText(xAxisLabel, bounds.removeFromBottom(14), juce::Justification::centred);
        
        // Y label (rotated would be ideal, but just put at top)
        g.drawText(yAxisLabel, bounds.removeFromTop(14), juce::Justification::centred);
    }
    
    void mouseDown(const juce::MouseEvent& e) override { updateFromMouse(e); }
    void mouseDrag(const juce::MouseEvent& e) override { updateFromMouse(e); }
    
private:
    void updateFromMouse(const juce::MouseEvent& e)
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        xValue = juce::jlimit(0.0f, 1.0f, (e.position.x - bounds.getX()) / bounds.getWidth());
        yValue = juce::jlimit(0.0f, 1.0f, 1.0f - (e.position.y - bounds.getY()) / bounds.getHeight());
        repaint();
        
        if (onValueChanged)
            onValueChanged(xValue, yValue);
    }
    
    float xValue = 0.5f;
    float yValue = 0.5f;
    juce::String xAxisLabel = "X";
    juce::String yAxisLabel = "Y";
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYPad)
};

} // namespace Performance
