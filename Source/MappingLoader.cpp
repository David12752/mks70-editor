/*
  ==============================================================================

    MappingLoader.cpp
    Implementation of parameter mapping loader

  ==============================================================================
*/

#include "MappingLoader.h"

//==============================================================================
MappingLoader::MappingLoader()
    : loaded(false)
{
}

MappingLoader::~MappingLoader()
{
}

//==============================================================================
// Loading
//==============================================================================
bool MappingLoader::loadFromFile(const juce::File& jsonFile)
{
    clear();
    
    if (!jsonFile.existsAsFile())
    {
        lastError = "File does not exist: " + jsonFile.getFullPathName();
        return false;
    }
    
    juce::String content = jsonFile.loadFileAsString();
    return loadFromString(content);
}

bool MappingLoader::loadFromString(const juce::String& jsonString)
{
    clear();
    
    juce::var json;
    auto result = juce::JSON::parse(jsonString, json);
    
    if (result.failed())
    {
        lastError = "JSON parse error: " + result.getErrorMessage();
        return false;
    }
    
    return parseJson(json);
}

void MappingLoader::clear()
{
    loaded = false;
    lastError.clear();
    deviceName.clear();
    firmwareVersion.clear();
    description.clear();
    
    parameterMap.clear();
    sysexMap.clear();
    nrpnMap.clear();
    
    ToneParams.clear();
    patchParameters.clear();
    arpParameters.clear();
}

//==============================================================================
// Parameter Access
//==============================================================================
ParameterInfo MappingLoader::getParameter(const juce::String& name) const
{
    auto it = parameterMap.find(name);
    if (it != parameterMap.end())
        return it->second;
    
    return ParameterInfo(); // Return empty parameter
}

bool MappingLoader::hasParameter(const juce::String& name) const
{
    return parameterMap.find(name) != parameterMap.end();
}

std::vector<ParameterInfo> MappingLoader::getAllToneParameters() const
{
    return ToneParams;
}

std::vector<ParameterInfo> MappingLoader::getAllPatchParameters() const
{
    return patchParameters;
}

std::vector<ParameterInfo> MappingLoader::getAllArpParameters() const
{
    return arpParameters;
}

std::vector<ParameterInfo> MappingLoader::getParametersByCategory(const juce::String& category) const
{
    std::vector<ParameterInfo> result;
    
    for (const auto& pair : parameterMap)
    {
        if (pair.second.category == category)
            result.push_back(pair.second);
    }
    
    return result;
}

std::vector<ParameterGroup> MappingLoader::getParameterGroups(const juce::String& type) const
{
    std::map<juce::String, ParameterGroup> groups;
    
    // Select parameters by type
    const std::vector<ParameterInfo>* params = nullptr;
    if (type == "tone")
        params = &ToneParams;
    else if (type == "patch")
        params = &patchParameters;
    else if (type == "arp")
        params = &arpParameters;
    
    if (params == nullptr)
        return std::vector<ParameterGroup>();
    
    // Group by category
    for (const auto& param : *params)
    {
        if (groups.find(param.category) == groups.end())
            groups[param.category] = ParameterGroup(param.category, param.category);
        
        groups[param.category].parameters.push_back(param);
    }
    
    // Convert to vector
    std::vector<ParameterGroup> result;
    for (const auto& pair : groups)
        result.push_back(pair.second);
    
    return result;
}

//==============================================================================
// MIDI Mapping Lookup
//==============================================================================
ParameterInfo MappingLoader::getParameterBySysEx(int offset) const
{
    auto it = sysexMap.find(offset);
    if (it != sysexMap.end())
        return it->second;
    
    return ParameterInfo();
}

ParameterInfo MappingLoader::getParameterByNRPN(int msb, int lsb) const
{
    auto key = std::make_pair(msb, lsb);
    auto it = nrpnMap.find(key);
    if (it != nrpnMap.end())
        return it->second;
    
    return ParameterInfo();
}

//==============================================================================
// Export
//==============================================================================
juce::String MappingLoader::exportToJson() const
{
    juce::var json = juce::var(new juce::DynamicObject());
    auto* obj = json.getDynamicObject();
    
    obj->setProperty("device", deviceName);
    obj->setProperty("firmware", firmwareVersion);
    obj->setProperty("description", description);
    
    // Export tone parameters
    juce::Array<juce::var> toneArray;
    for (const auto& param : ToneParams)
    {
        auto* paramObj = new juce::DynamicObject();
        paramObj->setProperty("name", param.name);
        paramObj->setProperty("displayName", param.displayName);
        paramObj->setProperty("category", param.category);
        paramObj->setProperty("sysexOffset", param.sysexOffset);
        paramObj->setProperty("nrpnMSB", param.nrpnMSB);
        paramObj->setProperty("nrpnLSB", param.nrpnLSB);
        paramObj->setProperty("min", param.minValue);
        paramObj->setProperty("max", param.maxValue);
        paramObj->setProperty("default", param.defaultValue);
        
        if (!param.unit.isEmpty())
            paramObj->setProperty("unit", param.unit);
        
        if (!param.valueLabels.isEmpty())
        {
            juce::Array<juce::var> labels;
            for (const auto& label : param.valueLabels)
                labels.add(label);
            paramObj->setProperty("values", labels);
        }
        
        toneArray.add(juce::var(paramObj));
    }
    obj->setProperty("ToneParams", toneArray);
    
    // Export patch parameters
    juce::Array<juce::var> patchArray;
    for (const auto& param : patchParameters)
    {
        auto* paramObj = new juce::DynamicObject();
        paramObj->setProperty("name", param.name);
        paramObj->setProperty("displayName", param.displayName);
        paramObj->setProperty("sysexOffset", param.sysexOffset);
        paramObj->setProperty("min", param.minValue);
        paramObj->setProperty("max", param.maxValue);
        paramObj->setProperty("default", param.defaultValue);
        patchArray.add(juce::var(paramObj));
    }
    obj->setProperty("patchParameters", patchArray);
    
    // Export arp parameters
    juce::Array<juce::var> arpArray;
    for (const auto& param : arpParameters)
    {
        auto* paramObj = new juce::DynamicObject();
        paramObj->setProperty("name", param.name);
        paramObj->setProperty("displayName", param.displayName);
        paramObj->setProperty("nrpnMSB", param.nrpnMSB);
        paramObj->setProperty("nrpnLSB", param.nrpnLSB);
        paramObj->setProperty("min", param.minValue);
        paramObj->setProperty("max", param.maxValue);
        paramObj->setProperty("default", param.defaultValue);
        arpArray.add(juce::var(paramObj));
    }
    obj->setProperty("arpParameters", arpArray);
    
    return juce::JSON::toString(json, true);
}

