#include "AutomationPanel.h"

using namespace juce;

AutomationPanel::AutomationPanel()
{
    // === SECTIONS ===
    timingSection = std::make_unique<GlowingSection>("Timing", DS::Color::Accent::magenta());
    addAndMakeVisible(timingSection.get());
    
    // NEW: Quick Access Section for popular parameters
    quickAccessSection = std::make_unique<GlowingSection>("Quick Access - Gisha Mehira", DS::Color::Accent::cyan());
    addAndMakeVisible(quickAccessSection.get());
    
    parameterSection = std::make_unique<GlowingSection>("Add Parameter Lane", DS::Color::Accent::warning());
    addAndMakeVisible(parameterSection.get());
    
    lanesSection = std::make_unique<GlowingSection>("Automation Lanes", DS::Color::Accent::primary());
    addAndMakeVisible(lanesSection.get());
    
    toolsSection = std::make_unique<GlowingSection>("Tools", DS::Color::textSecondary());
    addAndMakeVisible(toolsSection.get());
    
    // Transport section no longer used - we have a bottom transport bar
    transportSection = std::make_unique<GlowingSection>("Transport", DS::Color::Accent::cyan());
    transportSection->setVisible(false);
    
    // === INFO PANEL ===
    // Info panel removed - space used for Quick Access
    
    // === QUICK ACCESS BUTTONS ===
    createQuickAccessButtons();
    
    // Category labels
    quickFilterLabel.setText("Filter:", dontSendNotification);
    quickFilterLabel.setFont(DS::Font::small());
    quickFilterLabel.setColour(Label::textColourId, AutomationColors::vcf());
    addAndMakeVisible(quickFilterLabel);
    
    quickEnvLabel.setText("Envelope:", dontSendNotification);
    quickEnvLabel.setFont(DS::Font::small());
    quickEnvLabel.setColour(Label::textColourId, AutomationColors::env());
    addAndMakeVisible(quickEnvLabel);
    
    quickLfoLabel.setText("LFO:", dontSendNotification);
    quickLfoLabel.setFont(DS::Font::small());
    quickLfoLabel.setColour(Label::textColourId, AutomationColors::lfo());
    addAndMakeVisible(quickLfoLabel);
    
    quickMixerLabel.setText("Mix:", dontSendNotification);
    quickMixerLabel.setFont(DS::Font::small());
    quickMixerLabel.setColour(Label::textColourId, AutomationColors::mixer());
    addAndMakeVisible(quickMixerLabel);

    // === TRANSPORT BAR (Bottom, DAW style) ===
    // Use simple ASCII text instead of Unicode symbols that may not render
    styleTransportButton(btnRew, "<<");
    btnRew.setTooltip("Rewind to start (Home)");
    styleTransportButton(btnStop, "[]");
    btnStop.setTooltip("Stop playback (Space)");
    styleTransportButton(btnPlay, ">");
    btnPlay.setTooltip("Start playback (Space)");
    styleTransportButton(btnRec, "O", true);  // Red for rec
    btnRec.setTooltip("Record automation (R) - arm lanes first");
    styleTransportButton(btnFwd, ">>");
    btnFwd.setTooltip("Fast forward to end (End)");
    
    addAndMakeVisible(btnRew);
    addAndMakeVisible(btnStop);
    addAndMakeVisible(btnPlay);
    addAndMakeVisible(btnRec);
    addAndMakeVisible(btnFwd);
    
    timeLabel.setFont(DS::Font::mono());
    timeLabel.setColour(Label::textColourId, DS::Color::Accent::cyan());
    timeLabel.setColour(Label::backgroundColourId, DS::Color::bgInput());
    timeLabel.setJustificationType(Justification::centred);
    timeLabel.setText("00:00.000 / 00:45.000", dontSendNotification);
    timeLabel.setTooltip("Current position / total length");
    addAndMakeVisible(timeLabel);

    btnRew.onClick = [this](){ recorder.setPlaybackPosition(0.0); updateTimeLabel(); };
    btnStop.onClick = [this](){ recorder.stopPlayback(); lastPlaybackValues.clear(); };
    btnPlay.onClick = [this](){ lastPlaybackValues.clear(); recorder.startPlayback(); };
    btnRec.onClick = [this]() { 
        if (!recorder.isRecording()) 
            recorder.startRecording(); 
        else 
            recorder.stopRecording(); 
        btnRec.setToggleState(recorder.isRecording(), dontSendNotification);
    };
    btnFwd.onClick = [this](){ recorder.setPlaybackPosition(viewTimeEnd); updateTimeLabel(); };

    // === TIMING ===
    tempoSlider = std::make_unique<StyledSlider>("BPM", StyledSlider::Knob, DS::Color::Accent::magenta());
    tempoSlider->setRange(20.0, 300.0, 0.1);
    tempoSlider->setValue(recorder.getTempo());
    tempoSlider->setTooltip("Tempo for sync and snap grid");
    tempoSlider->getSlider().onValueChange = [this](){ recorder.setTempo(tempoSlider->getValue()); };
    addAndMakeVisible(tempoSlider.get());

    syncMode = std::make_unique<StyledComboBox>("Sync");
    syncMode->addItems({"Internal", "MIDI Clock", "Host Tempo"});
    syncMode->setTooltip("Sync to internal, external MIDI, or DAW host");
    syncMode->getComboBox().onChange = [this]()
    {
        AutomationRecorder::SyncMode m = AutomationRecorder::INTERNAL;
        if (syncMode->getSelectedId() == 2) m = AutomationRecorder::MIDI_CLOCK;
        else if (syncMode->getSelectedId() == 3) m = AutomationRecorder::HOST_TEMPO;
        recorder.setSyncMode(m);
    };
    addAndMakeVisible(syncMode.get());

    loopToggle.setButtonText("LOOP");
    loopToggle.setClickingTogglesState(true);
    loopToggle.setColour(TextButton::buttonColourId, DS::Color::bgInput());
    loopToggle.setColour(TextButton::buttonOnColourId, DS::Color::Accent::cyan());
    loopToggle.setColour(TextButton::textColourOffId, DS::Color::textSecondary());
    loopToggle.setColour(TextButton::textColourOnId, DS::Color::textPrimary());
    loopToggle.setTooltip("Enable loop playback (L)");
    loopToggle.onClick = [this]() { updateLoop(); };
    addAndMakeVisible(loopToggle);

    // === MKS-70 PARAMETER TARGETING ===
    targetToneCombo = std::make_unique<StyledComboBox>("Target");
    targetToneCombo->addItems({"Upper Tone", "Lower Tone", "Patch"});
    targetToneCombo->setTooltip("Select which part to automate");
    targetToneCombo->getComboBox().onChange = [this]() {
        populateParameterList(targetToneCombo->getSelectedId());
    };
    addAndMakeVisible(targetToneCombo.get());
    
    targetParamCombo = std::make_unique<StyledComboBox>("Parameter");
    targetParamCombo->setTooltip("Select parameter to add as automation lane");
    addAndMakeVisible(targetParamCombo.get());
    
    // Populate initial list with actual MKS-70 parameters
    populateParameterList(1);
    
    addTargetBtn.setButtonText("+ Add Lane");
    addTargetBtn.setColour(TextButton::buttonColourId, DS::Color::Accent::warning().withAlpha(0.3f));
    addTargetBtn.setColour(TextButton::textColourOffId, DS::Color::Accent::warning());
    addTargetBtn.setTooltip("Add selected parameter as new automation lane");
    addTargetBtn.onClick = [this]()
    {
        auto& lanes = recorder.getLanes();
        AutomationRecorder::AutomationLane lane;
        String tonePrefix = targetToneCombo->getSelectedId() == 1 ? "Upper." : 
                           (targetToneCombo->getSelectedId() == 2 ? "Lower." : "Patch.");
        lane.paramId = tonePrefix + targetParamCombo->getComboBox().getText();
        lane.enabled = true;
        lane.armed = false;
        lanes.push_back(std::move(lane));
        refreshLaneViews();
    };
    addAndMakeVisible(addTargetBtn);

    // === LANES ===
    lanesViewport.setViewedComponent(&lanesContent, false);
    lanesViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(lanesViewport);
    
    btnAddLane.setButtonText("+ Add Generic Lane");
    btnAddLane.setColour(TextButton::buttonColourId, DS::Color::Accent::primary().withAlpha(0.3f));
    btnAddLane.setColour(TextButton::textColourOffId, DS::Color::Accent::primary());
    btnAddLane.setTooltip("Add a new generic automation lane");
    btnAddLane.onClick = [this]()
    {
        auto& lanes = recorder.getLanes();
        AutomationRecorder::AutomationLane lane;
        lane.paramId = "Param" + String(lanes.size() + 1);
        lane.enabled = true;
        lane.armed = false;
        lanes.push_back(std::move(lane));
        refreshLaneViews();
    };
    addAndMakeVisible(btnAddLane);

    // === ZOOM CONTROLS ===
    styleZoomButton(btnZoomIn, "+");
    btnZoomIn.setTooltip("Zoom in (=)");
    btnZoomIn.onClick = [this]() { setZoom(zoomFactor * 1.25); };
    addAndMakeVisible(btnZoomIn);
    
    styleZoomButton(btnZoomOut, "-");
    btnZoomOut.setTooltip("Zoom out (-)");
    btnZoomOut.onClick = [this]() { setZoom(zoomFactor / 1.25); };
    addAndMakeVisible(btnZoomOut);
    
    styleZoomButton(btnZoomFit, "Fit");
    btnZoomFit.setTooltip("Fit all automation in view (F)");
    btnZoomFit.onClick = [this]() { zoomToFit(); };
    addAndMakeVisible(btnZoomFit);
    
    zoomLabel.setText("100%", dontSendNotification);
    zoomLabel.setFont(DS::Font::small());
    zoomLabel.setColour(Label::textColourId, DS::Color::textSecondary());
    zoomLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(zoomLabel);

    // === TOOLS (Right side menu) ===
    styleToolButton(toolDraw, "Draw");
    toolDraw.setTooltip("Draw automation points freehand (D)");
    toolDraw.setToggleState(true, dontSendNotification); // Default tool
    styleToolButton(toolLine, "Line");
    toolLine.setTooltip("Draw straight lines between points (L)");
    styleToolButton(toolSelect, "Select");
    toolSelect.setTooltip("Select and move points (V)");
    styleToolButton(toolErase, "Erase");
    toolErase.setTooltip("Erase automation points (E)");
    
    addAndMakeVisible(toolDraw);
    addAndMakeVisible(toolLine);
    addAndMakeVisible(toolSelect);
    addAndMakeVisible(toolErase);
    
    toolDraw.onClick = [this](){ 
        setAllLaneEditMode(AutomationLaneComponent::DRAW);
        toolDraw.setToggleState(true, dontSendNotification);
        toolLine.setToggleState(false, dontSendNotification);
        toolSelect.setToggleState(false, dontSendNotification);
        toolErase.setToggleState(false, dontSendNotification);
    };
    toolLine.onClick = [this](){ 
        setAllLaneEditMode(AutomationLaneComponent::LINE); 
        toolDraw.setToggleState(false, dontSendNotification);
        toolLine.setToggleState(true, dontSendNotification);
        toolSelect.setToggleState(false, dontSendNotification);
        toolErase.setToggleState(false, dontSendNotification);
    };
    toolSelect.onClick = [this](){ 
        setAllLaneEditMode(AutomationLaneComponent::SELECT); 
        toolDraw.setToggleState(false, dontSendNotification);
        toolLine.setToggleState(false, dontSendNotification);
        toolSelect.setToggleState(true, dontSendNotification);
        toolErase.setToggleState(false, dontSendNotification);
    };
    toolErase.onClick = [this](){ 
        setAllLaneEditMode(AutomationLaneComponent::ERASE); 
        toolDraw.setToggleState(false, dontSendNotification);
        toolLine.setToggleState(false, dontSendNotification);
        toolSelect.setToggleState(false, dontSendNotification);
        toolErase.setToggleState(true, dontSendNotification);
    };

    // NEW: Curve tool
    styleToolButton(toolCurve, "Curve");
    toolCurve.setTooltip("Draw smooth curves (C)");
    addAndMakeVisible(toolCurve);
    toolCurve.onClick = [this](){
        // Will use bezier curves instead of straight lines
        setAllLaneEditMode(AutomationLaneComponent::LINE);  // Reuse line for now
        toolDraw.setToggleState(false, dontSendNotification);
        toolLine.setToggleState(false, dontSendNotification);
        toolCurve.setToggleState(true, dontSendNotification);
        toolSelect.setToggleState(false, dontSendNotification);
        toolErase.setToggleState(false, dontSendNotification);
    };
    
    // NEW: Smooth tool
    styleToolButton(toolSmooth, "Smooth");
    toolSmooth.setTooltip("Smooth selected automation (S)");
    addAndMakeVisible(toolSmooth);
    toolSmooth.onClick = [this](){ smoothSelectedLanes(); };
    
    // NEW: Quantize tool
    styleToolButton(toolQuantize, "Quantize");
    toolQuantize.setTooltip("Quantize values to grid (Q)");
    addAndMakeVisible(toolQuantize);
    toolQuantize.onClick = [this](){ quantizeSelectedLanes(); };

    snapCombo = std::make_unique<StyledComboBox>("Snap");
    snapCombo->addItems({"Off", "1/4", "1/8", "1/16", "1/32", "1/64"});
    snapCombo->setSelectedId(3);  // Default 1/8
    snapCombo->setTooltip("Snap grid resolution for editing");
    addAndMakeVisible(snapCombo.get());
    
    // NEW: Curve preset combo
    curveTypeCombo = std::make_unique<StyledComboBox>("Curves");
    curveTypeCombo->addItems({"Linear", "Ease In", "Ease Out", "S-Curve", "Exponential", "Logarithmic"});
    curveTypeCombo->setSelectedId(1);
    curveTypeCombo->setTooltip("Curve shape preset for line tool");
    curveTypeCombo->getComboBox().onChange = [this]() { applyCurvePreset(curveTypeCombo->getSelectedId()); };
    addAndMakeVisible(curveTypeCombo.get());

    styleToolButton(btnCopy, "Copy");
    btnCopy.setTooltip("Copy selected automation (Ctrl+C)");
    styleToolButton(btnPaste, "Paste");
    btnPaste.setTooltip("Paste automation (Ctrl+V)");
    styleToolButton(btnClear, "Clear");
    btnClear.setTooltip("Clear all points in selected lanes");
    
    // NEW: Undo/Redo for automation
    styleToolButton(btnUndo, "Undo");
    btnUndo.setTooltip("Undo last automation edit (Ctrl+Z)");
    styleToolButton(btnRedo, "Redo");
    btnRedo.setTooltip("Redo automation edit (Ctrl+Y)");
    
    styleToolButton(btnImport, "Import");
    btnImport.setTooltip("Import automation from JSON file");
    styleToolButton(btnExport, "Export");
    btnExport.setTooltip("Export automation to JSON or MIDI file");
    
    addAndMakeVisible(btnCopy);
    addAndMakeVisible(btnPaste);
    addAndMakeVisible(btnClear);
    addAndMakeVisible(btnUndo);
    addAndMakeVisible(btnRedo);
    addAndMakeVisible(btnImport);
    addAndMakeVisible(btnExport);

    btnClear.onClick = [this]()
    {
        for (auto& lane : recorder.getLanes()) lane.points.clear();
        refreshLaneViews();
    };

    btnExport.onClick = [this]()
    {
        FileChooser chooser("Export Automation", File(), "*.json;*.mid");
        chooser.launchAsync(FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles,
            [this](const FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file == File()) return;
                if (file.getFileExtension().equalsIgnoreCase(".json"))
                    recorder.exportToJSON(file);
                else
                    recorder.exportToMidiFile(file);
            });
    };

    btnImport.onClick = [this]()
    {
        FileChooser chooser("Import Automation", File(), "*.json");
        chooser.launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
            [this](const FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file == File()) return;
                if (recorder.importFromJSON(file))
                    refreshLaneViews();
            });
    };

    startTimerHz(60);
    
    // Add default automation lane for VCF Cutoff
    {
        auto& lanes = recorder.getLanes();
        AutomationRecorder::AutomationLane lane;
        lane.paramId = "Upper.VCF Cutoff";
        lane.enabled = true;
        lane.armed = false;
        lanes.push_back(std::move(lane));
    }
    
    refreshLaneViews();
}

