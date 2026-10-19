#include "CompareManager.h"

//==============================================================================
// Storage operations

void CompareManager::storeToSlot(Slot slot, const FullPatch& patch)
{
    slots[slot] = patch;
    slots[slot].timestamp = juce::Time::getCurrentTime();
    
    DBG("CompareManager: Stored patch '" + patch.name + "' to " + getSlotName(slot));
}

FullPatch& CompareManager::getSlot(Slot slot)
{
    ensureSlotExists(slot);
    return slots[slot];
}

const FullPatch& CompareManager::getSlot(Slot slot) const
{
    auto it = slots.find(slot);
    if (it == slots.end())
    {
        static FullPatch emptyPatch;
        return emptyPatch;
    }
    return it->second;
}

void CompareManager::clearSlot(Slot slot)
{
    slots.erase(slot);
    DBG("CompareManager: Cleared " + getSlotName(slot));
}

bool CompareManager::hasSlot(Slot slot) const
{
    return slots.find(slot) != slots.end();
}

void CompareManager::setActiveSlot(Slot slot)
{
    activeSlot = slot;
    DBG("CompareManager: Active slot set to " + getSlotName(slot));
}

CompareManager::Slot CompareManager::getActiveSlot() const
{
    return activeSlot;
}

//==============================================================================
// Compare operations

std::vector<ParameterDiff> CompareManager::compareSlots(Slot a, Slot b) const
{
    std::vector<ParameterDiff> diffs;
    
    if (!hasSlot(a) || !hasSlot(b))
    {
        DBG("CompareManager: Cannot compare - missing slot data");
        return diffs;
    }
    
    const auto& patchA = getSlot(a);
    const auto& patchB = getSlot(b);
    
    // Compare tone parameters (upper)
    auto toneUpperDiffs = compareToneParams(patchA.toneUpper, patchB.toneUpper, "Tone Upper");
    diffs.insert(diffs.end(), toneUpperDiffs.begin(), toneUpperDiffs.end());
    
    // Compare tone parameters (lower)
    auto toneLowerDiffs = compareToneParams(patchA.toneLower, patchB.toneLower, "Tone Lower");
    diffs.insert(diffs.end(), toneLowerDiffs.begin(), toneLowerDiffs.end());
    
    // Compare patch parameters
    auto patchDiffs = comparePatchParams(patchA.patch, patchB.patch);
    diffs.insert(diffs.end(), patchDiffs.begin(), patchDiffs.end());
    
    // Compare arpeggiator parameters
    auto arpDiffs = compareArpParams(patchA.arp, patchB.arp);
    diffs.insert(diffs.end(), arpDiffs.begin(), arpDiffs.end());
    
    DBG("CompareManager: Found " + juce::String(diffs.size()) + " differences between " + 
        getSlotName(a) + " and " + getSlotName(b));
    
    return diffs;
}

bool CompareManager::hasChanges(Slot slot) const
{
    if (!hasSlot(slot) || !hasSlot(SLOT_ORIGINAL))
        return false;
    
    const auto& current = getSlot(slot);
    const auto& original = getSlot(SLOT_ORIGINAL);
    
    return current != original;
}

int CompareManager::getDifferenceCount(Slot a, Slot b) const
{
    auto diffs = compareSlots(a, b);
    return static_cast<int>(diffs.size());
}

bool CompareManager::isDifferent(const juce::String& parameterName) const
{
    if (!hasSlot(SLOT_A) || !hasSlot(SLOT_B))
        return false;
    
    auto diffs = compareSlots(SLOT_A, SLOT_B);
    
    for (const auto& diff : diffs)
    {
        if (diff.parameterName == parameterName)
            return true;
    }
    
    return false;
}

std::pair<int, int> CompareManager::getParameterValues(const juce::String& parameterName) const
{
    if (!hasSlot(SLOT_A) || !hasSlot(SLOT_B))
        return {-1, -1};
    
    auto diffs = compareSlots(SLOT_A, SLOT_B);
    
    for (const auto& diff : diffs)
    {
        if (diff.parameterName == parameterName)
            return {diff.valueA, diff.valueB};
    }
    
    return {-1, -1};
}

//==============================================================================
// Quick switch operations

void CompareManager::swapAB()
{
    if (!hasSlot(SLOT_A) || !hasSlot(SLOT_B))
    {
        DBG("CompareManager: Cannot swap - missing slot data");
        return;
    }
    
    auto temp = slots[SLOT_A];
    slots[SLOT_A] = slots[SLOT_B];
    slots[SLOT_B] = temp;
    
    DBG("CompareManager: Swapped slots A and B");
}

void CompareManager::copySlot(Slot from, Slot to)
{
    if (!hasSlot(from))
    {
        DBG("CompareManager: Cannot copy - source slot empty");
        return;
    }
    
    slots[to] = slots[from];
    slots[to].timestamp = juce::Time::getCurrentTime();
    
    DBG("CompareManager: Copied " + getSlotName(from) + " to " + getSlotName(to));
}

//==============================================================================
// Utility

juce::String CompareManager::getSlotName(Slot slot)
{
    switch (slot)
    {
        case SLOT_A:         return "Slot A";
        case SLOT_B:         return "Slot B";
        case SLOT_ORIGINAL:  return "Original";
        case SLOT_CLIPBOARD: return "Clipboard";
        default:             return "Unknown";
    }
}

void CompareManager::clearAll()
{
    slots.clear();
    activeSlot = SLOT_A;
    DBG("CompareManager: Cleared all slots");
}

//==============================================================================
// Private helper methods

void CompareManager::ensureSlotExists(Slot slot)
{
    if (slots.find(slot) == slots.end())
    {
        slots[slot] = FullPatch();
    }
}

std::vector<ParameterDiff> CompareManager::compareToneParams(
    const ToneParams& a, 
    const ToneParams& b, 
    const juce::String& section) const
{
    std::vector<ParameterDiff> diffs;
    
    // For now, do simple struct comparison
    // TODO: Implement field-by-field comparison with proper names from MappingLoader
    if (memcmp(&a, &b, sizeof(ToneParams)) != 0)
    {
        ParameterDiff diff(
            "Tone Parameters Changed",
            section,
            0,
            0,
            1
        );
        diffs.push_back(diff);
    }
    
    return diffs;
}

std::vector<ParameterDiff> CompareManager::comparePatchParams(
    const PatchParameters& a, 
    const PatchParameters& b) const
{
    std::vector<ParameterDiff> diffs;
    
    // For now, do simple struct comparison
    if (memcmp(&a, &b, sizeof(PatchParameters)) != 0)
    {
        ParameterDiff diff(
            "Patch Parameters Changed",
            "Patch",
            0,
            0,
            1
        );
        diffs.push_back(diff);
    }
    
    return diffs;
}

std::vector<ParameterDiff> CompareManager::compareArpParams(
    const ArpParameters& a, 
    const ArpParameters& b) const
{
    std::vector<ParameterDiff> diffs;
    
    // For now, do simple struct comparison
    if (memcmp(&a, &b, sizeof(ArpParameters)) != 0)
    {
        ParameterDiff diff(
            "Arpeggiator Parameters Changed",
            "Arpeggiator",
            0,
            0,
            1
        );
        diffs.push_back(diff);
    }
    
    return diffs;
}
