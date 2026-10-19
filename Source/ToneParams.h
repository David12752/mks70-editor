/*
  ==============================================================================

    ToneParams.h
    Data structures for MKS-70 Tone parameters (Upper and Lower)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
// Tone Parameters Structure for MKS-70
// Supports both Upper (Tone A) and Lower (Tone B) tones
//==============================================================================
struct ToneParams
{
    // DCO-1 Section
    int dco1Range;          // 0-3 (16', 8', 4', 2')
    int dco1Waveform;       // 0-3 (Pulse, Saw, Square, Triangle)
    int dco1Tune;           // 0-127
    int dco1LFOMod;         // 0-127
    int dco1ENVMod;         // 0-127
    
    // DCO-2 Section
    int dco2Range;          // 0-3 (16', 8', 4', 2')
    int dco2Waveform;       // 0-3 (Pulse, Saw, Square, Triangle)
    int dco2CrossMod;       // 0-3
    int dco2Tune;           // 0-127
    int dco2FineTune;       // 0-127
    int dco2LFOMod;         // 0-127
    int dco2ENVMod;         // 0-127
    
    // DCO Common
    int dcoDynamics;        // 0-3
    int dcoENVMode;         // 0-3
    
    // Mixer Section
    int mixerDCO1Level;     // 0-127
    int mixerDCO2Level;     // 0-127
    int mixerENVMod;        // 0-127
    int mixerDynamics;      // 0-3
    int mixerENVMode;       // 0-3
    
    // VCF Section
    int vcfHPF;             // 0-3
    int vcfCutoff;          // 0-127
    int vcfResonance;       // 0-127
    int vcfLFOMod;          // 0-127
    int vcfENVMod;          // 0-127
    int vcfKeyFollow;       // 0-127
    int vcfDynamics;        // 0-3
    int vcfENVMode;         // 0-3
    
    // VCA Section
    int vcaMode;            // 0-1 (Gate, Env)
    int vcaLevel;           // 0-127
    int vcaDynamics;        // 0-3
    
    // ENV1 Section (7-Stage Envelope)
    int env1Time1;          // 0-127 (Attack)
    int env1Level1;         // 0-127
    int env1Time2;          // 0-127 (Decay)
    int env1Level2;         // 0-127
    int env1Time3;          // 0-127
    int env1Level3;         // 0-127 (Sustain)
    int env1Time4;          // 0-127 (Release)
    int env1KeyFollow;      // 0-3
    
    // ENV2 Section (7-Stage Envelope)
    int env2Time1;          // 0-127 (Attack)
    int env2Level1;         // 0-127
    int env2Time2;          // 0-127 (Decay)
    int env2Level2;         // 0-127
    int env2Time3;          // 0-127
    int env2Level3;         // 0-127 (Sustain)
    int env2Time4;          // 0-127 (Release)
    int env2KeyFollow;      // 0-3
    
    // ENV3 Section (ADSR for VCF)
    int env3Attack;         // 0-127
    int env3Decay;          // 0-127
    int env3Sustain;        // 0-127
    int env3Release;        // 0-127
    
    // ENV4 Section (ADSR for VCA)
    int env4Attack;         // 0-127
    int env4Decay;          // 0-127
    int env4Sustain;        // 0-127
    int env4Release;        // 0-127
    
    // LFO-1 Section
    int lfo1Waveform;       // 0-3 (Random, Square, Saw, Sine)
    int lfo1Delay;          // 0-127
    int lfo1Rate;           // 0-127
    
    // LFO-2 Section
    int lfo2Waveform;       // 0-3 (Random, Square, Saw, Sine)
    int lfo2Delay;          // 0-127
    int lfo2Rate;           // 0-127
    
    // Chorus
    int chorusMode;         // 0-2 (Off, I, II)
    
    // Constructor with default values
    ToneParams()
        : dco1Range(1), dco1Waveform(1), dco1Tune(64), dco1LFOMod(0), dco1ENVMod(0),
          dco2Range(1), dco2Waveform(1), dco2CrossMod(0), dco2Tune(64), dco2FineTune(64),
          dco2LFOMod(0), dco2ENVMod(0),
          dcoDynamics(0), dcoENVMode(0),
          mixerDCO1Level(100), mixerDCO2Level(100), mixerENVMod(0), mixerDynamics(0), mixerENVMode(0),
          vcfHPF(0), vcfCutoff(127), vcfResonance(0), vcfLFOMod(0), vcfENVMod(64), 
          vcfKeyFollow(64), vcfDynamics(0), vcfENVMode(0),
          vcaMode(1), vcaLevel(127), vcaDynamics(0),
          env1Time1(0), env1Level1(100), env1Time2(64), env1Level2(80), env1Time3(64), env1Level3(100), env1Time4(64), env1KeyFollow(0),
          env2Time1(0), env2Level1(100), env2Time2(64), env2Level2(80), env2Time3(64), env2Level3(100), env2Time4(64), env2KeyFollow(0),
          env3Attack(0), env3Decay(64), env3Sustain(100), env3Release(64),
          env4Attack(0), env4Decay(64), env4Sustain(100), env4Release(64),
          lfo1Waveform(3), lfo1Delay(0), lfo1Rate(64),
          lfo2Waveform(3), lfo2Delay(0), lfo2Rate(64),
          chorusMode(0)
    {
    }
    
    // Reset to default values
    void reset()
    {
        *this = ToneParams();
    }
};
