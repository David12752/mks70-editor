/*
  ==============================================================================

    ToneLowerTab.cpp
    Component for Lower Tone (Tone B) parameters

  ==============================================================================
*/

#include "ToneLowerTab.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MidiLearnManager.h"
#include "MidiMappingEditor.h"
#include "WaveformDisplay.h"
#include "MixerWaveformDisplay.h"
#include "Theme.h"

//==============================================================================
ToneLowerTab::ToneLowerTab()
{
    titleLabel.setText("Lower Tone (Tone B)", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(Theme::heading1());
    addAndMakeVisible(titleLabel);
    
    setupLabels();
    setupComboBoxes();
    setupSliders();
    setupButtons();

    // Envelope editors (currently using ADSR display - TODO: update to 7-Stage)
    addAndMakeVisible(env1Editor);
    addAndMakeVisible(env2Editor);
    env1Editor.setShowValues(true);
    env2Editor.setShowValues(true);
    // TODO: Update envelope editors for 7-Stage structure
    // env1Editor.setEnvelope(parameters.env1Time1, parameters.env1Time2, parameters.env1Level1, parameters.env1Time4);
    // env2Editor.setEnvelope(parameters.env2Time1, parameters.env2Time2, parameters.env2Level1, parameters.env2Time4);
    // Callbacks disabled until 7-Stage support added to InteractiveEnvelopeEditor

    // Waveform displays
    addAndMakeVisible(dco1WaveDisplay);
    addAndMakeVisible(dco2WaveDisplay);
    addAndMakeVisible(mixerWaveDisplay);
    dco1WaveDisplay.setTitle("DCO-1");
    dco2WaveDisplay.setTitle("DCO-2");
    mixerWaveDisplay.setTitle("Mix");
    dco1WaveDisplay.setShowGrid(true);
    dco2WaveDisplay.setShowGrid(true);
    mixerWaveDisplay.setShowGrid(true);
    dco1WaveDisplay.setAllowWaveSelection(true);
    dco2WaveDisplay.setAllowWaveSelection(true);
    // Tooltips removed for compatibility; context menu still available
    dco1WaveDisplay.onRequestWaveformChange = [this](int wf){ dco1WaveformCombo.setSelectedId(wf + 1); onParameterChanged(); updateWaveDisplays(); };
    dco2WaveDisplay.onRequestWaveformChange = [this](int wf){ dco2WaveformCombo.setSelectedId(wf + 1); onParameterChanged(); updateWaveDisplays(); };

    setupListeners();

    // Register MIDI Learn for important sliders (lower tone parameter IDs)
    registerMidiForSlider(dco1TuneSlider,        "toneLower.dco1.tune");
    registerMidiForSlider(dco1LFODepthSlider,    "toneLower.dco1.lfoDepth");
    registerMidiForSlider(dco1ENVDepthSlider,    "toneLower.dco1.envDepth");
    registerMidiForSlider(dco1PulseWidthSlider,  "toneLower.dco1.pulseWidth");
    registerMidiForSlider(dco1AfterSensSlider,   "toneLower.dco1.aftertouch");

    registerMidiForSlider(dco2TuneSlider,        "toneLower.dco2.tune");
    registerMidiForSlider(dco2FineTuneSlider,    "toneLower.dco2.fineTune");
    registerMidiForSlider(dco2LFODepthSlider,    "toneLower.dco2.lfoDepth");
    registerMidiForSlider(dco2ENVDepthSlider,    "toneLower.dco2.envDepth");
    registerMidiForSlider(dco2PulseWidthSlider,  "toneLower.dco2.pulseWidth");
    registerMidiForSlider(dco2AfterSensSlider,   "toneLower.dco2.aftertouch");

    registerMidiForSlider(mixerDCO1LevelSlider,  "toneLower.mixer.dco1Level");
    registerMidiForSlider(mixerDCO2LevelSlider,  "toneLower.mixer.dco2Level");
    registerMidiForSlider(mixerENVDepthSlider,   "toneLower.mixer.envDepth");

    registerMidiForSlider(vcfCutoffSlider,       "toneLower.vcf.cutoff");
    registerMidiForSlider(vcfResonanceSlider,    "toneLower.vcf.resonance");
    registerMidiForSlider(vcfENVDepthSlider,     "toneLower.vcf.envDepth");
    registerMidiForSlider(vcfLFO1DepthSlider,    "toneLower.vcf.lfoDepth");
    registerMidiForSlider(vcfKeyFollowSlider,    "toneLower.vcf.keyFollow");
    registerMidiForSlider(vcfAfterSensSlider,    "toneLower.vcf.aftertouch");

    registerMidiForSlider(vcaLevelSlider,        "toneLower.vca.level");
    // LFO sliders + waveform
    registerMidiForSlider(lfo1DelaySlider,       "toneLower.lfo1.delay");
    registerMidiForSlider(lfo1RateSlider,        "toneLower.lfo1.rate");
    registerMidiForCombo(lfo1WaveformCombo,      "toneLower.lfo1.waveform");
    // ENV1/2 (7-stage subset mapped to ADSR names for MIDI learn ergonomics)
    registerMidiForSlider(env1Time1Slider,       "toneLower.env1.attack");
    registerMidiForSlider(env1Time2Slider,       "toneLower.env1.decay");
    registerMidiForSlider(env1Level1Slider,      "toneLower.env1.sustain");
    registerMidiForSlider(env1Time4Slider,       "toneLower.env1.release");
    registerMidiForSlider(env1KeyFollowSlider,   "toneLower.env1.keyFollow");
    registerMidiForSlider(env1VelSensSlider,     "toneLower.env1.velocity");
    registerMidiForSlider(env2Time1Slider,       "toneLower.env2.attack");
    registerMidiForSlider(env2Time2Slider,       "toneLower.env2.decay");
    registerMidiForSlider(env2Level1Slider,      "toneLower.env2.sustain");
    registerMidiForSlider(env2Time4Slider,       "toneLower.env2.release");
    registerMidiForSlider(env2KeyFollowSlider,   "toneLower.env2.keyFollow");
    registerMidiForSlider(env2VelSensSlider,     "toneLower.env2.velocity");
    // ENV3/4 (ADSR)
    registerMidiForSlider(env3AttackSlider,      "toneLower.env3.attack");
    registerMidiForSlider(env3DecaySlider,       "toneLower.env3.decay");
    registerMidiForSlider(env3SustainSlider,     "toneLower.env3.sustain");
    registerMidiForSlider(env3ReleaseSlider,     "toneLower.env3.release");
    registerMidiForSlider(env4AttackSlider,      "toneLower.env4.attack");
    registerMidiForSlider(env4DecaySlider,       "toneLower.env4.decay");
    registerMidiForSlider(env4SustainSlider,     "toneLower.env4.sustain");
    registerMidiForSlider(env4ReleaseSlider,     "toneLower.env4.release");
}

ToneLowerTab::~ToneLowerTab()
{
}

void ToneLowerTab::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw subtle separator lines between sections
    g.setColour(Theme::border().withAlpha(0.5f));
    
    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(24 + 4); // Title
    
    // After DCO section
    int y1 = area.getY() + 50 + 22 * 6 + 4 * 5 + 8;
    g.drawHorizontalLine(y1, 8.0f, (float)getWidth() - 8.0f);
}