void AutomationPanel::populateParameterList(int toneIndex)
{
    targetParamCombo->getComboBox().clear();
    
    if (toneIndex == 1 || toneIndex == 2)  // Upper or Lower Tone
    {
        // DCO1 parameters
        targetParamCombo->getComboBox().addItem("DCO1 Range", 1);
        targetParamCombo->getComboBox().addItem("DCO1 Waveform", 2);
        targetParamCombo->getComboBox().addItem("DCO1 Tune", 3);
        targetParamCombo->getComboBox().addItem("DCO1 LFO Mod", 4);
        targetParamCombo->getComboBox().addItem("DCO1 ENV Mod", 5);
        
        // DCO2 parameters
        targetParamCombo->getComboBox().addItem("DCO2 Range", 6);
        targetParamCombo->getComboBox().addItem("DCO2 Waveform", 7);
        targetParamCombo->getComboBox().addItem("DCO2 Crossmod", 8);
        targetParamCombo->getComboBox().addItem("DCO2 Tune", 9);
        targetParamCombo->getComboBox().addItem("DCO2 Fine Tune", 10);
        targetParamCombo->getComboBox().addItem("DCO2 LFO Mod", 11);
        targetParamCombo->getComboBox().addItem("DCO2 ENV Mod", 12);
        
        // Mixer
        targetParamCombo->getComboBox().addItem("Mixer DCO1", 13);
        targetParamCombo->getComboBox().addItem("Mixer DCO2", 14);
        targetParamCombo->getComboBox().addItem("Mixer Envelope", 15);
        targetParamCombo->getComboBox().addItem("Mixer Dynamics", 16);
        targetParamCombo->getComboBox().addItem("Mixer Mode", 17);
        
        // VCF parameters
        targetParamCombo->getComboBox().addItem("VCF HPF Cutoff", 18);
        targetParamCombo->getComboBox().addItem("VCF Cutoff", 19);
        targetParamCombo->getComboBox().addItem("VCF Resonance", 20);
        targetParamCombo->getComboBox().addItem("VCF LFO Mod", 21);
        targetParamCombo->getComboBox().addItem("VCF ENV Mod", 22);
        targetParamCombo->getComboBox().addItem("VCF ENV Depth", 23);
        targetParamCombo->getComboBox().addItem("VCF Dynamics", 24);
        targetParamCombo->getComboBox().addItem("VCF Key Follow", 25);
        
        // VCA parameters
        targetParamCombo->getComboBox().addItem("VCA Level", 26);
        targetParamCombo->getComboBox().addItem("VCA Dynamics", 27);
        
        // LFO parameters
        targetParamCombo->getComboBox().addItem("LFO Waveform", 28);
        targetParamCombo->getComboBox().addItem("LFO Delay", 29);
        targetParamCombo->getComboBox().addItem("LFO Rate", 30);
        
        // ENV1 parameters
        targetParamCombo->getComboBox().addItem("ENV1 Attack", 31);
        targetParamCombo->getComboBox().addItem("ENV1 Decay", 32);
        targetParamCombo->getComboBox().addItem("ENV1 Sustain", 33);
        targetParamCombo->getComboBox().addItem("ENV1 Release", 34);
        targetParamCombo->getComboBox().addItem("ENV1 Key Follow", 35);
        
        // ENV2 parameters
        targetParamCombo->getComboBox().addItem("ENV2 Attack", 36);
        targetParamCombo->getComboBox().addItem("ENV2 Decay", 37);
        targetParamCombo->getComboBox().addItem("ENV2 Sustain", 38);
        targetParamCombo->getComboBox().addItem("ENV2 Release", 39);
        targetParamCombo->getComboBox().addItem("ENV2 Key Follow", 40);
        
        // Chorus
        targetParamCombo->getComboBox().addItem("Chorus", 41);
    }
    else  // Patch
    {
        targetParamCombo->getComboBox().addItem("Key Mode", 1);
        targetParamCombo->getComboBox().addItem("Split Point", 2);
        targetParamCombo->getComboBox().addItem("Balance", 3);
        targetParamCombo->getComboBox().addItem("Total Volume", 4);
        targetParamCombo->getComboBox().addItem("Portamento Time", 5);
        targetParamCombo->getComboBox().addItem("Portamento Mode", 6);
        targetParamCombo->getComboBox().addItem("Bend Range", 7);
        targetParamCombo->getComboBox().addItem("Assign Mode", 8);
        targetParamCombo->getComboBox().addItem("Unison Detune", 9);
        targetParamCombo->getComboBox().addItem("Hold Mode", 10);
        targetParamCombo->getComboBox().addItem("Chase Play", 11);
        targetParamCombo->getComboBox().addItem("Chase Level", 12);
        targetParamCombo->getComboBox().addItem("Chase Time", 13);
    }
    
    targetParamCombo->setSelectedId(1);
}

