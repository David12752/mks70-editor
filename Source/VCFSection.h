#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "SectionComponent.h"
#include "LabeledSlider.h"
#include "RealisticKnob.h"
#include "DCOSection.h"  // For FirmwareVersion enum
#include "MKS70_Mapping.h"

class MKS70EditorAudioProcessorEditor;

//==============================================================================
// VCFSection - Filter section with Minimoog-style knobs
//==============================================================================
class VCFSection : public juce::Component {
public:
    VCFSection() {
        section = std::make_unique<SectionComponent>("VCF", DesignSystem::Color::AccentVCF());
        addAndMakeVisible(section.get());
        
        // HPF as rotary knob with surrounding indicators
        hpfKnob = std::make_unique<RealisticKnob>("HPF", RealisticKnob::KnobType::Range);
        hpfKnob->setItems({"0", "1", "2", "3"});
        hpfKnob->setTooltip("High Pass Filter\n\nRemoves low frequencies.\nHigher = thinner sound.\n\nTip: Use to cut through a mix or remove muddiness.");
        addAndMakeVisible(hpfKnob.get());
        
        // Main sliders - now knobs with tooltips
        cutoffSlider = std::make_unique<LabeledSlider>("Cutoff", LabeledSlider::Knob);
        cutoffSlider->setRange(0, 127);
        cutoffSlider->setDefaultValue(127);
        cutoffSlider->getSlider().setTooltip("Filter Cutoff Frequency\n\nSets the low-pass filter cutoff.\nLow = dark/muffled, High = bright.\n\nTip: The most important filter control!");
        addAndMakeVisible(cutoffSlider.get());
        
        resonanceSlider = std::make_unique<LabeledSlider>("Reso", LabeledSlider::Knob);
        resonanceSlider->setRange(0, 127);
        resonanceSlider->setDefaultValue(0);
        resonanceSlider->getSlider().setTooltip("Filter Resonance (Q)\n\nEmphasizes frequencies at cutoff.\nHigh values = sharp whistling peak.\n\nTip: Self-oscillates at maximum!");
        addAndMakeVisible(resonanceSlider.get());
        
        // Modulation knobs with tooltips
        lfoModSlider = std::make_unique<LabeledSlider>("LFO", LabeledSlider::Knob);
        lfoModSlider->setRange(0, 127);
        lfoModSlider->setDefaultValue(0);
        lfoModSlider->getSlider().setTooltip("LFO1 to Filter Modulation\n\nCreates rhythmic filter sweeps.\n\nTip: Use slow LFO rate for wah-wah effect.");
        addAndMakeVisible(lfoModSlider.get());
        
        lfo2ModSlider = std::make_unique<LabeledSlider>("LFO2", LabeledSlider::Knob);
        lfo2ModSlider->setRange(0, 127);
        lfo2ModSlider->setDefaultValue(0);
        lfo2ModSlider->getSlider().setTooltip("LFO2 to Filter Modulation (Vecoven X3)\n\nIndependent LFO2 modulation depth.\nUse for complex modulation patterns.\n\nTip: Combine with LFO1 for evolving sounds!");
        addAndMakeVisible(lfo2ModSlider.get());
        
        envModSlider = std::make_unique<LabeledSlider>("ENV", LabeledSlider::Knob);
        envModSlider->setRange(0, 127);
        envModSlider->setDefaultValue(0);
        envModSlider->getSlider().setTooltip("Envelope to Filter Modulation\n\nMakes filter follow envelope shape.\n+ = sweeps open, - = sweeps closed\n\nTip: Essential for classic synth bass!");
        addAndMakeVisible(envModSlider.get());
        
        keyFollowSlider = std::make_unique<LabeledSlider>("Key F", LabeledSlider::Knob);
        keyFollowSlider->setRange(0, 127);
        keyFollowSlider->setDefaultValue(0);
        keyFollowSlider->getSlider().setTooltip("Keyboard Follow\n\nHigher notes = higher cutoff.\nMimics acoustic instrument behavior.\n\nTip: Set to 100% for consistent timbre across keyboard.");
        addAndMakeVisible(keyFollowSlider.get());
        
        // Dynamics as knob with indicators
        dynamicsKnob = std::make_unique<RealisticKnob>("Dyn", RealisticKnob::KnobType::Range);
        dynamicsKnob->setItems({"Off", "1", "2", "3"});
        dynamicsKnob->setTooltip("Velocity to Filter Dynamics\n\nHarder playing = brighter sound.\n\nTip: Adds expression to keyboard performance.");
        addAndMakeVisible(dynamicsKnob.get());
        
        // Env Mode as knob
        envModeKnob = std::make_unique<RealisticKnob>("Env", RealisticKnob::KnobType::Range);
        envModeKnob->setItems({"E1", "E2", "E1+E2", "E1xE2"});
        envModeKnob->setTooltip("VCF Envelope Mode\n\nE1 = ENV-1 only\nE2 = ENV-2 only\nE1+E2 = Sum of both\nE1xE2 = Product (multiply)\n\nTip: Use E1 for classic filter sweeps!");
        addAndMakeVisible(envModeKnob.get());
    }
    