void ToneLowerTab::resized()
{
    auto area = getLocalBounds().reduced(8);
    
    // Title - smaller
    titleLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(4);
    
    // Constants for compact layout
    const int rowH = 22;
    const int labelW = 45;
    const int comboW = 80;
    const int sliderW = 100;
    const int waveH = 50;
    const int gap = 4;
    const int sectionGap = 8;
    
    // Calculate column widths (3 columns)
    const int totalWidth = area.getWidth();
    const int colW = (totalWidth - 2 * gap) / 3;
    
    // ===== ROW 1: DCO-1, DCO-2, Mixer (with waveforms) =====
    auto dcoRow = area.removeFromTop(waveH + rowH * 6 + gap * 5);
    
    // DCO-1 Column
    auto dco1Col = dcoRow.removeFromLeft(colW);
    dco1Label.setBounds(dco1Col.removeFromTop(rowH));
    dco1WaveDisplay.setBounds(dco1Col.removeFromTop(waveH).reduced(2));
    
    auto r = dco1Col.removeFromTop(rowH);
    dco1RangeLabel.setBounds(r.removeFromLeft(labelW));
    dco1RangeCombo.setBounds(r.removeFromLeft(comboW));
    
    r = dco1Col.removeFromTop(rowH);
    dco1WaveformLabel.setBounds(r.removeFromLeft(labelW));
    dco1WaveformCombo.setBounds(r.removeFromLeft(comboW));
    
    r = dco1Col.removeFromTop(rowH);
    dco1TuneLabel.setBounds(r.removeFromLeft(labelW));
    dco1TuneSlider.setBounds(r);
    
    r = dco1Col.removeFromTop(rowH);
    dco1LFODepthLabel.setBounds(r.removeFromLeft(labelW));
    dco1LFODepthSlider.setBounds(r);
    
    r = dco1Col.removeFromTop(rowH);
    dco1ENVDepthLabel.setBounds(r.removeFromLeft(labelW));
    dco1ENVDepthSlider.setBounds(r);
    
    dcoRow.removeFromLeft(gap);
    
    // DCO-2 Column
    auto dco2Col = dcoRow.removeFromLeft(colW);
    dco2Label.setBounds(dco2Col.removeFromTop(rowH));
    dco2WaveDisplay.setBounds(dco2Col.removeFromTop(waveH).reduced(2));
    
    r = dco2Col.removeFromTop(rowH);
    dco2RangeLabel.setBounds(r.removeFromLeft(labelW));
    dco2RangeCombo.setBounds(r.removeFromLeft(comboW));
    
    r = dco2Col.removeFromTop(rowH);
    dco2WaveformLabel.setBounds(r.removeFromLeft(labelW));
    dco2WaveformCombo.setBounds(r.removeFromLeft(comboW));
    
    r = dco2Col.removeFromTop(rowH);
    dco2TuneLabel.setBounds(r.removeFromLeft(labelW));
    dco2TuneSlider.setBounds(r);
    
    r = dco2Col.removeFromTop(rowH);
    dco2FineTuneLabel.setBounds(r.removeFromLeft(labelW));
    dco2FineTuneSlider.setBounds(r);
    
    r = dco2Col.removeFromTop(rowH);
    dco2LFODepthLabel.setBounds(r.removeFromLeft(labelW));
    dco2LFODepthSlider.setBounds(r);
    
    r = dco2Col.removeFromTop(rowH);
    dco2ENVDepthLabel.setBounds(r.removeFromLeft(labelW));
    dco2ENVDepthSlider.setBounds(r);
    
    dcoRow.removeFromLeft(gap);
    
    // Mixer Column
    auto mixerCol = dcoRow;
    mixerLabel.setBounds(mixerCol.removeFromTop(rowH));
    mixerWaveDisplay.setBounds(mixerCol.removeFromTop(waveH).reduced(2));
    
    r = mixerCol.removeFromTop(rowH);
    mixerDCO1LevelLabel.setBounds(r.removeFromLeft(labelW));
    mixerDCO1LevelSlider.setBounds(r);
    
    r = mixerCol.removeFromTop(rowH);
    mixerDCO2LevelLabel.setBounds(r.removeFromLeft(labelW));
    mixerDCO2LevelSlider.setBounds(r);
    
    r = mixerCol.removeFromTop(rowH);
    mixerENVDepthLabel.setBounds(r.removeFromLeft(labelW));
    mixerENVDepthSlider.setBounds(r);
    
    r = mixerCol.removeFromTop(rowH);
    mixerDynamicsLabel.setBounds(r.removeFromLeft(labelW));
    mixerDynamicsCombo.setBounds(r.removeFromLeft(comboW));
    
    r = mixerCol.removeFromTop(rowH);
    mixerENVModeLabel.setBounds(r.removeFromLeft(labelW));
    mixerENVModeCombo.setBounds(r.removeFromLeft(comboW));
    
    area.removeFromTop(sectionGap);
    
    // ===== ROW 2: DCO Options, HPF/VCF, VCA =====
    auto filterRow = area.removeFromTop(rowH * 5 + gap * 4);
    
    // DCO Options Column
    auto dcoOptCol = filterRow.removeFromLeft(colW);
    r = dcoOptCol.removeFromTop(rowH);
    dcoCrossModLabel.setBounds(r.removeFromLeft(labelW));
    dcoCrossModCombo.setBounds(r.removeFromLeft(comboW));
    
    r = dcoOptCol.removeFromTop(rowH);
    dcoDynamicsLabel.setBounds(r.removeFromLeft(labelW));
    dcoDynamicsCombo.setBounds(r.removeFromLeft(comboW));
    
    r = dcoOptCol.removeFromTop(rowH);
    dcoENVModeLabel.setBounds(r.removeFromLeft(labelW));
    dcoENVModeCombo.setBounds(r.removeFromLeft(comboW));
    
    filterRow.removeFromLeft(gap);
    
    // VCF Column
    auto vcfCol = filterRow.removeFromLeft(colW);
    vcfLabel.setBounds(vcfCol.removeFromTop(rowH));
    
    r = vcfCol.removeFromTop(rowH);
    hpfCutoffLabel.setBounds(r.removeFromLeft(labelW));
    hpfCutoffCombo.setBounds(r.removeFromLeft(comboW));
    
    r = vcfCol.removeFromTop(rowH);
    vcfCutoffLabel.setBounds(r.removeFromLeft(labelW));
    vcfCutoffSlider.setBounds(r);
    
    r = vcfCol.removeFromTop(rowH);
    vcfResonanceLabel.setBounds(r.removeFromLeft(labelW));
    vcfResonanceSlider.setBounds(r);
    
    r = vcfCol.removeFromTop(rowH);
    vcfENVDepthLabel.setBounds(r.removeFromLeft(labelW));
    vcfENVDepthSlider.setBounds(r);
    
    filterRow.removeFromLeft(gap);
    
    // VCA Column
    auto vcaCol = filterRow;
    vcaLabel.setBounds(vcaCol.removeFromTop(rowH));
    
    r = vcaCol.removeFromTop(rowH);
    vcaLevelLabel.setBounds(r.removeFromLeft(labelW));
    vcaLevelSlider.setBounds(r);
    
    r = vcaCol.removeFromTop(rowH);
    vcaModeLabel.setBounds(r.removeFromLeft(labelW));
    vcaModeCombo.setBounds(r.removeFromLeft(comboW));
    
    r = vcaCol.removeFromTop(rowH);
    vcaDynamicsLabel.setBounds(r.removeFromLeft(labelW));
    vcaDynamicsCombo.setBounds(r.removeFromLeft(comboW));
    
    area.removeFromTop(sectionGap);
    
    // ===== ROW 3: VCF continued, LFO, Chorus =====
    auto row3 = area.removeFromTop(rowH * 4 + gap * 3);
    
    auto vcf2Col = row3.removeFromLeft(colW);
    r = vcf2Col.removeFromTop(rowH);
    vcfLFO1DepthLabel.setBounds(r.removeFromLeft(labelW));
    vcfLFO1DepthSlider.setBounds(r);
    
    r = vcf2Col.removeFromTop(rowH);
    vcfKeyFollowLabel.setBounds(r.removeFromLeft(labelW));
    vcfKeyFollowSlider.setBounds(r);
    
    r = vcf2Col.removeFromTop(rowH);
    vcfDynamicsLabel.setBounds(r.removeFromLeft(labelW));
    vcfDynamicsCombo.setBounds(r.removeFromLeft(comboW));
    
    r = vcf2Col.removeFromTop(rowH);
    vcfENVModeLabel.setBounds(r.removeFromLeft(labelW));
    vcfENVModeCombo.setBounds(r.removeFromLeft(comboW));
    
    row3.removeFromLeft(gap);
    
    // LFO Column
    auto lfoCol = row3.removeFromLeft(colW);
    lfoLabel.setBounds(lfoCol.removeFromTop(rowH));
    
    r = lfoCol.removeFromTop(rowH);
    lfo1WaveformLabel.setBounds(r.removeFromLeft(labelW));
    lfo1WaveformCombo.setBounds(r.removeFromLeft(comboW));
    
    r = lfoCol.removeFromTop(rowH);
    lfo1DelayLabel.setBounds(r.removeFromLeft(labelW));
    lfo1DelaySlider.setBounds(r);
    
    r = lfoCol.removeFromTop(rowH);
    lfo1RateLabel.setBounds(r.removeFromLeft(labelW));
    lfo1RateSlider.setBounds(r);
    
    row3.removeFromLeft(gap);
    
    // Chorus & Buttons Column
    auto chorusCol = row3;
    r = chorusCol.removeFromTop(rowH);
    chorusLabel.setBounds(r.removeFromLeft(labelW));
    chorusCombo.setBounds(r.removeFromLeft(comboW));
    
    chorusCol.removeFromTop(gap);
    initButton.setBounds(chorusCol.removeFromTop(rowH));
    chorusCol.removeFromTop(gap);
    randomizeButton.setBounds(chorusCol.removeFromTop(rowH));
    chorusCol.removeFromTop(gap);
    copyToUpperButton.setBounds(chorusCol.removeFromTop(rowH));
    
    area.removeFromTop(sectionGap);
    
    // ===== ROW 4: ENV-1, ENV-2, ENV-3/4 (stacked) =====
    auto envRow = area.removeFromTop(rowH * 6 + gap * 5);
    
    // ENV-1 Column
    auto env1Col = envRow.removeFromLeft(colW);
    env1Label.setBounds(env1Col.removeFromTop(rowH));
    
    r = env1Col.removeFromTop(rowH);
    env1Time1Label.setBounds(r.removeFromLeft(25));
    env1Time1Slider.setBounds(r);
    
    r = env1Col.removeFromTop(rowH);
    env1Time2Label.setBounds(r.removeFromLeft(25));
    env1Time2Slider.setBounds(r);
    
    r = env1Col.removeFromTop(rowH);
    env1Level1Label.setBounds(r.removeFromLeft(25));
    env1Level1Slider.setBounds(r);
    
    r = env1Col.removeFromTop(rowH);
    env1Time4Label.setBounds(r.removeFromLeft(25));
    env1Time4Slider.setBounds(r);
    
    r = env1Col.removeFromTop(rowH);
    env1KeyFollowLabel.setBounds(r.removeFromLeft(35));
    env1KeyFollowSlider.setBounds(r);
    
    envRow.removeFromLeft(gap);
    
    // ENV-2 Column
    auto env2Col = envRow.removeFromLeft(colW);
    env2Label.setBounds(env2Col.removeFromTop(rowH));
    
    r = env2Col.removeFromTop(rowH);
    env2Time1Label.setBounds(r.removeFromLeft(25));
    env2Time1Slider.setBounds(r);
    
    r = env2Col.removeFromTop(rowH);
    env2Time2Label.setBounds(r.removeFromLeft(25));
    env2Time2Slider.setBounds(r);
    
    r = env2Col.removeFromTop(rowH);
    env2Level1Label.setBounds(r.removeFromLeft(25));
    env2Level1Slider.setBounds(r);
    
    r = env2Col.removeFromTop(rowH);
    env2Time4Label.setBounds(r.removeFromLeft(25));
    env2Time4Slider.setBounds(r);
    
    r = env2Col.removeFromTop(rowH);
    env2KeyFollowLabel.setBounds(r.removeFromLeft(35));
    env2KeyFollowSlider.setBounds(r);
    
    envRow.removeFromLeft(gap);
    
    // ENV-3 & ENV-4 Column (stacked)
    auto env34Col = envRow;
    env3Label.setBounds(env34Col.removeFromTop(rowH));
    
    // ENV-3 as horizontal row
    r = env34Col.removeFromTop(rowH);
    env3AttackLabel.setBounds(r.removeFromLeft(15));
    env3AttackSlider.setBounds(r.removeFromLeft(55));
    env3DecayLabel.setBounds(r.removeFromLeft(15));
    env3DecaySlider.setBounds(r.removeFromLeft(55));
    
    r = env34Col.removeFromTop(rowH);
    env3SustainLabel.setBounds(r.removeFromLeft(15));
    env3SustainSlider.setBounds(r.removeFromLeft(55));
    env3ReleaseLabel.setBounds(r.removeFromLeft(15));
    env3ReleaseSlider.setBounds(r.removeFromLeft(55));
    
    env34Col.removeFromTop(gap);
    env4Label.setBounds(env34Col.removeFromTop(rowH));
    
    // ENV-4 as horizontal row
    r = env34Col.removeFromTop(rowH);
    env4AttackLabel.setBounds(r.removeFromLeft(15));
    env4AttackSlider.setBounds(r.removeFromLeft(55));
    env4DecayLabel.setBounds(r.removeFromLeft(15));
    env4DecaySlider.setBounds(r.removeFromLeft(55));
    
    r = env34Col.removeFromTop(rowH);
    env4SustainLabel.setBounds(r.removeFromLeft(15));
    env4SustainSlider.setBounds(r.removeFromLeft(55));
    env4ReleaseLabel.setBounds(r.removeFromLeft(15));
    env4ReleaseSlider.setBounds(r.removeFromLeft(55));
    
    area.removeFromTop(sectionGap);
    
    // ===== ROW 5: Envelope Editors (side by side) =====
    auto editorRow = area;
    const int editorH = juce::jmax(60, editorRow.getHeight());
    auto editorArea = editorRow.removeFromTop(editorH);
    
    env1Editor.setBounds(editorArea.removeFromLeft(colW + colW/2).reduced(2));
    editorArea.removeFromLeft(gap);
    env2Editor.setBounds(editorArea.reduced(2));
    
    // Hide labels that are part of the section headers
    hpfLabel.setVisible(false);
}

