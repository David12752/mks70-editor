/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MacroSetupDialog.h"
#include "Theme.h"
#include "ThemeManager.h"
#include "HeaderBar.h"
// Ensure new design components compile
#include "MKS70LookAndFeel.h"
#include "SectionComponent.h"
#include "LabeledSlider.h"
#include "LabeledComboBox.h"
#include "LayoutHelpers.h"
#include "ResponsiveContainer.h"
#include "DCOSection.h"
#include "VCFSection.h"
#include "EnvelopeSection.h"
#include "MixerSection.h"
#include "VCASection.h"
#include "LFOSection.h"
#include "ChorusSection.h"
#include "TonePanel.h"
#include "StatusBar.h"
#include "MKS70_Mapping.h"  // For SysEx address constants

//==============================================================================
MKS70EditorAudioProcessorEditor::MKS70EditorAudioProcessorEditor (MKS70EditorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      tabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    // Install custom LookAndFeel for the entire editor tree
    setLookAndFeel(&appLookAndFeel);
    // Apply theme palette to the installed LookAndFeel
    Theme::applyToLookAndFeel(getLookAndFeel());
    
    // Set tab bar depth to ensure visible tab labels
    tabbedComponent.setTabBarDepth(36);
    
    // Setup header bar
    addAndMakeVisible(headerBar);
    headerBar.setProcessor(&audioProcessor);
    headerBar.onCollapseToggle = [this]() { toggleHeaderBar(); };
    headerBar.onSettingsClick = [this]() {
        const int settingsTabIndex = 9;  // Settings is last after Automation/Modulation/AI
        if (currentTabIndex == settingsTabIndex) {
            selectTab(previousTabIndex);  // Toggle back
        } else {
            selectTab(settingsTabIndex);
        }
    };
    headerBar.onCompareToggle = [this](bool on) { 
        if (on) showComparePanel(); 
        else hideComparePanel(); 
    };
    headerBar.onFirmwareChanged = [this](FirmwareVersion fw) {
        // Update firmware version on both tone panels
        toneUpperPanel.setFirmwareVersion(fw);
        toneLowerPanel.setFirmwareVersion(fw);
        DBG("Firmware version changed to: " + juce::String(fw == FirmwareVersion::Original ? "Original" : 
            (fw == FirmwareVersion::Vecoven3 ? "Vecoven V3.x" : "Vecoven V4.x")));
        resized();  // Re-layout to adjust for new firmware controls
    };
    
    // Bank/Program change - send MIDI Bank Select and Program Change
    headerBar.onBankProgramChange = [this](int bank, int program) {
        auto& midiHandler = audioProcessor.getMidiHandler();
        // MKS-70 uses Bank Select CC#0 (0-7 for banks A-H) and Program Change (0-7 for programs 1-8)
        midiHandler.sendCC(1, 0, bank);  // Bank Select MSB on Channel 1
        midiHandler.sendProgramChange(1, program);  // Program Change on Channel 1
        DBG("Bank/Program change: Bank=" + juce::String(bank) + " Program=" + juce::String(program));
    };
    
    // Connect preset navigation from HeaderBar to editor's preset system
    headerBar.onPresetSelected = [this](int index) {
        // Header navigates presets - sync with editor's preset system
        // The HeaderBar already updates its own display
        DBG("HeaderBar preset selected: index=" + juce::String(index) + ", presetFiles count=" + juce::String(presetFiles.size()));
        
        if (index >= 0 && !presetFiles.isEmpty()) {
            currentPresetIndex = index % presetFiles.size();
            auto presetFile = presetFiles[currentPresetIndex];
            
            DBG("Loading preset file: " + presetFile.getFullPathName());
            
            // Load the preset file
            if (presetFile.existsAsFile() && presetFile.hasFileExtension(".syx"))
            {
                juce::MemoryBlock data;
                if (presetFile.loadFileAsData(data))
                {
                    DBG("Loaded " + juce::String(data.getSize()) + " bytes from SysEx file");
                    
                    // Parse the SysEx to update internal state
                    audioProcessor.getMidiHandler().loadSysExFromData(data.getData(), data.getSize());
                    
                    // Check if MIDI output is connected
                    if (!audioProcessor.getMidiHandler().isOutputConnected())
                    {
                        DBG("WARNING: No MIDI output connected - preset loaded to UI but NOT sent to synth!");
                    }
                    else
                    {
                        // Send the raw SysEx data to BOTH tones so presets play on whole keyboard
                        audioProcessor.getMidiHandler().sendRawSysExToBothTones(data);
                        // Set KeyMode to Whole (0) so both tones play across entire keyboard
                        audioProcessor.getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_KEY_MODE, 0);
                        DBG("Sent SysEx data to BOTH tones + KeyMode=Whole");
                    }
                    
                    refreshUI();
                }
                else
                {
                    DBG("Failed to load file data!");
                }
            }
            else
            {
                DBG("File doesn't exist or wrong extension: " + presetFile.getFullPathName());
            }
            
            // Update other displays (mini toolbar, bottom patch name)
            miniPresetLabel.setText(presetFile.getFileNameWithoutExtension(), juce::dontSendNotification);
            bottomPatchName.setText(presetFile.getFileNameWithoutExtension(), juce::dontSendNotification);
            DBG("Preset selected from HeaderBar: " + presetFile.getFullPathName());
        }
        else
        {
            DBG("No preset files available or invalid index!");
        }
    };
    
    // Setup expand bar (shown when header is collapsed)
    addChildComponent(expandBar);
    expandBar.onExpand = [this]() { toggleHeaderBar(); };
    expandBar.setVisible(false);
    
    // Setup toolbar (legacy - hidden)
    setupToolbar();
    
    // === SETUP MINI PRESET BAR ===
    setupMiniPresetBar();
    
    // === SETUP VERTICAL TABS ===
    setupVerticalTabs();
    
    // === SETUP BOTTOM TOOLBAR ===
    setupBottomToolbar();
    
    // Prepare scrollable pages for tabs (show new TonePanel; keep legacy tabs for state wiring)
    toneUpperPage.container.addAndMakeVisible(toneUpperPanel);
    toneLowerPage.container.addAndMakeVisible(toneLowerPanel);
    patchPage.container.addAndMakeVisible(patchTab);
    arpPage.container.addAndMakeVisible(arpTab);
    librarianPage.container.addAndMakeVisible(librarianTab);
    automationPage.container.addAndMakeVisible(automationPanel);
    modulationPage.container.addAndMakeVisible(modulationPanel);
    macroPage.container.addAndMakeVisible(macroTab);
    settingsPage.container.addAndMakeVisible(settingsTab);
    aiAssistantPage.container.addAndMakeVisible(aiAssistantPanel);
    
    // MIDI Test Panel is now inside SettingsTab

    // Add all tabs to the tabbed component using theme panel colour
    tabbedComponent.addTab("Tone Upper", Theme::bgPanel(), &toneUpperPage, false);
    tabbedComponent.addTab("Tone Lower", Theme::bgPanel(), &toneLowerPage, false);
    tabbedComponent.addTab("Patch", Theme::bgPanel(), &patchPage, false);
    tabbedComponent.addTab("Arpeggiator", Theme::bgPanel(), &arpPage, false);
    tabbedComponent.addTab("Librarian", Theme::bgPanel(), &librarianPage, false);
    tabbedComponent.addTab("Automation", Theme::bgPanel(), &automationPage, false);
    tabbedComponent.addTab("Modulation", Theme::bgPanel(), &modulationPage, false);
    tabbedComponent.addTab("Macros", Theme::bgPanel(), &macroPage, false);
    tabbedComponent.addTab("AI Assistant", Theme::bgPanel(), &aiAssistantPage, false);
    tabbedComponent.addTab("Settings", Theme::bgPanel(), &settingsPage, false);
    // MIDI Test panel moved to Settings tab
    
    // Add tabbed component directly so tab bar stays pinned
    addAndMakeVisible(tabbedComponent);
    
    // Initialize tabs with processor reference
    toneUpperTab.setProcessor(&audioProcessor);
    toneLowerTab.setProcessor(&audioProcessor);
    patchTab.setProcessor(&audioProcessor);
    arpTab.setProcessor(&audioProcessor);
    settingsTab.setProcessor(&audioProcessor);
    
    // Connect theme change callback
    settingsTab.onThemeChanged = [this](int themeId) {
        applyTheme(themeId);
    };
    
    // Connect firmware version change callback
    settingsTab.onFirmwareVersionChanged = [this](int fwId) {
        // Convert combo ID (1,2,3) to FirmwareVersion enum (0,1,2)
        FirmwareVersion fwVersion = static_cast<FirmwareVersion>(fwId - 1);
        toneUpperPanel.setFirmwareVersion(fwVersion);
        toneLowerPanel.setFirmwareVersion(fwVersion);
        // ArpTab needs to hide for Original ROM (no arpeggiator support)
        arpTab.setVisible(fwId >= 2);  // V3.x and V4.x have arpeggiator
        DBG("Firmware version changed to: " + juce::String(fwId));
    };
    settingsTab.applySettingsToProcessor();

    // Provide editor pointer to tabs that may need it (legacy tabs only)
    toneUpperTab.setEditor(this);
    toneLowerTab.setEditor(this);
    
    // Setup MIDI Learn for new TonePanel sections
    toneUpperPanel.setEditor(this);
    toneLowerPanel.setEditor(this);
    
    // Connect Init callbacks to send MIDI to synth
    toneUpperPanel.onInitTone = [this](const ToneParams& initTone) {
        toneUpperPanel.setToneParams(initTone);
        audioProcessor.setToneParameters(true, initTone, true);
        DBG("Init tone sent to synth: Upper");
    };
    toneLowerPanel.onInitTone = [this](const ToneParams& initTone) {
        toneLowerPanel.setToneParams(initTone);
        audioProcessor.setToneParameters(false, initTone, true);
        DBG("Init tone sent to synth: Lower");
    };
    
    // Connect Copy callbacks - Copy Upper to Lower and vice versa
    toneUpperPanel.onCopyToOther = [this](const ToneParams& params) {
        toneLowerPanel.setToneParams(params);
        audioProcessor.setToneParameters(false, params, true);
        DBG("Copied Upper tone to Lower");
    };
    toneLowerPanel.onCopyToOther = [this](const ToneParams& params) {
        toneUpperPanel.setToneParams(params);
        audioProcessor.setToneParameters(true, params, true);
        DBG("Copied Lower tone to Upper");
    };
    
    // Connect Swap callbacks
    toneUpperPanel.onSwapTones = toneLowerPanel.onSwapTones = [this]() {
        ToneParams upperParams = toneUpperPanel.getToneParams();
        ToneParams lowerParams = toneLowerPanel.getToneParams();
        toneUpperPanel.setToneParams(lowerParams);
        toneLowerPanel.setToneParams(upperParams);
        audioProcessor.setToneParameters(true, lowerParams, true);
        audioProcessor.setToneParameters(false, upperParams, true);
        DBG("Swapped Upper and Lower tones");
    };
    
    // === LIBRARIAN TAB CALLBACKS ===
    // Send patch to synth
    librarianTab.onSendPatchToSynth = [this](const ToneParams& upper, const ToneParams& lower, const PatchParameters& patch) {
        audioProcessor.setToneParameters(true, upper, true);
        audioProcessor.setToneParameters(false, lower, true);
        audioProcessor.setPatchParameters(patch, true);
        DBG("Librarian: Sent complete patch to synth");
    };
    
    // Send raw SysEx data to synth - send to BOTH tones so presets play on whole keyboard
    librarianTab.onSendSysExToSynth = [this](const juce::MemoryBlock& sysexData) {
        audioProcessor.getMidiHandler().sendRawSysExToBothTones(sysexData);
        DBG("Librarian: Sent " + juce::String(sysexData.getSize()) + " bytes of SysEx to BOTH tones");
    };
    
    // Request patch from synth
    librarianTab.onRequestPatchFromSynth = [this]() {
        audioProcessor.requestPatchDump();
        DBG("Librarian: Requested patch from synth");
    };
    
    // Request tones from synth
    librarianTab.onRequestToneUpperFromSynth = [this]() {
        audioProcessor.requestToneDump(true);
        DBG("Librarian: Requested Upper tone from synth");
    };
    
    librarianTab.onRequestToneLowerFromSynth = [this]() {
        audioProcessor.requestToneDump(false);
        DBG("Librarian: Requested Lower tone from synth");
    };
    
    // Get current parameters from editor
    librarianTab.onGetCurrentToneUpper = [this]() -> ToneParams {
        return audioProcessor.getToneUpper();
    };
    
    librarianTab.onGetCurrentToneLower = [this]() -> ToneParams {
        return audioProcessor.getToneLower();
    };
    
    librarianTab.onGetCurrentPatch = [this]() -> PatchParameters {
        return audioProcessor.getPatchParameters();
    };
    
    // Load tone/patch to editor
    librarianTab.onLoadToneToEditor = [this](const ToneParams& tone, bool isUpper) {
        if (isUpper)
        {
            toneUpperPanel.setToneParams(tone);
            audioProcessor.setToneParameters(true, tone, false);
            DBG("Librarian: Loaded preset to Upper tone");
        }
        else
        {
            toneLowerPanel.setToneParams(tone);
            audioProcessor.setToneParameters(false, tone, false);
            DBG("Librarian: Loaded preset to Lower tone");
        }
    };
    
    librarianTab.onLoadPatchToEditor = [this](const PatchParameters& patch) {
        audioProcessor.setPatchParameters(patch, false);
        DBG("Librarian: Loaded patch to editor");
    };
    
    // Connect LibrarianTab to HeaderBar - sync current preset
    librarianTab.onPresetLoaded = [this](int index, const juce::String& name) {
        headerBar.setCurrentPresetIndex(index);
        headerBar.setPresetName(name);
        miniPresetLabel.setText(name, juce::dontSendNotification);
        bottomPatchName.setText(name, juce::dontSendNotification);
        DBG("HeaderBar synced to preset index " + juce::String(index) + ": " + name);
    };
    
    // Connect HeaderBar navigation to Librarian
    headerBar.onNavigatePreset = [this](int index) {
        // Navigate in Librarian and load/send the preset
        librarianTab.selectPresetByIndex(index);
        
        // Update preset name in header
        auto name = librarianTab.getPresetName(index);
        if (name.isNotEmpty())
        {
            headerBar.setPresetName(name);
            miniPresetLabel.setText(name, juce::dontSendNotification);
            bottomPatchName.setText(name, juce::dontSendNotification);
        }
    };
    
    // Provide preset count to HeaderBar from Librarian
    headerBar.onGetPresetCount = [this]() {
        return librarianTab.getPresetCount();
    };
    
    // Initialize preset list in HeaderBar from Librarian (with longer delay to ensure presets are loaded)
    juce::Timer::callAfterDelay(1000, [this]() {
        auto names = librarianTab.getPresetNames();
        if (!names.empty())
        {
            headerBar.setPresetList(names);
            DBG("HeaderBar preset list initialized with " + juce::String(names.size()) + " presets");
        }
        else
        {
            DBG("WARNING: No presets found in Librarian!");
        }
    });
    
    // Initialize compare panel
    addChildComponent(comparePanel);
    comparePanel.setProcessor(&audioProcessor);
    comparePanel.setVisible(false);
    
    // Initialize morph panel
    addChildComponent(morphPanel);
    morphPanel.setProcessor(&audioProcessor);
    morphPanel.setVisible(false);
    
    // Initialize random panel
    addChildComponent(randomPanel);
    randomPanel.setProcessor(&audioProcessor);
    randomPanel.setVisible(false);
    randomPanel.onClose = [this]() { hideRandomPanel(); };
    randomPanel.onPatchApplied = [this]() { refreshUI(); };
    
    // Initialize macro strip
    addAndMakeVisible(macroStrip);
    macroStrip.setMacroManager(&macroManager);
    macroStrip.onSetupMacro = [this](int macroIndex) { showMacroSetupDialog(macroIndex); };
    
    // Connect MacroTab to macro manager
    macroTab.setMacroManager(&macroManager);

    // Automation playback drives the same parameter path as UI edits/MIDI Learn.
    automationPanel.onPlaybackValue = [this](const juce::String& paramId, int value)
    {
        applyAutomationValue(paramId, value);
        statusBar.flashMidiOut();
    };

    // Zoom control in toolbar
    addAndMakeVisible(zoomCombo);
    zoomCombo.addItem("80%", 80);
    zoomCombo.addItem("100%", 100);
    zoomCombo.addItem("125%", 125);
    zoomCombo.addItem("150%", 150);
    zoomCombo.setSelectedId(100, juce::dontSendNotification);
    zoomCombo.onChange = [this]()
    {
        int id = zoomCombo.getSelectedId();
        if (id <= 0) id = 100;
        zoomFactor = (float) id / 100.0f;
        applyZoom();
    };
    zoomCombo.setTooltip("Zoom (Ctrl+= / Ctrl+- / Ctrl+0)");
    
    // Set the editor size with resizable constraints (Stage 4: prevent clipping)
    setSize (1000, 700);  // Reduced height to fit standard 768px screens
    setResizable(true, true);  // Allow window resizing
    setResizeLimits(900, 600, 3840, 2160);  // Reduced minimum to fit smaller screens
    setWantsKeyboardFocus(true);
    
    // For standalone builds, configure the parent window with all buttons
    #if JucePlugin_Build_Standalone
    juce::Timer::callAfterDelay(100, [this]()
    {
        if (auto* topLevelComp = getTopLevelComponent())
        {   
            if (auto* window = dynamic_cast<juce::DocumentWindow*>(topLevelComp))
            {
                // Enable all title bar buttons including maximize
                window->setTitleBarButtonsRequired(
                    juce::DocumentWindow::minimiseButton | 
                    juce::DocumentWindow::maximiseButton | 
                    juce::DocumentWindow::closeButton, 
                    false);
                window->setResizable(true, true);
                window->setResizeLimits(900, 750, 3840, 2160);
            }
        }
    });
    #endif
    
    // Initial UI refresh
    refreshUI();
    layoutScrollableTabs();

    // MIDI Learn: connect raw MIDI from processor's MidiHandler to manager
    audioProcessor.getMidiHandler().onRawMidi = [this](const juce::MidiMessage& msg) {
        midiLearnManager.processMidiMessage(msg);
        // Flash MIDI In indicator on status bar
        statusBar.flashMidiIn();
    };
    
    // Initialize status bar
    addAndMakeVisible(statusBar);
    
    // Initialize keyboard shortcuts overlay
    addChildComponent(shortcutsOverlay);
    shortcutsOverlay.setVisible(false);
    shortcutsOverlay.onClose = [this]() {
        shortcutsOverlayVisible = false;
        shortcutsOverlay.setVisible(false);
    };
    
    // Initialize toast notification manager
    ToastManager::getInstance().setParentComponent(this);

    // Optional: react to learn complete (visual cue)
    midiLearnManager.onLearnComplete = [this](const MidiLearnManager::MidiMapping&){ 
        ToastManager::getInstance().showSuccess("MIDI Learn complete");
        repaint(); 
    };
    
    // CRITICAL: Connect MIDI Learn to synth output
    // When a mapped controller changes a parameter, send it to the MKS-70
    midiLearnManager.onMappedParameterChange = [this](const juce::String& paramId, int value) {
        // Parse paramId to determine tone (UPPER/LOWER) and SysEx address
        // Format: "PREFIX_SECTION_PARAM" e.g., "UPPER_DCO1_TUNE", "LOWER_VCF_CUTOFF"
        
        bool isUpper = paramId.startsWith("UPPER");
        
        // Get SysEx address from parameter name
        int sysexAddr = getSysExAddressFromParamId(paramId);
        
        if (sysexAddr >= 0 && sysexAddr != MKS70Mapping::PARAM_NOT_AVAILABLE) {
            audioProcessor.setToneParameter(isUpper, sysexAddr, value);
            DBG("MIDI Learn -> Synth: " + paramId + " = " + juce::String(value) + " (SysEx addr: 0x" + juce::String::toHexString(sysexAddr) + ")");
            
            // Flash MIDI Out indicator
            statusBar.flashMidiOut();
        } else {
            DBG("MIDI Learn: Unknown parameter: " + paramId);
        }
    };
    
    // Initialize preset list from filesystem
    refreshPresetList();
}

