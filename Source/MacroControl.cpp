/*
  ==============================================================================

    MacroControl.cpp
    Macro control system implementation

  ==============================================================================
*/

#include "MacroControl.h"

//==============================================================================
// MacroControl Implementation

MacroControl::MacroControl()
    : name("Macro"), value(0), midiCC(-1), midiChannel(0)
{
}

MacroControl::~MacroControl()
{
}

void MacroControl::setName(const juce::String& newName)
{
    name = newName;
}

void MacroControl::setValue(int newValue)
{
    value = juce::jlimit(0, 127, newValue);
}

void MacroControl::addMapping(const ParameterMapping& mapping)
{
    // Check if parameter already mapped
    for (auto& m : mappings)
    {
        if (m.paramId == mapping.paramId)
        {
            // Update existing mapping
            m = mapping;
            DBG("MacroControl: Updated mapping for " + mapping.paramId);
            return;
        }
    }
    
    // Add new mapping
    mappings.push_back(mapping);
    DBG("MacroControl: Added mapping for " + mapping.paramId);
}

void MacroControl::removeMapping(const juce::String& paramId)
{
    mappings.erase(
        std::remove_if(mappings.begin(), mappings.end(),
            [&paramId](const ParameterMapping& m) { return m.paramId == paramId; }),
        mappings.end());
    
    DBG("MacroControl: Removed mapping for " + paramId);
}

void MacroControl::clearMappings()
{
    mappings.clear();
    DBG("MacroControl: Cleared all mappings");
}

float MacroControl::getMappedValue(const juce::String& paramId) const
{
    for (const auto& m : mappings)
    {
        if (m.paramId == paramId)
        {
            // Normalize macro value to 0.0-1.0
            float normalized = value / 127.0f;
            
            // Invert if needed
            if (m.inverted)
                normalized = 1.0f - normalized;
            
            // Apply curve
            float curved = applyCurve(normalized, m.curve);
            
            // Map to parameter range
            return m.minValue + curved * (m.maxValue - m.minValue);
        }
    }
    
    return 0.0f;  // Not mapped
}

bool MacroControl::hasMapping(const juce::String& paramId) const
{
    for (const auto& m : mappings)
    {
        if (m.paramId == paramId)
            return true;
    }
    return false;
}

void MacroControl::setMidiCC(int cc)
{
    midiCC = juce::jlimit(-1, 127, cc);
}

void MacroControl::setMidiChannel(int channel)
{
    midiChannel = juce::jlimit(0, 15, channel);
}

float MacroControl::applyCurve(float normalized, MorphEngine::MorphCurve curve) const
{
    switch (curve)
    {
        case MorphEngine::CURVE_LINEAR:
            return normalized;
            
        case MorphEngine::CURVE_EXPONENTIAL:
            return normalized * normalized;
            
        case MorphEngine::CURVE_LOGARITHMIC:
            return std::sqrt(normalized);
            
        case MorphEngine::CURVE_SCURVE:
        {
            // Smooth S-curve using cubic easing
            if (normalized < 0.5f)
                return 2.0f * normalized * normalized;
            else
            {
                float t = 1.0f - normalized;
                return 1.0f - 2.0f * t * t;
            }
        }
            
        case MorphEngine::CURVE_STEPPED:
        {
            // 4 steps
            if (normalized < 0.25f) return 0.0f;
            if (normalized < 0.5f) return 0.33f;
            if (normalized < 0.75f) return 0.67f;
            return 1.0f;
        }
            
        default:
            return normalized;
    }
}

std::unique_ptr<juce::XmlElement> MacroControl::toXml() const
{
    auto xml = std::make_unique<juce::XmlElement>("Macro");
    
    xml->setAttribute("name", name);
    xml->setAttribute("value", value);
    xml->setAttribute("midiCC", midiCC);
    xml->setAttribute("midiChannel", midiChannel);
    
    // Mappings
    for (const auto& m : mappings)
    {
        auto mappingXml = xml->createNewChildElement("Mapping");
        mappingXml->setAttribute("paramId", m.paramId);
        mappingXml->setAttribute("minValue", static_cast<double>(m.minValue));
        mappingXml->setAttribute("maxValue", static_cast<double>(m.maxValue));
        mappingXml->setAttribute("curve", static_cast<int>(m.curve));
        mappingXml->setAttribute("inverted", m.inverted);
    }
    
    return xml;
}