void ToneLowerTab::setupLabels()
{
    // Section labels
    dco1Label.setText("DCO-1", juce::dontSendNotification);
    dco1Label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(dco1Label);
    
    dco2Label.setText("DCO-2", juce::dontSendNotification);
    dco2Label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(dco2Label);
    
    mixerLabel.setText("Mixer", juce::dontSendNotification);
    mixerLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(mixerLabel);
    
    hpfLabel.setText("HPF", juce::dontSendNotification);
    hpfLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(hpfLabel);
    
    vcfLabel.setText("VCF", juce::dontSendNotification);
    vcfLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(vcfLabel);
    
    vcaLabel.setText("VCA", juce::dontSendNotification);
    vcaLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(vcaLabel);
    
    env1Label.setText("ENV-1", juce::dontSendNotification);
    env1Label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(env1Label);
    
    env2Label.setText("ENV-2", juce::dontSendNotification);
    env2Label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(env2Label);
    
    lfoLabel.setText("LFO", juce::dontSendNotification);
    lfoLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(lfoLabel);
    
    chorusLabel.setText("Chorus", juce::dontSendNotification);
    addAndMakeVisible(chorusLabel);
    
    // Parameter labels
    dco1RangeLabel.setText("Range", juce::dontSendNotification);
    addAndMakeVisible(dco1RangeLabel);
    
    dco1WaveformLabel.setText("Wave", juce::dontSendNotification);
    addAndMakeVisible(dco1WaveformLabel);
    
    dco1TuneLabel.setText("Tune", juce::dontSendNotification);
    addAndMakeVisible(dco1TuneLabel);
    
    dco2RangeLabel.setText("Range", juce::dontSendNotification);
    addAndMakeVisible(dco2RangeLabel);
    
    dco2WaveformLabel.setText("Wave", juce::dontSendNotification);
    addAndMakeVisible(dco2WaveformLabel);
    
    dco2TuneLabel.setText("Tune", juce::dontSendNotification);
    addAndMakeVisible(dco2TuneLabel);
    
    mixerDCO1LevelLabel.setText("DCO1", juce::dontSendNotification);
    addAndMakeVisible(mixerDCO1LevelLabel);
    
    mixerDCO2LevelLabel.setText("DCO2", juce::dontSendNotification);
    addAndMakeVisible(mixerDCO2LevelLabel);
    
    hpfCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    addAndMakeVisible(hpfCutoffLabel);
    
    vcfCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    addAndMakeVisible(vcfCutoffLabel);
    
    vcfResonanceLabel.setText("Res", juce::dontSendNotification);
    addAndMakeVisible(vcfResonanceLabel);
    
    vcfENVDepthLabel.setText("ENV", juce::dontSendNotification);
    addAndMakeVisible(vcfENVDepthLabel);
    
    vcaLevelLabel.setText("Level", juce::dontSendNotification);
    addAndMakeVisible(vcaLevelLabel);
    
    env1Time1Label.setText("A", juce::dontSendNotification);
    addAndMakeVisible(env1Time1Label);
    
    env1Time2Label.setText("D", juce::dontSendNotification);
    addAndMakeVisible(env1Time2Label);
    env1Level1Label.setText("S", juce::dontSendNotification);
    addAndMakeVisible(env1Level1Label);
    env1Time4Label.setText("R", juce::dontSendNotification);
    addAndMakeVisible(env1Time4Label);
    
    env2Time1Label.setText("A", juce::dontSendNotification);
    addAndMakeVisible(env2Time1Label);
    
    env2Time2Label.setText("D", juce::dontSendNotification);
    addAndMakeVisible(env2Time2Label);
    env2Level1Label.setText("S", juce::dontSendNotification);
    addAndMakeVisible(env2Level1Label);
    env2Time4Label.setText("R", juce::dontSendNotification);
    addAndMakeVisible(env2Time4Label);
    
    lfo1WaveformLabel.setText("Wave", juce::dontSendNotification);
    addAndMakeVisible(lfo1WaveformLabel);
    
    // Extra parameter labels
    dcoCrossModLabel.setText("X-Mod", juce::dontSendNotification);
    addAndMakeVisible(dcoCrossModLabel);
    dco1LFODepthLabel.setText("LFO", juce::dontSendNotification);
    addAndMakeVisible(dco1LFODepthLabel);
    dco1ENVDepthLabel.setText("ENV", juce::dontSendNotification);
    addAndMakeVisible(dco1ENVDepthLabel);
    dco1PulseWidthLabel.setText("PW", juce::dontSendNotification);
    addAndMakeVisible(dco1PulseWidthLabel);
    dco1AfterSensLabel.setText("After", juce::dontSendNotification);
    addAndMakeVisible(dco1AfterSensLabel);
    dco2FineTuneLabel.setText("Fine", juce::dontSendNotification);
    addAndMakeVisible(dco2FineTuneLabel);
    dco2LFODepthLabel.setText("LFO", juce::dontSendNotification);
    addAndMakeVisible(dco2LFODepthLabel);
    dco2ENVDepthLabel.setText("ENV", juce::dontSendNotification);
    addAndMakeVisible(dco2ENVDepthLabel);
    dco2PulseWidthLabel.setText("PW", juce::dontSendNotification);
    addAndMakeVisible(dco2PulseWidthLabel);
    dco2AfterSensLabel.setText("After", juce::dontSendNotification);
    addAndMakeVisible(dco2AfterSensLabel);
    mixerENVDepthLabel.setText("ENV", juce::dontSendNotification);
    addAndMakeVisible(mixerENVDepthLabel);
    vcfLFO1DepthLabel.setText("LFO", juce::dontSendNotification);
    addAndMakeVisible(vcfLFO1DepthLabel);
    vcfKeyFollowLabel.setText("KeyF", juce::dontSendNotification);
    addAndMakeVisible(vcfKeyFollowLabel);
    vcfAfterSensLabel.setText("After", juce::dontSendNotification);
    addAndMakeVisible(vcfAfterSensLabel);
    lfo1DelayLabel.setText("Delay", juce::dontSendNotification);
    addAndMakeVisible(lfo1DelayLabel);
    lfo1RateLabel.setText("Rate", juce::dontSendNotification);
    addAndMakeVisible(lfo1RateLabel);
    
    // New labels for missing params
    dcoDynamicsLabel.setText("Dyn", juce::dontSendNotification);
    addAndMakeVisible(dcoDynamicsLabel);
    dcoENVModeLabel.setText("EnvM", juce::dontSendNotification);
    addAndMakeVisible(dcoENVModeLabel);
    mixerDynamicsLabel.setText("Dyn", juce::dontSendNotification);
    addAndMakeVisible(mixerDynamicsLabel);
    mixerENVModeLabel.setText("EnvM", juce::dontSendNotification);
    addAndMakeVisible(mixerENVModeLabel);
    vcfDynamicsLabel.setText("Dyn", juce::dontSendNotification);
    addAndMakeVisible(vcfDynamicsLabel);
    vcfENVModeLabel.setText("EnvM", juce::dontSendNotification);
    addAndMakeVisible(vcfENVModeLabel);
    vcaModeLabel.setText("Mode", juce::dontSendNotification);
    addAndMakeVisible(vcaModeLabel);
    vcaDynamicsLabel.setText("Dyn", juce::dontSendNotification);
    addAndMakeVisible(vcaDynamicsLabel);
    vcaAfterSensLabel.setText("After", juce::dontSendNotification);
    addAndMakeVisible(vcaAfterSensLabel);
    env1KeyFollowLabel.setText("KeyF", juce::dontSendNotification);
    addAndMakeVisible(env1KeyFollowLabel);
    env1VelSensLabel.setText("Vel", juce::dontSendNotification);
    addAndMakeVisible(env1VelSensLabel);
    env2KeyFollowLabel.setText("KeyF", juce::dontSendNotification);
    addAndMakeVisible(env2KeyFollowLabel);
    env2VelSensLabel.setText("Vel", juce::dontSendNotification);
    addAndMakeVisible(env2VelSensLabel);

    // ENV3/4 ADSR labels
    env3Label.setText("ENV-3 (VCF)", juce::dontSendNotification);
    env3Label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(env3Label);
    env3AttackLabel.setText("A", juce::dontSendNotification);
    addAndMakeVisible(env3AttackLabel);
    env3DecayLabel.setText("D", juce::dontSendNotification);
    addAndMakeVisible(env3DecayLabel);
    env3SustainLabel.setText("S", juce::dontSendNotification);
    addAndMakeVisible(env3SustainLabel);
    env3ReleaseLabel.setText("R", juce::dontSendNotification);
    addAndMakeVisible(env3ReleaseLabel);

    env4Label.setText("ENV-4 (VCA)", juce::dontSendNotification);
    env4Label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(env4Label);
    env4AttackLabel.setText("A", juce::dontSendNotification);
    addAndMakeVisible(env4AttackLabel);
    env4DecayLabel.setText("D", juce::dontSendNotification);
    addAndMakeVisible(env4DecayLabel);
    env4SustainLabel.setText("S", juce::dontSendNotification);
    addAndMakeVisible(env4SustainLabel);
    env4ReleaseLabel.setText("R", juce::dontSendNotification);
    addAndMakeVisible(env4ReleaseLabel);
}

