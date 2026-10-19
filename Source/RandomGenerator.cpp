/*
  ==============================================================================

    RandomGenerator.cpp
    Smart random patch generation implementation

  ==============================================================================
*/

#include "RandomGenerator.h"

//==============================================================================
RandomGenerator::RandomGenerator()
{
    random.setSeedRandomly();
    initializeCategoryTemplates();
}

RandomGenerator::~RandomGenerator()
{
}

//==============================================================================
// Generation Methods

FullPatch RandomGenerator::generate(SoundCategory category)
{
    auto it = categoryTemplates.find(category);
    if (it == categoryTemplates.end())
    {
        DBG("RandomGenerator: Category " + juce::String(static_cast<int>(category)) + " not found");
        return generateRandom();
    }
    
    const auto& tmpl = it->second;
    
    FullPatch patch;
    patch.toneUpper = generateToneParams(tmpl, false);
    patch.toneLower = generateToneParams(tmpl, true);
    patch.patch = generatePatchParams(tmpl);
    patch.arp = generateArpParams(tmpl);
    patch.name = tmpl.name + " " + juce::String(random.nextInt(1000));
    patch.timestamp = juce::Time::getCurrentTime();
    
    DBG("RandomGenerator: Generated " + tmpl.name + " patch");
    
    return patch;
}

FullPatch RandomGenerator::generateFromTemplate(const FullPatch& templatePatch, float variation)
{
    float savedRandomness = randomnessAmount;
    randomnessAmount = juce::jlimit(0.0f, 1.0f, variation);
    
    FullPatch patch = templatePatch;
    
    // Apply variation to tone, patch, and arp parameters
    applyVariation(patch.toneUpper, randomnessAmount);
    applyVariation(patch.toneLower, randomnessAmount);
    applyVariation(patch.patch, randomnessAmount);
    applyVariation(patch.arp, randomnessAmount);
    
    randomnessAmount = savedRandomness;
    
    patch.name = "Var " + templatePatch.name;
    patch.timestamp = juce::Time::getCurrentTime();
    
    DBG("RandomGenerator: Generated variation with " + juce::String(variation * 100.0f, 0) + "% variation");
    
    return patch;
}

FullPatch RandomGenerator::generateRandom()
{
    FullPatch patch;
    
    CategoryTemplate emptyTemplate;  // No constraints
    
    patch.toneUpper = generateToneParams(emptyTemplate, false);
    patch.toneLower = generateToneParams(emptyTemplate, true);
    patch.patch = generatePatchParams(emptyTemplate);
    patch.arp = generateArpParams(emptyTemplate);
    patch.name = "Random " + juce::String(random.nextInt(10000));
    patch.timestamp = juce::Time::getCurrentTime();
    
    DBG("RandomGenerator: Generated completely random patch");
    
    return patch;
}

//==============================================================================
// Parameter Control

void RandomGenerator::setParameterRange(const juce::String& param, int minValue, int maxValue)
{
    userParamRanges[param] = std::make_pair(minValue, maxValue);
    DBG("RandomGenerator: Set range for " + param + ": " + juce::String(minValue) + "-" + juce::String(maxValue));
}

void RandomGenerator::clearParameterRange(const juce::String& param)
{
    userParamRanges.erase(param);
}

void RandomGenerator::clearAllParameterRanges()
{
    userParamRanges.clear();
    DBG("RandomGenerator: Cleared all parameter ranges");
}

void RandomGenerator::lockParameter(const juce::String& param, int value)
{
    lockedParameters[param] = value;
    DBG("RandomGenerator: Locked " + param + " to " + juce::String(value));
}

void RandomGenerator::unlockParameter(const juce::String& param)
{
    lockedParameters.erase(param);
}

void RandomGenerator::clearAllLocks()
{
    lockedParameters.clear();
    DBG("RandomGenerator: Cleared all locked parameters");
}

void RandomGenerator::setRandomness(float amount)
{
    randomnessAmount = juce::jlimit(0.0f, 1.0f, amount);
    DBG("RandomGenerator: Set randomness to " + juce::String(randomnessAmount * 100.0f, 0) + "%");
}

void RandomGenerator::enableMusicalConstraints(bool enable)
{
    musicalConstraintsEnabled = enable;
    DBG("RandomGenerator: Musical constraints " + juce::String(enable ? "enabled" : "disabled"));
}

void RandomGenerator::setComplexity(float level)
{
    complexityLevel = juce::jlimit(0.0f, 1.0f, level);
    DBG("RandomGenerator: Set complexity to " + juce::String(complexityLevel * 100.0f, 0) + "%");
}

//==============================================================================
// Category Info

