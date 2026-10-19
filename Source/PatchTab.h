/*
  ==============================================================================

    PatchTab.h
    Component for Patch parameters - Using new PatchPanel design

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PatchParameters.h"
#include "PatchPanel.h"

// Forward declaration
class MKS70EditorAudioProcessor;

//==============================================================================
class PatchTab : public juce::Component
{
public:
    PatchTab();
    ~PatchTab() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Get/Set parameters
    PatchParameters getParameters() const { return parameters; }
    void setParameters(const PatchParameters& params);
    
    // Processor integration
    void setProcessor(MKS70EditorAudioProcessor* proc) { processor = proc; }
    void refreshFromProcessor();
    
    // Copy/Paste
    void copyToClipboard();
    void pasteFromClipboard();
    
    // Initialize patch to default
    void initPatch();
    
    // Access to the panel for external connections
    PatchPanel& getPanel() { return panel; }

private:
    PatchParameters parameters;
    MKS70EditorAudioProcessor* processor = nullptr;
    
    // New design panel
    PatchPanel panel;
    juce::Viewport viewport;  // For scrolling
    
    // Clipboard for copy/paste
    static PatchParameters clipboard;
    static bool clipboardValid;
    
    // Sync panel controls to parameters
    void syncPanelToParameters();
    void syncParametersFromPanel();
    
    // Setup MIDI listeners on panel controls
    void setupListeners();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatchTab)
};
