#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "ToneParameters.h"
#include "PatchParameters.h"

/**
 * CompareSlots.h
 * A/B/C/D Compare System for comparing multiple preset variations
 * Like Serum, Diva, and other professional synths
 * 
 * Part of Improvement #6: Compare A/B/C/D
 */

namespace Compare
{

//==============================================================================
/**
 * Slot identifier
 */
enum class Slot
{
    A = 0,
    B = 1,
    C = 2,
    D = 3
};

//==============================================================================
/**
 * Complete state snapshot for comparison
 */
struct SlotState
{
    ToneParams toneUpperParams;
    ToneParams toneLowerParams;
    PatchParameters patchParams;
    
    juce::String name = "Empty";
    bool hasData = false;
    juce::Time savedTime;
    
    void clear()
    {
        name = "Empty";
        hasData = false;
        toneUpperParams = ToneParams();
        toneLowerParams = ToneParams();
        patchParams = PatchParameters();
    }
};

//==============================================================================
/**
 * Compare Slot Manager
 * Manages 4 slots for A/B/C/D comparison
 */
class SlotManager
{
public:
    SlotManager()
    {
        // Initialize all slots as empty
        for (auto& slot : slots)
        {
            slot.clear();
        }
        
        // Slot A is always active by default
        slots[0].name = "Current";
        currentSlot = Slot::A;
    }
    
    //==========================================================================
    // Slot Management
    //==========================================================================
    
    void saveToSlot(Slot slot, const ToneParams& toneUpper, const ToneParams& toneLower, 
                    const PatchParameters& patch, const juce::String& name = "")
    {
        int index = static_cast<int>(slot);
        slots[index].toneUpperParams = toneUpper;
        slots[index].toneLowerParams = toneLower;
        slots[index].patchParams = patch;
        slots[index].hasData = true;
        slots[index].savedTime = juce::Time::getCurrentTime();
        
        if (name.isNotEmpty())
            slots[index].name = name;
        else
            slots[index].name = getSlotLetter(slot);
    }
    
    void copySlot(Slot from, Slot to)
    {
        int fromIdx = static_cast<int>(from);
        int toIdx = static_cast<int>(to);
        
        if (slots[fromIdx].hasData)
        {
            slots[toIdx] = slots[fromIdx];
            slots[toIdx].savedTime = juce::Time::getCurrentTime();
        }
    }
    
    void swapSlots(Slot a, Slot b)
    {
        int aIdx = static_cast<int>(a);
        int bIdx = static_cast<int>(b);
        
        std::swap(slots[aIdx], slots[bIdx]);
    }
    
    void clearSlot(Slot slot)
    {
        int index = static_cast<int>(slot);
        slots[index].clear();
    }
    
    //==========================================================================
    // Recall Functions
    //==========================================================================
    
    bool recallSlot(Slot slot, ToneParams& toneUpper, ToneParams& toneLower, 
                    PatchParameters& patch)
    {
        int index = static_cast<int>(slot);
        
        if (!slots[index].hasData)
            return false;
        
        toneUpper = slots[index].toneUpperParams;
        toneLower = slots[index].toneLowerParams;
        patch = slots[index].patchParams;
        
        currentSlot = slot;
        return true;
    }
    
    const SlotState& getSlotState(Slot slot) const
    {
        return slots[static_cast<int>(slot)];
    }
    
    bool slotHasData(Slot slot) const
    {
        return slots[static_cast<int>(slot)].hasData;
    }
    
    Slot getCurrentSlot() const { return currentSlot; }
    void setCurrentSlot(Slot slot) { currentSlot = slot; }
    
    //==========================================================================
    // Utility
    //==========================================================================
    
    static juce::String getSlotLetter(Slot slot)
    {
        switch (slot)
        {
            case Slot::A: return "A";
            case Slot::B: return "B";
            case Slot::C: return "C";
            case Slot::D: return "D";
            default: return "?";
        }
    }
    
    static juce::Colour getSlotColor(Slot slot)
    {
        switch (slot)
        {
            case Slot::A: return juce::Colour(0xFF4CAF50); // Green
            case Slot::B: return juce::Colour(0xFF2196F3); // Blue
            case Slot::C: return juce::Colour(0xFFFF9800); // Orange
            case Slot::D: return juce::Colour(0xFFE91E63); // Pink
            default: return DS::Color::textMuted;
        }
    }
    
private:
    std::array<SlotState, 4> slots;
    Slot currentSlot = Slot::A;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlotManager)
};

//==============================================================================
/**
 * Slot Button
 * Single button for A/B/C/D selection
 */
