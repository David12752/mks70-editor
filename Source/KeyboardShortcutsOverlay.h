#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * KeyboardShortcutsOverlay - Shows all available keyboard shortcuts
 * Triggered by pressing F1
 */
class KeyboardShortcutsOverlay : public juce::Component {
public:
    KeyboardShortcutsOverlay() {
        setAlwaysOnTop(true);
        setWantsKeyboardFocus(true);
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Semi-transparent background
        g.setColour(juce::Colour(0xE5101015));
        g.fillRoundedRectangle(bounds, DS::Radius::L);
        
        // Border
        g.setColour(DS::Color::Accent::primary().withAlpha(0.5f));
        g.drawRoundedRectangle(bounds.reduced(1), DS::Radius::L, 2.0f);
        
        // Title
        auto titleArea = bounds.removeFromTop(50);
        g.setColour(DS::Color::Accent::primary());
        g.setFont(DS::Font::xlarge());
        g.drawText("Keyboard Shortcuts", titleArea, juce::Justification::centred);
        
        // Subtitle
        g.setColour(DS::Color::textMuted());
        g.setFont(DS::Font::small());
        g.drawText("Press F1 or Escape to close", titleArea.translated(0, 25), juce::Justification::centred);
        
        bounds.reduce(30, 10);
        
        // Define shortcut groups
        struct ShortcutGroup {
            juce::String title;
            std::vector<std::pair<juce::String, juce::String>> shortcuts;
        };
        
        std::vector<ShortcutGroup> groups = {
            { "Navigation", {
                { "Ctrl+1", "Tone Upper" },
                { "Ctrl+2", "Tone Lower" },
                { "Ctrl+3", "Patch" },
                { "Ctrl+4", "Arpeggiator" },
                { "Ctrl+5", "Librarian" },
                { "Ctrl+6", "Automation" },
                { "Ctrl+7", "Macros" },
                { "Ctrl+8", "Settings" }
            }},
            { "Edit", {
                { "Ctrl+Z", "Undo" },
                { "Ctrl+Y", "Redo" },
                { "Ctrl+Shift+Z", "Redo (Alt)" },
                { "Ctrl+C", "Copy Tone" },
                { "Ctrl+V", "Paste Tone" }
            }},
            { "Zoom", {
                { "Ctrl+=", "Zoom In" },
                { "Ctrl+-", "Zoom Out" },
                { "Ctrl+0", "Reset Zoom (100%)" }
            }},
            { "Tools", {
                { "F1", "Show Shortcuts" },
                { "Ctrl+I", "Initialize Tone" },
                { "Ctrl+R", "Randomize" },
                { "Ctrl+A", "Compare A/B" },
                { "Ctrl+M", "Morph Panel" }
            }}
        };
        
        // Calculate layout
        int numColumns = 2;
        float columnWidth = bounds.getWidth() / numColumns;
        float rowHeight = 24;
        
        int groupIndex = 0;
        for (const auto& group : groups) {
            int col = groupIndex % numColumns;
            float baseX = bounds.getX() + col * columnWidth;
            float baseY = bounds.getY() + (groupIndex / numColumns) * (8 * rowHeight + 40);
            
            // Group title
            g.setColour(DS::Color::Accent::cyan());
            g.setFont(DS::Font::medium());
            g.drawText(group.title, baseX, baseY, columnWidth - 20, 30, juce::Justification::left);
            
            baseY += 30;
            
            // Shortcuts
            for (size_t i = 0; i < group.shortcuts.size(); i++) {
                float y = baseY + i * rowHeight;
                
                // Key combination (styled)
                g.setColour(DS::Color::bgElevated());
                juce::Rectangle<float> keyRect(baseX, y + 2, 90, 20);
                g.fillRoundedRectangle(keyRect, 4);
                
                g.setColour(DS::Color::Accent::primary());
                g.setFont(DS::Font::small());
                g.drawText(group.shortcuts[i].first, keyRect, juce::Justification::centred);
                
                // Description
                g.setColour(DS::Color::textSecondary());
                g.drawText(group.shortcuts[i].second, baseX + 100, y, columnWidth - 120, 24, juce::Justification::left);
            }
            
            groupIndex++;
        }
        
        // Footer
        auto footer = getLocalBounds().toFloat().removeFromBottom(35);
        g.setColour(DS::Color::textMuted().withAlpha(0.5f));
        g.setFont(DS::Font::tiny());
        g.drawText("MKS-70 Editor | Tip: Right-click any knob for MIDI Learn", footer, juce::Justification::centred);
    }
    
    void mouseDown(const juce::MouseEvent&) override {
        if (onClose) onClose();
    }
    
    bool keyPressed(const juce::KeyPress& key) override {
        if (key == juce::KeyPress::escapeKey || key == juce::KeyPress::F1Key) {
            if (onClose) onClose();
            return true;
        }
        return false;
    }
    
    std::function<void()> onClose;
};
