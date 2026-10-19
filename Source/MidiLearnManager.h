#pragma once

#include <JuceHeader.h>
#include <map>
#include <vector>

/**
 * MIDI Learn manager: learns mappings from incoming MIDI to parameter IDs
 * and dispatches mapped values. Supports CC, NRPN, Note, Aftertouch, Pitch Bend.
 */
class MidiLearnManager
{
public:
    struct MidiMapping
    {
        juce::String paramId;
        int channel = -1; // 0-15, -1 = omni
        enum Type { CC, NRPN, NOTE, AFTERTOUCH, PITCHBEND } type = CC;
        int controller = 0; // CC number, NRPN number, or note number
        int minIn = 0;      // Input range
        int maxIn = 127;
        int minOut = 0;     // Output range (parameter value)
        int maxOut = 127;
        bool inverted = false;
        bool relative = false;  // For endless encoders
        float smoothing = 0.0f; // 0-1, smoothing amount
    };

    // Learn mode
    void startLearn(const juce::String& targetParamId);
    void stopLearn();
    bool isLearning() const { return learning; }
    juce::String getLearningTarget() const { return currentLearningParam; }

    // Process incoming MIDI
    void processMidiMessage(const juce::MidiMessage& msg);

    // Mappings
    void addMapping(const MidiMapping& mapping);
    void removeMapping(const juce::String& paramId);
    MidiMapping* getMappingForParam(const juce::String& paramId);
    std::vector<MidiMapping>& getAllMappings();

    // Presets
    void saveMappingPreset(const juce::File& file);
    void loadMappingPreset(const juce::File& file);
    void clearAllMappings();

    // Callbacks
    std::function<void(const juce::String& paramId, int value)> onMappedParameterChange;
    std::function<void(const MidiMapping&)> onLearnComplete;

    // Optional: per-param sink registration for direct UI updates
    void registerParamSink(const juce::String& paramId, std::function<void(int)> sink);
    void unregisterParamSink(const juce::String& paramId);

private:
    // Internal helpers
    static int clampInt(int v, int lo, int hi) { return juce::jlimit(lo, hi, v); }
    static int scaleInt(int inVal, int inMin, int inMax, int outMin, int outMax, bool inverted);
    void dispatchValue(const juce::String& paramId, int value);

    // NRPN detection state per MIDI channel
    struct NRPNState
    {
        int nrpnMsb = -1; // CC 99
        int nrpnLsb = -1; // CC 98
        int dataMsb = -1; // CC 6
        int dataLsb = -1; // CC 38
        bool hasNumber() const { return nrpnMsb >= 0 && nrpnLsb >= 0; }
        int number() const { return (nrpnMsb << 7) | (nrpnLsb & 0x7F); }
        int value14() const { return ((dataMsb < 0 ? 0 : dataMsb) << 7) | (dataLsb < 0 ? 0 : dataLsb); }
        void reset() { nrpnMsb = nrpnLsb = dataMsb = dataLsb = -1; }
    };

    std::map<juce::String, MidiMapping> mappings; // keyed by paramId
    juce::String currentLearningParam;
    bool learning = false;

    // Cached vector for getAllMappings()
    std::vector<MidiMapping> allMappingsCache;

    // Runtime smoothing/relative state
    std::map<juce::String, double> lastOutputForParam; // for smoothing
    std::map<juce::String, int> lastInputForParam;      // for relative encoders
    NRPNState nrpnStates[16];

    // Optional: sinks for direct UI updates
    std::map<juce::String, std::function<void(int)>> paramSinks;
};
