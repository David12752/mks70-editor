/*
  ==============================================================================

    PresetManager.cpp
    Implementation of Preset/Librarian functionality

  ==============================================================================
*/

#include "PresetManager.h"
#include "MKS70_Mapping.h"

//==============================================================================
PresetManager::PresetManager()
{
    // Default library path in user's Documents folder
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    libraryPath = documentsDir.getChildFile("MKS-70 Editor").getChildFile("Library");
    
    // Create directory structure if it doesn't exist
    if (!libraryPath.exists())
        libraryPath.createDirectory();
        
    getTonePresetsFolder().createDirectory();
    getPatchPresetsFolder().createDirectory();
}

//==============================================================================
void PresetManager::setLibraryPath(const juce::File& path)
{
    libraryPath = path;
    if (!libraryPath.exists())
        libraryPath.createDirectory();
    
    getTonePresetsFolder().createDirectory();
    getPatchPresetsFolder().createDirectory();
    
    scanLibrary();
}

juce::File PresetManager::getTonePresetsFolder() const
{
    return libraryPath.getChildFile("Tones");
}

juce::File PresetManager::getPatchPresetsFolder() const
{
    return libraryPath.getChildFile("Patches");
}

//==============================================================================
void PresetManager::scanLibrary()
{
    tonePresets.clear();
    patchPresets.clear();
    
    // Scan tone presets
    auto toneFolder = getTonePresetsFolder();
    if (toneFolder.exists())
    {
        for (const auto& file : toneFolder.findChildFiles(juce::File::findFiles, false, "*.mks70tone"))
        {
            try
            {
                auto json = juce::JSON::parse(file);
                if (auto* obj = json.getDynamicObject())
                {
                    PresetLibraryEntry entry;
                    entry.id = file.getFileNameWithoutExtension();
                    entry.type = PresetType::Tone;
                    entry.metadata = PresetMetadata::fromVar(obj->getProperty("metadata"));
                    entry.file = file;
                    tonePresets.push_back(entry);
                }
            }
            catch (...) {}
        }
    }
    
    // Scan patch presets
    auto patchFolder = getPatchPresetsFolder();
    if (patchFolder.exists())
    {
        for (const auto& file : patchFolder.findChildFiles(juce::File::findFiles, false, "*.mks70patch"))
        {
            try
            {
                auto json = juce::JSON::parse(file);
                if (auto* obj = json.getDynamicObject())
                {
                    PresetLibraryEntry entry;
                    entry.id = file.getFileNameWithoutExtension();
                    entry.type = PresetType::Patch;
                    entry.metadata = PresetMetadata::fromVar(obj->getProperty("metadata"));
                    entry.file = file;
                    patchPresets.push_back(entry);
                }
            }
            catch (...) {}
        }
    }
    
    // Sort by name
    auto sortByName = [](const PresetLibraryEntry& a, const PresetLibraryEntry& b) {
        return a.metadata.name.compareIgnoreCase(b.metadata.name) < 0;
    };
    
    std::sort(tonePresets.begin(), tonePresets.end(), sortByName);
    std::sort(patchPresets.begin(), patchPresets.end(), sortByName);
    
    listeners.call(&Listener::presetLibraryChanged);
}

//==============================================================================
juce::String PresetManager::generatePresetId(const juce::String& name)
{
    // Create a safe filename from the preset name
    juce::String id = name.trim();
    
    // Replace invalid characters
    const juce::String invalidChars = "<>:\"/\\|?*";
    for (int i = 0; i < invalidChars.length(); ++i)
        id = id.replaceCharacter(invalidChars[i], '_');
    
    // Add timestamp if file already exists
    auto toneFile = getTonePresetsFolder().getChildFile(id + ".mks70tone");
    auto patchFile = getPatchPresetsFolder().getChildFile(id + ".mks70patch");
    
    if (toneFile.exists() || patchFile.exists())
    {
        id += "_" + juce::String(juce::Time::currentTimeMillis());
    }
    
    return id;
}