static juce::String normalizeParamIdForMapping(juce::String paramId)
{
    paramId = paramId.toUpperCase();
    paramId = paramId.replaceCharacter('.', '_')
                     .replaceCharacter(' ', '_')
                     .replaceCharacter('-', '_')
                     .replaceCharacter('/', '_');
    paramId = paramId.replace(">", "_")
                     .replace("__", "_");

    return paramId;
}

void MKS70EditorAudioProcessorEditor::applyAutomationValue(const juce::String& paramId, int value)
{
    const int clampedValue = juce::jlimit(0, 127, value);
    const auto normalized = normalizeParamIdForMapping(paramId);

    if (normalized.startsWith("PATCH_"))
    {
        juce::String param = normalized.substring(6);
        int patchParam = -1;

        if (param == "TONE_SELECT") patchParam = 0x10;
        else if (param == "KEY_MODE") patchParam = 0x11;
        else if (param == "BALANCE" || param == "UPPER_LOWER_BALANCE") patchParam = 0x12;
        else if (param == "ASSIGN_MODE") patchParam = 0x13;
        else if (param == "PORTAMENTO_SWITCH") patchParam = 0x14;
        else if (param == "PORTAMENTO_TIME") patchParam = 0x15;
        else if (param == "GLIDE") patchParam = 0x16;
        else if (param == "BEND_RANGE") patchParam = 0x18;
        else if (param == "SPLIT_POINT" || param == "UPPER_SPLIT") patchParam = 0x19;
        else if (param == "LOWER_SPLIT") patchParam = 0x1A;
        else if (param == "TOTAL_VOLUME") patchParam = 0x1C;
        else if (param == "UNISON_DETUNE") patchParam = 0x1D;
        else if (param == "CHASE_PLAY") patchParam = 0x20;
        else if (param == "CHASE_LEVEL") patchParam = 0x21;
        else if (param == "CHASE_TIME") patchParam = 0x22;

        if (patchParam >= 0)
        {
            audioProcessor.setPatchParameter(patchParam, clampedValue);
            refreshUI();
        }

        return;
    }

    const bool isLower = normalized.startsWith("LOWER_");
    const int sysexAddr = getSysExAddressFromParamId(normalized);

    if (sysexAddr >= 0 && sysexAddr != MKS70Mapping::PARAM_NOT_AVAILABLE)
    {
        audioProcessor.setToneParameter(!isLower, sysexAddr, clampedValue);
        refreshUI();
    }
}

