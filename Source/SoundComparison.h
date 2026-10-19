#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * SoundComparison.h
 * Visual and audible comparison between presets/patches
 * 
 * Part of Improvement #18: Sound Comparison Tool
 */

namespace SoundCompare
{

//==============================================================================
/**
 * Parameter difference entry
 */
struct ParamDiff
{
    juce::String paramName;
    juce::String category;
    float valueA = 0.0f;
    float valueB = 0.0f;
    int rawValueA = 0;
    int rawValueB = 0;
    float differencePercent = 0.0f;
    
    bool hasDifference() const { return rawValueA != rawValueB; }
};

//==============================================================================
/**
 * Sound Comparison Result
 */
struct ComparisonResult
{
    juce::String patchNameA;
    juce::String patchNameB;
    std::vector<ParamDiff> differences;
    int totalParams = 0;
    int differentParams = 0;
    float overallSimilarity = 0.0f; // 0-100%
    
    void calculateStats()
    {
        differentParams = 0;
        for (const auto& diff : differences)
        {
            if (diff.hasDifference())
                differentParams++;
        }
        
        if (totalParams > 0)
            overallSimilarity = 100.0f * (1.0f - static_cast<float>(differentParams) / totalParams);
    }
};

//==============================================================================
/**
 * Comparison Bar - Visual diff indicator
 */
class ComparisonBar : public juce::Component
{
public:
    ComparisonBar()
    {
    }
    
    void setValues(float a, float b)
    {
        valueA = juce::jlimit(0.0f, 1.0f, a);
        valueB = juce::jlimit(0.0f, 1.0f, b);
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1);
        float barHeight = (bounds.getHeight() - 4) / 2;
        
        // Bar A
        auto barABounds = bounds.removeFromTop(barHeight);
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(barABounds, 2);
        
        g.setColour(juce::Colour(0xFF6366f1)); // Blue-ish
        g.fillRoundedRectangle(barABounds.withWidth(barABounds.getWidth() * valueA), 2);
        
        bounds.removeFromTop(4);
        
        // Bar B
        auto barBBounds = bounds.removeFromTop(barHeight);
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(barBBounds, 2);
        
        g.setColour(juce::Colour(0xFFec4899)); // Pink
        g.fillRoundedRectangle(barBBounds.withWidth(barBBounds.getWidth() * valueB), 2);
        
        // Difference indicator
        if (std::abs(valueA - valueB) > 0.01f)
        {
            g.setColour(DS::Color::accent);
            float minX = juce::jmin(valueA, valueB) * getWidth();
            float maxX = juce::jmax(valueA, valueB) * getWidth();
            g.fillRect(minX, bounds.getY() - 2, maxX - minX, 4.0f);
        }
    }
    
private:
    float valueA = 0.0f;
    float valueB = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComparisonBar)
};

//==============================================================================
/**
 * Compact Sound Comparison Panel - Modern floating design
 */