//==============================================================================
// Tone Presets
//==============================================================================
bool PresetManager::saveTonePreset(const ToneData& tone, const PresetMetadata& metadata)
{
    TonePreset preset;
    preset.toneData = tone;
    preset.metadata = metadata;
    preset.metadata.dateModified = juce::Time::getCurrentTime();
    
    if (preset.metadata.dateCreated == juce::Time())
        preset.metadata.dateCreated = juce::Time::getCurrentTime();
    
    juce::String id = generatePresetId(metadata.name);
    auto file = getTonePresetsFolder().getChildFile(id + ".mks70tone");
    
    if (preset.saveToFile(file))
    {
        scanLibrary();
        listeners.call(&Listener::presetSaved, id);
        return true;
    }
    
    return false;
}

TonePreset PresetManager::loadTonePreset(const juce::String& presetId)
{
    auto file = getTonePresetsFolder().getChildFile(presetId + ".mks70tone");
    if (file.exists())
    {
        auto preset = TonePreset::loadFromFile(file);
        listeners.call(&Listener::presetLoaded, presetId);
        return preset;
    }
    return TonePreset();
}

bool PresetManager::deleteTonePreset(const juce::String& presetId)
{
    auto file = getTonePresetsFolder().getChildFile(presetId + ".mks70tone");
    if (file.exists() && file.deleteFile())
    {
        scanLibrary();
        return true;
    }
    return false;
}

//==============================================================================
// Patch Presets
//==============================================================================
bool PresetManager::savePatchPreset(const ToneData& upper, const ToneData& lower,
                                     const PresetMetadata& metadata)
{
    PatchPreset preset;
    preset.upperTone = upper;
    preset.lowerTone = lower;
    preset.metadata = metadata;
    preset.metadata.dateModified = juce::Time::getCurrentTime();
    
    if (preset.metadata.dateCreated == juce::Time())
        preset.metadata.dateCreated = juce::Time::getCurrentTime();
    
    juce::String id = generatePresetId(metadata.name);
    auto file = getPatchPresetsFolder().getChildFile(id + ".mks70patch");
    
    if (preset.saveToFile(file))
    {
        scanLibrary();
        listeners.call(&Listener::presetSaved, id);
        return true;
    }
    
    return false;
}

PatchPreset PresetManager::loadPatchPreset(const juce::String& presetId)
{
    auto file = getPatchPresetsFolder().getChildFile(presetId + ".mks70patch");
    if (file.exists())
    {
        auto preset = PatchPreset::loadFromFile(file);
        listeners.call(&Listener::presetLoaded, presetId);
        return preset;
    }
    return PatchPreset();
}

bool PresetManager::deletePatchPreset(const juce::String& presetId)
{
    auto file = getPatchPresetsFolder().getChildFile(presetId + ".mks70patch");
    if (file.exists() && file.deleteFile())
    {
        scanLibrary();
        return true;
    }
    return false;
}

//==============================================================================
// SysEx Import/Export
//==============================================================================
std::vector<ToneData> PresetManager::importSysExFile(const juce::File& file)
{
    std::vector<ToneData> result;
    
    juce::FileInputStream stream(file);
    if (!stream.openedOk())
        return result;
    
    std::vector<uint8_t> fileData;
    fileData.resize(static_cast<size_t>(stream.getTotalLength()));
    stream.read(fileData.data(), static_cast<int>(fileData.size()));
    
    // Parse SysEx messages
    size_t pos = 0;
    while (pos < fileData.size())
    {
        // Find start of SysEx (0xF0)
        while (pos < fileData.size() && fileData[pos] != 0xF0)
            ++pos;
        
        if (pos >= fileData.size())
            break;
        
        // Find end of SysEx (0xF7)
        size_t start = pos;
        while (pos < fileData.size() && fileData[pos] != 0xF7)
            ++pos;
        
        if (pos >= fileData.size())
            break;
        
        // Extract SysEx message
        std::vector<uint8_t> sysex(fileData.begin() + start, fileData.begin() + pos + 1);
        
        // Check if it's a Roland MKS-70 message
        if (sysex.size() > 6 && sysex[1] == 0x41) // Roland ID
        {
            // Try to parse as tone data
            try
            {
                auto tone = parseToneSysEx(sysex);
                if (!tone.toneName.isEmpty())
                    result.push_back(tone);
            }
            catch (...) {}
        }
        
        ++pos;
    }
    
    return result;
}