class SlotButton : public juce::Component
{
public:
    SlotButton(Slot slot) : slotId(slot)
    {
        setRepaintsOnMouseActivity(true);
    }
    
    void setActive(bool shouldBeActive)
    {
        isActive = shouldBeActive;
        repaint();
    }
    
    void setHasData(bool hasData)
    {
        containsData = hasData;
        repaint();
    }
    
    void setSlotName(const juce::String& name)
    {
        slotName = name;
        repaint();
    }
    
    std::function<void(Slot)> onClick;
    std::function<void(Slot)> onRightClick;
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        bool isHovered = isMouseOver();
        
        juce::Colour slotColor = SlotManager::getSlotColor(slotId);
        
        // Background
        if (isActive)
        {
            // Active slot - filled with gradient
            juce::ColourGradient gradient(
                slotColor.withAlpha(0.8f),
                bounds.getX(), bounds.getY(),
                slotColor.withAlpha(0.5f),
                bounds.getX(), bounds.getBottom(),
                false
            );
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds, DS::Radius::medium);
            
            // Glow
            g.setColour(slotColor.withAlpha(0.3f));
            g.drawRoundedRectangle(bounds.expanded(1), DS::Radius::medium + 1, 2.0f);
        }
        else if (isHovered)
        {
            g.setColour(slotColor.withAlpha(0.2f));
            g.fillRoundedRectangle(bounds, DS::Radius::medium);
        }
        else
        {
            g.setColour(DS::Color::surfaceAlt);
            g.fillRoundedRectangle(bounds, DS::Radius::medium);
        }
        
        // Border
        g.setColour(isActive ? slotColor : DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        // Letter
        g.setColour(isActive ? juce::Colours::white : (containsData ? slotColor : DS::Color::textMuted));
        g.setFont(DS::Font::heading.withHeight(20.0f));
        g.drawText(SlotManager::getSlotLetter(slotId), bounds, juce::Justification::centred);
        
        // Data indicator (small dot)
        if (containsData && !isActive)
        {
            float dotSize = 6.0f;
            float dotX = bounds.getRight() - dotSize - 4;
            float dotY = bounds.getY() + 4;
            g.setColour(slotColor);
            g.fillEllipse(dotX, dotY, dotSize, dotSize);
        }
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            if (onRightClick)
                onRightClick(slotId);
        }
        else
        {
            if (onClick)
                onClick(slotId);
        }
    }
    
private:
    Slot slotId;
    bool isActive = false;
    bool containsData = false;
    juce::String slotName;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlotButton)
};

//==============================================================================
/**
 * Compare Panel
 * Full UI panel with A/B/C/D buttons and controls
 */
class ComparePanel : public juce::Component
{
public:
    ComparePanel(SlotManager& manager) : slotManager(manager)
    {
        // Create slot buttons
        for (int i = 0; i < 4; i++)
        {
            auto slot = static_cast<Slot>(i);
            auto button = std::make_unique<SlotButton>(slot);
            
            button->onClick = [this](Slot s) { selectSlot(s); };
            button->onRightClick = [this](Slot s) { showSlotMenu(s); };
            
            addAndMakeVisible(*button);
            slotButtons.push_back(std::move(button));
        }
        
        // Copy button
        copyButton.setButtonText("Copy");
        copyButton.onClick = [this] { showCopyMenu(); };
        addAndMakeVisible(copyButton);
        
        // Swap button
        swapButton.setButtonText("Swap");
        swapButton.onClick = [this] { showSwapMenu(); };
        addAndMakeVisible(swapButton);
        
        updateButtonStates();
    }
    
    std::function<void(Slot)> onSlotSelected;
    std::function<void()> onStateChanged;
    
    void updateButtonStates()
    {
        for (int i = 0; i < 4; i++)
        {
            auto slot = static_cast<Slot>(i);
            slotButtons[i]->setActive(slotManager.getCurrentSlot() == slot);
            slotButtons[i]->setHasData(slotManager.slotHasData(slot));
        }
        repaint();
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::sm);
        
        // Slot buttons
        int buttonSize = 36;
        int spacing = 4;
        
        auto buttonsArea = bounds.removeFromLeft(buttonSize * 4 + spacing * 3);
        
        for (int i = 0; i < 4; i++)
        {
            slotButtons[i]->setBounds(buttonsArea.removeFromLeft(buttonSize));
            if (i < 3)
                buttonsArea.removeFromLeft(spacing);
        }
        
        bounds.removeFromLeft(DS::Space::md);
        
