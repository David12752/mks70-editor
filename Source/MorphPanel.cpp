/*
  ==============================================================================

    MorphPanel.cpp
    Morph/Crossfade panel implementation

  ==============================================================================
*/

#include "MorphPanel.h"
#include "PluginProcessor.h"

//==============================================================================
MorphPanel::MorphPanel()
{
    setupUI();
    morphEngine.addListener(this);
}

MorphPanel::~MorphPanel()
{
    morphEngine.removeListener(this);
    morphEngine.stopAutoMorph();
}

void MorphPanel::setProcessor(MKS70EditorAudioProcessor* proc)
{
    processor = proc;
}

//==============================================================================
// UI Setup

void MorphPanel::setupUI()
{
    // Header
    titleLabel.setText("MORPH CONTROL", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    closeButton.setButtonText("X");
    closeButton.addListener(this);
    addAndMakeVisible(closeButton);
    
    setupSourcePanels();
    setupMorphSlider();
    setupMidiControl();
    setupAutoMorph();
    setupExclusions();
    setupCurves();
    setupSnapshots();
}

void MorphPanel::setupSourcePanels()
{
    // Source A
    sourceAGroup.setText("SOURCE A");
    sourceAGroup.setTextLabelPosition(juce::Justification::centredTop);
    addAndMakeVisible(sourceAGroup);
    
    sourceALabel.setText("(No patch loaded)", juce::dontSendNotification);
    sourceALabel.setFont(juce::Font(14.0f, juce::Font::bold));
    sourceALabel.setJustificationType(juce::Justification::centred);
    sourceALabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);
    addAndMakeVisible(sourceALabel);
    
    loadAButton.setButtonText("Load from Library");
    loadAButton.addListener(this);
    addAndMakeVisible(loadAButton);
    
    currentToAButton.setButtonText("Current -> A");
    currentToAButton.addListener(this);
    addAndMakeVisible(currentToAButton);
    
    // Source B
    sourceBGroup.setText("SOURCE B");
    sourceBGroup.setTextLabelPosition(juce::Justification::centredTop);
    addAndMakeVisible(sourceBGroup);
    
    sourceBLabel.setText("(No patch loaded)", juce::dontSendNotification);
    sourceBLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    sourceBLabel.setJustificationType(juce::Justification::centred);
    sourceBLabel.setColour(juce::Label::textColourId, juce::Colours::lightcoral);
    addAndMakeVisible(sourceBLabel);
    
    loadBButton.setButtonText("Load from Library");
    loadBButton.addListener(this);
    addAndMakeVisible(loadBButton);
    
    currentToBButton.setButtonText("Current -> B");
    currentToBButton.addListener(this);
    addAndMakeVisible(currentToBButton);
}

void MorphPanel::setupMorphSlider()
{
    // Main slider
    morphSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    morphSlider.setRange(0.0, 1.0, 0.001);
    morphSlider.setValue(0.0);
    morphSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    morphSlider.addListener(this);
    addAndMakeVisible(morphSlider);
    
    // Position label
    morphPositionLabel.setText("0%", juce::dontSendNotification);
    morphPositionLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    morphPositionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(morphPositionLabel);
    
    // Source indicators
    sourceAIndicator.setText("A <--", juce::dontSendNotification);
    sourceAIndicator.setJustificationType(juce::Justification::left);
    sourceAIndicator.setColour(juce::Label::textColourId, juce::Colours::lightblue);
    addAndMakeVisible(sourceAIndicator);
    
    sourceBIndicator.setText("--> B", juce::dontSendNotification);
    sourceBIndicator.setJustificationType(juce::Justification::right);
    sourceBIndicator.setColour(juce::Label::textColourId, juce::Colours::lightcoral);
    addAndMakeVisible(sourceBIndicator);
}

void MorphPanel::setupMidiControl()
{
    midiGroup.setText("MIDI Control");
    midiGroup.setTextLabelPosition(juce::Justification::centredTop);
    addAndMakeVisible(midiGroup);
    
    midiCCLabel.setText("Link to MIDI CC:", juce::dontSendNotification);
    midiCCLabel.setFont(juce::FontOptions(13.0f));
    addAndMakeVisible(midiCCLabel);
    
    midiCCCombo.addItem("None", 1);
    for (int i = 1; i <= 127; i++)
        midiCCCombo.addItem("CC " + juce::String(i), i + 1);
    midiCCCombo.setSelectedId(1);
    midiCCCombo.addListener(this);
    addAndMakeVisible(midiCCCombo);
    
    linkToModWheelToggle.setButtonText("Link to Mod Wheel (CC 1)");
    linkToModWheelToggle.addListener(this);
    addAndMakeVisible(linkToModWheelToggle);

    midiLearnToggle.setButtonText("MIDI Learn");
    midiLearnToggle.addListener(this);
    addAndMakeVisible(midiLearnToggle);
}