// Helper function to map parameter names to SysEx addresses
int MKS70EditorAudioProcessorEditor::getSysExAddressFromParamId(const juce::String& paramId)
{
    // Remove tone prefix to get parameter name
    juce::String param = normalizeParamIdForMapping(paramId);
    if (param.startsWith("UPPER_")) param = param.substring(6);
    else if (param.startsWith("LOWER_")) param = param.substring(6);
    
    // DCO1 parameters
    if (param == "DCO1_RANGE") return MKS70Mapping::DCO1_RANGE_SYSEX;
    if (param == "DCO1_WAVE" || param == "DCO1_WAVEFORM") return MKS70Mapping::DCO1_WAVEFORM_SYSEX;
    if (param == "DCO1_TUNE") return MKS70Mapping::DCO1_TUNE_SYSEX;
    if (param == "DCO1_LFO" || param == "DCO1_LFO_MOD") return MKS70Mapping::DCO1_LFO_DEPTH_SYSEX;
    if (param == "DCO1_ENV" || param == "DCO1_ENV_MOD") return MKS70Mapping::DCO1_ENV_DEPTH_SYSEX;
    if (param == "DCO1_DYNAMICS") return MKS70Mapping::DCO1_DYNAMICS_SYSEX;
    if (param == "DCO1_ENV_SRC") return MKS70Mapping::DCO1_ENV_SOURCE_SYSEX;
    if (param == "DCO1_PWM_WIDTH") return MKS70Mapping::PWM_DCO1_WIDTH_SYSEX;
    if (param == "DCO1_PWM_LFO") return MKS70Mapping::PWM_DCO1_LFO_DEPTH_SYSEX;
    if (param == "DCO1_PWM_ENV") return MKS70Mapping::PWM_DCO1_ENV_DEPTH_SYSEX;
    
    // DCO2 parameters
    if (param == "DCO2_RANGE") return MKS70Mapping::DCO2_RANGE_SYSEX;
    if (param == "DCO2_WAVE" || param == "DCO2_WAVEFORM") return MKS70Mapping::DCO2_WAVEFORM_SYSEX;
    if (param == "DCO2_TUNE") return MKS70Mapping::DCO2_TUNE_SYSEX;
    if (param == "DCO2_FINE" || param == "DCO2_FINE_TUNE") return MKS70Mapping::DCO2_FINETUNE_SYSEX;
    if (param == "DCO2_LFO" || param == "DCO2_LFO_MOD") return MKS70Mapping::DCO2_LFO_DEPTH_SYSEX;
    if (param == "DCO2_ENV" || param == "DCO2_ENV_MOD") return MKS70Mapping::DCO2_ENV_DEPTH_SYSEX;
    if (param == "DCO2_DYNAMICS") return MKS70Mapping::DCO2_DYNAMICS_SYSEX;
    if (param == "DCO2_ENV_SRC") return MKS70Mapping::DCO2_ENV_SOURCE_SYSEX;
    if (param == "DCO2_CROSSMOD" || param == "DCO2_CROSS_MOD" || param == "DCO2_XMOD") return MKS70Mapping::DCO_CROSSMOD_SYSEX;
    if (param == "DCO2_PWM_WIDTH") return MKS70Mapping::PWM_DCO2_WIDTH_SYSEX;
    if (param == "DCO2_PWM_LFO") return MKS70Mapping::PWM_DCO2_LFO_DEPTH_SYSEX;
    if (param == "DCO2_PWM_ENV") return MKS70Mapping::PWM_DCO2_ENV_DEPTH_SYSEX;
    
    // Mixer parameters
    if (param == "MIXER_DCO1" || param == "MIX_DCO1") return MKS70Mapping::MIXER_DCO1_LEVEL_SYSEX;
    if (param == "MIXER_DCO2" || param == "MIX_DCO2") return MKS70Mapping::MIXER_DCO2_LEVEL_SYSEX;
    if (param == "MIXER_ENV" || param == "MIXER_ENVELOPE" || param == "MIX_ENV") return MKS70Mapping::MIXER_ENV_DEPTH_SYSEX;
    if (param == "MIXER_DYNAMICS" || param == "MIX_DYN") return MKS70Mapping::MIXER_DYNAMICS_SYSEX;
    if (param == "MIXER_ENV_SRC" || param == "MIXER_MODE" || param == "MIX_ENV_SRC") return MKS70Mapping::MIXER_ENV_SOURCE_SYSEX;
    
    // VCF parameters
    if (param == "VCF_CUTOFF" || param == "VCF_FREQ") return MKS70Mapping::VCF_CUTOFF_SYSEX;
    if (param == "VCF_RESONANCE" || param == "VCF_RES") return MKS70Mapping::VCF_RESONANCE_SYSEX;
    if (param == "VCF_LFO" || param == "VCF_LFO_MOD") return MKS70Mapping::VCF_LFO1_DEPTH_SYSEX;
    if (param == "VCF_ENV" || param == "VCF_ENV_MOD" || param == "VCF_ENV_DEPTH") return MKS70Mapping::VCF_ENV_DEPTH_SYSEX;
    if (param == "VCF_KEY" || param == "VCF_KEY_FOLLOW") return MKS70Mapping::VCF_KEY_FOLLOW_SYSEX;
    if (param == "VCF_DYNAMICS" || param == "VCF_DYN") return MKS70Mapping::VCF_DYNAMICS_SYSEX;
    if (param == "VCF_ENV_SRC") return MKS70Mapping::VCF_ENV_SOURCE_SYSEX;
    if (param == "VCF_HPF" || param == "VCF_HPF_CUTOFF" || param == "HPF") return MKS70Mapping::HPF_CUTOFF_SYSEX;
    
    // VCA parameters
    if (param == "VCA_LEVEL") return MKS70Mapping::VCA_LEVEL_SYSEX;
    if (param == "VCA_ENV_SRC") return MKS70Mapping::VCA_ENV_SOURCE_SYSEX;
    if (param == "VCA_DYNAMICS" || param == "VCA_DYN") return MKS70Mapping::VCA_DYNAMICS_SYSEX;
    
    // LFO1 parameters
    if (param == "LFO_WAVE" || param == "LFO_WAVEFORM" || param == "LFO1_WAVE" || param == "LFO1_WAVEFORM") return MKS70Mapping::LFO1_WAVEFORM_SYSEX;
    if (param == "LFO_DELAY" || param == "LFO1_DELAY") return MKS70Mapping::LFO1_DELAY_SYSEX;
    if (param == "LFO_RATE" || param == "LFO1_RATE") return MKS70Mapping::LFO1_RATE_SYSEX;
    
    // LFO2 parameters (Vecoven)
    if (param == "LFO2_WAVE") return MKS70Mapping::LFO2_WAVEFORM_SYSEX;
    if (param == "LFO2_DELAY") return MKS70Mapping::LFO2_DELAY_SYSEX;
    if (param == "LFO2_RATE") return MKS70Mapping::LFO2_RATE_SYSEX;
    
    // ENV1 parameters (7-stage)
    if (param == "ENV1_A" || param == "ENV1_ATTACK") return MKS70Mapping::ENV1_TIME1_SYSEX;
    if (param == "ENV1_D" || param == "ENV1_DECAY") return MKS70Mapping::ENV1_TIME2_SYSEX;
    if (param == "ENV1_S" || param == "ENV1_SUSTAIN") return MKS70Mapping::ENV1_LEVEL3_SYSEX;
    if (param == "ENV1_R" || param == "ENV1_RELEASE") return MKS70Mapping::ENV1_TIME4_SYSEX;
    if (param == "ENV1_KEY_FOLLOW") return MKS70Mapping::ENV1_KEYFOLLOW_LOOP_SYSEX;
    
    // ENV2 parameters (7-stage)
    if (param == "ENV2_A" || param == "ENV2_ATTACK") return MKS70Mapping::ENV2_TIME1_SYSEX;
    if (param == "ENV2_D" || param == "ENV2_DECAY") return MKS70Mapping::ENV2_TIME2_SYSEX;
    if (param == "ENV2_S" || param == "ENV2_SUSTAIN") return MKS70Mapping::ENV2_LEVEL3_SYSEX;
    if (param == "ENV2_R" || param == "ENV2_RELEASE") return MKS70Mapping::ENV2_TIME4_SYSEX;
    if (param == "ENV2_KEY_FOLLOW") return MKS70Mapping::ENV2_KEYFOLLOW_LOOP_SYSEX;
    
    // ENV3 parameters (ADSR - VCF)
    if (param == "ENV3_A" || param == "ENV3_ATTACK") return MKS70Mapping::ENV3_ATTACK_SYSEX;
    if (param == "ENV3_D" || param == "ENV3_DECAY") return MKS70Mapping::ENV3_DECAY_SYSEX;
    if (param == "ENV3_S" || param == "ENV3_SUSTAIN") return MKS70Mapping::ENV3_SUSTAIN_SYSEX;
    if (param == "ENV3_R" || param == "ENV3_RELEASE") return MKS70Mapping::ENV3_RELEASE_SYSEX;
    
    // ENV4 parameters (ADSR - VCA)
    if (param == "ENV4_A" || param == "ENV4_ATTACK") return MKS70Mapping::ENV4_ATTACK_SYSEX;
    if (param == "ENV4_D" || param == "ENV4_DECAY") return MKS70Mapping::ENV4_DECAY_SYSEX;
    if (param == "ENV4_S" || param == "ENV4_SUSTAIN") return MKS70Mapping::ENV4_SUSTAIN_SYSEX;
    if (param == "ENV4_R" || param == "ENV4_RELEASE") return MKS70Mapping::ENV4_RELEASE_SYSEX;
    
    // Chorus
    if (param == "CHORUS" || param == "CHORUS_TYPE") return MKS70Mapping::CHORUS_SYSEX;
    
    return -1;  // Unknown parameter
}

