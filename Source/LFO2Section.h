#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "SectionComponent.h"
#include "LabeledSlider.h"
#include "LabeledComboBox.h"
#include "RealisticKnob.h"
#include "CompactSwitch.h"

class MKS70EditorAudioProcessorEditor;

//==============================================================================
// LFO2Visualizer - Animated LFO waveform display for LFO2
//==============================================================================
class LFO2Visualizer : public juce::Component, private juce::Timer {
public:
    LFO2Visualizer() {
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
        
        // Calculate how many cycles based on rate (offset phase for visual distinction)
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
        
        // Glow effect - use different color for LFO2 (cyan/teal)
        juce::Colour waveColor = juce::Colour(0xff00d4dd); // Cyan-teal for LFO2
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
// LFO2Section - Second LFO (Vecoven X3 upgrade)
//==============================================================================
class LFO2Section : public juce::Component {
public:
    LFO2Section() {
        section = std::make_unique<SectionComponent>("LFO 2", juce::Colour(0xff00d4dd)); // Cyan-teal
        addAndMakeVisible(section.get());
        
        // Visualizer
        visualizer = std::make_unique<LFO2Visualizer>();
        addAndMakeVisible(visualizer.get());

        // LFO Wave Knob with waveform icons
        waveKnob = std::make_unique<RealisticKnob>("Wave", RealisticKnob::KnobType::Waveform);
        waveKnob->setRange(0, 3, 1);
        waveKnob->setItems({"Tri", "Saw", "Sqr", "S&H"});
        waveKnob->setTooltip("LFO2 Waveform\n\nTri = Smooth vibrato\nSaw = Ramp sweeps\nSqr = Abrupt switching\nS&H = Random steps\n\n(Vecoven X3 Feature)");
        waveKnob->onChange = [this]() {
            visualizer->setWaveform(static_cast<int>(waveKnob->getValue()));
        };
        addAndMakeVisible(waveKnob.get());

        delay = std::make_unique<LabeledSlider>("Delay", LabeledSlider::Knob);
        delay->setRange(0, 127);
        delay->setDefaultValue(0);
        delay->getSlider().setTooltip("LFO2 Delay\n\nTime before LFO2 starts after key press.\n0 = Immediate\n127 = Long delay\n\n(Vecoven X3 Feature)");
        delay->getSlider().onValueChange = [this]() {
            visualizer->setDelay((float)delay->getValue());
        };
        addAndMakeVisible(delay.get());

        rate = std::make_unique<LabeledSlider>("Rate", LabeledSlider::Knob);
        rate->setRange(0, 127);
        rate->setDefaultValue(64);
        rate->getSlider().setTooltip("LFO2 Speed\n\nControls oscillation speed.\nUse for independent modulation vs LFO1.\n\n(Vecoven X3 Feature)");
        rate->getSlider().onValueChange = [this]() {
            visualizer->setRate((float)rate->getValue());
        };
        addAndMakeVisible(rate.get());
        
        // LFO1 Depth - cross-modulation
        lfo1Depth = std::make_unique<LabeledSlider>("LFO1→", LabeledSlider::Knob);
        lfo1Depth->setRange(0, 127);
        lfo1Depth->setDefaultValue(0);
        lfo1Depth->getSlider().setTooltip("LFO1 Depth on LFO2\n\nHow much LFO1 modulates LFO2's rate.\n0 = No modulation\n127 = Maximum cross-mod\n\n(Vecoven X3 Feature)");
        addAndMakeVisible(lfo1Depth.get());
        
        // Sync toggle (Vecoven 4.x)
        syncSwitch = std::make_unique<CompactSwitch>("Sync");
        syncSwitch->setOptions({"Off", "On"});
        syncSwitch->setTooltip("MIDI Clock Sync (Vecoven 4.x)\n\nWhen On, LFO2 syncs to MIDI clock.\nRequires Vecoven 4.x firmware.");
        addAndMakeVisible(syncSwitch.get());
    }

    void setEditor(MKS70EditorAudioProcessorEditor* ed, const juce::String& prefix) {
        syncSwitch->setEditor(ed);
        syncSwitch->setParameterName(prefix + "_LFO2_SYNC");
        
        delay->setEditor(ed);
        delay->setParameterName(prefix + "_LFO2_DELAY");
        
        rate->setEditor(ed);
        rate->setParameterName(prefix + "_LFO2_RATE");
        
        lfo1Depth->setEditor(ed);
        lfo1Depth->setParameterName(prefix + "_LFO2_LFO1_DEPTH");
    }

    void resized() override {
        using namespace DesignSystem;
        section->setBounds(getLocalBounds());
        auto bounds = section->getContentBounds();
        
        constexpr int knobW = 60;
        constexpr int knobH = 75;
        constexpr int sliderH = 80;
        constexpr int switchH = 40;
        
        // Top: Visualizer (compact)
        visualizer->setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(2);
        
        // Row 1: Wave knob, Rate slider, Delay slider
        auto knobRow = bounds.removeFromTop(sliderH);
        int colW = knobRow.getWidth() / 3;
        waveKnob->setBounds(knobRow.removeFromLeft(colW).withSizeKeepingCentre(knobW, knobH));
        rate->setBounds(knobRow.removeFromLeft(colW).withHeight(sliderH));
        delay->setBounds(knobRow.withHeight(sliderH));
        
        bounds.removeFromTop(2);
        
        // Row 2: LFO1 Depth and Sync toggle
        auto controlRow = bounds.removeFromTop(sliderH);
        lfo1Depth->setBounds(controlRow.removeFromLeft(controlRow.getWidth() / 2).withHeight(sliderH));
        syncSwitch->setBounds(controlRow.withSizeKeepingCentre(knobW * 2, switchH));
    }

    int getIdealHeight() const {
        using namespace DesignSystem;
        return Size::SectionHeaderHeight + 28 + 2 + 80 + 2 + 80 + 6;
    }
    
    LabeledSlider& getRateSlider() { return *rate; }
    LabeledSlider& getDelaySlider() { return *delay; }
    LabeledSlider& getLFO1DepthSlider() { return *lfo1Depth; }
    RealisticKnob& getWaveKnob() { return *waveKnob; }
    CompactSwitch& getSyncSwitch() { return *syncSwitch; }
    
    // Methods to update the visualizer from external callbacks
    void updateVisualizerWave(int w) { visualizer->setWaveform(w); }
    void updateVisualizerRate(float r) { visualizer->setRate(r); }
    void updateVisualizerDelay(float d) { visualizer->setDelay(d); }

private:
    std::unique_ptr<SectionComponent> section;
    std::unique_ptr<LFO2Visualizer> visualizer;
    std::unique_ptr<RealisticKnob> waveKnob;
    std::unique_ptr<CompactSwitch> syncSwitch;
    std::unique_ptr<LabeledSlider> delay, rate, lfo1Depth;
};