void AutomationPanel::styleTransportButton(TextButton& btn, const String& text, bool isRecButton)
{
    btn.setButtonText(text);
    btn.setColour(TextButton::buttonColourId, DS::Color::bgElevated());
    btn.setColour(TextButton::buttonOnColourId, isRecButton ? Colour(0xFFE53935) : DS::Color::Accent::cyan());
    btn.setColour(TextButton::textColourOffId, isRecButton ? Colour(0xFFE53935) : DS::Color::textPrimary());
    btn.setColour(TextButton::textColourOnId, DS::Color::textPrimary());
    if (isRecButton) {
        btn.setClickingTogglesState(true);
    }
}

void AutomationPanel::styleToolButton(TextButton& btn, const String& text)
{
    btn.setButtonText(text);
    btn.setClickingTogglesState(true);
    btn.setColour(TextButton::buttonColourId, DS::Color::bgInput());
    btn.setColour(TextButton::buttonOnColourId, DS::Color::Accent::primary().withAlpha(0.5f));
    btn.setColour(TextButton::textColourOffId, DS::Color::textSecondary());
    btn.setColour(TextButton::textColourOnId, DS::Color::Accent::primary());
}

void AutomationPanel::paint(Graphics& g)
{
    g.fillAll(DS::Color::bg());

    auto bounds = getLocalBounds().reduced(DS::Space::M);
    
    // Skip top sections and bottom transport
    bounds.removeFromBottom(44 + DS::Space::S);
    bounds.removeFromRight(100 + DS::Space::M);  // Tools panel
    bounds.removeFromTop(90);  // Parameter section height
    bounds.removeFromTop(DS::Space::M);
    
    // Timeline ruler area
    auto timeline = bounds.removeFromTop(28);
    
    // Timeline background
    g.setColour(DS::Color::bgCard());
    g.fillRoundedRectangle(timeline.toFloat(), DS::Radius::M);
    
    // Draw timeline border
    g.setColour(DS::Color::border());
    g.drawRoundedRectangle(timeline.toFloat().reduced(0.5f), DS::Radius::M, 1.0f);
    
    // Timeline tick marks and labels
    g.setFont(DS::Font::small());
    
    const double majorEvery = 10.0;
    auto fmt = [](double tt)
    {
        int minutes = (int) (tt / 60.0);
        double seconds = tt - minutes * 60.0;
        return String::formatted("%02d:%02d", minutes, (int) std::round(seconds));
    };
    
    for (double t = std::ceil(viewTimeStart / majorEvery) * majorEvery; t <= viewTimeEnd; t += majorEvery)
    {
        float x = (float) juce::jmap(t, viewTimeStart, viewTimeEnd, (double) timeline.getX(), (double) timeline.getRight());
        
        // Tick line
        g.setColour(DS::Color::border());
        g.drawLine(x, (float)timeline.getY() + 5, x, (float)timeline.getBottom() - 5, 1.0f);
        
        // Label
        g.setColour(DS::Color::textSecondary());
        String label = fmt(t);
        g.drawText(label, Rectangle<int>((int)x - 20, timeline.getY(), 40, timeline.getHeight()), 
                   Justification::centred);
    }
    
    // Playhead
    double pos = recorder.getPlaybackPosition();
    float x = (float) juce::jmap(pos, viewTimeStart, viewTimeEnd, 
                                  (double) timeline.getX(), (double) timeline.getRight());
    g.setColour(DS::Color::Accent::cyan());
    g.drawLine(x, (float)timeline.getY(), x, (float)timeline.getBottom(), 2.0f);
    
    // Draw playhead triangle at top
    Path triangle;
    triangle.addTriangle(x - 5, (float)timeline.getY(), x + 5, (float)timeline.getY(), x, (float)timeline.getY() + 8);
    g.fillPath(triangle);
    
    // === BOTTOM TRANSPORT BAR ===
    auto transportBar = getLocalBounds().removeFromBottom(44);
    
    // Transport bar background
    g.setColour(DS::Color::bgCard());
    g.fillRect(transportBar.toFloat());
    
    // Top border
    g.setColour(DS::Color::border());
    g.drawHorizontalLine(transportBar.getY(), 0, (float)getWidth());
}