MKS70EditorAudioProcessorEditor::~MKS70EditorAudioProcessorEditor()
{
    // Reset LookAndFeel to avoid dangling reference
    setLookAndFeel(nullptr);
}

//==============================================================================
void MKS70EditorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Main background
    g.fillAll(juce::Colour(0xFF0D0D0F));
    
    auto bounds = getLocalBounds();
    
    // Top mini preset bar background
    auto topBar = bounds.removeFromTop(20);
    g.setColour(juce::Colour(0xFF15151A));
    g.fillRect(topBar);
    g.setColour(juce::Colour(0xFF2D2D3A));
    g.fillRect(topBar.getX(), topBar.getBottom() - 1, topBar.getWidth(), 1);
    
    // Bottom toolbar background
    auto bottomArea = bounds.removeFromBottom(28);
    g.setColour(juce::Colour(0xFF1A1A1F));
    g.fillRect(bottomArea);
    g.setColour(juce::Colour(0xFF2D2D3A));
    g.fillRect(bottomArea.getX(), bottomArea.getY(), bottomArea.getWidth(), 1);
    
    // Vertical tab bar background (60px with animated tabs)
    auto tabBarArea = bounds.removeFromLeft(60);
    g.setColour(juce::Colour(0xFF16161E));
    g.fillRect(tabBarArea);
    g.setColour(juce::Colour(0xFF2D2D3A));
    g.fillRect(tabBarArea.getRight() - 1, tabBarArea.getY(), 1, tabBarArea.getHeight());
}

void MKS70EditorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // === NO HEADER BAR - Everything moved to unified toolbar ===
    headerBar.setVisible(false);
    expandBar.setVisible(false);
    toolbar.setVisible(false);
    macroStrip.setVisible(false);
    statusBar.setVisible(false);
    
    // === UNIFIED TOOLBAR (36px) at TOP - Combines preset nav + MIDI + controls ===
    auto unifiedToolbar = bounds.removeFromTop(36);
    layoutUnifiedToolbar(unifiedToolbar);
    
    // === VERTICAL TAB BAR ON LEFT (60px) - Animated icons with breathing glow ===
    auto tabBar = bounds.removeFromLeft(60);
    layoutVerticalTabs(tabBar);
    
    // === CONTENT AREA ===
    // Hide the TabbedComponent's own tab bar
    tabbedComponent.setTabBarDepth(0);
    tabbedComponent.setBounds(bounds);
    
    // If compare panel is visible, show it as overlay
    if (comparePanelVisible)
    {
        auto panelBounds = getLocalBounds().reduced(50);
        comparePanel.setBounds(panelBounds);
    }
    
    // If morph panel is visible, show it as overlay
    if (morphPanelVisible)
    {
        auto panelBounds = getLocalBounds().reduced(40);
        morphPanel.setBounds(panelBounds);
    }
    
    // If random panel is visible, show it as overlay
    if (randomPanelVisible)
    {
        auto panelBounds = getLocalBounds().reduced(40);
        randomPanel.setBounds(panelBounds);
    }
    
    // If shortcuts overlay is visible, show centered
    if (shortcutsOverlayVisible)
    {
        auto overlayBounds = getLocalBounds().reduced(60, 40);
        shortcutsOverlay.setBounds(overlayBounds);
    }

    // Ensure scrollable pages match new sizes
    layoutScrollableTabs();
}

void MKS70EditorAudioProcessorEditor::layoutVerticalTabs(juce::Rectangle<int> area)
{
    // Fill entire left side with elegant tab buttons
    const int numTabs = (int)verticalTabButtons.size();
    if (numTabs == 0) return;
    
    // Calculate tab height to fill available space
    const int totalHeight = area.getHeight();
    const int padding = 8;  // Top and bottom padding
    const int gap = 4;      // Gap between buttons
    const int availableHeight = totalHeight - (padding * 2) - (gap * (numTabs - 1));
    const int tabHeight = availableHeight / numTabs;
    
    area.removeFromTop(padding);
    
    for (size_t i = 0; i < verticalTabButtons.size(); ++i)
    {
        auto& btn = verticalTabButtons[i];
        btn->setBounds(area.removeFromTop(tabHeight).reduced(4, 2));
        if (i < verticalTabButtons.size() - 1)
            area.removeFromTop(gap);
    }
}

