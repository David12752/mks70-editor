#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "SectionComponent.h"
#include "LabeledSlider.h"
#include "LabeledComboBox.h"

class MKS70EditorAudioProcessorEditor;

//==============================================================================
// EnvelopeSection - ADSR section (visualizer removed - use DynamicOscilloscope)
//==============================================================================
class EnvelopeSection : public juce::Component {
public:
    explicit EnvelopeSection(int envNumber) : envNum(envNumber) {
        juce::String title = "ENV-" + juce::String(envNumber);
        juce::Colour accent = envNumber == 1 ? 
            DS::Color::Section::env1() : DS::Color::Section::env2();
        
        section = std::make_unique<SectionComponent>(title, accent);
        addAndMakeVisible(section.get());
        
        // Note: ADSRVisualizer removed - DynamicOscilloscope now handles visualization
        
        // ADSR Knobs with tooltips
        attackSlider = std::make_unique<LabeledSlider>("A", LabeledSlider::Knob);
        attackSlider->setRange(0, 127);
        attackSlider->setDefaultValue(0);
        attackSlider->getSlider().onValueChange = [this]() { updateVisualizer(); };
        attackSlider->getSlider().setTooltip("Attack Time\n\nTime for the envelope to reach peak level.\n0 = instant, 127 = ~10 seconds\n\nTip: Fast attack for percussive sounds, slow for pads.");
        addAndMakeVisible(attackSlider.get());
        
        decaySlider = std::make_unique<LabeledSlider>("D", LabeledSlider::Knob);
        decaySlider->setRange(0, 127);
        decaySlider->setDefaultValue(64);
        decaySlider->getSlider().onValueChange = [this]() { updateVisualizer(); };
        decaySlider->getSlider().setTooltip("Decay Time\n\nTime to fall from peak to sustain level.\n\nTip: Short decay = snappy, Long decay = expressive.");
        addAndMakeVisible(decaySlider.get());
        
        sustainSlider = std::make_unique<LabeledSlider>("S", LabeledSlider::Knob);
        sustainSlider->setRange(0, 127);
        sustainSlider->setDefaultValue(100);
        sustainSlider->getSlider().onValueChange = [this]() { updateVisualizer(); };
        sustainSlider->getSlider().setTooltip("Sustain Level\n\nLevel held while key is pressed.\n0 = percussive, 127 = organ-like\n\nTip: Higher sustain for legato playing.");
        addAndMakeVisible(sustainSlider.get());
        
        releaseSlider = std::make_unique<LabeledSlider>("R", LabeledSlider::Knob);
        releaseSlider->setRange(0, 127);
        releaseSlider->setDefaultValue(32);
        releaseSlider->getSlider().onValueChange = [this]() { updateVisualizer(); };
        releaseSlider->getSlider().setTooltip("Release Time\n\nTime to fade to zero after key release.\n\nTip: Short for staccato, long for ambient/reverb tails.");
        addAndMakeVisible(releaseSlider.get());
        
        // Key Follow - compact with tooltip
        keyFollowCombo = std::make_unique<LabeledComboBox>("KeyF", LabeledComboBox::Compact);
        keyFollowCombo->addItems({"Off", "1", "2", "3"});
        keyFollowCombo->getComboBox().setTooltip("Envelope Key Follow\n\nHigher notes = faster envelope.\nAdds natural acoustic behavior.\n\nTip: Use for more expressive keyboard playing.");
        addAndMakeVisible(keyFollowCombo.get());
    }
    
    void setEditor(MKS70EditorAudioProcessorEditor* ed, const juce::String& prefix) {
        juce::String envPrefix = prefix + "_ENV" + juce::String(envNum);
        
        attackSlider->setEditor(ed);
        attackSlider->setParameterName(envPrefix + "_A");
        
        decaySlider->setEditor(ed);
        decaySlider->setParameterName(envPrefix + "_D");
        
        sustainSlider->setEditor(ed);
        sustainSlider->setParameterName(envPrefix + "_S");
        
        releaseSlider->setEditor(ed);
        releaseSlider->setParameterName(envPrefix + "_R");
        
        // Note: keyFollowCombo binding handled separately via direct combo access
        // Use getKeyFollowCombo().getComboBox() for manual MIDI routing
    }
    
    void resized() override {
        using namespace DesignSystem;
        
        section->setBounds(getLocalBounds());
        auto bounds = section->getContentBounds();
        
        constexpr int headerControlH = 16;
        constexpr int sliderH = 110;  // Much larger knobs
        constexpr int spacing = 4;

        auto topRow = bounds.removeFromTop(headerControlH);
        keyFollowCombo->setBounds(topRow.removeFromRight(58));

        bounds.removeFromTop(spacing);

        // Use full available height for knobs
        auto sliderRow = bounds;
        const int knobW = sliderRow.getWidth() / 4;
        attackSlider->setBounds(sliderRow.removeFromLeft(knobW).reduced(2, 0));
        decaySlider->setBounds(sliderRow.removeFromLeft(knobW).reduced(2, 0));
        sustainSlider->setBounds(sliderRow.removeFromLeft(knobW).reduced(2, 0));
        releaseSlider->setBounds(sliderRow.reduced(2, 0));
    }
    
    int getIdealHeight() const {
        return DesignSystem::Size::SectionHeaderHeight + 16 + 4 + 120 + 8;  // Larger knobs
    }
    
    LabeledSlider& getAttackSlider() { return *attackSlider; }
    LabeledSlider& getDecaySlider() { return *decaySlider; }
    LabeledSlider& getSustainSlider() { return *sustainSlider; }
    LabeledSlider& getReleaseSlider() { return *releaseSlider; }
    LabeledComboBox& getKeyFollowCombo() { return *keyFollowCombo; }
    
    // Connect sliders to repaint visualizer
    void connectToRepaint() {
        // Already connected in constructor via onValueChange
    }

private:
    int envNum;
    std::unique_ptr<SectionComponent> section;
    // ADSRVisualizer removed - using DynamicOscilloscope instead
    std::unique_ptr<LabeledSlider> attackSlider, decaySlider, sustainSlider, releaseSlider;
    std::unique_ptr<LabeledComboBox> keyFollowCombo;
    
    void updateVisualizer() {
        // Visualization now handled by DynamicOscilloscope
        repaint();
    }
};
