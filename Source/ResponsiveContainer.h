#pragma once
#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "DesignSystem.h"

class ResponsiveContainer : public juce::Component {
public:
    enum class Mode {
        FixedColumns,      // מספר עמודות קבוע
        AutoColumns,       // עמודות אוטומטיות לפי רוחב
        SingleColumn,      // עמודה אחת (למסכים צרים)
        TwoColumn          // שתי עמודות
    };
    
    ResponsiveContainer() = default;
    
    void addSection(std::unique_ptr<juce::Component> section, int preferredWidth = 200) {
        sections.push_back({ std::move(section), preferredWidth });
        addAndMakeVisible(sections.back().component.get());
    }
    
    void resized() override {
        using namespace DesignSystem;
        
        auto bounds = getLocalBounds().reduced(Spacing::L);
        const int availableWidth = bounds.getWidth();
        
        // Determine mode based on width
        Mode mode;
        if (availableWidth < 500)
            mode = Mode::SingleColumn;
        else if (availableWidth < 900)
            mode = Mode::TwoColumn;
        else
            mode = Mode::AutoColumns;
        
        switch (mode) {
            case Mode::SingleColumn:
                layoutSingleColumn(bounds);
                break;
            case Mode::TwoColumn:
                layoutTwoColumn(bounds);
                break;
            case Mode::AutoColumns:
                layoutAutoColumns(bounds);
                break;
            default:
                break;
        }
    }
    
private:
    struct SectionInfo {
        std::unique_ptr<juce::Component> component;
        int preferredWidth;
    };
    std::vector<SectionInfo> sections;
    
    void layoutSingleColumn(juce::Rectangle<int> bounds) {
        using namespace DesignSystem;
        int y = bounds.getY();
        
        for (auto& section : sections) {
            int height = section.component->getHeight();
            if (height == 0) height = 200; // Default height
            
            section.component->setBounds(bounds.getX(), y, bounds.getWidth(), height);
            y += height + Spacing::L;
        }
    }
    
    void layoutTwoColumn(juce::Rectangle<int> bounds) {
        using namespace DesignSystem;
        
        const int colWidth = (bounds.getWidth() - Spacing::L) / 2;
        int leftY = bounds.getY();
        int rightY = bounds.getY();
        bool useLeft = true;
        
        for (auto& section : sections) {
            int height = section.component->getHeight();
            if (height == 0) height = 200;
            
            if (useLeft) {
                section.component->setBounds(bounds.getX(), leftY, colWidth, height);
                leftY += height + Spacing::L;
            } else {
                section.component->setBounds(bounds.getX() + colWidth + Spacing::L, 
                                            rightY, colWidth, height);
                rightY += height + Spacing::L;
            }
            useLeft = !useLeft;
        }
    }
    
    void layoutAutoColumns(juce::Rectangle<int> bounds) {
        using namespace DesignSystem;
        
        // Calculate how many sections fit per row
        int currentX = bounds.getX();
        int currentY = bounds.getY();
        int rowHeight = 0;
        
        for (auto& section : sections) {
            int width = section.preferredWidth;
            int height = section.component->getHeight();
            if (height == 0) height = 200;
            
            // Check if fits in current row
            if (currentX + width > bounds.getRight() && currentX > bounds.getX()) {
                // Start new row
                currentX = bounds.getX();
                currentY += rowHeight + Spacing::L;
                rowHeight = 0;
            }
            
            section.component->setBounds(currentX, currentY, width, height);
            currentX += width + Spacing::L;
            rowHeight = juce::jmax(rowHeight, height);
        }
    }
};