juce::String RandomGenerator::getCategoryName(SoundCategory category)
{
    switch (category)
    {
        case CAT_BASS:          return "Bass";
        case CAT_PAD:           return "Pad";
        case CAT_LEAD:          return "Lead";
        case CAT_BRASS:         return "Brass";
        case CAT_STRINGS:       return "Strings";
        case CAT_KEYS:          return "Keys";
        case CAT_BELLS:         return "Bells";
        case CAT_FX:            return "FX";
        case CAT_PERCUSSION:    return "Percussion";
        case CAT_EXPERIMENTAL:  return "Experimental";
        default:                return "Unknown";
    }
}

juce::String RandomGenerator::getCategoryDescription(SoundCategory category)
{
    switch (category)
    {
        case CAT_BASS:          return "Deep, powerful bass sounds";
        case CAT_PAD:           return "Warm, evolving pads";
        case CAT_LEAD:          return "Sharp, cutting leads";
        case CAT_BRASS:         return "Brass-like timbres";
        case CAT_STRINGS:       return "String ensemble sounds";
        case CAT_KEYS:          return "Piano/EP-like sounds";
        case CAT_BELLS:         return "Bell and chime tones";
        case CAT_FX:            return "Sound effects and atmospheres";
        case CAT_PERCUSSION:    return "Percussive sounds";
        case CAT_EXPERIMENTAL:  return "No constraints - wild sounds";
        default:                return "";
    }
}

//==============================================================================
// Template Initialization

void RandomGenerator::initializeCategoryTemplates()
{
    initializeBassTemplate();
    initializePadTemplate();
    initializeLeadTemplate();
    initializeBrassTemplate();
    initializeStringsTemplate();
    initializeKeysTemplate();
    initializeBellsTemplate();
    initializeFXTemplate();
    initializePercussionTemplate();
    initializeExperimentalTemplate();
}

void RandomGenerator::initializeBassTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "Bass";
    tmpl.description = "Deep, powerful bass sounds";
    
    // DCO - Low octaves, simple waveforms
    tmpl.paramRanges["dco1Range"] = {0, 1};  // 32', 16'
    tmpl.paramRanges["dco2Range"] = {0, 1};
    tmpl.paramRanges["dco1Waveform"] = {0, 1};  // Pulse, Saw
    tmpl.paramRanges["dco2Waveform"] = {0, 1};
    tmpl.paramRanges["dco2Tune"] = {48, 52};  // Near unison
    
    // VCF - Lower cutoff for warmth
    tmpl.paramRanges["vcfCutoff"] = {20, 60};
    tmpl.paramRanges["vcfResonance"] = {0, 40};
    tmpl.paramRanges["vcfENVMod"] = {30, 80};
    
    // ENV - Punchy attack, sustaining
    // tmpl.paramRanges["env1Attack"] = {0, 20};
    // tmpl.paramRanges["env1Decay"] = {30, 70};
    // tmpl.paramRanges["env1Sustain"] = {60, 100};
    // tmpl.paramRanges["env1Release"] = {20, 60};
    
    // VCA - Normal or gate
    tmpl.paramRanges["vcaLevel"] = {80, 100};
    
    categoryTemplates[CAT_BASS] = tmpl;
}

void RandomGenerator::initializePadTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "Pad";
    tmpl.description = "Warm, evolving pads";
    
    // DCO - Mid octaves, smooth waveforms
    tmpl.paramRanges["dco1Range"] = {1, 2};  // 16', 8'
    tmpl.paramRanges["dco2Range"] = {1, 2};
    tmpl.paramRanges["dco1Waveform"] = {0, 2};  // Pulse, Saw
    tmpl.paramRanges["dco2Waveform"] = {0, 2};
    tmpl.paramRanges["dco2FineTune"] = {45, 55};  // Slight detune
    
    // VCF - Open sound
    tmpl.paramRanges["vcfCutoff"] = {50, 90};
    tmpl.paramRanges["vcfResonance"] = {10, 50};
    tmpl.paramRanges["vcfLFOMod"] = {20, 60};
    
    // ENV - Slow attack and release
    // tmpl.paramRanges["env1Attack"] = {40, 80};
    // tmpl.paramRanges["env1Decay"] = {50, 80};
    // tmpl.paramRanges["env1Sustain"] = {70, 100};
    // tmpl.paramRanges["env1Release"] = {60, 100};
    
    // LFO - Slow modulation
    tmpl.paramRanges["lfoRate"] = {10, 40};
    tmpl.paramRanges["lfoDelay"] = {20, 60};
    
    // Chorus - Usually on for warmth
    tmpl.lockedValues["chorusMode"] = 1;  // On
    
    categoryTemplates[CAT_PAD] = tmpl;
}

