/*
  ==============================================================================

    PatchTab.cpp
    Component for Patch parameters - Using new PatchPanel design

  ==============================================================================
*/

#include "PatchTab.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PatchTab::PatchTab()
{
    viewport.setViewedComponent(&panel, false);
    viewport.setScrollBarsShown(true, false);  // Horizontal scroll only
    addAndMakeVisible(viewport);
    
    // === Setup Listeners for real-time MIDI sending ===
    setupListeners();
}

void PatchTab::setupListeners()
{
    // Helper lambda for slider changes with MIDI send (RealisticPatchFader)
    auto setupSliderListener = [this](RealisticPatchFader& fader, int paramIndex, const juce::String& paramName) {
        fader.getSlider().onValueChange = [this, paramIndex, paramName, &fader]() {
            int value = static_cast<int>(fader.getSlider().getValue());
            if (processor != nullptr)
            {
                processor->setPatchParameter(paramIndex, value);
                DBG("Patch: " + paramName + " = " + juce::String(value));
            }
        };
    };
    
    // Helper lambda for combo box changes
    auto setupComboListener = [this](StyledComboBox& combo, int paramIndex, const juce::String& paramName) {
        combo.getComboBox().onChange = [this, paramIndex, paramName, &combo]() {
            int value = combo.getComboBox().getSelectedId() - 1;  // Convert to 0-based
            if (processor != nullptr)
            {
                processor->setPatchParameter(paramIndex, value);
                DBG("Patch: " + paramName + " = " + juce::String(value));
            }
        };
    };
    
    // Performance Section - Parameter indices based on MKS-70 SysEx mapping
    // Indices reference: Balance=12, Split=19/1A, Volume=1C, KeyMode, etc.
    setupComboListener(panel.getToneSelectCombo(), 0x10, "Tone Select");  // IPR 10
    setupComboListener(panel.getKeyModeCombo(), 0x11, "Key Mode");         // IPR 11
    setupSliderListener(panel.getSplitPointSlider(), 0x19, "Split Point"); // IPR 19
    setupSliderListener(panel.getBalanceSlider(), 0x12, "Balance");        // IPR 12
    setupSliderListener(panel.getTotalVolumeSlider(), 0x1C, "Total Volume"); // IPR 1C
    
    // Portamento Section
    panel.getPortaSwitch().onClick = [this]() {
        int value = panel.getPortaSwitch().getToggleState() ? 1 : 0;
        if (processor != nullptr)
        {
            processor->setPatchParameter(0x14, value);  // Portamento Switch
            DBG("Patch: Portamento = " + juce::String(value));
        }
    };
    
    panel.getGlideSwitch().onClick = [this]() {
        int value = panel.getGlideSwitch().getToggleState() ? 1 : 0;
        if (processor != nullptr)
        {
            processor->setPatchParameter(0x16, value);  // Glide Switch
            DBG("Patch: Glide = " + juce::String(value));
        }
    };
    
    setupSliderListener(panel.getPortaTimeSlider(), 0x15, "Portamento Time"); // IPR 15
    setupComboListener(panel.getPortaModeCombo(), 0x17, "Portamento Mode");   // IPR 17
    
    // Pitch / Assign Section
    setupSliderListener(panel.getBendRangeSlider(), 0x18, "Bend Range");      // IPR 18
    setupSliderListener(panel.getUnisonDetuneSlider(), 0x1D, "Unison Detune"); // IPR 1D
    setupComboListener(panel.getAssignModeCombo(), 0x13, "Assign Mode");      // IPR 13
    setupComboListener(panel.getHoldModeCombo(), 0x1E, "Hold Mode");          // IPR 1E
    
    // Chase Play Section
    panel.getChaseSwitch().onClick = [this]() {
        int value = panel.getChaseSwitch().getToggleState() ? 1 : 0;
        if (processor != nullptr)
        {
            processor->setPatchParameter(0x20, value);  // Chase Switch
            DBG("Patch: Chase = " + juce::String(value));
        }
    };
    
    setupSliderListener(panel.getChaseLevelSlider(), 0x21, "Chase Level");   // IPR 21
    setupSliderListener(panel.getChaseTimeSlider(), 0x22, "Chase Time");     // IPR 22
    setupComboListener(panel.getChaseModeCombo(), 0x23, "Chase Mode");       // IPR 23
    
    // === Action Buttons ===
    
    // Send All - Send complete patch to synth
    panel.getSendBtn().onClick = [this]() {
        if (processor != nullptr)
        {
            syncParametersFromPanel();
            processor->setPatchParameters(parameters, true);
            DBG("Patch: Sent all parameters to synth");
        }
    };
    
    // Request - Request patch from synth
    panel.getRequestBtn().onClick = [this]() {
        if (processor != nullptr)
        {
            processor->requestPatchDump();
            DBG("Patch: Requested patch from synth");
        }
    };
    
    // Init - Initialize to default values
    panel.getInitBtn().onClick = [this]() {
        initPatch();
        if (processor != nullptr)
        {
            processor->setPatchParameters(parameters, true);
        }
        DBG("Patch: Initialized to defaults");
    };
    
    // Copy
    panel.getCopyBtn().onClick = [this]() {
        copyToClipboard();
    };
    
    // Paste
    panel.getPasteBtn().onClick = [this]() {
        pasteFromClipboard();
        if (processor != nullptr)
        {
            processor->setPatchParameters(parameters, true);
        }
    };
    
    // Compare with Hardware
    panel.getCompareHwBtn().onClick = [this]() {
        if (processor != nullptr)
        {
            processor->requestPatchDump();
            DBG("Patch: Comparing with hardware state...");
        }
    };
    
    // Write to Memory (permanent save) - uses sendPatchToMemory
    panel.getWriteMemBtn().onClick = [this]() {
        if (processor != nullptr)
        {
            // Warning: this writes to MKS-70 memory!
            int bank = panel.getBankCombo().getComboBox().getSelectedId() - 1;
            int program = static_cast<int>(panel.getProgramSlider().getSlider().getValue()) - 1;
            // Use existing function that sends complete patch to memory
            processor->getMidiHandler().sendPatchToMemory(bank, program,
                processor->getToneUpper(), processor->getToneLower(), processor->getPatchParameters());
            DBG("Patch: Written to memory Bank " + juce::String(bank + 1) + " Program " + juce::String(program + 1));
        }
    };
    
    // Program navigation
    panel.getPrevBtn().onClick = [this]() {
        auto& slider = panel.getProgramSlider().getSlider();
        double newVal = slider.getValue() - 1;
        if (newVal >= slider.getMinimum())
        {
            slider.setValue(newVal);
            if (processor != nullptr)
            {
                // Send program change on MIDI channel 1
                processor->getMidiHandler().sendProgramChange(1, static_cast<int>(newVal) - 1);
            }
        }
    };
    
    panel.getNextBtn().onClick = [this]() {
        auto& slider = panel.getProgramSlider().getSlider();
        double newVal = slider.getValue() + 1;
        if (newVal <= slider.getMaximum())
        {
            slider.setValue(newVal);
            if (processor != nullptr)
            {
                // Send program change on MIDI channel 1
                processor->getMidiHandler().sendProgramChange(1, static_cast<int>(newVal) - 1);
            }
        }
    };
    
    // Bank change - send Bank Select CC and then Program Change
    panel.getBankCombo().getComboBox().onChange = [this]() {
        if (processor != nullptr)
        {
            int bank = panel.getBankCombo().getComboBox().getSelectedId() - 1;
            int program = static_cast<int>(panel.getProgramSlider().getSlider().getValue()) - 1;
            // Send Bank Select MSB (CC 0) followed by Program Change
            processor->getMidiHandler().sendCC(1, 0, bank);  // Bank Select MSB
            processor->getMidiHandler().sendProgramChange(1, program);
            DBG("Patch: Bank changed to " + juce::String(bank + 1));
        }
    };
    
    // Program slider change
    panel.getProgramSlider().getSlider().onValueChange = [this]() {
        if (processor != nullptr)
        {
            int program = static_cast<int>(panel.getProgramSlider().getSlider().getValue()) - 1;
            processor->getMidiHandler().sendProgramChange(1, program);
            DBG("Patch: Program changed to " + juce::String(program + 1));
        }
    };
}

