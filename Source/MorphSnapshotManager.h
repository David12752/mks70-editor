/*
  ==============================================================================

    MorphSnapshotManager.h
    Manager for saving and recalling morph positions as snapshots

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompareManager.h"
#include "MorphEngine.h"

//==============================================================================
/**
 * MorphSnapshot - A saved morph position with its resulting patch
 */
struct MorphSnapshot
{
    float position;             // 0.0 to 1.0
    juce::String name;
    FullPatch resultPatch;      // The calculated morphed patch
    juce::Time timestamp;
    
    MorphSnapshot() : position(0.0f) {}
    
    MorphSnapshot(float pos, const juce::String& n, const FullPatch& patch)
        : position(pos), name(n), resultPatch(patch), timestamp(juce::Time::getCurrentTime())
    {}
};

//==============================================================================
/**
 * MorphSnapshotManager - Manages morph snapshots
 * Allows saving morph positions and exporting them as new patches
 */
class MorphSnapshotManager
{
public:
    MorphSnapshotManager();
    ~MorphSnapshotManager();
    
    // Snapshot management
    void captureSnapshot(const juce::String& name, float position, const FullPatch& patch);
    bool recallSnapshot(int index, FullPatch& outPatch);
    void deleteSnapshot(int index);
    void clearAllSnapshots();
    
    // Access snapshots
    const std::vector<MorphSnapshot>& getSnapshots() const { return snapshots; }
    int getSnapshotCount() const { return static_cast<int>(snapshots.size()); }
    const MorphSnapshot* getSnapshot(int index) const;
    
    // Export functionality
    void exportAsNewPatches(std::vector<FullPatch>& outPatches) const;
    void exportSnapshotToPatch(int index, FullPatch& outPatch) const;
    
    // Sort and organize
    void sortByPosition();
    void sortByName();
    void sortByTimestamp();
    
    // Save/Load from file
    bool saveToFile(const juce::File& file);
    bool loadFromFile(const juce::File& file);
    
    // XML serialization
    std::unique_ptr<juce::XmlElement> toXml() const;
    void fromXml(const juce::XmlElement& xml);

private:
    std::vector<MorphSnapshot> snapshots;
    
    // Helper methods
    juce::String generateDefaultName(int index) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorphSnapshotManager)
};