void MKS70EditorAudioProcessorEditor::layoutUnifiedToolbar(juce::Rectangle<int> area)
{
    auto inner = area.reduced(4, 2);
    
    // === LEFT SECTION: Firmware + Category + Preset Navigation ===
    firmwareCombo.setBounds(inner.removeFromLeft(60).reduced(0, 2));
    inner.removeFromLeft(4);
    miniCategoryCombo.setBounds(inner.removeFromLeft(70).reduced(0, 2));
    inner.removeFromLeft(8);
    
    // Preset navigation (centered group)
    auto presetNav = inner.removeFromLeft(220);
    miniPrevBtn.setBounds(presetNav.removeFromLeft(24).reduced(2, 3));
    miniPresetLabel.setBounds(presetNav.removeFromLeft(168).reduced(2, 2));
    miniNextBtn.setBounds(presetNav.removeFromLeft(24).reduced(2, 3));
    inner.removeFromLeft(8);
    
    miniMenuBtn.setBounds(inner.removeFromLeft(28).reduced(2, 3));
    inner.removeFromLeft(16);
    
    // === CENTER SECTION: MIDI LED + Morph Slider (IMPROVED) ===
    bottomMidiLED.setBounds(inner.removeFromLeft(8).reduced(0, 4));
    inner.removeFromLeft(12);
    
    // Morph Slider with UPPER/LOWER labels and percentage
    morphLabelLeft.setBounds(inner.removeFromLeft(38).reduced(0, 3));   // "UPPER"
    inner.removeFromLeft(4);
    morphSlider.setBounds(inner.removeFromLeft(120).reduced(0, 4));     // Wider slider
    inner.removeFromLeft(4);
    morphLabel.setBounds(inner.removeFromLeft(32).reduced(0, 3));       // "50%"
    inner.removeFromLeft(4);
    morphLabelRight.setBounds(inner.removeFromLeft(38).reduced(0, 3));  // "LOWER"
    inner.removeFromLeft(12);
    
    // === RIGHT SECTION: Controls ===
    bottomSettingsBtn.setBounds(inner.removeFromRight(40).reduced(0, 3));
    inner.removeFromRight(4);
    bottomCompareBtn.setBounds(inner.removeFromRight(32).reduced(0, 3));
    inner.removeFromRight(4);
    bottomRedoBtn.setBounds(inner.removeFromRight(24).reduced(0, 3));
    inner.removeFromRight(2);
    bottomUndoBtn.setBounds(inner.removeFromRight(24).reduced(0, 3));
}

void MKS70EditorAudioProcessorEditor::layoutBottomToolbar(juce::Rectangle<int> area)
{
    // Left side - MIDI LED only
    area.removeFromLeft(8);  // Left padding
    bottomMidiLED.setBounds(area.removeFromLeft(8).reduced(0, 4));
    area.removeFromLeft(4);
    
    // Right side - Settings, Compare, Undo/Redo
    bottomSettingsBtn.setBounds(area.removeFromRight(40).reduced(0, 3));
    area.removeFromRight(4);
    bottomCompareBtn.setBounds(area.removeFromRight(32).reduced(0, 3));
    area.removeFromRight(4);
    bottomRedoBtn.setBounds(area.removeFromRight(24).reduced(0, 3));
    area.removeFromRight(2);
    bottomUndoBtn.setBounds(area.removeFromRight(24).reduced(0, 3));
}

//==============================================================================
void MKS70EditorAudioProcessorEditor::refreshUI()
{
    // Refresh all tabs to display current parameter values
    toneUpperTab.refreshFromProcessor();
    toneLowerTab.refreshFromProcessor();
    toneUpperPanel.refreshFromParams(audioProcessor.getToneUpper());
    toneLowerPanel.refreshFromParams(audioProcessor.getToneLower());
    patchTab.refreshFromProcessor();
    arpTab.refreshFromProcessor();
}

void MKS70EditorAudioProcessorEditor::refreshAllPanels()
{
    // Full refresh after Total Recall - update all panels and UI elements
    DBG("Total Recall: Refreshing all panels...");
    
    // Refresh main tabs
    toneUpperTab.refreshFromProcessor();
    toneLowerTab.refreshFromProcessor();
    patchTab.refreshFromProcessor();
    arpTab.refreshFromProcessor();
    
    // Update tone panels
    toneUpperPanel.refreshFromParams(audioProcessor.getToneUpper());
    toneLowerPanel.refreshFromParams(audioProcessor.getToneLower());
    
    // Refresh settings if visible
    settingsTab.refreshFromProcessor();
    
    // Flash MIDI indicators to show Total Recall activity
    statusBar.flashMidiIn();
    statusBar.flashMidiOut();
    
    DBG("Total Recall: State restored from DAW project");
    
    // Trigger full repaint
    repaint();
}

void MKS70EditorAudioProcessorEditor::updateUndoRedoButtons()
{
    auto& undoMgr = audioProcessor.getUndoManager();
    
    // Update enabled state
    bottomUndoBtn.setEnabled(undoMgr.canUndo());
    bottomRedoBtn.setEnabled(undoMgr.canRedo());
    
    // Update colors based on state
    if (undoMgr.canUndo())
    {
        bottomUndoBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::cyan());
        bottomUndoBtn.setTooltip("Undo: " + undoMgr.getUndoDescription());
    }
    else
    {
        bottomUndoBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textDisabled());
        bottomUndoBtn.setTooltip("Nothing to undo");
    }
    
    if (undoMgr.canRedo())
    {
        bottomRedoBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::cyan());
        bottomRedoBtn.setTooltip("Redo: " + undoMgr.getRedoDescription());
    }
    else
    {
        bottomRedoBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textDisabled());
        bottomRedoBtn.setTooltip("Nothing to redo");
    }
}

void MKS70EditorAudioProcessorEditor::showComparePanel()
{
    // Store original state when entering compare mode
    audioProcessor.storeCurrentToSlot(CompareManager::Slot::SLOT_ORIGINAL);
    
    comparePanelVisible = true;
    comparePanel.setVisible(true);
    comparePanel.toFront(true);
    comparePanel.grabKeyboardFocus();
    comparePanel.updateSlotNames();
    comparePanel.refreshDifferences();
    resized();
    
    updateCompareIndicator();
    
    DBG("Compare panel shown");
}

void MKS70EditorAudioProcessorEditor::hideComparePanel()
{
    comparePanelVisible = false;
    comparePanel.setVisible(false);
    resized();
    
    updateCompareIndicator();
    
    DBG("Compare panel hidden");
}

CompareManager::Slot MKS70EditorAudioProcessorEditor::getActiveSlot() const
{
    // Get active slot from compare manager
    return audioProcessor.compareManager.getActiveSlot();
}

void MKS70EditorAudioProcessorEditor::setupToolbar()
{
    addAndMakeVisible(toolbar);
    toolbar.setOpaque(true);
    
    // Compare button
    toolbar.addAndMakeVisible(compareButton);
    compareButton.setButtonText("Compare A/B");
    compareButton.setColour(juce::TextButton::buttonColourId, Theme::accentPrimary());
    compareButton.onClick = [this]() { showComparePanel(); };
    
    // Morph button
    toolbar.addAndMakeVisible(morphButton);
    morphButton.setButtonText("Morph");
    morphButton.setColour(juce::TextButton::buttonColourId, Theme::accentWarn());
    morphButton.onClick = [this]() { showMorphPanel(); };
    
    // Random button
    toolbar.addAndMakeVisible(randomButton);
    randomButton.setButtonText("Random");
    randomButton.setColour(juce::TextButton::buttonColourId, Theme::accentOk());
    randomButton.onClick = [this]() { showRandomPanel(); };
    
    // Compare indicator
    toolbar.addAndMakeVisible(compareIndicator);
    compareIndicator.setText("", juce::dontSendNotification);
    compareIndicator.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    compareIndicator.setJustificationType(juce::Justification::centred);
    compareIndicator.setColour(juce::Label::backgroundColourId, juce::Colour(0xff404040));
    compareIndicator.setColour(juce::Label::textColourId, juce::Colours::white);
    compareIndicator.setVisible(false);
    
    // Layout toolbar
    auto toolbarBounds = juce::Rectangle<int>(0, 0, getWidth(), 40);
    toolbarBounds.reduce(5, 5);
    
    compareButton.setBounds(toolbarBounds.removeFromLeft(120).reduced(2));
    toolbarBounds.removeFromLeft(5);
    morphButton.setBounds(toolbarBounds.removeFromLeft(80).reduced(2));
    toolbarBounds.removeFromLeft(5);
    randomButton.setBounds(toolbarBounds.removeFromLeft(80).reduced(2));
    toolbarBounds.removeFromLeft(10);
    compareIndicator.setBounds(toolbarBounds.removeFromLeft(80).reduced(2));
    toolbarBounds.removeFromRight(10);
    // Zoom combo on the right
    if (zoomCombo.getNumItems() > 0)
        zoomCombo.setBounds(toolbarBounds.removeFromRight(120).reduced(2));
}

void MKS70EditorAudioProcessorEditor::updateCompareIndicator()
{
    if (comparePanelVisible)
    {
        auto activeSlot = getActiveSlot();
        juce::String text = (activeSlot == CompareManager::Slot::SLOT_A) ? "SLOT A" : "SLOT B";
        juce::Colour color = (activeSlot == CompareManager::Slot::SLOT_A) ? 
            Theme::accentPrimary() : Theme::accentWarn();
        
        compareIndicator.setText(text, juce::dontSendNotification);
        compareIndicator.setColour(juce::Label::backgroundColourId, color);
        compareIndicator.setVisible(true);
    }
    else
    {
        compareIndicator.setVisible(false);
    }
}