void ToneLowerTab::setupComboBoxes()
{
    // DCO-1 Range
    dco1RangeCombo.addItem("16'", 1);
    dco1RangeCombo.addItem("8'", 2);
    dco1RangeCombo.addItem("4'", 3);
    dco1RangeCombo.addItem("2'", 4);
    dco1RangeCombo.setSelectedId(2);
    addAndMakeVisible(dco1RangeCombo);
    
    // DCO-1 Waveform
    dco1WaveformCombo.addItem("Pulse", 1);
    dco1WaveformCombo.addItem("Saw", 2);
    dco1WaveformCombo.addItem("Square", 3);
    dco1WaveformCombo.addItem("Triangle", 4);
    dco1WaveformCombo.setSelectedId(2);
    addAndMakeVisible(dco1WaveformCombo);
    
    // DCO-2 Range
    dco2RangeCombo.addItem("16'", 1);
    dco2RangeCombo.addItem("8'", 2);
    dco2RangeCombo.addItem("4'", 3);
    dco2RangeCombo.addItem("2'", 4);
    dco2RangeCombo.setSelectedId(2);
    addAndMakeVisible(dco2RangeCombo);
    
    // DCO-2 Waveform
    dco2WaveformCombo.addItem("Pulse", 1);
    dco2WaveformCombo.addItem("Saw", 2);
    dco2WaveformCombo.addItem("Square", 3);
    dco2WaveformCombo.addItem("Triangle", 4);
    dco2WaveformCombo.setSelectedId(2);
    addAndMakeVisible(dco2WaveformCombo);
    
    // DCO Cross Mod (0..3)
    dcoCrossModCombo.addItem("Off", 1);
    dcoCrossModCombo.addItem("Sync", 2);
    dcoCrossModCombo.addItem("Metal", 3);
    dcoCrossModCombo.addItem("Both", 4);
    dcoCrossModCombo.setSelectedId(1);
    addAndMakeVisible(dcoCrossModCombo);
    
    // HPF Cutoff
    hpfCutoffCombo.addItem("0", 1);
    hpfCutoffCombo.addItem("1", 2);
    hpfCutoffCombo.addItem("2", 3);
    hpfCutoffCombo.addItem("3", 4);
    hpfCutoffCombo.setSelectedId(1);
    addAndMakeVisible(hpfCutoffCombo);
    
    // LFO Waveform
    lfo1WaveformCombo.addItem("Random", 1);
    lfo1WaveformCombo.addItem("Square", 2);
    lfo1WaveformCombo.addItem("Saw", 3);
    lfo1WaveformCombo.addItem("Sine", 4);
    lfo1WaveformCombo.setSelectedId(4);
    addAndMakeVisible(lfo1WaveformCombo);
    
    // Chorus
    chorusCombo.addItem("Off", 1);
    chorusCombo.addItem("I", 2);
    chorusCombo.addItem("II", 3);
    chorusCombo.setSelectedId(1);
    addAndMakeVisible(chorusCombo);
    
    // DCO Dynamics
    dcoDynamicsCombo.addItem("0", 1);
    dcoDynamicsCombo.addItem("1", 2);
    dcoDynamicsCombo.addItem("2", 3);
    dcoDynamicsCombo.addItem("3", 4);
    dcoDynamicsCombo.setSelectedId(1);
    addAndMakeVisible(dcoDynamicsCombo);
    
    // DCO ENV Mode
    dcoENVModeCombo.addItem("0", 1);
    dcoENVModeCombo.addItem("1", 2);
    dcoENVModeCombo.addItem("2", 3);
    dcoENVModeCombo.addItem("3", 4);
    dcoENVModeCombo.setSelectedId(1);
    addAndMakeVisible(dcoENVModeCombo);
    
    // Mixer Dynamics
    mixerDynamicsCombo.addItem("0", 1);
    mixerDynamicsCombo.addItem("1", 2);
    mixerDynamicsCombo.addItem("2", 3);
    mixerDynamicsCombo.addItem("3", 4);
    mixerDynamicsCombo.setSelectedId(1);
    addAndMakeVisible(mixerDynamicsCombo);
    
    // Mixer ENV Mode
    mixerENVModeCombo.addItem("0", 1);
    mixerENVModeCombo.addItem("1", 2);
    mixerENVModeCombo.addItem("2", 3);
    mixerENVModeCombo.addItem("3", 4);
    mixerENVModeCombo.setSelectedId(1);
    addAndMakeVisible(mixerENVModeCombo);
    
    // VCF Dynamics
    vcfDynamicsCombo.addItem("0", 1);
    vcfDynamicsCombo.addItem("1", 2);
    vcfDynamicsCombo.addItem("2", 3);
    vcfDynamicsCombo.addItem("3", 4);
    vcfDynamicsCombo.setSelectedId(1);
    addAndMakeVisible(vcfDynamicsCombo);
    
    // VCF ENV Mode
    vcfENVModeCombo.addItem("0", 1);
    vcfENVModeCombo.addItem("1", 2);
    vcfENVModeCombo.addItem("2", 3);
    vcfENVModeCombo.addItem("3", 4);
    vcfENVModeCombo.setSelectedId(1);
    addAndMakeVisible(vcfENVModeCombo);
    
    // VCA Mode
    vcaModeCombo.addItem("Gate", 1);
    vcaModeCombo.addItem("Env", 2);
    vcaModeCombo.setSelectedId(2);
    addAndMakeVisible(vcaModeCombo);
    
    // VCA Dynamics
    vcaDynamicsCombo.addItem("0", 1);
    vcaDynamicsCombo.addItem("1", 2);
    vcaDynamicsCombo.addItem("2", 3);
    vcaDynamicsCombo.addItem("3", 4);
    vcaDynamicsCombo.setSelectedId(1);
    addAndMakeVisible(vcaDynamicsCombo);
}

