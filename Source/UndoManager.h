/*
  ==============================================================================

    UndoManager.h
    Undo/Redo system for MKS-70 Editor
    
    Features:
    - Ctrl+Z / Ctrl+Y support
    - History panel with all actions
    - Jump to any point in history
    - Auto-save snapshots

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToneParameters.h"
#include "PatchParameters.h"
#include "ArpParameters.h"

//==============================================================================
/**
    UndoableAction - Base class for all undoable actions
*/
class UndoableAction
{
public:
    virtual ~UndoableAction() = default;
    
    virtual void perform() = 0;
    virtual void undo() = 0;
    
    virtual juce::String getDescription() const = 0;
    virtual juce::String getCategory() const { return "General"; }
    
    juce::Time getTimestamp() const { return timestamp; }
    
protected:
    juce::Time timestamp = juce::Time::getCurrentTime();
};

//==============================================================================
/**
    ParameterChangeAction - Undoable parameter change
*/
class ParameterChangeAction : public UndoableAction
{
public:
    ParameterChangeAction(const juce::String& paramName, 
                          const juce::String& section,
                          int oldValue, 
                          int newValue,
                          std::function<void(int)> applyFunc)
        : parameterName(paramName)
        , sectionName(section)
        , previousValue(oldValue)
        , currentValue(newValue)
        , applyFunction(applyFunc)
    {
    }
    
    void perform() override
    {
        if (applyFunction)
            applyFunction(currentValue);
    }
    
    void undo() override
    {
        if (applyFunction)
            applyFunction(previousValue);
    }
    
    juce::String getDescription() const override
    {
        return parameterName + ": " + juce::String(previousValue) + " → " + juce::String(currentValue);
    }
    
    juce::String getCategory() const override { return sectionName; }
    
private:
    juce::String parameterName;
    juce::String sectionName;
    int previousValue;
    int currentValue;
    std::function<void(int)> applyFunction;
};

//==============================================================================
/**
    ToneSnapshotAction - Undoable full tone change
*/
class ToneSnapshotAction : public UndoableAction
{
public:
    ToneSnapshotAction(const juce::String& description,
                       bool isUpper,
                       const ToneParams& oldTone,
                       const ToneParams& newTone,
                       std::function<void(const ToneParams&)> applyFunc)
        : desc(description)
        , upper(isUpper)
        , previousTone(oldTone)
        , currentTone(newTone)
        , applyFunction(applyFunc)
    {
    }
    
    void perform() override
    {
        if (applyFunction)
            applyFunction(currentTone);
    }
    
    void undo() override
    {
        if (applyFunction)
            applyFunction(previousTone);
    }
    
    juce::String getDescription() const override { return desc; }
    juce::String getCategory() const override { return upper ? "Tone Upper" : "Tone Lower"; }
    
private:
    juce::String desc;
    bool upper;
    ToneParams previousTone;
    ToneParams currentTone;
    std::function<void(const ToneParams&)> applyFunction;
};

//==============================================================================
/**
    MKS70UndoManager - Main undo/redo manager
*/
class MKS70UndoManager
{
public:
    MKS70UndoManager(int maxHistorySize = 100)
        : maxHistory(maxHistorySize)
    {
    }
    
    //==========================================================================
    // Core operations
    
    void performAction(std::unique_ptr<UndoableAction> action)
    {
        // Clear redo stack when new action is performed
        redoStack.clear();
        
        // Perform the action
        action->perform();
        
        // Add to undo stack
        undoStack.push_back(std::move(action));
        
        // Trim history if needed
        while (undoStack.size() > static_cast<size_t>(maxHistory))
            undoStack.erase(undoStack.begin());
        
        // Notify listeners
        if (onHistoryChanged)
            onHistoryChanged();
    }

    // Convenience helpers to create and perform common actions
    void performParameterChange(const juce::String& paramName,
                                const juce::String& section,
                                int oldValue,
                                int newValue,
                                std::function<void(int)> applyFunc)
    {
        performAction(std::make_unique<ParameterChangeAction>(paramName, section, oldValue, newValue, applyFunc));
    }

    void performToneSnapshot(const juce::String& description,
                              bool isUpper,
                              const ToneParams& oldTone,
                              const ToneParams& newTone,
                              std::function<void(const ToneParams&)> applyFunc)
    {
        performAction(std::make_unique<ToneSnapshotAction>(description, isUpper, oldTone, newTone, applyFunc));
    }
    
    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }
    
    void undo()
    {
        if (!canUndo()) return;
        
        auto action = std::move(undoStack.back());
        undoStack.pop_back();
        
        action->undo();
        
        redoStack.push_back(std::move(action));
        
        if (onHistoryChanged)
            onHistoryChanged();
    }
    
    void redo()
    {
        if (!canRedo()) return;
        
        auto action = std::move(redoStack.back());
        redoStack.pop_back();
        
        action->perform();
        
        undoStack.push_back(std::move(action));
        
        if (onHistoryChanged)
            onHistoryChanged();
    }
    
    //==========================================================================
    // History access
    
    int getUndoCount() const { return static_cast<int>(undoStack.size()); }
    int getRedoCount() const { return static_cast<int>(redoStack.size()); }
    
    juce::String getUndoDescription() const
    {
        return canUndo() ? undoStack.back()->getDescription() : "";
    }
    
    juce::String getRedoDescription() const
    {
        return canRedo() ? redoStack.back()->getDescription() : "";
    }
    
    // Get history for display (newest first)
    std::vector<std::pair<juce::String, juce::String>> getUndoHistory() const
    {
        std::vector<std::pair<juce::String, juce::String>> history;
        for (auto it = undoStack.rbegin(); it != undoStack.rend(); ++it)
        {
            history.push_back({(*it)->getDescription(), (*it)->getCategory()});
        }
        return history;
    }
    
    // Jump to specific point in history
    void jumpToHistoryPoint(int index)
    {
        // Index 0 = current state, index 1 = one undo back, etc.
        int undosNeeded = index;
        while (undosNeeded > 0 && canUndo())
        {
            undo();
            undosNeeded--;
        }
    }
    
    void clearHistory()
    {
        undoStack.clear();
        redoStack.clear();
        
        if (onHistoryChanged)
            onHistoryChanged();
    }
    
    //==========================================================================
    // Callbacks
    
    std::function<void()> onHistoryChanged;
    
