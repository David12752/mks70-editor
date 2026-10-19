/*
  ==============================================================================

    ArpTab.cpp
    Component for Arpeggiator parameters - Using new ArpPanel design
    Modulation panel moved to separate tab

  ==============================================================================
*/

#include "ArpTab.h"
#include "PluginProcessor.h"

// Static members for clipboard
ArpParameters ArpTab::clipboard;
bool ArpTab::clipboardValid = false;

//==============================================================================
ArpTab::ArpTab()
{
    // Add arp panel directly (no tabs needed anymore)
    addAndMakeVisible(arpPanel);
    
    // Connect Tap Tempo button
    arpPanel.getTapTempoButton().onClick = [this]() { handleTapTempo(); };
    
    // Connect Copy/Paste buttons
    arpPanel.getCopyBtn().onClick = [this]() { copyToClipboard(); };
    arpPanel.getPasteBtn().onClick = [this]() { pasteFromClipboard(); };
    arpPanel.getRequestBtn().onClick = [this]() {
        if (processor != nullptr)
            processor->getMidiHandler().requestAllParameters();
        DBG("ArpTab: Requested current MKS-70 parameter state");
    };
    
    // === Connect arpeggiator controls to send NRPN MIDI ===
    // NRPN 2/16: BPM (40-300)
    arpPanel.getBpmSlider().getSlider().onValueChange = [this]() {
        int value = static_cast<int>(arpPanel.getBpmSlider().getValue());
        parameters.tempo = value;
        if (processor != nullptr)
            processor->setArpParameter(16, value);  // NRPN LSB 16
    };
    
    // NRPN 2/17: Mode (0-4)
    arpPanel.getModeCombo().getComboBox().onChange = [this]() {
        int value = arpPanel.getModeCombo().getSelectedId() - 1;
        parameters.mode = value;
        if (processor != nullptr)
            processor->setArpParameter(17, value);  // NRPN LSB 17
    };
    
    // NRPN 2/18: Octave (1-4)
    arpPanel.getOctaveCombo().getComboBox().onChange = [this]() {
        int value = arpPanel.getOctaveCombo().getSelectedId();
        parameters.octaveSpan = value;
        if (processor != nullptr)
            processor->setArpParameter(18, value);  // NRPN LSB 18
    };
    
    // NRPN 2/19: Rate (0-2: 1/4, 1/8, 1/16)
    arpPanel.getRateCombo().getComboBox().onChange = [this]() {
        int value = arpPanel.getRateCombo().getSelectedId() - 1;
        parameters.playRate = value;
        if (processor != nullptr)
            processor->setArpParameter(19, value);  // NRPN LSB 19
    };
    
    // NRPN 2/20: Latch (0-1)
    arpPanel.getLatchButton().onClick = [this]() {
        int value = arpPanel.getLatchButton().getToggleState() ? 1 : 0;
        parameters.latch = value;
        if (processor != nullptr)
            processor->setArpParameter(20, value);  // NRPN LSB 20
    };
    
    // NRPN 2/15: Arp On/Off (0-1)
    arpPanel.getArpOnButton().onClick = [this]() {
        int value = arpPanel.getArpOnButton().getToggleState() ? 1 : 0;
        parameters.arpOn = value;
        if (processor != nullptr)
            processor->setArpParameter(15, value);  // NRPN LSB 15
    };

    arpPanel.getSyncCombo().getComboBox().onChange = [this]() {
        parameters.sync = arpPanel.getSyncCombo().getSelectedId() - 1;
    };

    arpPanel.getGateSlider().getSlider().onValueChange = [this]() {
        parameters.duration = static_cast<int>(arpPanel.getGateSlider().getValue() / 10);
    };

    arpPanel.getVelocitySlider().getSlider().onValueChange = [this]() {
        parameters.fixedVelocity = static_cast<int>(arpPanel.getVelocitySlider().getValue());
    };
    
    // Send All Arp Settings to Synth
    arpPanel.getSendBtn().onClick = [this]() {
        if (processor == nullptr) return;
        syncParametersFromPanel();
        // Send all arp parameters via NRPN
        processor->setArpParameter(15, parameters.arpOn);
        processor->setArpParameter(16, parameters.tempo);
        processor->setArpParameter(17, parameters.mode);
        processor->setArpParameter(18, parameters.octaveSpan);
        processor->setArpParameter(19, parameters.playRate);
        processor->setArpParameter(20, parameters.latch);
        DBG("ArpTab: Sent all arp parameters to synth");
    };
}