void MacroControl::fromXml(const juce::XmlElement& xml)
{
    name = xml.getStringAttribute("name", "Macro");
    value = xml.getIntAttribute("value", 0);
    midiCC = xml.getIntAttribute("midiCC", -1);
    midiChannel = xml.getIntAttribute("midiChannel", 0);
    
    mappings.clear();
    
    for (auto* mappingXml : xml.getChildIterator())
    {
        if (mappingXml->hasTagName("Mapping"))
        {
            ParameterMapping m;
            m.paramId = mappingXml->getStringAttribute("paramId");
            m.minValue = static_cast<float>(mappingXml->getDoubleAttribute("minValue"));
            m.maxValue = static_cast<float>(mappingXml->getDoubleAttribute("maxValue"));
            m.curve = static_cast<MorphEngine::MorphCurve>(mappingXml->getIntAttribute("curve", 0));
            m.inverted = mappingXml->getBoolAttribute("inverted", false);
            mappings.push_back(m);
        }
    }
}

//==============================================================================
// MacroManager Implementation

MacroManager::MacroManager()
    : learningMacro(-1)
{
    // Initialize default macro names
    macros[0].setName("Brightness");
    macros[1].setName("Attack");
    macros[2].setName("Vibrato");
    macros[3].setName("Movement");
    macros[4].setName("Filter Sweep");
    macros[5].setName("Resonance");
    macros[6].setName("Envelope");
    macros[7].setName("Dynamics");
    
    // Default MIDI CC assignments
    for (int i = 0; i < NUM_MACROS; i++)
    {
        macros[i].setMidiCC(74 + i);  // CC 74-81
        macros[i].setMidiChannel(0);
    }
}

MacroManager::~MacroManager()
{
}

MacroControl& MacroManager::getMacro(int index)
{
    jassert(index >= 0 && index < NUM_MACROS);
    return macros[index];
}

const MacroControl& MacroManager::getMacro(int index) const
{
    jassert(index >= 0 && index < NUM_MACROS);
    return macros[index];
}

void MacroManager::setMacroValue(int index, int value)
{
    if (index >= 0 && index < NUM_MACROS)
    {
        macros[index].setValue(value);
        notifyValueChanged(index, value);
    }
}

int MacroManager::getMacroValue(int index) const
{
    if (index >= 0 && index < NUM_MACROS)
        return macros[index].getValue();
    return 0;
}

std::vector<std::pair<juce::String, float>> MacroManager::getMacroMappedValues(int index) const
{
    std::vector<std::pair<juce::String, float>> result;
    
    if (index >= 0 && index < NUM_MACROS)
    {
        const auto& macro = macros[index];
        for (const auto& mapping : macro.getMappings())
        {
            float value = macro.getMappedValue(mapping.paramId);
            result.push_back({mapping.paramId, value});
        }
    }
    
    return result;
}

void MacroManager::saveMacroPreset(const juce::String& name)
{
    // TODO: Implement preset saving to file
    DBG("MacroManager: Save preset '" + name + "' - not implemented yet");
}

void MacroManager::loadMacroPreset(const juce::String& name)
{
    // TODO: Implement preset loading from file
    DBG("MacroManager: Load preset '" + name + "' - not implemented yet");
}

std::vector<juce::String> MacroManager::getPresetNames() const
{
    // TODO: Scan preset directory
    return {"Default", "Bright Pad", "Moving Strings", "Dynamic Lead"};
}

void MacroManager::startLearn(int macroIndex)
{
    if (macroIndex >= 0 && macroIndex < NUM_MACROS)
    {
        learningMacro = macroIndex;
        learnedParams.clear();
        DBG("MacroManager: Started learning for macro " + juce::String(macroIndex));
    }
}

void MacroManager::addLearnedParameter(const juce::String& paramId, float currentValue)
{
    if (learningMacro >= 0 && learningMacro < NUM_MACROS)
    {
        // Add mapping with current value as midpoint
        MacroControl::ParameterMapping mapping;
        mapping.paramId = paramId;
        mapping.minValue = juce::jmax(0.0f, currentValue - 30.0f);  // -30 from current
        mapping.maxValue = juce::jmin(127.0f, currentValue + 30.0f);  // +30 from current
        mapping.curve = MorphEngine::CURVE_LINEAR;
        mapping.inverted = false;
        
        macros[learningMacro].addMapping(mapping);
        learnedParams.push_back(paramId);
        
        notifyMappingsChanged(learningMacro);
        
        DBG("MacroManager: Learned parameter " + paramId + " for macro " + juce::String(learningMacro));
    }
}

