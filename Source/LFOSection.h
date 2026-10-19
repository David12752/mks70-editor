#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "SectionComponent.h"
#include "LabeledSlider.h"
#include "LabeledComboBox.h"
#include "RealisticKnob.h"
#include "CompactSwitch.h"
#include "DCOSection.h"  // For FirmwareVersion enum
#include "MKS70_Mapping.h"

class MKS70EditorAudioProcessorEditor;

//==============================================================================
// LFOVisualizer - Animated LFO waveform display
//==============================================================================
class LFOVisualizer : public juce::Component, private juce::Timer {
public:
    LFOVisualizer() {
        startTimerHz(30);
    }
    
    void setWaveform(int w) { waveType = w; repaint(); }
    void setRate(float r) { rate = r; } // 0-127
    void setDelay(float d) { delayTime = d; }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(DS::Color::bgInput());
        g.fillRoundedRectangle(bounds, DS::Radius::S);
        
        // Draw waveform
        juce::Path path;
        float w = bounds.getWidth();
        float h = bounds.getHeight();
        float centerY = bounds.getCentreY();
        float amp = h * 0.35f;
        
        // Calculate how many cycles based on rate
        float cycles = 1.5f + (rate / 127.0f) * 2.0f;
        
        for (int i = 0; i <= (int)w; i++) {
            float t = (float)i / w * cycles * juce::MathConstants<float>::twoPi + phase;
            float y = centerY;
            
            switch (waveType) {
                case 0: // Triangle
                    y = centerY - amp * (2.0f * std::abs(2.0f * (t / juce::MathConstants<float>::twoPi - std::floor(t / juce::MathConstants<float>::twoPi + 0.5f))) - 1.0f);
                    break;
                case 1: // Saw
                    y = centerY - amp * (2.0f * std::fmod(t / juce::MathConstants<float>::twoPi, 1.0f) - 1.0f);
                    break;
                case 2: // Square
                    y = centerY - amp * (std::sin(t) > 0 ? 1.0f : -1.0f);
                    break;
                case 3: // S&H (random steps)
                    {
                        int segment = (int)(t / (juce::MathConstants<float>::twoPi / 4.0f)) % 8;
                        float values[] = {0.7f, -0.3f, 0.9f, -0.8f, 0.2f, -0.5f, 0.4f, -0.6f};
                        y = centerY - amp * values[segment];
                    }
                    break;
                default:
                    y = centerY - amp * std::sin(t);
            }
            
            if (i == 0) path.startNewSubPath(bounds.getX() + i, y);
            else path.lineTo(bounds.getX() + i, y);
        }
        
        // Glow effect
        juce::Colour waveColor = DS::Color::Section::lfo();
        g.setColour(waveColor.withAlpha(0.15f));
        g.strokePath(path, juce::PathStrokeType(4.0f));
        
        g.setColour(waveColor.withAlpha(0.4f));
        g.strokePath(path, juce::PathStrokeType(2.0f));
        
        g.setColour(waveColor);
        g.strokePath(path, juce::PathStrokeType(1.2f));
        
        // Animated playhead
        float playX = bounds.getX() + (phase / (juce::MathConstants<float>::twoPi * 2.0f)) * w;
        playX = bounds.getX() + std::fmod(playX - bounds.getX(), w);
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.drawVerticalLine((int)playX, bounds.getY(), bounds.getBottom());
    }
    
private:
    int waveType = 0;
    float rate = 64.0f;
    float delayTime = 0.0f;
    float phase = 0.0f;
    
    void timerCallback() override {
        phase += 0.05f + (rate / 127.0f) * 0.15f;
        if (phase > juce::MathConstants<float>::twoPi * 4.0f)
            phase -= juce::MathConstants<float>::twoPi * 4.0f;
        repaint();
    }
};

