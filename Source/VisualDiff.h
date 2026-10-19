/*
  ==============================================================================

    VisualDiff.h
    Enhanced visual diff component for Compare mode

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
    DiffBarComponent - Visual bar showing parameter difference
*/
class DiffBarComponent : public juce::Component
{
public:
    DiffBarComponent() {}
    
    void setValues(int valueA, int valueB, int minVal = 0, int maxVal = 127)
    {
        this->valueA = valueA;
        this->valueB = valueB;
        this->minValue = minVal;
        this->maxValue = maxVal;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        float range = (float)(maxValue - minValue);
        if (range <= 0) range = 1;
        
        // Background track
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRoundedRectangle(bounds, 3.0f);
        
        // Calculate positions
        float normA = (float)(valueA - minValue) / range;
        float normB = (float)(valueB - minValue) / range;
        
        float posA = bounds.getX() + normA * bounds.getWidth();
        float posB = bounds.getX() + normB * bounds.getWidth();
        
        // Draw difference bar between values
        if (valueA != valueB)
        {
            float left = juce::jmin(posA, posB);
            float right = juce::jmax(posA, posB);
            
            // Gradient from red to green based on direction
            juce::Colour diffColour = (valueB > valueA) 
                ? DS::Colors::success.withAlpha(0.4f)
                : DS::Colors::error.withAlpha(0.4f);
            
            g.setColour(diffColour);
            g.fillRoundedRectangle(left, bounds.getY(), right - left, bounds.getHeight(), 2.0f);
        }
        
        // Draw value A marker (original)
        g.setColour(DS::Colors::text.withAlpha(0.7f));
        g.drawLine(posA, bounds.getY(), posA, bounds.getBottom(), 2.0f);
        
        // Draw value B marker (current/comparison)
        g.setColour(DS::Colors::accent);
        g.fillEllipse(posB - 4, bounds.getCentreY() - 4, 8, 8);
    }
    
private:
    int valueA = 0;
    int valueB = 0;
    int minValue = 0;
    int maxValue = 127;
};

//==============================================================================
/**
    DiffRowComponent - Single row in the diff display
*/
class DiffRowComponent : public juce::Component
{
public:
    DiffRowComponent()
    {
        addAndMakeVisible(diffBar);
    }
    
    void setData(const juce::String& param, const juce::String& section,
                 int valA, int valB, int minV = 0, int maxV = 127)
    {
        paramName = param;
        sectionName = section;
        valueA = valA;
        valueB = valB;
        diffBar.setValues(valA, valB, minV, maxV);
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Alternating row background
        bool isEven = (rowIndex % 2 == 0);
        g.setColour(isEven ? juce::Colour(0xff252525) : juce::Colour(0xff2a2a2a));
        g.fillRect(bounds);
        
        // Highlight if different
        bool isDifferent = (valueA != valueB);
        if (isDifferent)
        {
            g.setColour(DS::Colors::warning.withAlpha(0.1f));
            g.fillRect(bounds);
            
            // Side indicator
            g.setColour(DS::Colors::warning);
            g.fillRect(bounds.removeFromLeft(3));
        }
        
        // Section tag
        auto tagArea = bounds.removeFromLeft(60);
        g.setColour(getSectionColour(sectionName).withAlpha(0.8f));
        g.setFont(10.0f);
        g.drawText(sectionName, tagArea.reduced(4, 2), juce::Justification::centredLeft);
        
        // Parameter name
        auto nameArea = bounds.removeFromLeft(120);
        g.setColour(isDifferent ? DS::Colors::text : DS::Colors::textMuted);
        g.setFont(12.0f);
        g.drawText(paramName, nameArea.reduced(4, 0), juce::Justification::centredLeft);
        
        // Value A
        auto valAArea = bounds.removeFromLeft(50);
        g.setColour(DS::Colors::textMuted);
        g.drawText(juce::String(valueA), valAArea, juce::Justification::centred);
        
        // Arrow
        auto arrowArea = bounds.removeFromLeft(30);
        if (isDifferent)
        {
            g.setColour(valueB > valueA ? DS::Colors::success : DS::Colors::error);
            g.drawText(valueB > valueA ? "→" : "←", arrowArea, juce::Justification::centred);
        }
        
        // Value B  
        auto valBArea = bounds.removeFromLeft(50);
        g.setColour(isDifferent ? DS::Colors::accent : DS::Colors::textMuted);
        g.setFont(juce::Font(12.0f).boldened());
        g.drawText(juce::String(valueB), valBArea, juce::Justification::centred);
        
        // Difference badge
        auto diffArea = bounds.removeFromLeft(50);
        if (isDifferent)
        {
            int diff = valueB - valueA;
            juce::String diffText = (diff > 0 ? "+" : "") + juce::String(diff);
            
            g.setColour(diff > 0 ? DS::Colors::success : DS::Colors::error);
            g.fillRoundedRectangle(diffArea.reduced(5, 4).toFloat(), 3.0f);
            
            g.setColour(DS::Colors::background);
            g.setFont(10.0f);
            g.drawText(diffText, diffArea, juce::Justification::centred);
        }
    }
    