bool PresetManager::exportToSysEx(const ToneData& tone, const juce::File& file)
{
    auto sysex = generateToneSysEx(tone);
    
    juce::FileOutputStream stream(file);
    if (stream.openedOk())
    {
        stream.setPosition(0);
        stream.truncate();
        stream.write(sysex.data(), sysex.size());
        return true;
    }
    
    return false;
}

bool PresetManager::exportToSysEx(const std::vector<ToneData>& tones, const juce::File& file)
{
    juce::FileOutputStream stream(file);
    if (!stream.openedOk())
        return false;
    
    stream.setPosition(0);
    stream.truncate();
    
    int toneNum = 1;
    for (const auto& tone : tones)
    {
        auto sysex = generateToneSysEx(tone, toneNum++);
        stream.write(sysex.data(), sysex.size());
    }
    
    return true;
}

//==============================================================================
ToneData PresetManager::parseToneSysEx(const std::vector<uint8_t>& sysex)
{
    ToneData tone;
    
    // MKS-70 APR (All Parameters) format for tone:
    // F0 41 35 [Channel] 24 20 [Tone] [10-byte name] [50 parameter bytes] F7
    // Total: 68 bytes for files generated by this editor.
    constexpr int dataStart = 7;
    constexpr int nameLength = 10;
    constexpr int parameterCount = 50;
    constexpr int minimumSize = dataStart + nameLength + parameterCount + 1;
    
    if (sysex.size() < minimumSize)
        return tone;
    
    // Verify header
    if (sysex[0] != 0xF0 || sysex[1] != 0x41 || sysex[2] != 0x35)
        return tone;
    
    if (sysex[4] != 0x24 || sysex[5] != 0x20)
        return tone;
    
    // Parse tone name (first 10 bytes of data)
    juce::String name;
    for (int i = 0; i < 10; ++i)
    {
        char c = static_cast<char>(sysex[dataStart + i]);
        if (c >= 32 && c < 127)
            name += c;
    }
    tone.toneName = name.trim();
    
    // Parse tone parameters starting at offset 10
    int paramStart = dataStart + 10;
    
    // DCO1
    tone.dco1Range = sysex[paramStart + 0];
    tone.dco1Waveform = sysex[paramStart + 1];
    tone.dco1Tune = sysex[paramStart + 2];
    tone.dco1LFOMod = sysex[paramStart + 3];
    tone.dco1ENVMod = sysex[paramStart + 4];
    
    // DCO2
    tone.dco2Range = sysex[paramStart + 5];
    tone.dco2Waveform = sysex[paramStart + 6];
    tone.dco2CrossMod = sysex[paramStart + 7];
    tone.dco2Tune = sysex[paramStart + 8];
    tone.dco2FineTune = sysex[paramStart + 9];
    tone.dco2LFOMod = sysex[paramStart + 10];
    tone.dco2ENVMod = sysex[paramStart + 11];
    tone.dcoDynamics = sysex[paramStart + 12];
    tone.dcoENVMode = sysex[paramStart + 13];
    
    // Mixer
    tone.mixerDCO1Level = sysex[paramStart + 14];
    tone.mixerDCO2Level = sysex[paramStart + 15];
    tone.mixerENVMod = sysex[paramStart + 16];
    tone.mixerDynamics = sysex[paramStart + 17];
    tone.mixerENVMode = sysex[paramStart + 18];
    
    // VCF
    tone.vcfHPF = sysex[paramStart + 19];
    tone.vcfCutoff = sysex[paramStart + 20];
    tone.vcfResonance = sysex[paramStart + 21];
    tone.vcfLFOMod = sysex[paramStart + 22];
    tone.vcfENVMod = sysex[paramStart + 23];
    tone.vcfKeyFollow = sysex[paramStart + 24];
    tone.vcfDynamics = sysex[paramStart + 25];
    tone.vcfENVMode = sysex[paramStart + 26];
    
    // VCA
    tone.vcaLevel = sysex[paramStart + 27];
    tone.vcaDynamics = sysex[paramStart + 28];
    tone.chorusMode = sysex[paramStart + 29];
    
    // LFO1
    tone.lfo1Waveform = sysex[paramStart + 30];
    tone.lfo1Delay = sysex[paramStart + 31];
    tone.lfo1Rate = sysex[paramStart + 32];
    
    // ENV1
    tone.env1Time1 = sysex[paramStart + 33];
    tone.env1Level1 = sysex[paramStart + 34];
    tone.env1Time2 = sysex[paramStart + 35];
    tone.env1Level2 = sysex[paramStart + 36];
    tone.env1Time3 = sysex[paramStart + 37];
    tone.env1Level3 = sysex[paramStart + 38];
    tone.env1Time4 = sysex[paramStart + 39];
    tone.env1KeyFollow = sysex[paramStart + 40];
    
    // ENV2
    tone.env2Time1 = sysex[paramStart + 41];
    tone.env2Level1 = sysex[paramStart + 42];
    tone.env2Time2 = sysex[paramStart + 43];
    tone.env2Level2 = sysex[paramStart + 44];
    tone.env2Time3 = sysex[paramStart + 45];
    tone.env2Level3 = sysex[paramStart + 46];
    tone.env2Time4 = sysex[paramStart + 47];
    tone.env2KeyFollow = sysex[paramStart + 48];
    
    // VCA Mode
    tone.vcaMode = sysex[paramStart + 49];
    
    return tone;
}

