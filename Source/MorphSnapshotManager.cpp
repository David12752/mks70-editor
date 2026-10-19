/*
  ==============================================================================

    MorphSnapshotManager.cpp
    Morph snapshot management implementation

  ==============================================================================
*/

#include "MorphSnapshotManager.h"

//==============================================================================
MorphSnapshotManager::MorphSnapshotManager()
{
}

MorphSnapshotManager::~MorphSnapshotManager()
{
}

//==============================================================================
// Snapshot Management

void MorphSnapshotManager::captureSnapshot(const juce::String& name, float position, const FullPatch& patch)
{
    // Clamp position
    position = juce::jlimit(0.0f, 1.0f, position);
    
    // Create snapshot with name or default
    juce::String snapshotName = name.isNotEmpty() ? name : generateDefaultName(static_cast<int>(snapshots.size()));
    
    MorphSnapshot snapshot(position, snapshotName, patch);
    snapshots.push_back(snapshot);
    
    DBG("MorphSnapshotManager: Captured snapshot '" + snapshotName + "' at position " + 
        juce::String(position * 100.0f, 1) + "%");
}

bool MorphSnapshotManager::recallSnapshot(int index, FullPatch& outPatch)
{
    if (index < 0 || index >= static_cast<int>(snapshots.size()))
    {
        DBG("MorphSnapshotManager: Invalid snapshot index " + juce::String(index));
        return false;
    }
    
    outPatch = snapshots[index].resultPatch;
    
    DBG("MorphSnapshotManager: Recalled snapshot '" + snapshots[index].name + "'");
    return true;
}

void MorphSnapshotManager::deleteSnapshot(int index)
{
    if (index >= 0 && index < static_cast<int>(snapshots.size()))
    {
        juce::String name = snapshots[index].name;
        snapshots.erase(snapshots.begin() + index);
        DBG("MorphSnapshotManager: Deleted snapshot '" + name + "'");
    }
}

void MorphSnapshotManager::clearAllSnapshots()
{
    int count = static_cast<int>(snapshots.size());
    snapshots.clear();
    DBG("MorphSnapshotManager: Cleared " + juce::String(count) + " snapshots");
}

//==============================================================================
// Access

const MorphSnapshot* MorphSnapshotManager::getSnapshot(int index) const
{
    if (index >= 0 && index < static_cast<int>(snapshots.size()))
        return &snapshots[index];
    
    return nullptr;
}

//==============================================================================
// Export

void MorphSnapshotManager::exportAsNewPatches(std::vector<FullPatch>& outPatches) const
{
    outPatches.clear();
    outPatches.reserve(snapshots.size());
    
    for (const auto& snapshot : snapshots)
    {
        FullPatch patch = snapshot.resultPatch;
        
        // Update patch name to include snapshot name
        std::memcpy(patch.patch.patchName, snapshot.name.toRawUTF8(), 
                    juce::jmin(10, snapshot.name.length()));
        patch.patch.patchName[10] = '\0';
        
        patch.name = snapshot.name;
        patch.timestamp = snapshot.timestamp;
        
        outPatches.push_back(patch);
    }
    
    DBG("MorphSnapshotManager: Exported " + juce::String(outPatches.size()) + " patches");
}

void MorphSnapshotManager::exportSnapshotToPatch(int index, FullPatch& outPatch) const
{
    if (index >= 0 && index < static_cast<int>(snapshots.size()))
    {
        outPatch = snapshots[index].resultPatch;
        
        // Update patch name
        const auto& snapshot = snapshots[index];
        std::memcpy(outPatch.patch.patchName, snapshot.name.toRawUTF8(), 
                    juce::jmin(10, snapshot.name.length()));
        outPatch.patch.patchName[10] = '\0';
        
        outPatch.name = snapshot.name;
        outPatch.timestamp = snapshot.timestamp;
        
        DBG("MorphSnapshotManager: Exported snapshot '" + snapshot.name + "' to patch");
    }
}

//==============================================================================
// Sort and Organize

void MorphSnapshotManager::sortByPosition()
{
    std::sort(snapshots.begin(), snapshots.end(),
        [](const MorphSnapshot& a, const MorphSnapshot& b) {
            return a.position < b.position;
        });
    
    DBG("MorphSnapshotManager: Sorted snapshots by position");
}