void RandomGenerator::initializeLeadTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "Lead";
    tmpl.description = "Sharp, cutting leads";
    
    // DCO - High octaves, bright waveforms
    tmpl.paramRanges["dco1Range"] = {2, 3};  // 8', 4'
    tmpl.paramRanges["dco2Range"] = {2, 3};
    tmpl.paramRanges["dco1Waveform"] = {1, 2};  // Saw preferred
    tmpl.paramRanges["dco2Waveform"] = {1, 2};
    
    // VCF - Higher cutoff, more resonance
    tmpl.paramRanges["vcfCutoff"] = {60, 100};
    tmpl.paramRanges["vcfResonance"] = {20, 80};
    tmpl.paramRanges["vcfENVMod"] = {40, 100};
    tmpl.paramRanges["vcfKeyFollow"] = {60, 100};
    
    // ENV - Fast attack, moderate sustain
    // tmpl.paramRanges["env1Attack"] = {0, 30};
    // tmpl.paramRanges["env1Decay"] = {30, 60};
    // tmpl.paramRanges["env1Sustain"] = {50, 80};
    // tmpl.paramRanges["env1Release"] = {20, 50};
    
    // LFO - Vibrato
    tmpl.paramRanges["lfoRate"] = {30, 70};
    tmpl.paramRanges["dco1LFOMod"] = {20, 60};
    tmpl.paramRanges["dco2LFOMod"] = {20, 60};
    
    categoryTemplates[CAT_LEAD] = tmpl;
}

void RandomGenerator::initializeBrassTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "Brass";
    tmpl.description = "Brass-like timbres";
    
    // DCO - Mid-high octaves, pulse + saw
    tmpl.paramRanges["dco1Range"] = {2, 2};  // 8'
    tmpl.paramRanges["dco2Range"] = {2, 2};
    tmpl.lockedValues["dco1Waveform"] = 0;  // Pulse
    tmpl.lockedValues["dco2Waveform"] = 1;  // Saw
    
    // VCF - Dynamic filter movement
    tmpl.paramRanges["vcfCutoff"] = {40, 70};
    tmpl.paramRanges["vcfResonance"] = {30, 70};
    tmpl.paramRanges["vcfENVMod"] = {60, 100};
    tmpl.paramRanges["vcfDynamics"] = {50, 100};
    
    // ENV - Brass-like response
    // tmpl.paramRanges["env1Attack"] = {20, 50};
    // tmpl.paramRanges["env1Decay"] = {40, 70};
    // tmpl.paramRanges["env1Sustain"] = {60, 90};
    // tmpl.paramRanges["env1Release"] = {30, 60};
    
    // Dynamics important for brass
    tmpl.paramRanges["vcaDynamics"] = {60, 100};
    
    categoryTemplates[CAT_BRASS] = tmpl;
}

void RandomGenerator::initializeStringsTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "Strings";
    tmpl.description = "String ensemble sounds";
    
    // DCO - Mid octaves, smooth
    tmpl.paramRanges["dco1Range"] = {1, 2};  // 16', 8'
    tmpl.paramRanges["dco2Range"] = {1, 2};
    tmpl.lockedValues["dco1Waveform"] = 1;  // Saw
    tmpl.lockedValues["dco2Waveform"] = 1;  // Saw
    tmpl.paramRanges["dco2FineTune"] = {48, 52};  // Slight detune
    
    // VCF - Smooth filtering
    tmpl.paramRanges["vcfCutoff"] = {50, 80};
    tmpl.paramRanges["vcfResonance"] = {5, 30};
    tmpl.paramRanges["vcfENVMod"] = {20, 50};
    
    // ENV - Slow attack like bowing
    // tmpl.paramRanges["env1Attack"] = {30, 70};
    // tmpl.paramRanges["env1Decay"] = {40, 70};
    // tmpl.paramRanges["env1Sustain"] = {70, 100};
    // tmpl.paramRanges["env1Release"] = {40, 80};
    
    // Chorus always on for ensemble effect
    tmpl.lockedValues["chorusMode"] = 1;
    
    categoryTemplates[CAT_STRINGS] = tmpl;
}

void RandomGenerator::initializeKeysTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "Keys";
    tmpl.description = "Piano/EP-like sounds";
    
    // DCO - Standard octaves
    tmpl.paramRanges["dco1Range"] = {2, 2};  // 8'
    tmpl.paramRanges["dco2Range"] = {2, 3};  // 8', 4'
    
    // VCF - Bright, not too resonant
    tmpl.paramRanges["vcfCutoff"] = {60, 90};
    tmpl.paramRanges["vcfResonance"] = {0, 20};
    tmpl.paramRanges["vcfKeyFollow"] = {50, 100};
    
    // ENV - Piano-like decay
    // tmpl.paramRanges["env1Attack"] = {0, 10};
    // tmpl.paramRanges["env1Decay"] = {40, 80};
    // tmpl.paramRanges["env1Sustain"] = {0, 40};
    // tmpl.paramRanges["env1Release"] = {30, 70};
    
    // VCA - Level dynamics important
    tmpl.paramRanges["vcaDynamics"] = {50, 100};
    
    categoryTemplates[CAT_KEYS] = tmpl;
}