    void resized() override
    {
        // Diff bar gets the remaining space
        auto bounds = getLocalBounds();
        bounds.removeFromLeft(360);  // Skip fixed columns
        diffBar.setBounds(bounds.reduced(5, 3));
    }
    
    void setRowIndex(int index) { rowIndex = index; repaint(); }
    
private:
    juce::String paramName;
    juce::String sectionName;
    int valueA = 0;
    int valueB = 0;
    int rowIndex = 0;
    
    DiffBarComponent diffBar;
    
    juce::Colour getSectionColour(const juce::String& section)
    {
        if (section.containsIgnoreCase("DCO"))
            return DS::Colors::sectionDCO;
        if (section.containsIgnoreCase("VCF"))
            return DS::Colors::sectionVCF;
        if (section.containsIgnoreCase("VCA"))
            return DS::Colors::sectionVCA;
        if (section.containsIgnoreCase("LFO"))
            return DS::Colors::sectionLFO;
        if (section.containsIgnoreCase("ENV"))
            return DS::Colors::sectionENV;
        if (section.containsIgnoreCase("Mixer"))
            return DS::Colors::sectionMixer;
        return DS::Colors::accent;
    }
};

//==============================================================================
/**
    VisualDiffTable - Full visual diff table component
*/
class VisualDiffTable : public juce::Component
{
public:
    VisualDiffTable()
    {
        // Header
        setSize(600, 400);
    }
    
    void setDifferences(const std::vector<std::tuple<juce::String, juce::String, int, int>>& diffs)
    {
        rows.clear();
        
        int index = 0;
        for (const auto& diff : diffs)
        {
            auto row = std::make_unique<DiffRowComponent>();
            row->setData(std::get<0>(diff), std::get<1>(diff), 
                        std::get<2>(diff), std::get<3>(diff));
            row->setRowIndex(index++);
            addAndMakeVisible(row.get());
            rows.push_back(std::move(row));
        }
        
        setSize(getWidth(), static_cast<int>(rows.size() * rowHeight + headerHeight));
        resized();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        
        // Header background
        auto header = bounds.removeFromTop(headerHeight);
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRect(header);
        
        // Header text
        g.setColour(DS::Colors::textMuted);
        g.setFont(11.0f);
        
        auto hb = header.toFloat();
        g.drawText("Section", hb.removeFromLeft(60), juce::Justification::centred);
        g.drawText("Parameter", hb.removeFromLeft(120), juce::Justification::centredLeft);
        g.drawText("A", hb.removeFromLeft(50), juce::Justification::centred);
        g.drawText("", hb.removeFromLeft(30), juce::Justification::centred);
        g.drawText("B", hb.removeFromLeft(50), juce::Justification::centred);
        g.drawText("Diff", hb.removeFromLeft(50), juce::Justification::centred);
        g.drawText("Visual Comparison", hb, juce::Justification::centred);
        
        // Separator line
        g.setColour(DS::Colors::border);
        g.drawHorizontalLine(headerHeight - 1, 0.0f, (float)getWidth());
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(headerHeight);
        
        for (auto& row : rows)
        {
            row->setBounds(bounds.removeFromTop(rowHeight));
        }
    }
    
    int getChangedCount() const
    {
        int count = 0;
        for (const auto& row : rows)
        {
            // Would need to track this in DiffRowComponent
        }
        return count;
    }
    
private:
    std::vector<std::unique_ptr<DiffRowComponent>> rows;
    static constexpr int headerHeight = 30;
    static constexpr int rowHeight = 28;
};

//==============================================================================
/**
    DiffSummaryBadge - Quick summary of differences
*/
class DiffSummaryBadge : public juce::Component
{
public:
    DiffSummaryBadge()
    {
        setSize(150, 30);
    }
    
    void setCounts(int total, int changed)
    {
        totalParams = total;
        changedParams = changed;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Progress bar showing percentage of changes
        if (totalParams > 0)
        {
            float percent = (float)changedParams / (float)totalParams;
            auto barBounds = bounds.reduced(3);
            barBounds.setWidth(barBounds.getWidth() * percent);
            
            juce::Colour barColour;
            if (percent < 0.25f)
                barColour = DS::Colors::success;
            else if (percent < 0.5f)
                barColour = DS::Colors::warning;
            else
                barColour = DS::Colors::error;
            
            g.setColour(barColour.withAlpha(0.3f));
            g.fillRoundedRectangle(barBounds, 3.0f);
        }
        
        // Text
        g.setColour(DS::Colors::text);
        g.setFont(12.0f);
        
        juce::String text = juce::String(changedParams) + " / " + juce::String(totalParams) + " changed";
        if (changedParams == 0)
            text = "Identical";
        
        g.drawText(text, bounds, juce::Justification::centred);
    }
    
private:
    int totalParams = 0;
    int changedParams = 0;
};
