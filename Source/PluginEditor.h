/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DesignSystem.h"
#include "ToneUpperTab.h"
#include "ToneLowerTab.h"
#include "PatchTab.h"
#include "ArpTab.h"
#include "LibrarianTab.h"
#include "SettingsTab.h"
#include "ComparePanel.h"
#include "MorphPanel.h"
#include "RandomPanel.h"
#include "MacroStrip.h"
#include "MacroTab.h"
#include "AutomationPanel.h"
#include "ArpModulationPanel.h"  // Modulation panel (was in ArpTab)
#include "MidiLearnManager.h"
#include "MKS70LookAndFeel.h"
#include "KeyboardShortcutsOverlay.h"
#include "ToastNotification.h"
// New refactored tone panel
#include "TonePanel.h"
#include "HeaderBar.h"
#include "StatusBar.h"
#include "AIAssistant.h"
// MidiTestPanel now included via SettingsTab.h

//==============================================================================
// Animated Vertical Tab Button with breathing glow effect
//==============================================================================
class AnimatedTabButton : public juce::Button, public juce::Timer
{
public:
    AnimatedTabButton(const juce::String& label, const juce::String& iconText, 
                      juce::Colour accentCol, int idx)
        : juce::Button(label), iconLabel(iconText), accentColor(accentCol), tabIndex(idx)
    {
        setClickingTogglesState(true);
        setRadioGroupId(999);
    }
    
    ~AnimatedTabButton() override { stopTimer(); }
    
    void paintButton(juce::Graphics& g, bool isMouseOver, bool /*isMouseDown*/) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Calculate breathing alpha (smooth pulsing)
        float breathAlpha = 0.0f;
        if (isMouseOver && !getToggleState())
        {
            breathAlpha = 0.15f + 0.1f * std::sin(breathPhase);
        }
        
        // Use theme colors for backgrounds
        auto bgColor = DS::Color::bgElevated();
        auto hoverColor = DS::Color::bgHover();
        
        // Background
        if (getToggleState())
        {
            // Active - gradient accent background
            g.setGradientFill(juce::ColourGradient(
                accentColor.withAlpha(0.3f), bounds.getTopLeft(),
                accentColor.withAlpha(0.15f), bounds.getBottomRight(), false));
            g.fillRoundedRectangle(bounds.reduced(2), 10.0f);
            
            // Outer glow effect
            for (float i = 6.0f; i > 0; i -= 2.0f)
            {
                float alpha = (1.0f - i / 6.0f) * 0.1f;
                g.setColour(accentColor.withAlpha(alpha));
                g.fillRoundedRectangle(bounds.expanded(i * 0.3f).reduced(2), 10.0f + i);
            }
            
            // Left accent bar (thicker, full height)
            g.setColour(accentColor);
            g.fillRoundedRectangle(bounds.getX(), bounds.getY() + 4, 4.0f, bounds.getHeight() - 8, 2.0f);
        }
        else if (isMouseOver)
        {
            // Hover with breathing glow
            g.setColour(hoverColor.withAlpha(0.8f + breathAlpha * 0.5f));
            g.fillRoundedRectangle(bounds.reduced(2), 10.0f);
            
            // Pulsing border
            g.setColour(accentColor.withAlpha(0.3f + breathAlpha));
            g.drawRoundedRectangle(bounds.reduced(2), 10.0f, 1.5f);
        }
        else
        {
            // Inactive - subtle background
            g.setColour(bgColor.withAlpha(0.4f));
            g.fillRoundedRectangle(bounds.reduced(2), 10.0f);
        }
        
        // Icon area (top 60%)
        auto iconBounds = bounds;
        iconBounds.removeFromBottom(bounds.getHeight() * 0.35f);
        g.setFont(juce::FontOptions(18.0f).withStyle("Bold"));
        g.setColour(getToggleState() ? accentColor : 
                    (isMouseOver ? DS::Color::textPrimary() : DS::Color::textSecondary()));
        g.drawText(iconLabel, iconBounds.reduced(4), juce::Justification::centred, false);
        
