#pragma once

#include <JuceHeader.h>

class AutomationRecorder
{
public:
    // Data
    struct AutomationPoint {
        double timestamp = 0.0; // seconds
        juce::String paramId;
        int value = 0;          // 0..127
    };

    struct AutomationLane {
        juce::String paramId;
        std::vector<AutomationPoint> points;
        bool enabled = true;
        bool armed = false;
    };

    enum SyncMode { INTERNAL, MIDI_CLOCK, HOST_TEMPO };

    AutomationRecorder();

    // Recording
    void startRecording();
    void stopRecording();
    bool isRecording() const { return recording; }
    void recordParameterChange(const juce::String& paramId, int value, double timestamp);

    // Playback
    void startPlayback();
    void stopPlayback();
    void updatePlaybackPosition();
    void setPlaybackPosition(double timeSeconds);
    void setLoop(double startTime, double endTime);
    void setTempo(double bpm);
    std::vector<std::pair<juce::String, int>> getValuesAtTime(double timeSeconds) const;

    // Accessors
    std::vector<AutomationLane>& getLanes() { return lanes; }
    const std::vector<AutomationLane>& getLanes() const { return lanes; }
    double getPlaybackPosition() const { return playbackPosSeconds; }
    bool isPlaying() const { return playing; }
    bool isLoopEnabled() const { return loopEnabled && loopEndSeconds > loopStartSeconds; }
    std::pair<double,double> getLoopRange() const { return { loopStartSeconds, loopEndSeconds }; }
    double getTempo() const { return tempoBPM; }
    void setSyncMode(SyncMode mode);
    SyncMode getSyncMode() const { return syncMode; }

    // Export
    bool exportToMidiFile(const juce::File& file);
    bool exportToJSON(const juce::File& file);
    bool importFromJSON(const juce::File& file);

private:
    AutomationLane* findOrCreateLane(const juce::String& paramId);
    static int clampMidiValue(int v) { return juce::jlimit(0, 127, v); }
    static int paramIdToController(const juce::String& paramId);

    std::vector<AutomationLane> lanes;
    double recordingStartTimeSeconds = 0.0;
    bool recording = false;

    bool playing = false;
    double playbackStartWallClockSeconds = 0.0; // wall clock at start
    double playbackPosSeconds = 0.0;           // current position

    bool loopEnabled = false;
    double loopStartSeconds = 0.0;
    double loopEndSeconds = 0.0;

    double tempoBPM = 120.0;
    SyncMode syncMode = INTERNAL;

    juce::CriticalSection lock;
};
