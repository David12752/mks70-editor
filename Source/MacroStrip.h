/*
  ==============================================================================

    MacroStrip.h
    Horizontal strip with 8 macro controls

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MacroControl.h"

//==============================================================================
/**
 * Single macro slider with label and CC display
 */
class MacroSlider : public juce::Component,
                    public juce::Slider::Listener
{
public:
    MacroSlider();
    ~MacroSlider() override;
    
    void setMacroIndex(int index);
    void setMacroControl(MacroControl* macro);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void sliderValueChanged(juce::Slider* slider) override;
    
    // Callback for value changes
    std::function<void(int macroIndex, int value)> onValueChange;
    
    // Callback for setup button
    std::function<void(int macroIndex)> onSetupClick;

private:
    int macroIndex = 0;
    MacroControl* macroControl = nullptr;
    
    juce::Slider slider;
    juce::Label nameLabel;
    juce::Label valueLabel;
    juce::Label ccLabel;
    juce::TextButton setupButton;
    
    void updateLabels();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroSlider)
};

//==============================================================================
/**
 * MacroStrip - Collapsible horizontal strip showing all 8 macros
 */
class MacroStrip : public juce::Component,
                   public MacroManager::Listener
{
public:
    MacroStrip();
    ~MacroStrip() override;
    
    void setMacroManager(MacroManager* manager);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // MacroManager::Listener
    void macroValueChanged(int macroIndex, int newValue) override;
    void macroMappingsChanged(int macroIndex) override;
    
    // Callback for setup dialog
    std::function<void(int macroIndex)> onSetupMacro;
    
    // Collapse/expand
    bool isExpanded() const { return expanded; }
    void setExpanded(bool shouldExpand);
    int getCollapsedHeight() const { return 28; }
    int getExpandedHeight() const { return 100; }
    int getPreferredHeight() const { return expanded ? getExpandedHeight() : getCollapsedHeight(); }
    
    // Callback when expanded state changes
    std::function<void(bool)> onExpandedChanged;

private:
    MacroManager* macroManager = nullptr;
    MacroSlider macroSliders[MacroManager::NUM_MACROS];
    juce::Label titleLabel;
    juce::TextButton helpButton;
    juce::TextButton expandButton;
    bool expanded = false;
    
    void showHelp();
    void toggleExpanded();
    void updateAllSliders();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroStrip)
};