void MorphPanel::setupAutoMorph()
{
    autoMorphGroup.setText("AUTO MORPH");
    autoMorphGroup.setTextLabelPosition(juce::Justification::centredTop);
    addAndMakeVisible(autoMorphGroup);
    
    autoMorphModeLabel.setText("Mode:", juce::dontSendNotification);
    addAndMakeVisible(autoMorphModeLabel);
    
    populateModeCombo();
    autoMorphModeCombo.setSelectedId(1); // Linear
    autoMorphModeCombo.addListener(this);
    addAndMakeVisible(autoMorphModeCombo);
    
    durationLabel.setText("Duration:", juce::dontSendNotification);
    addAndMakeVisible(durationLabel);
    
    durationSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    durationSlider.setRange(0.1, 30.0, 0.1);
    durationSlider.setValue(2.0);
    durationSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    durationSlider.addListener(this);
    addAndMakeVisible(durationSlider);
    
    durationValueLabel.setText("2.0 sec", juce::dontSendNotification);
    durationValueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(durationValueLabel);
    
    startAutoMorphButton.setButtonText("START");
    startAutoMorphButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green.darker());
    startAutoMorphButton.addListener(this);
    addAndMakeVisible(startAutoMorphButton);
    
    stopAutoMorphButton.setButtonText("STOP");
    stopAutoMorphButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red.darker());
    stopAutoMorphButton.addListener(this);
    stopAutoMorphButton.setEnabled(false);
    addAndMakeVisible(stopAutoMorphButton);
    
    oneShotToggle.setButtonText("One-shot");
    oneShotToggle.setRadioGroupId(1001);
    oneShotToggle.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(oneShotToggle);
    
    loopToggle.setButtonText("Loop");
    loopToggle.setRadioGroupId(1001);
    addAndMakeVisible(loopToggle);
    
    pingPongToggle.setButtonText("Ping-Pong");
    pingPongToggle.setRadioGroupId(1001);
    addAndMakeVisible(pingPongToggle);
}

void MorphPanel::setupExclusions()
{
    exclusionsGroup.setText("PARAMETER EXCLUSIONS");
    exclusionsGroup.setTextLabelPosition(juce::Justification::centredTop);
    addAndMakeVisible(exclusionsGroup);
    
    exclusionsLabel.setText("Exclude from morphing:", juce::dontSendNotification);
    exclusionsLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(exclusionsLabel);
    
    exclusionsViewport.setViewedComponent(&exclusionsContent, false);
    exclusionsViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(exclusionsViewport);
    
    createExclusionToggles();
}

void MorphPanel::setupCurves()
{
    curvesGroup.setText("MORPH CURVES");
    curvesGroup.setTextLabelPosition(juce::Justification::centredTop);
    addAndMakeVisible(curvesGroup);
    
    curvesLabel.setText("Set curve per section:", juce::dontSendNotification);
    curvesLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(curvesLabel);
    
    // DCO Section
    dcoSectionLabel.setText("DCO Section:", juce::dontSendNotification);
    addAndMakeVisible(dcoSectionLabel);
    populateCurveCombo(dcoSectionCurveCombo);
    dcoSectionCurveCombo.addListener(this);
    addAndMakeVisible(dcoSectionCurveCombo);
    
    // VCF Section
    vcfSectionLabel.setText("VCF Section:", juce::dontSendNotification);
    addAndMakeVisible(vcfSectionLabel);
    populateCurveCombo(vcfSectionCurveCombo);
    vcfSectionCurveCombo.addListener(this);
    addAndMakeVisible(vcfSectionCurveCombo);
    
    // ENV Section
    envSectionLabel.setText("ENV Section:", juce::dontSendNotification);
    addAndMakeVisible(envSectionLabel);
    populateCurveCombo(envSectionCurveCombo);
    envSectionCurveCombo.addListener(this);
    addAndMakeVisible(envSectionCurveCombo);
    
    // VCA Section
    vcaSectionLabel.setText("VCA Section:", juce::dontSendNotification);
    addAndMakeVisible(vcaSectionLabel);
    populateCurveCombo(vcaSectionCurveCombo);
    vcaSectionCurveCombo.addListener(this);
    addAndMakeVisible(vcaSectionCurveCombo);
    
    // LFO Section
    lfoSectionLabel.setText("LFO Section:", juce::dontSendNotification);
    addAndMakeVisible(lfoSectionLabel);
    populateCurveCombo(lfoSectionCurveCombo);
    lfoSectionCurveCombo.addListener(this);
    addAndMakeVisible(lfoSectionCurveCombo);
    
    // Mixer Section
    mixerSectionLabel.setText("Mixer Section:", juce::dontSendNotification);
    addAndMakeVisible(mixerSectionLabel);
    populateCurveCombo(mixerSectionCurveCombo);
    mixerSectionCurveCombo.addListener(this);
    addAndMakeVisible(mixerSectionCurveCombo);
}

