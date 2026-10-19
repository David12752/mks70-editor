#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * KeyboardShortcuts.h
 * Comprehensive keyboard shortcut system with overlay display
 * 
 * Part of Improvement #11: Keyboard Shortcuts Panel
 */

namespace Shortcuts
{

//==============================================================================
/**
 * Shortcut definition
 */
struct Shortcut
{
    juce::String id;
    juce::String name;
    juce::String description;
    juce::String category;
    juce::KeyPress keyPress;
    std::function<void()> action;
    
    juce::String getKeyString() const
    {
        juce::String result;
        
        if (keyPress.getModifiers().isCtrlDown())
            result += "Ctrl+";
        if (keyPress.getModifiers().isShiftDown())
            result += "Shift+";
        if (keyPress.getModifiers().isAltDown())
            result += "Alt+";
        
        int keyCode = keyPress.getKeyCode();
        
        if (keyCode >= 'A' && keyCode <= 'Z')
            result += juce::String::charToString(static_cast<juce::juce_wchar>(keyCode));
        else if (keyCode >= '0' && keyCode <= '9')
            result += juce::String::charToString(static_cast<juce::juce_wchar>(keyCode));
        else if (keyCode == juce::KeyPress::F1Key) result += "F1";
        else if (keyCode == juce::KeyPress::F2Key) result += "F2";
        else if (keyCode == juce::KeyPress::F3Key) result += "F3";
        else if (keyCode == juce::KeyPress::F4Key) result += "F4";
        else if (keyCode == juce::KeyPress::F5Key) result += "F5";
        else if (keyCode == juce::KeyPress::escapeKey) result += "Esc";
        else if (keyCode == juce::KeyPress::returnKey) result += "Enter";
        else if (keyCode == juce::KeyPress::spaceKey) result += "Space";
        else if (keyCode == juce::KeyPress::tabKey) result += "Tab";
        else if (keyCode == juce::KeyPress::deleteKey) result += "Delete";
        else result += juce::KeyPress::createFromDescription(keyPress.getTextDescription()).getTextDescription();
        
        return result;
    }
};

//==============================================================================
/**
 * Shortcut Manager
 */
class ShortcutManager
{
public:
    static ShortcutManager& getInstance()
    {
        static ShortcutManager instance;
        return instance;
    }
    
    void registerShortcut(const Shortcut& shortcut)
    {
        shortcuts.push_back(shortcut);
    }
    
    void setAction(const juce::String& id, std::function<void()> action)
    {
        for (auto& shortcut : shortcuts)
        {
            if (shortcut.id == id)
            {
                shortcut.action = action;
                return;
            }
        }
    }
    
