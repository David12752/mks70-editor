/*
  ==============================================================================

    PresetManager.h
    Preset/Librarian functionality for MKS-70 Editor
    
    Supports:
    - Tone presets (individual Upper/Lower tones)
    - Patch presets (complete patches with both tones)
    - SysEx import/export
    - Bank management

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToneData.h"

//==============================================================================
// Preset Types
//==============================================================================
enum class PresetType
{
    Tone,       // Single tone (Upper or Lower)
    Patch,      // Complete patch (both tones + patch settings)
    Bank        // Full bank of patches
};

//==============================================================================
// Preset Metadata
//==============================================================================
struct PresetMetadata
{
    juce::String name;
    juce::String author;
    juce::String category;      // Bass, Pad, Lead, Keys, Strings, Brass, FX, etc.
    juce::String tags;          // Comma-separated tags
    juce::String description;
    juce::Time dateCreated;
    juce::Time dateModified;
    bool isFavorite = false;
    int rating = 0;             // 0-5 stars
    
    juce::var toVar() const
    {
        auto obj = new juce::DynamicObject();
        obj->setProperty("name", name);
        obj->setProperty("author", author);
        obj->setProperty("category", category);
        obj->setProperty("tags", tags);
        obj->setProperty("description", description);
        obj->setProperty("dateCreated", dateCreated.toMilliseconds());
        obj->setProperty("dateModified", dateModified.toMilliseconds());
        obj->setProperty("isFavorite", isFavorite);
        obj->setProperty("rating", rating);
        return juce::var(obj);
    }
    
    static PresetMetadata fromVar(const juce::var& v)
    {
        PresetMetadata meta;
        if (auto* obj = v.getDynamicObject())
        {
            meta.name = obj->getProperty("name").toString();
            meta.author = obj->getProperty("author").toString();
            meta.category = obj->getProperty("category").toString();
            meta.tags = obj->getProperty("tags").toString();
            meta.description = obj->getProperty("description").toString();
            meta.dateCreated = juce::Time(static_cast<juce::int64>(obj->getProperty("dateCreated")));
            meta.dateModified = juce::Time(static_cast<juce::int64>(obj->getProperty("dateModified")));
            meta.isFavorite = obj->getProperty("isFavorite");
            meta.rating = obj->getProperty("rating");
        }
        return meta;
    }
};

//==============================================================================
// Tone Preset
//==============================================================================
struct TonePreset
{
    PresetMetadata metadata;
    ToneData toneData;
    
    juce::var toVar() const
    {
        auto obj = new juce::DynamicObject();
        obj->setProperty("type", "tone");
        obj->setProperty("version", 1);
        obj->setProperty("metadata", metadata.toVar());
        obj->setProperty("data", toneData.toVar());
        return juce::var(obj);
    }
    
    static TonePreset fromVar(const juce::var& v)
    {
        TonePreset preset;
        if (auto* obj = v.getDynamicObject())
        {
            preset.metadata = PresetMetadata::fromVar(obj->getProperty("metadata"));
            preset.toneData = ToneData::fromVar(obj->getProperty("data"));
        }
        return preset;
    }
    
    bool saveToFile(const juce::File& file) const
    {
        juce::FileOutputStream stream(file);
        if (stream.openedOk())
        {
            stream.setPosition(0);
            stream.truncate();
            juce::JSON::writeToStream(stream, toVar(), true);
            return true;
        }
        return false;
    }
    
    static TonePreset loadFromFile(const juce::File& file)
    {
        auto json = juce::JSON::parse(file);
        return fromVar(json);
    }
};

//==============================================================================
// Patch Preset (includes both tones + patch settings)
//==============================================================================
struct PatchPreset
{
    PresetMetadata metadata;
    ToneData upperTone;
    ToneData lowerTone;
    
    // Patch-level settings
    int keyMode = 0;            // 0=Whole, 1=Dual, 2=Split
    int splitPoint = 60;        // MIDI note number
    int upperDetune = 0;
    int lowerDetune = 0;
    int portamentoTime = 0;
    int portamentoMode = 0;
    int bendRange = 2;
    int afterTouchMode = 0;
    int upperLevel = 100;
    int lowerLevel = 100;
    
    juce::var toVar() const
    {
        auto obj = new juce::DynamicObject();
        obj->setProperty("type", "patch");
        obj->setProperty("version", 1);
        obj->setProperty("metadata", metadata.toVar());
        obj->setProperty("upperTone", upperTone.toVar());
        obj->setProperty("lowerTone", lowerTone.toVar());
        
        auto patchObj = new juce::DynamicObject();
        patchObj->setProperty("keyMode", keyMode);
        patchObj->setProperty("splitPoint", splitPoint);
        patchObj->setProperty("upperDetune", upperDetune);
        patchObj->setProperty("lowerDetune", lowerDetune);
        patchObj->setProperty("portamentoTime", portamentoTime);
        patchObj->setProperty("portamentoMode", portamentoMode);
        patchObj->setProperty("bendRange", bendRange);
        patchObj->setProperty("afterTouchMode", afterTouchMode);
        patchObj->setProperty("upperLevel", upperLevel);
        patchObj->setProperty("lowerLevel", lowerLevel);
        obj->setProperty("patchSettings", juce::var(patchObj));
        
        return juce::var(obj);
    }
    
    static PatchPreset fromVar(const juce::var& v)
    {
        PatchPreset preset;
        if (auto* obj = v.getDynamicObject())
        {
            preset.metadata = PresetMetadata::fromVar(obj->getProperty("metadata"));
            preset.upperTone = ToneData::fromVar(obj->getProperty("upperTone"));
            preset.lowerTone = ToneData::fromVar(obj->getProperty("lowerTone"));
            
            if (auto* patchObj = obj->getProperty("patchSettings").getDynamicObject())
            {
                preset.keyMode = patchObj->getProperty("keyMode");
                preset.splitPoint = patchObj->getProperty("splitPoint");
                preset.upperDetune = patchObj->getProperty("upperDetune");
                preset.lowerDetune = patchObj->getProperty("lowerDetune");
                preset.portamentoTime = patchObj->getProperty("portamentoTime");
                preset.portamentoMode = patchObj->getProperty("portamentoMode");
                preset.bendRange = patchObj->getProperty("bendRange");
                preset.afterTouchMode = patchObj->getProperty("afterTouchMode");
                preset.upperLevel = patchObj->getProperty("upperLevel");
                preset.lowerLevel = patchObj->getProperty("lowerLevel");
            }
        }
        return preset;
    }
    
    bool saveToFile(const juce::File& file) const
    {
        juce::FileOutputStream stream(file);
        if (stream.openedOk())
        {
            stream.setPosition(0);
            stream.truncate();
            juce::JSON::writeToStream(stream, toVar(), true);
            return true;
        }
        return false;
    }
    
    static PatchPreset loadFromFile(const juce::File& file)
    {
        auto json = juce::JSON::parse(file);
        return fromVar(json);
    }
};

//==============================================================================
// Preset Library Entry (for listing)
//==============================================================================
struct PresetLibraryEntry
{
    juce::String id;            // Unique identifier (filename without extension)
    PresetType type;
    PresetMetadata metadata;
    juce::File file;
};

//==============================================================================
// Preset Manager - Main class for librarian functionality
//==============================================================================
class PresetManager
{
public:
    PresetManager();
    ~PresetManager() = default;
    
    //==========================================================================
    // Library Management
    //==========================================================================
    void setLibraryPath(const juce::File& path);
    juce::File getLibraryPath() const { return libraryPath; }
    
    void scanLibrary();
    const std::vector<PresetLibraryEntry>& getTonePresets() const { return tonePresets; }
    const std::vector<PresetLibraryEntry>& getPatchPresets() const { return patchPresets; }
    
    //==========================================================================
    // Tone Presets
    //==========================================================================
    bool saveTonePreset(const ToneData& tone, const PresetMetadata& metadata);
    TonePreset loadTonePreset(const juce::String& presetId);
    bool deleteTonePreset(const juce::String& presetId);
    
    //==========================================================================
    // Patch Presets
    //==========================================================================
    bool savePatchPreset(const ToneData& upper, const ToneData& lower, 
                         const PresetMetadata& metadata);
    PatchPreset loadPatchPreset(const juce::String& presetId);
    bool deletePatchPreset(const juce::String& presetId);
    
    //==========================================================================
    // SysEx Import/Export
    //==========================================================================
    std::vector<ToneData> importSysExFile(const juce::File& file);
    bool exportToSysEx(const ToneData& tone, const juce::File& file);
    bool exportToSysEx(const std::vector<ToneData>& tones, const juce::File& file);
    
    // Parse raw SysEx data
    ToneData parseToneSysEx(const std::vector<uint8_t>& sysex);
    std::vector<uint8_t> generateToneSysEx(const ToneData& tone, int toneNumber = 1);
    
    //==========================================================================
    // Categories
    //==========================================================================
    static juce::StringArray getCategories()
    {
        return { "Bass", "Pad", "Lead", "Keys", "Strings", "Brass", 
                 "Pluck", "Organ", "Bell", "FX", "Percussion", "Other" };
    }
    
    //==========================================================================
    // Search & Filter
    //==========================================================================
    std::vector<PresetLibraryEntry> searchPresets(const juce::String& query, 
                                                   PresetType type = PresetType::Tone);
    std::vector<PresetLibraryEntry> filterByCategory(const juce::String& category,
                                                      PresetType type = PresetType::Tone);
    std::vector<PresetLibraryEntry> getFavorites(PresetType type = PresetType::Tone);
    
    //==========================================================================
    // Listeners
    //==========================================================================
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void presetLibraryChanged() {}
        virtual void presetLoaded(const juce::String& presetId) {}
        virtual void presetSaved(const juce::String& presetId) {}
    };
    
    void addListener(Listener* listener) { listeners.add(listener); }
    void removeListener(Listener* listener) { listeners.remove(listener); }
    
private:
    juce::File libraryPath;
    std::vector<PresetLibraryEntry> tonePresets;
    std::vector<PresetLibraryEntry> patchPresets;
    juce::ListenerList<Listener> listeners;
    
    juce::File getTonePresetsFolder() const;
    juce::File getPatchPresetsFolder() const;
    juce::String generatePresetId(const juce::String& name);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