void MorphPanel::setupSnapshots()
{
    snapshotsGroup.setText("MORPH SNAPSHOTS");
    snapshotsGroup.setTextLabelPosition(juce::Justification::centredTop);
    addAndMakeVisible(snapshotsGroup);
    
    snapshotsLabel.setText("Saved morph positions:", juce::dontSendNotification);
    snapshotsLabel.setFont(juce::FontOptions(12.0f));
    addAndMakeVisible(snapshotsLabel);
    
    // Create list model
    snapshotsListModel = std::make_unique<SnapshotsListModel>(snapshotManager);
    snapshotsList.setModel(snapshotsListModel.get());
    snapshotsList.setRowHeight(24);
    addAndMakeVisible(snapshotsList);
    
    captureSnapshotButton.setButtonText("Capture");
    captureSnapshotButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green.darker());
    captureSnapshotButton.addListener(this);
    addAndMakeVisible(captureSnapshotButton);
    
    recallSnapshotButton.setButtonText("Recall");
    recallSnapshotButton.addListener(this);
    addAndMakeVisible(recallSnapshotButton);
    
    deleteSnapshotButton.setButtonText("Delete");
    deleteSnapshotButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red.darker());
    deleteSnapshotButton.addListener(this);
    addAndMakeVisible(deleteSnapshotButton);
    
    exportSnapshotsButton.setButtonText("Export All");
    exportSnapshotsButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange.darker());
    exportSnapshotsButton.addListener(this);
    addAndMakeVisible(exportSnapshotsButton);
    
    clearSnapshotsButton.setButtonText("Clear All");
    clearSnapshotsButton.addListener(this);
    addAndMakeVisible(clearSnapshotsButton);
    
    sortSnapshotsCombo.addItem("Sort by Position", 1);
    sortSnapshotsCombo.addItem("Sort by Name", 2);
    sortSnapshotsCombo.addItem("Sort by Time", 3);
    sortSnapshotsCombo.setSelectedId(1);
    sortSnapshotsCombo.addListener(this);
    addAndMakeVisible(sortSnapshotsCombo);
}

void MorphPanel::createExclusionToggles()
{
    exclusionToggles.clear();
    
    // Common exclusions
    std::vector<juce::String> commonExclusions = {
        "dco1Range", "dco2Range", "keyMode", "assignMode",
        "toneSel", "splitPoint", "arpOn", "tempo"
    };
    
    int yPos = 5;
    for (const auto& paramName : commonExclusions)
    {
        auto toggle = std::make_unique<juce::ToggleButton>(paramName);
        toggle->setBounds(5, yPos, 150, 24);
        toggle->addListener(this);
        exclusionsContent.addAndMakeVisible(toggle.get());
        exclusionToggles.push_back(std::move(toggle));
        yPos += 28;
    }
    
    exclusionsContent.setSize(160, yPos);
}

//==============================================================================
// Painting and Layout

void MorphPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Draw border
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 2);
    
    // Draw morph slider track decoration
    auto sliderBounds = morphSlider.getBounds();
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(sliderBounds.toFloat().reduced(0, 10), 5.0f);
    
    // Draw position marker
    float position = static_cast<float>(morphSlider.getValue());
    int markerX = sliderBounds.getX() + static_cast<int>(position * sliderBounds.getWidth());
    g.setColour(juce::Colours::yellow);
    g.fillEllipse(markerX - 8, sliderBounds.getCentreY() - 8, 16, 16);
    g.setColour(juce::Colours::white);
    g.drawEllipse(markerX - 8, sliderBounds.getCentreY() - 8, 16, 16, 2.0f);
}

