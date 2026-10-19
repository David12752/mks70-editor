/*
  ==============================================================================

    MorphEngine.cpp
    Morph/Crossfade engine implementation

  ==============================================================================
*/

#include "MorphEngine.h"
#include <cmath>

MorphEngine::MorphEngine()
{
    random.setSeedRandomly();
}

MorphEngine::~MorphEngine()
{
    stopAutoMorph();
}

//==============================================================================
// Source patch management

void MorphEngine::setSourceA(const FullPatch& patch)
{
    sourceA = patch;
    hasSourceA = true;
    DBG("MorphEngine: Source A set");
}

void MorphEngine::setSourceB(const FullPatch& patch)
{
    sourceB = patch;
    hasSourceB = true;
    DBG("MorphEngine: Source B set");
}

bool MorphEngine::hasSources() const
{
    return hasSourceA && hasSourceB;
}

//==============================================================================
// Morphing

FullPatch MorphEngine::morph(float position)
{
    if (!hasSources())
    {
        DBG("MorphEngine: Cannot morph - sources not set");
        return sourceA;
    }
    
    // Clamp position
    position = juce::jlimit(0.0f, 1.0f, position);
    
    FullPatch result;
    
    // Morph tone parameters
    result.toneUpper = morphToneParams(sourceA.toneUpper, sourceB.toneUpper, position);
    result.toneLower = morphToneParams(sourceA.toneLower, sourceB.toneLower, position);
    
    // Morph patch parameters
    result.patch = morphPatchParams(sourceA.patch, sourceB.patch, position);
    
    // Morph arpeggiator parameters
    result.arp = morphArpParams(sourceA.arp, sourceB.arp, position);
    
    // Create morphed name
    result.name = juce::String("Morph ") + juce::String(static_cast<int>(position * 100)) + "%";
    result.timestamp = juce::Time::getCurrentTime();
    
    return result;
}

//==============================================================================
// Per-parameter control

void MorphEngine::setParameterMorphCurve(const juce::String& paramName, MorphCurve curve)
{
    parameterCurves[paramName] = curve;
    DBG("MorphEngine: Set curve for " + paramName + " to " + juce::String(static_cast<int>(curve)));
}

void MorphEngine::excludeParameter(const juce::String& paramName)
{
    excludedParameters.insert(paramName);
    DBG("MorphEngine: Excluded parameter " + paramName);
}

void MorphEngine::includeParameter(const juce::String& paramName)
{
    excludedParameters.erase(paramName);
    DBG("MorphEngine: Included parameter " + paramName);
}

bool MorphEngine::isParameterExcluded(const juce::String& paramName) const
{
    return excludedParameters.find(paramName) != excludedParameters.end();
}

void MorphEngine::clearExclusions()
{
    excludedParameters.clear();
    DBG("MorphEngine: Cleared all exclusions");
}

MorphEngine::MorphCurve MorphEngine::getParameterCurve(const juce::String& paramName) const
{
    auto it = parameterCurves.find(paramName);
    if (it != parameterCurves.end())
        return it->second;
    
    return defaultCurve;
}

//==============================================================================
// Animation

void MorphEngine::startAutoMorph(float durationSeconds, MorphMode mode)
{
    morphDuration = juce::jmax(0.1f, durationSeconds);
    currentMode = mode;
    currentPosition = 0.0f;
    morphDirection = true;
    startTime = juce::Time::currentTimeMillis();
    autoMorphActive = true;
    
    // For random walk, set initial target
    if (mode == RANDOM_WALK)
    {
        randomWalkTarget = random.nextFloat();
    }
    
    // Start timer
    int intervalMs = 1000 / updateRateHz;
    startTimer(intervalMs);
    
    DBG("MorphEngine: Started auto-morph, duration=" + juce::String(durationSeconds) + 
        "s, mode=" + juce::String(static_cast<int>(mode)));
}

void MorphEngine::stopAutoMorph()
{
    if (autoMorphActive)
    {
        stopTimer();
        autoMorphActive = false;
        DBG("MorphEngine: Stopped auto-morph");
    }
}

void MorphEngine::setUpdateRate(int hz)
{
    updateRateHz = juce::jlimit(1, 120, hz);
    
    if (autoMorphActive)
    {
        int intervalMs = 1000 / updateRateHz;
        startTimer(intervalMs);
    }
}

//==============================================================================
// Listener management

void MorphEngine::addListener(Listener* listener)
{
    listeners.add(listener);
}

void MorphEngine::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