void AutomationPanel::resized()
{
    auto bounds = getLocalBounds().reduced(DS::Space::S);
    
    const int sectionGap = DS::Space::S;
    const int smallGap = 4;
    
    // === BOTTOM: Transport Bar ===
    auto transportBar = bounds.removeFromBottom(44);
    bounds.removeFromBottom(smallGap);
    
    // Center transport controls
    auto transportCenter = transportBar.withSizeKeepingCentre(480, 36);
    
    const int btnSize = 30;
    btnRew.setBounds(transportCenter.removeFromLeft(btnSize));
    transportCenter.removeFromLeft(3);
    btnStop.setBounds(transportCenter.removeFromLeft(btnSize));
    transportCenter.removeFromLeft(3);
    btnPlay.setBounds(transportCenter.removeFromLeft(btnSize + 6));
    transportCenter.removeFromLeft(3);
    btnRec.setBounds(transportCenter.removeFromLeft(btnSize));
    transportCenter.removeFromLeft(3);
    btnFwd.setBounds(transportCenter.removeFromLeft(btnSize));
    transportCenter.removeFromLeft(10);
    timeLabel.setBounds(transportCenter.removeFromLeft(150));
    transportCenter.removeFromLeft(10);
    loopToggle.setBounds(transportCenter.removeFromLeft(46).withHeight(24).withY(transportCenter.getY() + 4));
    
    // (Info Panel removed - space used for larger Quick Access)
    
    // === RIGHT SIDE: Tools Panel ===
    auto rightPanel = bounds.removeFromRight(90);
    toolsSection->setBounds(rightPanel);
    auto toolsContent = toolsSection->getContentBounds();
    
    const int toolBtnH = 20;
    const int toolGap = 1;
    
    toolDraw.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    toolLine.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    toolCurve.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    toolSelect.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    toolErase.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(3);
    
    snapCombo->setBounds(toolsContent.removeFromTop(22));
    toolsContent.removeFromTop(toolGap);
    curveTypeCombo->setBounds(toolsContent.removeFromTop(22));
    toolsContent.removeFromTop(3);
    
    toolSmooth.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    toolQuantize.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(3);
    
    btnCopy.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    btnPaste.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    btnClear.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    btnUndo.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    btnRedo.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(3);
    
    btnImport.setBounds(toolsContent.removeFromTop(toolBtnH));
    toolsContent.removeFromTop(toolGap);
    btnExport.setBounds(toolsContent.removeFromTop(toolBtnH));
    
    bounds.removeFromRight(sectionGap);
    
    // === TOP: Timing + Quick Access (ENLARGED) ===
    auto topRow = bounds.removeFromTop(90);
    
    // Timing section
    int timingW = 140;
    auto timingBounds = topRow.removeFromLeft(timingW);
    timingSection->setBounds(timingBounds);
    auto timingContent = timingSection->getContentBounds();
    tempoSlider->setBounds(timingContent.removeFromLeft(65));
    timingContent.removeFromLeft(4);
    syncMode->setBounds(timingContent.removeFromTop(30));
    
    topRow.removeFromLeft(sectionGap);
    
    // Quick Access - single row
    quickAccessSection->setBounds(topRow);
    auto quickContent = quickAccessSection->getContentBounds();
    
    const int qBtnW = 60;  // Enlarged buttons
    const int qBtnH = 28;   // Taller buttons
    const int qBtnGap = 4;
    const int labelW = 45;
    
    auto quickRow = quickContent.withTrimmedTop(4);  // Full area for buttons
    
    quickFilterLabel.setBounds(quickRow.removeFromLeft(labelW));
    for (int i = 0; i < 3 && i < quickButtons.size(); ++i) {
        quickButtons[i]->setBounds(quickRow.removeFromLeft(qBtnW));
        quickRow.removeFromLeft(qBtnGap);
    }
    quickRow.removeFromLeft(6);
    
    quickEnvLabel.setBounds(quickRow.removeFromLeft(labelW));
    for (int i = 3; i < 6 && i < quickButtons.size(); ++i) {
        quickButtons[i]->setBounds(quickRow.removeFromLeft(qBtnW));
        quickRow.removeFromLeft(qBtnGap);
    }
    quickRow.removeFromLeft(6);
    
    quickLfoLabel.setBounds(quickRow.removeFromLeft(labelW));
    for (int i = 6; i < 8 && i < quickButtons.size(); ++i) {
        quickButtons[i]->setBounds(quickRow.removeFromLeft(qBtnW));
        quickRow.removeFromLeft(qBtnGap);
    }
    quickRow.removeFromLeft(6);
    
    quickMixerLabel.setBounds(quickRow.removeFromLeft(labelW));
    for (int i = 8; i < quickButtons.size(); ++i) {
        quickButtons[i]->setBounds(quickRow.removeFromLeft(qBtnW));
        quickRow.removeFromLeft(qBtnGap);
    }
    
    bounds.removeFromTop(sectionGap);
    
    // === SECOND ROW: Parameter Targeting + Zoom ===
    auto paramRow = bounds.removeFromTop(30);
    
    parameterSection->setBounds(paramRow.removeFromLeft(420));
    auto paramContent = parameterSection->getContentBounds();
    
    targetToneCombo->setBounds(paramContent.removeFromLeft(95));
    paramContent.removeFromLeft(4);
    targetParamCombo->setBounds(paramContent.removeFromLeft(140));
    paramContent.removeFromLeft(4);
    addTargetBtn.setBounds(paramContent.removeFromLeft(75));
    paramContent.removeFromLeft(4);
    btnAddLane.setBounds(paramContent.removeFromLeft(95));
    
    // Zoom controls on the right - inline
    paramRow.removeFromLeft(sectionGap);
    auto zoomArea = paramRow;
    btnZoomOut.setBounds(zoomArea.removeFromLeft(26).reduced(1));
    zoomLabel.setBounds(zoomArea.removeFromLeft(40));
    btnZoomIn.setBounds(zoomArea.removeFromLeft(26).reduced(1));
    zoomArea.removeFromLeft(4);
    btnZoomFit.setBounds(zoomArea.removeFromLeft(32).reduced(1));
    
    bounds.removeFromTop(smallGap);
    
    // === TIMELINE RULER ===
    bounds.removeFromTop(24);  // Painted in paint()
    bounds.removeFromTop(smallGap);
    
    // === LANES SECTION - USE REMAINING SPACE ===
    lanesSection->setBounds(bounds);
    auto lanesContent_ = lanesSection->getContentBounds();
    
    // Viewport for lanes - with scrolling
    lanesViewport.setBounds(lanesContent_);

    // Layout lane views vertically
    int y = 0;
    const int laneHeight = 75;  // Slightly smaller lanes
    const int laneGap = 4;
    for (auto* lv : laneViews)
    {
        lv->setBounds(0, y, lanesViewport.getWidth() - lanesViewport.getScrollBarThickness(), laneHeight);
        y += laneHeight + laneGap;
    }
    this->lanesContent.setSize(lanesViewport.getWidth(), juce::jmax(y, lanesViewport.getHeight()));
}