void MorphPanel::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Header
    auto headerArea = area.removeFromTop(40);
    closeButton.setBounds(headerArea.removeFromRight(40));
    titleLabel.setBounds(headerArea);
    
    area.removeFromTop(10);
    
    // Source panels
    auto sourcesArea = area.removeFromTop(100);
    auto sourceAArea = sourcesArea.removeFromLeft(sourcesArea.getWidth() / 2 - 20);
    sourcesArea.removeFromLeft(40); // Gap
    auto sourceBArea = sourcesArea;
    
    sourceAGroup.setBounds(sourceAArea);
    auto sourceAContent = sourceAArea.reduced(10, 25);
    sourceALabel.setBounds(sourceAContent.removeFromTop(24));
    sourceAContent.removeFromTop(5);
    auto sourceAButtons = sourceAContent.removeFromTop(30);
    loadAButton.setBounds(sourceAButtons.removeFromLeft(sourceAButtons.getWidth() / 2 - 5));
    sourceAButtons.removeFromLeft(10);
    currentToAButton.setBounds(sourceAButtons);
    
    sourceBGroup.setBounds(sourceBArea);
    auto sourceBContent = sourceBArea.reduced(10, 25);
    sourceBLabel.setBounds(sourceBContent.removeFromTop(24));
    sourceBContent.removeFromTop(5);
    auto sourceBButtons = sourceBContent.removeFromTop(30);
    loadBButton.setBounds(sourceBButtons.removeFromLeft(sourceBButtons.getWidth() / 2 - 5));
    sourceBButtons.removeFromLeft(10);
    currentToBButton.setBounds(sourceBButtons);
    
    area.removeFromTop(15);
    
    // Morph slider
    auto sliderArea = area.removeFromTop(80);
    auto indicatorsArea = sliderArea.removeFromTop(24);
    sourceAIndicator.setBounds(indicatorsArea.removeFromLeft(50));
    sourceBIndicator.setBounds(indicatorsArea.removeFromRight(50));
    morphPositionLabel.setBounds(indicatorsArea);
    
    sliderArea.removeFromTop(5);
    morphSlider.setBounds(sliderArea.removeFromTop(40));
    
    area.removeFromTop(10);
    
    // Four columns: MIDI Control | Auto Morph | Exclusions & Curves | Snapshots
    auto columnsArea = area;
    auto midiArea = columnsArea.removeFromLeft(columnsArea.getWidth() / 4 - 10);
    columnsArea.removeFromLeft(12);
    auto autoMorphArea = columnsArea.removeFromLeft(columnsArea.getWidth() / 3 - 8);
    columnsArea.removeFromLeft(12);
    auto middleColumnArea = columnsArea.removeFromLeft(columnsArea.getWidth() / 2 - 6);
    columnsArea.removeFromLeft(12);
    auto snapshotsArea = columnsArea;
    
    // MIDI Control
    midiGroup.setBounds(midiArea);
    auto midiContent = midiArea.reduced(10, 25);
    auto midiCCRow = midiContent.removeFromTop(24);
    midiCCLabel.setBounds(midiCCRow.removeFromLeft(120));
    midiCCCombo.setBounds(midiCCRow);
    midiContent.removeFromTop(5);
    linkToModWheelToggle.setBounds(midiContent.removeFromTop(24));
    midiContent.removeFromTop(5);
    midiLearnToggle.setBounds(midiContent.removeFromTop(24));
    
    // Auto Morph
    autoMorphGroup.setBounds(autoMorphArea);
    auto autoMorphContent = autoMorphArea.reduced(10, 25);
    auto modeRow = autoMorphContent.removeFromTop(24);
    autoMorphModeLabel.setBounds(modeRow.removeFromLeft(50));
    autoMorphModeCombo.setBounds(modeRow);
    autoMorphContent.removeFromTop(5);
    
    auto durationRow = autoMorphContent.removeFromTop(24);
    durationLabel.setBounds(durationRow.removeFromLeft(70));
    durationValueLabel.setBounds(durationRow.removeFromRight(60));
    durationSlider.setBounds(durationRow);
    autoMorphContent.removeFromTop(5);
    
    auto buttonsRow = autoMorphContent.removeFromTop(30);
    startAutoMorphButton.setBounds(buttonsRow.removeFromLeft(buttonsRow.getWidth() / 2 - 5));
    buttonsRow.removeFromLeft(10);
    stopAutoMorphButton.setBounds(buttonsRow);
    autoMorphContent.removeFromTop(5);
    
    auto togglesRow = autoMorphContent.removeFromTop(24);
    int toggleWidth = togglesRow.getWidth() / 3 - 5;
    oneShotToggle.setBounds(togglesRow.removeFromLeft(toggleWidth));
    togglesRow.removeFromLeft(8);
    loopToggle.setBounds(togglesRow.removeFromLeft(toggleWidth));
    togglesRow.removeFromLeft(8);
    pingPongToggle.setBounds(togglesRow);
    
    // Middle column: Exclusions and Curves
    auto exclusionsArea = middleColumnArea.removeFromTop(middleColumnArea.getHeight() / 2 - 7);
    middleColumnArea.removeFromTop(15);
    auto curvesArea = middleColumnArea;
    
    // Exclusions
    exclusionsGroup.setBounds(exclusionsArea);
    auto exclusionsContent = exclusionsArea.reduced(10, 25);
    exclusionsLabel.setBounds(exclusionsContent.removeFromTop(20));
    exclusionsViewport.setBounds(exclusionsContent);
    
    // Curves
    curvesGroup.setBounds(curvesArea);
    auto curvesContent = curvesArea.reduced(10, 25);
    curvesLabel.setBounds(curvesContent.removeFromTop(20));
    curvesContent.removeFromTop(5);
    
    auto curveRow = curvesContent.removeFromTop(24);
    dcoSectionLabel.setBounds(curveRow.removeFromLeft(100));
    dcoSectionCurveCombo.setBounds(curveRow);
    curvesContent.removeFromTop(3);
    
    curveRow = curvesContent.removeFromTop(24);
    vcfSectionLabel.setBounds(curveRow.removeFromLeft(100));
    vcfSectionCurveCombo.setBounds(curveRow);
    curvesContent.removeFromTop(3);
    
    curveRow = curvesContent.removeFromTop(24);
    envSectionLabel.setBounds(curveRow.removeFromLeft(100));
    envSectionCurveCombo.setBounds(curveRow);
    curvesContent.removeFromTop(3);
    
    curveRow = curvesContent.removeFromTop(24);
    vcaSectionLabel.setBounds(curveRow.removeFromLeft(100));
    vcaSectionCurveCombo.setBounds(curveRow);
    curvesContent.removeFromTop(3);
    
    curveRow = curvesContent.removeFromTop(24);
    lfoSectionLabel.setBounds(curveRow.removeFromLeft(100));
    lfoSectionCurveCombo.setBounds(curveRow);
    curvesContent.removeFromTop(3);
    
    curveRow = curvesContent.removeFromTop(24);
    mixerSectionLabel.setBounds(curveRow.removeFromLeft(100));
    mixerSectionCurveCombo.setBounds(curveRow);
    
    // Snapshots panel
    snapshotsGroup.setBounds(snapshotsArea);
    auto snapshotsContent = snapshotsArea.reduced(10, 25);
    snapshotsLabel.setBounds(snapshotsContent.removeFromTop(20));
    snapshotsContent.removeFromTop(5);
    
    // Buttons row 1
    auto snapshotButtons1 = snapshotsContent.removeFromTop(28);
    captureSnapshotButton.setBounds(snapshotButtons1.removeFromLeft(snapshotButtons1.getWidth() / 2 - 3));
    snapshotButtons1.removeFromLeft(6);
    recallSnapshotButton.setBounds(snapshotButtons1);
    snapshotsContent.removeFromTop(5);
    
    // Buttons row 2
    auto snapshotButtons2 = snapshotsContent.removeFromTop(28);
    deleteSnapshotButton.setBounds(snapshotButtons2.removeFromLeft(snapshotButtons2.getWidth() / 2 - 3));
    snapshotButtons2.removeFromLeft(6);
    clearSnapshotsButton.setBounds(snapshotButtons2);
    snapshotsContent.removeFromTop(5);
    
    // Sort combo
    sortSnapshotsCombo.setBounds(snapshotsContent.removeFromTop(24));
    snapshotsContent.removeFromTop(5);
    
    // Export button
    exportSnapshotsButton.setBounds(snapshotsContent.removeFromTop(28));
    snapshotsContent.removeFromTop(5);
    
    // List
    snapshotsList.setBounds(snapshotsContent);
}