void ToneLowerTab::setupSliders()
{
    // DCO-1
    dco1TuneSlider.setRange(0, 127, 1);
    dco1TuneSlider.setValue(64);
    dco1TuneSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dco1TuneSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(dco1TuneSlider);
    dco1LFODepthSlider.setRange(0, 127, 1);
    dco1LFODepthSlider.setValue(0);
    dco1LFODepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dco1LFODepthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(dco1LFODepthSlider);
    dco1ENVDepthSlider.setRange(0, 127, 1);
    dco1ENVDepthSlider.setValue(0);
    dco1ENVDepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dco1ENVDepthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(dco1ENVDepthSlider);
    
    // DCO-2
    dco2TuneSlider.setRange(0, 127, 1);
    dco2TuneSlider.setValue(64);
    dco2TuneSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dco2TuneSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(dco2TuneSlider);
    dco2FineTuneSlider.setRange(0, 127, 1);
    dco2FineTuneSlider.setValue(64);
    dco2FineTuneSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dco2FineTuneSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(dco2FineTuneSlider);
    dco2LFODepthSlider.setRange(0, 127, 1);
    dco2LFODepthSlider.setValue(0);
    dco2LFODepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dco2LFODepthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(dco2LFODepthSlider);
    dco2ENVDepthSlider.setRange(0, 127, 1);
    dco2ENVDepthSlider.setValue(0);
    dco2ENVDepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dco2ENVDepthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(dco2ENVDepthSlider);
    
    // Mixer
    mixerDCO1LevelSlider.setRange(0, 127, 1);
    mixerDCO1LevelSlider.setValue(100);
    mixerDCO1LevelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixerDCO1LevelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(mixerDCO1LevelSlider);
    
    mixerDCO2LevelSlider.setRange(0, 127, 1);
    mixerDCO2LevelSlider.setValue(100);
    mixerDCO2LevelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixerDCO2LevelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(mixerDCO2LevelSlider);
    mixerENVDepthSlider.setRange(0, 127, 1);
    mixerENVDepthSlider.setValue(0);
    mixerENVDepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixerENVDepthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(mixerENVDepthSlider);
    
    // VCF
    vcfCutoffSlider.setRange(0, 127, 1);
    vcfCutoffSlider.setValue(127);
    vcfCutoffSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    vcfCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(vcfCutoffSlider);
    
    vcfResonanceSlider.setRange(0, 127, 1);
    vcfResonanceSlider.setValue(0);
    vcfResonanceSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    vcfResonanceSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(vcfResonanceSlider);
    
    vcfENVDepthSlider.setRange(0, 127, 1);
    vcfENVDepthSlider.setValue(64);
    vcfENVDepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    vcfENVDepthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(vcfENVDepthSlider);
    vcfLFO1DepthSlider.setRange(0, 127, 1);
    vcfLFO1DepthSlider.setValue(0);
    vcfLFO1DepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    vcfLFO1DepthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(vcfLFO1DepthSlider);
    vcfKeyFollowSlider.setRange(0, 127, 1);
    vcfKeyFollowSlider.setValue(64);
    vcfKeyFollowSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    vcfKeyFollowSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(vcfKeyFollowSlider);
    
    // VCA
    vcaLevelSlider.setRange(0, 127, 1);
    vcaLevelSlider.setValue(127);
    vcaLevelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    vcaLevelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(vcaLevelSlider);
    
    // ENV-1
    env1Time1Slider.setRange(0, 127, 1);
    env1Time1Slider.setValue(0);
    env1Time1Slider.setSliderStyle(juce::Slider::LinearHorizontal);
    env1Time1Slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env1Time1Slider);
    
    env1Time2Slider.setRange(0, 127, 1);
    env1Time2Slider.setValue(64);
    env1Time2Slider.setSliderStyle(juce::Slider::LinearHorizontal);
    env1Time2Slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env1Time2Slider);
    
    env1Level1Slider.setRange(0, 127, 1);
    env1Level1Slider.setValue(100);
    env1Level1Slider.setSliderStyle(juce::Slider::LinearHorizontal);
    env1Level1Slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env1Level1Slider);
    
    env1Time4Slider.setRange(0, 127, 1);
    env1Time4Slider.setValue(64);
    env1Time4Slider.setSliderStyle(juce::Slider::LinearHorizontal);
    env1Time4Slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env1Time4Slider);
    
    // ENV-2
    env2Time1Slider.setRange(0, 127, 1);
    env2Time1Slider.setValue(0);
    env2Time1Slider.setSliderStyle(juce::Slider::LinearHorizontal);
    env2Time2Slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env2Time1Slider);
    
    env2Time2Slider.setRange(0, 127, 1);
    env2Time2Slider.setValue(64);
    env2Time2Slider.setSliderStyle(juce::Slider::LinearHorizontal);
    env2Time2Slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env2Time2Slider);
    
    env2Level1Slider.setRange(0, 127, 1);
    env2Level1Slider.setValue(100);
    env2Level1Slider.setSliderStyle(juce::Slider::LinearHorizontal);
    env2Level1Slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env2Level1Slider);
    
    env2Time4Slider.setRange(0, 127, 1);
    env2Time4Slider.setValue(64);
    env2Time4Slider.setSliderStyle(juce::Slider::LinearHorizontal);
    env2Time4Slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env2Time4Slider);
    
    // ENV Key Follow
    env1KeyFollowSlider.setRange(0, 3, 1);
    env1KeyFollowSlider.setValue(0);
    env1KeyFollowSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    env1KeyFollowSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env1KeyFollowSlider);
    
    env2KeyFollowSlider.setRange(0, 3, 1);
    env2KeyFollowSlider.setValue(0);
    env2KeyFollowSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    env2KeyFollowSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(env2KeyFollowSlider);
    
    // LFO
    lfo1DelaySlider.setRange(0, 127, 1);
    lfo1DelaySlider.setValue(0);
    lfo1DelaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lfo1DelaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(lfo1DelaySlider);
    lfo1RateSlider.setRange(0, 127, 1);
    lfo1RateSlider.setValue(64);
    lfo1RateSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lfo1RateSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(lfo1RateSlider);

    // ENV3/4 ADSR sliders
    for (auto* s : { &env3AttackSlider, &env3DecaySlider, &env3SustainSlider, &env3ReleaseSlider,
                     &env4AttackSlider, &env4DecaySlider, &env4SustainSlider, &env4ReleaseSlider })
    {
        s->setRange(0, 127, 1);
        s->setSliderStyle(juce::Slider::LinearHorizontal);
        s->setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        addAndMakeVisible(*s);
    }
}

