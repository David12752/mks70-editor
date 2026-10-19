/*
  ==============================================================================

    ParameterInfo.h
    Data structure for parameter mapping information

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
// Parameter Information Structure
// Contains all metadata for a single parameter including MIDI mapping
//==============================================================================
struct ParameterInfo
{
    juce::String name;              // Internal name (e.g., "DCO1_Range")
    juce::String displayName;       // Display name (e.g., "DCO-1 Range")
    juce::String category;          // Category (e.g., "DCO", "VCF", "ENV")
    juce::String type;              // Type: "tone", "patch", "arp"
    
    // MIDI Mapping
    int sysexOffset;                // Offset in SysEx data stream
    int nrpnMSB;                    // NRPN parameter MSB
    int nrpnLSB;                    // NRPN parameter LSB
    
    // Value Range
    int minValue;                   // Minimum value
    int maxValue;                   // Maximum value
    int defaultValue;               // Default value
    
    // Display Options
    juce::StringArray valueLabels;  // Optional labels for discrete values
    juce::String unit;              // Unit of measurement (e.g., "Hz", "%")
    bool isDiscrete;                // True if parameter has discrete values
    
    // Firmware-specific
    juce::String firmware;          // Required firmware (e.g., "Vecoven 3.x", "Vecoven 4.x")
    bool isVecoven;                 // True if Vecoven-specific parameter
    
    // Constructor
    ParameterInfo()
        : sysexOffset(-1),
          nrpnMSB(-1),
          nrpnLSB(-1),
          minValue(0),
          maxValue(127),
          defaultValue(0),
          isDiscrete(false),
          isVecoven(false)
    {
    }
    
    // Check if parameter has value labels
    bool hasValueLabels() const
    {
        return !valueLabels.isEmpty();
    }
    
    // Get display string for a value
    juce::String getDisplayString(int value) const
    {
        if (hasValueLabels() && value >= 0 && value < valueLabels.size())
            return valueLabels[value];
        
        juce::String result = juce::String(value);
        if (unit.isNotEmpty())
            result += " " + unit;
        
        return result;
    }
    
    // Validate value is in range
    bool isValidValue(int value) const
    {
        return value >= minValue && value <= maxValue;
    }
    
    // Clamp value to valid range
    int clampValue(int value) const
    {
        return juce::jlimit(minValue, maxValue, value);
    }
};

//==============================================================================
// Parameter Group - for organizing parameters in UI
//==============================================================================
struct ParameterGroup
{
    juce::String name;              // Group name
    juce::String displayName;       // Display name
    std::vector<ParameterInfo> parameters;
    
    ParameterGroup() {}
    
    ParameterGroup(const juce::String& n, const juce::String& dn)
        : name(n), displayName(dn)
    {
    }
};