        // Action buttons
        int actionButtonWidth = 50;
        copyButton.setBounds(bounds.removeFromLeft(actionButtonWidth).reduced(2));
        bounds.removeFromLeft(4);
        swapButton.setBounds(bounds.removeFromLeft(actionButtonWidth).reduced(2));
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        // Label
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText("COMPARE", bounds.reduced(4).removeFromRight(60), juce::Justification::centredRight);
    }
    
private:
    void selectSlot(Slot slot)
    {
        if (slotManager.slotHasData(slot))
        {
            slotManager.setCurrentSlot(slot);
            updateButtonStates();
            
            if (onSlotSelected)
                onSlotSelected(slot);
        }
        else
        {
            // Save current to this slot
            // This would need access to the current parameters
            slotManager.setCurrentSlot(slot);
            updateButtonStates();
        }
    }
    
    void showSlotMenu(Slot slot)
    {
        juce::PopupMenu menu;
        
        juce::String slotLetter = SlotManager::getSlotLetter(slot);
        
        menu.addItem(1, "Save to Slot " + slotLetter);
        menu.addItem(2, "Clear Slot " + slotLetter, slotManager.slotHasData(slot));
        menu.addSeparator();
        menu.addItem(3, "Rename Slot " + slotLetter, slotManager.slotHasData(slot));
        
        menu.showMenuAsync(juce::PopupMenu::Options(),
            [this, slot](int result)
            {
                switch (result)
                {
                    case 1: // Save
                        if (onStateChanged)
                            onStateChanged();
                        break;
                    case 2: // Clear
                        slotManager.clearSlot(slot);
                        updateButtonStates();
                        break;
                    case 3: // Rename
                        // Would show rename dialog
                        break;
                }
            });
    }
    
    void showCopyMenu()
    {
        juce::PopupMenu menu;
        
        Slot current = slotManager.getCurrentSlot();
        
        for (int i = 0; i < 4; i++)
        {
            auto slot = static_cast<Slot>(i);
            if (slot != current)
            {
                juce::String label = "Copy to " + SlotManager::getSlotLetter(slot);
                menu.addItem(i + 1, label);
            }
        }
        
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&copyButton),
            [this, current](int result)
            {
                if (result > 0)
                {
                    Slot target = static_cast<Slot>(result - 1);
                    slotManager.copySlot(current, target);
                    updateButtonStates();
                }
            });
    }
    
    void showSwapMenu()
    {
        juce::PopupMenu menu;
        
        Slot current = slotManager.getCurrentSlot();
        
        for (int i = 0; i < 4; i++)
        {
            auto slot = static_cast<Slot>(i);
            if (slot != current && slotManager.slotHasData(slot))
            {
                juce::String label = "Swap with " + SlotManager::getSlotLetter(slot);
                menu.addItem(i + 1, label);
            }
        }
        
        if (menu.getNumItems() == 0)
        {
            menu.addItem(-1, "No other slots have data", false);
        }
        
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&swapButton),
            [this, current](int result)
            {
                if (result > 0)
                {
                    Slot target = static_cast<Slot>(result - 1);
                    slotManager.swapSlots(current, target);
                    updateButtonStates();
                }
            });
    }
    
    SlotManager& slotManager;
    std::vector<std::unique_ptr<SlotButton>> slotButtons;
    juce::TextButton copyButton;
    juce::TextButton swapButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComparePanel)
};

//==============================================================================
/**
 * Mini Compare Strip
 * Compact version for header bar
 */
class MiniCompareStrip : public juce::Component
{
public:
    MiniCompareStrip(SlotManager& manager) : slotManager(manager)
    {
        for (int i = 0; i < 4; i++)
        {
            auto slot = static_cast<Slot>(i);
            auto button = std::make_unique<SlotButton>(slot);
            
            button->onClick = [this](Slot s)
            {
                if (onSlotSelected)
                    onSlotSelected(s);
            };
            
            addAndMakeVisible(*button);
            slotButtons.push_back(std::move(button));
        }
        
        updateStates();
    }
    
    std::function<void(Slot)> onSlotSelected;
    
    void updateStates()
    {
        for (int i = 0; i < 4; i++)
        {
            auto slot = static_cast<Slot>(i);
            slotButtons[i]->setActive(slotManager.getCurrentSlot() == slot);
            slotButtons[i]->setHasData(slotManager.slotHasData(slot));
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        int buttonWidth = bounds.getWidth() / 4;
        
        for (int i = 0; i < 4; i++)
        {
            slotButtons[i]->setBounds(bounds.removeFromLeft(buttonWidth));
        }
    }
    
private:
    SlotManager& slotManager;
    std::vector<std::unique_ptr<SlotButton>> slotButtons;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MiniCompareStrip)
};

} // namespace Compare
