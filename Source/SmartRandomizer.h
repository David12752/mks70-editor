#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "ToneParameters.h"
#include "PatchParameters.h"

/**
 * SmartRandomizer.h
 * Intelligent randomization with categories, amounts, and parameter locking
 * Like Serum's dice function but smarter
 * 
 * Part of Improvement #7: Smart Randomizer
 */

namespace Randomizer
{

//==============================================================================
/**
 * Randomization categories
 */
enum class Category
{
    All,
    Oscillators,
    Filter,
    Envelopes,
    LFO,
    Effects,
    Modulation
};

//==============================================================================
/**
 * Parameter lock state
 * Locked parameters won't be randomized
 */
struct ParameterLock
{
    // DCO Locks
    bool dco1Range = false;
    bool dco1Waveform = false;
    bool dco1Tune = false;
    bool dco1LFO = false;
    bool dco1ENV = false;
    
    bool dco2Range = false;
    bool dco2Waveform = false;
    bool dco2Tune = false;
    bool dco2Detune = false;
    bool dco2XMod = false;
    
    bool dcoMix = false;
    
    // VCF Locks
    bool vcfCutoff = false;
    bool vcfResonance = false;
    bool vcfEnvAmount = false;
    bool vcfLFO = false;
    bool vcfKeyFollow = false;
    
    // VCA Locks
    bool vcaLevel = false;
    
    // Envelope Locks
    bool env1ADSR = false;
    bool env2ADSR = false;
    
    // LFO Locks
    bool lfoWaveform = false;
    bool lfoRate = false;
    bool lfoDelay = false;
    
    // Patch Locks
    bool voiceMode = false;
    bool portamento = false;
    bool bendRange = false;
    
    void lockAll()
    {
        dco1Range = dco1Waveform = dco1Tune = dco1LFO = dco1ENV = true;
        dco2Range = dco2Waveform = dco2Tune = dco2Detune = dco2XMod = true;
        dcoMix = true;
        vcfCutoff = vcfResonance = vcfEnvAmount = vcfLFO = vcfKeyFollow = true;
        vcaLevel = true;
        env1ADSR = env2ADSR = true;
        lfoWaveform = lfoRate = lfoDelay = true;
        voiceMode = portamento = bendRange = true;
    }
    
    void unlockAll()
    {
        dco1Range = dco1Waveform = dco1Tune = dco1LFO = dco1ENV = false;
        dco2Range = dco2Waveform = dco2Tune = dco2Detune = dco2XMod = false;
        dcoMix = false;
        vcfCutoff = vcfResonance = vcfEnvAmount = vcfLFO = vcfKeyFollow = false;
        vcaLevel = false;
        env1ADSR = env2ADSR = false;
        lfoWaveform = lfoRate = lfoDelay = false;
        voiceMode = portamento = bendRange = false;
    }
    
    void lockCategory(Category cat, bool locked)
    {
        switch (cat)
        {
            case Category::Oscillators:
                dco1Range = dco1Waveform = dco1Tune = dco1LFO = dco1ENV = locked;
                dco2Range = dco2Waveform = dco2Tune = dco2Detune = dco2XMod = locked;
                dcoMix = locked;
                break;
            case Category::Filter:
                vcfCutoff = vcfResonance = vcfEnvAmount = vcfLFO = vcfKeyFollow = locked;
                break;
            case Category::Envelopes:
                env1ADSR = env2ADSR = locked;
                break;
            case Category::LFO:
                lfoWaveform = lfoRate = lfoDelay = locked;
                break;
            case Category::Modulation:
                dco1LFO = dco1ENV = dco2XMod = vcfLFO = vcfEnvAmount = locked;
                break;
            default:
                break;
        }
    }
};

//==============================================================================
/**
 * Randomization preset - predefined sound character templates
 */
struct RandomPreset
{
    juce::String name;
    juce::String description;
    Category focusCategory = Category::All;
    float intensity = 0.5f;
    ParameterLock locks;
    