void RandomGenerator::initializeBellsTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "Bells";
    tmpl.description = "Bell and chime tones";
    
    // DCO - High octaves, inharmonic intervals
    tmpl.paramRanges["dco1Range"] = {3, 3};  // 4'
    tmpl.paramRanges["dco2Range"] = {3, 3};
    tmpl.paramRanges["dco2Tune"] = {53, 59};  // 5th + detune
    
    // VCF - Bright, ringing
    tmpl.paramRanges["vcfCutoff"] = {70, 100};
    tmpl.paramRanges["vcfResonance"] = {40, 80};
    tmpl.paramRanges["vcfENVMod"] = {60, 100};
    
    // ENV - Fast attack, long decay
    // tmpl.paramRanges["env1Attack"] = {0, 5};
    // tmpl.paramRanges["env1Decay"] = {60, 100};
    // tmpl.paramRanges["env1Sustain"] = {0, 30};
    // tmpl.paramRanges["env1Release"] = {70, 100};
    
    categoryTemplates[CAT_BELLS] = tmpl;
}

void RandomGenerator::initializeFXTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "FX";
    tmpl.description = "Sound effects and atmospheres";
    
    // Wide ranges for experimentation
    tmpl.paramRanges["dco1Range"] = {0, 3};
    tmpl.paramRanges["dco2Range"] = {0, 3};
    tmpl.paramRanges["dco2CrossMod"] = {30, 100};  // More cross-mod
    
    // Extreme filtering
    tmpl.paramRanges["vcfResonance"] = {40, 100};
    tmpl.paramRanges["vcfLFOMod"] = {40, 100};
    
    // LFO modulation important
    tmpl.paramRanges["lfoRate"] = {0, 100};
    tmpl.paramRanges["lfoDelay"] = {0, 100};
    
    categoryTemplates[CAT_FX] = tmpl;
}

void RandomGenerator::initializePercussionTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "Percussion";
    tmpl.description = "Percussive sounds";
    
    // VCF - Impact and decay
    tmpl.paramRanges["vcfCutoff"] = {30, 90};
    tmpl.paramRanges["vcfResonance"] = {20, 80};
    tmpl.paramRanges["vcfENVMod"] = {60, 100};
    
    // ENV - Very fast attack, quick decay
    // tmpl.paramRanges["env1Attack"] = {0, 5};
    // tmpl.paramRanges["env1Decay"] = {10, 50};
    // tmpl.paramRanges["env1Sustain"] = {0, 20};
    // tmpl.paramRanges["env1Release"] = {5, 30};
    
    // VCA - Gate mode
    tmpl.lockedValues["vcaMode"] = 0;  // Gate
    
    categoryTemplates[CAT_PERCUSSION] = tmpl;
}

void RandomGenerator::initializeExperimentalTemplate()
{
    CategoryTemplate tmpl;
    tmpl.name = "Experimental";
    tmpl.description = "No constraints - wild sounds";
    
    // No ranges - full random
    
    categoryTemplates[CAT_EXPERIMENTAL] = tmpl;
}

//==============================================================================
// Generation Helpers