private:
    std::vector<std::unique_ptr<UndoableAction>> undoStack;
    std::vector<std::unique_ptr<UndoableAction>> redoStack;
    int maxHistory;
};

//==============================================================================
/**
    UndoHistoryPanel - Visual panel showing undo history
*/
class UndoHistoryPanel : public juce::Component,
                         public juce::ListBoxModel
{
public:
    UndoHistoryPanel(MKS70UndoManager& manager)
        : undoManager(manager)
    {
        titleLabel.setText("History", juce::dontSendNotification);
        titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFF8FAFC));
        addAndMakeVisible(titleLabel);
        
        historyList.setModel(this);
        historyList.setRowHeight(28);
        historyList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xFF1A1A1F));
        historyList.setColour(juce::ListBox::outlineColourId, juce::Colour(0xFF2D2D3A));
        addAndMakeVisible(historyList);
        
        undoButton.setButtonText("Undo");
        undoButton.onClick = [this]() { undoManager.undo(); refresh(); };
        addAndMakeVisible(undoButton);
        
        redoButton.setButtonText("Redo");
        redoButton.onClick = [this]() { undoManager.redo(); refresh(); };
        addAndMakeVisible(redoButton);
        
        clearButton.setButtonText("Clear");
        clearButton.onClick = [this]() { undoManager.clearHistory(); refresh(); };
        addAndMakeVisible(clearButton);
        
        // Subscribe to history changes
        undoManager.onHistoryChanged = [this]() { refresh(); };
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xFF0D0D0F));
        
        // Border
        g.setColour(juce::Colour(0xFF2D2D3A));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 8.0f, 1.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(12);
        
        titleLabel.setBounds(bounds.removeFromTop(30));
        bounds.removeFromTop(8);
        
        auto buttonRow = bounds.removeFromBottom(36);
        undoButton.setBounds(buttonRow.removeFromLeft(70));
        buttonRow.removeFromLeft(8);
        redoButton.setBounds(buttonRow.removeFromLeft(70));
        buttonRow.removeFromLeft(8);
        clearButton.setBounds(buttonRow.removeFromLeft(70));
        
        bounds.removeFromBottom(8);
        historyList.setBounds(bounds);
    }
    
    // ListBoxModel
    int getNumRows() override
    {
        return static_cast<int>(historyCache.size()) + 1; // +1 for current state
    }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g,
                          int width, int height, bool rowIsSelected) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height);
        
        // Background
        if (rowIsSelected)
            g.fillAll(juce::Colour(0xFF6366F1).withAlpha(0.3f));
        else if (rowNumber % 2 == 0)
            g.fillAll(juce::Colour(0xFF1A1A1F));
        else
            g.fillAll(juce::Colour(0xFF15151A));
        
        bounds.reduce(8, 4);
        
        if (rowNumber == 0)
        {
            // Current state
            g.setColour(juce::Colour(0xFF22D3EE));
            g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
            g.drawText("● Current State", bounds, juce::Justification::centredLeft);
        }
        else if (rowNumber - 1 < static_cast<int>(historyCache.size()))
        {
            auto& item = historyCache[rowNumber - 1];
            
            // Category badge
            g.setColour(juce::Colour(0xFF64748B));
            g.setFont(juce::FontOptions(10.0f));
            auto categoryBounds = bounds.removeFromLeft(70);
            g.drawText(item.second, categoryBounds, juce::Justification::centredLeft);
            
            // Description
            g.setColour(juce::Colour(0xFFF8FAFC));
            g.setFont(juce::FontOptions(12.0f));
            g.drawText(item.first, bounds, juce::Justification::centredLeft);
        }
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent&) override
    {
        if (row > 0)
        {
            undoManager.jumpToHistoryPoint(row);
            refresh();
        }
    }
    
    void refresh()
    {
        historyCache = undoManager.getUndoHistory();
        historyList.updateContent();
        historyList.repaint();
        
        undoButton.setEnabled(undoManager.canUndo());
        redoButton.setEnabled(undoManager.canRedo());
    }
    
private:
    MKS70UndoManager& undoManager;
    
    juce::Label titleLabel;
    juce::ListBox historyList;
    juce::TextButton undoButton, redoButton, clearButton;
    
    std::vector<std::pair<juce::String, juce::String>> historyCache;
};