    void setEditor(MKS70EditorAudioProcessorEditor* ed, const juce::String& prefix) {
        cutoffSlider->setEditor(ed);
        cutoffSlider->setParameterName(prefix + "_VCF_CUTOFF");
        
        resonanceSlider->setEditor(ed);
        resonanceSlider->setParameterName(prefix + "_VCF_RESO");
        
        lfoModSlider->setEditor(ed);
        lfoModSlider->setParameterName(prefix + "_VCF_LFO");
        
        lfo2ModSlider->setEditor(ed);
        lfo2ModSlider->setParameterName(prefix + "_VCF_LFO2");
        
        envModSlider->setEditor(ed);
        envModSlider->setParameterName(prefix + "_VCF_ENV");
        
        keyFollowSlider->setEditor(ed);
        keyFollowSlider->setParameterName(prefix + "_VCF_KEYF");
    }
    
    void resized() override {
        using namespace DesignSystem;
        
        section->setBounds(getLocalBounds());
        auto bounds = section->getContentBounds();
        
        constexpr int knobSize = 60;  // RealisticKnob size
        constexpr int knobH = 75;     // RealisticKnob height
        constexpr int sliderH = 80;   // LabeledSlider height
        
        // Top row: selector knobs (HPF, Dynamics, EnvMode) - compact
        auto selectorRow = bounds.removeFromTop(knobH);
        int selectorW = selectorRow.getWidth() / 3;
        hpfKnob->setBounds(selectorRow.removeFromLeft(selectorW).withSizeKeepingCentre(knobSize, knobH));
        dynamicsKnob->setBounds(selectorRow.removeFromLeft(selectorW).withSizeKeepingCentre(knobSize, knobH));
        envModeKnob->setBounds(selectorRow.withSizeKeepingCentre(knobSize, knobH));
        
        bounds.removeFromTop(2);
        
        // Bottom row: main filter knobs - 6 columns if LFO2 shown, 5 if hidden
        int numKnobs = showLFO2 ? 6 : 5;
        int knobW = bounds.getWidth() / numKnobs;
        cutoffSlider->setBounds(bounds.removeFromLeft(knobW).withHeight(sliderH));
        resonanceSlider->setBounds(bounds.removeFromLeft(knobW).withHeight(sliderH));
        keyFollowSlider->setBounds(bounds.removeFromLeft(knobW).withHeight(sliderH));
        lfoModSlider->setBounds(bounds.removeFromLeft(knobW).withHeight(sliderH));
        if (showLFO2)
            lfo2ModSlider->setBounds(bounds.removeFromLeft(knobW).withHeight(sliderH));
        envModSlider->setBounds(bounds.withHeight(sliderH));
    }
    
    int getIdealHeight() const {
        using namespace DesignSystem;
        return Size::SectionHeaderHeight + 
               62 + 2 + 68 + 6;  // Two rows of compact knobs
    }
    
    LabeledSlider& getCutoffSlider() { return *cutoffSlider; }
    LabeledSlider& getResonanceSlider() { return *resonanceSlider; }
    LabeledSlider& getLFOModSlider() { return *lfoModSlider; }
    LabeledSlider& getLFO2ModSlider() { return *lfo2ModSlider; }
    LabeledSlider& getEnvModSlider() { return *envModSlider; }
    LabeledSlider& getKeyFollowSlider() { return *keyFollowSlider; }
    RealisticKnob& getHPFKnob() { return *hpfKnob; }
    RealisticKnob& getDynamicsKnob() { return *dynamicsKnob; }
    RealisticKnob& getEnvModeKnob() { return *envModeKnob; }
    
    // Firmware version support - LFO2 mod is V4.x only
    void setFirmwareVersion(FirmwareVersion version) {
        showLFO2 = (version == FirmwareVersion::Vecoven4 &&
                    MKS70Mapping::VCF_LFO2_DEPTH_SYSEX != MKS70Mapping::PARAM_NOT_AVAILABLE);
        lfo2ModSlider->setVisible(showLFO2);
        resized();
    }
    
private:
    std::unique_ptr<SectionComponent> section;
    std::unique_ptr<RealisticKnob> hpfKnob, dynamicsKnob, envModeKnob;
    std::unique_ptr<LabeledSlider> cutoffSlider, resonanceSlider;
    std::unique_ptr<LabeledSlider> lfoModSlider, lfo2ModSlider, envModSlider, keyFollowSlider;
    bool showLFO2 = false;  // Shown only when a real SysEx mapping exists
};