void ToneLowerTab::setupButtons()
{
    initButton.setButtonText("Init Tone");
    addAndMakeVisible(initButton);
    
    randomizeButton.setButtonText("Randomize");
    addAndMakeVisible(randomizeButton);
    
    copyToUpperButton.setButtonText("Copy to Upper");
    addAndMakeVisible(copyToUpperButton);
}

void ToneLowerTab::setParameters(const ToneParams& params)
{
    parameters = params;
    
    // Update UI controls
    dco1RangeCombo.setSelectedId(parameters.dco1Range + 1);
    dco1WaveformCombo.setSelectedId(parameters.dco1Waveform + 1);
    dco1TuneSlider.setValue(parameters.dco1Tune);
    dco1LFODepthSlider.setValue(parameters.dco1LFOMod);
    dco1ENVDepthSlider.setValue(parameters.dco1ENVMod);
    
    dco2RangeCombo.setSelectedId(parameters.dco2Range + 1);
    dco2WaveformCombo.setSelectedId(parameters.dco2Waveform + 1);
    dco2TuneSlider.setValue(parameters.dco2Tune);
    dco2FineTuneSlider.setValue(parameters.dco2FineTune);
    dco2LFODepthSlider.setValue(parameters.dco2LFOMod);
    dco2ENVDepthSlider.setValue(parameters.dco2ENVMod);
    
    mixerDCO1LevelSlider.setValue(parameters.mixerDCO1Level);
    mixerDCO2LevelSlider.setValue(parameters.mixerDCO2Level);
    mixerENVDepthSlider.setValue(parameters.mixerENVMod);
    
    hpfCutoffCombo.setSelectedId(parameters.vcfHPF + 1);
    
    vcfCutoffSlider.setValue(parameters.vcfCutoff);
    vcfResonanceSlider.setValue(parameters.vcfResonance);
    vcfENVDepthSlider.setValue(parameters.vcfENVMod);
    vcfLFO1DepthSlider.setValue(parameters.vcfLFOMod);
    vcfKeyFollowSlider.setValue(parameters.vcfKeyFollow);
    
    vcaLevelSlider.setValue(parameters.vcaLevel);
    
    env1Time1Slider.setValue(parameters.env1Time1);
    env1Time2Slider.setValue(parameters.env1Time2);
    env1Level1Slider.setValue(parameters.env1Level1);
    env1Time4Slider.setValue(parameters.env1Time4);
    env1Editor.setEnvelope7Stage(parameters.env1Time1,
                                 parameters.env1Level1,
                                 parameters.env1Time2,
                                 parameters.env1Level2,
                                 parameters.env1Time3,
                                 parameters.env1Level3,
                                 parameters.env1Time4);
    env1Editor.setKeyFollow(parameters.env1KeyFollow);
    
    env2Time1Slider.setValue(parameters.env2Time1);
    env2Time2Slider.setValue(parameters.env2Time2);
    env2Level1Slider.setValue(parameters.env2Level1);
    env2Time4Slider.setValue(parameters.env2Time4);
    env2Editor.setEnvelope7Stage(parameters.env2Time1,
                                 parameters.env2Level1,
                                 parameters.env2Time2,
                                 parameters.env2Level2,
                                 parameters.env2Time3,
                                 parameters.env2Level3,
                                 parameters.env2Time4);
    env2Editor.setKeyFollow(parameters.env2KeyFollow);
    
    lfo1WaveformCombo.setSelectedId(parameters.lfo1Waveform + 1);
    lfo1DelaySlider.setValue(parameters.lfo1Delay);
    lfo1RateSlider.setValue(parameters.lfo1Rate);

    // ENV3/4
    env3AttackSlider.setValue(parameters.env3Attack);
    env3DecaySlider.setValue(parameters.env3Decay);
    env3SustainSlider.setValue(parameters.env3Sustain);
    env3ReleaseSlider.setValue(parameters.env3Release);
    env4AttackSlider.setValue(parameters.env4Attack);
    env4DecaySlider.setValue(parameters.env4Decay);
    env4SustainSlider.setValue(parameters.env4Sustain);
    env4ReleaseSlider.setValue(parameters.env4Release);
    
    chorusCombo.setSelectedId(parameters.chorusMode + 1);
    
    // New params
    dcoDynamicsCombo.setSelectedId(parameters.dcoDynamics + 1);
    dcoENVModeCombo.setSelectedId(parameters.dcoENVMode + 1);
    mixerDynamicsCombo.setSelectedId(parameters.mixerDynamics + 1);
    mixerENVModeCombo.setSelectedId(parameters.mixerENVMode + 1);
    vcfDynamicsCombo.setSelectedId(parameters.vcfDynamics + 1);
    vcfENVModeCombo.setSelectedId(parameters.vcfENVMode + 1);
    vcaModeCombo.setSelectedId(parameters.vcaMode + 1);
    vcaDynamicsCombo.setSelectedId(parameters.vcaDynamics + 1);
    env1KeyFollowSlider.setValue(parameters.env1KeyFollow);
    env2KeyFollowSlider.setValue(parameters.env2KeyFollow);

    updateWaveDisplays();
    repaint();
}

void ToneLowerTab::refreshFromProcessor()
{
    if (processor != nullptr)
    {
        setParameters(processor->getToneLower());
    }
}

void ToneLowerTab::setupListeners()
{
    auto paramChanged = [this]() { onParameterChanged(); updateWaveDisplays(); };

    // ComboBox listeners
    dco1RangeCombo.onChange = paramChanged;
    dco1WaveformCombo.onChange = paramChanged;
    dco2RangeCombo.onChange = paramChanged;
    dco2WaveformCombo.onChange = paramChanged;
    dcoCrossModCombo.onChange = paramChanged;
    hpfCutoffCombo.onChange = paramChanged;
    lfo1WaveformCombo.onChange = paramChanged;
    chorusCombo.onChange = paramChanged;
    dcoDynamicsCombo.onChange = paramChanged;
    dcoENVModeCombo.onChange = paramChanged;
    mixerDynamicsCombo.onChange = paramChanged;
    mixerENVModeCombo.onChange = paramChanged;
    vcfDynamicsCombo.onChange = paramChanged;
    vcfENVModeCombo.onChange = paramChanged;
    vcaModeCombo.onChange = paramChanged;
    vcaDynamicsCombo.onChange = paramChanged;
    
    // Slider listeners
    dco1TuneSlider.onValueChange = paramChanged;
    dco1LFODepthSlider.onValueChange = paramChanged;
    dco1ENVDepthSlider.onValueChange = paramChanged;
    dco2TuneSlider.onValueChange = paramChanged;
    dco2FineTuneSlider.onValueChange = paramChanged;
    dco2LFODepthSlider.onValueChange = paramChanged;
    dco2ENVDepthSlider.onValueChange = paramChanged;
    mixerDCO1LevelSlider.onValueChange = paramChanged;
    mixerDCO2LevelSlider.onValueChange = paramChanged;
    mixerENVDepthSlider.onValueChange = paramChanged;
    vcfCutoffSlider.onValueChange = paramChanged;
    vcfResonanceSlider.onValueChange = paramChanged;
    vcfENVDepthSlider.onValueChange = paramChanged;
    vcfLFO1DepthSlider.onValueChange = paramChanged;
    vcfKeyFollowSlider.onValueChange = paramChanged;
    vcaLevelSlider.onValueChange = paramChanged;
    lfo1DelaySlider.onValueChange = paramChanged;
    lfo1RateSlider.onValueChange = paramChanged;
    env1KeyFollowSlider.onValueChange = paramChanged;
    env2KeyFollowSlider.onValueChange = paramChanged;
    env1Time1Slider.onValueChange = paramChanged;
    env1Time2Slider.onValueChange = paramChanged;
    env1Level1Slider.onValueChange = paramChanged;
    env1Time4Slider.onValueChange = paramChanged;
    env2Time1Slider.onValueChange = paramChanged;
    env2Time2Slider.onValueChange = paramChanged;
    env2Level1Slider.onValueChange = paramChanged;
    env2Time4Slider.onValueChange = paramChanged;
    lfo1DelaySlider.onValueChange = paramChanged;
    lfo1RateSlider.onValueChange = paramChanged;

    env3AttackSlider.onValueChange = paramChanged;
    env3DecaySlider.onValueChange = paramChanged;
    env3SustainSlider.onValueChange = paramChanged;
    env3ReleaseSlider.onValueChange = paramChanged;
    env4AttackSlider.onValueChange = paramChanged;
    env4DecaySlider.onValueChange = paramChanged;
    env4SustainSlider.onValueChange = paramChanged;
    env4ReleaseSlider.onValueChange = paramChanged;
}

