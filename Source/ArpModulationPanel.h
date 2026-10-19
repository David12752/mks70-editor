/*
  ==============================================================================
    ArpModulationPanel.h - Advanced Modulation Panel for Arpeggiator Tab
    
    Provides comprehensive modulation controls for creative sound design:
    - LFO-based modulations (rate, depth, shapes)
    - Step sequencer for parameter modulation
    - Velocity/Note modulation mappings
    - Randomization controls
    
    Integrated into ArpTab for one-stop creative workflow.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "GlowingSection.h"
#include "StyledSlider.h"
#include "StyledComboBox.h"

//==============================================================================
// Modulation target parameters available in MKS-70
//==============================================================================
namespace ArpModTargets {
    // Filter targets
    inline juce::StringArray getFilterTargets() {
        return {"VCF Cutoff", "VCF Resonance", "VCF Env Depth", "VCF LFO Depth"};
    }
    
    // Oscillator targets
    inline juce::StringArray getOscTargets() {
        return {"DCO1 Tune", "DCO2 Tune", "DCO1 PW", "DCO2 PW", "Cross Mod"};
    }
    
    // Amplitude/Mix targets
    inline juce::StringArray getMixTargets() {
        return {"VCA Level", "DCO1 Level", "DCO2 Level", "Balance"};
    }
    
    // LFO targets
    inline juce::StringArray getLFOTargets() {
        return {"LFO Rate", "LFO Delay", "LFO->DCO", "LFO->VCF"};
    }
    
    // Performance targets
    inline juce::StringArray getPerfTargets() {
        return {"Portamento", "Chorus", "Detune"};
    }
    
    // All targets combined
    inline juce::StringArray getAllTargets() {
        juce::StringArray all;
        all.addArray(getFilterTargets());
        all.addArray(getOscTargets());
        all.addArray(getMixTargets());
        all.addArray(getLFOTargets());
        all.addArray(getPerfTargets());
        return all;
    }
}

//==============================================================================
// Modulation Slot - Single modulation routing
//==============================================================================
class ModSlot : public juce::Component {
public:
    ModSlot(int slotNumber, juce::Colour accentColor) 
        : slotNum(slotNumber), accent(accentColor) 
    {
        // Enable toggle
        enableBtn.setButtonText(juce::String(slotNumber));
        enableBtn.setClickingTogglesState(true);
        enableBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        enableBtn.setColour(juce::TextButton::buttonOnColourId, accent);
        enableBtn.setTooltip("Enable/disable modulation slot " + juce::String(slotNumber));
        addAndMakeVisible(enableBtn);
        
        // Source combo
        sourceCombo.addItem("LFO 1", 1);
        sourceCombo.addItem("LFO 2", 2);
        sourceCombo.addItem("ENV 1", 3);
        sourceCombo.addItem("ENV 2", 4);
        sourceCombo.addItem("Velocity", 5);
        sourceCombo.addItem("Note#", 6);
        sourceCombo.addItem("Random", 7);
        sourceCombo.addItem("Step Seq", 8);
        sourceCombo.setSelectedId(1);
        sourceCombo.setTooltip("Modulation source");
        addAndMakeVisible(sourceCombo);
        
        // Target combo
        auto targets = ArpModTargets::getAllTargets();
        for (int i = 0; i < targets.size(); ++i)
            targetCombo.addItem(targets[i], i + 1);
        targetCombo.setSelectedId(1);
        targetCombo.setTooltip("Modulation target parameter");
        addAndMakeVisible(targetCombo);
        
        // Amount slider
        amountSlider.setRange(-100.0, 100.0, 1.0);
        amountSlider.setValue(0.0);
        amountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        amountSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 20);
        amountSlider.setColour(juce::Slider::thumbColourId, accent);
        amountSlider.setColour(juce::Slider::trackColourId, accent.withAlpha(0.3f));
        amountSlider.setTooltip("Modulation amount (-100% to +100%)");
        addAndMakeVisible(amountSlider);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(2);
        const int gap = 4;
        
        enableBtn.setBounds(bounds.removeFromLeft(28));
        bounds.removeFromLeft(gap);
        
        sourceCombo.setBounds(bounds.removeFromLeft(80));
        bounds.removeFromLeft(gap);
        
        targetCombo.setBounds(bounds.removeFromLeft(100));
        bounds.removeFromLeft(gap);
        
        amountSlider.setBounds(bounds);
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Subtle background when enabled
        if (enableBtn.getToggleState()) {
            g.setColour(accent.withAlpha(0.1f));
            g.fillRoundedRectangle(bounds, 4.0f);
        }
        
        g.setColour(DS::Color::border().withAlpha(0.5f));
        g.drawRoundedRectangle(bounds.reduced(1), 4.0f, 0.5f);
    }
    
    // Getters
    bool isEnabled() const { return enableBtn.getToggleState(); }
    int getSourceId() const { return sourceCombo.getSelectedId(); }
    int getTargetId() const { return targetCombo.getSelectedId(); }
    double getAmount() const { return amountSlider.getValue(); }
    
    // Setters
    void setEnabled(bool enabled) { enableBtn.setToggleState(enabled, juce::dontSendNotification); }
    void setSource(int id) { sourceCombo.setSelectedId(id, juce::dontSendNotification); }
    void setTarget(int id) { targetCombo.setSelectedId(id, juce::dontSendNotification); }
    void setAmount(double amt) { amountSlider.setValue(amt, juce::dontSendNotification); }
    
    std::function<void()> onChange;
    
private:
    int slotNum;
    juce::Colour accent;
    juce::TextButton enableBtn;
    juce::ComboBox sourceCombo;
    juce::ComboBox targetCombo;
    juce::Slider amountSlider;
};

//==============================================================================
// Step Sequencer Display - 16-step modulation sequencer
//==============================================================================
class StepSequencer : public juce::Component {
public:
    static constexpr int NUM_STEPS = 16;
    
    StepSequencer() {
        for (int i = 0; i < NUM_STEPS; ++i)
            stepValues[i] = 64; // Center value
        currentStep = 0;
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(DS::Color::bgInput());
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Border
        g.setColour(DS::Color::border());
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        
        // Draw steps
        float stepW = bounds.getWidth() / NUM_STEPS;
        float centerY = bounds.getCentreY();
        float maxH = bounds.getHeight() * 0.45f;
        
        for (int i = 0; i < NUM_STEPS; ++i) {
            float x = bounds.getX() + i * stepW;
            float barX = x + 2;
            float barW = stepW - 4;
            
            // Step value bar
            float value = (stepValues[i] - 64) / 64.0f;  // -1 to +1
            float barH = std::abs(value) * maxH;
            float barY = value >= 0 ? centerY - barH : centerY;
            
            juce::Colour barCol = (i == currentStep) ? DS::Color::Accent::cyan() 
                                                     : DS::Color::Accent::primary();
            
            // Highlight current step
            if (i == currentStep) {
                g.setColour(barCol.withAlpha(0.2f));
                g.fillRect(x, bounds.getY(), stepW, bounds.getHeight());
            }
            
            // Draw bar
            g.setColour(barCol.withAlpha(0.8f));
            g.fillRoundedRectangle(barX, barY, barW, barH, 2.0f);
            
            // Step number
            g.setColour(DS::Color::textMuted());
            g.setFont(8.0f);
            g.drawText(juce::String(i + 1), juce::Rectangle<float>(x, bounds.getBottom() - 12, stepW, 10),
                      juce::Justification::centred);
        }
        
        // Center line
        g.setColour(DS::Color::textMuted().withAlpha(0.5f));
        g.drawHorizontalLine((int)centerY, bounds.getX(), bounds.getRight());
    }
    
    void mouseDown(const juce::MouseEvent& e) override {
        updateStepFromMouse(e);
    }
    
    void mouseDrag(const juce::MouseEvent& e) override {
        updateStepFromMouse(e);
    }
    
    void setStep(int index, int value) {
        if (index >= 0 && index < NUM_STEPS) {
            stepValues[index] = juce::jlimit(0, 127, value);
            repaint();
            if (onStepChanged)
                onStepChanged(index, stepValues[index]);
        }
    }
    
    int getStep(int index) const {
        return (index >= 0 && index < NUM_STEPS) ? stepValues[index] : 64;
    }
    
    void setCurrentStep(int step) {
        currentStep = step % NUM_STEPS;
        repaint();
    }
    
    void randomize() {
        juce::Random rng;
        for (int i = 0; i < NUM_STEPS; ++i)
            stepValues[i] = rng.nextInt(128);
        repaint();
    }
    
    void clear() {
        for (int i = 0; i < NUM_STEPS; ++i)
            stepValues[i] = 64;
        repaint();
    }
    
    std::function<void(int, int)> onStepChanged;
    
private:
    int stepValues[NUM_STEPS];
    int currentStep;
    
    void updateStepFromMouse(const juce::MouseEvent& e) {
        auto bounds = getLocalBounds().reduced(2);
        float stepW = bounds.getWidth() / (float)NUM_STEPS;
        
        int stepIndex = (int)((e.x - bounds.getX()) / stepW);
        stepIndex = juce::jlimit(0, NUM_STEPS - 1, stepIndex);
        
        float relY = 1.0f - (e.y - bounds.getY()) / (float)bounds.getHeight();
        int value = (int)(relY * 127.0f);
        value = juce::jlimit(0, 127, value);
        
        setStep(stepIndex, value);
    }
};

//==============================================================================
// LFO Shape Display - Visual feedback for LFO
//==============================================================================
class LFOShapeDisplay : public juce::Component {
public:
    enum Shape { Sine, Triangle, Saw, Square, Random, SampleHold };
    
    void setShape(Shape s) { shape = s; repaint(); }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(4);
        
        // Background
        g.setColour(DS::Color::bgInput());
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Draw waveform
        g.setColour(DS::Color::Accent::cyan());
        juce::Path wavePath;
        
        float w = bounds.getWidth();
        float h = bounds.getHeight();
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float amp = h * 0.4f;
        
        wavePath.startNewSubPath(bounds.getX(), cy);
        
        for (float x = 0; x <= w; x += 2) {
            float t = x / w;
            float y = cy;
            
            switch (shape) {
                case Sine:
                    y = cy - std::sin(t * 2.0f * juce::MathConstants<float>::pi) * amp;
                    break;
                case Triangle:
                    y = cy - (1.0f - std::abs(4.0f * std::fmod(t, 1.0f) - 2.0f) - 1.0f) * amp;
                    break;
                case Saw:
                    y = cy - (2.0f * std::fmod(t, 1.0f) - 1.0f) * amp;
                    break;
                case Square:
                    y = cy - (std::fmod(t, 1.0f) < 0.5f ? 1.0f : -1.0f) * amp;
                    break;
                case Random:
                case SampleHold:
                    y = cy - (std::sin(t * 17.0f) * std::cos(t * 13.0f)) * amp;
                    break;
            }
            
            wavePath.lineTo(bounds.getX() + x, y);
        }
        
        g.strokePath(wavePath, juce::PathStrokeType(1.5f));
        
        // Border
        g.setColour(DS::Color::border());
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    }
    
private:
    Shape shape = Sine;
};

//==============================================================================
// Main Arp Modulation Panel
//==============================================================================
class ArpModulationPanel : public juce::Component {
public:
    ArpModulationPanel() {
        // === MOD MATRIX SECTION ===
        matrixSection = std::make_unique<GlowingSection>("MODULATION MATRIX - Matritsat Modulatzia",
            DS::Color::Accent::magenta());
        addAndMakeVisible(matrixSection.get());
        
        // Create 4 modulation slots
        for (int i = 0; i < 4; ++i) {
            auto* slot = new ModSlot(i + 1, DS::Color::Accent::magenta());
            modSlots.add(slot);
            addAndMakeVisible(slot);
        }
        
        // === LFO SECTION ===
        lfoSection = std::make_unique<GlowingSection>("MODULATION LFO",
            DS::Color::Accent::cyan());
        addAndMakeVisible(lfoSection.get());
        
        // LFO Rate
        lfoRate = std::make_unique<StyledSlider>("Rate", StyledSlider::Knob, 
            DS::Color::Accent::cyan());
        lfoRate->setRange(0.1, 20.0, 0.1);
        lfoRate->setValue(1.0);
        lfoRate->setTooltip("LFO rate in Hz (0.1 - 20 Hz)");
        addAndMakeVisible(lfoRate.get());
        
        // LFO Depth
        lfoDepth = std::make_unique<StyledSlider>("Depth", StyledSlider::Knob,
            DS::Color::Accent::cyan());
        lfoDepth->setRange(0, 100, 1);
        lfoDepth->setValue(50);
        lfoDepth->setTooltip("LFO modulation depth (0-100%)");
        addAndMakeVisible(lfoDepth.get());
        
        // LFO Shape
        lfoShape = std::make_unique<StyledComboBox>("Shape");
        lfoShape->addItems({"Sine", "Triangle", "Saw Up", "Saw Down", "Square", "Random"});
        lfoShape->setSelectedId(1);
        lfoShape->setTooltip("LFO waveform shape");
        lfoShape->getComboBox().onChange = [this]() {
            lfoDisplay.setShape((LFOShapeDisplay::Shape)(lfoShape->getSelectedId() - 1));
        };
        addAndMakeVisible(lfoShape.get());
        
        // LFO Display
        addAndMakeVisible(lfoDisplay);
        
        // Sync button
        lfoSync.setButtonText("SYNC");
        lfoSync.setClickingTogglesState(true);
        lfoSync.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        lfoSync.setColour(juce::TextButton::buttonOnColourId, DS::Color::Accent::warning());
        lfoSync.setTooltip("Sync LFO to arpeggiator tempo");
        addAndMakeVisible(lfoSync);
        
        // === STEP SEQUENCER SECTION ===
        stepSection = std::make_unique<GlowingSection>("STEP SEQUENCER - Sekwentzer Tza'adim",
            DS::Color::Accent::warning());
        addAndMakeVisible(stepSection.get());
        
        addAndMakeVisible(stepSeq);
        
        // Step seq controls
        stepTarget = std::make_unique<StyledComboBox>("Target");
        auto targets = ArpModTargets::getAllTargets();
        for (int i = 0; i < targets.size(); ++i)
            stepTarget->addItem(targets[i], i + 1);
        stepTarget->setSelectedId(1);
        stepTarget->setTooltip("Parameter controlled by step sequencer");
        addAndMakeVisible(stepTarget.get());
        
        stepAmount = std::make_unique<StyledSlider>("Amount", StyledSlider::Knob,
            DS::Color::Accent::warning());
        stepAmount->setRange(0, 100, 1);
        stepAmount->setValue(100);
        stepAmount->setTooltip("Step sequencer modulation amount");
        addAndMakeVisible(stepAmount.get());
        
        // Randomize button
        randomizeBtn.setButtonText("Random");
        randomizeBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::warning().withAlpha(0.3f));
        randomizeBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::warning());
        randomizeBtn.onClick = [this]() { stepSeq.randomize(); };
        addAndMakeVisible(randomizeBtn);
        
        // Clear button
        clearBtn.setButtonText("Clear");
        clearBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        clearBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        clearBtn.onClick = [this]() { stepSeq.clear(); };
        addAndMakeVisible(clearBtn);
        
        // === VELOCITY MAPPING SECTION ===
        velocitySection = std::make_unique<GlowingSection>("VELOCITY MAPPING - Mipui Velocity",
            DS::Color::Accent::success());
        addAndMakeVisible(velocitySection.get());
        
        // Velocity to Filter
        velFilter = std::make_unique<StyledSlider>("Vel>Filter", StyledSlider::Knob,
            DS::Color::Accent::success());
        velFilter->setRange(-100, 100, 1);
        velFilter->setValue(0);
        velFilter->setTooltip("Velocity to filter cutoff (negative = inverse)");
        addAndMakeVisible(velFilter.get());
        
        // Velocity to Level
        velLevel = std::make_unique<StyledSlider>("Vel>Level", StyledSlider::Knob,
            DS::Color::Accent::success());
        velLevel->setRange(0, 100, 1);
        velLevel->setValue(100);
        velLevel->setTooltip("Velocity to output level");
        addAndMakeVisible(velLevel.get());
        
        // Velocity to Attack
        velAttack = std::make_unique<StyledSlider>("Vel>Atk", StyledSlider::Knob,
            DS::Color::Accent::success());
        velAttack->setRange(-100, 100, 1);
        velAttack->setValue(0);
        velAttack->setTooltip("Velocity to envelope attack time");
        addAndMakeVisible(velAttack.get());
        
        // Velocity curve
        velCurve = std::make_unique<StyledComboBox>("Curve");
        velCurve->addItems({"Linear", "Exponential", "Logarithmic", "S-Curve", "Fixed"});
        velCurve->setSelectedId(1);
        velCurve->setTooltip("Velocity response curve");
        addAndMakeVisible(velCurve.get());
        
        // === RANDOMIZER SECTION ===
        randSection = std::make_unique<GlowingSection>("HUMANIZE - Enoshi",
            DS::Color::Accent::primary());
        addAndMakeVisible(randSection.get());
        
        // Timing randomization
        randTiming = std::make_unique<StyledSlider>("Timing", StyledSlider::Knob,
            DS::Color::Accent::primary());
        randTiming->setRange(0, 100, 1);
        randTiming->setValue(0);
        randTiming->setTooltip("Random timing variation (humanize)");
        addAndMakeVisible(randTiming.get());
        
        // Velocity randomization
        randVelocity = std::make_unique<StyledSlider>("Velocity", StyledSlider::Knob,
            DS::Color::Accent::primary());
        randVelocity->setRange(0, 100, 1);
        randVelocity->setValue(0);
        randVelocity->setTooltip("Random velocity variation");
        addAndMakeVisible(randVelocity.get());
        
        // Pitch randomization
        randPitch = std::make_unique<StyledSlider>("Pitch", StyledSlider::Knob,
            DS::Color::Accent::primary());
        randPitch->setRange(0, 100, 1);
        randPitch->setValue(0);
        randPitch->setTooltip("Random pitch variation in cents");
        addAndMakeVisible(randPitch.get());
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(DS::Space::M);
        const int gap = DS::Space::M;
        const int smallGap = DS::Space::S;
        
        // Calculate column widths - 3 columns layout
        int totalWidth = bounds.getWidth();
        int leftColW = (int)(totalWidth * 0.35f);
        int midColW = (int)(totalWidth * 0.35f);
        int rightColW = totalWidth - leftColW - midColW - gap * 2;
        
        // Left column: Mod Matrix + Step Sequencer
        auto leftCol = bounds.removeFromLeft(leftColW);
        bounds.removeFromLeft(gap);
        
        // Middle column: LFO + Velocity
        auto midCol = bounds.removeFromLeft(midColW);
        bounds.removeFromLeft(gap);
        
        // Right column: Randomizer
        auto rightCol = bounds;
        
        // === LEFT COLUMN ===
        // Mod Matrix section
        int matrixH = 140;
        matrixSection->setBounds(leftCol.removeFromTop(matrixH));
        auto matrixContent = matrixSection->getContentBounds();
        
        const int slotH = 28;
        for (auto* slot : modSlots) {
            slot->setBounds(matrixContent.removeFromTop(slotH));
            matrixContent.removeFromTop(2);
        }
        
        leftCol.removeFromTop(gap);
        
        // Step Sequencer section
        stepSection->setBounds(leftCol);
        auto stepContent = stepSection->getContentBounds();
        
        // Controls row at top
        auto stepControls = stepContent.removeFromTop(40);
        stepTarget->setBounds(stepControls.removeFromLeft(120));
        stepControls.removeFromLeft(smallGap);
        stepAmount->setBounds(stepControls.removeFromLeft(70));
        stepControls.removeFromLeft(smallGap);
        randomizeBtn.setBounds(stepControls.removeFromLeft(60).reduced(2));
        stepControls.removeFromLeft(4);
        clearBtn.setBounds(stepControls.removeFromLeft(50).reduced(2));
        
        stepContent.removeFromTop(smallGap);
        
        // Step sequencer display
        stepSeq.setBounds(stepContent);
        
        // === MIDDLE COLUMN ===
        // LFO section
        int lfoH = 160;
        lfoSection->setBounds(midCol.removeFromTop(lfoH));
        auto lfoContent = lfoSection->getContentBounds();
        
        // LFO display at top
        lfoDisplay.setBounds(lfoContent.removeFromTop(50));
        lfoContent.removeFromTop(smallGap);
        
        // Controls row
        auto lfoRow = lfoContent.removeFromTop(70);
        int knobW = (lfoRow.getWidth() - smallGap * 2) / 3;
        lfoRate->setBounds(lfoRow.removeFromLeft(knobW));
        lfoRow.removeFromLeft(smallGap);
        lfoDepth->setBounds(lfoRow.removeFromLeft(knobW));
        lfoRow.removeFromLeft(smallGap);
        
        // Shape and sync in remaining space
        auto shapeArea = lfoRow;
        lfoShape->setBounds(shapeArea.removeFromTop(36));
        shapeArea.removeFromTop(4);
        lfoSync.setBounds(shapeArea.withHeight(24));
        
        midCol.removeFromTop(gap);
        
        // Velocity section
        velocitySection->setBounds(midCol);
        auto velContent = velocitySection->getContentBounds();
        
        // Knobs row
        auto velRow = velContent.removeFromTop(70);
        knobW = (velRow.getWidth() - smallGap * 2) / 3;
        velFilter->setBounds(velRow.removeFromLeft(knobW));
        velRow.removeFromLeft(smallGap);
        velLevel->setBounds(velRow.removeFromLeft(knobW));
        velRow.removeFromLeft(smallGap);
        velAttack->setBounds(velRow);
        
        velContent.removeFromTop(smallGap);
        
        // Curve combo
        velCurve->setBounds(velContent.removeFromTop(36).reduced(4, 0));
        
        // === RIGHT COLUMN ===
        randSection->setBounds(rightCol);
        auto randContent = randSection->getContentBounds();
        
        // Knobs vertically
        int knobH = (randContent.getHeight() - smallGap * 2) / 3;
        randTiming->setBounds(randContent.removeFromTop(knobH));
        randContent.removeFromTop(smallGap);
        randVelocity->setBounds(randContent.removeFromTop(knobH));
        randContent.removeFromTop(smallGap);
        randPitch->setBounds(randContent);
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DS::Color::bg());
    }
    
    // === GETTERS ===
    StyledSlider& getLFORate() { return *lfoRate; }
    StyledSlider& getLFODepth() { return *lfoDepth; }
    StyledComboBox& getLFOShape() { return *lfoShape; }
    juce::TextButton& getLFOSync() { return lfoSync; }
    StepSequencer& getStepSequencer() { return stepSeq; }
    StyledSlider& getVelFilter() { return *velFilter; }
    StyledSlider& getVelLevel() { return *velLevel; }
    StyledSlider& getVelAttack() { return *velAttack; }
    StyledSlider& getRandTiming() { return *randTiming; }
    StyledSlider& getRandVelocity() { return *randVelocity; }
    StyledSlider& getRandPitch() { return *randPitch; }
    
private:
    // Sections
    std::unique_ptr<GlowingSection> matrixSection;
    std::unique_ptr<GlowingSection> lfoSection;
    std::unique_ptr<GlowingSection> stepSection;
    std::unique_ptr<GlowingSection> velocitySection;
    std::unique_ptr<GlowingSection> randSection;
    
    // Mod Matrix
    juce::OwnedArray<ModSlot> modSlots;
    
    // LFO
    std::unique_ptr<StyledSlider> lfoRate;
    std::unique_ptr<StyledSlider> lfoDepth;
    std::unique_ptr<StyledComboBox> lfoShape;
    juce::TextButton lfoSync;
    LFOShapeDisplay lfoDisplay;
    
    // Step Sequencer
    StepSequencer stepSeq;
    std::unique_ptr<StyledComboBox> stepTarget;
    std::unique_ptr<StyledSlider> stepAmount;
    juce::TextButton randomizeBtn;
    juce::TextButton clearBtn;
    
    // Velocity
    std::unique_ptr<StyledSlider> velFilter;
    std::unique_ptr<StyledSlider> velLevel;
    std::unique_ptr<StyledSlider> velAttack;
    std::unique_ptr<StyledComboBox> velCurve;
    
    // Randomizer
    std::unique_ptr<StyledSlider> randTiming;
    std::unique_ptr<StyledSlider> randVelocity;
    std::unique_ptr<StyledSlider> randPitch;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArpModulationPanel)
};