class ComparisonPanel : public juce::Component, public juce::TableListBoxModel
{
public:
    ComparisonPanel()
    {
        // Compact table setup
        table.setModel(this);
        table.setHeader(new juce::TableHeaderComponent());
        
        table.getHeader().addColumn("Param", 1, 100);
        table.getHeader().addColumn("A", 2, 45);
        table.getHeader().addColumn("B", 3, 45);
        table.getHeader().addColumn("%", 4, 35);
        
        table.setHeaderHeight(22);
        table.setRowHeight(20);
        table.getHeader().setColour(juce::TableHeaderComponent::backgroundColourId, DS::Color::bgElevated());
        table.getHeader().setColour(juce::TableHeaderComponent::textColourId, DS::Color::textSecondary());
        addAndMakeVisible(table);
        
        // Compact toggle
        showDiffsOnly.setButtonText("Diff only");
        showDiffsOnly.setToggleState(true, juce::dontSendNotification);
        showDiffsOnly.onClick = [this] { refreshTable(); };
        showDiffsOnly.setColour(juce::ToggleButton::textColourId, DS::Color::textSecondary());
        addAndMakeVisible(showDiffsOnly);
        
        // Compact category filter
        categoryFilter.addItem("All", 1);
        categoryFilter.addItem("DCO", 2);
        categoryFilter.addItem("VCF", 3);
        categoryFilter.addItem("VCA", 4);
        categoryFilter.addItem("ENV", 5);
        categoryFilter.addItem("LFO", 6);
        categoryFilter.setSelectedId(1);
        categoryFilter.onChange = [this] { refreshTable(); };
        categoryFilter.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
        categoryFilter.setColour(juce::ComboBox::outlineColourId, DS::Color::border());
        categoryFilter.setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
        addAndMakeVisible(categoryFilter);
        
        // Close button
        closeBtn.setButtonText("X");
        closeBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::error().withAlpha(0.3f));
        closeBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textPrimary());
        closeBtn.onClick = [this]() { if (onClose) onClose(); };
        addAndMakeVisible(closeBtn);
    }
    
    std::function<void()> onClose;
    
    void setComparison(const ComparisonResult& result)
    {
        comparisonResult = result;
        refreshTable();
        repaint();
    }
    
    void refreshTable()
    {
        filteredDiffs.clear();
        
        juce::String catFilter = "";
        if (categoryFilter.getSelectedId() > 1)
            catFilter = categoryFilter.getText();
        
        for (const auto& diff : comparisonResult.differences)
        {
            if (showDiffsOnly.getToggleState() && !diff.hasDifference())
                continue;
            
            if (catFilter.isNotEmpty() && diff.category != catFilter)
                continue;
            
            filteredDiffs.push_back(diff);
        }
        
        table.updateContent();
    }
    
    // TableListBoxModel
    int getNumRows() override { return static_cast<int>(filteredDiffs.size()); }
    
    void paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll(DS::Color::accent.withAlpha(0.2f));
        else if (rowNumber % 2)
            g.fillAll(DS::Color::surfaceAlt.withAlpha(0.3f));
    }
    
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override
    {
        if (rowNumber >= static_cast<int>(filteredDiffs.size()))
            return;
        
        const auto& diff = filteredDiffs[rowNumber];
        g.setFont(DS::Font::body);
        
        switch (columnId)
        {
            case 1: // Parameter name
                g.setColour(DS::Color::text);
                g.drawText(diff.paramName, 4, 0, width - 8, height, juce::Justification::centredLeft);
                break;
                
            case 2: // Value A
                g.setColour(juce::Colour(0xFF6366f1));
                g.drawText(juce::String(diff.rawValueA), 4, 0, width - 8, height, juce::Justification::centred);
                break;
                
            case 3: // Value B
                g.setColour(juce::Colour(0xFFec4899));
                g.drawText(juce::String(diff.rawValueB), 4, 0, width - 8, height, juce::Justification::centred);
                break;
                
            case 4: // Diff percent
                if (diff.hasDifference())
                {
                    g.setColour(DS::Color::accent);
                    g.drawText(juce::String(static_cast<int>(diff.differencePercent)) + "%",
                              4, 0, width - 8, height, juce::Justification::centred);
                }
                else
                {
                    g.setColour(DS::Color::textMuted);
                    g.drawText("-", 4, 0, width - 8, height, juce::Justification::centred);
                }
                break;
                
            case 5: // Visual bar - handled in refreshComponentForCell
                break;
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(8);
        
        // Header row with title and close button
        auto headerRow = bounds.removeFromTop(24);
        closeBtn.setBounds(headerRow.removeFromRight(22).withSizeKeepingCentre(20, 20));
        headerRow.removeFromRight(4);
        // Title is painted, not a component
        
        bounds.removeFromTop(4);
        
        // Compact stats row
        auto statsRow = bounds.removeFromTop(20);
        // Stats painted in paint()
        
        bounds.removeFromTop(4);
        
        // Filter row (compact)
        auto filterRow = bounds.removeFromTop(22);
        showDiffsOnly.setBounds(filterRow.removeFromLeft(70));
        filterRow.removeFromLeft(8);
        categoryFilter.setBounds(filterRow.removeFromLeft(60));
        
        bounds.removeFromTop(4);
        
        // Table takes remaining space
        table.setBounds(bounds);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Compact card background with subtle glow
        g.setColour(DS::Color::bgCard());
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // Border with accent
        g.setColour(DS::Color::Accent::magenta().withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.5f);
        
        auto inner = bounds.reduced(8);
        
        // Compact header
        auto headerBounds = inner.removeFromTop(24);
        g.setColour(DS::Color::Accent::magenta());
        g.setFont(DS::Font::bold(11.0f));
        g.drawText("COMPARE", headerBounds, juce::Justification::centredLeft);
        
        inner.removeFromTop(4);
        
        // Patch names row (compact)
        auto namesRow = inner.removeFromTop(20);
        g.setFont(DS::Font::small());
        g.setColour(juce::Colour(0xFF6366f1));
        g.drawText("A:" + comparisonResult.patchNameA.substring(0, 12), 
                   namesRow.removeFromLeft(namesRow.getWidth() / 2), 
                   juce::Justification::centredLeft);
        
        g.setColour(juce::Colour(0xFFec4899));
        g.drawText("B:" + comparisonResult.patchNameB.substring(0, 12), 
                   namesRow, juce::Justification::centredLeft);
    }
    
private:
    ComparisonResult comparisonResult;
    std::vector<ParamDiff> filteredDiffs;
    
    juce::TableListBox table;
    juce::ToggleButton showDiffsOnly;
    juce::ComboBox categoryFilter;
    juce::TextButton closeBtn;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComparisonPanel)
};

//==============================================================================
/**
 * Mini similarity indicator
 */
class SimilarityBadge : public juce::Component
{
public:
    void setSimilarity(float percent)
    {
        similarity = juce::jlimit(0.0f, 100.0f, percent);
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background arc
        g.setColour(DS::Color::surfaceAlt);
        juce::Path bgArc;
        bgArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                            bounds.getWidth() / 2 - 4, bounds.getHeight() / 2 - 4,
                            0, -2.5f, 2.5f, true);
        g.strokePath(bgArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
        
        // Value arc
        float angle = (similarity / 100.0f) * 5.0f - 2.5f;
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                               bounds.getWidth() / 2 - 4, bounds.getHeight() / 2 - 4,
                               0, -2.5f, angle, true);
        
        juce::Colour arcColor;
        if (similarity >= 80) arcColor = juce::Colour(0xFF10b981);
        else if (similarity >= 50) arcColor = juce::Colour(0xFFf59e0b);
        else arcColor = juce::Colour(0xFFef4444);
        
        g.setColour(arcColor);
        g.strokePath(valueArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        
        // Text
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::body.boldened());
        g.drawText(juce::String(static_cast<int>(similarity)) + "%",
                   bounds, juce::Justification::centred);
    }
    
private:
    float similarity = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimilarityBadge)
};

} // namespace SoundCompare