ArpTab::~ArpTab()
{
    // Nothing to clean up
}

void ArpTab::paint(juce::Graphics& g)
{
    g.fillAll(DS::Color::bg());
}

void ArpTab::resized()
{
    // Arp panel fills the entire tab area
    arpPanel.setBounds(getLocalBounds());
}

void ArpTab::syncPanelToParameters()
{
    // Core Vecoven arp parameters (NRPN 2/16-20)
    arpPanel.getArpOnButton().setToggleState(parameters.arpOn != 0, juce::dontSendNotification);
    arpPanel.getBpmSlider().setValue(parameters.tempo, juce::dontSendNotification);
    arpPanel.getModeCombo().setSelectedId(parameters.mode + 1, juce::dontSendNotification);
    arpPanel.getOctaveCombo().setSelectedId(parameters.octaveSpan, juce::dontSendNotification);
    arpPanel.getRateCombo().setSelectedId(parameters.playRate + 1, juce::dontSendNotification);
    arpPanel.getLatchButton().setToggleState(parameters.latch != 0, juce::dontSendNotification);
    
    // Software-side features
    arpPanel.getSyncCombo().setSelectedId(parameters.sync + 1, juce::dontSendNotification);
    arpPanel.getGateSlider().setValue(parameters.duration * 10, juce::dontSendNotification);  // Map to gate %
    arpPanel.getVelocitySlider().setValue(parameters.fixedVelocity, juce::dontSendNotification);
}

void ArpTab::syncParametersFromPanel()
{
    // Core Vecoven arp parameters (NRPN 2/16-20)
    parameters.arpOn = arpPanel.getArpOnButton().getToggleState() ? 1 : 0;
    parameters.tempo = static_cast<int>(arpPanel.getBpmSlider().getValue());
    parameters.mode = arpPanel.getModeCombo().getSelectedId() - 1;
    parameters.octaveSpan = arpPanel.getOctaveCombo().getSelectedId();
    parameters.playRate = arpPanel.getRateCombo().getSelectedId() - 1;
    parameters.latch = arpPanel.getLatchButton().getToggleState() ? 1 : 0;
    
    // Software-side features
    parameters.sync = arpPanel.getSyncCombo().getSelectedId() - 1;
    parameters.duration = static_cast<int>(arpPanel.getGateSlider().getValue() / 10);  // Map from gate %
    parameters.fixedVelocity = static_cast<int>(arpPanel.getVelocitySlider().getValue());
}

void ArpTab::setParameters(const ArpParameters& params)
{
    parameters = params;
    syncPanelToParameters();
    repaint();
}

void ArpTab::refreshFromProcessor()
{
    if (processor != nullptr)
    {
        setParameters(processor->getArpParameters());
    }
}

void ArpTab::handleTapTempo()
{
    double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    
    // Clear old taps if timeout exceeded
    if (!tapTimes.empty() && (now - tapTimes.back()) > tapTimeout)
        tapTimes.clear();
    
    tapTimes.push_back(now);
    
    // Keep only last N taps
    while (tapTimes.size() > maxTaps)
        tapTimes.erase(tapTimes.begin());
    
    // Calculate BPM if we have at least 2 taps
    if (tapTimes.size() >= 2)
    {
        double totalInterval = tapTimes.back() - tapTimes.front();
        double avgInterval = totalInterval / (tapTimes.size() - 1);
        int bpm = static_cast<int>(60.0 / avgInterval);
        
        // Clamp to valid range
        bpm = juce::jlimit(30, 300, bpm);
        
        arpPanel.getBpmSlider().setValue(bpm, juce::sendNotificationSync);
        parameters.tempo = bpm;
        
        DBG("Tap Tempo: " + juce::String(bpm) + " BPM (from " + juce::String(tapTimes.size()) + " taps)");
    }
}

void ArpTab::copyToClipboard()
{
    syncParametersFromPanel();
    clipboard = parameters;
    clipboardValid = true;
    DBG("ArpTab: Copied settings to clipboard");
}

void ArpTab::pasteFromClipboard()
{
    if (clipboardValid)
    {
        setParameters(clipboard);
        DBG("ArpTab: Pasted settings from clipboard");
    }
    else
    {
        DBG("ArpTab: Clipboard is empty");
    }
}
