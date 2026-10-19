/*
  ==============================================================================

    MorphPanel.h
    Morph/Crossfade panel for smooth patch interpolation

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MorphEngine.h"
#include "CompareManager.h"
#include "MorphSnapshotManager.h"

// Forward declaration
class MKS70EditorAudioProcessor;

//==============================================================================
/**
 * SnapshotsListModel - ListBox model for displaying snapshots
 */
class SnapshotsListModel : public juce::ListBoxModel
{
public:
    SnapshotsListModel(MorphSnapshotManager& manager) : snapshotManager(manager) {}
    
    int getNumRows() override
    {
        return snapshotManager.getSnapshotCount();
    }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll(juce::Colour(0xff4080ff));
        else
            g.fillAll(juce::Colour(0xff2a2a2a));
        
        const auto* snapshot = snapshotManager.getSnapshot(rowNumber);
        if (snapshot)
        {
            g.setColour(juce::Colours::white);
            g.setFont(14.0f);
            
            juce::String text = juce::String(rowNumber + 1) + ". " + snapshot->name + 
                               " (" + juce::String(snapshot->position * 100.0f, 0) + "%)";
            
            g.drawText(text, 5, 0, width - 10, height, juce::Justification::centredLeft, true);
        }
    }
    
private:
    MorphSnapshotManager& snapshotManager;
};

//==============================================================================
/**
 * MorphPanel - UI for morphing/crossfading between two patches
 * Provides real-time interpolation with animation and curve control
 */
class MorphPanel : public juce::Component,
                   public juce::Slider::Listener,
                   public juce::Button::Listener,
                   public juce::ComboBox::Listener,
                   public MorphEngine::Listener
{
public:
    MorphPanel();
    ~MorphPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Processor integration
    void setProcessor(MKS70EditorAudioProcessor* proc);
    
    // Load patches to morph sources
    void loadSourceA();
    void loadSourceB();
    void loadCurrentToA();
    void loadCurrentToB();
    
    // Apply morphed patch to processor
    void applyMorphedPatch();
    
    // Snapshot management
    void captureCurrentSnapshot();
    void recallSnapshot(int index);
    void deleteSnapshot(int index);
    void exportSnapshots();
    void refreshSnapshotsList();
    
    // Slider listener
    void sliderValueChanged(juce::Slider* slider) override;
    
    // Button listener
    void buttonClicked(juce::Button* button) override;
    
    // ComboBox listener
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    
    // MorphEngine listener
    void morphPositionChanged(float newPosition) override;
    void morphCompleted() override;

private:
    MKS70EditorAudioProcessor* processor = nullptr;
    MorphEngine morphEngine;
    MorphSnapshotManager snapshotManager;
    
    // Source patch info
    juce::String sourceAName;
    juce::String sourceBName;
    bool hasSourceA = false;
    bool hasSourceB = false;
    
    // UI Components
    
    // Header
    juce::Label titleLabel;
    juce::TextButton closeButton;
    
    // Source panels
    juce::GroupComponent sourceAGroup;
    juce::GroupComponent sourceBGroup;
    juce::Label sourceALabel;
    juce::Label sourceBLabel;
    juce::TextButton loadAButton;
    juce::TextButton loadBButton;
    juce::TextButton currentToAButton;
    juce::TextButton currentToBButton;
    
    // Main morph slider
    juce::Slider morphSlider;
    juce::Label morphPositionLabel;
    juce::Label sourceAIndicator;
    juce::Label sourceBIndicator;
    
    // MIDI Control
    juce::GroupComponent midiGroup;
    juce::Label midiCCLabel;
    juce::ComboBox midiCCCombo;
    juce::ToggleButton linkToModWheelToggle;
    juce::ToggleButton midiLearnToggle;
    
    // Auto Morph
    juce::GroupComponent autoMorphGroup;
    juce::Label autoMorphModeLabel;
    juce::ComboBox autoMorphModeCombo;
    juce::Label durationLabel;
    juce::Slider durationSlider;
    juce::Label durationValueLabel;
    juce::TextButton startAutoMorphButton;
    juce::TextButton stopAutoMorphButton;
    juce::ToggleButton oneShotToggle;
    juce::ToggleButton loopToggle;
    juce::ToggleButton pingPongToggle;
    
    // Parameter Exclusions
    juce::GroupComponent exclusionsGroup;
    juce::Label exclusionsLabel;
    juce::Viewport exclusionsViewport;
    juce::Component exclusionsContent;
    std::vector<std::unique_ptr<juce::ToggleButton>> exclusionToggles;
    
    // Morph Curves
    juce::GroupComponent curvesGroup;
    juce::Label curvesLabel;
    juce::Label dcoSectionLabel;
    juce::ComboBox dcoSectionCurveCombo;
    juce::Label vcfSectionLabel;
    juce::ComboBox vcfSectionCurveCombo;
    juce::Label envSectionLabel;
    juce::ComboBox envSectionCurveCombo;
    juce::Label vcaSectionLabel;
    juce::ComboBox vcaSectionCurveCombo;
    juce::Label lfoSectionLabel;
    juce::ComboBox lfoSectionCurveCombo;
    juce::Label mixerSectionLabel;
    juce::ComboBox mixerSectionCurveCombo;
    
    // Snapshots section
    juce::GroupComponent snapshotsGroup;
    juce::Label snapshotsLabel;
    juce::ListBox snapshotsList;
    juce::TextButton captureSnapshotButton;
    juce::TextButton recallSnapshotButton;
    juce::TextButton deleteSnapshotButton;
    juce::TextButton exportSnapshotsButton;
    juce::TextButton clearSnapshotsButton;
    juce::ComboBox sortSnapshotsCombo;
    std::unique_ptr<SnapshotsListModel> snapshotsListModel;
    
    // Setup methods
    void setupUI();
    void setupSourcePanels();
    void setupMorphSlider();
    void setupMidiControl();
    void setupAutoMorph();
    void setupExclusions();
    void setupCurves();
    void setupSnapshots();
    
    // Helper methods
    void updateMorphPosition(float position);
    void updateSourceLabels();
    void populateCurveCombo(juce::ComboBox& combo);
    void populateModeCombo();
    void createExclusionToggles();
    MorphEngine::MorphCurve getCurveFromCombo(juce::ComboBox& combo);
    void applySectionCurve(const juce::String& sectionPrefix, MorphEngine::MorphCurve curve);
    juce::String formatDuration(float seconds);
    
    // Animation state
    bool autoMorphActive = false;
    MorphEngine::MorphMode currentAutoMorphMode = MorphEngine::LINEAR;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorphPanel)
};
