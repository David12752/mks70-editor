/*
  ==============================================================================

    ArpTab.h
    Component for Arpeggiator parameters - Using new ArpPanel design
    Modulation panel moved to separate tab

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ArpParameters.h"
#include "ArpPanel.h"
#include "DesignSystem.h"

// Forward declaration
class MKS70EditorAudioProcessor;

//==============================================================================
class ArpTab : public juce::Component
{
public:
    ArpTab();
    ~ArpTab() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Get/Set parameters
    ArpParameters getParameters() const { return parameters; }
    void setParameters(const ArpParameters& params);
    
    // Processor integration
    void setProcessor(MKS70EditorAudioProcessor* proc) { processor = proc; }
    void refreshFromProcessor();
    
    // Tap Tempo
    void handleTapTempo();
    
    // Copy/Paste
    void copyToClipboard();
    void pasteFromClipboard();
    
    // Get panel for direct access
    ArpPanel& getPanel() { return arpPanel; }

private:
    ArpParameters parameters;
    MKS70EditorAudioProcessor* processor = nullptr;
    
    // Arpeggiator panel - no longer using tabbed component
    ArpPanel arpPanel;
    
    // Tap tempo tracking
    std::vector<double> tapTimes;
    static constexpr int maxTaps = 4;
    static constexpr double tapTimeout = 2.0;  // seconds
    
    // Clipboard for copy/paste
    static ArpParameters clipboard;
    static bool clipboardValid;
    
    void syncPanelToParameters();
    void syncParametersFromPanel();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArpTab)
};