void MorphSnapshotManager::sortByName()
{
    std::sort(snapshots.begin(), snapshots.end(),
        [](const MorphSnapshot& a, const MorphSnapshot& b) {
            return a.name < b.name;
        });
    
    DBG("MorphSnapshotManager: Sorted snapshots by name");
}

void MorphSnapshotManager::sortByTimestamp()
{
    std::sort(snapshots.begin(), snapshots.end(),
        [](const MorphSnapshot& a, const MorphSnapshot& b) {
            return a.timestamp < b.timestamp;
        });
    
    DBG("MorphSnapshotManager: Sorted snapshots by timestamp");
}

//==============================================================================
// File Operations

bool MorphSnapshotManager::saveToFile(const juce::File& file)
{
    auto xml = toXml();
    if (xml == nullptr)
        return false;
    
    bool success = xml->writeTo(file);
    
    if (success)
        DBG("MorphSnapshotManager: Saved " + juce::String(snapshots.size()) + " snapshots to " + file.getFullPathName());
    else
        DBG("MorphSnapshotManager: Failed to save to " + file.getFullPathName());
    
    return success;
}

bool MorphSnapshotManager::loadFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        DBG("MorphSnapshotManager: File does not exist: " + file.getFullPathName());
        return false;
    }
    
    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr)
    {
        DBG("MorphSnapshotManager: Failed to parse XML from " + file.getFullPathName());
        return false;
    }
    
    fromXml(*xml);
    DBG("MorphSnapshotManager: Loaded " + juce::String(snapshots.size()) + " snapshots from " + file.getFullPathName());
    
    return true;
}

//==============================================================================
// XML Serialization

std::unique_ptr<juce::XmlElement> MorphSnapshotManager::toXml() const
{
    auto xml = std::make_unique<juce::XmlElement>("MorphSnapshots");
    xml->setAttribute("count", static_cast<int>(snapshots.size()));
    
    for (const auto& snapshot : snapshots)
    {
        auto snapshotXml = xml->createNewChildElement("Snapshot");
        snapshotXml->setAttribute("name", snapshot.name);
        snapshotXml->setAttribute("position", static_cast<double>(snapshot.position));
        snapshotXml->setAttribute("timestamp", static_cast<double>(snapshot.timestamp.toMilliseconds()));
        
        // Save patch data (simplified - you may want to use full serialization)
        auto patchXml = snapshotXml->createNewChildElement("Patch");
        patchXml->setAttribute("patchName", juce::String::fromUTF8(snapshot.resultPatch.patch.patchName, 11));
        patchXml->setAttribute("toneName", juce::String::fromUTF8(snapshot.resultPatch.patch.toneName, 11));
        
        // Note: For full implementation, serialize all ToneParams, PatchParameters, ArpParameters
        // This is a simplified version
    }
    
    return xml;
}

void MorphSnapshotManager::fromXml(const juce::XmlElement& xml)
{
    if (xml.getTagName() != "MorphSnapshots")
        return;
    
    snapshots.clear();
    
    for (auto* snapshotXml : xml.getChildIterator())
    {
        if (snapshotXml->getTagName() != "Snapshot")
            continue;
        
        MorphSnapshot snapshot;
        snapshot.name = snapshotXml->getStringAttribute("name");
        snapshot.position = static_cast<float>(snapshotXml->getDoubleAttribute("position"));
        snapshot.timestamp = juce::Time(static_cast<juce::int64>(snapshotXml->getDoubleAttribute("timestamp")));
        
        // Load patch data
        if (auto* patchXml = snapshotXml->getChildByName("Patch"))
        {
            juce::String patchName = patchXml->getStringAttribute("patchName");
            juce::String toneName = patchXml->getStringAttribute("toneName");
            
            std::memcpy(snapshot.resultPatch.patch.patchName, patchName.toRawUTF8(),
                        juce::jmin(10, patchName.length()));
            snapshot.resultPatch.patch.patchName[10] = '\0';
            
            std::memcpy(snapshot.resultPatch.patch.toneName, toneName.toRawUTF8(),
                        juce::jmin(10, toneName.length()));
            snapshot.resultPatch.patch.toneName[10] = '\0';
            
            // Note: Load full parameters in production version
        }
        
        snapshots.push_back(snapshot);
    }
}

//==============================================================================
// Helper Methods

juce::String MorphSnapshotManager::generateDefaultName(int index) const
{
    return "Morph " + juce::String(index + 1);
}