//==============================================================================
// Listeners

void MorphPanel::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &morphSlider)
    {
        float position = static_cast<float>(slider->getValue());
        updateMorphPosition(position);
        applyMorphedPatch();
    }
    else if (slider == &durationSlider)
    {
        durationValueLabel.setText(formatDuration(static_cast<float>(slider->getValue())), 
                                    juce::dontSendNotification);
    }
}

void MorphPanel::buttonClicked(juce::Button* button)
{
    if (button == &closeButton)
    {
        setVisible(false);
    }
    else if (button == &currentToAButton)
    {
        loadCurrentToA();
    }
    else if (button == &currentToBButton)
    {
        loadCurrentToB();
    }
    else if (button == &loadAButton)
    {
        loadSourceA();
    }
    else if (button == &loadBButton)
    {
        loadSourceB();
    }
    else if (button == &startAutoMorphButton)
    {
        if (!morphEngine.hasSources())
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                "Cannot Start",
                "Please load both Source A and Source B patches first.",
                "OK");
            return;
        }
        
        // Determine mode from radio buttons
        MorphEngine::MorphMode mode = MorphEngine::LINEAR;
        if (loopToggle.getToggleState())
        {
            mode = (autoMorphModeCombo.getSelectedId() == 4) ? MorphEngine::LFO_SINE :
                   (autoMorphModeCombo.getSelectedId() == 5) ? MorphEngine::LFO_TRIANGLE :
                   MorphEngine::LINEAR;
        }
        else if (pingPongToggle.getToggleState())
        {
            mode = MorphEngine::PINGPONG;
        }
        else if (autoMorphModeCombo.getSelectedId() == 3)
        {
            mode = MorphEngine::RANDOM_WALK;
        }
        else
        {
            mode = MorphEngine::LINEAR;
        }
        
        float duration = static_cast<float>(durationSlider.getValue());
        morphEngine.startAutoMorph(duration, mode);
        
        autoMorphActive = true;
        startAutoMorphButton.setEnabled(false);
        stopAutoMorphButton.setEnabled(true);
        morphSlider.setEnabled(false);
    }
    else if (button == &stopAutoMorphButton)
    {
        morphEngine.stopAutoMorph();
        autoMorphActive = false;
        startAutoMorphButton.setEnabled(true);
        stopAutoMorphButton.setEnabled(false);
        morphSlider.setEnabled(true);
    }
    else if (button == &linkToModWheelToggle)
    {
        if (button->getToggleState())
            midiCCCombo.setSelectedId(2); // CC 1
    }
    else if (button == &captureSnapshotButton)
    {
        captureCurrentSnapshot();
    }
    else if (button == &recallSnapshotButton)
    {
        int selectedRow = snapshotsList.getSelectedRow();
        if (selectedRow >= 0)
            recallSnapshot(selectedRow);
        else
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                "No Selection", "Please select a snapshot to recall.", "OK");
    }
    else if (button == &deleteSnapshotButton)
    {
        int selectedRow = snapshotsList.getSelectedRow();
        if (selectedRow >= 0)
            deleteSnapshot(selectedRow);
        else
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                "No Selection", "Please select a snapshot to delete.", "OK");
    }
    else if (button == &exportSnapshotsButton)
    {
        exportSnapshots();
    }
    else if (button == &clearSnapshotsButton)
    {
        if (snapshotManager.getSnapshotCount() > 0)
        {
            juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::QuestionIcon,
                "Clear All Snapshots",
                "Are you sure you want to clear all " + juce::String(snapshotManager.getSnapshotCount()) + " snapshots?",
                "Yes", "No", nullptr,
                juce::ModalCallbackFunction::create([this](int result) {
                    if (result == 1) // OK clicked
                    {
                        snapshotManager.clearAllSnapshots();
                        refreshSnapshotsList();
                    }
                }));
        }
    }
    else
    {
        // Check if it's an exclusion toggle
        for (const auto& toggle : exclusionToggles)
        {
            if (button == toggle.get())
            {
                if (toggle->getToggleState())
                    morphEngine.excludeParameter(toggle->getButtonText());
                else
                    morphEngine.includeParameter(toggle->getButtonText());
                break;
            }
        }
    }
}

