#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "DCOSection.h"
#include "Juno60Fader.h"
#include "VCFSection.h"
#include "VCASection.h"
#include "EnvelopeSection.h"
#include "LFOSection.h"
#include "LFO2Section.h"
#include "MixerSection.h"
#include "ChorusSection.h"
#include "DynamicOscilloscope.h"
#include "ToneParams.h"
#include "MKS70_Mapping.h"
#include "MKS70_Mapping.h"

class MKS70EditorAudioProcessorEditor;

class TonePanel : public juce::Component {
public:
    explicit TonePanel(bool isUpper = true) : upperTone(isUpper) {
        // Create all sections
        dco1 = std::make_unique<DCOSection>(1, false);
        dco2 = std::make_unique<DCOSection>(2, true);
        mixer = std::make_unique<MixerSection>();
        vcf = std::make_unique<VCFSection>();
        vca = std::make_unique<VCASection>();
        env1 = std::make_unique<EnvelopeSection>(1);
        env2 = std::make_unique<EnvelopeSection>(2);
        lfo = std::make_unique<LFOSection>();
        lfo2 = std::make_unique<LFO2Section>();
        chorus = std::make_unique<ChorusSection>();
        oscilloscope = std::make_unique<DynamicOscilloscope>();

        addAndMakeVisible(dco1.get());
        addAndMakeVisible(dco2.get());
        addAndMakeVisible(mixer.get());
        addAndMakeVisible(vcf.get());
        addAndMakeVisible(vca.get());
        addAndMakeVisible(env1.get());
        addAndMakeVisible(env2.get());
        addAndMakeVisible(lfo.get());
        addAndMakeVisible(lfo2.get());
        addAndMakeVisible(chorus.get());
        addAndMakeVisible(oscilloscope.get());

        // Tools button (small icon to open menu)
        toolsBtn.setButtonText("[M]");
        toolsBtn.setTooltip("Tools Menu (Init, Random, Copy...)");
        toolsBtn.onClick = [this]() { showToolsMenu(); };
        addAndMakeVisible(toolsBtn);
        
        // === BOTTOM TOOLBAR ===
        setupBottomToolbar();
        
        // Connect sections to oscilloscope for real-time visualization
        connectToOscilloscope();
    }
    
    void setupBottomToolbar() {
        // Quick action buttons for the bottom toolbar
        initBtn.setButtonText("Init");
        initBtn.setTooltip("Initialize tone to factory defaults");
        initBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        initBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        initBtn.onClick = [this]() { initAllParameters(); };
        addAndMakeVisible(initBtn);
        
        copyBtn.setButtonText(upperTone ? "Copy>Lower" : "Copy>Upper");
        copyBtn.setTooltip(upperTone ? "Copy this tone to Lower" : "Copy this tone to Upper");
        copyBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        copyBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::cyan());
        copyBtn.onClick = [this]() { if (onCopyToOther) onCopyToOther(getToneParams()); };
        addAndMakeVisible(copyBtn);
        
