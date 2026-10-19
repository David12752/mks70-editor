#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "SectionComponent.h"
#include "LabeledSlider.h"
#include "LabeledComboBox.h"
#include "RealisticKnob.h"
#include "Juno60Fader.h"
#include "CompactSwitch.h"
#include "MKS70_Mapping.h"

class MKS70EditorAudioProcessorEditor;

// Firmware version enum - shared across the app
enum class FirmwareVersion {
    Original = 0,   // Original MKS-70 ROM
    Vecoven3 = 1,   // Vecoven V3.x
    Vecoven4 = 2    // Vecoven V4.x (includes LFO2, PWM, etc)
};

class DCOSection : public juce::Component {
public:
    DCOSection(int dcoNumber, bool hasCrossMod = false)
        : dcoNum(dcoNumber), showCrossMod(hasCrossMod)
    {
        juce::String title = "DCO-" + juce::String(dcoNumber);
        // Use new vibrant section colors
        juce::Colour accent = dcoNumber == 1 ?
            DS::Color::Section::dco1() :
            DS::Color::Section::dco2();
        
        section = std::make_unique<SectionComponent>(title, accent);
        addAndMakeVisible(section.get());
        
        // === TOP ROW: Range and Wave knobs ===
        rangeKnob = std::make_unique<RealisticKnob>("Range", RealisticKnob::KnobType::Octave);
        rangeKnob->setRange(0, 3, 1);
        rangeKnob->setItems({"16'", "8'", "4'", "2'"});
        rangeKnob->setTooltip("Octave Range");
        addAndMakeVisible(rangeKnob.get());
        
        waveKnob = std::make_unique<RealisticKnob>("Wave", RealisticKnob::KnobType::Waveform);
        waveKnob->setRange(0, 3, 1);
        waveKnob->setItems({"Pulse", "Saw", "Square", "Noise"});
        waveKnob->setTooltip("Waveform Shape");
        addAndMakeVisible(waveKnob.get());
        
        // === FADERS: Tune, LFO, ENV (Juno-60 style) ===
        tuneFader = std::make_unique<Juno60Fader>("Tune");
        tuneFader->setRange(0, 127);
        tuneFader->setValue(64, juce::dontSendNotification);
        tuneFader->getSlider().setTooltip("Fine Tune (64 = center)");
        addAndMakeVisible(tuneFader.get());
        
        lfoFader = std::make_unique<Juno60Fader>("LFO");
        lfoFader->setRange(0, 127);
        lfoFader->setValue(0, juce::dontSendNotification);
        lfoFader->getSlider().setTooltip("LFO Pitch Modulation");
        addAndMakeVisible(lfoFader.get());
        
        envFader = std::make_unique<Juno60Fader>("ENV");
        envFader->setRange(0, 127);
        envFader->setValue(0, juce::dontSendNotification);
        envFader->getSlider().setTooltip("Envelope Pitch Modulation");
        addAndMakeVisible(envFader.get());
        
        // === KNOBS: Dynamics and Env Source ===
        dynamicsSwitch = std::make_unique<CompactSwitch>("Dynamics");
        dynamicsSwitch->setOptions({"Off", "1", "2", "3"});
        dynamicsSwitch->setTooltip("Velocity Dynamics");
        addAndMakeVisible(dynamicsSwitch.get());
        
        envSrcSwitch = std::make_unique<CompactSwitch>("Env Src");
        envSrcSwitch->setOptions({"E1", "E2", "E1+E2", "E1xE2"});
        envSrcSwitch->setTooltip("DCO Envelope Mode\n\nE1 = ENV-1 only\nE2 = ENV-2 only\nE1+E2 = Sum of both\nE1xE2 = Product (multiply)");
        addAndMakeVisible(envSrcSwitch.get());
        
        // === PWM Section (Vecoven V4.x ONLY) ===
        pwmFader = std::make_unique<Juno60Fader>("PWM");
        pwmFader->setRange(0, 127);
        pwmFader->setValue(64, juce::dontSendNotification);
        pwmFader->getSlider().setTooltip("Pulse Width (V4.x only)");
        addAndMakeVisible(pwmFader.get());
        
        pwmLfoFader = std::make_unique<Juno60Fader>("P-LFO");
        pwmLfoFader->setRange(0, 127);
        pwmLfoFader->setValue(0, juce::dontSendNotification);
        pwmLfoFader->getSlider().setTooltip("PWM LFO Depth (V4.x only)");
        addAndMakeVisible(pwmLfoFader.get());
        
        pwmEnvFader = std::make_unique<Juno60Fader>("P-ENV");
        pwmEnvFader->setRange(0, 127);
        pwmEnvFader->setValue(0, juce::dontSendNotification);
        pwmEnvFader->getSlider().setTooltip("PWM Envelope Depth (V4.x only)");
        addAndMakeVisible(pwmEnvFader.get());
        
        // === DCO2 Only: CrossMod and Fine ===
        if (showCrossMod) {
            crossModKnob = std::make_unique<RealisticKnob>("X-Mod", RealisticKnob::KnobType::Range);
            crossModKnob->setRange(0, 3, 1);
            crossModKnob->setItems({"Off", "Sync1", "Sync2", "Metal"});
            crossModKnob->setTooltip("Cross Modulation");
            addAndMakeVisible(crossModKnob.get());
            
            fineFader = std::make_unique<Juno60Fader>("Fine");
            fineFader->setRange(0, 127);
            fineFader->setValue(64, juce::dontSendNotification);
            fineFader->getSlider().setTooltip("Very Fine Tune");
            addAndMakeVisible(fineFader.get());
        }
        
        // Default to V3.x (no PWM visible)
        setFirmwareVersion(FirmwareVersion::Vecoven3);
    }
    