void AutomationPanel::timerCallback()
{
    if (recorder.isPlaying())
    {
        recorder.updatePlaybackPosition();
        emitPlaybackValues();
        updateTimeLabel();
        repaint();  // Redraw playhead
    }
}

void AutomationPanel::emitPlaybackValues()
{
    if (!onPlaybackValue)
        return;

    const auto values = recorder.getValuesAtTime(recorder.getPlaybackPosition());
    for (const auto& value : values)
    {
        auto it = lastPlaybackValues.find(value.first);
        if (it != lastPlaybackValues.end() && it->second == value.second)
            continue;

        lastPlaybackValues[value.first] = value.second;
        onPlaybackValue(value.first, value.second);
    }
}

void AutomationPanel::refreshLaneViews()
{
    // Clear and rebuild lane components
    for (auto* lv : laneViews) lanesContent.removeChildComponent(lv);
    laneViews.clear(true);

    for (auto& lane : recorder.getLanes())
    {
        auto* lv = new AutomationLaneComponent();
        lv->setLane(&lane);
        lv->setTimeRange(viewTimeStart, viewTimeEnd);
        lv->setValueRange(0, 127);
        laneViews.add(lv);
        lanesContent.addAndMakeVisible(lv);
    }

    resized();
    repaint();
}

void AutomationPanel::setAllLaneEditMode(AutomationLaneComponent::EditMode mode)
{
    for (auto* lv : laneViews) lv->setEditMode(mode);
}

