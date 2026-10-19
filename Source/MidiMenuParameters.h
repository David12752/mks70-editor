/*
  ==============================================================================

    MidiMenuParameters.h
    Vecoven 3.x MIDI Menu Parameter Definitions
    
    Contains all parameter metadata for the MKS-70 MIDI menu settings,
    arpeggiator parameters, and NRPN tone mappings.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <map>
#include <vector>

namespace MKS70
{

//==============================================================================
// Enum types for quad-state parameters
//==============================================================================

enum class MidiToggle : int
{
    OFF = 0,
    ON = 1
};

enum class MidiQuadState : int
{
    OFF = 0,
    RECV = 1,
    SEND = 2,
    ON = 3
};

enum class NoteFilter : int
{
    ALL = 0,
    ODD = 1,
    EVEN = 2
};

enum class EditDisplaySource : int
{
    OFF = 0,
    SYSEX = 1,
    PG800 = 2,
    CC = 3,
    ALL = 4
};

//==============================================================================
// Arpeggiator enums
//==============================================================================

enum class ArpMode : int
{
    UP = 0,
    DOWN = 1,
    UP_DOWN = 2,
    PLAYED = 3,
    RANDOM = 4
};

enum class ArpInsert : int
{
    NONE = 0,
    HIGH = 1,
    LOW = 2,
    THREE_ONE = 3,
    FOUR_TWO = 4
};

enum class ArpRate : int
{
    HALF = 0,       // 1/2
    QUARTER = 1,    // 1/4
    EIGHTH = 2,     // 1/8
    EIGHTH_T = 3,   // 1/8T
    SIXTEENTH = 4,  // 1/16
    SIXTEENTH_T = 5,// 1/16T
    THIRTY_TWO = 6  // 1/32
};

enum class ArpVelocity : int
{
    FIXED = 0,
    PLAYED = 1
};

enum class ArpMidiSync : int
{
    OFF = 0,
    IN = 1,
    OUT = 2,
    ALL = 3
};

//==============================================================================
// Parameter metadata structure
//==============================================================================

struct MidiMenuParam
{
    int paramNumber;
    juce::String displayName;
    juce::String description;
    int minValue;
    int maxValue;
    int defaultValue;
    juce::StringArray valueNames;  // Empty if numeric range
    bool isJX10Only;
    juce::String note;
    
    bool hasEnumValues() const { return !valueNames.isEmpty(); }
    
    juce::String getValueName(int value) const
    {
        if (hasEnumValues() && value >= 0 && value < valueNames.size())
            return valueNames[value];
        return juce::String(value);
    }
};

//==============================================================================
// MIDI Menu Parameters Table (Vecoven 3.x)
//==============================================================================

namespace MidiMenu
{
    // MIDI Channels
    static const MidiMenuParam CTRL_CHANNEL = {
        11, "Control Channel", "Control Channel - Main control channel for patch operations",
        1, 16, 1, {}, false, ""
    };
    
    static const MidiMenuParam UPPER_CHANNEL = {
        12, "Upper Channel", "Upper/Tone A MIDI Channel",
        1, 16, 1, {}, false, ""
    };
    
    static const MidiMenuParam LOWER_CHANNEL = {
        13, "Lower Channel", "Lower/Tone B MIDI Channel",
        1, 16, 2, {}, false, ""
    };
    
    // SysEx Settings
    static const MidiMenuParam SYSEX = {
        14, "SysEx", "Main SysEx Enable",
        0, 3, 3, {"OFF", "RECV", "SEND", "ON"}, false, ""
    };
    
    static const MidiMenuParam SYSEX_IPR = {
        15, "SysEx IPR", "Individual Parameter Reception",
        0, 3, 3, {"OFF", "RECV", "SEND", "ON"}, false, ""
    };
    
    static const MidiMenuParam SYSEX_APR = {
        16, "SysEx APR", "All Parameters Reception",
        0, 3, 3, {"OFF", "RECV", "SEND", "ON"}, false, ""
    };
    
    static const MidiMenuParam MIDI_CC = {
        23, "MIDI CC", "MIDI CC/NRPN Enable",
        0, 3, 3, {"OFF", "RECV", "SEND", "ON"}, false, ""
    };
    
    // Program Change
    static const MidiMenuParam PATCH_PROG_CHG = {
        17, "Patch Prog Change", "Patch Program Change Enable",
        0, 1, 1, {"OFF", "ON"}, false, ""
    };
    
    static const MidiMenuParam UPPER_PROG_CHG = {
        21, "Upper Prog Change", "Upper Tone Program Change",
        0, 1, 1, {"OFF", "ON"}, false, ""
    };
    
    static const MidiMenuParam LOWER_PROG_CHG = {
        22, "Lower Prog Change", "Lower Tone Program Change",
        0, 1, 1, {"OFF", "ON"}, false, ""
    };
    
    // Note Messages (MKS-70 specific)
    static const MidiMenuParam NOTE_MSG = {
        18, "Note Messages", "Note Message Filter (MKS-70)",
        0, 2, 0, {"ALL", "ODD", "EVEN"}, false,
        "ALL=all notes, ODD=odd channels only, EVEN=even channels only"
    };
    
    // Local Control (JX-10 Only)
    static const MidiMenuParam LOCAL_UPPER = {
        31, "Local Upper", "Local Control Upper",
        0, 1, 1, {"OFF", "ON"}, true, ""
    };
    
    static const MidiMenuParam LOCAL_LOWER = {
        32, "Local Lower", "Local Control Lower",
        0, 1, 1, {"OFF", "ON"}, true, ""
    };
    
    // PG-800 Support
    static const MidiMenuParam PG800 = {
        41, "PG-800", "PG-800 Programmer Support",
        0, 1, 1, {"OFF", "ON"}, false, ""
    };
    
    // Display Options
    static const MidiMenuParam EDIT_DISPLAY = {
        42, "Edit Display", "Show edits on display from",
        0, 4, 4, {"OFF", "SYSEX", "PG800", "CC", "ALL"}, false, ""
    };
    
    // Compatibility Mode
    static const MidiMenuParam COMPAT_MODE = {
        43, "Compatibility", "Compatibility with old software",
        0, 1, 0, {"OFF", "ON"}, false, ""
    };
    
    // VFD Display
    static const MidiMenuParam GU280_DISPLAY = {
        44, "GU280 Display", "Noritake GU280 VFD Support",
        0, 1, 0, {"OFF", "ON"}, false,
        "Reset by holding ENTER during power-up"
    };
    
    // Collect all parameters in an array for iteration
    inline std::vector<const MidiMenuParam*> getAllParams()
    {
        return {
            &CTRL_CHANNEL, &UPPER_CHANNEL, &LOWER_CHANNEL,
            &SYSEX, &SYSEX_IPR, &SYSEX_APR, &MIDI_CC,
            &PATCH_PROG_CHG, &UPPER_PROG_CHG, &LOWER_PROG_CHG,
            &NOTE_MSG, &LOCAL_UPPER, &LOCAL_LOWER,
            &PG800, &EDIT_DISPLAY, &COMPAT_MODE, &GU280_DISPLAY
        };
    }
}

//==============================================================================
// Arpeggiator Patch Structure
//==============================================================================

struct ArpPatch
{
    int bpm = 120;           // 40-300
    ArpMode mode = ArpMode::UP;
    int octave = 1;          // 1-4
    ArpInsert insert = ArpInsert::NONE;
    ArpRate rate = ArpRate::EIGHTH;
    int duration = 8;        // 1-15, 16=LEGATO
    ArpVelocity velocity = ArpVelocity::PLAYED;
    int fixedVelocity = 100; // 1-127, used when velocity=FIXED
    ArpMidiSync midi = ArpMidiSync::ALL;
    bool latch = false;
    
    static juce::String getRateName(ArpRate r)
    {
        static const char* names[] = {"1/2", "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32"};
        return names[static_cast<int>(r)];
    }
    
    static juce::String getModeName(ArpMode m)
    {
        static const char* names[] = {"UP", "DOWN", "UP-DOWN", "PLAYED", "RANDOM"};
        return names[static_cast<int>(m)];
    }
    
    static juce::String getInsertName(ArpInsert i)
    {
        static const char* names[] = {"NONE", "HIGH", "LOW", "3-1", "4-2"};
        return names[static_cast<int>(i)];
    }
    
    static juce::String getMidiSyncName(ArpMidiSync s)
    {
        static const char* names[] = {"OFF", "IN", "OUT", "ALL"};
        return names[static_cast<int>(s)];
    }
    
    juce::String getDurationName() const
    {
        return duration == 16 ? "LEGATO" : juce::String(duration);
    }
};

//==============================================================================
// Tone NRPN Parameter Definition
//==============================================================================

struct ToneNRPNParam
{
    int cc;
    juce::String name;
    int minValue;
    int maxValue;
    bool bipolar;       // If true, center is 64
    juce::StringArray valueNames;
    
    bool hasEnumValues() const { return !valueNames.isEmpty(); }
};

namespace ToneNRPN
{
    // DCO-1
    static const ToneNRPNParam DCO1_RANGE     = { 0,  "DCO1 Range",     0, 3, false, {"16'", "8'", "4'", "2'"} };
    static const ToneNRPNParam DCO1_WAVE      = { 1,  "DCO1 Wave",      0, 3, false, {"SAW", "PULSE", "SQUARE", "NOISE"} };
    static const ToneNRPNParam DCO1_TUNE      = { 2,  "DCO1 Tune",      0, 127, true, {} };
    static const ToneNRPNParam DCO1_LFO       = { 3,  "DCO1 LFO",       0, 127, false, {} };
    static const ToneNRPNParam DCO1_ENV       = { 4,  "DCO1 Env",       0, 127, false, {} };
    
    // DCO-2
    static const ToneNRPNParam DCO2_RANGE     = { 5,  "DCO2 Range",     0, 3, false, {"16'", "8'", "4'", "2'"} };
    static const ToneNRPNParam DCO2_WAVE      = { 6,  "DCO2 Wave",      0, 3, false, {"SAW", "PULSE", "SQUARE", "NOISE"} };
    static const ToneNRPNParam DCO2_XMOD      = { 7,  "DCO2 XMod",      0, 3, false, {"OFF", "SYNC1", "SYNC2", "XMOD"} };
    static const ToneNRPNParam DCO2_TUNE      = { 8,  "DCO2 Tune",      0, 127, true, {} };
    static const ToneNRPNParam DCO2_FINE      = { 9,  "DCO2 Fine",      0, 127, true, {} };
    static const ToneNRPNParam DCO2_LFO       = { 10, "DCO2 LFO",       0, 127, false, {} };
    static const ToneNRPNParam DCO2_ENV       = { 11, "DCO2 Env",       0, 127, false, {} };
    
    // DCO Common
    static const ToneNRPNParam DCO_DYNAMICS   = { 15, "DCO Dynamics",   0, 3, false, {"OFF", "1", "2", "3"} };
    static const ToneNRPNParam DCO_ENV_MODE   = { 16, "DCO Env Mode",   0, 3, false, {"ENV1+", "ENV1-", "ENV2+", "ENV2-"} };
    
    // Mixer
    static const ToneNRPNParam MIX_DCO1       = { 17, "Mix DCO1",       0, 127, false, {} };
    static const ToneNRPNParam MIX_DCO2       = { 18, "Mix DCO2",       0, 127, false, {} };
    static const ToneNRPNParam MIX_ENV        = { 19, "Mix Env",        0, 127, false, {} };
    static const ToneNRPNParam MIX_DYNAMICS   = { 20, "Mix Dynamics",   0, 3, false, {"OFF", "1", "2", "3"} };
    static const ToneNRPNParam MIX_ENV_MODE   = { 21, "Mix Env Mode",   0, 3, false, {"ENV1+", "ENV1-", "ENV2+", "ENV2-"} };
    
    // VCF
    static const ToneNRPNParam HPF_CUTOFF     = { 22, "HPF Cutoff",     0, 3, false, {"0", "1", "2", "3"} };
    static const ToneNRPNParam VCF_CUTOFF     = { 23, "VCF Cutoff",     0, 127, false, {} };
    static const ToneNRPNParam VCF_RESONANCE  = { 24, "VCF Resonance",  0, 127, false, {} };
    static const ToneNRPNParam VCF_LFO        = { 25, "VCF LFO",        0, 127, false, {} };
    static const ToneNRPNParam VCF_ENV        = { 26, "VCF Env",        0, 127, false, {} };
    static const ToneNRPNParam VCF_KEY        = { 27, "VCF Key Follow", 0, 127, false, {} };
    static const ToneNRPNParam VCF_DYNAMICS   = { 28, "VCF Dynamics",   0, 3, false, {"OFF", "1", "2", "3"} };
    static const ToneNRPNParam VCF_ENV_MODE   = { 29, "VCF Env Mode",   0, 3, false, {"ENV1+", "ENV1-", "ENV2+", "ENV2-"} };
    
    // VCA
    static const ToneNRPNParam VCA_LEVEL      = { 30, "VCA Level",      0, 127, false, {} };
    static const ToneNRPNParam VCA_DYNAMICS   = { 31, "VCA Dynamics",   0, 3, false, {"OFF", "1", "2", "3"} };
    static const ToneNRPNParam VCA_ENV_MODE   = { 47, "VCA Env Mode",   0, 1, false, {"ENV2", "GATE"} };
    
    // Chorus
    static const ToneNRPNParam CHORUS         = { 32, "Chorus",         0, 2, false, {"OFF", "1", "2"} };
    
    // LFO
    static const ToneNRPNParam LFO_WAVE       = { 33, "LFO Wave",       0, 2, false, {"TRIANGLE", "SQUARE", "RANDOM"} };
    static const ToneNRPNParam LFO_DELAY      = { 34, "LFO Delay",      0, 127, false, {} };
    static const ToneNRPNParam LFO_RATE       = { 35, "LFO Rate",       0, 127, false, {} };
    
    // ENV-1 (VCF)
    static const ToneNRPNParam ENV1_ATTACK    = { 36, "ENV1 Attack",    0, 127, false, {} };
    static const ToneNRPNParam ENV1_DECAY     = { 37, "ENV1 Decay",     0, 127, false, {} };
    static const ToneNRPNParam ENV1_SUSTAIN   = { 38, "ENV1 Sustain",   0, 127, false, {} };
    static const ToneNRPNParam ENV1_RELEASE   = { 39, "ENV1 Release",   0, 127, false, {} };
    static const ToneNRPNParam ENV1_KEY       = { 40, "ENV1 Key",       0, 127, false, {} };
    
    // ENV-2 (VCA)
    static const ToneNRPNParam ENV2_ATTACK    = { 41, "ENV2 Attack",    0, 127, false, {} };
    static const ToneNRPNParam ENV2_DECAY     = { 42, "ENV2 Decay",     0, 127, false, {} };
    static const ToneNRPNParam ENV2_SUSTAIN   = { 43, "ENV2 Sustain",   0, 127, false, {} };
    static const ToneNRPNParam ENV2_RELEASE   = { 44, "ENV2 Release",   0, 127, false, {} };
    static const ToneNRPNParam ENV2_KEY       = { 45, "ENV2 Key",       0, 127, false, {} };
}

//==============================================================================
// Value Conversion Utilities
//==============================================================================

namespace Conversions
{
    // 0-99 display from 0-127 MIDI
    inline int toDisplay99(int midi) { return juce::jlimit(0, 99, juce::roundToInt(midi * 99.0f / 127.0f)); }
    inline int fromDisplay99(int display) { return juce::jlimit(0, 127, juce::roundToInt(display * 127.0f / 99.0f)); }
    
    // -50 to +50 display from 0-127 MIDI
    inline int toBipolar50(int midi) { return juce::roundToInt((midi * 100.0f / 127.0f) - 50.0f); }
    inline int fromBipolar50(int display) { return juce::jlimit(0, 127, juce::roundToInt((display + 50.0f) * 127.0f / 100.0f)); }
    
    // -24 to +24 from special encoding
    inline int toChromatic(int midi) { return midi < 25 ? midi : midi - 128; }
    inline int fromChromatic(int display) { return display >= 0 ? display : display + 128; }
    
    // Note name from MIDI note number
    inline juce::String toNoteName(int midi)
    {
        static const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        int octave = (midi / 12) - 1;
        int note = midi % 12;
        return juce::String(notes[note]) + juce::String(octave);
    }
    
    inline int fromNoteName(const juce::String& name)
    {
        // Parse note name like "C4", "F#3", etc.
        static const std::map<juce::String, int> noteMap = {
            {"C", 0}, {"C#", 1}, {"Db", 1}, {"D", 2}, {"D#", 3}, {"Eb", 3},
            {"E", 4}, {"F", 5}, {"F#", 6}, {"Gb", 6}, {"G", 7}, {"G#", 8},
            {"Ab", 8}, {"A", 9}, {"A#", 10}, {"Bb", 10}, {"B", 11}
        };
        
        // Find where the octave number starts
        int octaveStart = 0;
        for (int i = 1; i < name.length(); ++i)
        {
            if (name[i] >= '0' && name[i] <= '9' || name[i] == '-')
            {
                octaveStart = i;
                break;
            }
        }
        
        juce::String notePart = name.substring(0, octaveStart);
        int octave = name.substring(octaveStart).getIntValue();
        
        auto it = noteMap.find(notePart);
        if (it != noteMap.end())
            return (octave + 1) * 12 + it->second;
        
        return 60; // Default to middle C
    }
}

} // namespace MKS70
