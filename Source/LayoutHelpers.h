#pragma once
#include <JuceHeader.h>
#include <vector>
#include "DesignSystem.h"

namespace Layout {

    //==========================================================================
    // חישוב גריד אוטומטי
    //==========================================================================
    struct GridLayout {
        int numColumns{};
        int numRows{};
        int itemWidth{};
        int itemHeight{};
        int gapX{};
        int gapY{};
        
        static GridLayout calculate(juce::Rectangle<int> bounds, int numItems,
                                    int minItemWidth, int idealItemHeight,
                                    int gap = DesignSystem::Spacing::M)
        {
            GridLayout result{};
            result.gapX = result.gapY = gap;
            
            // Calculate columns that fit
            result.numColumns = juce::jmax(1, (bounds.getWidth() + gap) / (minItemWidth + gap));
            result.numRows = (numItems + result.numColumns - 1) / result.numColumns;
            
            // Distribute width evenly
            result.itemWidth = (bounds.getWidth() - (result.numColumns - 1) * gap) / result.numColumns;
            result.itemHeight = idealItemHeight;
            
            return result;
        }
        
        juce::Rectangle<int> getItemBounds(juce::Rectangle<int> containerBounds, int index) const {
            const int col = index % numColumns;
            const int row = index / numColumns;
            
            return juce::Rectangle<int>(
                containerBounds.getX() + col * (itemWidth + gapX),
                containerBounds.getY() + row * (itemHeight + gapY),
                itemWidth,
                itemHeight
            );
        }
    };

    //==========================================================================
    // FlexBox wrapper לשימוש קל
    //==========================================================================
    class FlexRow {
    public:
        explicit FlexRow(int gap = DesignSystem::Spacing::M) : spacing(gap) {}
        
        FlexRow& add(juce::Component& comp, float flexGrow = 0, int minWidth = 0, int maxWidth = -1) {
            juce::FlexItem item(comp);
            item.flexGrow = flexGrow;
            if (minWidth > 0) item.minWidth = (float) minWidth;
            if (maxWidth > 0) item.maxWidth = (float) maxWidth;
            item.margin = juce::FlexItem::Margin(0, (float) spacing / 2.0f, 0, (float) spacing / 2.0f);
            items.push_back(item);
            return *this;
        }
        
        FlexRow& addSpacer(float flexGrow = 1.0f) {
            juce::FlexItem item;
            item.flexGrow = flexGrow;
            items.push_back(item);
            return *this;
        }
        
        FlexRow& addFixed(juce::Component& comp, int width) {
            juce::FlexItem item(comp);
            item.width = (float) width;
            item.margin = juce::FlexItem::Margin(0, (float) spacing / 2.0f, 0, (float) spacing / 2.0f);
            items.push_back(item);
            return *this;
        }
        
        void performLayout(juce::Rectangle<int> bounds) {
            juce::FlexBox fb;
            fb.flexDirection = juce::FlexBox::Direction::row;
            fb.alignItems = juce::FlexBox::AlignItems::center;
            fb.justifyContent = juce::FlexBox::JustifyContent::flexStart;
            
            for (auto& item : items)
                fb.items.add(item);
            
            fb.performLayout(bounds.toFloat());
        }
        
    private:
        std::vector<juce::FlexItem> items;
        int spacing;
    };

    class FlexColumn {
    public:
        explicit FlexColumn(int gap = DesignSystem::Spacing::M) : spacing(gap) {}
        
        FlexColumn& add(juce::Component& comp, float flexGrow = 0, int minHeight = 0) {
            juce::FlexItem item(comp);
            item.flexGrow = flexGrow;
            if (minHeight > 0) item.minHeight = (float) minHeight;
            item.margin = juce::FlexItem::Margin((float) spacing / 2.0f, 0, (float) spacing / 2.0f, 0);
            items.push_back(item);
            return *this;
        }
        
        FlexColumn& addFixed(juce::Component& comp, int height) {
            juce::FlexItem item(comp);
            item.height = (float) height;
            item.margin = juce::FlexItem::Margin((float) spacing / 2.0f, 0, (float) spacing / 2.0f, 0);
            items.push_back(item);
            return *this;
        }
        
        FlexColumn& addSpacer(float flexGrow = 1.0f) {
            juce::FlexItem item;
            item.flexGrow = flexGrow;
            items.push_back(item);
            return *this;
        }
        
        void performLayout(juce::Rectangle<int> bounds) {
            juce::FlexBox fb;
            fb.flexDirection = juce::FlexBox::Direction::column;
            fb.alignItems = juce::FlexBox::AlignItems::stretch;
            
            for (auto& item : items)
                fb.items.add(item);
            
            fb.performLayout(bounds.toFloat());
        }
        
    private:
        std::vector<juce::FlexItem> items;
        int spacing;
    };
}