std::vector<uint8_t> PresetManager::generateToneSysEx(const ToneData& tone, int toneNumber)
{
    std::vector<uint8_t> sysex;
    
    // Header
    sysex.push_back(0xF0);                      // SysEx Start
    sysex.push_back(MKS70Mapping::ROLAND_ID);   // 0x41 - Roland ID
    sysex.push_back(MKS70Mapping::CMD_APR);     // 0x35 - APR command
    sysex.push_back(0x00);                      // Channel (Device ID)
    sysex.push_back(MKS70Mapping::MODEL_ID2);   // 0x24 - MKS-70 Model ID
    sysex.push_back(MKS70Mapping::LEVEL_TONE);  // 0x20 - Tone level
    sysex.push_back(static_cast<uint8_t>(toneNumber)); // Tone number
    
    // Tone name (10 bytes)
    juce::String name = tone.toneName.substring(0, 10);
    while (name.length() < 10)
        name += " ";
    for (int i = 0; i < 10; ++i)
        sysex.push_back(static_cast<uint8_t>(name[i]));
    
    // DCO1
    sysex.push_back(static_cast<uint8_t>(tone.dco1Range));
    sysex.push_back(static_cast<uint8_t>(tone.dco1Waveform));
    sysex.push_back(static_cast<uint8_t>(tone.dco1Tune));
    sysex.push_back(static_cast<uint8_t>(tone.dco1LFOMod));
    sysex.push_back(static_cast<uint8_t>(tone.dco1ENVMod));
    
    // DCO2
    sysex.push_back(static_cast<uint8_t>(tone.dco2Range));
    sysex.push_back(static_cast<uint8_t>(tone.dco2Waveform));
    sysex.push_back(static_cast<uint8_t>(tone.dco2CrossMod));
    sysex.push_back(static_cast<uint8_t>(tone.dco2Tune));
    sysex.push_back(static_cast<uint8_t>(tone.dco2FineTune));
    sysex.push_back(static_cast<uint8_t>(tone.dco2LFOMod));
    sysex.push_back(static_cast<uint8_t>(tone.dco2ENVMod));
    sysex.push_back(static_cast<uint8_t>(tone.dcoDynamics));
    sysex.push_back(static_cast<uint8_t>(tone.dcoENVMode));
    
    // Mixer
    sysex.push_back(static_cast<uint8_t>(tone.mixerDCO1Level));
    sysex.push_back(static_cast<uint8_t>(tone.mixerDCO2Level));
    sysex.push_back(static_cast<uint8_t>(tone.mixerENVMod));
    sysex.push_back(static_cast<uint8_t>(tone.mixerDynamics));
    sysex.push_back(static_cast<uint8_t>(tone.mixerENVMode));
    
    // VCF
    sysex.push_back(static_cast<uint8_t>(tone.vcfHPF));
    sysex.push_back(static_cast<uint8_t>(tone.vcfCutoff));
    sysex.push_back(static_cast<uint8_t>(tone.vcfResonance));
    sysex.push_back(static_cast<uint8_t>(tone.vcfLFOMod));
    sysex.push_back(static_cast<uint8_t>(tone.vcfENVMod));
    sysex.push_back(static_cast<uint8_t>(tone.vcfKeyFollow));
    sysex.push_back(static_cast<uint8_t>(tone.vcfDynamics));
    sysex.push_back(static_cast<uint8_t>(tone.vcfENVMode));
    
    // VCA
    sysex.push_back(static_cast<uint8_t>(tone.vcaLevel));
    sysex.push_back(static_cast<uint8_t>(tone.vcaDynamics));
    sysex.push_back(static_cast<uint8_t>(tone.chorusMode));
    
    // LFO1
    sysex.push_back(static_cast<uint8_t>(tone.lfo1Waveform));
    sysex.push_back(static_cast<uint8_t>(tone.lfo1Delay));
    sysex.push_back(static_cast<uint8_t>(tone.lfo1Rate));
    
    // ENV1
    sysex.push_back(static_cast<uint8_t>(tone.env1Time1));
    sysex.push_back(static_cast<uint8_t>(tone.env1Level1));
    sysex.push_back(static_cast<uint8_t>(tone.env1Time2));
    sysex.push_back(static_cast<uint8_t>(tone.env1Level2));
    sysex.push_back(static_cast<uint8_t>(tone.env1Time3));
    sysex.push_back(static_cast<uint8_t>(tone.env1Level3));
    sysex.push_back(static_cast<uint8_t>(tone.env1Time4));
    sysex.push_back(static_cast<uint8_t>(tone.env1KeyFollow));
    
    // ENV2
    sysex.push_back(static_cast<uint8_t>(tone.env2Time1));
    sysex.push_back(static_cast<uint8_t>(tone.env2Level1));
    sysex.push_back(static_cast<uint8_t>(tone.env2Time2));
    sysex.push_back(static_cast<uint8_t>(tone.env2Level2));
    sysex.push_back(static_cast<uint8_t>(tone.env2Time3));
    sysex.push_back(static_cast<uint8_t>(tone.env2Level3));
    sysex.push_back(static_cast<uint8_t>(tone.env2Time4));
    sysex.push_back(static_cast<uint8_t>(tone.env2KeyFollow));
    
    // VCA Mode
    sysex.push_back(static_cast<uint8_t>(tone.vcaMode));
    
    // End
    sysex.push_back(0xF7);
    
    return sysex;
}

