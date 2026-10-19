/*
  ==============================================================================
    DeviceSettingsPanel.h - Compact MKS-70 Device Settings (No Scroll)
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "PluginProcessor.h"
#include "MKS70_Mapping.h"

//==============================================================================
class DeviceSettingsPanel : public juce::Component
{
public:
    DeviceSettingsPanel()
    {
        // Connection Row
        addLabel(connLabel, "Connection");
        midiInCombo.setTextWhenNothingSelected("MIDI In");
        styleCombo(midiInCombo); addAndMakeVisible(midiInCombo);
        midiInCombo.onChange = [this]() {
            if (processor) {
                int idx = midiInCombo.getSelectedId() - 1;
                processor->getMidiHandler().setInputDevice(idx);
                DBG("Device MIDI In selected: " << idx);
                updateConnectionStatus();
            }
        };
        
        midiOutCombo.setTextWhenNothingSelected("MIDI Out");
        styleCombo(midiOutCombo); addAndMakeVisible(midiOutCombo);
        midiOutCombo.onChange = [this]() {
            if (processor) {
                int idx = midiOutCombo.getSelectedId() - 1;
                processor->getMidiHandler().setOutputDevice(idx);
                DBG("Device MIDI Out selected: " << idx);
                updateConnectionStatus();
            }
        };
        
        testBtn.setButtonText("Test"); styleBtn(testBtn);
        testBtn.onClick = [this]{ testConnection(); };
        addAndMakeVisible(testBtn);
        
        refreshBtn.setButtonText("Refresh"); styleBtn(refreshBtn);
        refreshBtn.onClick = [this]{ refreshMidiDevices(); };
        addAndMakeVisible(refreshBtn);
        
        statusLabel.setText("Not connected", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, DS::Color::Accent::warning());
        addAndMakeVisible(statusLabel);
        
        // Controller MIDI Input Row
        addLabel(ctrlMidiLabel, "Controller");
        ctrlMidiInCombo.setTextWhenNothingSelected("Controller 1");
        styleCombo(ctrlMidiInCombo); addAndMakeVisible(ctrlMidiInCombo);
        ctrlMidiInCombo.onChange = [this]() {
            if (processor) {
                int idx = ctrlMidiInCombo.getSelectedId() - 2; // -2 because "None" is ID 1
                DBG("Controller 1 MIDI In selected: " << idx);
                processor->getMidiHandler().setControllerInput1(idx);
            }
        };
        
        ctrlMidiInCombo2.setTextWhenNothingSelected("Controller 2");
        styleCombo(ctrlMidiInCombo2); addAndMakeVisible(ctrlMidiInCombo2);
        ctrlMidiInCombo2.onChange = [this]() {
            if (processor) {
                int idx = ctrlMidiInCombo2.getSelectedId() - 2; // -2 because "None" is ID 1
                DBG("Controller 2 MIDI In selected: " << idx);
                processor->getMidiHandler().setControllerInput2(idx);
            }
        };
        
        // Channels Row
        addLabel(chLabel, "Channels");
        addLabel(ctrlLabel, "Ctrl:"); 
        for(int i=1;i<=16;++i) ctrlCh.addItem(juce::String(i),i);
        ctrlCh.setSelectedId(1); styleCombo(ctrlCh); addAndMakeVisible(ctrlCh);
        ctrlCh.onChange = [this]() {
            int ch = ctrlCh.getSelectedId() - 1; // 0-15
            DBG("Controller channel changed to: " << (ch + 1));
            if (processor) {
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_CTRL_CHANNEL, ch);
            }
        };
        
        addLabel(upperLabel, "Upper:");
        for(int i=1;i<=16;++i) upperCh.addItem(juce::String(i),i);
        upperCh.setSelectedId(1); styleCombo(upperCh); addAndMakeVisible(upperCh);
        upperCh.onChange = [this]() {
            int ch = upperCh.getSelectedId() - 1; // 0-15
            DBG("Upper channel changed to: " << (ch + 1));
            if (processor) {
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_UPPER_CHANNEL, ch);
            }
        };
        
        addLabel(lowerLabel, "Lower:");
        for(int i=1;i<=16;++i) lowerCh.addItem(juce::String(i),i);
        lowerCh.setSelectedId(2); styleCombo(lowerCh); addAndMakeVisible(lowerCh);
        lowerCh.onChange = [this]() {
            int ch = lowerCh.getSelectedId() - 1; // 0-15
            DBG("Lower channel changed to: " << (ch + 1));
            if (processor) {
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_LOWER_CHANNEL, ch);
            }
        };
        
        // SysEx Row
        addLabel(sysexLabel, "SysEx");
        sysexToggle.setButtonText("SysEx"); sysexToggle.setToggleState(true, juce::dontSendNotification);
        styleToggle(sysexToggle); addAndMakeVisible(sysexToggle);
        sysexToggle.onClick = [this]() {
            if (processor) {
                // Value: OFF=0, RECV=1, SEND=2, ON=3 (toggle just uses ON/OFF)
                int val = sysexToggle.getToggleState() ? 3 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_SYSEX_MODE, val);
            }
        };
        iprToggle.setButtonText("IPR"); iprToggle.setToggleState(true, juce::dontSendNotification);
        styleToggle(iprToggle); addAndMakeVisible(iprToggle);
        iprToggle.onClick = [this]() {
            if (processor) {
                int val = iprToggle.getToggleState() ? 3 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_SYSEX_IPR, val);
            }
        };
        aprToggle.setButtonText("APR"); aprToggle.setToggleState(true, juce::dontSendNotification);
        styleToggle(aprToggle); addAndMakeVisible(aprToggle);
        aprToggle.onClick = [this]() {
            if (processor) {
                int val = aprToggle.getToggleState() ? 3 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_SYSEX_APR, val);
            }
        };
        ccToggle.setButtonText("CC"); ccToggle.setToggleState(true, juce::dontSendNotification);
        styleToggle(ccToggle); addAndMakeVisible(ccToggle);
        ccToggle.onClick = [this]() {
            if (processor) {
                int val = ccToggle.getToggleState() ? 3 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_MIDI_CC, val);
            }
        };
        
        // Prog Change Row
        addLabel(progLabel, "Prog Change");
        patchProg.setButtonText("Patch"); patchProg.setToggleState(true, juce::dontSendNotification);
        styleToggle(patchProg); addAndMakeVisible(patchProg);
        patchProg.onClick = [this]() {
            if (processor) {
                int val = patchProg.getToggleState() ? 1 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_PATCH_PROG_CHG, val);
            }
        };
        upperProg.setButtonText("Upper"); upperProg.setToggleState(true, juce::dontSendNotification);
        styleToggle(upperProg); addAndMakeVisible(upperProg);
        upperProg.onClick = [this]() {
            if (processor) {
                int val = upperProg.getToggleState() ? 1 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_UPPER_PROG_CHG, val);
            }
        };
        lowerProg.setButtonText("Lower"); lowerProg.setToggleState(true, juce::dontSendNotification);
        styleToggle(lowerProg); addAndMakeVisible(lowerProg);
        lowerProg.onClick = [this]() {
            if (processor) {
                int val = lowerProg.getToggleState() ? 1 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_LOWER_PROG_CHG, val);
            }
        };
        
        // PG-800 Row
        addLabel(pgLabel, "Display");
        pg800Toggle.setButtonText("PG-800"); pg800Toggle.setToggleState(true, juce::dontSendNotification);
        styleToggle(pg800Toggle); addAndMakeVisible(pg800Toggle);
        pg800Toggle.onClick = [this]() {
            if (processor) {
                int val = pg800Toggle.getToggleState() ? 1 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_PG800, val);
            }
        };
        gu280Toggle.setButtonText("GU280"); gu280Toggle.setToggleState(false, juce::dontSendNotification);
        styleToggle(gu280Toggle); addAndMakeVisible(gu280Toggle);
        gu280Toggle.onClick = [this]() {
            if (processor) {
                int val = gu280Toggle.getToggleState() ? 1 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_GU280, val);
            }
        };
        addLabel(editDispLabel, "Edit:");
        editDispCombo.addItem("OFF",1); editDispCombo.addItem("SYSEX",2);
        editDispCombo.addItem("PG800",3); editDispCombo.addItem("CC",4); editDispCombo.addItem("ALL",5);
        editDispCombo.setSelectedId(5); styleCombo(editDispCombo); addAndMakeVisible(editDispCombo);
        editDispCombo.onChange = [this]() {
            if (processor) {
                int val = editDispCombo.getSelectedId() - 1; // OFF=0, SYSEX=1, PG800=2, CC=3, ALL=4
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_EDIT_DISPLAY, val);
            }
        };
        
        // Compat Row
        addLabel(compatLabel, "Compat");
        compatToggle.setButtonText("Compat Mode"); compatToggle.setToggleState(false, juce::dontSendNotification);
        styleToggle(compatToggle); addAndMakeVisible(compatToggle);
        compatToggle.onClick = [this]() {
            if (processor) {
                int val = compatToggle.getToggleState() ? 1 : 0;
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_COMPAT_MODE, val);
            }
        };
        addLabel(noteMsgLabel, "Notes:");
        noteMsgCombo.addItem("ALL",1); noteMsgCombo.addItem("ODD",2); noteMsgCombo.addItem("EVEN",3);
        noteMsgCombo.setSelectedId(1); styleCombo(noteMsgCombo); addAndMakeVisible(noteMsgCombo);
        noteMsgCombo.onChange = [this]() {
            if (processor) {
                int val = noteMsgCombo.getSelectedId() - 1; // ALL=0, ODD=1, EVEN=2
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::PARAM_NOTE_MSG, val);
            }
        };
        
        // Arp Row
        addLabel(arpLabel, "Arpeggio");
        addLabel(arpPatchLabel, "Patch:");
        for(int i=1;i<=8;++i) arpPatchCombo.addItem("ARP-"+juce::String(i),i);
        arpPatchCombo.setSelectedId(1); styleCombo(arpPatchCombo); addAndMakeVisible(arpPatchCombo);
        arpPatchCombo.onChange = [this]() {
            if (processor) {
                int patch = arpPatchCombo.getSelectedId() - 1; // 0-7
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::ARP_PATCH, patch);
            }
        };
        addLabel(bpmLabel, "BPM:");
        bpmSlider.setRange(40,300,1); bpmSlider.setValue(120);
        bpmSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        bpmSlider.setTextBoxStyle(juce::Slider::TextBoxRight,false,40,18);
        styleSlider(bpmSlider); addAndMakeVisible(bpmSlider);
        bpmSlider.onValueChange = [this]() {
            if (processor) {
                int bpm = static_cast<int>(bpmSlider.getValue());
                processor->getArpParameters().tempo = bpm;
                DBG("Arp BPM changed to: " << bpm);
                // BPM is scaled: 40-300 maps to 0-127 for MIDI
                int midiVal = juce::jlimit(0, 127, (bpm - 40) * 127 / 260);
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::ARP_BPM, midiVal);
            }
        };
        addLabel(modeLabel, "Mode:");
        modeCombo.addItem("UP",1); modeCombo.addItem("DOWN",2); modeCombo.addItem("U-D",3);
        modeCombo.addItem("PLAY",4); modeCombo.addItem("RND",5);
        modeCombo.setSelectedId(1); styleCombo(modeCombo); addAndMakeVisible(modeCombo);
        modeCombo.onChange = [this]() {
            if (processor) {
                int mode = modeCombo.getSelectedId() - 1; // 0-4
                processor->getArpParameters().mode = mode;
                DBG("Arp Mode changed to: " << mode);
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::ARP_MODE, mode);
            }
        };
        
        // Arp Row 2
        addLabel(octLabel, "Oct:");
        for(int i=1;i<=4;++i) octCombo.addItem(juce::String(i),i);
        octCombo.setSelectedId(1); styleCombo(octCombo); addAndMakeVisible(octCombo);
        octCombo.onChange = [this]() {
            if (processor) {
                int oct = octCombo.getSelectedId(); // 1-4
                processor->getArpParameters().octaveSpan = oct;
                DBG("Arp Octave changed to: " << oct);
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::ARP_OCTAVE, oct - 1); // 0-3
            }
        };
        addLabel(rateLabel, "Rate:");
        rateCombo.addItem("1/4",1); rateCombo.addItem("1/8",2); rateCombo.addItem("1/16",3);
        rateCombo.setSelectedId(2); styleCombo(rateCombo); addAndMakeVisible(rateCombo);
        rateCombo.onChange = [this]() {
            if (processor) {
                int rate = rateCombo.getSelectedId() - 1; // 0-2
                processor->getArpParameters().playRate = rate;
                DBG("Arp Rate changed to: " << rate);
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::ARP_RATE, rate);
            }
        };
        latchToggle.setButtonText("Latch"); latchToggle.setToggleState(false, juce::dontSendNotification);
        styleToggle(latchToggle); addAndMakeVisible(latchToggle);
        latchToggle.onClick = [this]() {
            if (processor) {
                int latch = latchToggle.getToggleState() ? 1 : 0;
                processor->getArpParameters().latch = latch;
                DBG("Arp Latch changed to: " << latch);
                processor->getMidiHandler().sendGlobalParameter(MKS70Mapping::ARP_LATCH, latch);
            }
        };
        
        // System Row
        addLabel(sysLabel, "System");
        reqBulkBtn.setButtonText("Request Bulk");
        reqBulkBtn.onClick = [this]() {
            if (processor) {
                if (!processor->getMidiHandler().isConnected()) {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "MIDI Not Connected",
                        "Please select MIDI In/Out devices first",
                        "OK"
                    );
                    return;
                }
                processor->getMidiHandler().requestBulkDump(0); // Request Bank 0
                DBG("DeviceSettings: Requesting bulk dump from bank 0");
                statusLabel.setText("Requesting bulk...", juce::dontSendNotification);
                statusLabel.setColour(juce::Label::textColourId, DS::Color::Accent::warning());
            }
        };
        styleBtn(reqBulkBtn); addAndMakeVisible(reqBulkBtn);
        
        sendBulkBtn.setButtonText("Send Bulk");
        sendBulkBtn.onClick = [this]() {
            if (processor) {
                // Send current patches as bulk dump
                // processor->getMidiHandler().sendBulkDump(processor->getPatchBank());
                DBG("DeviceSettings: Send bulk not yet implemented - needs patch bank access");
            }
        };
        styleBtn(sendBulkBtn); addAndMakeVisible(sendBulkBtn);
        
        writeBtn.setButtonText("Write Memory");
        writeBtn.onClick = [this]() {
            // Write current settings to MKS-70 internal memory
            // This requires specific SysEx command (not in current MidiHandler)
            DBG("DeviceSettings: Write to memory - user should press MIDI->WRITE->ENTER on hardware");
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                "Write to Memory",
                "To save settings to MKS-70 memory:\n\n"
                "1. Press MIDI button\n"
                "2. Press WRITE button\n"
                "3. Press ENTER button\n\n"
                "This will save current settings to internal memory.",
                "OK"
            );
        };
        styleBtn(writeBtn); addAndMakeVisible(writeBtn);
        
        resetBtn.setButtonText("Factory Reset");
        resetBtn.onClick = [this]() {
            juce::AlertWindow::showOkCancelBox(
                juce::AlertWindow::WarningIcon,
                "Factory Reset",
                "WARNING: Factory reset will erase all patches!\n\n"
                "To perform factory reset on MKS-70:\n\n"
                "1. Turn off the unit\n"
                "2. Hold WRITE + LEFT ARROW buttons\n"
                "3. Turn on the unit while holding buttons\n"
                "4. Release buttons when display shows reset\n\n"
                "Continue?",
                "OK",
                "Cancel",
                nullptr,
                juce::ModalCallbackFunction::create([](int result) {
                    if (result == 1) {
                        DBG("DeviceSettings: Factory reset instructions shown");
                    }
                })
            );
        };
        resetBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::error().darker(0.3f));
        styleBtn(resetBtn); addAndMakeVisible(resetBtn);
        
        refreshMidi();
    }
    
    void paint(juce::Graphics& g) override { g.fillAll(DS::Color::bg()); }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(12);
        int totalWidth = bounds.getWidth();
        int labelW = juce::jmax(90, totalWidth / 10);  // Responsive label width
        int gap = 8;
        int rowH = 28;
        
        // Row 1: Connection - use full width
        auto r1 = bounds.removeFromTop(rowH);
        connLabel.setBounds(r1.removeFromLeft(labelW));
        r1.removeFromLeft(gap);
        int remainingW = r1.getWidth() - 60 - 70 - 140 - 4*gap;  // test(60) + refresh(70) + status(140)
        int comboW = remainingW / 2;
        midiInCombo.setBounds(r1.removeFromLeft(comboW));
        r1.removeFromLeft(gap);
        midiOutCombo.setBounds(r1.removeFromLeft(comboW));
        r1.removeFromLeft(gap);
        testBtn.setBounds(r1.removeFromLeft(60));
        r1.removeFromLeft(gap);
        refreshBtn.setBounds(r1.removeFromLeft(70));
        r1.removeFromLeft(gap);
        statusLabel.setBounds(r1);
        bounds.removeFromTop(gap);
        
        // Row 1.5: Controller MIDI Inputs - two combos side by side
        auto r1_5 = bounds.removeFromTop(rowH);
        ctrlMidiLabel.setBounds(r1_5.removeFromLeft(labelW));
        r1_5.removeFromLeft(gap);
        int ctrlComboW = (r1_5.getWidth() - gap) / 2;  // Split remaining space for 2 combos
        ctrlMidiInCombo.setBounds(r1_5.removeFromLeft(ctrlComboW));
        r1_5.removeFromLeft(gap);
        ctrlMidiInCombo2.setBounds(r1_5);
        bounds.removeFromTop(gap);
        
        // Row 2: Channels - distribute evenly
        auto r2 = bounds.removeFromTop(rowH);
        chLabel.setBounds(r2.removeFromLeft(labelW));
        r2.removeFromLeft(gap);
        int chLabelW = 50;
        int chComboW = (r2.getWidth() - 3*chLabelW - 5*gap) / 3;
        ctrlLabel.setBounds(r2.removeFromLeft(chLabelW));
        r2.removeFromLeft(gap);
        ctrlCh.setBounds(r2.removeFromLeft(chComboW));
        r2.removeFromLeft(gap);
        upperLabel.setBounds(r2.removeFromLeft(chLabelW));
        r2.removeFromLeft(gap);
        upperCh.setBounds(r2.removeFromLeft(chComboW));
        r2.removeFromLeft(gap);
        lowerLabel.setBounds(r2.removeFromLeft(chLabelW));
        r2.removeFromLeft(gap);
        lowerCh.setBounds(r2.removeFromLeft(chComboW));
        bounds.removeFromTop(gap);
        
        // Row 3: SysEx - toggles
        auto r3 = bounds.removeFromTop(rowH);
        sysexLabel.setBounds(r3.removeFromLeft(labelW));
        r3.removeFromLeft(gap);
        int toggleW = (r3.getWidth() - 3*gap) / 4;
        sysexToggle.setBounds(r3.removeFromLeft(toggleW));
        r3.removeFromLeft(gap);
        iprToggle.setBounds(r3.removeFromLeft(toggleW));
        r3.removeFromLeft(gap);
        aprToggle.setBounds(r3.removeFromLeft(toggleW));
        r3.removeFromLeft(gap);
        ccToggle.setBounds(r3);
        bounds.removeFromTop(gap);
        
        // Row 4: Prog Change - toggles
        auto r4 = bounds.removeFromTop(rowH);
        progLabel.setBounds(r4.removeFromLeft(labelW));
        r4.removeFromLeft(gap);
        int progW = (r4.getWidth() - 2*gap) / 3;
        patchProg.setBounds(r4.removeFromLeft(progW));
        r4.removeFromLeft(gap);
        upperProg.setBounds(r4.removeFromLeft(progW));
        r4.removeFromLeft(gap);
        lowerProg.setBounds(r4);
        bounds.removeFromTop(gap);
        
        // Row 5: PG-800
        auto r5 = bounds.removeFromTop(rowH);
        pgLabel.setBounds(r5.removeFromLeft(labelW));
        r5.removeFromLeft(gap);
        int dispToggleW = 90;
        pg800Toggle.setBounds(r5.removeFromLeft(dispToggleW));
        r5.removeFromLeft(gap);
        gu280Toggle.setBounds(r5.removeFromLeft(dispToggleW));
        r5.removeFromLeft(gap);
        editDispLabel.setBounds(r5.removeFromLeft(50));
        r5.removeFromLeft(gap);
        editDispCombo.setBounds(r5);
        bounds.removeFromTop(gap);
        
        // Row 6: Compat
        auto r6 = bounds.removeFromTop(rowH);
        compatLabel.setBounds(r6.removeFromLeft(labelW));
        r6.removeFromLeft(gap);
        int compatToggleW = 130;
        int noteMsgLabelW = 50;
        compatToggle.setBounds(r6.removeFromLeft(compatToggleW));
        r6.removeFromLeft(gap);
        noteMsgLabel.setBounds(r6.removeFromLeft(noteMsgLabelW));
        r6.removeFromLeft(gap);
        noteMsgCombo.setBounds(r6);
        bounds.removeFromTop(gap);
        
        // Row 7: Arpeggio line 1
        auto r7 = bounds.removeFromTop(rowH);
        arpLabel.setBounds(r7.removeFromLeft(labelW));
        r7.removeFromLeft(gap);
        int arpLabelW = 45;
        int arpComboW = 70;
        int bpmLabelW = 35;
        int bpmSliderW = 120;
        int modeLabelW = 40;
        arpPatchLabel.setBounds(r7.removeFromLeft(arpLabelW));
        r7.removeFromLeft(gap);
        arpPatchCombo.setBounds(r7.removeFromLeft(arpComboW));
        r7.removeFromLeft(gap);
        bpmLabel.setBounds(r7.removeFromLeft(bpmLabelW));
        r7.removeFromLeft(gap);
        bpmSlider.setBounds(r7.removeFromLeft(bpmSliderW));
        r7.removeFromLeft(gap);
        modeLabel.setBounds(r7.removeFromLeft(modeLabelW));
        r7.removeFromLeft(gap);
        modeCombo.setBounds(r7);
        bounds.removeFromTop(gap);
        
        // Row 8: Arpeggio line 2
        auto r8 = bounds.removeFromTop(rowH);
        r8.removeFromLeft(labelW + gap);  // Indent to align with row 7
        int octLabelW = 30;
        int octComboW = 50;
        int rateLabelW = 35;
        int rateComboW = 60;
        octLabel.setBounds(r8.removeFromLeft(octLabelW));
        r8.removeFromLeft(gap);
        octCombo.setBounds(r8.removeFromLeft(octComboW));
        r8.removeFromLeft(gap);
        rateLabel.setBounds(r8.removeFromLeft(rateLabelW));
        r8.removeFromLeft(gap);
        rateCombo.setBounds(r8.removeFromLeft(rateComboW));
        r8.removeFromLeft(gap);
        latchToggle.setBounds(r8.removeFromLeft(80));
        bounds.removeFromTop(gap);
        
        // Row 9: System - buttons distributed
        auto r9 = bounds.removeFromTop(rowH);
        sysLabel.setBounds(r9.removeFromLeft(labelW));
        r9.removeFromLeft(gap);
        int btnW = (r9.getWidth() - 3*gap) / 4;
        reqBulkBtn.setBounds(r9.removeFromLeft(btnW));
        r9.removeFromLeft(gap);
        sendBulkBtn.setBounds(r9.removeFromLeft(btnW));
        r9.removeFromLeft(gap);
        writeBtn.setBounds(r9.removeFromLeft(btnW));
        r9.removeFromLeft(gap);
        resetBtn.setBounds(r9);
    }
    
    void setProcessor(MKS70EditorAudioProcessor* p) { processor = p; }
    
    void refreshMidiDevices()
    {
        DBG("DeviceSettingsPanel: Refreshing MIDI devices...");
        
        // Store current selections
        int currentInIdx = midiInCombo.getSelectedId() - 1;
        int currentOutIdx = midiOutCombo.getSelectedId() - 1;
        int currentCtrl1Idx = ctrlMidiInCombo.getSelectedId() - 2;
        int currentCtrl2Idx = ctrlMidiInCombo2.getSelectedId() - 2;
        
        // Clear all combos
        midiInCombo.clear();
        midiOutCombo.clear();
        ctrlMidiInCombo.clear();
        ctrlMidiInCombo2.clear();
        
        // Repopulate MIDI Input devices
        auto inDevices = juce::MidiInput::getAvailableDevices();
        midiInCombo.addItem("None", 1);
        for (int i = 0; i < inDevices.size(); ++i)
            midiInCombo.addItem(inDevices[i].name, i + 2);
        
        // Repopulate MIDI Output devices
        auto outDevices = juce::MidiOutput::getAvailableDevices();
        midiOutCombo.addItem("None", 1);
        for (int i = 0; i < outDevices.size(); ++i)
            midiOutCombo.addItem(outDevices[i].name, i + 2);
        
        // Repopulate Controller inputs (both combos)
        ctrlMidiInCombo.addItem("None", 1);
        ctrlMidiInCombo2.addItem("None", 1);
        for (int i = 0; i < inDevices.size(); ++i)
        {
            ctrlMidiInCombo.addItem(inDevices[i].name, i + 2);
            ctrlMidiInCombo2.addItem(inDevices[i].name, i + 2);
        }
        
        // Restore previous selections if still valid
        if (currentInIdx >= 0 && currentInIdx < inDevices.size())
            midiInCombo.setSelectedId(currentInIdx + 2, juce::dontSendNotification);
        else
            midiInCombo.setSelectedId(1, juce::dontSendNotification);
            
        if (currentOutIdx >= 0 && currentOutIdx < outDevices.size())
            midiOutCombo.setSelectedId(currentOutIdx + 2, juce::dontSendNotification);
        else
            midiOutCombo.setSelectedId(1, juce::dontSendNotification);
            
        if (currentCtrl1Idx >= 0 && currentCtrl1Idx < inDevices.size())
            ctrlMidiInCombo.setSelectedId(currentCtrl1Idx + 2, juce::dontSendNotification);
        else
            ctrlMidiInCombo.setSelectedId(1, juce::dontSendNotification);
            
        if (currentCtrl2Idx >= 0 && currentCtrl2Idx < inDevices.size())
            ctrlMidiInCombo2.setSelectedId(currentCtrl2Idx + 2, juce::dontSendNotification);
        else
            ctrlMidiInCombo2.setSelectedId(1, juce::dontSendNotification);
        
        DBG("DeviceSettingsPanel: Found " + juce::String(inDevices.size()) + " MIDI IN, " + juce::String(outDevices.size()) + " MIDI OUT devices");
        
        updateConnectionStatus();
    }
    
private:
    MKS70EditorAudioProcessor* processor = nullptr;
    
    // Labels
    juce::Label connLabel, chLabel, sysexLabel, progLabel, pgLabel, compatLabel, arpLabel, sysLabel;
    juce::Label ctrlLabel, upperLabel, lowerLabel, editDispLabel, noteMsgLabel;
    juce::Label arpPatchLabel, bpmLabel, modeLabel, octLabel, rateLabel;
    juce::Label statusLabel;
    juce::Label ctrlMidiLabel;  // NEW: Controller MIDI Input label
    
    // Connection
    juce::ComboBox midiInCombo, midiOutCombo;
    juce::TextButton testBtn;
    juce::TextButton refreshBtn;  // NEW: Refresh MIDI devices button
    juce::ComboBox ctrlMidiInCombo;  // Controller MIDI Input 1
    juce::ComboBox ctrlMidiInCombo2; // Controller MIDI Input 2 (NEW)
    
    // Channels
    juce::ComboBox ctrlCh, upperCh, lowerCh;
    
    // SysEx
    juce::ToggleButton sysexToggle, iprToggle, aprToggle, ccToggle;
    
    // Prog Change
    juce::ToggleButton patchProg, upperProg, lowerProg;
    
    // PG-800
    juce::ToggleButton pg800Toggle, gu280Toggle;
    juce::ComboBox editDispCombo;
    
    // Compat
    juce::ToggleButton compatToggle;
    juce::ComboBox noteMsgCombo;
    
    // Arp
    juce::ComboBox arpPatchCombo, modeCombo, octCombo, rateCombo;
    juce::Slider bpmSlider;
    juce::ToggleButton latchToggle;
    
    // System
    juce::TextButton reqBulkBtn, sendBulkBtn, writeBtn, resetBtn;
    
    void addLabel(juce::Label& l, const juce::String& text)
    {
        l.setText(text, juce::dontSendNotification);
        l.setColour(juce::Label::textColourId, DS::Color::Accent::primary());
        l.setFont(DS::Font::small());
        addAndMakeVisible(l);
    }
    
    void styleCombo(juce::ComboBox& c)
    {
        c.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
        c.setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
        c.setColour(juce::ComboBox::outlineColourId, DS::Color::border());
    }
    
    void styleToggle(juce::ToggleButton& t)
    {
        t.setColour(juce::ToggleButton::textColourId, DS::Color::textPrimary());
        t.setColour(juce::ToggleButton::tickColourId, DS::Color::Accent::cyan());
    }
    
    void updateConnectionStatus()
    {
        if (!processor) return;
        
        auto& midi = processor->getMidiHandler();
        if (midi.isConnected()) {
            statusLabel.setText("Connected", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, DS::Color::Accent::success());
        } else {
            statusLabel.setText("Not connected", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, DS::Color::Accent::warning());
        }
    }
    
    void styleSlider(juce::Slider& s)
    {
        s.setColour(juce::Slider::thumbColourId, DS::Color::Accent::primary());
        s.setColour(juce::Slider::textBoxTextColourId, DS::Color::textPrimary());
        s.setColour(juce::Slider::textBoxBackgroundColourId, DS::Color::bgInput());
    }
    
    void styleBtn(juce::TextButton& b)
    {
        b.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        b.setColour(juce::TextButton::textColourOffId, DS::Color::textPrimary());
    }
    
    void refreshMidi()
    {
        midiInCombo.clear(); midiOutCombo.clear(); ctrlMidiInCombo.clear(); ctrlMidiInCombo2.clear();
        
        // Add "None" option for controller inputs (ID 1)
        ctrlMidiInCombo.addItem("None", 1);
        ctrlMidiInCombo2.addItem("None", 1);
        
        int inputId = 2; // Start from ID 2 for actual devices
        for(auto& d : juce::MidiInput::getAvailableDevices()) {
            midiInCombo.addItem(d.name, midiInCombo.getNumItems()+1);
            ctrlMidiInCombo.addItem(d.name, inputId);
            ctrlMidiInCombo2.addItem(d.name, inputId);
            inputId++;
        }
        for(auto& d : juce::MidiOutput::getAvailableDevices())
            midiOutCombo.addItem(d.name, midiOutCombo.getNumItems()+1);
    }
    
    void testConnection()
    {
        if (!processor) {
            statusLabel.setText("No processor", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, DS::Color::Accent::error());
            return;
        }
        
        auto& midi = processor->getMidiHandler();
        if (!midi.isConnected()) {
            statusLabel.setText("Not connected", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, DS::Color::Accent::error());
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "MIDI Not Connected",
                "Please select MIDI In/Out devices above",
                "OK"
            );
            return;
        }
        
        statusLabel.setText("Testing...", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, DS::Color::Accent::warning());
        
        // Send MIDI Identity Request: F0 7E 00 06 01 F7
        std::vector<uint8_t> identityRequest = {0xF0, 0x7E, 0x00, 0x06, 0x01, 0xF7};
        midi.sendSysEx(identityRequest);
        DBG("DeviceSettings: Sent MIDI Identity Request");
        
        // Give time for response, then check status
        juce::Timer::callAfterDelay(1000, [this]{
            statusLabel.setText("Request sent", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, DS::Color::Accent::success());
            DBG("DeviceSettings: Identity request sent - check for response in MIDI handler");
        });
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceSettingsPanel)
};