    void registerDefaults()
    {
        // File operations
        registerShortcut({"save", "Save Patch", "Save current patch", "File",
            juce::KeyPress('S', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"load", "Load Patch", "Load a patch file", "File",
            juce::KeyPress('O', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"new", "New Patch", "Initialize patch", "File",
            juce::KeyPress('N', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        
        // Edit operations
        registerShortcut({"undo", "Undo", "Undo last change", "Edit",
            juce::KeyPress('Z', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"redo", "Redo", "Redo last undone change", "Edit",
            juce::KeyPress('Y', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"copy", "Copy Tone", "Copy current tone", "Edit",
            juce::KeyPress('C', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"paste", "Paste Tone", "Paste copied tone", "Edit",
            juce::KeyPress('V', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"random", "Randomize", "Randomize parameters", "Edit",
            juce::KeyPress('R', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        
        // View operations
        registerShortcut({"toggle_upper", "Toggle Upper Tone", "Switch to Upper Tone tab", "View",
            juce::KeyPress('1', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"toggle_lower", "Toggle Lower Tone", "Switch to Lower Tone tab", "View",
            juce::KeyPress('2', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"toggle_patch", "Toggle Patch", "Switch to Patch tab", "View",
            juce::KeyPress('3', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"toggle_librarian", "Toggle Librarian", "Switch to Librarian tab", "View",
            juce::KeyPress('4', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        
        // MIDI operations
        registerShortcut({"send_all", "Send All", "Send all parameters to synth", "MIDI",
            juce::KeyPress('T', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"request_all", "Request All", "Request all parameters from synth", "MIDI",
            juce::KeyPress('G', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"panic", "MIDI Panic", "Send all notes off", "MIDI",
            juce::KeyPress(juce::KeyPress::escapeKey, juce::ModifierKeys::ctrlModifier, 0), nullptr});
        
        // Compare slots
        registerShortcut({"compare_ab", "Compare A/B", "Toggle A/B comparison", "Compare",
            juce::KeyPress('B', juce::ModifierKeys::ctrlModifier, 0), nullptr});
        registerShortcut({"slot_a", "Slot A", "Select compare slot A", "Compare",
            juce::KeyPress('A', juce::ModifierKeys::altModifier, 0), nullptr});
        registerShortcut({"slot_b", "Slot B", "Select compare slot B", "Compare",
            juce::KeyPress('B', juce::ModifierKeys::altModifier, 0), nullptr});
        registerShortcut({"slot_c", "Slot C", "Select compare slot C", "Compare",
            juce::KeyPress('C', juce::ModifierKeys::altModifier, 0), nullptr});
        registerShortcut({"slot_d", "Slot D", "Select compare slot D", "Compare",
            juce::KeyPress('D', juce::ModifierKeys::altModifier, 0), nullptr});
        
        // Help
        registerShortcut({"show_shortcuts", "Show Shortcuts", "Show keyboard shortcuts overlay", "Help",
            juce::KeyPress(juce::KeyPress::F1Key, 0, 0), nullptr});
    }
    
    bool handleKeyPress(const juce::KeyPress& key)
    {
        for (auto& shortcut : shortcuts)
        {
            if (shortcut.keyPress == key && shortcut.action)
            {
                shortcut.action();
                return true;
            }
        }
        return false;
    }
    
    const std::vector<Shortcut>& getShortcuts() const { return shortcuts; }
    
    juce::StringArray getCategories() const
    {
        juce::StringArray categories;
        for (const auto& shortcut : shortcuts)
        {
            if (!categories.contains(shortcut.category))
                categories.add(shortcut.category);
        }
        return categories;
    }
    
    std::vector<Shortcut> getShortcutsInCategory(const juce::String& category) const
    {
        std::vector<Shortcut> result;
        for (const auto& shortcut : shortcuts)
        {
            if (shortcut.category == category)
                result.push_back(shortcut);
        }
        return result;
    }
    
private:
    ShortcutManager() { registerDefaults(); }
    
    std::vector<Shortcut> shortcuts;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShortcutManager)
};

//==============================================================================
/**
 * Keyboard Shortcut Overlay
 * Full-screen overlay showing all shortcuts
 */
class ShortcutsOverlay : public juce::Component
{
public:
    ShortcutsOverlay()
    {
        setAlpha(0.0f);
    }
    
    void show()
    {
        setVisible(true);
        juce::Desktop::getInstance().getAnimator().animateComponent(
            this, getBounds(), 1.0f, 200, false, 1.0, 0.0);
    }
    
    void hide()
    {
        juce::Desktop::getInstance().getAnimator().animateComponent(
            this, getBounds(), 0.0f, 200, false, 1.0, 0.0);
        
        juce::Timer::callAfterDelay(200, [this] { setVisible(false); });
    }
    
    void paint(juce::Graphics& g) override
    {
        // Dark overlay
        g.fillAll(juce::Colour(0xDD000000));
        
        auto& manager = ShortcutManager::getInstance();
        auto categories = manager.getCategories();
        
        auto bounds = getLocalBounds().reduced(40);
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading.withHeight(28.0f));
        g.drawText("Keyboard Shortcuts", bounds.removeFromTop(50), juce::Justification::centred);
        
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::body);
        g.drawText("Press ESC or F1 to close", bounds.removeFromTop(25), juce::Justification::centred);
        
        bounds.removeFromTop(20);
        
        // Calculate columns
        int numColumns = juce::jmin(4, categories.size());
        int columnWidth = bounds.getWidth() / numColumns;
        
        for (int col = 0; col < numColumns; col++)
        {
            if (col >= categories.size())
                break;
            
            auto columnBounds = bounds.removeFromLeft(columnWidth).reduced(10, 0);
            drawCategory(g, columnBounds, categories[col], manager);
        }
    }
    
    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key.isKeyCode(juce::KeyPress::escapeKey) || 
            key.isKeyCode(juce::KeyPress::F1Key))
        {
            hide();
            return true;
        }
        return false;
    }
    
    void mouseDown(const juce::MouseEvent&) override
    {
        hide();
    }
    
private:
    void drawCategory(juce::Graphics& g, juce::Rectangle<int> bounds, 
                      const juce::String& category, ShortcutManager& manager)
    {
        auto shortcuts = manager.getShortcutsInCategory(category);
        
        // Category header
        g.setColour(DS::Color::accent);
        g.setFont(DS::Font::heading.withHeight(16.0f));
        g.drawText(category, bounds.removeFromTop(25), juce::Justification::centredLeft);
        
        g.setColour(DS::Color::border);
        g.drawHorizontalLine(bounds.getY(), bounds.getX(), bounds.getRight());
        bounds.removeFromTop(10);
        
        // Shortcuts
        for (const auto& shortcut : shortcuts)
        {
            auto row = bounds.removeFromTop(22);
            
            // Key badge
            auto keyBounds = row.removeFromRight(80);
            g.setColour(DS::Color::surfaceAlt);
            g.fillRoundedRectangle(keyBounds.toFloat().reduced(2), 4);
            
            g.setColour(DS::Color::accent);
            g.setFont(DS::Font::label.boldened());
            g.drawText(shortcut.getKeyString(), keyBounds, juce::Justification::centred);
            
            // Name
            g.setColour(DS::Color::text);
            g.setFont(DS::Font::body);
            g.drawText(shortcut.name, row, juce::Justification::centredLeft);
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShortcutsOverlay)
};

//==============================================================================
/**
 * Shortcuts Panel (for Settings tab)
 */
class ShortcutsPanel : public juce::Component
{
public:
    ShortcutsPanel()
    {
        auto& manager = ShortcutManager::getInstance();
        
        // Build the list
        auto categories = manager.getCategories();
        
        for (const auto& category : categories)
        {
            auto shortcuts = manager.getShortcutsInCategory(category);
            
            for (const auto& shortcut : shortcuts)
            {
                shortcutList.push_back({shortcut.name, shortcut.getKeyString(), category});
            }
        }
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        auto contentBounds = bounds.reduced(DS::Space::md).toNearestInt();
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading);
        g.drawText("Keyboard Shortcuts", contentBounds.removeFromTop(24), juce::Justification::centredLeft);
        
        contentBounds.removeFromTop(DS::Space::sm);
        
        // List header
        auto headerRow = contentBounds.removeFromTop(20);
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText("Action", headerRow.removeFromLeft(150), juce::Justification::centredLeft);
        g.drawText("Shortcut", headerRow.removeFromLeft(80), juce::Justification::centred);
        g.drawText("Category", headerRow, juce::Justification::centredLeft);
        
        g.setColour(DS::Color::border);
        g.drawHorizontalLine(contentBounds.getY(), contentBounds.getX(), contentBounds.getRight());
        contentBounds.removeFromTop(5);
        
        // List items
        juce::String lastCategory;
        for (const auto& item : shortcutList)
        {
            if (contentBounds.getHeight() < 20)
                break;
            
            auto row = contentBounds.removeFromTop(22);
            
            g.setColour(DS::Color::text);
            g.setFont(DS::Font::body);
            g.drawText(item.name, row.removeFromLeft(150), juce::Justification::centredLeft);
            
            // Key badge
            auto keyBounds = row.removeFromLeft(80);
            g.setColour(DS::Color::surfaceAlt);
            g.fillRoundedRectangle(keyBounds.toFloat().reduced(4, 2), 3);
            g.setColour(DS::Color::accent);
            g.setFont(DS::Font::label);
            g.drawText(item.key, keyBounds, juce::Justification::centred);
            
            g.setColour(DS::Color::textMuted);
            g.setFont(DS::Font::body);
            g.drawText(item.category, row, juce::Justification::centredLeft);
        }
    }
    
private:
    struct ShortcutEntry
    {
        juce::String name;
        juce::String key;
        juce::String category;
    };
    
    std::vector<ShortcutEntry> shortcutList;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShortcutsPanel)
};

} // namespace Shortcuts