//==============================================================================
// Search & Filter
//==============================================================================
std::vector<PresetLibraryEntry> PresetManager::searchPresets(const juce::String& query, PresetType type)
{
    std::vector<PresetLibraryEntry> results;
    const auto& presets = (type == PresetType::Tone) ? tonePresets : patchPresets;
    
    juce::String lowerQuery = query.toLowerCase();
    
    for (const auto& entry : presets)
    {
        if (entry.metadata.name.toLowerCase().contains(lowerQuery) ||
            entry.metadata.category.toLowerCase().contains(lowerQuery) ||
            entry.metadata.tags.toLowerCase().contains(lowerQuery) ||
            entry.metadata.author.toLowerCase().contains(lowerQuery))
        {
            results.push_back(entry);
        }
    }
    
    return results;
}

std::vector<PresetLibraryEntry> PresetManager::filterByCategory(const juce::String& category, PresetType type)
{
    std::vector<PresetLibraryEntry> results;
    const auto& presets = (type == PresetType::Tone) ? tonePresets : patchPresets;
    
    for (const auto& entry : presets)
    {
        if (entry.metadata.category.equalsIgnoreCase(category))
            results.push_back(entry);
    }
    
    return results;
}

std::vector<PresetLibraryEntry> PresetManager::getFavorites(PresetType type)
{
    std::vector<PresetLibraryEntry> results;
    const auto& presets = (type == PresetType::Tone) ? tonePresets : patchPresets;
    
    for (const auto& entry : presets)
    {
        if (entry.metadata.isFavorite)
            results.push_back(entry);
    }
    
    return results;
}
