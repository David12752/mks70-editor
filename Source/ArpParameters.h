/*
  ==============================================================================

    ArpParameters.h
    Data structures for MKS-70 Arpeggiator parameters (Vecoven firmware)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
// Arpeggiator Parameters Structure for MKS-70 with Vecoven firmware
// These parameters are specific to the Vecoven 3.x upgrade
//==============================================================================
struct ArpParameters
{
    // Arpeggiator Control
    int arpOn;                  // 0-1 (Off/On)
    int tempo;                  // 30-300 (Beats Per Minute) - Vecoven firmware range
    int mode;                   // 0-4 (Up, Down, Up-Down, Played, Random)
    int octaveSpan;             // 1-4 (Number of octaves)
    int insert;                 // 0-3 (None, Highest, Lowest, Chord)
    int playRate;               // 0-7 (1/2, 1/4, 1/8, 1/16, 1/32, Triplets)
    int duration;               // 1-15 (Duration steps)
    int legato;                 // 0-1 (Off/On)
    int velocityMode;           // 0-1 (Fixed, Played)
    int fixedVelocity;          // 1-127 (Velocity value when mode is Fixed)
    int midiOut;                // 0-2 (Off, Internal, All+Out)
    int sync;                   // 0-1 (Internal, External MIDI Clock)
    int latch;                  // 0-1 (Off/On)
    
    // Constructor with default values
    ArpParameters()
        : arpOn(0), tempo(120),
          mode(0), octaveSpan(1), insert(0), playRate(2),  // Default 1/8
          duration(8), legato(0), velocityMode(1), fixedVelocity(100), 
          midiOut(1), sync(0), latch(0)
    {
    }
    
    // Reset to default values
    void reset()
    {
        *this = ArpParameters();
    }
    
    // Validation helpers
    bool isValidTempo(int value) const
    {
        return value >= 20 && value <= 300;
    }
    
    bool isValidMode(int value) const
    {
        return value >= 0 && value <= 4;
    }
    
    bool isValidOctaveSpan(int value) const
    {
        return value >= 1 && value <= 4;
    }
    
    bool isValidInsert(int value) const
    {
        return value >= 0 && value <= 3;
    }
    
    bool isValidPlayRate(int value) const
    {
        return value >= 0 && value <= 7;
    }
    
    bool isValidDuration(int value) const
    {
        return value >= 1 && value <= 15;
    }
    
    bool isValidVelocity(int value) const
    {
        return value >= 1 && value <= 127;
    }
};