void MorphPanel::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &dcoSectionCurveCombo)
    {
        applySectionCurve("dco", getCurveFromCombo(*comboBox));
    }
    else if (comboBox == &vcfSectionCurveCombo)
    {
        applySectionCurve("vcf", getCurveFromCombo(*comboBox));
    }
    else if (comboBox == &envSectionCurveCombo)
    {
        applySectionCurve("env", getCurveFromCombo(*comboBox));
    }
    else if (comboBox == &vcaSectionCurveCombo)
    {
        applySectionCurve("vca", getCurveFromCombo(*comboBox));
    }
    else if (comboBox == &lfoSectionCurveCombo)
    {
        applySectionCurve("lfo", getCurveFromCombo(*comboBox));
    }
    else if (comboBox == &mixerSectionCurveCombo)
    {
        applySectionCurve("mixer", getCurveFromCombo(*comboBox));
    }
    else if (comboBox == &sortSnapshotsCombo)
    {
        switch (comboBox->getSelectedId())
        {
            case 1: snapshotManager.sortByPosition(); break;
            case 2: snapshotManager.sortByName(); break;
            case 3: snapshotManager.sortByTimestamp(); break;
        }
        refreshSnapshotsList();
    }
}

void MorphPanel::morphPositionChanged(float newPosition)
{
    // Called by MorphEngine during auto-morph
    morphSlider.setValue(newPosition, juce::dontSendNotification);
    updateMorphPosition(newPosition);
    applyMorphedPatch();
}

void MorphPanel::morphCompleted()
{
    // Auto-morph finished (one-shot mode)
    if (oneShotToggle.getToggleState())
    {
        autoMorphActive = false;
        startAutoMorphButton.setEnabled(true);
        stopAutoMorphButton.setEnabled(false);
        morphSlider.setEnabled(true);
    }
}

