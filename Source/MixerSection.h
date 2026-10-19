#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "SectionComponent.h"
#include "LabeledSlider.h"
#include "LabeledComboBox.h"
#include "Juno60Fader.h"

class MKS70EditorAudioProcessorEditor;

class MixerSection : public juce::Component {
public:
    MixerSection() {
        section = std::make_unique<SectionComponent>("Mixer", DS::Color::Section::mixer());
        addAndMakeVisible(section.get());

        // Replace knobs with tall faders for better precision
        dco1Fader = std::make_unique<Juno60Fader>("DCO1");
        dco1Fader->setRange(0, 127);
        dco1Fader->setValue(100, juce::dontSendNotification);
        dco1Fader->getSlider().setTooltip("DCO1 Volume\n\nLevel of oscillator 1 in the mix.\n\nTip: Balance with DCO2 for desired tone.");
        addAndMakeVisible(dco1Fader.get());

        dco2Fader = std::make_unique<Juno60Fader>("DCO2");
        dco2Fader->setRange(0, 127);
        dco2Fader->setValue(100, juce::dontSendNotification);
        dco2Fader->getSlider().setTooltip("DCO2 Volume\n\nLevel of oscillator 2 in the mix.\nSet to 0 for single-oscillator sounds.\n\nTip: Uneven mix creates timbral variation.");
        addAndMakeVisible(dco2Fader.get());

        envFader = std::make_unique<Juno60Fader>("ENV");
        envFader->setRange(0, 127);
        envFader->setValue(0, juce::dontSendNotification);
        envFader->getSlider().setTooltip("Envelope Mixer Modulation\n\nDynamic oscillator balance.\nCreates evolving timbral changes.\n\nTip: Use with crossfade between DCO waves.");
        addAndMakeVisible(envFader.get());

        dynamicsCombo = std::make_unique<LabeledComboBox>("Dyn", LabeledComboBox::Compact);
        dynamicsCombo->addItems({"0", "1", "2", "3"});
        dynamicsCombo->getComboBox().setTooltip("Velocity to Mix\n\nHarder playing affects DCO balance.\n\nTip: Adds expression to layered sounds.");
        addAndMakeVisible(dynamicsCombo.get());

        envModeCombo = std::make_unique<LabeledComboBox>("EnvM", LabeledComboBox::Compact);
        envModeCombo->addItems({"E2-", "E2+", "E1-", "E1+"});
        envModeCombo->getComboBox().setTooltip("Mixer Envelope Mode\n\nWhich envelope and polarity.\n+ = Increases DCO2 over time\n- = Decreases DCO2 over time\n\nNote: Vecoven 4.x adds E3/E4 options!");
        addAndMakeVisible(envModeCombo.get());
    }

    void setEditor(MKS70EditorAudioProcessorEditor* ed, const juce::String& prefix) {
        dco1Fader->setEditor(ed);
        dco1Fader->setParameterName(prefix + "_MIX_DCO1");
        
        dco2Fader->setEditor(ed);
        dco2Fader->setParameterName(prefix + "_MIX_DCO2");
        
        envFader->setEditor(ed);
        envFader->setParameterName(prefix + "_MIX_ENV");
    }

    void resized() override {
        using namespace DesignSystem;
        section->setBounds(getLocalBounds());
        auto bounds = section->getContentBounds();

        // Combos at top (compact)
        auto comboRow = bounds.removeFromTop(20);
        int comboW = (comboRow.getWidth() - 4) / 2;
        dynamicsCombo->setBounds(comboRow.removeFromLeft(comboW));
        comboRow.removeFromLeft(4);
        envModeCombo->setBounds(comboRow);
        
        bounds.removeFromTop(4);

        // Three tall faders for volume controls
        const int faderWidth = 52;
        auto faderRow = bounds;
        int faderSlotW = faderRow.getWidth() / 3;
        
        dco1Fader->setBounds(faderRow.removeFromLeft(faderSlotW).withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
        dco2Fader->setBounds(faderRow.removeFromLeft(faderSlotW).withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
        envFader->setBounds(faderRow.withSizeKeepingCentre(faderWidth, faderRow.getHeight() - 4));
    }

    int getIdealHeight() const {
        using namespace DesignSystem;
        return Size::SectionHeaderHeight + 120;  // Taller for faders
    }
    
    Juno60Fader& getDCO1Level() { return *dco1Fader; }
    Juno60Fader& getDCO2Level() { return *dco2Fader; }
    Juno60Fader& getEnvDepth() { return *envFader; }
    LabeledComboBox& getDynamicsCombo() { return *dynamicsCombo; }
    LabeledComboBox& getEnvModeCombo() { return *envModeCombo; }

private:
    std::unique_ptr<SectionComponent> section;
    std::unique_ptr<Juno60Fader> dco1Fader, dco2Fader, envFader;
    std::unique_ptr<LabeledComboBox> dynamicsCombo, envModeCombo;
};
