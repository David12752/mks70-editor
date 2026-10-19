/*
  ==============================================================================

    RandomGenerator.h
    Smart random patch generation with categories and constraints

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompareManager.h"
#include <map>
#include <set>

//==============================================================================
/**
 * RandomGenerator - Smart patch generation with musical constraints
 * Generates patches based on sound categories and templates
 */
class RandomGenerator
{
public:
    // Sound categories for patch generation
    enum SoundCategory
    {
        CAT_BASS,           // Deep, powerful bass sounds
        CAT_PAD,            // Warm, evolving pads
        CAT_LEAD,           // Sharp, cutting leads
        CAT_BRASS,          // Brass-like timbres
        CAT_STRINGS,        // String ensemble sounds
        CAT_KEYS,           // Piano/EP-like sounds
        CAT_BELLS,          // Bell and chime tones
        CAT_FX,             // Sound effects and atmospheres
        CAT_PERCUSSION,     // Percussive sounds
        CAT_EXPERIMENTAL    // No constraints
    };
    
    RandomGenerator();
    ~RandomGenerator();
    
    // Generation methods
    FullPatch generate(SoundCategory category);
    FullPatch generateFromTemplate(const FullPatch& templatePatch, float variation);
    FullPatch generateRandom();  // Completely random
    
    // Fine control
    void setParameterRange(const juce::String& param, int minValue, int maxValue);
    void clearParameterRange(const juce::String& param);
    void clearAllParameterRanges();
    
    void lockParameter(const juce::String& param, int value);
    void unlockParameter(const juce::String& param);
    void clearAllLocks();
    
    void setRandomness(float amount);  // 0.0 = template-only, 1.0 = full random
    float getRandomness() const { return randomnessAmount; }
    
    // Smart generation options
    void enableMusicalConstraints(bool enable);
    bool areMusicalConstraintsEnabled() const { return musicalConstraintsEnabled; }
    
    void setComplexity(float level);  // 0.0 = simple, 1.0 = complex
    float getComplexity() const { return complexityLevel; }
    
    // Category info
    static juce::String getCategoryName(SoundCategory category);
    static juce::String getCategoryDescription(SoundCategory category);

private:
    // Category template structure
    struct CategoryTemplate
    {
        juce::String name;
        juce::String description;
        std::map<juce::String, std::pair<int, int>> paramRanges;  // min, max
        std::map<juce::String, float> paramWeights;  // 0.0-1.0 probability of changing
        std::map<juce::String, int> lockedValues;  // Fixed values for category
        
        CategoryTemplate() = default;
    };
    
    // Random state
    juce::Random random;
    float randomnessAmount = 0.7f;  // Default to 70% randomness
    float complexityLevel = 0.5f;   // Default to medium complexity
    bool musicalConstraintsEnabled = true;
    
    // User overrides
    std::map<juce::String, std::pair<int, int>> userParamRanges;
    std::map<juce::String, int> lockedParameters;
    
    // Category templates
    std::map<SoundCategory, CategoryTemplate> categoryTemplates;
    
    // Initialization
    void initializeCategoryTemplates();
    void initializeBassTemplate();
    void initializePadTemplate();
    void initializeLeadTemplate();
    void initializeBrassTemplate();
    void initializeStringsTemplate();
    void initializeKeysTemplate();
    void initializeBellsTemplate();
    void initializeFXTemplate();
    void initializePercussionTemplate();
    void initializeExperimentalTemplate();
    
    // Generation helpers
    ToneParams generateToneParams(const CategoryTemplate& tmpl, bool isLower);
    PatchParameters generatePatchParams(const CategoryTemplate& tmpl);
    ArpParameters generateArpParams(const CategoryTemplate& tmpl);
    
    int generateValue(const juce::String& paramName, int defaultMin, int defaultMax, 
                     const CategoryTemplate& tmpl);
    int generateValueWithWeight(const juce::String& paramName, int min, int max, 
                               float weight, const CategoryTemplate& tmpl);
    
    // Musical constraints
    void applyMusicalConstraints(ToneParams& tone);
    void constrainEnvelopes(ToneParams& tone);
    void constrainFilterResonance(ToneParams& tone);
    void constrainDetuning(ToneParams& tone);
    void constrainLFO(ToneParams& tone);
    
    // Complexity adjustments
    void applyComplexity(ToneParams& tone);
    void applyComplexityToMixer(ToneParams& tone);
    void applyComplexityToModulation(ToneParams& tone);
    
    // Utilities
    int clamp(int value, int min, int max);
    float randomFloat(float min = 0.0f, float max = 1.0f);
    bool randomChance(float probability);  // Returns true with given probability
    
    // Variation helpers
    void applyVariation(ToneParams& tone, float amount);
    void applyVariation(PatchParameters& patch, float amount);
    void applyVariation(ArpParameters& arp, float amount);
    int jitter(int value, int min, int max, float amount);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RandomGenerator)
};