    // Bias settings (0-1, 0.5 = neutral)
    float brightnessBias = 0.5f;      // Dark <-> Bright
    float movementBias = 0.5f;        // Static <-> Moving
    float complexityBias = 0.5f;      // Simple <-> Complex
    float attackBias = 0.5f;          // Soft <-> Percussive
};

//==============================================================================
/**
 * Smart Randomizer Engine
 */
class RandomizerEngine
{
public:
    RandomizerEngine()
    {
        initializePresets();
    }
    
    //==========================================================================
    // Core Randomization
    //==========================================================================
    
    void randomizeToneParams(ToneParams& params, float intensity = 0.5f)
    {
        if (intensity <= 0.0f)
            return;
        
        // DCO 1
        if (!locks.dco1Range)
            params.dco1Range = randomChoice({0, 1, 2}, intensity);
        if (!locks.dco1Waveform)
            params.dco1Waveform = randomChoice({0, 1, 2, 3, 4}, intensity);
        if (!locks.dco1Tune)
            params.dco1Tune = randomBipolar(intensity, 12); // +/- 12 semitones
        if (!locks.dco1LFO)
            params.dco1LFO = randomUnipolar(intensity, 127);
        if (!locks.dco1ENV)
            params.dco1ENV = randomUnipolar(intensity, 127);
        
        // DCO 2
        if (!locks.dco2Range)
            params.dco2Range = randomChoice({0, 1, 2}, intensity);
        if (!locks.dco2Waveform)
            params.dco2Waveform = randomChoice({0, 1, 2, 3, 4, 5}, intensity);
        if (!locks.dco2Tune)
            params.dco2Tune = randomBipolar(intensity, 12);
        if (!locks.dco2Detune)
            params.dco2FineTune = randomBipolar(intensity, 63);
        if (!locks.dco2XMod)
            params.dco2XMod = randomUnipolar(intensity, 127);
        
        // Mixer
        if (!locks.dcoMix)
            params.mixerDCO1 = randomUnipolar(intensity, 127);
        if (!locks.dcoMix)
            params.mixerDCO2 = randomUnipolar(intensity, 127);
        if (!locks.dcoMix)
            params.mixerEnvMod = randomUnipolar(intensity, 127);
        
        // VCF
        if (!locks.vcfCutoff)
            params.vcfCutoff = randomUnipolarBiased(intensity, 127, currentPreset.brightnessBias);
        if (!locks.vcfResonance)
            params.vcfResonance = randomUnipolar(intensity * 0.7f, 127); // Less extreme resonance
        if (!locks.vcfEnvAmount)
            params.vcfEnvMod = randomUnipolar(intensity, 127);
        if (!locks.vcfLFO)
            params.vcfLFO = randomUnipolar(intensity, 127);
        if (!locks.vcfKeyFollow)
            params.vcfKeyFollow = randomChoice({0, 1, 2, 3}, intensity);
        
        // VCA
        if (!locks.vcaLevel)
            params.vcaLevel = 100 + randomInt(0, static_cast<int>(27 * intensity)); // 100-127
        
        // ENV 1
        if (!locks.env1ADSR)
        {
            params.env1Attack = randomUnipolarBiased(intensity, 127, 1.0f - currentPreset.attackBias);
            params.env1Decay = randomUnipolar(intensity, 127);
            params.env1Sustain = 64 + randomInt(-32, static_cast<int>(32 * intensity));
            params.env1Release = randomUnipolar(intensity, 127);
        }
        
        // ENV 2
        if (!locks.env2ADSR)
        {
            params.env2Attack = randomUnipolarBiased(intensity, 127, 1.0f - currentPreset.attackBias);
            params.env2Decay = randomUnipolar(intensity, 127);
            params.env2Sustain = 64 + randomInt(-32, static_cast<int>(32 * intensity));
            params.env2Release = randomUnipolar(intensity, 127);
        }
        
        // LFO
        if (!locks.lfoWaveform)
            params.lfoWaveform = randomChoice({0, 1, 2, 3, 4, 5}, intensity);
        if (!locks.lfoRate)
            params.lfoRate = randomUnipolarBiased(intensity, 127, currentPreset.movementBias);
        if (!locks.lfoDelay)
            params.lfoDelay = randomUnipolar(intensity, 127);
    }
    