//==============================================================================
// Source Loading

void MorphPanel::loadCurrentToA()
{
    if (!processor)
        return;
    
    FullPatch currentPatch;
    currentPatch.toneUpper = processor->getToneUpper();
    currentPatch.toneLower = processor->getToneLower();
    currentPatch.patch = processor->getPatchParameters();
    currentPatch.arp = processor->getArpParameters();
    currentPatch.name = "Current";
    currentPatch.timestamp = juce::Time::getCurrentTime();
    
    morphEngine.setSourceA(currentPatch);
    hasSourceA = true;
    sourceAName = "Current";
    updateSourceLabels();
    
    DBG("MorphPanel: Loaded Current to A");
}

void MorphPanel::loadCurrentToB()
{
    if (!processor)
        return;
    
    FullPatch currentPatch;
    currentPatch.toneUpper = processor->getToneUpper();
    currentPatch.toneLower = processor->getToneLower();
    currentPatch.patch = processor->getPatchParameters();
    currentPatch.arp = processor->getArpParameters();
    currentPatch.name = "Current";
    currentPatch.timestamp = juce::Time::getCurrentTime();
    
    morphEngine.setSourceB(currentPatch);
    hasSourceB = true;
    sourceBName = "Current";
    updateSourceLabels();
    
    DBG("MorphPanel: Loaded Current to B");
}

void MorphPanel::loadSourceA()
{
    // TODO: Open librarian dialog to select patch
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
        "Load Source A",
        "Library integration coming soon. Use 'Current  A' for now.",
        "OK");
}

void MorphPanel::loadSourceB()
{
    // TODO: Open librarian dialog to select patch
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
        "Load Source B",
        "Library integration coming soon. Use 'Current  B' for now.",
        "OK");
}

void MorphPanel::applyMorphedPatch()
{
    if (!processor || !morphEngine.hasSources())
        return;
    
    float position = static_cast<float>(morphSlider.getValue());
    FullPatch morphed = morphEngine.morph(position);
    
    processor->setToneParameters(true, morphed.toneUpper, true);
    processor->setToneParameters(false, morphed.toneLower, true);
    processor->setPatchParameters(morphed.patch, true);
    processor->setArpParameters(morphed.arp, false);
    
    DBG("MorphPanel: Applied morphed patch at " + juce::String(position * 100.0f, 1) + "%");
}

//==============================================================================
// Helper Methods

void MorphPanel::updateMorphPosition(float position)
{
    int percentage = static_cast<int>(position * 100.0f);
    morphPositionLabel.setText(juce::String(percentage) + "%", juce::dontSendNotification);
    repaint();
}

void MorphPanel::updateSourceLabels()
{
    sourceALabel.setText(hasSourceA ? sourceAName : "(No patch loaded)", 
                         juce::dontSendNotification);
    sourceBLabel.setText(hasSourceB ? sourceBName : "(No patch loaded)", 
                         juce::dontSendNotification);
}

void MorphPanel::populateCurveCombo(juce::ComboBox& combo)
{
    combo.addItem("Linear", 1);
    combo.addItem("Exponential", 2);
    combo.addItem("Logarithmic", 3);
    combo.addItem("S-Curve", 4);
    combo.addItem("Stepped", 5);
    combo.setSelectedId(1); // Default to Linear
}

void MorphPanel::populateModeCombo()
{
    autoMorphModeCombo.addItem("Linear", 1);
    autoMorphModeCombo.addItem("Ping-Pong", 2);
    autoMorphModeCombo.addItem("Random Walk", 3);
    autoMorphModeCombo.addItem("LFO Sine", 4);
    autoMorphModeCombo.addItem("LFO Triangle", 5);
}

MorphEngine::MorphCurve MorphPanel::getCurveFromCombo(juce::ComboBox& combo)
{
    switch (combo.getSelectedId())
    {
        case 1: return MorphEngine::CURVE_LINEAR;
        case 2: return MorphEngine::CURVE_EXPONENTIAL;
        case 3: return MorphEngine::CURVE_LOGARITHMIC;
        case 4: return MorphEngine::CURVE_SCURVE;
        case 5: return MorphEngine::CURVE_STEPPED;
        default: return MorphEngine::CURVE_LINEAR;
    }
}

