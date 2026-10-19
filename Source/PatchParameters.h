/*
  ==============================================================================

    PatchParameters.h
    Data structures for MKS-70 Patch parameters
    Based on Vecoven V4.x firmware mapping (Global/System)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
// Patch Parameters Structure for MKS-70 with Vecoven V4.x
// Contains global patch settings that affect both tones
//==============================================================================
struct PatchParameters
{
    // Patch Info
    char toneName[11];          // Tone name (10 chars + null)
    char patchName[11];         // Patch name (10 chars + null)
    
    // Bank & Patch Selection
    int bankMSB;                // CC 0 (MSB), 0-31 (32 banks)
    int bankLSB;                // CC 32 (LSB)
    int patchNumber;            // Program Change 0-127
    
    // Global Balance & Split (SysEx Global, IPR OpCode 36)
    int upperLowerBalance;      // SysEx Address 12, 0-127
    int upperSplit;             // SysEx Address 19, 0-127 (MIDI note)
    int lowerSplit;             // SysEx Address 1A, 0-127 (MIDI note)
    int totalVolume;            // SysEx Address 1C, 0-127
    
    // Tone Selection & Key Mode
    int toneSel;                // 0-2 (Upper, Lower, Whole)
    int keyMode;                // 0-2 (Whole, Dual, Split)
    int splitPoint;             // 0-127 (MIDI note number for split)
    int balance;                // 0-127 (Upper/Lower balance)
    
    // Portamento
    int portamentoSwitch;       // 0-1 (Off, On)
    int portamentoTime;         // 0-127
    
    // Bender
    int bendRange;              // 0-12 (semitones)
    
    // Assignment
    int assignMode;             // 0-5 (Poly1, Poly2, Unison1, Unison2, Solo1, Solo2)
    int glide;                  // 0-1 (Off, On)
    int unisonDetune;           // 0-127
    
    // Chase Play
    int chasePlay;              // 0-1 (Off, On)
    int chaseLevel;             // 0-127
    int chaseTime;              // 0-127
    
    // Constructor with default values
    PatchParameters()
        : bankMSB(0), bankLSB(0), patchNumber(0),
          upperLowerBalance(64), upperSplit(60), lowerSplit(60), totalVolume(100),
          toneSel(0), keyMode(0), splitPoint(60), balance(64),
          portamentoSwitch(0), portamentoTime(0),
          bendRange(2),
          assignMode(0), glide(0), unisonDetune(0),
          chasePlay(0), chaseLevel(0), chaseTime(0)
    {
        // Initialize names
        strcpy(toneName, "Init Tone");
        strcpy(patchName, "Init Patch");
    }
    
    // Getters and setters for names
    juce::String getPatchName() const { return juce::String(patchName); }
    juce::String getToneName() const { return juce::String(toneName); }
    void setPatchName(const juce::String& name) { strncpy(patchName, name.toRawUTF8(), sizeof(patchName) - 1); patchName[sizeof(patchName) - 1] = '\0'; }
    void setToneName(const juce::String& name) { strncpy(toneName, name.toRawUTF8(), sizeof(toneName) - 1); toneName[sizeof(toneName) - 1] = '\0'; }
    
    // Reset to default values
    void reset()
    {
        *this = PatchParameters();
    }
};