    void randomizePatchParams(PatchParameters& params, float intensity = 0.5f)
    {
        if (!locks.voiceMode)
            params.keyMode = randomChoice({0, 1, 2}, intensity);
        if (!locks.portamento)
        {
            params.portamento = randomUnipolar(intensity * 0.5f, 127);
            params.portamentoMode = randomChoice({0, 1}, intensity);
        }
        if (!locks.bendRange)
            params.bendRange = randomChoice({0, 2, 3, 5, 7, 12}, intensity);
    }
    
    //==========================================================================
    // Category-based Randomization
    //==========================================================================
    
    void randomizeCategory(ToneParams& params, Category category, float intensity = 0.5f)
    {
        ParameterLock savedLocks = locks;
        locks.lockAll();
        locks.lockCategory(category, false);
        randomizeToneParams(params, intensity);
        locks = savedLocks;
    }
    
    //==========================================================================
    // Preset-based Randomization
    //==========================================================================
    
    void setPreset(const juce::String& presetName)
    {
        for (const auto& preset : presets)
        {
            if (preset.name == presetName)
            {
                currentPreset = preset;
                locks = preset.locks;
                return;
            }
        }
    }
    
    void randomizeWithPreset(ToneParams& params, const juce::String& presetName, float intensity = 0.5f)
    {
        setPreset(presetName);
        randomizeToneParams(params, intensity * currentPreset.intensity);
    }
    
    const std::vector<RandomPreset>& getPresets() const { return presets; }
    
    //==========================================================================
    // Lock Management
    //==========================================================================
    
    ParameterLock& getLocks() { return locks; }
    const ParameterLock& getLocks() const { return locks; }
    
    void lockParameter(const juce::String& paramId, bool locked)
    {
        // Map parameter IDs to lock flags
        if (paramId == "dco1Range") locks.dco1Range = locked;
        else if (paramId == "dco1Waveform") locks.dco1Waveform = locked;
        else if (paramId == "dco1Tune") locks.dco1Tune = locked;
        else if (paramId == "vcfCutoff") locks.vcfCutoff = locked;
        else if (paramId == "vcfResonance") locks.vcfResonance = locked;
        // ... etc
    }
    
private:
    void initializePresets()
    {
        // Bass preset
        {
            RandomPreset preset;
            preset.name = "Bass";
            preset.description = "Deep bass sounds with low filter";
            preset.brightnessBias = 0.3f;
            preset.attackBias = 0.7f;
            preset.movementBias = 0.3f;
            preset.complexityBias = 0.4f;
            preset.intensity = 0.6f;
            preset.locks.dco1Range = true; // Keep oscillator ranges for bass
            preset.locks.dco2Range = true;
            presets.push_back(preset);
        }
        
        // Pad preset
        {
            RandomPreset preset;
            preset.name = "Pad";
            preset.description = "Soft, evolving pad sounds";
            preset.brightnessBias = 0.5f;
            preset.attackBias = 0.2f;
            preset.movementBias = 0.6f;
            preset.complexityBias = 0.5f;
            preset.intensity = 0.5f;
            presets.push_back(preset);
        }
        
        // Lead preset
        {
            RandomPreset preset;
            preset.name = "Lead";
            preset.description = "Bright, cutting lead sounds";
            preset.brightnessBias = 0.7f;
            preset.attackBias = 0.6f;
            preset.movementBias = 0.5f;
            preset.complexityBias = 0.5f;
            preset.intensity = 0.6f;
            presets.push_back(preset);
        }
        
        // Keys preset
        {
            RandomPreset preset;
            preset.name = "Keys";
            preset.description = "Keyboard and piano-like sounds";
            preset.brightnessBias = 0.5f;
            preset.attackBias = 0.8f;
            preset.movementBias = 0.2f;
            preset.complexityBias = 0.3f;
            preset.intensity = 0.4f;
            preset.locks.lfoRate = true;
            preset.locks.lfoWaveform = true;
            presets.push_back(preset);
        }
        
        // FX preset
        {
            RandomPreset preset;
            preset.name = "FX";
            preset.description = "Sound effects and weird sounds";
            preset.brightnessBias = 0.5f;
            preset.attackBias = 0.5f;
            preset.movementBias = 0.8f;
            preset.complexityBias = 0.9f;
            preset.intensity = 0.9f;
            presets.push_back(preset);
        }
        
        // Subtle preset
        {
            RandomPreset preset;
            preset.name = "Subtle";
            preset.description = "Small variations on current sound";
            preset.intensity = 0.2f;
            presets.push_back(preset);
        }
        
        // Wild preset
        {
            RandomPreset preset;
            preset.name = "Wild";
            preset.description = "Completely random, anything goes";
            preset.intensity = 1.0f;
            presets.push_back(preset);
        }
    }
    