        // Label at bottom (small caps style)
        auto labelBounds = bounds.removeFromBottom(bounds.getHeight() * 0.35f);
        g.setFont(juce::FontOptions(8.0f).withStyle("Bold"));
        g.setColour(getToggleState() ? accentColor.brighter(0.2f) : 
                    (isMouseOver ? DS::Color::textSecondary() : DS::Color::textMuted()));
        g.drawText(getName(), labelBounds.reduced(2), juce::Justification::centred, false);
    }
    
    void mouseEnter(const juce::MouseEvent& e) override
    {
        juce::Button::mouseEnter(e);
        startTimer(25); // Smooth animation
    }
    
    void mouseExit(const juce::MouseEvent& e) override
    {
        juce::Button::mouseExit(e);
        stopTimer();
        breathPhase = 0.0f;
        repaint();
    }
    
    void timerCallback() override
    {
        breathPhase += 0.12f;
        if (breathPhase > juce::MathConstants<float>::twoPi)
            breathPhase -= juce::MathConstants<float>::twoPi;
        repaint();
    }
    
    int getTabIndex() const { return tabIndex; }
    
private:
    juce::String iconLabel;
    juce::Colour accentColor;
    int tabIndex;
    float breathPhase = 0.0f;
};

//==============================================================================
/**
*/
class MKS70EditorAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MKS70EditorAudioProcessorEditor (MKS70EditorAudioProcessor&);
    ~MKS70EditorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;
    
    // Public refresh method for callbacks
    void refreshUI();
    void refreshAllPanels();  // Full refresh after Total Recall
    
    // Show/hide compare panel
    void showComparePanel();
    void hideComparePanel();
    
    // Show/hide morph panel
    void showMorphPanel();
    void hideMorphPanel();
    
    // Show/hide random panel
    void showRandomPanel();
    void hideRandomPanel();
    
    // Get compare mode status
    bool isCompareModeActive() const { return comparePanelVisible; }
    CompareManager::Slot getActiveSlot() const;
    
    // Update compare mode indicator
    void updateCompareIndicator();
    
    // Get processor for compare mode checks
    MKS70EditorAudioProcessor& getProcessor() { return audioProcessor; }

    // MIDI Learn manager access
    MidiLearnManager& getMidiLearnManager() { return midiLearnManager; }
    
    // Helper to map parameter names to SysEx addresses for MIDI Learn
    int getSysExAddressFromParamId(const juce::String& paramId);
    void applyAutomationValue(const juce::String& paramId, int value);
    
    // Header bar collapse toggle
    void toggleHeaderBar();
    bool isHeaderBarVisible() const { return headerBarVisible; }
    
    // Status bar access
    StatusBar& getStatusBar() { return statusBar; }
    
    // Theme management
    void applyTheme(int themeId);
    
    // Undo/Redo helpers
    void updateUndoRedoButtons();
    
    // Vertical tabs layout helpers
    void setupVerticalTabs();
    void setupBottomToolbar();
    void setupMiniPresetBar();
    void layoutVerticalTabs(juce::Rectangle<int> area);
    void layoutUnifiedToolbar(juce::Rectangle<int> area);  // New unified toolbar
    void layoutBottomToolbar(juce::Rectangle<int> area);
    // ...right toolbar removed...
    void layoutMiniPresetBar(juce::Rectangle<int> area);
    void selectTab(int index);
    
    // Preset navigation
    void loadNextPreset();
    void loadPrevPreset();
    void updatePresetDisplay();
    void refreshPresetList();
    int currentPresetIndex = 0;
    juce::Array<juce::File> presetFiles;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MKS70EditorAudioProcessor& audioProcessor;

  // Global LookAndFeel instance for the editor tree
  MKS70LookAndFeel appLookAndFeel;
  
    // Header bar
    HeaderBar headerBar;
    ExpandBar expandBar;  // Shown when header is collapsed
    bool headerBarVisible = true;
    
    // Mini Preset Bar (top)
    juce::Label miniPresetLabel;
    juce::TextButton miniPrevBtn, miniNextBtn;
    juce::TextButton miniMenuBtn;
    juce::ComboBox miniCategoryCombo;
    juce::ComboBox firmwareCombo;  // Firmware version selector in unified toolbar
    
    // Toolbar
    juce::Component toolbar;
    juce::TextButton compareButton;
    juce::TextButton morphButton;
    juce::TextButton randomButton;
    juce::Label compareIndicator;
    
    // Tab component with all editor tabs - VERTICAL on left
    juce::TabbedComponent tabbedComponent;
    
    // Vertical tab buttons (animated with glow)
    std::vector<std::unique_ptr<AnimatedTabButton>> verticalTabButtons;
    int currentTabIndex = 0;
    int previousTabIndex = 0;  // Track previous tab for settings toggle
    
    // Bottom toolbar components
    juce::Label bottomLogoLabel;
    juce::TextButton bottomPrevBtn, bottomNextBtn;
    juce::Label bottomPatchName;
    juce::Label bottomMidiLED;  // MIDI activity LED
    juce::TextButton bottomUndoBtn, bottomRedoBtn;  // Undo/Redo buttons
    juce::TextButton bottomCompareBtn, bottomSettingsBtn;
    
    // Morph slider in toolbar (Upper/Lower balance) - with percentage display
    juce::Slider morphSlider;
    juce::Label morphLabel;        // Center: percentage display
    juce::Label morphLabelLeft;    // Left: "UPPER" label
    juce::Label morphLabelRight;   // Right: "LOWER" label

    // Zoom control
    juce::ComboBox zoomCombo;
    float zoomFactor = 1.0f;

    void applyZoom();
    void layoutScrollableTabs();
    
    // Scrollable tab pages (keep tab content scrollable, tabs pinned)
    struct ScrollPage : public juce::Component {
      juce::Viewport viewport;
      juce::Component container;
      ScrollPage() {
        addAndMakeVisible(viewport);
        viewport.setScrollBarsShown(true, false); // vertical only
        viewport.setViewedComponent(&container, false);
      }
      void resized() override {
        viewport.setBounds(getLocalBounds());
      }
    };

    ScrollPage toneUpperPage;
    ScrollPage toneLowerPage;
    ScrollPage patchPage;
    ScrollPage arpPage;
    ScrollPage librarianPage;
    ScrollPage automationPage;
    ScrollPage modulationPage;  // Was automationPage - now hosts modulation panel
    ScrollPage macroPage;
    ScrollPage settingsPage;
    ScrollPage aiAssistantPage;
    // midiTestPage moved to SettingsTab

    // Actual content components hosted inside the scroll pages
    ToneUpperTab toneUpperTab;
    ToneLowerTab toneLowerTab;
    // Refactored UI panels (visual only for now)
    TonePanel toneUpperPanel { true };
    TonePanel toneLowerPanel { false };
    PatchTab patchTab;
    ArpTab arpTab;
    LibrarianTab librarianTab;
    AutomationPanel automationPanel;
    SettingsTab settingsTab;
    ArpModulationPanel modulationPanel;  // Was AutomationPanel - now hosts modulation
    MacroTab macroTab;
    AIAssistantPanel aiAssistantPanel;
    // MidiTestPanel moved to SettingsTab
    
    // Compare panel
    ComparePanel comparePanel;
    bool comparePanelVisible = false;
    
    // Morph panel
    MorphPanel morphPanel;
    bool morphPanelVisible = false;
    
    // Random panel
    RandomPanel randomPanel;
    bool randomPanelVisible = false;
    
    // Macro controls
    MacroManager macroManager;
    MacroStrip macroStrip;

    // MIDI Learn
    MidiLearnManager midiLearnManager;
    
    // Status bar
    StatusBar statusBar;
    
    // Keyboard shortcuts overlay
    KeyboardShortcutsOverlay shortcutsOverlay;
    bool shortcutsOverlayVisible = false;
    
public:
    // Public accessors for MIDI Learn and Macro functionality
    MacroManager& getMacroManager() { return macroManager; }
    
private:
    // Helper methods
    void setupToolbar();
    void showMacroSetupDialog(int macroIndex);
    void layoutScrollableTab(ScrollPage& page, juce::Component& content, int minHeight);
    // Content will expand to fill available space - these are minimum heights
    int getUpperBaseHeight() const { return 0; }  // Use available height
    int getLowerBaseHeight() const { return 0; }
    int getPatchBaseHeight() const { return 0; }
    int getArpBaseHeight() const { return 0; }
    int getLibrarianBaseHeight() const { return 0; }
    int getModulationBaseHeight() const { return 700; }  // Height for modulation panel
    int getMacroBaseHeight() const { return 0; }
    int getSettingsBaseHeight() const { return 0; }
    int getAiAssistantBaseHeight() const { return 0; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MKS70EditorAudioProcessorEditor)
};