void MacroManager::stopLearn()
{
    if (learningMacro >= 0)
    {
        DBG("MacroManager: Stopped learning, " + juce::String(learnedParams.size()) + " parameters learned");
        learningMacro = -1;
        learnedParams.clear();
    }
}

int MacroManager::findMacroByCC(int cc, int channel) const
{
    for (int i = 0; i < NUM_MACROS; i++)
    {
        if (macros[i].getMidiCC() == cc && macros[i].getMidiChannel() == channel)
            return i;
    }
    return -1;
}

void MacroManager::handleMidiCC(int cc, int channel, int value)
{
    int macroIndex = findMacroByCC(cc, channel);
    if (macroIndex >= 0)
    {
        setMacroValue(macroIndex, value);
        DBG("MacroManager: MIDI CC " + juce::String(cc) + " -> Macro " + juce::String(macroIndex) + " = " + juce::String(value));
    }
}

void MacroManager::loadTemplate(int macroIndex, const juce::String& templateName)
{
    if (macroIndex < 0 || macroIndex >= NUM_MACROS)
        return;
    
    if (templateName == "Brightness")
        applyBrightnessTemplate(macroIndex);
    else if (templateName == "Filter Sweep")
        applyFilterSweepTemplate(macroIndex);
    else if (templateName == "Envelope")
        applyEnvelopeTemplate(macroIndex);
    else if (templateName == "Vibrato")
        applyVibratoTemplate(macroIndex);
    else if (templateName == "Movement")
        applyMovementTemplate(macroIndex);
    else if (templateName == "Attack")
        applyAttackTemplate(macroIndex);
    else if (templateName == "Decay")
        applyDecayTemplate(macroIndex);
    else if (templateName == "Resonance")
        applyResonanceTemplate(macroIndex);
    
    notifyMappingsChanged(macroIndex);
}

std::vector<juce::String> MacroManager::getTemplateNames() const
{
    return {
        "Brightness",
        "Filter Sweep",
        "Envelope",
        "Vibrato",
        "Movement",
        "Attack",
        "Decay",
        "Resonance"
    };
}

void MacroManager::applyBrightnessTemplate(int macroIndex)
{
    auto& macro = macros[macroIndex];
    macro.clearMappings();
    macro.setName("Brightness");
    
    macro.addMapping(MacroControl::ParameterMapping("vcfCutoff", 30.0f, 127.0f, MorphEngine::CURVE_LINEAR, false));
    macro.addMapping(MacroControl::ParameterMapping("vcfResonance", 0.0f, 80.0f, MorphEngine::CURVE_EXPONENTIAL, false));
    macro.addMapping(MacroControl::ParameterMapping("mixerENVMod", 60.0f, 0.0f, MorphEngine::CURVE_LINEAR, true));
    macro.addMapping(MacroControl::ParameterMapping("dco1LFOMod", 0.0f, 30.0f, MorphEngine::CURVE_LINEAR, false));
}

void MacroManager::applyFilterSweepTemplate(int macroIndex)
{
    auto& macro = macros[macroIndex];
    macro.clearMappings();
    macro.setName("Filter Sweep");
    
    macro.addMapping(MacroControl::ParameterMapping("vcfCutoff", 0.0f, 127.0f, MorphEngine::CURVE_EXPONENTIAL, false));
    macro.addMapping(MacroControl::ParameterMapping("vcfENVMod", 80.0f, 20.0f, MorphEngine::CURVE_LINEAR, true));
    macro.addMapping(MacroControl::ParameterMapping("vcfResonance", 20.0f, 70.0f, MorphEngine::CURVE_SCURVE, false));
}

void MacroManager::applyEnvelopeTemplate(int macroIndex)
{
    auto& macro = macros[macroIndex];
    macro.clearMappings();
    macro.setName("Envelope");
    
    macro.addMapping(MacroControl::ParameterMapping("env1Attack", 0.0f, 80.0f, MorphEngine::CURVE_EXPONENTIAL, false));
    macro.addMapping(MacroControl::ParameterMapping("env1Decay", 20.0f, 100.0f, MorphEngine::CURVE_LINEAR, false));
    macro.addMapping(MacroControl::ParameterMapping("env1Release", 10.0f, 90.0f, MorphEngine::CURVE_LINEAR, false));
}