void AutomationPanel::updateTimeLabel()
{
    const double pos = recorder.getPlaybackPosition();
    auto fmt = [](double t)
    {
        int minutes = (int) (t / 60.0);
        double seconds = t - minutes * 60.0;
        return String::formatted("%02d:%06.3f", minutes, seconds);
    };
    timeLabel.setText(fmt(pos) + " / " + fmt(viewTimeEnd), dontSendNotification);
}

void AutomationPanel::updateLoop()
{
    if (loopToggle.getToggleState())
        recorder.setLoop(viewTimeStart, viewTimeEnd);
    else
        recorder.setLoop(0.0, 0.0);
}

void AutomationPanel::applyCurvePreset(int presetIndex)
{
    // Store curve type for use when drawing
    // 1=Linear, 2=Ease In, 3=Ease Out, 4=S-Curve, 5=Exponential, 6=Logarithmic
    DBG("Curve preset selected: " + String(presetIndex));
}

void AutomationPanel::smoothSelectedLanes()
{
    // Apply smoothing to all lanes with data
    for (auto& lane : recorder.getLanes())
    {
        if (lane.points.size() < 3) continue;
        
        // Simple 3-point moving average
        std::vector<int> smoothed;
        smoothed.push_back(lane.points[0].value);
        
        for (size_t i = 1; i < lane.points.size() - 1; ++i)
        {
            int avg = (lane.points[i-1].value + lane.points[i].value + lane.points[i+1].value) / 3;
            smoothed.push_back(avg);
        }
        smoothed.push_back(lane.points.back().value);
        
        // Apply smoothed values
        for (size_t i = 0; i < lane.points.size(); ++i)
        {
            lane.points[i].value = juce::jlimit(0, 127, smoothed[i]);
        }
    }
    refreshLaneViews();
    DBG("Smoothed automation lanes");
}

