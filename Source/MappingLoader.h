/*
  ==============================================================================

    MappingLoader.h
    Loads parameter mapping from JSON configuration files

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ParameterInfo.h"
#include <vector>
#include <map>

//==============================================================================
// MappingLoader Class
// Loads and manages parameter mapping from external JSON files
//==============================================================================
class MappingLoader
{
public:
    MappingLoader();
    ~MappingLoader();
    
    //==============================================================================
    // Loading
    //==============================================================================
    bool loadFromFile(const juce::File& jsonFile);
    bool loadFromString(const juce::String& jsonString);
    void clear();
    
    //==============================================================================
    // Device Information
    //==============================================================================
    juce::String getDeviceName() const { return deviceName; }
    juce::String getFirmwareVersion() const { return firmwareVersion; }
    juce::String getDescription() const { return description; }
    
    //==============================================================================
    // Parameter Access
    //==============================================================================
    ParameterInfo getParameter(const juce::String& name) const;
    bool hasParameter(const juce::String& name) const;
    
    std::vector<ParameterInfo> getAllToneParameters() const;
    std::vector<ParameterInfo> getAllPatchParameters() const;
    std::vector<ParameterInfo> getAllArpParameters() const;
    
    std::vector<ParameterInfo> getParametersByCategory(const juce::String& category) const;
    std::vector<ParameterGroup> getParameterGroups(const juce::String& type) const;
    
    //==============================================================================
    // MIDI Mapping Lookup
    //==============================================================================
    ParameterInfo getParameterBySysEx(int offset) const;
    ParameterInfo getParameterByNRPN(int msb, int lsb) const;
    
    //==============================================================================
    // Validation
    //==============================================================================
    bool isLoaded() const { return loaded; }
    juce::String getLastError() const { return lastError; }
    
    //==============================================================================
    // Export
    //==============================================================================
    juce::String exportToJson() const;
    bool saveToFile(const juce::File& jsonFile) const;
    
private:
    //==============================================================================
    // Parsing Methods
    //==============================================================================
    bool parseJson(const juce::var& json);
    bool parseDeviceInfo(const juce::var& json);
    bool parseParameters(const juce::var& json);
    ParameterInfo parseParameter(const juce::var& paramJson);
    
    //==============================================================================
    // Internal Data
    //==============================================================================
    bool loaded;
    juce::String lastError;
    
    juce::String deviceName;
    juce::String firmwareVersion;
    juce::String description;
    
    std::map<juce::String, ParameterInfo> parameterMap;
    std::map<int, ParameterInfo> sysexMap;              // SysEx offset -> Parameter
    std::map<std::pair<int, int>, ParameterInfo> nrpnMap;  // (MSB, LSB) -> Parameter
    
    std::vector<ParameterInfo> ToneParams;
    std::vector<ParameterInfo> patchParameters;
    std::vector<ParameterInfo> arpParameters;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingLoader)
};