void MorphPanel::applySectionCurve(const juce::String& sectionPrefix, MorphEngine::MorphCurve curve)
{
    // Apply curve to all parameters starting with sectionPrefix
    std::vector<juce::String> dcoParams = {
        "dco1Range", "dco1Waveform", "dco1Tune", "dco1LFOMod", "dco1ENVMod",
        "dco2Range", "dco2Waveform", "dco2CrossMod", "dco2Tune", "dco2FineTune",
        "dco2LFOMod", "dco2ENVMod", "dcoDynamics", "dcoENVMode"
    };
    
    std::vector<juce::String> vcfParams = {
        "vcfHPF", "vcfCutoff", "vcfResonance", "vcfLFOMod", "vcfENVMod",
        "vcfKeyFollow", "vcfDynamics", "vcfENVMode"
    };
    
    std::vector<juce::String> envParams = {
        "env1Attack", "env1Decay", "env1Sustain", "env1Release", "env1KeyFollow",
        "env2Attack", "env2Decay", "env2Sustain", "env2Release", "env2KeyFollow"
    };
    
    std::vector<juce::String> vcaParams = {
        "vcaMode", "vcaLevel", "vcaDynamics"
    };
    
    std::vector<juce::String> lfoParams = {
        "lfo1Waveform", "lfo1Delay", "lfo1Rate"
    };
    
    std::vector<juce::String> mixerParams = {
        "mixerDCO1Level", "mixerDCO2Level", "mixerENVMod", "mixerDynamics", "mixerENVMode"
    };
    
    const std::vector<juce::String>* params = nullptr;
    
    if (sectionPrefix == "dco") params = &dcoParams;
    else if (sectionPrefix == "vcf") params = &vcfParams;
    else if (sectionPrefix == "env") params = &envParams;
    else if (sectionPrefix == "vca") params = &vcaParams;
    else if (sectionPrefix == "lfo") params = &lfoParams;
    else if (sectionPrefix == "mixer") params = &mixerParams;
    
    if (params)
    {
        for (const auto& paramName : *params)
        {
            morphEngine.setParameterMorphCurve(paramName, curve);
        }
        DBG("MorphPanel: Applied " + juce::String(static_cast<int>(curve)) + 
            " curve to " + sectionPrefix + " section");
    }
}

juce::String MorphPanel::formatDuration(float seconds)
{
    return juce::String(seconds, 1) + " sec";
}

//==============================================================================
// Snapshot Management

void MorphPanel::captureCurrentSnapshot()
{
    if (!morphEngine.hasSources())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Cannot Capture",
            "Please load both Source A and Source B patches first.",
            "OK");
        return;
    }
    
    float position = static_cast<float>(morphSlider.getValue());
    FullPatch morphedPatch = morphEngine.morph(position);
    
    // Ask for name
    juce::String defaultName = "Morph " + juce::String(snapshotManager.getSnapshotCount() + 1) + 
                               " (" + juce::String(position * 100.0f, 0) + "%)";
    
    juce::String name = defaultName; // In production, use AlertWindow::showInputBox
    
    snapshotManager.captureSnapshot(name, position, morphedPatch);
    refreshSnapshotsList();
    
    DBG("MorphPanel: Captured snapshot at " + juce::String(position * 100.0f, 1) + "%");
}

void MorphPanel::recallSnapshot(int index)
{
    FullPatch patch;
    if (snapshotManager.recallSnapshot(index, patch))
    {
        if (!processor)
            return;
        
        // Apply to processor
        processor->setToneParameters(true, patch.toneUpper, false);
        processor->setToneParameters(false, patch.toneLower, false);
        processor->setPatchParameters(patch.patch, false);
        processor->setArpParameters(patch.arp, false);
        
        // Send to synth
        processor->sendToneToSynth(true);
        processor->sendToneToSynth(false);
        processor->sendPatchToSynth();
        
        // Update slider position
        const auto* snapshot = snapshotManager.getSnapshot(index);
        if (snapshot)
        {
            morphSlider.setValue(snapshot->position, juce::dontSendNotification);
            updateMorphPosition(snapshot->position);
        }
        
        DBG("MorphPanel: Recalled snapshot " + juce::String(index));
    }
}

void MorphPanel::deleteSnapshot(int index)
{
    snapshotManager.deleteSnapshot(index);
    refreshSnapshotsList();
}

void MorphPanel::exportSnapshots()
{
    if (snapshotManager.getSnapshotCount() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
            "No Snapshots",
            "There are no snapshots to export.",
            "OK");
        return;
    }
    
    std::vector<FullPatch> patches;
    snapshotManager.exportAsNewPatches(patches);
    
    // TODO: Add patches to librarian
    // For now, just show success message
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
        "Export Complete",
        "Exported " + juce::String(patches.size()) + " snapshots as new patches.\n" +
        "(Librarian integration coming soon)",
        "OK");
    
    DBG("MorphPanel: Exported " + juce::String(patches.size()) + " snapshots");
}

void MorphPanel::refreshSnapshotsList()
{
    snapshotsList.updateContent();
    snapshotsList.repaint();
}