        swapBtn.setButtonText("Swap");
        swapBtn.setTooltip("Swap Upper and Lower tones");
        swapBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        swapBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::magenta());
        swapBtn.onClick = [this]() { if (onSwapTones) onSwapTones(); };
        addAndMakeVisible(swapBtn);
        
        randBtn.setButtonText("Random");
        randBtn.setTooltip("Randomize all parameters");
        randBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        randBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::warning());
        randBtn.onClick = [this]() { randomizeAll(); };
        addAndMakeVisible(randBtn);
        
        sendBtn.setButtonText("Send");
        sendBtn.setTooltip("Send this tone to MKS-70");
        sendBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::primary().withAlpha(0.3f));
        sendBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::primary());
        sendBtn.onClick = [this]() { 
            if (onInitTone) onInitTone(getToneParams()); 
            DBG("Tone sent to synth");
        };
        addAndMakeVisible(sendBtn);
        
        // Tone indicator label
        toneLabel.setText(upperTone ? "UPPER TONE" : "LOWER TONE", juce::dontSendNotification);
        toneLabel.setFont(DS::Font::tiny().boldened());
        toneLabel.setColour(juce::Label::textColourId, 
            upperTone ? DS::Color::Accent::cyan() : DS::Color::Accent::magenta());
        toneLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(toneLabel);
        
        // Default to V3.x firmware
        setFirmwareVersion(FirmwareVersion::Vecoven3);
    }
    
    void setEditor(MKS70EditorAudioProcessorEditor* ed);  // Implementation at end of file
    
    // Set firmware version - shows/hides V4.x only controls
    void setFirmwareVersion(FirmwareVersion version) {
        currentFirmware = version;
        
        // Update DCO sections
        dco1->setFirmwareVersion(version);
        dco2->setFirmwareVersion(version);
        
        // LFO2 is V4.x only
        bool showLFO2 = (version == FirmwareVersion::Vecoven4 &&
                         MKS70Mapping::LFO2_WAVEFORM_SYSEX != MKS70Mapping::PARAM_NOT_AVAILABLE);
        lfo2->setVisible(showLFO2);
        
        // Update LFO section for V4.x features (sync)
        lfo->setFirmwareVersion(version);
        
        // Update VCF for V4.x features (LFO2 depth)
        vcf->setFirmwareVersion(version);
        
        resized();
        repaint();
    }
    
    FirmwareVersion getFirmwareVersion() const { return currentFirmware; }

    void resized() override {
        auto bounds = getLocalBounds().reduced(2);  // Minimal outer padding
        const int gap = 3;  // Slightly larger gap for better spacing
        
        // Tools button in top-right corner (small)
        toolsBtn.setBounds(bounds.getRight() - 22, bounds.getY(), 20, 20);
        
        // === BOTTOM TOOLBAR - 22px height ===
        const int toolbarHeight = 22;
        auto toolbarArea = bounds.removeFromBottom(toolbarHeight);
        bounds.removeFromBottom(2);
        
        // Layout toolbar buttons
        const int btnWidth = 52;
        const int btnHeight = 16;
        const int btnGap = 4;
        
        auto tbInner = toolbarArea.reduced(4, 2);
        
        // Left side - tone indicator
        toneLabel.setBounds(tbInner.removeFromLeft(80).withSizeKeepingCentre(78, 16));
        tbInner.removeFromLeft(btnGap);
        
        // Action buttons
        initBtn.setBounds(tbInner.removeFromLeft(btnWidth).withSizeKeepingCentre(btnWidth - 2, btnHeight));
        tbInner.removeFromLeft(btnGap);
        copyBtn.setBounds(tbInner.removeFromLeft(btnWidth + 14).withSizeKeepingCentre(btnWidth + 12, btnHeight));
        tbInner.removeFromLeft(btnGap);
        swapBtn.setBounds(tbInner.removeFromLeft(btnWidth - 4).withSizeKeepingCentre(btnWidth - 6, btnHeight));
        tbInner.removeFromLeft(btnGap);
        randBtn.setBounds(tbInner.removeFromLeft(btnWidth + 6).withSizeKeepingCentre(btnWidth + 4, btnHeight));
        
        // Right side - send button
        sendBtn.setBounds(tbInner.removeFromRight(btnWidth).withSizeKeepingCentre(btnWidth - 2, btnHeight));
        
        // === MAIN CONTENT LAYOUT ===
        const int totalWidth = bounds.getWidth();
        const int totalHeight = bounds.getHeight();
        
        // === TOP ROW: DCO1, DCO2, VCF - 40% height (for taller faders) ===
        const int topRowHeight = (int)(totalHeight * 0.40);
        auto topRow = bounds.removeFromTop(topRowHeight);
        
        // DCO1 and DCO2 (32% each), VCF (36%)
        const int dcoWidth = (int)(topRow.getWidth() * 0.32);
        
        dco1->setBounds(topRow.removeFromLeft(dcoWidth));
        topRow.removeFromLeft(gap);
        dco2->setBounds(topRow.removeFromLeft(dcoWidth));
        topRow.removeFromLeft(gap);
        vcf->setBounds(topRow);
        
        bounds.removeFromTop(gap);
        
        // === MIDDLE ROW: ENV1, Oscilloscope (CENTERED), ENV2 - 30% height (smaller knobs) ===
        const int midRowHeight = (int)(totalHeight * 0.30);
        auto midRow = bounds.removeFromTop(midRowHeight);
        
        // ENVs wider (30% each), Oscilloscope narrower and centered (40%)
        const int envWidth = (int)(midRow.getWidth() * 0.30);
        const int oscWidth = midRow.getWidth() - envWidth * 2 - gap * 2;
        
        env1->setBounds(midRow.removeFromLeft(envWidth));
        midRow.removeFromLeft(gap);
        oscilloscope->setBounds(midRow.removeFromLeft(oscWidth));
        midRow.removeFromLeft(gap);
        env2->setBounds(midRow);
        
        bounds.removeFromTop(gap);
        
        // === BOTTOM ROW: Adaptive layout based on LFO2 visibility ===
        auto bottomRow = bounds;
        bool showLFO2 = lfo2->isVisible();
        
        if (showLFO2) {
            // V4.x layout: LFO1, LFO2, VCA, Mixer, Chorus
            const int lfoWidth = (int)(bottomRow.getWidth() * 0.20);
            const int remainingWidth = bottomRow.getWidth() - lfoWidth * 2 - gap * 4;
            const int secondaryWidth = remainingWidth / 3;
            
            lfo->setBounds(bottomRow.removeFromLeft(lfoWidth));
            bottomRow.removeFromLeft(gap);
            lfo2->setBounds(bottomRow.removeFromLeft(lfoWidth));
            bottomRow.removeFromLeft(gap);
            vca->setBounds(bottomRow.removeFromLeft(secondaryWidth));
            bottomRow.removeFromLeft(gap);
            mixer->setBounds(bottomRow.removeFromLeft(secondaryWidth));
            bottomRow.removeFromLeft(gap);
            chorus->setBounds(bottomRow);
        } else {
            // V3.x/Original layout: LFO1 larger, VCA, Mixer, Chorus - better spacing
            const int lfoWidth = (int)(bottomRow.getWidth() * 0.28);
            const int remainingWidth = bottomRow.getWidth() - lfoWidth - gap * 3;
            const int secondaryWidth = remainingWidth / 3;
            
            lfo->setBounds(bottomRow.removeFromLeft(lfoWidth));
            bottomRow.removeFromLeft(gap);
            vca->setBounds(bottomRow.removeFromLeft(secondaryWidth));
            bottomRow.removeFromLeft(gap);
            mixer->setBounds(bottomRow.removeFromLeft(secondaryWidth));
            bottomRow.removeFromLeft(gap);
            chorus->setBounds(bottomRow);
        }
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(DS::Color::bg());
        
        // Draw bottom toolbar background
        auto toolbarArea = getLocalBounds().removeFromBottom(22).toFloat();
        
        // Toolbar gradient background
        juce::ColourGradient toolbarGrad(
            DS::Color::bgElevated(), toolbarArea.getTopLeft(),
            DS::Color::bgCard(), toolbarArea.getBottomLeft(), false
        );
        g.setGradientFill(toolbarGrad);
        g.fillRect(toolbarArea);
        
        // Bottom border of toolbar
        g.setColour(DS::Color::border().withAlpha(0.5f));
        g.fillRect(toolbarArea.getX(), toolbarArea.getBottom() - 1.0f, toolbarArea.getWidth(), 1.0f);
    }
    
    DynamicOscilloscope& getOscilloscope() { return *oscilloscope; }
    
    // Callback for init - will be set by parent to send MIDI
    std::function<void(const ToneParams&)> onInitTone;
    
    // Callback for copy to other tone
    std::function<void(const ToneParams&)> onCopyToOther;
    
    // Callback for swap tones
    std::function<void()> onSwapTones;
    
    // Is this the upper tone?
    bool isUpperTone() const { return upperTone; }
    
    // Get current tone parameters
    ToneParams getToneParams() const {
        ToneParams params;
        
        // DCO1
        params.dco1Range = static_cast<int>(dco1->getRangeKnob().getValue());
        params.dco1Waveform = static_cast<int>(dco1->getWaveKnob().getValue());
        params.dco1Tune = static_cast<int>(dco1->getTuneSlider().getValue());
        params.dco1LFOMod = static_cast<int>(dco1->getLFOModSlider().getValue());
        params.dco1ENVMod = static_cast<int>(dco1->getEnvModSlider().getValue());
        
        // DCO2
        params.dco2Range = static_cast<int>(dco2->getRangeKnob().getValue());
        params.dco2Waveform = static_cast<int>(dco2->getWaveKnob().getValue());
        params.dco2Tune = static_cast<int>(dco2->getTuneSlider().getValue());
        params.dco2LFOMod = static_cast<int>(dco2->getLFOModSlider().getValue());
        params.dco2ENVMod = static_cast<int>(dco2->getEnvModSlider().getValue());
        if (auto* fineSlider = dco2->getFineSlider())
            params.dco2FineTune = static_cast<int>(fineSlider->getValue());
        if (auto* crossModKnob = dco2->getCrossModKnob())
            params.dco2CrossMod = static_cast<int>(crossModKnob->getValue());
        
        // DCO common controls share the same hardware addresses
        params.dcoDynamics = dco1->getDynamicsSwitch().getValue();
        params.dcoENVMode = dco1->getEnvSrcSwitch().getValue();
        
        // VCF
        params.vcfCutoff = static_cast<int>(vcf->getCutoffSlider().getValue());
        params.vcfResonance = static_cast<int>(vcf->getResonanceSlider().getValue());
        params.vcfLFOMod = static_cast<int>(vcf->getLFOModSlider().getValue());
        params.vcfENVMod = static_cast<int>(vcf->getEnvModSlider().getValue());
        params.vcfKeyFollow = static_cast<int>(vcf->getKeyFollowSlider().getValue());
        params.vcfHPF = static_cast<int>(vcf->getHPFKnob().getValue());
        params.vcfDynamics = static_cast<int>(vcf->getDynamicsKnob().getValue());
        params.vcfENVMode = static_cast<int>(vcf->getEnvModeKnob().getValue());
        
        // ENV1
        params.env1Time1 = static_cast<int>(env1->getAttackSlider().getValue());
        params.env1Time2 = static_cast<int>(env1->getDecaySlider().getValue());
        params.env1Level3 = static_cast<int>(env1->getSustainSlider().getValue());
        params.env1Time4 = static_cast<int>(env1->getReleaseSlider().getValue());
        params.env1KeyFollow = juce::jlimit(0, 3, env1->getKeyFollowCombo().getSelectedId() - 1);
        
        // ENV2
        params.env2Time1 = static_cast<int>(env2->getAttackSlider().getValue());
        params.env2Time2 = static_cast<int>(env2->getDecaySlider().getValue());
        params.env2Level3 = static_cast<int>(env2->getSustainSlider().getValue());
        params.env2Time4 = static_cast<int>(env2->getReleaseSlider().getValue());
        params.env2KeyFollow = juce::jlimit(0, 3, env2->getKeyFollowCombo().getSelectedId() - 1);
        
        // LFO
        params.lfo1Rate = static_cast<int>(lfo->getRateSlider().getValue());
        params.lfo1Delay = static_cast<int>(lfo->getDelaySlider().getValue());
        params.lfo1Waveform = static_cast<int>(lfo->getWaveKnob().getValue());
        
        // LFO2
        params.lfo2Rate = static_cast<int>(lfo2->getRateSlider().getValue());
        params.lfo2Delay = static_cast<int>(lfo2->getDelaySlider().getValue());
        params.lfo2Waveform = static_cast<int>(lfo2->getWaveKnob().getValue());
        
        // Mixer
        params.mixerDCO1Level = static_cast<int>(mixer->getDCO1Level().getValue());
        params.mixerDCO2Level = static_cast<int>(mixer->getDCO2Level().getValue());
        params.mixerENVMod = static_cast<int>(mixer->getEnvDepth().getValue());
        params.mixerDynamics = juce::jlimit(0, 3, mixer->getDynamicsCombo().getSelectedId() - 1);
        params.mixerENVMode = juce::jlimit(0, 3, mixer->getEnvModeCombo().getSelectedId() - 1);
        
        // VCA
        params.vcaLevel = static_cast<int>(vca->getLevelSlider().getValue());
        params.vcaMode = static_cast<int>(vca->getEnvSourceKnob().getValue());
        params.vcaDynamics = static_cast<int>(vca->getDynamicsKnob().getValue());
        
        // Chorus
        params.chorusMode = static_cast<int>(chorus->getTypeSwitch().getValue());
        
        return params;
    }
    
    // Set tone parameters (for paste/copy operations)
    void setToneParams(const ToneParams& params) {
        // DCO1
        dco1->getRangeKnob().setValue(params.dco1Range, juce::dontSendNotification);
        dco1->getWaveKnob().setValue(params.dco1Waveform, juce::dontSendNotification);
        dco1->getTuneSlider().getSlider().setValue(params.dco1Tune, juce::dontSendNotification);
        dco1->getLFOModSlider().getSlider().setValue(params.dco1LFOMod, juce::dontSendNotification);
        dco1->getEnvModSlider().getSlider().setValue(params.dco1ENVMod, juce::dontSendNotification);
        dco1->getDynamicsSwitch().setValue(params.dcoDynamics);
        dco1->getEnvSrcSwitch().setValue(params.dcoENVMode);
        
        // DCO2
        dco2->getRangeKnob().setValue(params.dco2Range, juce::dontSendNotification);
        dco2->getWaveKnob().setValue(params.dco2Waveform, juce::dontSendNotification);
        dco2->getTuneSlider().getSlider().setValue(params.dco2Tune, juce::dontSendNotification);
        dco2->getLFOModSlider().getSlider().setValue(params.dco2LFOMod, juce::dontSendNotification);
        dco2->getEnvModSlider().getSlider().setValue(params.dco2ENVMod, juce::dontSendNotification);
        dco2->getDynamicsSwitch().setValue(params.dcoDynamics);
        dco2->getEnvSrcSwitch().setValue(params.dcoENVMode);
        if (auto* fineSlider = dco2->getFineSlider())
            fineSlider->setValue(params.dco2FineTune, juce::dontSendNotification);
        if (auto* crossModKnob = dco2->getCrossModKnob())
            crossModKnob->setValue(params.dco2CrossMod, juce::dontSendNotification);
        
        // VCF
        vcf->getCutoffSlider().setValue(params.vcfCutoff, juce::dontSendNotification);
        vcf->getResonanceSlider().setValue(params.vcfResonance, juce::dontSendNotification);
        vcf->getLFOModSlider().setValue(params.vcfLFOMod, juce::dontSendNotification);
        vcf->getEnvModSlider().setValue(params.vcfENVMod, juce::dontSendNotification);
        vcf->getKeyFollowSlider().setValue(params.vcfKeyFollow, juce::dontSendNotification);
        vcf->getHPFKnob().setValue(params.vcfHPF, juce::dontSendNotification);
        vcf->getDynamicsKnob().setValue(params.vcfDynamics, juce::dontSendNotification);
        vcf->getEnvModeKnob().setValue(params.vcfENVMode, juce::dontSendNotification);
        
        // ENV1
        env1->getAttackSlider().setValue(params.env1Time1, juce::dontSendNotification);
        env1->getDecaySlider().setValue(params.env1Time2, juce::dontSendNotification);
        env1->getSustainSlider().setValue(params.env1Level3, juce::dontSendNotification);
        env1->getReleaseSlider().setValue(params.env1Time4, juce::dontSendNotification);
        env1->getKeyFollowCombo().setSelectedId(params.env1KeyFollow + 1, juce::dontSendNotification);
        
        // ENV2
        env2->getAttackSlider().setValue(params.env2Time1, juce::dontSendNotification);
        env2->getDecaySlider().setValue(params.env2Time2, juce::dontSendNotification);
        env2->getSustainSlider().setValue(params.env2Level3, juce::dontSendNotification);
        env2->getReleaseSlider().setValue(params.env2Time4, juce::dontSendNotification);
        env2->getKeyFollowCombo().setSelectedId(params.env2KeyFollow + 1, juce::dontSendNotification);
        
        // LFO
        lfo->getRateSlider().setValue(params.lfo1Rate, juce::dontSendNotification);
        lfo->getDelaySlider().setValue(params.lfo1Delay, juce::dontSendNotification);
        lfo->getWaveKnob().setValue(params.lfo1Waveform, juce::dontSendNotification);
        lfo->updateVisualizerRate((float)params.lfo1Rate);
        lfo->updateVisualizerDelay((float)params.lfo1Delay);
        lfo->updateVisualizerWave(params.lfo1Waveform);
        
        // LFO2
        lfo2->getRateSlider().setValue(params.lfo2Rate, juce::dontSendNotification);
        lfo2->getDelaySlider().setValue(params.lfo2Delay, juce::dontSendNotification);
        lfo2->getWaveKnob().setValue(params.lfo2Waveform, juce::dontSendNotification);
        lfo2->updateVisualizerRate((float)params.lfo2Rate);
        lfo2->updateVisualizerDelay((float)params.lfo2Delay);
        lfo2->updateVisualizerWave(params.lfo2Waveform);
        
        // Mixer
        mixer->getDCO1Level().setValue(params.mixerDCO1Level, juce::dontSendNotification);
        mixer->getDCO2Level().setValue(params.mixerDCO2Level, juce::dontSendNotification);
        mixer->getEnvDepth().setValue(params.mixerENVMod, juce::dontSendNotification);
        mixer->getDynamicsCombo().setSelectedId(params.mixerDynamics + 1, juce::dontSendNotification);
        mixer->getEnvModeCombo().setSelectedId(params.mixerENVMode + 1, juce::dontSendNotification);
        
        // VCA
        vca->getLevelSlider().setValue(params.vcaLevel, juce::dontSendNotification);
        vca->getEnvSourceKnob().setValue(params.vcaMode, juce::dontSendNotification);
        vca->getDynamicsKnob().setValue(params.vcaDynamics, juce::dontSendNotification);
        
        // Chorus
        chorus->getTypeSwitch().setValue(params.chorusMode);
        
        DBG("Tone parameters applied from copy");
    }
    
    // Legacy getters for compatibility
    DCOSection& getDCO1Section() { return *dco1; }
    DCOSection& getDCO2Section() { return *dco2; }
    MixerSection& getMixerSection() { return *mixer; }
    VCFSection& getVCFSection() { return *vcf; }
    VCASection& getVCASection() { return *vca; }
    EnvelopeSection& getEnv1Section() { return *env1; }
    EnvelopeSection& getEnv2Section() { return *env2; }
    LFOSection& getLFOSection() { return *lfo; }
    LFO2Section& getLFO2Section() { return *lfo2; }
    ChorusSection& getChorusSection() { return *chorus; }
    
    // Alias for Total Recall refresh
    void refreshFromParams(const ToneParams& params) { setToneParams(params); }