    //==========================================================================
    // Random Helpers
    //==========================================================================
    
    int randomInt(int min, int max)
    {
        if (min >= max) return min;
        return min + random.nextInt(max - min + 1);
    }
    
    int randomUnipolar(float intensity, int max)
    {
        int range = static_cast<int>(max * intensity);
        return randomInt(0, range);
    }
    
    int randomUnipolarBiased(float intensity, int max, float bias)
    {
        // Bias: 0 = favor low values, 0.5 = neutral, 1 = favor high values
        int value = randomUnipolar(intensity, max);
        
        // Apply bias
        float normalized = static_cast<float>(value) / max;
        normalized = std::pow(normalized, 2.0f * (1.0f - bias));
        
        return static_cast<int>(normalized * max);
    }
    
    int randomBipolar(float intensity, int range)
    {
        int r = static_cast<int>(range * intensity);
        return randomInt(-r, r);
    }
    
    int randomChoice(std::initializer_list<int> choices, float intensity)
    {
        std::vector<int> choiceVec(choices);
        
        if (intensity < 1.0f && choiceVec.size() > 1)
        {
            // With lower intensity, prefer earlier choices (usually more "normal")
            int maxIndex = 1 + static_cast<int>((choiceVec.size() - 1) * intensity);
            return choiceVec[randomInt(0, maxIndex - 1)];
        }
        
        return choiceVec[randomInt(0, static_cast<int>(choiceVec.size()) - 1)];
    }
    
    juce::Random random;
    ParameterLock locks;
    RandomPreset currentPreset;
    std::vector<RandomPreset> presets;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RandomizerEngine)
};

//==============================================================================
/**
 * Randomizer Panel UI
 */
class RandomizerPanel : public juce::Component
{
public:
    RandomizerPanel(RandomizerEngine& engine) : randEngine(engine)
    {
        // Intensity slider
        intensitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
        intensitySlider.setRange(0.0, 1.0, 0.01);
        intensitySlider.setValue(0.5);
        intensitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        addAndMakeVisible(intensitySlider);
        
        intensityLabel.setText("Amount:", juce::dontSendNotification);
        intensityLabel.setFont(DS::Font::label);
        intensityLabel.setColour(juce::Label::textColourId, DS::Color::textMuted);
        addAndMakeVisible(intensityLabel);
        
        // Preset selector
        presetSelector.addItem("All", 1);
        int idx = 2;
        for (const auto& preset : engine.getPresets())
        {
            presetSelector.addItem(preset.name, idx++);
        }
        presetSelector.setSelectedId(1);
        addAndMakeVisible(presetSelector);
        
        // Category selector
        categorySelector.addItem("All", 1);
        categorySelector.addItem("Oscillators", 2);
        categorySelector.addItem("Filter", 3);
        categorySelector.addItem("Envelopes", 4);
        categorySelector.addItem("LFO", 5);
        categorySelector.addItem("Modulation", 6);
        categorySelector.setSelectedId(1);
        addAndMakeVisible(categorySelector);
        
        // Randomize button (dice)
        randomizeButton.setButtonText(juce::String::charToString(0x1F3B2)); // Dice emoji
        randomizeButton.onClick = [this] { performRandomize(); };
        addAndMakeVisible(randomizeButton);
        
        // Lock All / Unlock All
        lockAllButton.setButtonText("Lock All");
        lockAllButton.onClick = [this]
        {
            randEngine.getLocks().lockAll();
            if (onLocksChanged)
                onLocksChanged();
        };
        addAndMakeVisible(lockAllButton);
        
        unlockAllButton.setButtonText("Unlock All");
        unlockAllButton.onClick = [this]
        {
            randEngine.getLocks().unlockAll();
            if (onLocksChanged)
                onLocksChanged();
        };
        addAndMakeVisible(unlockAllButton);
    }
    