ToneParams RandomGenerator::generateToneParams(const CategoryTemplate& tmpl, bool isLower)
{
    ToneParams tone;
    
    // DCO-1
    tone.dco1Range = generateValue("dco1Range", 0, 3, tmpl);
    tone.dco1Waveform = generateValue("dco1Waveform", 0, 3, tmpl);
    tone.dco1Tune = generateValue("dco1Tune", 36, 60, tmpl);
    tone.dco1LFOMod = generateValue("dco1LFOMod", 0, 100, tmpl);
    tone.dco1ENVMod = generateValue("dco1ENVMod", 0, 100, tmpl);
    
    // DCO-2
    tone.dco2Range = generateValue("dco2Range", 0, 3, tmpl);
    tone.dco2Waveform = generateValue("dco2Waveform", 0, 3, tmpl);
    tone.dco2CrossMod = generateValue("dco2CrossMod", 0, 3, tmpl);
    tone.dco2Tune = generateValue("dco2Tune", 36, 60, tmpl);
    tone.dco2FineTune = generateValue("dco2FineTune", 0, 100, tmpl);
    tone.dco2LFOMod = generateValue("dco2LFOMod", 0, 100, tmpl);
    tone.dco2ENVMod = generateValue("dco2ENVMod", 0, 100, tmpl);
    
    // DCO Common
    tone.dcoDynamics = generateValue("dcoDynamics", 0, 100, tmpl);
    tone.dcoENVMode = generateValue("dcoENVMode", 0, 3, tmpl);
    
    // Mixer
    tone.mixerDCO1Level = generateValue("mixerDCO1Level", 50, 100, tmpl);
    tone.mixerDCO2Level = generateValue("mixerDCO2Level", 50, 100, tmpl);
    tone.mixerENVMod = generateValue("mixerENVMod", 0, 100, tmpl);
    tone.mixerDynamics = generateValue("mixerDynamics", 0, 100, tmpl);
    tone.mixerENVMode = generateValue("mixerENVMode", 0, 3, tmpl);
    
    // VCF
    tone.vcfHPF = generateValue("vcfHPF", 0, 3, tmpl);
    tone.vcfCutoff = generateValue("vcfCutoff", 0, 100, tmpl);
    tone.vcfResonance = generateValue("vcfResonance", 0, 100, tmpl);
    tone.vcfLFOMod = generateValue("vcfLFOMod", 0, 100, tmpl);
    tone.vcfENVMod = generateValue("vcfENVMod", 0, 100, tmpl);
    tone.vcfKeyFollow = generateValue("vcfKeyFollow", 0, 100, tmpl);
    tone.vcfDynamics = generateValue("vcfDynamics", 0, 100, tmpl);
    tone.vcfENVMode = generateValue("vcfENVMode", 0, 3, tmpl);
    
    // VCA
    tone.vcaMode = generateValue("vcaMode", 0, 1, tmpl);
    tone.vcaLevel = generateValue("vcaLevel", 50, 100, tmpl);
    tone.vcaDynamics = generateValue("vcaDynamics", 0, 100, tmpl);
    
    // ENV-1
    tone.env1Time1 = generateValue("env1Time1", 0, 100, tmpl);
    tone.env1Level1 = generateValue("env1Level1", 0, 100, tmpl);
    tone.env1Time2 = generateValue("env1Time2", 0, 100, tmpl);
    tone.env1Level2 = generateValue("env1Level2", 0, 100, tmpl);
    tone.env1Time3 = generateValue("env1Time3", 0, 100, tmpl);
    tone.env1Level3 = generateValue("env1Level3", 0, 100, tmpl);
    tone.env1Time4 = generateValue("env1Time4", 0, 100, tmpl);
    tone.env1KeyFollow = generateValue("env1KeyFollow", 0, 100, tmpl);
    
    // ENV-2
    tone.env2Time1 = generateValue("env2Time1", 0, 100, tmpl);
    tone.env2Level1 = generateValue("env2Level1", 0, 100, tmpl);
    tone.env2Time2 = generateValue("env2Time2", 0, 100, tmpl);
    tone.env2Level2 = generateValue("env2Level2", 0, 100, tmpl);
    tone.env2Time3 = generateValue("env2Time3", 0, 100, tmpl);
    tone.env2Level3 = generateValue("env2Level3", 0, 100, tmpl);
    tone.env2Time4 = generateValue("env2Time4", 0, 100, tmpl);
    tone.env2KeyFollow = generateValue("env2KeyFollow", 0, 100, tmpl);
    
    // LFO
    tone.lfo1Waveform = generateValue("lfo1Waveform", 0, 3, tmpl);
    tone.lfo1Delay = generateValue("lfo1Delay", 0, 127, tmpl);
    tone.lfo1Rate = generateValue("lfo1Rate", 0, 127, tmpl);
    
    // Chorus
    tone.chorusMode = generateValue("chorusMode", 0, 1, tmpl);
    
    // Apply musical constraints if enabled
    if (musicalConstraintsEnabled)
    {
        applyMusicalConstraints(tone);
    }
    
    // Apply complexity adjustments
    applyComplexity(tone);
    
    return tone;
}

PatchParameters RandomGenerator::generatePatchParams(const CategoryTemplate& tmpl)
{
    PatchParameters patch;
    
    // Initialize name
    std::memset(patch.toneName, 0, sizeof(patch.toneName));
    std::memset(patch.patchName, 0, sizeof(patch.patchName));
    
    patch.upperLowerBalance = generateValue("upperLowerBalance", 0, 100, tmpl);
    patch.upperSplit = generateValue("upperSplit", 36, 84, tmpl);
    patch.lowerSplit = generateValue("lowerSplit", 36, 84, tmpl);
    patch.totalVolume = generateValue("totalVolume", 80, 100, tmpl);
    
    patch.toneSel = generateValue("toneSel", 0, 2, tmpl);
    patch.keyMode = generateValue("keyMode", 0, 3, tmpl);
    patch.splitPoint = generateValue("splitPoint", 36, 84, tmpl);
    patch.balance = generateValue("balance", 0, 100, tmpl);
    
    patch.portamentoSwitch = generateValue("portamentoSwitch", 0, 1, tmpl);
    patch.portamentoTime = generateValue("portamentoTime", 0, 100, tmpl);
    
    patch.bendRange = generateValue("bendRange", 0, 12, tmpl);
    patch.assignMode = generateValue("assignMode", 0, 3, tmpl);
    patch.glide = generateValue("glide", 0, 100, tmpl);
    
    return patch;
}