//==============================================================================
// VERTICAL TABS SETUP
//==============================================================================
void MKS70EditorAudioProcessorEditor::setupVerticalTabs()
{
    // Icon symbols and labels for each tab
    const char* tabLabels[] = {
        "UPPER",     // Upper tone
        "LOWER",     // Lower tone  
        "PATCH",     // Patch
        "ARP",       // Arpeggiator
        "LIBRARY",   // Library
        "AUTO",      // Automation
        "MOD",       // Modulation (was Automation)
        "MACRO",     // Macros
        "AI",        // AI Assistant
        "SETTINGS"   // Settings (includes MIDI Test)
    };
    const char* tabIcons[] = {
        "U",    // Upper
        "L",    // Lower
        "P",    // Patch
        "A",    // Arp
        "LIB",  // Library
        "AUT",  // Automation
        "MOD",  // Modulation (was Automation)
        "M",    // Macros
        "AI",   // AI Assistant
        "SET"   // Settings
    };
    const char* tabTooltips[] = {"Upper Tone (1)", "Lower Tone (2)", "Patch (3)", "Arpeggiator (4)", "Library (5)", "Automation (6)", "Modulation (7)", "Macros (8)", "AI Assistant (9)", "Settings"};
    const juce::Colour tabColors[] = {
        juce::Colour(0xFF22D3EE),  // Cyan for Upper
        juce::Colour(0xFFEC4899),  // Magenta for Lower
        juce::Colour(0xFF6366F1),  // Indigo for Patch
        juce::Colour(0xFF8B5CF6),  // Purple for Arp
        juce::Colour(0xFFF59E0B),  // Amber for Library
        juce::Colour(0xFF10B981),  // Green for Automation
        juce::Colour(0xFFBA68C8),  // Purple for Modulation (similar to LFO color)
        juce::Colour(0xFFEF4444),  // Red for Macros
        juce::Colour(0xFF06B6D4),  // Teal for AI Assistant
        juce::Colour(0xFF64748B)   // Slate for Settings
    };
    
    for (int i = 0; i < 10; ++i)
    {
        auto btn = std::make_unique<AnimatedTabButton>(tabLabels[i], tabIcons[i], tabColors[i], i);
        btn->setTooltip(tabTooltips[i]);
        btn->onClick = [this, i]() { selectTab(i); };
        addAndMakeVisible(btn.get());
        verticalTabButtons.push_back(std::move(btn));
    }
    
    // Select first tab
    verticalTabButtons[0]->setToggleState(true, juce::dontSendNotification);
    currentTabIndex = 0;
}

void MKS70EditorAudioProcessorEditor::setupBottomToolbar()
{
    // Logo label
    bottomLogoLabel.setText("MKS-70", juce::dontSendNotification);
    // MIDI activity LED
    bottomMidiLED.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF10B981).darker(0.8f));
    bottomMidiLED.setJustificationType(juce::Justification::centred);
    bottomMidiLED.setText("", juce::dontSendNotification);
    addAndMakeVisible(bottomMidiLED);
    
    // Undo button
    bottomUndoBtn.setButtonText("Un");
    bottomUndoBtn.setTooltip("Undo (Ctrl+Z)");
    bottomUndoBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF252530));
    bottomUndoBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF94A3B8));
    bottomUndoBtn.onClick = [this]() {
        auto& undoMgr = audioProcessor.getUndoManager();
        if (undoMgr.canUndo()) {
            auto desc = undoMgr.getUndoDescription();
            undoMgr.undo();
            DBG("Undo: " + desc);
            refreshUI();
            updateUndoRedoButtons();
        }
    };
    addAndMakeVisible(bottomUndoBtn);
    
    // Redo button
    bottomRedoBtn.setButtonText("Re");
    bottomRedoBtn.setTooltip("Redo (Ctrl+Y)");
    bottomRedoBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF252530));
    bottomRedoBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF94A3B8));
    bottomRedoBtn.onClick = [this]() {
        auto& undoMgr = audioProcessor.getUndoManager();
        if (undoMgr.canRedo()) {
            auto desc = undoMgr.getRedoDescription();
            undoMgr.redo();
            DBG("Redo: " + desc);
            refreshUI();
            updateUndoRedoButtons();
        }
    };
    addAndMakeVisible(bottomRedoBtn);
    
    // Register undo manager callback
    audioProcessor.getUndoManager().onHistoryChanged = [this]() {
        updateUndoRedoButtons();
    };
    
    // Initial button state
    updateUndoRedoButtons();
    
    // Morph Slider (Upper/Lower Balance) - IMPROVED with percentage display
    // Left label: UPPER
    morphLabelLeft.setText("UPPER", juce::dontSendNotification);
    morphLabelLeft.setFont(juce::Font(9.0f, juce::Font::bold));
    morphLabelLeft.setColour(juce::Label::textColourId, DS::Color::Accent::cyan());
    morphLabelLeft.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(morphLabelLeft);
    
    // Center percentage label
    morphLabel.setText("50%", juce::dontSendNotification);
    morphLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    morphLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
    morphLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(morphLabel);
    
    // Right label: LOWER
    morphLabelRight.setText("LOWER", juce::dontSendNotification);
    morphLabelRight.setFont(juce::Font(9.0f, juce::Font::bold));
    morphLabelRight.setColour(juce::Label::textColourId, DS::Color::Accent::magenta());
    morphLabelRight.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(morphLabelRight);
    
    morphSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    morphSlider.setRange(0, 100, 1);  // Use percentage 0-100
    morphSlider.setValue(50);  // Center = equal balance
    morphSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    morphSlider.setColour(juce::Slider::backgroundColourId, DS::Color::bgInput());
    morphSlider.setColour(juce::Slider::thumbColourId, DS::Color::Accent::magenta());
    morphSlider.setColour(juce::Slider::trackColourId, DS::Color::Accent::magenta().withAlpha(0.4f));
    morphSlider.setTooltip("Upper/Lower Balance\n0% = Full Upper, 100% = Full Lower");
    morphSlider.onValueChange = [this]() {
        int percent = (int)morphSlider.getValue();
        int midiValue = (percent * 127) / 100;  // Convert % to MIDI 0-127
        
        // Update center label with percentage
        morphLabel.setText(juce::String(percent) + "%", juce::dontSendNotification);
        
        // Update colors based on position
        if (percent < 40) {
            morphLabel.setColour(juce::Label::textColourId, DS::Color::Accent::cyan());
        } else if (percent > 60) {
            morphLabel.setColour(juce::Label::textColourId, DS::Color::Accent::magenta());
        } else {
            morphLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        }
        
        // Send MIDI: Upper/Lower Balance is a global parameter
        audioProcessor.getMidiHandler().sendGlobalParameter(MKS70Mapping::GLOBAL_UL_BALANCE_SYSEX, midiValue);
        DBG("Morph (U/L Balance): " + juce::String(percent) + "% (MIDI: " + juce::String(midiValue) + ")");
    };
    addAndMakeVisible(morphSlider);
    
    // A/B Compare button
    bottomCompareBtn.setButtonText("A/B");
    bottomCompareBtn.setClickingTogglesState(true);
    bottomCompareBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF252530));
    bottomCompareBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFEC4899).darker(0.2f));
    bottomCompareBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF94A3B8));
    bottomCompareBtn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFEC4899));
    bottomCompareBtn.setTooltip("Compare A/B");
    bottomCompareBtn.onClick = [this]() {
        if (bottomCompareBtn.getToggleState())
            showComparePanel();
        else
            hideComparePanel();
    };
    addAndMakeVisible(bottomCompareBtn);
    
    // Settings button - toggles between settings and previous tab
    bottomSettingsBtn.setButtonText("SET");
    bottomSettingsBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF252530));
    bottomSettingsBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF94A3B8));
    bottomSettingsBtn.setTooltip("Settings (click again to return)");
    bottomSettingsBtn.onClick = [this]() {
        const int settingsTabIndex = 9;  // Settings is last after Automation/Modulation/AI
        if (currentTabIndex == settingsTabIndex) {
            // Already on settings - go back to previous tab
            selectTab(previousTabIndex);
        } else {
            // Go to settings
            selectTab(settingsTabIndex);
        }
    };
    addAndMakeVisible(bottomSettingsBtn);
}

void MKS70EditorAudioProcessorEditor::selectTab(int index)
{
    if (index < 0 || index >= tabbedComponent.getNumTabs()) return;
    
    // Track previous tab (but not if switching to/from settings)
    const int settingsTabIndex = 9;  // Settings is last after Automation/Modulation/AI
    if (currentTabIndex != settingsTabIndex && index != currentTabIndex) {
        previousTabIndex = currentTabIndex;
    }
    
    currentTabIndex = index;
    tabbedComponent.setCurrentTabIndex(index);
    
    // Update toggle states
    for (size_t i = 0; i < verticalTabButtons.size(); ++i)
        verticalTabButtons[i]->setToggleState((int)i == index, juce::dontSendNotification);
}