bool MappingLoader::saveToFile(const juce::File& jsonFile) const
{
    juce::String json = exportToJson();
    return jsonFile.replaceWithText(json);
}

//==============================================================================
// Parsing Methods
//==============================================================================
bool MappingLoader::parseJson(const juce::var& json)
{
    if (!json.isObject())
    {
        lastError = "Root element must be an object";
        return false;
    }
    
    if (!parseDeviceInfo(json))
        return false;
    
    if (!parseParameters(json))
        return false;
    
    loaded = true;
    return true;
}

bool MappingLoader::parseDeviceInfo(const juce::var& json)
{
    deviceName = json.getProperty("device", "Unknown").toString();
    firmwareVersion = json.getProperty("firmware", "Unknown").toString();
    description = json.getProperty("description", "").toString();
    
    return true;
}

bool MappingLoader::parseParameters(const juce::var& json)
{
    // Parse tone parameters
    if (json.hasProperty("ToneParams"))
    {
        auto* toneArray = json.getProperty("ToneParams", juce::var()).getArray();
        if (toneArray != nullptr)
        {
            for (const auto& paramVar : *toneArray)
            {
                ParameterInfo param = parseParameter(paramVar);
                param.type = "tone";
                
                ToneParams.push_back(param);
                parameterMap[param.name] = param;
                
                if (param.sysexOffset >= 0)
                    sysexMap[param.sysexOffset] = param;
                
                if (param.nrpnMSB >= 0 && param.nrpnLSB >= 0)
                    nrpnMap[std::make_pair(param.nrpnMSB, param.nrpnLSB)] = param;
            }
        }
    }
    
    // Parse patch parameters
    if (json.hasProperty("patchParameters"))
    {
        auto* patchArray = json.getProperty("patchParameters", juce::var()).getArray();
        if (patchArray != nullptr)
        {
            for (const auto& paramVar : *patchArray)
            {
                ParameterInfo param = parseParameter(paramVar);
                param.type = "patch";
                
                patchParameters.push_back(param);
                parameterMap[param.name] = param;
                
                if (param.sysexOffset >= 0)
                    sysexMap[param.sysexOffset] = param;
            }
        }
    }
    
    // Parse arp parameters
    if (json.hasProperty("arpParameters"))
    {
        auto* arpArray = json.getProperty("arpParameters", juce::var()).getArray();
        if (arpArray != nullptr)
        {
            for (const auto& paramVar : *arpArray)
            {
                ParameterInfo param = parseParameter(paramVar);
                param.type = "arp";
                param.isVecoven = true;
                
                arpParameters.push_back(param);
                parameterMap[param.name] = param;
                
                if (param.nrpnMSB >= 0 && param.nrpnLSB >= 0)
                    nrpnMap[std::make_pair(param.nrpnMSB, param.nrpnLSB)] = param;
            }
        }
    }
    
    return true;
}

ParameterInfo MappingLoader::parseParameter(const juce::var& paramJson)
{
    ParameterInfo param;
    
    param.name = paramJson.getProperty("name", "").toString();
    param.displayName = paramJson.getProperty("displayName", param.name).toString();
    param.category = paramJson.getProperty("category", "General").toString();
    
    param.sysexOffset = paramJson.getProperty("sysexOffset", -1);
    param.nrpnMSB = paramJson.getProperty("nrpnMSB", -1);
    param.nrpnLSB = paramJson.getProperty("nrpnLSB", -1);
    
    param.minValue = paramJson.getProperty("min", 0);
    param.maxValue = paramJson.getProperty("max", 127);
    param.defaultValue = paramJson.getProperty("default", 0);
    
    param.unit = paramJson.getProperty("unit", "").toString();
    param.firmware = paramJson.getProperty("firmware", "").toString();
    param.isVecoven = param.firmware.contains("Vecoven");
    
    // Parse value labels
    if (paramJson.hasProperty("values"))
    {
        auto* valuesArray = paramJson.getProperty("values", juce::var()).getArray();
        if (valuesArray != nullptr)
        {
            for (const auto& val : *valuesArray)
                param.valueLabels.add(val.toString());
            
            param.isDiscrete = true;
        }
    }
    
    return param;
}