PatchTab::~PatchTab()
{
    // Remove the panel from viewport before destruction
    viewport.setViewedComponent(nullptr, false);
}
void PatchTab::paint(juce::Graphics& g)
{
    // Panel handles its own painting
}

void PatchTab::resized()
{
    viewport.setBounds(getLocalBounds());
    // PatchPanel needs ~1000px width for 5-column layout with faders
    // Height: header(45) + toneNames(30) + gaps + sections with 5cm faders(~340) = ~500
    int panelWidth = juce::jmax(1000, getWidth());
    int panelHeight = juce::jmax(520, getHeight());  // Increased for 5cm faders + info boxes
    panel.setSize(panelWidth, panelHeight);
}

void PatchTab::syncPanelToParameters()
{
    // Sync parameters to panel controls
    panel.getToneSelectCombo().setSelectedId(parameters.toneSel + 1, juce::dontSendNotification);
    panel.getKeyModeCombo().setSelectedId(parameters.keyMode + 1, juce::dontSendNotification);
    panel.getSplitPointSlider().setValue(parameters.splitPoint, juce::dontSendNotification);
    panel.getBalanceSlider().setValue(parameters.balance, juce::dontSendNotification);
    
    panel.getPortaSwitch().setToggleState(parameters.portamentoSwitch != 0, juce::dontSendNotification);
    panel.getPortaTimeSlider().setValue(parameters.portamentoTime, juce::dontSendNotification);
    
    panel.getBendRangeSlider().setValue(parameters.bendRange, juce::dontSendNotification);
    panel.getUnisonDetuneSlider().setValue(parameters.unisonDetune, juce::dontSendNotification);
    panel.getAssignModeCombo().setSelectedId(parameters.assignMode + 1, juce::dontSendNotification);
    
    panel.getChaseSwitch().setToggleState(parameters.chasePlay != 0, juce::dontSendNotification);
    panel.getChaseLevelSlider().setValue(parameters.chaseLevel, juce::dontSendNotification);
    panel.getChaseTimeSlider().setValue(parameters.chaseTime, juce::dontSendNotification);
    
    panel.getPatchNameEditor().setText(parameters.getPatchName(), juce::dontSendNotification);
    panel.getToneANameEditor().setText(parameters.getToneName(), juce::dontSendNotification);
    panel.getToneBNameEditor().setText(parameters.getToneName(), juce::dontSendNotification);
}