//==============================================================================
// Timer callback for auto-morph

void MorphEngine::timerCallback()
{
    if (!autoMorphActive)
        return;
    
    juce::int64 elapsed = juce::Time::currentTimeMillis() - startTime;
    float progress = static_cast<float>(elapsed) / (morphDuration * 1000.0f);
    
    switch (currentMode)
    {
        case LINEAR:
            currentPosition = juce::jlimit(0.0f, 1.0f, progress);
            
            if (currentPosition >= 1.0f)
            {
                stopAutoMorph();
                listeners.call(&Listener::morphCompleted);
            }
            break;
            
        case PINGPONG:
        {
            float cycle = std::fmod(progress, 2.0f);
            if (cycle <= 1.0f)
                currentPosition = cycle;
            else
                currentPosition = 2.0f - cycle;
            break;
        }
            
        case RANDOM_WALK:
        {
            // Smooth random walk towards target
            float speed = 0.02f;  // Adjust for smoothness
            float diff = randomWalkTarget - currentPosition;
            currentPosition += diff * speed;
            
            // When close to target, pick new target
            if (std::abs(diff) < 0.05f)
            {
                randomWalkTarget = random.nextFloat();
            }
            break;
        }
            
        case LFO_SINE:
        {
            float angle = progress * juce::MathConstants<float>::twoPi;
            currentPosition = (std::sin(angle) + 1.0f) * 0.5f;
            break;
        }
            
        case LFO_TRIANGLE:
        {
            float cycle = std::fmod(progress, 1.0f);
            if (cycle <= 0.5f)
                currentPosition = cycle * 2.0f;
            else
                currentPosition = 2.0f - (cycle * 2.0f);
            break;
        }
    }
    
    // Notify listeners
    listeners.call(&Listener::morphPositionChanged, currentPosition);
}

//==============================================================================
// Interpolation helpers

float MorphEngine::applyCurve(float position, MorphCurve curve)
{
    switch (curve)
    {
        case CURVE_LINEAR:
            return position;
            
        case CURVE_EXPONENTIAL:
            return position * position;
            
        case CURVE_LOGARITHMIC:
            return std::sqrt(position);
            
        case CURVE_SCURVE:
            // Smoothstep function
            return position * position * (3.0f - 2.0f * position);
            
        case CURVE_STEPPED:
            return position < 0.5f ? 0.0f : 1.0f;
            
        default:
            return position;
    }
}

int MorphEngine::interpolateValue(int valueA, int valueB, float position, MorphCurve curve)
{
    // Apply curve to position
    float curvedPosition = applyCurve(position, curve);
    
    // Linear interpolation with curved position
    float result = valueA + (valueB - valueA) * curvedPosition;
    
    return static_cast<int>(std::round(result));
}

//==============================================================================
// Tone parameter morphing

