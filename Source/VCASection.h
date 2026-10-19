#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "SectionComponent.h"
#include "LabeledSlider.h"
#include "RealisticKnob.h"

class MKS70EditorAudioProcessorEditor;

class VCASection : public juce::Component {
public:
    VCASection() {
        section = std::make_unique<SectionComponent>("VCA", DS::Color::Section::vca());
        addAndMakeVisible(section.get());

        // VCA Level knob with tooltip
        level = std::make_unique<LabeledSlider>("Level", LabeledSlider::Knob);
        level->setRange(0, 127);
        level->setDefaultValue(100);
        level->getSlider().setTooltip("Master Volume\n\nOverall loudness of the voice.\n\nTip: Keep under 100 to avoid clipping when mixing tones.");
        addAndMakeVisible(level.get());

        // ENV Source as knob with surrounding indicators
        envSourceKnob = std::make_unique<RealisticKnob>("Env", RealisticKnob::KnobType::Range);
        envSourceKnob->setItems({"Gate", "ENV2"});
        envSourceKnob->setTooltip("VCA Envelope Mode\n\nGate = No envelope (holds at max while key pressed)\nENV2 = Uses ENV-2 for amplitude shaping\n\nTip: ENV2 for standard ADSR volume control!");
        addAndMakeVisible(envSourceKnob.get());

        // Dynamics as knob with surrounding indicators
        dynamicsKnob = std::make_unique<RealisticKnob>("Dyn", RealisticKnob::KnobType::Range);
        dynamicsKnob->setItems({"0", "1", "2", "3"});
        dynamicsKnob->setTooltip("Velocity Sensitivity\n\n0 = No velocity response\n3 = Maximum sensitivity\n\nTip: Higher values = more expressive playing.");
        addAndMakeVisible(dynamicsKnob.get());
    }

    void setEditor(MKS70EditorAudioProcessorEditor* ed, const juce::String& prefix) {
        level->setEditor(ed);
        level->setParameterName(prefix + "_VCA_LEVEL");
    }

    void resized() override {
        using namespace DesignSystem;
        section->setBounds(getLocalBounds());
        auto bounds = section->getContentBounds();

        constexpr int knobSize = 60;  // RealisticKnob size
        constexpr int knobH = 75;     // RealisticKnob height
        constexpr int sliderH = 80;   // LabeledSlider height
        
        // Three elements in a row - compact
        int colW = bounds.getWidth() / 3;
        level->setBounds(bounds.removeFromLeft(colW).withHeight(sliderH));
        envSourceKnob->setBounds(bounds.removeFromLeft(colW).withSizeKeepingCentre(knobSize, knobH));
        dynamicsKnob->setBounds(bounds.withSizeKeepingCentre(knobSize, knobH));
    }

    int getIdealHeight() const {
        using namespace DesignSystem;
        return Size::SectionHeaderHeight + 70 + 6;
    }
    
    LabeledSlider& getLevelSlider() { return *level; }
    RealisticKnob& getEnvSourceKnob() { return *envSourceKnob; }
    RealisticKnob& getDynamicsKnob() { return *dynamicsKnob; }

private:
    std::unique_ptr<SectionComponent> section;
    std::unique_ptr<LabeledSlider> level;
    std::unique_ptr<RealisticKnob> envSourceKnob, dynamicsKnob;
};