ArpParameters RandomGenerator::generateArpParams(const CategoryTemplate& tmpl)
{
    ArpParameters arp;
    
    arp.arpOn = generateValue("arpOn", 0, 1, tmpl);
    arp.tempo = generateValue("tempo", 40, 200, tmpl);
    arp.mode = generateValue("mode", 0, 5, tmpl);
    arp.octaveSpan = generateValue("octaveSpan", 1, 4, tmpl);
    arp.insert = generateValue("insert", 0, 1, tmpl);
    arp.playRate = generateValue("playRate", 0, 7, tmpl);
    arp.duration = generateValue("duration", 0, 100, tmpl);
    arp.legato = generateValue("legato", 0, 1, tmpl);
    arp.velocityMode = generateValue("velocityMode", 0, 2, tmpl);
    arp.fixedVelocity = generateValue("fixedVelocity", 0, 127, tmpl);
    arp.midiOut = generateValue("midiOut", 0, 1, tmpl);
    arp.sync = generateValue("sync", 0, 1, tmpl);
    arp.latch = generateValue("latch", 0, 1, tmpl);
    
    return arp;
}

int RandomGenerator::generateValue(const juce::String& paramName, int defaultMin, int defaultMax, 
                                   const CategoryTemplate& tmpl)
{
    // Check if parameter is locked
    auto lockedIt = lockedParameters.find(paramName);
    if (lockedIt != lockedParameters.end())
        return lockedIt->second;
    
    // Check template locked values
    auto tmplLockedIt = tmpl.lockedValues.find(paramName);
    if (tmplLockedIt != tmpl.lockedValues.end())
        return tmplLockedIt->second;
    
    // Determine range
    int minVal = defaultMin;
    int maxVal = defaultMax;
    
    // User range overrides template
    auto userIt = userParamRanges.find(paramName);
    if (userIt != userParamRanges.end())
    {
        minVal = userIt->second.first;
        maxVal = userIt->second.second;
    }
    else
    {
        // Use template range if available
        auto tmplIt = tmpl.paramRanges.find(paramName);
        if (tmplIt != tmpl.paramRanges.end())
        {
            minVal = tmplIt->second.first;
            maxVal = tmplIt->second.second;
        }
    }
    // Clamp to allowed domain defined by defaults
    minVal = juce::jlimit(defaultMin, defaultMax, minVal);
    maxVal = juce::jlimit(defaultMin, defaultMax, maxVal);
    if (minVal > maxVal) std::swap(minVal, maxVal);
    
    // Generate random value in range
    return random.nextInt(maxVal - minVal + 1) + minVal;
}

//==============================================================================
// Musical Constraints

void RandomGenerator::applyMusicalConstraints(ToneParams& tone)
{
    constrainEnvelopes(tone);
    constrainFilterResonance(tone);
    constrainDetuning(tone);
    constrainLFO(tone);
}

void RandomGenerator::constrainEnvelopes(ToneParams& tone)
{
    // Ensure time1 + time2 isn't too extreme
    if (tone.env1Time1 > 80 && tone.env1Time2 < 20)
        tone.env1Time2 = 30;
    
    if (tone.env1Time1 < 10 && tone.env1Time4 > 90)
        tone.env1Time4 = 60;  // Balance fast time1 with moderate time4
    
    // ENV-2 similar constraints
    if (tone.env2Time1 > 80 && tone.env2Time2 < 20)
        tone.env2Time2 = 30;
}

void RandomGenerator::constrainFilterResonance(ToneParams& tone)
{
    // High resonance requires careful cutoff
    if (tone.vcfResonance > 80)
    {
        tone.vcfCutoff = clamp(tone.vcfCutoff, 30, 80);  // Avoid extremes
    }
    
    // If cutoff is very low, reduce resonance to avoid mud
    if (tone.vcfCutoff < 20)
    {
        tone.vcfResonance = clamp(tone.vcfResonance, 0, 40);
    }
}