private:
    MKS70EditorAudioProcessorEditor* editor = nullptr;  // For MIDI transmission
    bool upperTone;
    int currentTheme = 0;
    FirmwareVersion currentFirmware = FirmwareVersion::Vecoven3;  // Default to V3.x (most common)
    
    // Helper method to send MIDI parameters
    // Using std::function to avoid circular dependency issues
    std::function<void(int tone, int param, int value)> sendMidiParameter;

    std::unique_ptr<DCOSection> dco1, dco2;
    std::unique_ptr<MixerSection> mixer;
    std::unique_ptr<VCFSection> vcf;
    std::unique_ptr<VCASection> vca;
    std::unique_ptr<EnvelopeSection> env1, env2;
    std::unique_ptr<LFOSection> lfo;
    std::unique_ptr<LFO2Section> lfo2;
    std::unique_ptr<ChorusSection> chorus;
    std::unique_ptr<DynamicOscilloscope> oscilloscope;

    juce::TextButton toolsBtn;  // Small tools button instead of multiple buttons
    
    // Bottom toolbar buttons
    juce::TextButton initBtn, copyBtn, swapBtn, randBtn, sendBtn;
    juce::Label toneLabel;
    
    void showToolsMenu() {
        juce::PopupMenu menu;
        
        // === TONE OPERATIONS ===
        menu.addSectionHeader(upperTone ? "Upper Tone" : "Lower Tone");
        menu.addItem(1, "Initialize All");
        menu.addItem(2, upperTone ? "Copy to Lower" : "Copy to Upper");
        menu.addItem(3, "Swap Upper/Lower");
        
        menu.addSeparator();
        
        // === RANDOMIZE ===
        juce::PopupMenu randMenu;
        randMenu.addItem(10, "All Parameters");
        randMenu.addItem(11, "DCO Only");
        randMenu.addItem(12, "VCF Only");
        randMenu.addItem(13, "Envelopes Only");
        randMenu.addItem(14, "LFO Only");
        randMenu.addItem(15, "Mixer Only");
        randMenu.addSeparator();
        randMenu.addItem(20, "Gentle Random (+/- 20%)");
        menu.addSubMenu("Randomize", randMenu);
        
        menu.addSeparator();
        
        // === MIDI ===
        menu.addSectionHeader("MIDI");
        menu.addItem(30, "Send All to Synth");
        menu.addItem(31, "Request from Synth");
        
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&toolsBtn), 
            [this](int result) { handleMenuResult(result); });
    }
    
    void handleMenuResult(int result) {
        switch (result) {
            case 1: initAllParameters(); break;
            case 2: if (onCopyToOther) onCopyToOther(getToneParams()); break;
            case 3: if (onSwapTones) onSwapTones(); break;
            case 10: randomizeAll(); break;
            case 11: randomizeDCO(); break;
            case 12: randomizeVCF(); break;
            case 13: randomizeEnvelopes(); break;
            case 14: randomizeLFO(); break;
            case 15: randomizeMixer(); break;
            case 20: randomizeGentle(); break;
        }
    }
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isRightButtonDown()) {
            auto* clickedComponent = getComponentAt(e.getPosition());
            if (clickedComponent == this) {
                showToolsMenu();
            }
        }
    }
    
    void randomizeSlider(LabeledSlider& slider) {
        double min = slider.getSlider().getMinimum();
        double max = slider.getSlider().getMaximum();
        double val = min + juce::Random::getSystemRandom().nextDouble() * (max - min);
        slider.setValue(val, juce::sendNotification);
    }
    
    void randomizeSlider(juce::Slider& slider) {
        double min = slider.getMinimum();
        double max = slider.getMaximum();
        double val = min + juce::Random::getSystemRandom().nextDouble() * (max - min);
        slider.setValue(val, juce::sendNotification);
    }
    
    void randomizeSlider(Juno60Fader& fader) {
        randomizeSlider(fader.getSlider());
    }
    
    void randomizeSliderGentle(LabeledSlider& slider, double range = 0.2) {
        double min = slider.getSlider().getMinimum();
        double max = slider.getSlider().getMaximum();
        double current = slider.getValue();
        double delta = (max - min) * range;
        double offset = (juce::Random::getSystemRandom().nextDouble() - 0.5) * 2.0 * delta;
        double val = juce::jlimit(min, max, current + offset);
        slider.setValue(val, juce::sendNotification);
    }
    
    void randomizeSliderGentle(juce::Slider& slider, double range = 0.2) {
        double min = slider.getMinimum();
        double max = slider.getMaximum();
        double current = slider.getValue();
        double delta = (max - min) * range;
        double offset = (juce::Random::getSystemRandom().nextDouble() - 0.5) * 2.0 * delta;
        double val = juce::jlimit(min, max, current + offset);
        slider.setValue(val, juce::sendNotification);
    }
    
    void randomizeSliderGentle(Juno60Fader& fader, double range = 0.2) {
        randomizeSliderGentle(fader.getSlider(), range);
    }
    
    void randomizeDCO() {
        randomizeSlider(dco1->getTuneSlider());
        randomizeSlider(dco1->getLFOModSlider());
        randomizeSlider(dco1->getEnvModSlider());
        randomizeSlider(dco2->getTuneSlider());
        randomizeSlider(dco2->getLFOModSlider());
        randomizeSlider(dco2->getEnvModSlider());
    }
    
    void randomizeVCF() {
        randomizeSlider(vcf->getCutoffSlider());
        randomizeSlider(vcf->getResonanceSlider());
        randomizeSlider(vcf->getLFOModSlider());
        randomizeSlider(vcf->getEnvModSlider());
        randomizeSlider(vcf->getKeyFollowSlider());
    }
    
    void randomizeEnvelopes() {
        randomizeSlider(env1->getAttackSlider());
        randomizeSlider(env1->getDecaySlider());
        randomizeSlider(env1->getSustainSlider());
        randomizeSlider(env1->getReleaseSlider());
        randomizeSlider(env2->getAttackSlider());
        randomizeSlider(env2->getDecaySlider());
        randomizeSlider(env2->getSustainSlider());
        randomizeSlider(env2->getReleaseSlider());
    }
    
    void randomizeLFO() {
        randomizeSlider(lfo->getRateSlider());
        randomizeSlider(lfo->getDelaySlider());
    }
    
    void randomizeMixer() {
        randomizeSlider(mixer->getDCO1Level());
        randomizeSlider(mixer->getDCO2Level());
    }
    
    void randomizeAll() {
        randomizeDCO();
        randomizeVCF();
        randomizeEnvelopes();
        randomizeLFO();
        randomizeMixer();
    }
    
    void randomizeGentle() {
        randomizeSliderGentle(dco1->getTuneSlider());
        randomizeSliderGentle(dco1->getLFOModSlider());
        randomizeSliderGentle(dco1->getEnvModSlider());
        randomizeSliderGentle(dco2->getTuneSlider());
        randomizeSliderGentle(dco2->getLFOModSlider());
        randomizeSliderGentle(dco2->getEnvModSlider());
        randomizeSliderGentle(vcf->getCutoffSlider());
        randomizeSliderGentle(vcf->getResonanceSlider());
        randomizeSliderGentle(env1->getAttackSlider());
        randomizeSliderGentle(env1->getDecaySlider());
        randomizeSliderGentle(env1->getSustainSlider());
        randomizeSliderGentle(env1->getReleaseSlider());
    }
    
    void initAllParameters() {
        // === MKS-70 HARDWARE INIT VALUES ===
        // These are the actual factory default values from the MKS-70 hardware
        
        // DCO-1: Range=8', Wave=Saw, Tune=0 (center), Mods=0
        dco1->getTuneSlider().getSlider().setValue(0, juce::sendNotification);
        dco1->getLFOModSlider().getSlider().setValue(0, juce::sendNotification);
        dco1->getEnvModSlider().getSlider().setValue(0, juce::sendNotification);
        dco1->getRangeKnob().setValue(1, juce::dontSendNotification);  // 8' (index 1)
        dco1->getWaveKnob().setValue(1, juce::dontSendNotification);   // Saw (index 1)
        
        // DCO-2: Range=8', Wave=Saw, Tune=0, FineTune=0 (center), Mods=0
        dco2->getTuneSlider().getSlider().setValue(0, juce::sendNotification);
        dco2->getLFOModSlider().getSlider().setValue(0, juce::sendNotification);
        dco2->getEnvModSlider().getSlider().setValue(0, juce::sendNotification);
        dco2->getRangeKnob().setValue(1, juce::dontSendNotification);  // 8' (index 1)
        dco2->getWaveKnob().setValue(1, juce::dontSendNotification);   // Saw (index 1)
        
        // VCF: Cutoff=127 (full open), Resonance=0, Mods=0, KeyFollow=0
        vcf->getCutoffSlider().setValue(127, juce::sendNotification);
        vcf->getResonanceSlider().setValue(0, juce::sendNotification);
        vcf->getLFOModSlider().setValue(0, juce::sendNotification);
        vcf->getEnvModSlider().setValue(0, juce::sendNotification);
        vcf->getKeyFollowSlider().setValue(0, juce::sendNotification);
        vcf->getHPFKnob().setValue(0, juce::sendNotification);        // Off (index 0)
        vcf->getDynamicsKnob().setValue(0, juce::sendNotification);   // Off (index 0)
        vcf->getEnvModeKnob().setValue(0, juce::sendNotification);    // E1 (index 0)
        
        // ENV1 (Pitch): All times=0, Sustain=0 (MKS-70 init has flat envelope)
        env1->getAttackSlider().setValue(0, juce::sendNotification);
        env1->getDecaySlider().setValue(0, juce::sendNotification);
        env1->getSustainSlider().setValue(0, juce::sendNotification);
        env1->getReleaseSlider().setValue(0, juce::sendNotification);
        
        // ENV2 (Filter): A=0, D=0, S=127, R=0 (sustain at max for filter)
        env2->getAttackSlider().setValue(0, juce::sendNotification);
        env2->getDecaySlider().setValue(0, juce::sendNotification);
        env2->getSustainSlider().setValue(127, juce::sendNotification);
        env2->getReleaseSlider().setValue(0, juce::sendNotification);
        
        // LFO: Rate=0, Delay=0, Wave=Triangle
        lfo->getRateSlider().setValue(0, juce::sendNotification);
        lfo->getDelaySlider().setValue(0, juce::sendNotification);
        lfo->getWaveKnob().setValue(0, juce::dontSendNotification);  // Triangle (index 0)
        
        // LFO2 (Vecoven X3): Rate=0, Delay=0, Wave=Triangle, LFO1 Depth=0
        lfo2->getRateSlider().setValue(0, juce::sendNotification);
        lfo2->getDelaySlider().setValue(0, juce::sendNotification);
        lfo2->getWaveKnob().setValue(0, juce::dontSendNotification);  // Triangle (index 0)
        lfo2->getLFO1DepthSlider().setValue(0, juce::sendNotification);
        
        // Mixer: Both DCOs at max (127)
        mixer->getDCO1Level().setValue(127, juce::sendNotification);
        mixer->getDCO2Level().setValue(127, juce::sendNotification);
        
        // VCA: Level=127 (full), ENV mode
        vca->getLevelSlider().setValue(127, juce::sendNotification);
        vca->getEnvSourceKnob().setValue(1, juce::sendNotification);  // E2 (index 1)
        vca->getDynamicsKnob().setValue(0, juce::sendNotification);   // Off (index 0)
        
        // Chorus: Off
        chorus->getTypeSwitch().setValue(0);    // Off (index 0)
        chorus->getRateSlider().setValue(0, juce::sendNotification);
        
        // Send to hardware if callback set
        if (onInitTone) {
            ToneParams initTone;
            initTone.reset();  // Use ToneParams defaults
            onInitTone(initTone);
        }
        
        DBG("Tone initialized to MKS-70 hardware defaults");
    }
    
    void connectToOscilloscope() {
        // DCO1 - switch to Oscillator view when touched
        dco1->getWaveKnob().onChange = [this]() {
            int value = static_cast<int>(dco1->getWaveKnob().getValue());
            oscilloscope->setDCO1Wave(value);
            oscilloscope->focusOnCategory("DCO");
            
            // Send raw UI value; the processor maps it to the MKS-70 SysEx value.
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::DCO1_WAVEFORM_SYSEX, value);
            }
        };
        dco2->getWaveKnob().onChange = [this]() {
            int value = static_cast<int>(dco2->getWaveKnob().getValue());
            oscilloscope->setDCO2Wave(value);
            oscilloscope->focusOnCategory("DCO");
            
            // Send raw UI value; the processor maps it to the MKS-70 SysEx value.
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::DCO2_WAVEFORM_SYSEX, value);
            }
        };
        
        dco1->getTuneSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco1->getTuneSlider().getValue();
            oscilloscope->setDCO1Tune(value);
            oscilloscope->focusOnCategory("DCO");
            
            // Send MIDI to synth
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::DCO1_TUNE_SYSEX, (int)value);
            }
        };
        dco2->getTuneSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco2->getTuneSlider().getValue();
            oscilloscope->setDCO2Tune(value);
            oscilloscope->focusOnCategory("DCO");
            
            // Send MIDI to synth
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::DCO2_TUNE_SYSEX, (int)value);
            }
        };
        
        // DCO Range knobs
        dco1->getRangeKnob().onChange = [this]() {
            int value = static_cast<int>(dco1->getRangeKnob().getValue());
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::DCO1_RANGE_SYSEX, value);
            }
        };
        dco2->getRangeKnob().onChange = [this]() {
            int value = static_cast<int>(dco2->getRangeKnob().getValue());
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::DCO2_RANGE_SYSEX, value);
            }
        };
        
        // DCO LFO Mod sliders
        dco1->getLFOModSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco1->getLFOModSlider().getValue();
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::DCO1_LFO_DEPTH_SYSEX, (int)value);
            }
        };
        dco2->getLFOModSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco2->getLFOModSlider().getValue();
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::DCO2_LFO_DEPTH_SYSEX, (int)value);
            }
        };
        
        // DCO Env Mod sliders
        dco1->getEnvModSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco1->getEnvModSlider().getValue();
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::DCO1_ENV_DEPTH_SYSEX, (int)value);
            }
        };
        dco2->getEnvModSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco2->getEnvModSlider().getValue();
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::DCO2_ENV_DEPTH_SYSEX, (int)value);
            }
        };
        
        // DCO Dynamics switches
        dco1->getDynamicsSwitch().onChange = [this](int value) {
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::DCO1_DYNAMICS_SYSEX, value);
            }
        };
        dco2->getDynamicsSwitch().onChange = [this](int value) {
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::DCO2_DYNAMICS_SYSEX, value);
            }
        };
        
        // DCO Env Source switches
        dco1->getEnvSrcSwitch().onChange = [this](int value) {
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::DCO1_ENV_SOURCE_SYSEX, value);
            }
        };
        dco2->getEnvSrcSwitch().onChange = [this](int value) {
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::DCO2_ENV_SOURCE_SYSEX, value);
            }
        };
        
        // DCO2 Fine Tune (only exists on DCO2)
        if (auto* fineSlider = dco2->getFineSlider()) {
            fineSlider->getSlider().onValueChange = [this, fineSlider]() {
                float value = (float)fineSlider->getValue();
                if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::DCO2_FINETUNE_SYSEX, (int)value);
                }
            };
        }
        
        // DCO2 Cross Mod (only exists on DCO2)
        if (auto* crossModKnob = dco2->getCrossModKnob()) {
            crossModKnob->onChange = [this, crossModKnob]() {
                int value = static_cast<int>(crossModKnob->getValue());
                if (sendMidiParameter) { 
                    int tone = upperTone ? 1 : 2; 
                    sendMidiParameter(tone, MKS70Mapping::DCO_CROSSMOD_SYSEX, value);
                }
            };
        }
        
        // === PWM Controls (Vecoven V4.x) ===
        
        // DCO1 PWM Width
        dco1->getPWMWidthSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco1->getPWMWidthSlider().getValue();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::PWM_DCO1_WIDTH_SYSEX, (int)value);
            }
        };
        
        // DCO1 PWM LFO Depth
        dco1->getPWMLFOSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco1->getPWMLFOSlider().getValue();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::PWM_DCO1_LFO_DEPTH_SYSEX, (int)value);
            }
        };
        
        // DCO1 PWM Envelope Depth
        dco1->getPWMEnvSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco1->getPWMEnvSlider().getValue();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::PWM_DCO1_ENV_DEPTH_SYSEX, (int)value);
            }
        };
        
        // DCO2 PWM Width
        dco2->getPWMWidthSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco2->getPWMWidthSlider().getValue();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::PWM_DCO2_WIDTH_SYSEX, (int)value);
            }
        };
        
        // DCO2 PWM LFO Depth
        dco2->getPWMLFOSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco2->getPWMLFOSlider().getValue();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::PWM_DCO2_LFO_DEPTH_SYSEX, (int)value);
            }
        };
        
        // DCO2 PWM Envelope Depth
        dco2->getPWMEnvSlider().getSlider().onValueChange = [this]() {
            float value = (float)dco2->getPWMEnvSlider().getValue();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::PWM_DCO2_ENV_DEPTH_SYSEX, (int)value);
            }
        };
        
        // VCF LFO Mod
        vcf->getLFOModSlider().getSlider().onValueChange = [this]() {
            float value = (float)vcf->getLFOModSlider().getValue();
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::VCF_LFO1_DEPTH_SYSEX, (int)value);
            }
        };
        
        // VCF LFO2 Mod (Vecoven X3)
        vcf->getLFO2ModSlider().getSlider().onValueChange = [this]() {
            float value = (float)vcf->getLFO2ModSlider().getValue();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::VCF_LFO2_DEPTH_SYSEX, (int)value);
            }
        };
        
        // VCF Key Follow
        vcf->getKeyFollowSlider().getSlider().onValueChange = [this]() {
            float value = (float)vcf->getKeyFollowSlider().getValue();
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::VCF_KEY_FOLLOW_SYSEX, (int)value);
            }
        };
        
        // VCF HPF Knob
        vcf->getHPFKnob().onChange = [this]() {
            int value = static_cast<int>(vcf->getHPFKnob().getValue());
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::HPF_CUTOFF_SYSEX, value);
            }
        };
        
        // VCF Dynamics Knob
        vcf->getDynamicsKnob().onChange = [this]() {
            int value = static_cast<int>(vcf->getDynamicsKnob().getValue());
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::VCF_DYNAMICS_SYSEX, value);
            }
        };
        
        // VCF Env Mode Knob
        vcf->getEnvModeKnob().onChange = [this]() {
            int value = static_cast<int>(vcf->getEnvModeKnob().getValue());
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::VCF_ENV_SOURCE_SYSEX, value);
            }
        };
        
        // VCA Level (already has callback for oscilloscope, keep existing)
        // VCA Env Source Knob
        vca->getEnvSourceKnob().onChange = [this]() {
            int value = static_cast<int>(vca->getEnvSourceKnob().getValue());
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::VCA_ENV_SOURCE_SYSEX, value);
            }
        };
        
        // VCA Dynamics Knob
        vca->getDynamicsKnob().onChange = [this]() {
            int value = static_cast<int>(vca->getDynamicsKnob().getValue());
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::VCA_DYNAMICS_SYSEX, value);
            }
        };
        
        // LFO Delay - also update visualizer
        lfo->getDelaySlider().getSlider().onValueChange = [this]() {
            float value = (float)lfo->getDelaySlider().getValue();
            // Update visualizer in LFOSection
            lfo->updateVisualizerDelay(value);
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::LFO_DELAY, (int)value);
            }
        };
        
        // LFO Wave Knob - also update visualizer
        lfo->getWaveKnob().onChange = [this]() {
            int value = static_cast<int>(lfo->getWaveKnob().getValue());
            // Update visualizer in LFOSection
            lfo->updateVisualizerWave(value);
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::V3::LFO_WAVEFORM, value);
            }
        };
        
        // LFO1 Sync Switch (Vecoven 4.x - may not work on V3.x)
        lfo->getSyncSwitch().onChange = [this](int value) {
            // Note: LFO Sync is Vecoven 4.x only feature - silently ignore on V3.x
            DBG("LFO Sync is V4.x only feature - value: " + juce::String(value));
        };
        
        // Mixer - switch to Audio Out view
        mixer->getDCO1Level().getSlider().onValueChange = [this]() {
            float value = (float)mixer->getDCO1Level().getValue();
            oscilloscope->setDCO1Level(value);
            oscilloscope->focusOnCategory("MIX");
            
            // Send MIDI to synth
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::MIXER_DCO1_LEVEL_SYSEX, (int)value);
            }
        };
        mixer->getDCO2Level().getSlider().onValueChange = [this]() {
            float value = (float)mixer->getDCO2Level().getValue();
            oscilloscope->setDCO2Level(value);
            oscilloscope->focusOnCategory("MIX");
            
            // Send MIDI to synth
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::MIXER_DCO2_LEVEL_SYSEX, (int)value);
            }
        };
        
        // Mixer Envelope Depth
        mixer->getEnvDepth().getSlider().onValueChange = [this]() {
            float value = (float)mixer->getEnvDepth().getValue();
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::MIXER_ENV_DEPTH_SYSEX, (int)value);
            }
        };
        
        // Mixer Dynamics Combo
        mixer->getDynamicsCombo().getComboBox().onChange = [this]() {
            int value = mixer->getDynamicsCombo().getComboBox().getSelectedItemIndex();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::MIXER_DYNAMICS_SYSEX, value);
            }
        };
        
        // Mixer Env Mode Combo
        mixer->getEnvModeCombo().getComboBox().onChange = [this]() {
            int value = mixer->getEnvModeCombo().getComboBox().getSelectedItemIndex();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::MIXER_ENV_SOURCE_SYSEX, value);
            }
        };
        
        // VCA Level
        vca->getLevelSlider().getSlider().onValueChange = [this]() {
            float value = (float)vca->getLevelSlider().getValue();
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::VCA_LEVEL_SYSEX, (int)value);
            }
        };
        
        // VCF - switch to Filter view
        vcf->getCutoffSlider().getSlider().onValueChange = [this]() {
            float value = (float)vcf->getCutoffSlider().getValue();
            oscilloscope->setCutoff(value);
            oscilloscope->focusOnCategory("VCF");
            
            // Send MIDI to synth
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::VCF_CUTOFF_SYSEX, (int)value);
            }
        };
        vcf->getResonanceSlider().getSlider().onValueChange = [this]() {
            float value = (float)vcf->getResonanceSlider().getValue();
            oscilloscope->setResonance(value);
            oscilloscope->focusOnCategory("VCF");
            
            // Send MIDI to synth
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::VCF_RESONANCE_SYSEX, (int)value);
            }
        };
        vcf->getEnvModSlider().getSlider().onValueChange = [this]() {
            float value = (float)vcf->getEnvModSlider().getValue();
            oscilloscope->setVCFEnvMod(value);
            oscilloscope->focusOnCategory("VCF");
            
            // Send MIDI to synth
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::VCF_ENV_DEPTH_SYSEX, (int)value);
            }
        };
        
        // LFO - switch to Audio Out view + update LFO visualizer
        lfo->getRateSlider().getSlider().onValueChange = [this]() {
            float value = (float)lfo->getRateSlider().getValue();
            oscilloscope->setLFORate(value);
            oscilloscope->focusOnCategory("LFO");
            // Update visualizer in LFOSection
            lfo->updateVisualizerRate(value);
            
            // Send MIDI to synth
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::LFO_RATE, (int)value);
            }
        };
        
        // ENV1 - switch to Envelope1 view
        auto env1AttackOrig = env1->getAttackSlider().getSlider().onValueChange;
        env1->getAttackSlider().getSlider().onValueChange = [this, env1AttackOrig]() {
            if (env1AttackOrig) env1AttackOrig();
            float value = (float)env1->getAttackSlider().getValue();
            oscilloscope->setEnv1Attack(value);
            oscilloscope->focusOnCategory("ENV1");
            
            // Send MIDI to synth (ENV1 ADSR)
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::ENV1_ATTACK, (int)value);
            }
        };
        auto env1DecayOrig = env1->getDecaySlider().getSlider().onValueChange;
        env1->getDecaySlider().getSlider().onValueChange = [this, env1DecayOrig]() {
            if (env1DecayOrig) env1DecayOrig();
            float value = (float)env1->getDecaySlider().getValue();
            oscilloscope->setEnv1Decay(value);
            oscilloscope->focusOnCategory("ENV1");
            
            // Send MIDI to synth (ENV1 Decay)
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::ENV1_DECAY, (int)value);
            }
        };
        auto env1SustainOrig = env1->getSustainSlider().getSlider().onValueChange;
        env1->getSustainSlider().getSlider().onValueChange = [this, env1SustainOrig]() {
            if (env1SustainOrig) env1SustainOrig();
            float value = (float)env1->getSustainSlider().getValue();
            oscilloscope->setEnv1Sustain(value);
            oscilloscope->focusOnCategory("ENV1");
            
            // Send MIDI to synth (ENV1 Sustain)
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::ENV1_SUSTAIN, (int)value);
            }
        };
        auto env1ReleaseOrig = env1->getReleaseSlider().getSlider().onValueChange;
        env1->getReleaseSlider().getSlider().onValueChange = [this, env1ReleaseOrig]() {
            if (env1ReleaseOrig) env1ReleaseOrig();
            float value = (float)env1->getReleaseSlider().getValue();
            oscilloscope->setEnv1Release(value);
            oscilloscope->focusOnCategory("ENV1");
            
            // Send MIDI to synth (ENV1 Release)
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::ENV1_RELEASE, (int)value);
            }
        };
        env1->getKeyFollowCombo().getComboBox().onChange = [this]() {
            int value = env1->getKeyFollowCombo().getSelectedId() - 1;
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::ENV1_KEYFOLLOW_LOOP_SYSEX, value);
            }
        };
        
        // ENV2 - switch to Envelope2 view
        auto env2AttackOrig = env2->getAttackSlider().getSlider().onValueChange;
        env2->getAttackSlider().getSlider().onValueChange = [this, env2AttackOrig]() {
            if (env2AttackOrig) env2AttackOrig();
            float value = (float)env2->getAttackSlider().getValue();
            oscilloscope->setEnv2Attack(value);
            oscilloscope->focusOnCategory("ENV2");
            
            // Send MIDI to synth (ENV2 ADSR)
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::ENV2_ATTACK, (int)value);
            }
        };
        auto env2DecayOrig = env2->getDecaySlider().getSlider().onValueChange;
        env2->getDecaySlider().getSlider().onValueChange = [this, env2DecayOrig]() {
            if (env2DecayOrig) env2DecayOrig();
            float value = (float)env2->getDecaySlider().getValue();
            oscilloscope->setEnv2Decay(value);
            oscilloscope->focusOnCategory("ENV2");
            
            // Send MIDI to synth (ENV2 Decay)
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::ENV2_DECAY, (int)value);
            }
        };
        auto env2SustainOrig = env2->getSustainSlider().getSlider().onValueChange;
        env2->getSustainSlider().getSlider().onValueChange = [this, env2SustainOrig]() {
            if (env2SustainOrig) env2SustainOrig();
            float value = (float)env2->getSustainSlider().getValue();
            oscilloscope->setEnv2Sustain(value);
            oscilloscope->focusOnCategory("ENV2");
            
            // Send MIDI to synth (ENV2 Sustain)
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::ENV2_SUSTAIN, (int)value);
            }
        };
        auto env2ReleaseOrig = env2->getReleaseSlider().getSlider().onValueChange;
        env2->getReleaseSlider().getSlider().onValueChange = [this, env2ReleaseOrig]() {
            if (env2ReleaseOrig) env2ReleaseOrig();
            float value = (float)env2->getReleaseSlider().getValue();
            oscilloscope->setEnv2Release(value);
            oscilloscope->focusOnCategory("ENV2");
            
            // Send MIDI to synth (ENV2 Release)
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::V3::ENV2_RELEASE, (int)value);
            }
        };
        env2->getKeyFollowCombo().getComboBox().onChange = [this]() {
            int value = env2->getKeyFollowCombo().getSelectedId() - 1;
            if (sendMidiParameter) { int tone = upperTone ? 1 : 2; sendMidiParameter(tone, MKS70Mapping::ENV2_KEYFOLLOW_LOOP_SYSEX, value);
            }
        };
        
        // Chorus Type Switch - V3.x Chorus address is 43
        chorus->getTypeSwitch().onChange = [this](int value) {
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::V3::CHORUS, value);
            }
        };
        
        // Note: Chorus Rate is not part of the standard MKS-70 MIDI spec
        // It's controlled internally by the synth when chorus type is selected
        
        // === LFO2 Controls (Vecoven X3 Feature - may not work on V3.x) ===
        // Note: LFO2 parameters are V4.x/X3 firmware extensions
        // They will be logged but may have no effect on V3.x hardware
        
        // LFO2 Waveform
        lfo2->getWaveKnob().onChange = [this]() {
            int value = static_cast<int>(lfo2->getWaveKnob().getValue());
            lfo2->updateVisualizerWave(value);  // Update visualizer
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::LFO2_WAVEFORM_SYSEX, value);
            }
        };
        
        // LFO2 Delay
        lfo2->getDelaySlider().getSlider().onValueChange = [this]() {
            float value = (float)lfo2->getDelaySlider().getValue();
            lfo2->updateVisualizerDelay(value);  // Update visualizer
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::LFO2_DELAY_SYSEX, (int)value);
            }
        };
        
        // LFO2 Rate
        lfo2->getRateSlider().getSlider().onValueChange = [this]() {
            float value = (float)lfo2->getRateSlider().getValue();
            lfo2->updateVisualizerRate(value);  // Update visualizer
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::LFO2_RATE_SYSEX, (int)value);
            }
        };
        
        // LFO2 LFO1 Depth (cross-modulation)
        lfo2->getLFO1DepthSlider().getSlider().onValueChange = [this]() {
            float value = (float)lfo2->getLFO1DepthSlider().getValue();
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::LFO2_LFO1_DEPTH_SYSEX, (int)value);
            }
        };
        
        // LFO2 Sync Switch
        lfo2->getSyncSwitch().onChange = [this](int value) {
            if (sendMidiParameter) { 
                int tone = upperTone ? 1 : 2; 
                sendMidiParameter(tone, MKS70Mapping::LFO2_SYNC_SYSEX, value);
            }
        };
    }
};