void ToneLowerTab::onParameterChanged()
{
    if (processor == nullptr)
        return;

    // Update parameters from UI
    parameters.dco1Range = dco1RangeCombo.getSelectedId() - 1;
    parameters.dco1Waveform = dco1WaveformCombo.getSelectedId() - 1;
    parameters.dco1Tune = (int) dco1TuneSlider.getValue();
    parameters.dco1LFOMod = (int) dco1LFODepthSlider.getValue();
    parameters.dco1ENVMod = (int) dco1ENVDepthSlider.getValue();

    parameters.dco2Range = dco2RangeCombo.getSelectedId() - 1;
    parameters.dco2Waveform = dco2WaveformCombo.getSelectedId() - 1;
    parameters.dco2Tune = (int) dco2TuneSlider.getValue();
    parameters.dco2CrossMod = dcoCrossModCombo.getSelectedId() - 1;
    parameters.dco2FineTune = (int) dco2FineTuneSlider.getValue();
    parameters.dco2LFOMod = (int) dco2LFODepthSlider.getValue();
    parameters.dco2ENVMod = (int) dco2ENVDepthSlider.getValue();

    parameters.mixerDCO1Level = (int) mixerDCO1LevelSlider.getValue();
    parameters.mixerDCO2Level = (int) mixerDCO2LevelSlider.getValue();
    parameters.mixerENVMod = (int) mixerENVDepthSlider.getValue();

    parameters.vcfHPF = hpfCutoffCombo.getSelectedId() - 1;
    parameters.vcfCutoff = (int) vcfCutoffSlider.getValue();
    parameters.vcfResonance = (int) vcfResonanceSlider.getValue();
    parameters.vcfENVMod = (int) vcfENVDepthSlider.getValue();
    parameters.vcfLFOMod = (int) vcfLFO1DepthSlider.getValue();
    parameters.vcfKeyFollow = (int) vcfKeyFollowSlider.getValue();

    parameters.vcaLevel = (int) vcaLevelSlider.getValue();

    parameters.env1Time1 = (int) env1Time1Slider.getValue();
    parameters.env1Time2 = (int) env1Time2Slider.getValue();
    parameters.env1Level1 = (int) env1Level1Slider.getValue();
    parameters.env1Time4 = (int) env1Time4Slider.getValue();

    parameters.env2Time1 = (int) env2Time1Slider.getValue();
    parameters.env2Time2 = (int) env2Time2Slider.getValue();
    parameters.env2Level1 = (int) env2Level1Slider.getValue();
    parameters.env2Time4 = (int) env2Time4Slider.getValue();

    parameters.lfo1Waveform = lfo1WaveformCombo.getSelectedId() - 1;
    parameters.lfo1Delay = (int) lfo1DelaySlider.getValue();
    parameters.lfo1Rate = (int) lfo1RateSlider.getValue();
    parameters.chorusMode = chorusCombo.getSelectedId() - 1;
    parameters.env3Attack = (int) env3AttackSlider.getValue();
    parameters.env3Decay = (int) env3DecaySlider.getValue();
    parameters.env3Sustain = (int) env3SustainSlider.getValue();
    parameters.env3Release = (int) env3ReleaseSlider.getValue();
    parameters.env4Attack = (int) env4AttackSlider.getValue();
    parameters.env4Decay = (int) env4DecaySlider.getValue();
    parameters.env4Sustain = (int) env4SustainSlider.getValue();
    parameters.env4Release = (int) env4ReleaseSlider.getValue();
    
    // New params
    parameters.dcoDynamics = dcoDynamicsCombo.getSelectedId() - 1;
    parameters.dcoENVMode = dcoENVModeCombo.getSelectedId() - 1;
    parameters.mixerDynamics = mixerDynamicsCombo.getSelectedId() - 1;
    parameters.mixerENVMode = mixerENVModeCombo.getSelectedId() - 1;
    parameters.vcfDynamics = vcfDynamicsCombo.getSelectedId() - 1;
    parameters.vcfENVMode = vcfENVModeCombo.getSelectedId() - 1;
    parameters.vcaMode = vcaModeCombo.getSelectedId() - 1;
    parameters.vcaDynamics = vcaDynamicsCombo.getSelectedId() - 1;
    parameters.env1KeyFollow = static_cast<int>(env1KeyFollowSlider.getValue());
    parameters.env2KeyFollow = static_cast<int>(env2KeyFollowSlider.getValue());

    // Send to synth (lower tone)
    processor->sendToneToSynth(false);
    DBG("ToneLowerTab: Parameter changed - sent to synth");

    // Update envelope editors to reflect current parameter values
    env1Editor.setEnvelope7Stage(parameters.env1Time1,
                                 parameters.env1Level1,
                                 parameters.env1Time2,
                                 parameters.env1Level2,
                                 parameters.env1Time3,
                                 parameters.env1Level3,
                                 parameters.env1Time4);
    env1Editor.setKeyFollow(parameters.env1KeyFollow);
    env2Editor.setEnvelope7Stage(parameters.env2Time1,
                                 parameters.env2Level1,
                                 parameters.env2Time2,
                                 parameters.env2Level2,
                                 parameters.env2Time3,
                                 parameters.env2Level3,
                                 parameters.env2Time4);
    env2Editor.setKeyFollow(parameters.env2KeyFollow);
}

void ToneLowerTab::paintOverChildren(juce::Graphics& g)
{
    if (editor == nullptr) return;
    // MIDI Learn status indicators (similar to upper tab)
    // draw a small indicator dot for mapped sliders
    // Note: minimal to avoid heavy dependencies
    auto& learn = editor->getMidiLearnManager();
    for (auto& kv : sliderParamIds)
    {
        auto* s = kv.first;
        const auto& paramId = kv.second;
        auto r = s->getBounds();
        if (learn.getMappingForParam(paramId) != nullptr)
        {
            g.setColour(juce::Colours::green);
            g.fillEllipse((float)r.getRight() - 10.0f, (float)r.getY() + 4.0f, 6.0f, 6.0f);
        }
    }
}

void ToneLowerTab::registerMidiForSlider(juce::Slider& slider, const juce::String& paramId)
{
    sliderParamIds[&slider] = paramId;
    sliderSinkRegistered[&slider] = false;
    slider.addMouseListener(this, true);
    slider.setTooltip(paramId + " — Right-click for MIDI Learn");
}

void ToneLowerTab::registerMidiForCombo(juce::ComboBox& combo, const juce::String& paramId)
{
    comboParamIds[&combo] = paramId;
    comboSinkRegistered[&combo] = false;
    combo.addMouseListener(this, true);
    combo.setTooltip(paramId + " — Right-click for MIDI Learn");
}

void ToneLowerTab::showLearnMenuForSlider(juce::Slider* slider)
{
    if (editor == nullptr || slider == nullptr) return;
    auto& learn = editor->getMidiLearnManager();
    auto paramId = sliderParamIds[slider];

    if (!sliderSinkRegistered[slider])
    {
        learn.registerParamSink(paramId, [slider](int v){
            juce::MessageManager::callAsync([slider, v](){ slider->setValue(v, juce::sendNotificationSync); });
        });
        sliderSinkRegistered[slider] = true;
    }

    juce::PopupMenu menu;
    menu.addItem(1, "MIDI Learn");
    menu.addItem(2, "Clear MIDI Assignment");
    if (learn.getMappingForParam(paramId) != nullptr)
    {
        menu.addSeparator();
        menu.addItem(3, "Edit Mapping...");
    }

    menu.showMenuAsync(juce::PopupMenu::Options(), [this, paramId, slider](int result)
    {
        if (editor == nullptr) return;
        auto& learnMgr = editor->getMidiLearnManager();
        if (result == 1) learnMgr.startLearn(paramId);
        if (result == 2) { learnMgr.removeMapping(paramId); }
        if (result == 3)
        {
            auto* mm = new MidiMappingEditor(paramId, learnMgr);
            mm->onClose = [mm]() { delete mm; };
            mm->addToDesktop(juce::ComponentPeer::windowIsTemporary | juce::ComponentPeer::windowHasTitleBar);
            mm->centreWithSize(520, 320);
            mm->setVisible(true);
        }
        repaint();
    });
}