void AutomationPanel::quantizeSelectedLanes()
{
    // Quantize values to common steps (0, 16, 32, 48, 64, 80, 96, 112, 127)
    const int steps[] = {0, 16, 32, 48, 64, 80, 96, 112, 127};
    const int numSteps = 9;
    
    for (auto& lane : recorder.getLanes())
    {
        for (auto& pt : lane.points)
        {
            // Find nearest step
            int nearest = 0;
            int minDiff = 999;
            for (int i = 0; i < numSteps; ++i)
            {
                int diff = std::abs(pt.value - steps[i]);
                if (diff < minDiff)
                {
                    minDiff = diff;
                    nearest = steps[i];
                }
            }
            pt.value = nearest;
        }
    }
    refreshLaneViews();
    DBG("Quantized automation lanes");
}

void AutomationPanel::styleZoomButton(TextButton& btn, const String& text)
{
    btn.setButtonText(text);
    btn.setColour(TextButton::buttonColourId, DS::Color::bgInput());
    btn.setColour(TextButton::buttonOnColourId, DS::Color::Accent::primary().withAlpha(0.4f));
    btn.setColour(TextButton::textColourOffId, DS::Color::textSecondary());
    btn.setColour(TextButton::textColourOnId, DS::Color::textPrimary());
}

