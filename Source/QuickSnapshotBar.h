/*
  ==============================================================================
    QuickSnapshotBar.h
    8 instant save/load slots for quick sound snapshots
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
 * QuickSnapshotSlot - Single snapshot slot button with visual feedback
 */
class QuickSnapshotSlot : public juce::Component
{
public:
    QuickSnapshotSlot(int slotNumber)
        : number(slotNumber)
    {
        setTooltip("Click: Load | Shift+Click: Save | Ctrl+Click: Clear");
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        
        // Background
        juce::Colour bgColor;
        if (isMouseOver())
            bgColor = DS::Color::bgHover();
        else if (hasData)
            bgColor = DS::Color::Accent::primary().withAlpha(0.3f);
        else
            bgColor = DS::Color::bgPanel();
            
        g.setColour(bgColor);
        g.fillRoundedRectangle(bounds, DS::Dim::cornerRadius());
        
        // Border
        g.setColour(hasData ? DS::Color::Accent::primary() : DS::Color::border());
        g.drawRoundedRectangle(bounds, DS::Dim::cornerRadius(), 1.0f);
        
        // Number
        g.setColour(hasData ? DS::Color::textPrimary() : DS::Color::textMuted());
        g.setFont(DS::Font::label().withStyle("Bold"));
        g.drawText(juce::String(number + 1), bounds, juce::Justification::centred);
        
        // Filled indicator
        if (hasData)
        {
            g.setColour(DS::Color::Accent::primary());
            g.fillEllipse(bounds.getRight() - 8, bounds.getY() + 2, 5, 5);
        }
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isShiftDown())
        {
            // Save
            if (onSave) onSave(number);
        }
        else if (e.mods.isCtrlDown())
        {
            // Clear
            if (onClear) onClear(number);
        }
        else
        {
            // Load
            if (hasData && onLoad) onLoad(number);
        }
    }
    
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
    
    void setHasData(bool has) 
    { 
        hasData = has; 
        repaint();
    }
    
    void setSnapshotName(const juce::String& name)
    {
        snapshotName = name;
        setTooltip(name.isEmpty() ? "Click: Load | Shift+Click: Save | Ctrl+Click: Clear"
                                  : name + "\nClick: Load | Shift+Click: Save | Ctrl+Click: Clear");
    }
    
    std::function<void(int)> onSave;
    std::function<void(int)> onLoad;
    std::function<void(int)> onClear;
    
private:
    int number;
    bool hasData = false;
    juce::String snapshotName;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuickSnapshotSlot)
};

//==============================================================================
/**
 * QuickSnapshotBar - Bar with 8 quick snapshot slots
 */
class QuickSnapshotBar : public juce::Component
{
public:
    QuickSnapshotBar(std::function<juce::var()> getStateFunc,
                     std::function<void(const juce::var&)> setStateFunc)
        : getCurrentState(getStateFunc),
          applyState(setStateFunc)
    {
        // Create 8 slots
        for (int i = 0; i < 8; ++i)
        {
            auto* slot = new QuickSnapshotSlot(i);
            slot->onSave = [this](int idx) { saveToSlot(idx); };
            slot->onLoad = [this](int idx) { loadFromSlot(idx); };
            slot->onClear = [this](int idx) { clearSlot(idx); };
            slots.add(slot);
            addAndMakeVisible(slot);
        }
        
        // Label
        titleLabel.setText("Quick Snapshots", juce::dontSendNotification);
        titleLabel.setFont(DS::Font::tiny());
        titleLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        addAndMakeVisible(titleLabel);
        
        // Load saved snapshots
        loadSnapshotsFromDisk();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bgPanel().darker(0.1f));
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(4);
        titleLabel.setBounds(bounds.removeFromLeft(80));
        
        int slotWidth = (bounds.getWidth() - 14) / 8;  // 8 slots with 2px gaps
        
        for (int i = 0; i < 8; ++i)
        {
            slots[i]->setBounds(bounds.removeFromLeft(slotWidth));
            bounds.removeFromLeft(2);
        }
    }
    
    //==========================================================================
    // Snapshot Operations
    //==========================================================================
    
    void saveToSlot(int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= 8) return;
        if (!getCurrentState) return;
        
        snapshots[slotIndex] = getCurrentState();
        snapshotNames[slotIndex] = "Snapshot " + juce::String(slotIndex + 1) 
                                 + " (" + juce::Time::getCurrentTime().toString(true, true, false) + ")";
        
        slots[slotIndex]->setHasData(true);
        slots[slotIndex]->setSnapshotName(snapshotNames[slotIndex]);
        
        saveSnapshotsToDisk();
        
        if (onStatusMessage)
            onStatusMessage("Saved to Snapshot " + juce::String(slotIndex + 1));
    }
    
    void loadFromSlot(int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= 8) return;
        if (!snapshots[slotIndex].isObject()) return;
        if (!applyState) return;
        
        applyState(snapshots[slotIndex]);
        
        if (onStatusMessage)
            onStatusMessage("Loaded Snapshot " + juce::String(slotIndex + 1));
    }
    
    void clearSlot(int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= 8) return;
        
        snapshots[slotIndex] = juce::var();
        snapshotNames[slotIndex] = "";
        
        slots[slotIndex]->setHasData(false);
        slots[slotIndex]->setSnapshotName("");
        
        saveSnapshotsToDisk();
        
        if (onStatusMessage)
            onStatusMessage("Cleared Snapshot " + juce::String(slotIndex + 1));
    }
    
    void clearAllSlots()
    {
        for (int i = 0; i < 8; ++i)
            clearSlot(i);
    }
    
    //==========================================================================
    // Persistence
    //==========================================================================
    
    void saveSnapshotsToDisk()
    {
        auto file = getSnapshotsFile();
        
        juce::DynamicObject::Ptr root = new juce::DynamicObject();
        
        for (int i = 0; i < 8; ++i)
        {
            if (snapshots[i].isObject())
            {
                juce::DynamicObject::Ptr slotObj = new juce::DynamicObject();
                slotObj->setProperty("name", snapshotNames[i]);
                slotObj->setProperty("data", snapshots[i]);
                root->setProperty("slot" + juce::String(i), juce::var(slotObj.get()));
            }
        }
        
        file.replaceWithText(juce::JSON::toString(juce::var(root.get())));
    }
    
    void loadSnapshotsFromDisk()
    {
        auto file = getSnapshotsFile();
        if (!file.existsAsFile()) return;
        
        auto json = juce::JSON::parse(file.loadFileAsString());
        if (!json.isObject()) return;
        
        for (int i = 0; i < 8; ++i)
        {
            auto slotVar = json.getProperty("slot" + juce::String(i), juce::var());
            if (slotVar.isObject())
            {
                snapshotNames[i] = slotVar.getProperty("name", "").toString();
                snapshots[i] = slotVar.getProperty("data", juce::var());
                
                slots[i]->setHasData(snapshots[i].isObject());
                slots[i]->setSnapshotName(snapshotNames[i]);
            }
        }
    }
    
    std::function<void(const juce::String&)> onStatusMessage;
    
private:
    juce::File getSnapshotsFile()
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("MKS70Editor");
        dir.createDirectory();
        return dir.getChildFile("quick_snapshots.json");
    }
    
    std::function<juce::var()> getCurrentState;
    std::function<void(const juce::var&)> applyState;
    
    juce::Label titleLabel;
    juce::OwnedArray<QuickSnapshotSlot> slots;
    
    juce::var snapshots[8];
    juce::String snapshotNames[8];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuickSnapshotBar)
};
