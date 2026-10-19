/*
  ==============================================================================

    MacroControl.h
    Macro control system for mapping single control to multiple parameters

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MorphEngine.h"  // For MorphCurve enum
#include <array>
#include <vector>

//==============================================================================
/**
 * MacroControl - Single macro with multiple parameter mappings
 */
class MacroControl
{
public:
    struct ParameterMapping
    {
        juce::String paramId;
        float minValue;      // Value when macro = 0
        float maxValue;      // Value when macro = 127
        MorphEngine::MorphCurve curve = MorphEngine::CURVE_LINEAR;
        bool inverted = false;  // Reverse direction
        
        ParameterMapping() = default;
        ParameterMapping(const juce::String& id, float min, float max, 
                        MorphEngine::MorphCurve c = MorphEngine::CURVE_LINEAR, bool inv = false)
            : paramId(id), minValue(min), maxValue(max), curve(c), inverted(inv) {}
    };
    
    MacroControl();
    ~MacroControl();
    
    // Name and value
    void setName(const juce::String& newName);
    juce::String getName() const { return name; }
    
    void setValue(int newValue);
    int getValue() const { return value; }
    
    // Parameter mappings
    void addMapping(const ParameterMapping& mapping);
    void removeMapping(const juce::String& paramId);
    void clearMappings();
    
    const std::vector<ParameterMapping>& getMappings() const { return mappings; }
    std::vector<ParameterMapping>& getMappings() { return mappings; }
    
    // Get mapped value for specific parameter
    float getMappedValue(const juce::String& paramId) const;
    bool hasMapping(const juce::String& paramId) const;
    
    // MIDI
    void setMidiCC(int cc);
    int getMidiCC() const { return midiCC; }
    
    void setMidiChannel(int channel);
    int getMidiChannel() const { return midiChannel; }
    
    // Serialization
    std::unique_ptr<juce::XmlElement> toXml() const;
    void fromXml(const juce::XmlElement& xml);

private:
    juce::String name;
    int value = 0;  // 0-127
    std::vector<ParameterMapping> mappings;
    int midiCC = -1;      // -1 = not assigned
    int midiChannel = 0;  // 0-15
    
    // Apply curve to normalized value (0.0-1.0)
    float applyCurve(float normalized, MorphEngine::MorphCurve curve) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroControl)
};

//==============================================================================
/**
 * MacroManager - Manages 8 macro controls with presets and learn mode
 */
class MacroManager
{
public:
    static constexpr int NUM_MACROS = 8;
    
    MacroManager();
    ~MacroManager();
    
    // Macro access
    MacroControl& getMacro(int index);
    const MacroControl& getMacro(int index) const;
    
    void setMacroValue(int index, int value);
    int getMacroValue(int index) const;
    
    // Get all mapped values for a specific macro
    std::vector<std::pair<juce::String, float>> getMacroMappedValues(int index) const;
    
    // Presets
    void saveMacroPreset(const juce::String& name);
    void loadMacroPreset(const juce::String& name);
    std::vector<juce::String> getPresetNames() const;
    
    // Learn mode
    void startLearn(int macroIndex);
    void addLearnedParameter(const juce::String& paramId, float currentValue);
    void stopLearn();
    bool isLearning() const { return learningMacro >= 0; }
    int getLearningMacro() const { return learningMacro; }
    
    // MIDI CC mapping
    int findMacroByCC(int cc, int channel) const;
    void handleMidiCC(int cc, int channel, int value);
    
    // Preset templates
    void loadTemplate(int macroIndex, const juce::String& templateName);
    std::vector<juce::String> getTemplateNames() const;
    
    // Serialization
    std::unique_ptr<juce::XmlElement> toXml() const;
    void fromXml(const juce::XmlElement& xml);
    
    // Listener for value changes
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void macroValueChanged(int macroIndex, int newValue) = 0;
        virtual void macroMappingsChanged(int macroIndex) = 0;
    };
    
    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    std::array<MacroControl, NUM_MACROS> macros;
    int learningMacro = -1;
    std::vector<juce::String> learnedParams;
    
    juce::ListenerList<Listener> listeners;
    
    // Default templates
    void initializeDefaultTemplates();
    void applyBrightnessTemplate(int macroIndex);
    void applyFilterSweepTemplate(int macroIndex);
    void applyEnvelopeTemplate(int macroIndex);
    void applyVibratoTemplate(int macroIndex);
    void applyMovementTemplate(int macroIndex);
    void applyAttackTemplate(int macroIndex);
    void applyDecayTemplate(int macroIndex);
    void applyResonanceTemplate(int macroIndex);
    
    void notifyValueChanged(int macroIndex, int newValue);
    void notifyMappingsChanged(int macroIndex);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroManager)
};