    void setFirmwareVersion(FirmwareVersion version) {
        currentFirmware = version;
        bool showPWM = (version == FirmwareVersion::Vecoven4 &&
                        MKS70Mapping::PWM_DCO1_WIDTH_SYSEX != MKS70Mapping::PARAM_NOT_AVAILABLE);
        
        pwmFader->setVisible(showPWM);
        pwmLfoFader->setVisible(showPWM);
        pwmEnvFader->setVisible(showPWM);
        
        resized();
        repaint();
    }
    
    FirmwareVersion getFirmwareVersion() const { return currentFirmware; }
    
    void setEditor(MKS70EditorAudioProcessorEditor* ed, const juce::String& prefix) {
        tuneFader->setEditor(ed);
        tuneFader->setParameterName(prefix + "_TUNE");
        tuneFader->getSlider().onValueChange = [this]() {
            if (onTuneChanged) onTuneChanged(tuneFader->getValue());
        };
        
        lfoFader->setEditor(ed);
        lfoFader->setParameterName(prefix + "_LFO");
        lfoFader->getSlider().onValueChange = [this]() {
            if (onLfoChanged) onLfoChanged(lfoFader->getValue());
        };
        
        envFader->setEditor(ed);
        envFader->setParameterName(prefix + "_ENV");
        envFader->getSlider().onValueChange = [this]() {
            if (onEnvChanged) onEnvChanged(envFader->getValue());
        };
        
        // PWM controls (V4.x only)
        pwmFader->setEditor(ed);
        pwmFader->setParameterName(prefix + "_PWM_WIDTH");
        
        pwmLfoFader->setEditor(ed);
        pwmLfoFader->setParameterName(prefix + "_PWM_LFO");
        
        pwmEnvFader->setEditor(ed);
        pwmEnvFader->setParameterName(prefix + "_PWM_ENV");
        
        // RealisticKnobs
        rangeKnob->setEditor(ed);
        rangeKnob->setParameterName(prefix + "_RANGE");
        
        waveKnob->setEditor(ed);
        waveKnob->setParameterName(prefix + "_WAVE");
        
        // CompactSwitch for Dynamics and Env Source
        dynamicsSwitch->setEditor(ed);
        dynamicsSwitch->setParameterName(prefix + "_DYNAMICS");
        
        envSrcSwitch->setEditor(ed);
        envSrcSwitch->setParameterName(prefix + "_ENV_SRC");
        
        if (showCrossMod) {
            if (crossModKnob) {
                crossModKnob->setEditor(ed);
                crossModKnob->setParameterName(prefix + "_CROSSMOD");
            }
            if (fineFader) {
                fineFader->setEditor(ed);
                fineFader->setParameterName(prefix + "_FINE");
            }
        }
    }
    
