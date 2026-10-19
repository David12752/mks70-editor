#pragma once
#include <JuceHeader.h>
#include "ToneParams.h"
#include "PatchParameters.h"
#include "ArpParameters.h"
#include <map>
#include <vector>

// Forward declaration
struct ParameterDiff;
struct FullPatch;

/**
 * CompareManager - Manages A/B comparison of patches
 * Allows storing multiple patch snapshots and comparing them
 */
class CompareManager
{
public:
    CompareManager() = default;
    ~CompareManager() = default;
    
    // Buffer slots for storing patches
    enum Slot 
    { 
        SLOT_A,           // Current working patch
        SLOT_B,           // Alternative patch for comparison
        SLOT_ORIGINAL,    // Original patch before editing
        SLOT_CLIPBOARD    // Temporary storage for copy/paste
    };
    
    //==============================================================================
    // Storage operations
    
    /**
     * Store a complete patch to a slot
     * @param slot Target slot (A, B, ORIGINAL, or CLIPBOARD)
     * @param patch Complete patch data to store
     */
    void storeToSlot(Slot slot, const FullPatch& patch);
    
    /**
     * Get patch data from a slot
     * @param slot Source slot
     * @return Reference to the patch in the slot
     */
    FullPatch& getSlot(Slot slot);
    
    /**
     * Get patch data from a slot (const version)
     * @param slot Source slot
     * @return Const reference to the patch in the slot
     */
    const FullPatch& getSlot(Slot slot) const;
    
    /**
     * Clear a slot (reset to default values)
     * @param slot Slot to clear
     */
    void clearSlot(Slot slot);
    
    /**
     * Check if a slot contains valid data
     * @param slot Slot to check
     * @return true if slot has been initialized with data
     */
    bool hasSlot(Slot slot) const;
    
    /**
     * Set the active slot (for UI indicator purposes)
     * @param slot The slot that is now active
     */
    void setActiveSlot(Slot slot);
    
    /**
     * Get the currently active slot
     * @return The active slot (SLOT_A or SLOT_B), or SLOT_A by default
     */
    Slot getActiveSlot() const;
    
    //==============================================================================
    // Compare operations
    
    /**
     * Compare two slots and return list of differences
     * @param a First slot
     * @param b Second slot
     * @return Vector of parameter differences
     */
    std::vector<ParameterDiff> compareSlots(Slot a, Slot b) const;
    
    /**
     * Check if a slot has changes compared to ORIGINAL
     * @param slot Slot to check
     * @return true if slot differs from ORIGINAL
     */
    bool hasChanges(Slot slot) const;
    
    /**
     * Get total number of differences between two slots
     * @param a First slot
     * @param b Second slot
     * @return Number of parameters that differ
     */
    int getDifferenceCount(Slot a, Slot b) const;
    
    /**
     * Check if a specific parameter differs between slots A and B
     * @param parameterName Name of the parameter (e.g., "vcfCutoff")
     * @return true if parameter has different values in A and B
     */
    bool isDifferent(const juce::String& parameterName) const;
    
    /**
     * Get parameter values from both slots A and B
     * @param parameterName Name of the parameter
     * @return Pair of values (valueA, valueB), or {-1, -1} if not found
     */
    std::pair<int, int> getParameterValues(const juce::String& parameterName) const;
    
    //==============================================================================
    // Quick switch operations
    
    /**
     * Swap contents of slots A and B
     */
    void swapAB();
    
    /**
     * Copy slot A to B (or vice versa)
     * @param from Source slot
     * @param to Destination slot
     */
    void copySlot(Slot from, Slot to);
    
    //==============================================================================
    // Utility
    
    /**
     * Get slot name as string
     * @param slot Slot enum value
     * @return Human-readable slot name
     */
    static juce::String getSlotName(Slot slot);
    
    /**
     * Clear all slots
     */
    void clearAll();

private:
    std::map<Slot, FullPatch> slots;
    Slot activeSlot = SLOT_A;
    
    // Helper methods
    void ensureSlotExists(Slot slot);
    std::vector<ParameterDiff> compareToneParams(
        const ToneParams& a, 
        const ToneParams& b, 
        const juce::String& section) const;
    std::vector<ParameterDiff> comparePatchParams(
        const PatchParameters& a, 
        const PatchParameters& b) const;
    std::vector<ParameterDiff> compareArpParams(
        const ArpParameters& a, 
        const ArpParameters& b) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompareManager)
};

//==============================================================================
// Data structures

/**
 * ParameterDiff - Represents a difference between two parameter values
 */
struct ParameterDiff
{
    juce::String parameterName;      // e.g., "DCO1 Range"
    juce::String section;            // e.g., "DCO1", "VCF", "Patch", "Arp"
    int index;                       // Parameter index in array
    int valueA;                      // Value in slot A
    int valueB;                      // Value in slot B
    int difference;                  // B - A (signed)
    juce::String valueADisplay;      // Human-readable value A (e.g., "16'")
    juce::String valueBDisplay;      // Human-readable value B (e.g., "8'")
    
    ParameterDiff() 
        : index(-1), valueA(0), valueB(0), difference(0) {}
    
    ParameterDiff(const juce::String& name, const juce::String& sec, int idx, 
                  int a, int b, const juce::String& dispA = "", const juce::String& dispB = "")
        : parameterName(name), section(sec), index(idx),
          valueA(a), valueB(b), difference(b - a),
          valueADisplay(dispA.isEmpty() ? juce::String(a) : dispA),
          valueBDisplay(dispB.isEmpty() ? juce::String(b) : dispB)
    {}
};

/**
 * FullPatch - Complete patch data including all parameters
 */
struct FullPatch
{
    PatchParameters patch;        // Patch-level parameters
    ToneParams toneUpper;     // Upper tone parameters
    ToneParams toneLower;     // Lower tone parameters
    ArpParameters arp;            // Arpeggiator parameters
    juce::String name;            // Patch name
    juce::Time timestamp;         // When patch was stored
    
    FullPatch() : timestamp(juce::Time::getCurrentTime()) {}
    
    FullPatch(const PatchParameters& p, const ToneParams& tu, 
              const ToneParams& tl, const ArpParameters& a, 
              const juce::String& n = "")
        : patch(p), toneUpper(tu), toneLower(tl), arp(a), 
          name(n), timestamp(juce::Time::getCurrentTime())
    {}
    
    // Comparison operator
    bool operator==(const FullPatch& other) const
    {
        // Simple struct comparison using memcmp
        bool toneUpperMatch = (memcmp(&toneUpper, &other.toneUpper, sizeof(ToneParams)) == 0);
        bool toneLowerMatch = (memcmp(&toneLower, &other.toneLower, sizeof(ToneParams)) == 0);
        bool patchMatch = (memcmp(&patch, &other.patch, sizeof(PatchParameters)) == 0);
        bool arpMatch = (memcmp(&arp, &other.arp, sizeof(ArpParameters)) == 0);
        
        return toneUpperMatch && toneLowerMatch && patchMatch && arpMatch;
    }
    
    bool operator!=(const FullPatch& other) const
    {
        return !(*this == other);
    }
    
    /**
     * Get total number of parameters in a full patch
     */
    static int getTotalParameterCount()
    {
        return 93 + 93 + 64 + 32; // toneUpper + toneLower + patch + arp
    }
};
