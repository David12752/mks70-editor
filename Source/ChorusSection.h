#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "SectionComponent.h"
#include "LabeledSlider.h"
#include "CompactSwitch.h"

class MKS70EditorAudioProcessorEditor;

class ChorusSection : public juce::Component {
public:
    ChorusSection() {
        section = std::make_unique<SectionComponent>("Chorus", DS::Color::Section::chorus());
        addAndMakeVisible(section.get());

        // Chorus type as compact switch
        typeSwitch = std::make_unique<CompactSwitch>("Type");
        typeSwitch->setOptions({"Off", "I", "II"});
        typeSwitch->setTooltip("Chorus Type\n\nOff = No chorus\nI = Subtle stereo widening\nII = Deeper, more dramatic\n\nTip: Type II is the signature MKS-70 sound!");
        addAndMakeVisible(typeSwitch.get());

        // Rate knob with tooltip
        rate = std::make_unique<LabeledSlider>("Rate", LabeledSlider::Knob);
        rate->setRange(0, 127);
        rate->setDefaultValue(64);
        rate->getSlider().setTooltip("Chorus Speed\n\nModulation rate of the chorus effect.\nSlow = Gentle shimmer\nFast = Leslie-like rotation\n\nTip: Classic MKS-70 pad sound uses slow rate.");
        addAndMakeVisible(rate.get());
    }

    void setEditor(MKS70EditorAudioProcessorEditor* ed, const juce::String& prefix) {
        typeSwitch->setEditor(ed);
        typeSwitch->setParameterName(prefix + "_CHORUS_TYPE");
        
        rate->setEditor(ed);
        rate->setParameterName(prefix + "_CHORUS_RATE");
    }

    void resized() override {
        using namespace DesignSystem;
        section->setBounds(getLocalBounds());
        auto bounds = section->getContentBounds();

        constexpr int switchH = 40;   // CompactSwitch height
        constexpr int sliderH = 80;   // LabeledSlider height

        // Center the controls
        int totalWidth = bounds.getWidth();
        int elementWidth = totalWidth / 2 - 4;
        
        // Reduce bounds to center content
        bounds = bounds.reduced(4, 0);
        
        // Switch on left, knob on right - both centered
        auto leftCol = bounds.removeFromLeft(elementWidth + 2);
        auto rightCol = bounds;
        
        typeSwitch->setBounds(leftCol.withSizeKeepingCentre(elementWidth, switchH));
        rate->setBounds(rightCol.withSizeKeepingCentre(elementWidth, sliderH));
    }

    int getIdealHeight() const {
        using namespace DesignSystem;
        return Size::SectionHeaderHeight + 70 + 6;
    }
    
    LabeledSlider& getRateSlider() { return *rate; }
    CompactSwitch& getTypeSwitch() { return *typeSwitch; }

private:
    std::unique_ptr<SectionComponent> section;
    std::unique_ptr<LabeledSlider> rate;
    std::unique_ptr<CompactSwitch> typeSwitch;
};