void RandomGenerator::constrainDetuning(ToneParams& tone)
{
    // Keep fine tuning reasonable
    if (tone.dco2FineTune < 20 || tone.dco2FineTune > 80)
    {
        tone.dco2FineTune = 45 + random.nextInt(10);  // 45-54 range
    }
}

void RandomGenerator::constrainLFO(ToneParams& tone)
{
    // Very fast LFO can be harsh
    if (tone.lfo1Rate > 90)
    {
        // Reduce modulation amounts
        if (tone.dco1LFOMod > 50) tone.dco1LFOMod = 50;
        if (tone.dco2LFOMod > 50) tone.dco2LFOMod = 50;
        if (tone.vcfLFOMod > 50) tone.vcfLFOMod = 50;
    }
}

//==============================================================================
// Complexity Adjustments

void RandomGenerator::applyComplexity(ToneParams& tone)
{
    // Simple sounds (low complexity) use fewer modulation sources
    if (complexityLevel < 0.3f)
    {
        tone.mixerENVMod = clamp(tone.mixerENVMod, 0, 30);
        tone.dco1ENVMod = clamp(tone.dco1ENVMod, 0, 20);
        tone.dco2ENVMod = clamp(tone.dco2ENVMod, 0, 20);
    }
    // Complex sounds use more modulation
    else if (complexityLevel > 0.7f)
    {
        applyComplexityToModulation(tone);
    }
}

void RandomGenerator::applyComplexityToMixer(ToneParams& tone)
{
    // Complex sounds might have dynamic mixer
    if (randomChance(complexityLevel))
    {
        tone.mixerENVMod = random.nextInt(100);
        tone.mixerDynamics = random.nextInt(100);
    }
}

void RandomGenerator::applyComplexityToModulation(ToneParams& tone)
{
    // Add modulation routing for complex sounds
    if (randomChance(complexityLevel * 0.7f))
    {
        tone.vcfLFOMod = 30 + random.nextInt(70);
    }
    
    if (randomChance(complexityLevel * 0.5f))
    {
        tone.dco2CrossMod = 20 + random.nextInt(60);
    }
}

//==============================================================================
// Utilities

int RandomGenerator::clamp(int value, int min, int max)
{
    return juce::jlimit(min, max, value);
}

float RandomGenerator::randomFloat(float min, float max)
{
    return min + random.nextFloat() * (max - min);
}

bool RandomGenerator::randomChance(float probability)
{
    return random.nextFloat() < probability;
}

//==============================================================================
// Variation helpers

int RandomGenerator::jitter(int value, int min, int max, float amount)
{
    int range = max - min;
    float scale = juce::jlimit(0.0f, 1.0f, amount);
    int delta = (int) std::round((random.nextFloat() * 2.0f - 1.0f) * scale * (float) range);
    int out = value + delta;
    return clamp(out, min, max);
}