//==============================================================================
// LFOSection - Enhanced with visualizer
//==============================================================================
class LFOSection : public juce::Component {
public:
    LFOSection() {
        section = std::make_unique<SectionComponent>("LFO", DS::Color::Section::lfo());
        addAndMakeVisible(section.get());
        
        // Visualizer
        visualizer = std::make_unique<LFOVisualizer>();
        addAndMakeVisible(visualizer.get());

        // LFO Wave Knob with waveform icons
        waveKnob = std::make_unique<RealisticKnob>("Wave", RealisticKnob::KnobType::Waveform);
        waveKnob->setRange(0, 3, 1);
        waveKnob->setItems({"Tri", "Saw", "Sqr", "S&H"});
        waveKnob->setTooltip("LFO Waveform\n\nTri = Smooth vibrato\nSaw = Ramp sweeps\nSqr = Abrupt switching\nS&H = Random steps");
        waveKnob->onChange = [this]() {
            visualizer->setWaveform(static_cast<int>(waveKnob->getValue()));
        };
        addAndMakeVisible(waveKnob.get());

        delay = std::make_unique<LabeledSlider>("Delay", LabeledSlider::Knob);
        delay->setRange(0, 127);
        delay->setDefaultValue(0);
        delay->getSlider().setTooltip("LFO Delay\n\nTime before LFO starts after key press.\n0 = Immediate\n127 = Long delay\n\nTip: Add delay for natural vibrato that fades in.");
        delay->getSlider().onValueChange = [this]() {
            visualizer->setDelay((float)delay->getValue());
        };
        addAndMakeVisible(delay.get());

        rate = std::make_unique<LabeledSlider>("Rate", LabeledSlider::Knob);
        rate->setRange(0, 127);
        rate->setDefaultValue(64);
        rate->getSlider().setTooltip("LFO Speed\n\nControls oscillation speed.\nSlow = Gentle sweeps\nFast = Vibrato/tremolo\nVery fast = FM-like effects\n\nTip: 4-8 Hz is typical vibrato range.");
        rate->getSlider().onValueChange = [this]() {
            visualizer->setRate((float)rate->getValue());
        };
        addAndMakeVisible(rate.get());
        
        // Sync toggle (Vecoven 4.x)
        syncSwitch = std::make_unique<CompactSwitch>("Sync");
        syncSwitch->setOptions({"Off", "On"});
        syncSwitch->setTooltip("MIDI Clock Sync (Vecoven)\n\nWhen On, LFO syncs to MIDI clock.\nRequires Vecoven 4.x firmware.\n\nTip: Great for tempo-locked effects!");
        addAndMakeVisible(syncSwitch.get());
    }

    void setEditor(MKS70EditorAudioProcessorEditor* ed, const juce::String& prefix) {
        syncSwitch->setEditor(ed);
        syncSwitch->setParameterName(prefix + "_LFO_SYNC");
        
        delay->setEditor(ed);
        delay->setParameterName(prefix + "_LFO_DELAY");
        
        rate->setEditor(ed);
        rate->setParameterName(prefix + "_LFO_RATE");
    }

    void resized() override {
        using namespace DesignSystem;
        section->setBounds(getLocalBounds());
        auto bounds = section->getContentBounds();
        
        constexpr int knobW = 65;   // RealisticKnob width
        constexpr int knobH = 80;   // RealisticKnob height
        constexpr int sliderH = 85; // LabeledSlider height
        constexpr int switchH = 36; // CompactSwitch height
        
        // Top: Visualizer (MUCH BIGGER - takes more space)
        visualizer->setBounds(bounds.removeFromTop(60));
        bounds.removeFromTop(6);
        
        // Row 1: Wave knob, Rate slider, Delay slider (lower position)
        auto knobRow = bounds.removeFromTop(sliderH);
        int colW = knobRow.getWidth() / 3;
        waveKnob->setBounds(knobRow.removeFromLeft(colW).withSizeKeepingCentre(knobW, knobH));
        rate->setBounds(knobRow.removeFromLeft(colW).withHeight(sliderH));
        delay->setBounds(knobRow.withHeight(sliderH));
        
        bounds.removeFromTop(4);
        
        // Row 2: Sync toggle (centered, compact)
        auto syncRow = bounds.removeFromTop(switchH);
        syncSwitch->setBounds(syncRow.withSizeKeepingCentre(100, switchH));
    }

    int getIdealHeight() const {
        using namespace DesignSystem;
        return Size::SectionHeaderHeight + 60 + 6 + 85 + 4 + 36 + 8;  // Bigger visualizer
    }
    
    LabeledSlider& getRateSlider() { return *rate; }
    LabeledSlider& getDelaySlider() { return *delay; }
    RealisticKnob& getWaveKnob() { return *waveKnob; }
    CompactSwitch& getSyncSwitch() { return *syncSwitch; }
    
    // Methods to update the visualizer from external callbacks
    void updateVisualizerWave(int w) { visualizer->setWaveform(w); }
    void updateVisualizerRate(float r) { visualizer->setRate(r); }
    void updateVisualizerDelay(float d) { visualizer->setDelay(d); }
    
    // Firmware version support - Sync is V4.x only
    void setFirmwareVersion(FirmwareVersion version) {
        bool showSync = (version == FirmwareVersion::Vecoven4 &&
                         MKS70Mapping::LFO1_SYNC_SYSEX != MKS70Mapping::PARAM_NOT_AVAILABLE);
        syncSwitch->setVisible(showSync);
        resized();
    }

private:
    std::unique_ptr<SectionComponent> section;
    std::unique_ptr<LFOVisualizer> visualizer;
    std::unique_ptr<RealisticKnob> waveKnob;
    std::unique_ptr<CompactSwitch> syncSwitch;
    std::unique_ptr<LabeledSlider> delay, rate;
};