void AutomationPanel::setZoom(double newZoom)
{
    zoomFactor = jlimit(0.25, 4.0, newZoom);
    viewTimeEnd = 45.0 / zoomFactor;
    
    // Update zoom label
    int pct = (int)(zoomFactor * 100.0);
    zoomLabel.setText(String(pct) + "%", dontSendNotification);
    
    // Update lane time ranges
    for (auto* lv : laneViews)
        lv->setTimeRange(viewTimeStart, viewTimeEnd);
    
    repaint();
}

void AutomationPanel::zoomToFit()
{
    // Find the maximum time with automation data
    double maxTime = 10.0;  // Minimum 10 seconds
    for (auto& lane : recorder.getLanes())
    {
        for (auto& pt : lane.points)
        {
            if (pt.timestamp > maxTime)
                maxTime = pt.timestamp;
        }
    }
    
    maxTime += 5.0;  // Add some padding
    zoomFactor = 45.0 / maxTime;
    setZoom(zoomFactor);
}

void AutomationPanel::createQuickAccessButtons()
{
    // Filter parameters - most commonly automated
    auto* cutoff = new QuickParamButton("Cutoff", "Upper.VCF Cutoff", AutomationColors::vcf(), 
        "Filter cutoff - Teder Hitukh Filter");
    cutoff->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(cutoff);
    addAndMakeVisible(cutoff);
    
    auto* reso = new QuickParamButton("Reso", "Upper.VCF Resonance", AutomationColors::vcf(),
        "Filter resonance - Rezonans Filter");
    reso->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(reso);
    addAndMakeVisible(reso);
    
    auto* envMod = new QuickParamButton("Env Mod", "Upper.VCF ENV Mod", AutomationColors::vcf(),
        "Envelope to filter amount - Omek Ma'atefa leFilter");
    envMod->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(envMod);
    addAndMakeVisible(envMod);
    
    // Envelope parameters
    auto* attack = new QuickParamButton("Attack", "Upper.ENV1 Attack", AutomationColors::env(),
        "Filter envelope attack - Hatkafa");
    attack->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(attack);
    addAndMakeVisible(attack);
    
    auto* decay = new QuickParamButton("Decay", "Upper.ENV1 Decay", AutomationColors::env(),
        "Filter envelope decay - De'ikha");
    decay->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(decay);
    addAndMakeVisible(decay);
    
    auto* release = new QuickParamButton("Release", "Upper.ENV2 Release", AutomationColors::env(),
        "Amp envelope release - Shikhrur");
    release->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(release);
    addAndMakeVisible(release);
    
    // LFO parameters
    auto* lfoRate = new QuickParamButton("LFO Rate", "Upper.LFO Rate", AutomationColors::lfo(),
        "LFO speed - Mehirut LFO");
    lfoRate->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(lfoRate);
    addAndMakeVisible(lfoRate);
    
    auto* lfoMod = new QuickParamButton("LFO>VCF", "Upper.VCF LFO Mod", AutomationColors::lfo(),
        "LFO to filter modulation - Omek LFO leFilter");
    lfoMod->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(lfoMod);
    addAndMakeVisible(lfoMod);
    
    // Mixer/Level parameters
    auto* level = new QuickParamButton("Level", "Upper.VCA Level", AutomationColors::mixer(),
        "Output level - Otzmat Yetzi'a");
    level->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(level);
    addAndMakeVisible(level);
    
    auto* balance = new QuickParamButton("Balance", "Patch.Balance", AutomationColors::mixer(),
        "Upper/Lower balance - Izun Elyon/Tahton");
    balance->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(balance);
    addAndMakeVisible(balance);
    
    // Additional popular parameters
    auto* chorus = new QuickParamButton("Chorus", "Upper.Chorus", AutomationColors::chorus(),
        "Chorus mode - Matzav Chorus");
    chorus->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(chorus);
    addAndMakeVisible(chorus);
    
    auto* porta = new QuickParamButton("Porta", "Patch.Portamento Time", AutomationColors::patch(),
        "Portamento time - Zman Glisha");
    porta->onClick = [this](const String& id, const String& name) { onQuickButtonClicked(id, name); };
    quickButtons.add(porta);
    addAndMakeVisible(porta);
}

void AutomationPanel::onQuickButtonClicked(const String& paramId, const String& name)
{
    // (InfoPanel removed)
    
    // Update button selection states
    for (auto* btn : quickButtons)
    {
        btn->setSelected(btn->getParamId() == paramId);
    }
    
    // Add lane for this parameter
    auto& lanes = recorder.getLanes();
    
    // Check if lane already exists
    for (auto& lane : lanes)
    {
        if (lane.paramId == paramId)
        {
            // Lane exists, just show info
            return;
        }
    }
    
    // Add new lane
    AutomationRecorder::AutomationLane lane;
    lane.paramId = paramId;
    lane.enabled = true;
    lane.armed = false;
    lanes.push_back(std::move(lane));
    refreshLaneViews();
}