//==============================================================================
// MINI PRESET BAR
//==============================================================================
void MKS70EditorAudioProcessorEditor::setupMiniPresetBar()
{
    // Preset name label (clickable for menu)
    miniPresetLabel.setText("INIT PATCH", juce::dontSendNotification);
    miniPresetLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    miniPresetLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFE2E8F0));
    miniPresetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(miniPresetLabel);
    
    // Previous preset button
    miniPrevBtn.setButtonText("<");
    miniPrevBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    miniPrevBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF6366F1));
    miniPrevBtn.setTooltip("Previous preset");
    miniPrevBtn.onClick = [this]() { loadPrevPreset(); };
    addAndMakeVisible(miniPrevBtn);
    
    // Next preset button
    miniNextBtn.setButtonText(">");
    miniNextBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    miniNextBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF6366F1));
    miniNextBtn.setTooltip("Next preset");
    miniNextBtn.onClick = [this]() { loadNextPreset(); };
    addAndMakeVisible(miniNextBtn);
    
    // Menu button (3 dots or hamburger)
    miniMenuBtn.setButtonText("...");
    miniMenuBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    miniMenuBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF94A3B8));
    miniMenuBtn.setTooltip("Preset menu");
    miniMenuBtn.onClick = [this]() { 
        // Show preset browser or menu
        selectTab(4);  // Go to Library tab
    };
    addAndMakeVisible(miniMenuBtn);
    
    // Category selector
    miniCategoryCombo.addItem("All", 1);
    miniCategoryCombo.addItem("Bass", 2);
    miniCategoryCombo.addItem("Pads", 3);
    miniCategoryCombo.addItem("Leads", 4);
    miniCategoryCombo.addItem("Keys", 5);
    miniCategoryCombo.addItem("Brass", 6);
    miniCategoryCombo.addItem("Strings", 7);
    miniCategoryCombo.addItem("FX", 8);
    miniCategoryCombo.setSelectedId(1);
    miniCategoryCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF15151A));
    miniCategoryCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xFF94A3B8));
    miniCategoryCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF2D2D3A));
    miniCategoryCombo.setTooltip("Filter by category");
    addAndMakeVisible(miniCategoryCombo);
    
    // Firmware version selector
    firmwareCombo.addItem("Original", 1);
    firmwareCombo.addItem("V3.x", 2);
    firmwareCombo.addItem("V4.x", 3);
    firmwareCombo.setSelectedId(2); // Default to V3.x
    firmwareCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF15151A));
    firmwareCombo.setColour(juce::ComboBox::textColourId, DS::Color::Accent::cyan());
    firmwareCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF2D2D3A));
    firmwareCombo.setTooltip("MKS-70 Firmware Version\n\nOriginal = Standard ROM\nV3.x = Vecoven 3.x\nV4.x = Extended (LFO2, PWM)");
    firmwareCombo.onChange = [this]() {
        int selected = firmwareCombo.getSelectedId();
        FirmwareVersion fw = FirmwareVersion::Vecoven3;
        if (selected == 1) fw = FirmwareVersion::Original;
        else if (selected == 2) fw = FirmwareVersion::Vecoven3;
        else if (selected == 3) fw = FirmwareVersion::Vecoven4;
        toneUpperPanel.setFirmwareVersion(fw);
        toneLowerPanel.setFirmwareVersion(fw);
        DBG("Firmware changed to: " + juce::String(selected));
    };
    addAndMakeVisible(firmwareCombo);
}

void MKS70EditorAudioProcessorEditor::layoutMiniPresetBar(juce::Rectangle<int> area)
{
    auto inner = area.reduced(4, 2);
    
    // Left side: Category
    miniCategoryCombo.setBounds(inner.removeFromLeft(70).reduced(0, 2));
    inner.removeFromLeft(8);
    
    // Center: Preset navigation
    auto centerArea = inner.withSizeKeepingCentre(200, inner.getHeight());
    miniPrevBtn.setBounds(centerArea.removeFromLeft(24).reduced(2, 3));
    miniNextBtn.setBounds(centerArea.removeFromRight(24).reduced(2, 3));
    miniPresetLabel.setBounds(centerArea.reduced(2, 2));
    
    // Right side: Menu
    miniMenuBtn.setBounds(inner.removeFromRight(28).reduced(2, 3));
}

void MKS70EditorAudioProcessorEditor::applyZoom()
{
    // Apply scaling only to tab content, not the tab bar
    toneUpperPanel.setTransform(juce::AffineTransform::scale(zoomFactor));
    toneLowerPanel.setTransform(juce::AffineTransform::scale(zoomFactor));
    patchTab.setTransform(juce::AffineTransform::scale(zoomFactor));
    arpTab.setTransform(juce::AffineTransform::scale(zoomFactor));
    librarianTab.setTransform(juce::AffineTransform::scale(zoomFactor));
    automationPanel.setTransform(juce::AffineTransform::scale(zoomFactor));
    settingsTab.setTransform(juce::AffineTransform::scale(zoomFactor));
    modulationPanel.setTransform(juce::AffineTransform::scale(zoomFactor));
    macroTab.setTransform(juce::AffineTransform::scale(zoomFactor));
    layoutScrollableTabs();
    resized();
}

void MKS70EditorAudioProcessorEditor::showMorphPanel()
{
    morphPanelVisible = true;
    morphPanel.setVisible(true);
    morphPanel.toFront(true);
    morphPanel.grabKeyboardFocus();
    resized();
    
    DBG("Morph panel shown");
}

void MKS70EditorAudioProcessorEditor::hideMorphPanel()
{
    morphPanelVisible = false;
    morphPanel.setVisible(false);
    resized();
    
    DBG("Morph panel hidden");
}

void MKS70EditorAudioProcessorEditor::showRandomPanel()
{
    randomPanelVisible = true;
    randomPanel.setVisible(true);
    randomPanel.toFront(true);
    randomPanel.grabKeyboardFocus();
    resized();
    
    DBG("Random panel shown");
}

void MKS70EditorAudioProcessorEditor::hideRandomPanel()
{
    randomPanelVisible = false;
    randomPanel.setVisible(false);
    resized();
    
    DBG("Random panel hidden");
}

void MKS70EditorAudioProcessorEditor::showMacroSetupDialog(int macroIndex)
{
    // Create modal dialog for macro setup
    auto& macro = macroManager.getMacro(macroIndex);
    auto* dialog = new MacroSetupDialog(macro, macroManager);
    
    dialog->onClose = [dialog]()
    {
        delete dialog;
    };
    
    dialog->onMappingsChanged = [this]()
    {
        // Refresh macro strip display
        macroStrip.repaint();
    };
    
    // Show as modal dialog
    dialog->setAlwaysOnTop(true);
    dialog->addToDesktop(juce::ComponentPeer::windowIsTemporary | juce::ComponentPeer::windowHasTitleBar);
    dialog->centreWithSize(700, 600);
    dialog->setVisible(true);
    
    DBG("Macro setup dialog shown for macro " + juce::String(macroIndex));
}
bool MKS70EditorAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    // F1 - Show keyboard shortcuts
    if (key == juce::KeyPress::F1Key)
    {
        shortcutsOverlayVisible = !shortcutsOverlayVisible;
        shortcutsOverlay.setVisible(shortcutsOverlayVisible);
        if (shortcutsOverlayVisible)
        {
            shortcutsOverlay.toFront(true);
            shortcutsOverlay.grabKeyboardFocus();
            resized();
        }
        return true;
    }
    
    // Escape - Close any overlay
    if (key == juce::KeyPress::escapeKey)
    {
        if (shortcutsOverlayVisible)
        {
            shortcutsOverlayVisible = false;
            shortcutsOverlay.setVisible(false);
            return true;
        }
        if (comparePanelVisible)
        {
            hideComparePanel();
            return true;
        }
        if (morphPanelVisible)
        {
            hideMorphPanel();
            return true;
        }
        if (randomPanelVisible)
        {
            hideRandomPanel();
            return true;
        }
        return false;
    }
    
    const bool ctrl = key.getModifiers().isCtrlDown();
    if (!ctrl) return false;
    
    // Ctrl+1-9 - Tab navigation
    if (key.getKeyCode() >= '1' && key.getKeyCode() <= '9')
    {
        int tabIndex = key.getKeyCode() - '1';
        if (tabIndex < tabbedComponent.getNumTabs())
        {
            tabbedComponent.setCurrentTabIndex(tabIndex);
            return true;
        }
    }

    // Ctrl+Z -> Undo
    if (key.getKeyCode() == 'Z' && !key.getModifiers().isShiftDown())
    {
        if (audioProcessor.getUndoManager().canUndo())
        {
            auto actionName = audioProcessor.getUndoManager().getUndoDescription();
            audioProcessor.getUndoManager().undo();
            refreshUI();
            ToastManager::getInstance().showUndo(actionName);
            DBG("Undo performed: " + actionName);
        }
        return true;
    }
    
    // Ctrl+Y or Ctrl+Shift+Z -> Redo
    if (key.getKeyCode() == 'Y' || 
        (key.getKeyCode() == 'Z' && key.getModifiers().isShiftDown()))
    {
        if (audioProcessor.getUndoManager().canRedo())
        {
            auto actionName = audioProcessor.getUndoManager().getRedoDescription();
            audioProcessor.getUndoManager().redo();
            refreshUI();
            ToastManager::getInstance().showRedo(actionName);
            DBG("Redo performed: " + actionName);
        }
        return true;
    }

    auto setZoomId = [this](int id){
        if (zoomCombo.getSelectedId() != id)
            zoomCombo.setSelectedId(id, juce::sendNotification);
        else
            applyZoom();
    };

    // Ctrl+0 -> 100%
    if (key.getKeyCode() == '0') { setZoomId(100); return true; }

    // Ctrl+= or Ctrl++ -> zoom in
    if (key.getKeyCode() == '=' || key.getTextCharacter() == '+')
    {
        int id = zoomCombo.getSelectedId();
        if (id <= 0) id = 100;
        const int options[] = {80, 100, 125, 150};
        for (int i = 0; i < 4; ++i)
            if (options[i] == id && i < 3) { setZoomId(options[i+1]); return true; }
        setZoomId(150); // clamp to max
        return true;
    }

    // Ctrl+- -> zoom out
    if (key.getKeyCode() == '-' )
    {
        int id = zoomCombo.getSelectedId();
        if (id <= 0) id = 100;
        const int options[] = {80, 100, 125, 150};
        for (int i = 0; i < 4; ++i)
            if (options[i] == id && i > 0) { setZoomId(options[i-1]); return true; }
        setZoomId(80); // clamp to min
        return true;
    }

    return false;
}

