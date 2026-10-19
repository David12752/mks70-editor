/*
  ==============================================================================

    ComparePanel.h
    Compare Mode panel for A/B comparison of patches

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompareManager.h"

// Forward declaration
class MKS70EditorAudioProcessor;

//==============================================================================
/**
 * ComparePanel - UI for comparing two patch slots (A/B)
 * Shows differences between patches and allows switching between them
 */
class ComparePanel : public juce::Component,
                     public juce::Button::Listener,
                     public juce::ComboBox::Listener,
                     public juce::KeyListener
{
public:
    ComparePanel();
    ~ComparePanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Processor integration
    void setProcessor(MKS70EditorAudioProcessor* proc) { processor = proc; }
    
    // Store current state to slots
    void storeToSlotA();
    void storeToSlotB();
    
    // Recall slots to current state
    void recallSlotA();
    void recallSlotB();
    
    // Swap A/B slots
    void swapSlots();
    
    // Revert to original (stored when entering compare mode)
    void revertToOriginal();
    
    // Update the differences display
    void refreshDifferences();
    
    // Set active slot (highlights the button)
    void setActiveSlot(CompareManager::Slot slot);
    
    // Update slot names from manager
    void updateSlotNames();
    
    // Button listener
    void buttonClicked(juce::Button* button) override;
    
    // ComboBox listener
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    
    // KeyListener
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;

private:
    MKS70EditorAudioProcessor* processor = nullptr;
    CompareManager::Slot activeSlot = CompareManager::Slot::SLOT_A;
    
    // UI Components
    juce::Label titleLabel;
    juce::TextButton closeButton;
    
    // Slot panels
    juce::GroupComponent slotAGroup;
    juce::GroupComponent slotBGroup;
    juce::Label slotANameLabel;
    juce::Label slotBNameLabel;
    juce::TextButton slotAActiveButton;
    juce::TextButton slotBActiveButton;
    
    // Swap button
    juce::TextButton swapButton;
    
    // Differences table
    juce::Label differencesLabel;
    juce::Viewport differencesViewport;
    juce::Component differencesContent;
    
    // Filter controls
    juce::Label filterLabel;
    juce::ToggleButton showAllToggle;
    juce::ToggleButton showChangedOnlyToggle;
    juce::ComboBox sectionFilterCombo;
    
    // Action buttons
    juce::TextButton storeAButton;
    juce::TextButton storeBButton;
    juce::TextButton recallAButton;
    juce::TextButton recallBButton;
    juce::TextButton revertButton;
    
    // Differences data
    std::vector<ParameterDiff> differences;
    bool showChangedOnly = false;
    juce::String filterSection = "All";
    
    // Setup methods
    void setupUI();
    void setupSlotPanels();
    void setupDifferencesTable();
    void setupFilterControls();
    void setupActionButtons();
    
    // Helper methods
    void drawDifferencesTable(juce::Graphics& g);
    std::vector<ParameterDiff> getFilteredDifferences();
    juce::Colour getDifferenceColour(int diffValue);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComparePanel)
};