void PatchTab::syncParametersFromPanel()
{
    parameters.toneSel = panel.getToneSelectCombo().getSelectedId() - 1;
    parameters.keyMode = panel.getKeyModeCombo().getSelectedId() - 1;
    parameters.splitPoint = static_cast<int>(panel.getSplitPointSlider().getValue());
    parameters.balance = static_cast<int>(panel.getBalanceSlider().getValue());
    parameters.portamentoSwitch = panel.getPortaSwitch().getToggleState() ? 1 : 0;
    parameters.portamentoTime = static_cast<int>(panel.getPortaTimeSlider().getValue());
    parameters.bendRange = static_cast<int>(panel.getBendRangeSlider().getValue());
    parameters.unisonDetune = static_cast<int>(panel.getUnisonDetuneSlider().getValue());
    parameters.assignMode = panel.getAssignModeCombo().getSelectedId() - 1;
    parameters.chasePlay = panel.getChaseSwitch().getToggleState() ? 1 : 0;
    parameters.chaseLevel = static_cast<int>(panel.getChaseLevelSlider().getValue());
    parameters.chaseTime = static_cast<int>(panel.getChaseTimeSlider().getValue());
    parameters.setPatchName(panel.getPatchNameEditor().getText());
    parameters.setToneName(panel.getToneANameEditor().getText());
}

void PatchTab::setParameters(const PatchParameters& params)
{
    parameters = params;
    syncPanelToParameters();
    repaint();
}

void PatchTab::refreshFromProcessor()
{
    if (processor != nullptr)
    {
        setParameters(processor->getPatchParameters());
    }
}

// Static members for clipboard
PatchParameters PatchTab::clipboard;
bool PatchTab::clipboardValid = false;

void PatchTab::copyToClipboard()
{
    syncParametersFromPanel();
    clipboard = parameters;
    clipboardValid = true;
    DBG("PatchTab: Copied settings to clipboard");
}

void PatchTab::pasteFromClipboard()
{
    if (clipboardValid)
    {
        setParameters(clipboard);
        DBG("PatchTab: Pasted settings from clipboard");
    }
    else
    {
        DBG("PatchTab: Clipboard is empty");
    }
}

void PatchTab::initPatch()
{
    PatchParameters defaultParams;
    setParameters(defaultParams);
    DBG("PatchTab: Initialized to default patch");
}