    std::function<void()> onRandomize;
    std::function<void()> onLocksChanged;
    
    float getIntensity() const { return static_cast<float>(intensitySlider.getValue()); }
    
    juce::String getSelectedPreset() const
    {
        int id = presetSelector.getSelectedId();
        if (id == 1) return "";
        return presetSelector.getText();
    }
    
    Category getSelectedCategory() const
    {
        return static_cast<Category>(categorySelector.getSelectedId() - 1);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::sm);
        
        auto row1 = bounds.removeFromTop(30);
        intensityLabel.setBounds(row1.removeFromLeft(60));
        intensitySlider.setBounds(row1.removeFromLeft(150));
        row1.removeFromLeft(10);
        randomizeButton.setBounds(row1.removeFromLeft(40));
        
        bounds.removeFromTop(DS::Space::sm);
        
        auto row2 = bounds.removeFromTop(24);
        presetSelector.setBounds(row2.removeFromLeft(100));
        row2.removeFromLeft(10);
        categorySelector.setBounds(row2.removeFromLeft(100));
        
        bounds.removeFromTop(DS::Space::sm);
        
        auto row3 = bounds.removeFromTop(24);
        lockAllButton.setBounds(row3.removeFromLeft(70));
        row3.removeFromLeft(10);
        unlockAllButton.setBounds(row3.removeFromLeft(80));
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading);
        g.drawText("RANDOMIZER", bounds.reduced(DS::Space::sm).removeFromTop(20), 
                   juce::Justification::topRight);
    }
    
private:
    void performRandomize()
    {
        if (onRandomize)
            onRandomize();
    }
    
    RandomizerEngine& randEngine;
    
    juce::Slider intensitySlider;
    juce::Label intensityLabel;
    juce::ComboBox presetSelector;
    juce::ComboBox categorySelector;
    juce::TextButton randomizeButton;
    juce::TextButton lockAllButton;
    juce::TextButton unlockAllButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RandomizerPanel)
};

//==============================================================================
/**
 * Dice Button - Simple randomize button for toolbar
 */
class DiceButton : public juce::Component
{
public:
    DiceButton()
    {
        setRepaintsOnMouseActivity(true);
    }
    
    std::function<void()> onClick;
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        bool isHovered = isMouseOver();
        bool isDown = isMouseButtonDown();
        
        // Background
        if (isDown)
            g.setColour(DS::Color::accent.withAlpha(0.8f));
        else if (isHovered)
            g.setColour(DS::Color::accent.withAlpha(0.3f));
        else
            g.setColour(DS::Color::surfaceAlt);
        
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        // Border
        g.setColour(isHovered ? DS::Color::accent : DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        // Dice icon (simple representation)
        auto iconBounds = bounds.reduced(6);
        
        g.setColour(isDown ? juce::Colours::white : DS::Color::text);
        g.fillRoundedRectangle(iconBounds, 3);
        
        // Dots
        g.setColour(isDown ? DS::Color::accent : DS::Color::surface);
        float dotSize = 4.0f;
        float cx = iconBounds.getCentreX();
        float cy = iconBounds.getCentreY();
        
        g.fillEllipse(cx - dotSize/2, cy - dotSize/2, dotSize, dotSize); // Center
        g.fillEllipse(iconBounds.getX() + 3, iconBounds.getY() + 3, dotSize, dotSize); // Top-left
        g.fillEllipse(iconBounds.getRight() - dotSize - 3, iconBounds.getBottom() - dotSize - 3, dotSize, dotSize); // Bottom-right
    }
    
    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClick)
            onClick();
        repaint();
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiceButton)
};

} // namespace Randomizer
