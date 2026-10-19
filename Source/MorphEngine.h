/*
  ==============================================================================

    MorphEngine.h
    Morph/Crossfade engine for patch interpolation

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompareManager.h"

/**
 * MorphEngine - Interpolates between two patches with various curves and animation modes
 */
class MorphEngine : public juce::Timer
{
public:
    //==============================================================================
    // Morph modes for auto-morphing
    enum MorphMode
    {
        LINEAR,           // A→B
        PINGPONG,         // A→B→A→B...
        RANDOM_WALK,      // Random movement
        LFO_SINE,         // Sine wave
        LFO_TRIANGLE      // Triangle wave
    };
    
    // Interpolation curves for individual parameters
    enum MorphCurve
    {
        CURVE_LINEAR,
        CURVE_EXPONENTIAL,
        CURVE_LOGARITHMIC,
        CURVE_SCURVE,     // S-curve (smooth acceleration/deceleration)
        CURVE_STEPPED     // Jump only (no interpolation)
    };
    
    //==============================================================================
    MorphEngine();
    ~MorphEngine() override;
    
    //==============================================================================
    // Source patch management
    
    /**
     * Set source patch A
     * @param patch Patch data for position 0.0
     */
    void setSourceA(const FullPatch& patch);
    
    /**
     * Set source patch B
     * @param patch Patch data for position 1.0
     */
    void setSourceB(const FullPatch& patch);
    
    /**
     * Check if both sources are set
     * @return true if ready to morph
     */
    bool hasSources() const;
    
    //==============================================================================
    // Morphing
    
    /**
     * Generate morphed patch at specified position
     * @param position Morph position (0.0 = A, 1.0 = B)
     * @return Interpolated patch
     */
    FullPatch morph(float position);
    
    /**
     * Get current morph position (for auto-morph)
     * @return Current position between 0.0 and 1.0
     */
    float getCurrentPosition() const { return currentPosition; }
    
    //==============================================================================
    // Per-parameter control
    
    /**
     * Set interpolation curve for specific parameter
     * @param paramName Parameter name (e.g., "vcfCutoff")
     * @param curve Interpolation curve to use
     */
    void setParameterMorphCurve(const juce::String& paramName, MorphCurve curve);
    
    /**
     * Exclude parameter from morphing (keep at source A value)
     * @param paramName Parameter to exclude
     */
    void excludeParameter(const juce::String& paramName);
    
    /**
     * Include previously excluded parameter
     * @param paramName Parameter to include
     */
    void includeParameter(const juce::String& paramName);
    
    /**
     * Check if parameter is excluded
     * @param paramName Parameter to check
     * @return true if excluded
     */
    bool isParameterExcluded(const juce::String& paramName) const;
    
    /**
     * Clear all exclusions
     */
    void clearExclusions();
    
    /**
     * Get curve for parameter (returns default if not set)
     * @param paramName Parameter name
     * @return Curve type
     */
    MorphCurve getParameterCurve(const juce::String& paramName) const;
    
    //==============================================================================
    // Animation
    
    /**
     * Start automatic morphing animation
     * @param durationSeconds Time to complete one cycle
     * @param mode Animation mode
     */
    void startAutoMorph(float durationSeconds, MorphMode mode);
    
    /**
     * Stop automatic morphing
     */
    void stopAutoMorph();
    
    /**
     * Check if auto-morphing is active
     * @return true if animating
     */
    bool isAutoMorphing() const { return autoMorphActive; }
    
    /**
     * Set auto-morph update rate
     * @param hz Updates per second (default: 30)
     */
    void setUpdateRate(int hz);
    
    /**
     * Get current mode
     * @return Active morph mode
     */
    MorphMode getCurrentMode() const { return currentMode; }
    
    /**
     * Get morph duration
     * @return Duration in seconds
     */
    float getDuration() const { return morphDuration; }
    
    //==============================================================================
    // Listener interface for morph updates
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        /**
         * Called when morph position changes during auto-morph
         * @param newPosition Current position (0.0 - 1.0)
         */
        virtual void morphPositionChanged(float newPosition) = 0;
        
        /**
         * Called when auto-morph completes (LINEAR mode only)
         */
        virtual void morphCompleted() {}
    };
    
    void addListener(Listener* listener);
    void removeListener(Listener* listener);
    
private:
    //==============================================================================
    // Timer callback for auto-morph
    void timerCallback() override;
    
    // Interpolation helpers
    int interpolateValue(int valueA, int valueB, float position, MorphCurve curve);
    float applyCurve(float position, MorphCurve curve);
    
    // Tone parameter interpolation
    ToneParams morphToneParams(const ToneParams& a, const ToneParams& b, float position);
    
    // Patch parameter interpolation
    PatchParameters morphPatchParams(const PatchParameters& a, const PatchParameters& b, float position);
    
    // Arpeggiator parameter interpolation
    ArpParameters morphArpParams(const ArpParameters& a, const ArpParameters& b, float position);
    
    //==============================================================================
    // Source patches
    FullPatch sourceA;
    FullPatch sourceB;
    bool hasSourceA = false;
    bool hasSourceB = false;
    
    // Per-parameter settings
    std::map<juce::String, MorphCurve> parameterCurves;
    std::set<juce::String> excludedParameters;
    MorphCurve defaultCurve = CURVE_LINEAR;
    
    // Auto-morph state
    bool autoMorphActive = false;
    float currentPosition = 0.0f;
    float morphDuration = 1.0f;  // seconds
    MorphMode currentMode = LINEAR;
    int updateRateHz = 30;
    
    // Animation state
    juce::int64 startTime = 0;
    bool morphDirection = true;  // true = forward, false = backward (for PINGPONG)
    float randomWalkTarget = 0.0f;
    
    // Listeners
    juce::ListenerList<Listener> listeners;
    
    // Random generator
    juce::Random random;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorphEngine)
};