void MacroManager::applyVibratoTemplate(int macroIndex)
{
    auto& macro = macros[macroIndex];
    macro.clearMappings();
    macro.setName("Vibrato");
    
    macro.addMapping(MacroControl::ParameterMapping("dco1LFOMod", 0.0f, 60.0f, MorphEngine::CURVE_LINEAR, false));
    macro.addMapping(MacroControl::ParameterMapping("dco2LFOMod", 0.0f, 60.0f, MorphEngine::CURVE_LINEAR, false));
    macro.addMapping(MacroControl::ParameterMapping("lfoRate", 30.0f, 70.0f, MorphEngine::CURVE_LINEAR, false));
}

void MacroManager::applyMovementTemplate(int macroIndex)
{
    auto& macro = macros[macroIndex];
    macro.clearMappings();
    macro.setName("Movement");
    
    macro.addMapping(MacroControl::ParameterMapping("vcfLFOMod", 0.0f, 80.0f, MorphEngine::CURVE_LINEAR, false));
    macro.addMapping(MacroControl::ParameterMapping("lfoRate", 10.0f, 50.0f, MorphEngine::CURVE_LINEAR, false));
    macro.addMapping(MacroControl::ParameterMapping("dco2FineTune", 45.0f, 55.0f, MorphEngine::CURVE_LINEAR, false));
}

void MacroManager::applyAttackTemplate(int macroIndex)
{
    auto& macro = macros[macroIndex];
    macro.clearMappings();
    macro.setName("Attack");
    
    macro.addMapping(MacroControl::ParameterMapping("env1Attack", 0.0f, 100.0f, MorphEngine::CURVE_EXPONENTIAL, false));
    macro.addMapping(MacroControl::ParameterMapping("env2Attack", 0.0f, 100.0f, MorphEngine::CURVE_EXPONENTIAL, false));
}

void MacroManager::applyDecayTemplate(int macroIndex)
{
    auto& macro = macros[macroIndex];
    macro.clearMappings();
    macro.setName("Decay");
    
    macro.addMapping(MacroControl::ParameterMapping("env1Decay", 0.0f, 100.0f, MorphEngine::CURVE_LINEAR, false));
    macro.addMapping(MacroControl::ParameterMapping("env1Release", 0.0f, 100.0f, MorphEngine::CURVE_LINEAR, false));
}

void MacroManager::applyResonanceTemplate(int macroIndex)
{
    auto& macro = macros[macroIndex];
    macro.clearMappings();
    macro.setName("Resonance");
    
    macro.addMapping(MacroControl::ParameterMapping("vcfResonance", 0.0f, 100.0f, MorphEngine::CURVE_EXPONENTIAL, false));
    macro.addMapping(MacroControl::ParameterMapping("vcfKeyFollow", 0.0f, 100.0f, MorphEngine::CURVE_LINEAR, false));
}

std::unique_ptr<juce::XmlElement> MacroManager::toXml() const
{
    auto xml = std::make_unique<juce::XmlElement>("MacroManager");
    
    for (int i = 0; i < NUM_MACROS; i++)
    {
        xml->addChildElement(macros[i].toXml().release());
    }
    
    return xml;
}

void MacroManager::fromXml(const juce::XmlElement& xml)
{
    int i = 0;
    for (auto* macroXml : xml.getChildIterator())
    {
        if (i >= NUM_MACROS) break;
        
        if (macroXml->hasTagName("Macro"))
        {
            macros[i].fromXml(*macroXml);
            i++;
        }
    }
}

void MacroManager::addListener(Listener* listener)
{
    listeners.add(listener);
}

void MacroManager::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

void MacroManager::notifyValueChanged(int macroIndex, int newValue)
{
    listeners.call([macroIndex, newValue](Listener& l) {
        l.macroValueChanged(macroIndex, newValue);
    });
}

void MacroManager::notifyMappingsChanged(int macroIndex)
{
    listeners.call([macroIndex](Listener& l) {
        l.macroMappingsChanged(macroIndex);
    });
}