void RandomGenerator::applyVariation(ToneParams& t, float amount)
{
    // DCO-1
    t.dco1Range      = jitter(t.dco1Range, 0, 3, amount);
    t.dco1Waveform   = jitter(t.dco1Waveform, 0, 3, amount);
    t.dco1Tune       = jitter(t.dco1Tune, 0, 127, amount);
    t.dco1LFOMod     = jitter(t.dco1LFOMod, 0, 127, amount);
    t.dco1ENVMod     = jitter(t.dco1ENVMod, 0, 127, amount);

    // DCO-2
    t.dco2Range      = jitter(t.dco2Range, 0, 3, amount);
    t.dco2Waveform   = jitter(t.dco2Waveform, 0, 3, amount);
    t.dco2CrossMod   = jitter(t.dco2CrossMod, 0, 3, amount);
    t.dco2Tune       = jitter(t.dco2Tune, 0, 127, amount);
    t.dco2FineTune   = jitter(t.dco2FineTune, 0, 127, amount);
    t.dco2LFOMod     = jitter(t.dco2LFOMod, 0, 127, amount);
    t.dco2ENVMod     = jitter(t.dco2ENVMod, 0, 127, amount);

    // Mixer
    t.mixerDCO1Level = jitter(t.mixerDCO1Level, 0, 127, amount);
    t.mixerDCO2Level = jitter(t.mixerDCO2Level, 0, 127, amount);
    t.mixerENVMod    = jitter(t.mixerENVMod, 0, 127, amount);
    t.mixerDynamics  = jitter(t.mixerDynamics, 0, 3, amount);
    t.mixerENVMode   = jitter(t.mixerENVMode, 0, 3, amount);

    // VCF
    t.vcfHPF         = jitter(t.vcfHPF, 0, 3, amount);
    t.vcfCutoff      = jitter(t.vcfCutoff, 0, 127, amount);
    t.vcfResonance   = jitter(t.vcfResonance, 0, 127, amount);
    t.vcfLFOMod      = jitter(t.vcfLFOMod, 0, 127, amount);
    t.vcfENVMod      = jitter(t.vcfENVMod, 0, 127, amount);
    t.vcfKeyFollow   = jitter(t.vcfKeyFollow, 0, 127, amount);
    t.vcfDynamics    = jitter(t.vcfDynamics, 0, 3, amount);
    t.vcfENVMode     = jitter(t.vcfENVMode, 0, 3, amount);

    // VCA
    t.vcaMode        = jitter(t.vcaMode, 0, 1, amount);
    t.vcaLevel       = jitter(t.vcaLevel, 0, 127, amount);
    t.vcaDynamics    = jitter(t.vcaDynamics, 0, 3, amount);

    // ENV-1
    t.env1Time1      = jitter(t.env1Time1, 0, 127, amount);
    t.env1Level1     = jitter(t.env1Level1, 0, 127, amount);
    t.env1Time2      = jitter(t.env1Time2, 0, 127, amount);
    t.env1Level2     = jitter(t.env1Level2, 0, 127, amount);
    t.env1Time3      = jitter(t.env1Time3, 0, 127, amount);
    t.env1Level3     = jitter(t.env1Level3, 0, 127, amount);
    t.env1Time4      = jitter(t.env1Time4, 0, 127, amount);
    t.env1KeyFollow  = jitter(t.env1KeyFollow, 0, 3, amount);

    // ENV-2
    t.env2Time1      = jitter(t.env2Time1, 0, 127, amount);
    t.env2Level1     = jitter(t.env2Level1, 0, 127, amount);
    t.env2Time2      = jitter(t.env2Time2, 0, 127, amount);
    t.env2Level2     = jitter(t.env2Level2, 0, 127, amount);
    t.env2Time3      = jitter(t.env2Time3, 0, 127, amount);
    t.env2Level3     = jitter(t.env2Level3, 0, 127, amount);
    t.env2Time4      = jitter(t.env2Time4, 0, 127, amount);
    t.env2KeyFollow  = jitter(t.env2KeyFollow, 0, 3, amount);

    // LFO and Chorus
    t.lfo1Waveform    = jitter(t.lfo1Waveform, 0, 3, amount);
    t.lfo1Delay       = jitter(t.lfo1Delay, 0, 127, amount);
    t.lfo1Rate        = jitter(t.lfo1Rate, 0, 127, amount);
    t.chorusMode     = jitter(t.chorusMode, 0, 2, amount);

    // Re-apply constraints if enabled
    if (musicalConstraintsEnabled)
        applyMusicalConstraints(t);
}

void RandomGenerator::applyVariation(PatchParameters& p, float amount)
{
    p.upperLowerBalance = jitter(p.upperLowerBalance, 0, 100, amount);
    p.upperSplit        = jitter(p.upperSplit, 36, 84, amount);
    p.lowerSplit        = jitter(p.lowerSplit, 36, 84, amount);
    p.totalVolume       = jitter(p.totalVolume, 0, 100, amount);
    p.toneSel           = jitter(p.toneSel, 0, 2, amount);
    p.keyMode           = jitter(p.keyMode, 0, 3, amount);
    p.splitPoint        = jitter(p.splitPoint, 36, 84, amount);
    p.balance           = jitter(p.balance, 0, 100, amount);
    p.portamentoSwitch  = jitter(p.portamentoSwitch, 0, 1, amount);
    p.portamentoTime    = jitter(p.portamentoTime, 0, 100, amount);
    p.bendRange         = jitter(p.bendRange, 0, 12, amount);
    p.assignMode        = jitter(p.assignMode, 0, 3, amount);
    p.glide             = jitter(p.glide, 0, 100, amount);
}

void RandomGenerator::applyVariation(ArpParameters& a, float amount)
{
    a.arpOn         = jitter(a.arpOn, 0, 1, amount);
    a.tempo         = jitter(a.tempo, 40, 200, amount);
    a.mode          = jitter(a.mode, 0, 5, amount);
    a.octaveSpan    = jitter(a.octaveSpan, 1, 4, amount);
    a.insert        = jitter(a.insert, 0, 1, amount);
    a.playRate      = jitter(a.playRate, 0, 7, amount);
    a.duration      = jitter(a.duration, 0, 100, amount);
    a.legato        = jitter(a.legato, 0, 1, amount);
    a.velocityMode  = jitter(a.velocityMode, 0, 2, amount);
    a.fixedVelocity = jitter(a.fixedVelocity, 0, 127, amount);
    a.midiOut       = jitter(a.midiOut, 0, 1, amount);
    a.sync          = jitter(a.sync, 0, 1, amount);
    a.latch         = jitter(a.latch, 0, 1, amount);
}