void ToneLowerTab::showLearnMenuForCombo(juce::ComboBox* combo)
{
    if (editor == nullptr || combo == nullptr) return;
    auto& learn = editor->getMidiLearnManager();
    auto paramId = comboParamIds[combo];

    if (!comboSinkRegistered[combo])
    {
        learn.registerParamSink(paramId, [combo](int v){
            juce::MessageManager::callAsync([combo, v](){
                const int items = juce::jmax(1, combo->getNumItems());
                int idx = juce::jlimit(0, items - 1, (int) std::round(juce::jmap((double)v, 0.0, 127.0, 0.0, (double)(items - 1)))) ;
                combo->setSelectedItemIndex(idx, juce::sendNotificationSync);
            });
        });
        comboSinkRegistered[combo] = true;
    }

    juce::PopupMenu menu;
    menu.addItem(1, "MIDI Learn");
    menu.addItem(2, "Clear MIDI Assignment");
    if (learn.getMappingForParam(paramId) != nullptr)
    {
        menu.addSeparator();
        menu.addItem(3, "Edit Mapping...");
    }

    menu.showMenuAsync(juce::PopupMenu::Options(), [this, paramId](int result)
    {
        if (editor == nullptr) return;
        auto& learnMgr = editor->getMidiLearnManager();
        if (result == 1) learnMgr.startLearn(paramId);
        if (result == 2) { learnMgr.removeMapping(paramId); }
        if (result == 3)
        {
            auto* mm = new MidiMappingEditor(paramId, learnMgr);
            mm->onClose = [mm]() { delete mm; };
            mm->addToDesktop(juce::ComponentPeer::windowIsTemporary | juce::ComponentPeer::windowHasTitleBar);
            mm->centreWithSize(520, 320);
            mm->setVisible(true);
        }
        repaint();
    });
}

void ToneLowerTab::mouseDown(const juce::MouseEvent& e)
{
    if (!e.mods.isRightButtonDown())
        return;

    juce::Component* src = e.originalComponent;
    if (src == nullptr) src = e.eventComponent;
    
    // Check if click is on a control (for MIDI Learn)
    if (auto* s = dynamic_cast<juce::Slider*>(src))
    {
        if (sliderParamIds.find(s) != sliderParamIds.end())
        {
            showLearnMenuForSlider(s);
            return;
        }
    }
    if (auto* c = dynamic_cast<juce::ComboBox*>(src))
    {
        if (comboParamIds.find(c) != comboParamIds.end())
        {
            showLearnMenuForCombo(c);
            return;
        }
    }
    if (src != nullptr)
    {
        if (auto* parentSlider = src->findParentComponentOfClass<juce::Slider>())
        {
            if (sliderParamIds.find(parentSlider) != sliderParamIds.end())
            {
                showLearnMenuForSlider(parentSlider);
                return;
            }
        }
        if (auto* parentCombo = src->findParentComponentOfClass<juce::ComboBox>())
        {
            if (comboParamIds.find(parentCombo) != comboParamIds.end())
            {
                showLearnMenuForCombo(parentCombo);
                return;
            }
        }
    }
    
    // Show copy/paste context menu for background clicks
    showCopyPasteMenu(e);
}

void ToneLowerTab::showCopyPasteMenu(const juce::MouseEvent& e)
{
    auto& clipboard = ToneClipboard::getInstance();
    
    juce::PopupMenu menu;
    juce::PopupMenu copyMenu;
    juce::PopupMenu pasteMenu;
    
    // Copy submenu
    copyMenu.addItem(100, "Copy All Parameters");
    copyMenu.addSeparator();
    copyMenu.addItem(101, "Copy DCO-1");
    copyMenu.addItem(102, "Copy DCO-2");
    copyMenu.addItem(103, "Copy Mixer");
    copyMenu.addItem(104, "Copy VCF");
    copyMenu.addItem(105, "Copy VCA");
    copyMenu.addItem(106, "Copy LFO");
    copyMenu.addItem(107, "Copy ENV-1");
    copyMenu.addItem(108, "Copy ENV-2");
    
    menu.addSubMenu("Copy from Lower Tone", copyMenu);
    
    // Paste submenu (only enabled if clipboard has data)
    bool hasPaste = clipboard.hasClipboardData();
    juce::String pasteLabel = hasPaste 
        ? "Paste from " + clipboard.getSourceInfo() + " (" + clipboard.getSectionName(clipboard.getCopiedSection()) + ")"
        : "Paste (empty clipboard)";
    
    pasteMenu.addItem(200, pasteLabel, hasPaste);
    if (hasPaste)
    {
        pasteMenu.addSeparator();
        pasteMenu.addItem(201, "Paste All", clipboard.getCopiedSection() == ToneClipboard::Section::All);
        pasteMenu.addItem(202, "Paste DCO-1 only", clipboard.getCopiedSection() == ToneClipboard::Section::DCO1 || clipboard.getCopiedSection() == ToneClipboard::Section::All);
        pasteMenu.addItem(203, "Paste DCO-2 only", clipboard.getCopiedSection() == ToneClipboard::Section::DCO2 || clipboard.getCopiedSection() == ToneClipboard::Section::All);
        pasteMenu.addItem(204, "Paste Mixer only", clipboard.getCopiedSection() == ToneClipboard::Section::Mixer || clipboard.getCopiedSection() == ToneClipboard::Section::All);
        pasteMenu.addItem(205, "Paste VCF only", clipboard.getCopiedSection() == ToneClipboard::Section::VCF || clipboard.getCopiedSection() == ToneClipboard::Section::All);
        pasteMenu.addItem(206, "Paste VCA only", clipboard.getCopiedSection() == ToneClipboard::Section::VCA || clipboard.getCopiedSection() == ToneClipboard::Section::All);
        pasteMenu.addItem(207, "Paste LFO only", clipboard.getCopiedSection() == ToneClipboard::Section::LFO || clipboard.getCopiedSection() == ToneClipboard::Section::All);
        pasteMenu.addItem(208, "Paste ENV-1 only", clipboard.getCopiedSection() == ToneClipboard::Section::ENV1 || clipboard.getCopiedSection() == ToneClipboard::Section::All);
        pasteMenu.addItem(209, "Paste ENV-2 only", clipboard.getCopiedSection() == ToneClipboard::Section::ENV2 || clipboard.getCopiedSection() == ToneClipboard::Section::All);
    }
    
    menu.addSubMenu("Paste to Lower Tone", pasteMenu, hasPaste);
    
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea(juce::Rectangle<int>(e.getScreenX(), e.getScreenY(), 1, 1)),
        [this](int result)
        {
            auto& cb = ToneClipboard::getInstance();
            
            // Copy actions (100-108)
            if (result >= 100 && result <= 108)
            {
                ToneClipboard::Section section = ToneClipboard::Section::All;
                switch (result)
                {
                    case 100: section = ToneClipboard::Section::All; break;
                    case 101: section = ToneClipboard::Section::DCO1; break;
                    case 102: section = ToneClipboard::Section::DCO2; break;
                    case 103: section = ToneClipboard::Section::Mixer; break;
                    case 104: section = ToneClipboard::Section::VCF; break;
                    case 105: section = ToneClipboard::Section::VCA; break;
                    case 106: section = ToneClipboard::Section::LFO; break;
                    case 107: section = ToneClipboard::Section::ENV1; break;
                    case 108: section = ToneClipboard::Section::ENV2; break;
                }
                cb.copySection(parameters, section, false);  // false = Lower Tone
            }
            // Paste actions (200-209)
            else if (result >= 200 && result <= 209)
            {
                ToneParams newParams = parameters;
                
                if (result == 200 || result == 201)
                {
                    // Paste whatever was copied
                    cb.paste(newParams);
                }
                else
                {
                    // Paste all (since specific section paste requires clipboard modification)
                    cb.paste(newParams);
                }
                
                setParameters(newParams);
                onParameterChanged();
            }
        });
}

void ToneLowerTab::updateWaveDisplays()
{
    // DCO1
    const int d1Wave = dco1WaveformCombo.getSelectedId() - 1;
    dco1WaveDisplay.setOscillatorParams(d1Wave, 64, 0, 64);

    // DCO2
    const int d2Wave = dco2WaveformCombo.getSelectedId() - 1;
    const int cross = dcoCrossModCombo.getSelectedId() - 1;
    const int fine = (int) dco2FineTuneSlider.getValue();
    dco2WaveDisplay.setOscillatorParams(d2Wave, 64, cross, fine);

    // Mixer combined
    mixerWaveDisplay.setDCO1Params(d1Wave, (int)mixerDCO1LevelSlider.getValue());
    mixerWaveDisplay.setDCO2Params(d2Wave, (int)mixerDCO2LevelSlider.getValue(), cross, fine);
}