ToneParams MorphEngine::morphToneParams(const ToneParams& a, const ToneParams& b, float position)
{
    ToneParams result;
    
    // Helper macro for morphing a parameter
    #define MORPH_PARAM(name) \
        result.name = isParameterExcluded(#name) ? a.name : \
                      interpolateValue(a.name, b.name, position, getParameterCurve(#name))
    
    // DCO-1 Section
    MORPH_PARAM(dco1Range);
    MORPH_PARAM(dco1Waveform);
    MORPH_PARAM(dco1Tune);
    MORPH_PARAM(dco1LFOMod);
    MORPH_PARAM(dco1ENVMod);
    
    // DCO-2 Section
    MORPH_PARAM(dco2Range);
    MORPH_PARAM(dco2Waveform);
    MORPH_PARAM(dco2CrossMod);
    MORPH_PARAM(dco2Tune);
    MORPH_PARAM(dco2FineTune);
    MORPH_PARAM(dco2LFOMod);
    MORPH_PARAM(dco2ENVMod);
    
    // DCO Common
    MORPH_PARAM(dcoDynamics);
    MORPH_PARAM(dcoENVMode);
    
    // Mixer Section
    MORPH_PARAM(mixerDCO1Level);
    MORPH_PARAM(mixerDCO2Level);
    MORPH_PARAM(mixerENVMod);
    MORPH_PARAM(mixerDynamics);
    MORPH_PARAM(mixerENVMode);
    
    // VCF Section
    MORPH_PARAM(vcfHPF);
    MORPH_PARAM(vcfCutoff);
    MORPH_PARAM(vcfResonance);
    MORPH_PARAM(vcfLFOMod);
    MORPH_PARAM(vcfENVMod);
    MORPH_PARAM(vcfKeyFollow);
    MORPH_PARAM(vcfDynamics);
    MORPH_PARAM(vcfENVMode);
    
    // VCA Section
    MORPH_PARAM(vcaMode);
    MORPH_PARAM(vcaLevel);
    MORPH_PARAM(vcaDynamics);
    
    // ENV-1 Section (7-Stage)
    MORPH_PARAM(env1Time1);
    MORPH_PARAM(env1Level1);
    MORPH_PARAM(env1Time2);
    MORPH_PARAM(env1Level2);
    MORPH_PARAM(env1Time3);
    MORPH_PARAM(env1Level3);
    MORPH_PARAM(env1Time4);
    MORPH_PARAM(env1KeyFollow);
    
    // ENV-2 Section (7-Stage)
    MORPH_PARAM(env2Time1);
    MORPH_PARAM(env2Level1);
    MORPH_PARAM(env2Time2);
    MORPH_PARAM(env2Level2);
    MORPH_PARAM(env2Time3);
    MORPH_PARAM(env2Level3);
    MORPH_PARAM(env2Time4);
    MORPH_PARAM(env2KeyFollow);
    
    // LFO Section
    MORPH_PARAM(lfo1Waveform);
    MORPH_PARAM(lfo1Delay);
    MORPH_PARAM(lfo1Rate);
    
    // Chorus
    MORPH_PARAM(chorusMode);
    
    #undef MORPH_PARAM
    
    return result;
}

//==============================================================================
// Patch parameter morphing

PatchParameters MorphEngine::morphPatchParams(const PatchParameters& a, const PatchParameters& b, float position)
{
    PatchParameters result;
    
    // Copy names from source A
    std::memcpy(result.toneName, a.toneName, sizeof(a.toneName));
    std::memcpy(result.patchName, a.patchName, sizeof(a.patchName));
    
    #define MORPH_PATCH_PARAM(name) \
        result.name = isParameterExcluded("patch_" #name) ? a.name : \
                      interpolateValue(a.name, b.name, position, getParameterCurve("patch_" #name))
    
    // Bank & Patch (don't morph - use source A)
    result.bankMSB = a.bankMSB;
    result.bankLSB = a.bankLSB;
    result.patchNumber = a.patchNumber;
    
    // Global Balance & Split
    MORPH_PATCH_PARAM(upperLowerBalance);
    MORPH_PATCH_PARAM(upperSplit);
    MORPH_PATCH_PARAM(lowerSplit);
    MORPH_PATCH_PARAM(totalVolume);
    
    // Tone Selection & Key Mode
    MORPH_PATCH_PARAM(toneSel);
    MORPH_PATCH_PARAM(keyMode);
    MORPH_PATCH_PARAM(splitPoint);
    MORPH_PATCH_PARAM(balance);
    
    // Portamento
    MORPH_PATCH_PARAM(portamentoSwitch);
    MORPH_PATCH_PARAM(portamentoTime);
    
    // Bender
    MORPH_PATCH_PARAM(bendRange);
    
    // Assignment
    MORPH_PATCH_PARAM(assignMode);
    MORPH_PATCH_PARAM(glide);
    
    #undef MORPH_PATCH_PARAM
    
    return result;
}

//==============================================================================
// Arpeggiator parameter morphing

ArpParameters MorphEngine::morphArpParams(const ArpParameters& a, const ArpParameters& b, float position)
{
    ArpParameters result;
    
    #define MORPH_ARP_PARAM(name) \
        result.name = isParameterExcluded("arp_" #name) ? a.name : \
                      interpolateValue(a.name, b.name, position, getParameterCurve("arp_" #name))
    
    // Arpeggiator Control
    MORPH_ARP_PARAM(arpOn);
    MORPH_ARP_PARAM(tempo);
    MORPH_ARP_PARAM(mode);
    MORPH_ARP_PARAM(octaveSpan);
    MORPH_ARP_PARAM(insert);
    MORPH_ARP_PARAM(playRate);
    MORPH_ARP_PARAM(duration);
    MORPH_ARP_PARAM(legato);
    MORPH_ARP_PARAM(velocityMode);
    MORPH_ARP_PARAM(fixedVelocity);
    MORPH_ARP_PARAM(midiOut);
    MORPH_ARP_PARAM(sync);
    MORPH_ARP_PARAM(latch);
    
    #undef MORPH_ARP_PARAM
    
    return result;
}
