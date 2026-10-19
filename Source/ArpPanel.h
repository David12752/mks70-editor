/*
  ==============================================================================
    ArpPanel.h - MKS-70 Arpeggiator Panel (Vecoven Firmware)
    
    Clean, professional layout inspired by Arturia/NI plugins.
    Parameters mapped to actual MKS-70 Vecoven arpeggiator capabilities.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "GlowingSection.h"
#include "StyledSlider.h"
#include "StyledComboBox.h"

class ArpPanel : public juce::Component {
public:
    ArpPanel() {
        // Style helpers
        auto styleToggleButton = [](juce::TextButton& btn, juce::Colour accent, const juce::String& tooltip = "") {
            btn.setClickingTogglesState(true);
            btn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
            btn.setColour(juce::TextButton::buttonOnColourId, accent);
            btn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
            btn.setColour(juce::TextButton::textColourOnId, DS::Color::textPrimary());
            if (tooltip.isNotEmpty()) btn.setTooltip(tooltip);
        };
        
        auto styleActionButton = [](juce::TextButton& btn, juce::Colour accent, const juce::String& tooltip = "") {
            btn.setColour(juce::TextButton::buttonColourId, accent.withAlpha(0.2f));
            btn.setColour(juce::TextButton::textColourOffId, accent);
            if (tooltip.isNotEmpty()) btn.setTooltip(tooltip);
        };
        
        // === MAIN ARP ON/OFF (Large, prominent) ===
        arpOnButton.setButtonText("ARPEGGIATOR");
        styleToggleButton(arpOnButton, DS::Color::Accent::success(), "Enable/disable arpeggiator (Vecoven feature)");
        addAndMakeVisible(arpOnButton);
        
        // Action buttons
        sendBtn.setButtonText("Send to MKS");
        styleActionButton(sendBtn, DS::Color::Accent::success(), "Send all arp settings to MKS-70");
        addAndMakeVisible(sendBtn);
        
        requestBtn.setButtonText("Request");
        styleActionButton(requestBtn, DS::Color::Accent::cyan(), "Request arp settings from MKS-70");
        addAndMakeVisible(requestBtn);
        
        copyBtn.setButtonText("Copy");
        styleActionButton(copyBtn, DS::Color::Accent::magenta(), "Copy arp settings to clipboard");
        addAndMakeVisible(copyBtn);
        
        pasteBtn.setButtonText("Paste");
        styleActionButton(pasteBtn, DS::Color::Accent::magenta(), "Paste arp settings from clipboard");
        addAndMakeVisible(pasteBtn);
        
        // === MAIN SECTION: Core Arp Parameters (2 rows) ===
        mainSection = std::make_unique<GlowingSection>("ARPEGGIATOR SETTINGS", 
            DS::Color::Accent::primary());
        addAndMakeVisible(mainSection.get());
        
        // Row 1: BPM with big knob, Rate, Mode
        bpmSlider = std::make_unique<StyledSlider>("BPM", StyledSlider::Knob, 
            DS::Color::Accent::cyan());
        bpmSlider->setRange(40, 300, 1);  // Vecoven range: 40-300
        bpmSlider->setValue(120);
        bpmSlider->setTooltip("Tempo in BPM (40-300) - NRPN 2/16");
        addAndMakeVisible(bpmSlider.get());
        
        tapTempoButton.setButtonText("TAP TEMPO");
        tapTempoButton.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        tapTempoButton.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::cyan());
        tapTempoButton.setTooltip("Tap repeatedly to set tempo");
        addAndMakeVisible(tapTempoButton);
        
        modeCombo = std::make_unique<StyledComboBox>("Mode");
        modeCombo->addItems({"Up", "Down", "Up-Down", "Play", "Random"});  // Vecoven modes
        modeCombo->setTooltip("Arpeggio pattern direction - NRPN 2/17");
        addAndMakeVisible(modeCombo.get());
        
        octaveCombo = std::make_unique<StyledComboBox>("Octaves");
        octaveCombo->addItems({"1 Oct", "2 Oct", "3 Oct", "4 Oct"});  // 1-4 octaves
        octaveCombo->setTooltip("Number of octaves to span (1-4) - NRPN 2/18");
        addAndMakeVisible(octaveCombo.get());
        
        rateCombo = std::make_unique<StyledComboBox>("Rate");
        rateCombo->addItems({"1/4", "1/8", "1/16"});  // Vecoven rates
        rateCombo->setSelectedId(2);  // Default 1/8
        rateCombo->setTooltip("Note rate division - NRPN 2/19");
        addAndMakeVisible(rateCombo.get());
        
        // Latch button (prominent)
        latchButton.setButtonText("LATCH");
        styleToggleButton(latchButton, DS::Color::Accent::warning(), 
            "Hold arp running after keys released - NRPN 2/20");
        addAndMakeVisible(latchButton);
        
        // === EXTRA SECTION: Additional software features ===
        extraSection = std::make_unique<GlowingSection>("PERFORMANCE", 
            DS::Color::textSecondary());
        addAndMakeVisible(extraSection.get());
        
        // These are software-side features (not sent to synth)
        syncCombo = std::make_unique<StyledComboBox>("Sync");
        syncCombo->addItems({"Internal", "MIDI Clock"});
        syncCombo->setTooltip("Sync source (software feature)");
        addAndMakeVisible(syncCombo.get());
        
        gateSlider = std::make_unique<StyledSlider>("Gate", StyledSlider::Knob,
            DS::Color::Accent::primary());
        gateSlider->setRange(1, 100, 1);
        gateSlider->setValue(80);
        gateSlider->setTooltip("Gate length % (software feature - affects MIDI output)");
        addAndMakeVisible(gateSlider.get());
        
        velocitySlider = std::make_unique<StyledSlider>("Velocity", StyledSlider::Knob,
            DS::Color::Accent::primary());
        velocitySlider->setRange(1, 127, 1);
        velocitySlider->setValue(100);
        velocitySlider->setTooltip("Fixed velocity for arp notes (software feature)");
        addAndMakeVisible(velocitySlider.get());
        
        // Help label
        helpLabel.setText("Vecoven firmware arpeggiator. BPM/Mode/Octave/Rate/Latch are sent to synth.", 
                          juce::dontSendNotification);
        helpLabel.setFont(DS::Font::small());
        helpLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        helpLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(helpLabel);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(DS::Space::M);
        
        const int gap = DS::Space::M;
        const int smallGap = DS::Space::S;
        
        // === TOP ROW: ARP ON/OFF + Action Buttons ===
        auto topRow = bounds.removeFromTop(36);
        arpOnButton.setBounds(topRow.removeFromLeft(160).reduced(0, 2));
        topRow.removeFromLeft(gap * 2);
        
        sendBtn.setBounds(topRow.removeFromLeft(100).reduced(0, 4));
        topRow.removeFromLeft(smallGap);
        requestBtn.setBounds(topRow.removeFromLeft(80).reduced(0, 4));
        topRow.removeFromLeft(gap);
        copyBtn.setBounds(topRow.removeFromLeft(60).reduced(0, 4));
        topRow.removeFromLeft(smallGap);
        pasteBtn.setBounds(topRow.removeFromLeft(60).reduced(0, 4));
        
        bounds.removeFromTop(gap);
        
        // === HELP LABEL at bottom ===
        helpLabel.setBounds(bounds.removeFromBottom(20));
        bounds.removeFromBottom(smallGap);
        
        // === MAIN CONTENT: Two sections side by side ===
        // Left: Main Arp (larger), Right: Performance (smaller)
        const int rightWidth = 200;
        auto rightArea = bounds.removeFromRight(rightWidth);
        bounds.removeFromRight(gap);
        
        // === LEFT: Main Arp Section ===
        mainSection->setBounds(bounds);
        auto mainContent = mainSection->getContentBounds();
        
        // Large BPM knob on left
        const int bigKnobSize = 100;
        auto bpmArea = mainContent.removeFromLeft(bigKnobSize + 20);
        bpmSlider->setBounds(bpmArea.removeFromTop(bigKnobSize).withSizeKeepingCentre(bigKnobSize, bigKnobSize));
        bpmArea.removeFromTop(4);
        tapTempoButton.setBounds(bpmArea.removeFromTop(26).withSizeKeepingCentre(90, 26));
        
        mainContent.removeFromLeft(gap);
        
        // Controls grid: 2 columns x 2 rows
        const int controlWidth = (mainContent.getWidth() - gap) / 2;
        const int controlH = 50;
        
        // Row 1: Mode, Octaves
        auto row1 = mainContent.removeFromTop(controlH);
        modeCombo->setBounds(row1.removeFromLeft(controlWidth).reduced(2));
        row1.removeFromLeft(gap);
        octaveCombo->setBounds(row1.reduced(2));
        
        mainContent.removeFromTop(smallGap);
        
        // Row 2: Rate, Latch
        auto row2 = mainContent.removeFromTop(controlH);
        rateCombo->setBounds(row2.removeFromLeft(controlWidth).reduced(2));
        row2.removeFromLeft(gap);
        latchButton.setBounds(row2.reduced(2, 8));
        
        // === RIGHT: Performance Section ===
        extraSection->setBounds(rightArea);
        auto extraContent = extraSection->getContentBounds();
        
        syncCombo->setBounds(extraContent.removeFromTop(40).reduced(4));
        extraContent.removeFromTop(smallGap);
        
        // Two knobs side by side
        auto knobRow = extraContent.removeFromTop(80);
        int knobW = (knobRow.getWidth() - smallGap) / 2;
        gateSlider->setBounds(knobRow.removeFromLeft(knobW));
        knobRow.removeFromLeft(smallGap);
        velocitySlider->setBounds(knobRow);
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DS::Color::bg());
    }
    
    // Getters for external connections
    juce::TextButton& getArpOnButton() { return arpOnButton; }
    StyledSlider& getBpmSlider() { return *bpmSlider; }
    juce::TextButton& getTapTempoButton() { return tapTempoButton; }
    StyledComboBox& getModeCombo() { return *modeCombo; }
    StyledComboBox& getOctaveCombo() { return *octaveCombo; }
    StyledComboBox& getRateCombo() { return *rateCombo; }
    juce::TextButton& getLatchButton() { return latchButton; }
    StyledComboBox& getSyncCombo() { return *syncCombo; }
    StyledSlider& getGateSlider() { return *gateSlider; }
    StyledSlider& getVelocitySlider() { return *velocitySlider; }
    juce::TextButton& getSendBtn() { return sendBtn; }
    juce::TextButton& getRequestBtn() { return requestBtn; }
    juce::TextButton& getCopyBtn() { return copyBtn; }
    juce::TextButton& getPasteBtn() { return pasteBtn; }
    
    // Compatibility getters (map old names to new)
    StyledSlider& getDurationSlider() { return *gateSlider; }
    StyledSlider& getSwingSlider() { return *velocitySlider; }  // Repurposed
    juce::TextButton& getLegatoBtn() { return latchButton; }    // Map to latch
    
private:
    std::unique_ptr<GlowingSection> mainSection, extraSection;
    
    // Main toggle
    juce::TextButton arpOnButton;
    
    // Core arp parameters (sent to synth via NRPN)
    std::unique_ptr<StyledSlider> bpmSlider;
    juce::TextButton tapTempoButton;
    std::unique_ptr<StyledComboBox> modeCombo;
    std::unique_ptr<StyledComboBox> octaveCombo;
    std::unique_ptr<StyledComboBox> rateCombo;
    juce::TextButton latchButton;
    
    // Software-side features
    std::unique_ptr<StyledComboBox> syncCombo;
    std::unique_ptr<StyledSlider> gateSlider;
    std::unique_ptr<StyledSlider> velocitySlider;
    
    // Help
    juce::Label helpLabel;
    
    // Action buttons
    juce::TextButton sendBtn, requestBtn, copyBtn, pasteBtn;
};