void MKS70EditorAudioProcessorEditor::layoutScrollableTab(ScrollPage& page, juce::Component& content, int minHeight)
{
    const int pageWidth = page.getWidth();
    const int pageHeight = page.getHeight();
    if (pageWidth <= 0 || pageHeight <= 0) return;

    const float scale = juce::jmax(0.1f, zoomFactor);
    const int unscaledWidth = juce::jmax(1, juce::roundToInt(pageWidth / scale));
    const int unscaledHeight = juce::jmax(minHeight, juce::roundToInt(pageHeight / scale));

    content.setBounds(0, 0, unscaledWidth, unscaledHeight);
    page.container.setSize(pageWidth, juce::jmax(pageHeight, juce::roundToInt(unscaledHeight * scale)));
}

void MKS70EditorAudioProcessorEditor::layoutScrollableTabs()
{
    // Layout scrollable tone pages so vertical scroll works with zoom
    layoutScrollableTab(toneUpperPage, toneUpperPanel, getUpperBaseHeight());
    layoutScrollableTab(toneLowerPage, toneLowerPanel, getLowerBaseHeight());
    layoutScrollableTab(patchPage, patchTab, getPatchBaseHeight());
    layoutScrollableTab(arpPage, arpTab, getArpBaseHeight());
    layoutScrollableTab(librarianPage, librarianTab, getLibrarianBaseHeight());
    layoutScrollableTab(automationPage, automationPanel, getModulationBaseHeight());
    layoutScrollableTab(modulationPage, modulationPanel, getModulationBaseHeight());
    layoutScrollableTab(macroPage, macroTab, getMacroBaseHeight());
    layoutScrollableTab(aiAssistantPage, aiAssistantPanel, getAiAssistantBaseHeight());
    layoutScrollableTab(settingsPage, settingsTab, getSettingsBaseHeight());
    // MidiTestPanel layout handled inside SettingsTab
}

void MKS70EditorAudioProcessorEditor::toggleHeaderBar()
{
    headerBarVisible = !headerBarVisible;
    resized();
    DBG("Header bar visibility toggled: " + juce::String(headerBarVisible ? "visible" : "hidden"));
}

//==============================================================================
// Preset Navigation
//==============================================================================

void MKS70EditorAudioProcessorEditor::refreshPresetList()
{
    // Get presets from Presets folder (check multiple locations)
    // exe is in: Builds/VisualStudio2022/x64/Release/Standalone Plugin/ - need 5 levels up
    auto exeFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    DBG("EXE location: " + exeFile.getFullPathName());
    
    auto presetsFolder = exeFile
                            .getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory()
                            .getChildFile("Presets");
    
    DBG("Checking for presets in: " + presetsFolder.getFullPathName());
    
    if (!presetsFolder.exists())
    {
        DBG("Presets folder not found at exe-relative path, trying Documents...");
        presetsFolder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                            .getChildFile("MKS70 Editor").getChildFile("Presets");
        DBG("Trying Documents folder: " + presetsFolder.getFullPathName());
    }
    
    // Also check working directory
    if (!presetsFolder.exists())
    {
        presetsFolder = juce::File::getCurrentWorkingDirectory().getChildFile("Presets");
        DBG("Trying working directory: " + presetsFolder.getFullPathName());
    }
    
    presetFiles.clear();
    if (presetsFolder.exists())
    {
        presetsFolder.findChildFiles(presetFiles, juce::File::findFiles, true, "*.syx;*.mid;*.json;*.tone;*.patch");

        for (int i = presetFiles.size(); --i >= 0;)
        {
            const auto fileName = presetFiles[i].getFileName();
            const auto fullPath = presetFiles[i].getFullPathName();
            if (fileName.startsWith("._") || fullPath.containsIgnoreCase(juce::File::getSeparatorString() + "__MACOSX" + juce::File::getSeparatorString()))
                presetFiles.remove(i);
        }
    }
    
    if (presetFiles.isEmpty()) {
        DBG("No presets found in: " + presetsFolder.getFullPathName());
    }
    else {
        DBG("Found " + juce::String(presetFiles.size()) + " presets in: " + presetsFolder.getFullPathName());
    }
    
    // Update HeaderBar's preset list with actual file names
    juce::StringArray presetNames;
    for (auto& file : presetFiles)
        presetNames.add(file.getFileNameWithoutExtension());
    
    DBG("Setting " + juce::String(presetNames.size()) + " presets to HeaderBar");
    headerBar.setPresetList(presetNames);
}

void MKS70EditorAudioProcessorEditor::loadNextPreset()
{
    if (presetFiles.isEmpty())
        refreshPresetList();
    
    if (presetFiles.isEmpty())
        return;
    
    currentPresetIndex = (currentPresetIndex + 1) % presetFiles.size();
    auto presetFile = presetFiles[currentPresetIndex];
    
    // Actually load the preset file
    if (presetFile.existsAsFile())
    {
        if (presetFile.hasFileExtension(".syx"))
        {
            juce::MemoryBlock data;
            if (presetFile.loadFileAsData(data))
            {
                audioProcessor.getMidiHandler().loadSysExFromData(data.getData(), data.getSize());
                
                // Send to BOTH tones so presets play on whole keyboard
                audioProcessor.getMidiHandler().sendRawSysExToBothTones(data);
                // Set KeyMode to Whole (0) so both tones play across entire keyboard
                audioProcessor.getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_KEY_MODE, 0);
                
                refreshUI();
            }
        }
        // TODO: Add support for other formats
    }
    
    // Update all displays (mini toolbar, bottom patch name, header bar)
    auto presetName = presetFile.getFileNameWithoutExtension();
    miniPresetLabel.setText(presetName, juce::dontSendNotification);
    bottomPatchName.setText(presetName, juce::dontSendNotification);
    headerBar.setPatchName(presetName);
    
    DBG("Loaded preset: " + presetFile.getFullPathName());
}

void MKS70EditorAudioProcessorEditor::loadPrevPreset()
{
    if (presetFiles.isEmpty())
        refreshPresetList();
    
    if (presetFiles.isEmpty())
        return;
    
    currentPresetIndex = (currentPresetIndex - 1 + presetFiles.size()) % presetFiles.size();
    auto presetFile = presetFiles[currentPresetIndex];
    
    // Actually load the preset file
    if (presetFile.existsAsFile())
    {
        if (presetFile.hasFileExtension(".syx"))
        {
            juce::MemoryBlock data;
            if (presetFile.loadFileAsData(data))
            {
                audioProcessor.getMidiHandler().loadSysExFromData(data.getData(), data.getSize());
                
                // Send to BOTH tones so presets play on whole keyboard
                audioProcessor.getMidiHandler().sendRawSysExToBothTones(data);
                // Set KeyMode to Whole (0) so both tones play across entire keyboard
                audioProcessor.getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_KEY_MODE, 0);
                
                refreshUI();
            }
        }
        // TODO: Add support for other formats
    }
    
    // Update all displays (mini toolbar, bottom patch name, header bar)
    auto presetName = presetFile.getFileNameWithoutExtension();
    miniPresetLabel.setText(presetName, juce::dontSendNotification);
    bottomPatchName.setText(presetName, juce::dontSendNotification);
    headerBar.setPatchName(presetName);
    
    DBG("Loaded preset: " + presetFile.getFullPathName());
}

void MKS70EditorAudioProcessorEditor::updatePresetDisplay()
{
    // Called when preset changes from other source
    // miniPresetLabel and bottomPatchName should be synced
}

void MKS70EditorAudioProcessorEditor::applyTheme(int themeId)
{
    DBG("Applying theme: " << themeId);
    
    // Use ThemeManager to apply theme by index
    auto& themeManager = Theme::ThemeManager::getInstance();
    themeManager.setThemeByIndex(themeId - 1);  // ComboBox IDs are 1-based
    themeManager.applyToLookAndFeel(appLookAndFeel);
    
    // Get theme colors for direct application
    const auto& colors = themeManager.getColors();
    
    // Apply background color to this component
    setColour(juce::ResizableWindow::backgroundColourId, colors.background);
    
    // Force complete visual refresh - repaint triggers paint() which uses DS::Color
    // Since DS::Color now pulls from ThemeManager, all colors will be updated
    repaint();
    
    // Recursively repaint and re-lookup colors for all child components
    std::function<void(juce::Component*)> refreshRecursive = [&](juce::Component* comp) {
        if (comp == nullptr) return;
        
        // Trigger repaint which will use updated DS::Color values
        comp->repaint();
        
        // Force resized() to re-apply styling
        comp->resized();
        
        for (auto* child : comp->getChildren())
            refreshRecursive(child);
    };
    
    for (auto* child : getChildren())
        refreshRecursive(child);
        
    DBG("Theme applied: " << themeManager.getCurrentTheme().name);
}

// TonePanel::setEditor implementation (defined here to avoid circular dependency)
void TonePanel::setEditor(MKS70EditorAudioProcessorEditor* ed) {
    this->editor = ed;  // Store editor for MIDI transmission
    
    // Set up MIDI transmission function
    if (ed) {
        sendMidiParameter = [ed](int tone, int param, int value) {
            ed->getProcessor().setToneParameter(tone == MKS70Mapping::TONE_UPPER, param, value);
        };
    }
    
    juce::String prefix = upperTone ? "UPPER" : "LOWER";
    
    dco1->setEditor(ed, prefix + "_DCO1");
    dco2->setEditor(ed, prefix + "_DCO2");
    mixer->setEditor(ed, prefix);
    vcf->setEditor(ed, prefix);
    vca->setEditor(ed, prefix);
    env1->setEditor(ed, prefix);
    env2->setEditor(ed, prefix);
    lfo->setEditor(ed, prefix);
    chorus->setEditor(ed, prefix);
}
