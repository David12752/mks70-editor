/*
  ==============================================================================

    ToneLowerTab.h
    Component for Lower Tone (Tone B) parameters

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToneParams.h"
#include "InteractiveEnvelopeEditor.h"
#include "WaveformDisplay.h"
#include "MixerWaveformDisplay.h"
#include "ToneClipboard.h"

// Forward declaration
class MKS70EditorAudioProcessor;
class MKS70EditorAudioProcessorEditor;

//==============================================================================
class ToneLowerTab : public juce::Component
{
public:
    ToneLowerTab();
    ~ToneLowerTab() override;

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void resized() override;
    
    // Get/Set parameters
    ToneParams getParameters() const { return parameters; }
    void setParameters(const ToneParams& params);
    
    // Processor integration
    void setProcessor(MKS70EditorAudioProcessor* proc) { processor = proc; }
    void refreshFromProcessor();

    // Editor integration (for MIDI Learn)
    void setEditor(MKS70EditorAudioProcessorEditor* ed) { editor = ed; }

private:
    ToneParams parameters;
    MKS70EditorAudioProcessor* processor = nullptr;
    MKS70EditorAudioProcessorEditor* editor = nullptr;
    
    // Setup/listeners
    void setupComboBoxes();
    void setupSliders();
    void setupLabels();
    void setupButtons();
    void setupListeners();
    void registerMidiForSlider(juce::Slider& slider, const juce::String& paramId);
    void registerMidiForCombo(juce::ComboBox& combo, const juce::String& paramId);
    void showLearnMenuForSlider(juce::Slider* slider);
    void showLearnMenuForCombo(juce::ComboBox* combo);
    void showCopyPasteMenu(const juce::MouseEvent& e);
    void onParameterChanged();
    void updateWaveDisplays();
    
    // Labels
    juce::Label titleLabel;
    
    // ===== DCO-1 Section =====
    juce::Label dco1Label;
    juce::ComboBox dco1RangeCombo;
    juce::Label dco1RangeLabel;
    juce::ComboBox dco1WaveformCombo;
    juce::Label dco1WaveformLabel;
    juce::Slider dco1TuneSlider;
    juce::Label dco1TuneLabel;
    juce::Slider dco1PulseWidthSlider;
    juce::Label dco1PulseWidthLabel;
    juce::Slider dco1LFODepthSlider;
    juce::Label dco1LFODepthLabel;
    juce::Slider dco1ENVDepthSlider;
    juce::Label dco1ENVDepthLabel;
    juce::Slider dco1AfterSensSlider;
    juce::Label dco1AfterSensLabel;
    WaveformDisplay dco1WaveDisplay;
    
    // ===== DCO-2 Section =====
    juce::Label dco2Label;
    juce::ComboBox dco2RangeCombo;
    juce::Label dco2RangeLabel;
    juce::ComboBox dco2WaveformCombo;
    juce::Label dco2WaveformLabel;
    juce::ComboBox dcoCrossModCombo;
    juce::Label dcoCrossModLabel;
    juce::Slider dco2TuneSlider;
    juce::Label dco2TuneLabel;
    juce::Slider dco2FineTuneSlider;
    juce::Label dco2FineTuneLabel;
    juce::Slider dco2PulseWidthSlider;
    juce::Label dco2PulseWidthLabel;
    juce::Slider dco2LFODepthSlider;
    juce::Label dco2LFODepthLabel;
    juce::Slider dco2ENVDepthSlider;
    juce::Label dco2ENVDepthLabel;
    juce::Slider dco2AfterSensSlider;
    juce::Label dco2AfterSensLabel;
    juce::ComboBox dcoDynamicsCombo;
    juce::Label dcoDynamicsLabel;
    juce::ComboBox dcoENVModeCombo;
    juce::Label dcoENVModeLabel;
    WaveformDisplay dco2WaveDisplay;
    
    // ===== Mixer Section =====
    juce::Label mixerLabel;
    juce::Slider mixerDCO1LevelSlider;
    juce::Label mixerDCO1LevelLabel;
    juce::Slider mixerDCO2LevelSlider;
    juce::Label mixerDCO2LevelLabel;
    juce::Slider mixerENVDepthSlider;
    juce::Label mixerENVDepthLabel;
    juce::ComboBox mixerDynamicsCombo;
    juce::Label mixerDynamicsLabel;
    juce::ComboBox mixerENVModeCombo;
    juce::Label mixerENVModeLabel;
    MixerWaveformDisplay mixerWaveDisplay;
    
    // ===== HPF Section =====
    juce::Label hpfLabel;
    juce::ComboBox hpfCutoffCombo;
    juce::Label hpfCutoffLabel;
    
    // ===== VCF Section =====
    juce::Label vcfLabel;
    juce::Slider vcfCutoffSlider;
    juce::Label vcfCutoffLabel;
    juce::Slider vcfResonanceSlider;
    juce::Label vcfResonanceLabel;
    juce::Slider vcfLFO1DepthSlider;
    juce::Label vcfLFO1DepthLabel;
    juce::Slider vcfENVDepthSlider;
    juce::Label vcfENVDepthLabel;
    juce::Slider vcfKeyFollowSlider;
    juce::Label vcfKeyFollowLabel;
    juce::Slider vcfAfterSensSlider;
    juce::Label vcfAfterSensLabel;
    juce::ComboBox vcfDynamicsCombo;
    juce::Label vcfDynamicsLabel;
    juce::ComboBox vcfENVModeCombo;
    juce::Label vcfENVModeLabel;
    
    // ===== VCA Section =====
    juce::Label vcaLabel;
    juce::Slider vcaLevelSlider;
    juce::Label vcaLevelLabel;
    juce::Slider vcaAfterSensSlider;
    juce::Label vcaAfterSensLabel;
    juce::ComboBox vcaModeCombo;
    juce::Label vcaModeLabel;
    juce::ComboBox vcaDynamicsCombo;
    juce::Label vcaDynamicsLabel;
    
    // ===== ENV-1 Section =====
    juce::Label env1Label;
    juce::Slider env1Time1Slider;  // Attack
    juce::Label env1Time1Label;
    juce::Slider env1Time2Slider;  // Decay
    juce::Label env1Time2Label;
    juce::Slider env1Level1Slider; // Sustain (Level)
    juce::Label env1Level1Label;
    juce::Slider env1Time4Slider;  // Release
    juce::Label env1Time4Label;
    juce::Slider env1KeyFollowSlider; // Key Follow
    juce::Label env1KeyFollowLabel;
    juce::Slider env1VelSensSlider; // Velocity Sensitivity
    juce::Label env1VelSensLabel;
    
    // ===== ENV-2 Section =======
    juce::Label env2Label;
    juce::Slider env2Time1Slider;  // Attack
    juce::Label env2Time1Label;
    juce::Slider env2Time2Slider;  // Decay
    juce::Label env2Time2Label;
    juce::Slider env2Level1Slider; // Sustain (Level)
    juce::Label env2Level1Label;
    juce::Slider env2Time4Slider;  // Release
    juce::Label env2Time4Label;
    juce::Slider env2KeyFollowSlider; // Key Follow
    juce::Label env2KeyFollowLabel;
    juce::Slider env2VelSensSlider; // Velocity Sensitivity
    juce::Label env2VelSensLabel;

    // ===== ENV-3 Section (ADSR for VCF) =====
    juce::Label env3Label;
    juce::Slider env3AttackSlider;
    juce::Label env3AttackLabel;
    juce::Slider env3DecaySlider;
    juce::Label env3DecayLabel;
    juce::Slider env3SustainSlider;
    juce::Label env3SustainLabel;
    juce::Slider env3ReleaseSlider;
    juce::Label env3ReleaseLabel;

    // ===== ENV-4 Section (ADSR for VCA) =====
    juce::Label env4Label;
    juce::Slider env4AttackSlider;
    juce::Label env4AttackLabel;
    juce::Slider env4DecaySlider;
    juce::Label env4DecayLabel;
    juce::Slider env4SustainSlider;
    juce::Label env4SustainLabel;
    juce::Slider env4ReleaseSlider;
    juce::Label env4ReleaseLabel;
    
    // ===== LFO Section =====
    juce::Label lfoLabel;
    juce::ComboBox lfo1WaveformCombo;
    juce::Label lfo1WaveformLabel;
    juce::Slider lfo1DelaySlider;
    juce::Label lfo1DelayLabel;
    juce::Slider lfo1RateSlider;
    juce::Label lfo1RateLabel;
    
    // ===== Chorus Section =====
    juce::Label chorusLabel;
    juce::ComboBox chorusCombo;
    
    // ===== Buttons =====
    juce::TextButton initButton;
    juce::TextButton randomizeButton;
    juce::TextButton copyToUpperButton;
    
    // Envelope editors
    InteractiveEnvelopeEditor env1Editor;
    InteractiveEnvelopeEditor env2Editor;

    // MIDI Learn mapping per slider
    std::map<juce::Slider*, juce::String> sliderParamIds;
    std::map<juce::Slider*, bool> sliderSinkRegistered;
    // MIDI Learn mapping per combo
    std::map<juce::ComboBox*, juce::String> comboParamIds;
    std::map<juce::ComboBox*, bool> comboSinkRegistered;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToneLowerTab)
};