    void resized() override {
        section->setBounds(getLocalBounds());
        auto bounds = section->getContentBounds();
        
        bool showPWM = (currentFirmware == FirmwareVersion::Vecoven4 &&
                        MKS70Mapping::PWM_DCO1_WIDTH_SYSEX != MKS70Mapping::PARAM_NOT_AVAILABLE);
        
        // Layout constants - sizes depend on whether PWM is shown
        const int knobSize = showPWM ? 55 : 62;
        const int smallKnobSize = showPWM ? 48 : 55;
        const int faderWidth = showPWM ? 48 : 56;  // Wider faders
        const int spacing = showPWM ? 4 : 6;
        
        // === TOP ROW: Range & Wave knobs (+ X-Mod for DCO2) ===
        auto topRow = bounds.removeFromTop(knobSize + 8);
        
        if (showCrossMod && crossModKnob) {
            // DCO2: Range, Wave, X-Mod
            const int knobSlotW = topRow.getWidth() / 3;
            rangeKnob->setBounds(topRow.removeFromLeft(knobSlotW).withSizeKeepingCentre(knobSize, knobSize));
            waveKnob->setBounds(topRow.removeFromLeft(knobSlotW).withSizeKeepingCentre(knobSize, knobSize));
            crossModKnob->setBounds(topRow.withSizeKeepingCentre(knobSize, knobSize));
        } else {
            // DCO1: Range, Wave (centered)
            const int knobSlotW = topRow.getWidth() / 2;
            rangeKnob->setBounds(topRow.removeFromLeft(knobSlotW).withSizeKeepingCentre(knobSize, knobSize));
            waveKnob->setBounds(topRow.withSizeKeepingCentre(knobSize, knobSize));
        }
        
        bounds.removeFromTop(spacing);
        
        if (showPWM) {
            // === V4.x LAYOUT: Faders + PWM + small knobs ===
            
            // MIDDLE ROW: Main faders (Tune, LFO, ENV) + Fine for DCO2
            auto faderRow = bounds.removeFromTop(bounds.getHeight() / 2);
            const int numFaders = showCrossMod ? 4 : 3;
            const int faderSlotW = faderRow.getWidth() / numFaders;
            
            tuneFader->setBounds(faderRow.removeFromLeft(faderSlotW).withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
            lfoFader->setBounds(faderRow.removeFromLeft(faderSlotW).withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
            envFader->setBounds(faderRow.removeFromLeft(faderSlotW).withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
            
            if (showCrossMod && fineFader) {
                fineFader->setBounds(faderRow.withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
            }
            
            bounds.removeFromTop(spacing);
            
            // BOTTOM ROW: PWM faders + Dynamics/EnvSrc switches
            auto bottomRow = bounds;
            const int pwmWidth = (bottomRow.getWidth() * 3) / 5;
            
            auto pwmArea = bottomRow.removeFromLeft(pwmWidth);
            auto switchArea = bottomRow;
            
            // PWM faders
            const int pwmSlotW = pwmArea.getWidth() / 3;
            pwmFader->setBounds(pwmArea.removeFromLeft(pwmSlotW).withSizeKeepingCentre(faderWidth, pwmArea.getHeight() - 4));
            pwmLfoFader->setBounds(pwmArea.removeFromLeft(pwmSlotW).withSizeKeepingCentre(faderWidth, pwmArea.getHeight() - 4));
            pwmEnvFader->setBounds(pwmArea.withSizeKeepingCentre(faderWidth, pwmArea.getHeight() - 4));
            
            // Switches for Dynamics and EnvSrc
            const int switchH = 36;
            dynamicsSwitch->setBounds(switchArea.removeFromTop(switchH).reduced(2, 0));
            envSrcSwitch->setBounds(switchArea.removeFromTop(switchH).reduced(2, 0));
        } else {
            // === V3.x / Original LAYOUT: Larger faders, no PWM ===
            
            // MIDDLE ROW: Main faders (Tune, LFO, ENV) + Fine for DCO2
            auto faderRow = bounds.removeFromTop(bounds.getHeight() * 2 / 3);
            const int numFaders = showCrossMod ? 4 : 3;
            const int faderSlotW = faderRow.getWidth() / numFaders;
            
            tuneFader->setBounds(faderRow.removeFromLeft(faderSlotW).withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
            lfoFader->setBounds(faderRow.removeFromLeft(faderSlotW).withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
            envFader->setBounds(faderRow.removeFromLeft(faderSlotW).withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
            
            if (showCrossMod && fineFader) {
                fineFader->setBounds(faderRow.withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
            }
            
            bounds.removeFromTop(spacing);
            
            // BOTTOM ROW: Dynamics & EnvSrc switches (centered)
            auto bottomRow = bounds;
            const int switchH = 36;
            dynamicsSwitch->setBounds(bottomRow.removeFromTop(switchH).reduced(4, 0));
            envSrcSwitch->setBounds(bottomRow.removeFromTop(switchH).reduced(4, 0));
        }
    }
    
    // Getters
    Juno60Fader& getTuneSlider() { return *tuneFader; }
    Juno60Fader& getLFOModSlider() { return *lfoFader; }
    Juno60Fader& getEnvModSlider() { return *envFader; }
    Juno60Fader& getPWMWidthSlider() { return *pwmFader; }
    Juno60Fader& getPWMLFOSlider() { return *pwmLfoFader; }
    Juno60Fader& getPWMEnvSlider() { return *pwmEnvFader; }
    Juno60Fader* getFineSlider() { return fineFader.get(); }
    RealisticKnob* getCrossModKnob() { return crossModKnob.get(); }
    RealisticKnob& getRangeKnob() { return *rangeKnob; }
    RealisticKnob& getWaveKnob() { return *waveKnob; }
    CompactSwitch& getDynamicsSwitch() { return *dynamicsSwitch; }
    CompactSwitch& getEnvSrcSwitch() { return *envSrcSwitch; }
    
    // Callbacks for oscilloscope connection
    std::function<void(double)> onTuneChanged;
    std::function<void(double)> onLfoChanged;
    std::function<void(double)> onEnvChanged;

private:
    int dcoNum;
    bool showCrossMod;
    FirmwareVersion currentFirmware = FirmwareVersion::Vecoven3;
    
    std::unique_ptr<SectionComponent> section;
    
    // Top row knobs
    std::unique_ptr<RealisticKnob> rangeKnob, waveKnob;
    std::unique_ptr<RealisticKnob> crossModKnob;  // DCO2 only
    
    // Main faders
    std::unique_ptr<Juno60Fader> tuneFader, lfoFader, envFader;
    std::unique_ptr<Juno60Fader> fineFader;  // DCO2 only
    
    // PWM faders (V4.x only)
    std::unique_ptr<Juno60Fader> pwmFader, pwmLfoFader, pwmEnvFader;
    
    // Switches for Dynamics and Env Source
    std::unique_ptr<CompactSwitch> dynamicsSwitch, envSrcSwitch;
};
