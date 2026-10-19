/*
  ==============================================================================

    ToneClipboard.h
    Singleton clipboard for copying/pasting tone parameters between tabs

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToneParams.h"

//==============================================================================
/**
    ToneClipboard - Singleton for copying/pasting tone parameters
*/
class ToneClipboard
{
public:
    // Singleton access
    static ToneClipboard& getInstance()
    {
        static ToneClipboard instance;
        return instance;
    }
    
    // Copy full tone
    void copyTone(const ToneParams& params, bool isUpper)
    {
        clipboardParams = params;
        sourceTone = isUpper ? "Upper" : "Lower";
        hasData = true;
        copiedSection = Section::All;
    }
    
    // Copy specific section
    enum class Section
    {
        DCO1,
        DCO2,
        Mixer,
        VCF,
        VCA,
        LFO,    // Combined LFO-1 and LFO-2
        LFO1,
        LFO2,
        ENV1,
        ENV2,
        ENV3,
        ENV4,
        All
    };
    
    void copySection(const ToneParams& params, Section section, bool isUpper)
    {
        clipboardParams = params;
        copiedSection = section;
        sourceTone = isUpper ? "Upper" : "Lower";
        hasData = true;
    }
    
    // Paste to target (returns the parameters to paste, caller applies them)
    bool paste(ToneParams& targetParams) const
    {
        if (!hasData) return false;
        
        switch (copiedSection)
        {
            case Section::DCO1:
                targetParams.dco1Range = clipboardParams.dco1Range;
                targetParams.dco1Waveform = clipboardParams.dco1Waveform;
                targetParams.dco1Tune = clipboardParams.dco1Tune;
                targetParams.dco1LFOMod = clipboardParams.dco1LFOMod;
                targetParams.dco1ENVMod = clipboardParams.dco1ENVMod;
                break;
                
            case Section::DCO2:
                targetParams.dco2Range = clipboardParams.dco2Range;
                targetParams.dco2Waveform = clipboardParams.dco2Waveform;
                targetParams.dco2CrossMod = clipboardParams.dco2CrossMod;
                targetParams.dco2Tune = clipboardParams.dco2Tune;
                targetParams.dco2FineTune = clipboardParams.dco2FineTune;
                targetParams.dco2LFOMod = clipboardParams.dco2LFOMod;
                targetParams.dco2ENVMod = clipboardParams.dco2ENVMod;
                break;
                
            case Section::Mixer:
                targetParams.mixerDCO1Level = clipboardParams.mixerDCO1Level;
                targetParams.mixerDCO2Level = clipboardParams.mixerDCO2Level;
                targetParams.mixerENVMod = clipboardParams.mixerENVMod;
                targetParams.mixerDynamics = clipboardParams.mixerDynamics;
                targetParams.mixerENVMode = clipboardParams.mixerENVMode;
                break;
                
            case Section::VCF:
                targetParams.vcfHPF = clipboardParams.vcfHPF;
                targetParams.vcfCutoff = clipboardParams.vcfCutoff;
                targetParams.vcfResonance = clipboardParams.vcfResonance;
                targetParams.vcfLFOMod = clipboardParams.vcfLFOMod;
                targetParams.vcfENVMod = clipboardParams.vcfENVMod;
                targetParams.vcfKeyFollow = clipboardParams.vcfKeyFollow;
                targetParams.vcfDynamics = clipboardParams.vcfDynamics;
                targetParams.vcfENVMode = clipboardParams.vcfENVMode;
                break;
                
            case Section::VCA:
                targetParams.vcaMode = clipboardParams.vcaMode;
                targetParams.vcaLevel = clipboardParams.vcaLevel;
                targetParams.vcaDynamics = clipboardParams.vcaDynamics;
                break;
                
            case Section::LFO:
                // Combined LFO copies both LFO-1 and LFO-2
                targetParams.lfo1Waveform = clipboardParams.lfo1Waveform;
                targetParams.lfo1Delay = clipboardParams.lfo1Delay;
                targetParams.lfo1Rate = clipboardParams.lfo1Rate;
                targetParams.lfo2Waveform = clipboardParams.lfo2Waveform;
                targetParams.lfo2Delay = clipboardParams.lfo2Delay;
                targetParams.lfo2Rate = clipboardParams.lfo2Rate;
                break;
                
            case Section::LFO1:
                targetParams.lfo1Waveform = clipboardParams.lfo1Waveform;
                targetParams.lfo1Delay = clipboardParams.lfo1Delay;
                targetParams.lfo1Rate = clipboardParams.lfo1Rate;
                break;
                
            case Section::LFO2:
                targetParams.lfo2Waveform = clipboardParams.lfo2Waveform;
                targetParams.lfo2Delay = clipboardParams.lfo2Delay;
                targetParams.lfo2Rate = clipboardParams.lfo2Rate;
                break;
                
            case Section::ENV1:
                targetParams.env1Time1 = clipboardParams.env1Time1;
                targetParams.env1Level1 = clipboardParams.env1Level1;
                targetParams.env1Time2 = clipboardParams.env1Time2;
                targetParams.env1Level2 = clipboardParams.env1Level2;
                targetParams.env1Time3 = clipboardParams.env1Time3;
                targetParams.env1Level3 = clipboardParams.env1Level3;
                targetParams.env1Time4 = clipboardParams.env1Time4;
                targetParams.env1KeyFollow = clipboardParams.env1KeyFollow;
                break;
                
            case Section::ENV2:
                targetParams.env2Time1 = clipboardParams.env2Time1;
                targetParams.env2Level1 = clipboardParams.env2Level1;
                targetParams.env2Time2 = clipboardParams.env2Time2;
                targetParams.env2Level2 = clipboardParams.env2Level2;
                targetParams.env2Time3 = clipboardParams.env2Time3;
                targetParams.env2Level3 = clipboardParams.env2Level3;
                targetParams.env2Time4 = clipboardParams.env2Time4;
                targetParams.env2KeyFollow = clipboardParams.env2KeyFollow;
                break;
                
            case Section::ENV3:
                targetParams.env3Attack = clipboardParams.env3Attack;
                targetParams.env3Decay = clipboardParams.env3Decay;
                targetParams.env3Sustain = clipboardParams.env3Sustain;
                targetParams.env3Release = clipboardParams.env3Release;
                break;
                
            case Section::ENV4:
                targetParams.env4Attack = clipboardParams.env4Attack;
                targetParams.env4Decay = clipboardParams.env4Decay;
                targetParams.env4Sustain = clipboardParams.env4Sustain;
                targetParams.env4Release = clipboardParams.env4Release;
                break;
                
            case Section::All:
            default:
                targetParams = clipboardParams;
                break;
        }
        
        return true;
    }
    
    bool hasClipboardData() const { return hasData; }
    juce::String getSourceInfo() const { return sourceTone + " Tone"; }
    Section getCopiedSection() const { return copiedSection; }
    
    juce::String getSectionName(Section section) const
    {
        switch (section)
        {
            case Section::DCO1:  return "DCO-1";
            case Section::DCO2:  return "DCO-2";
            case Section::Mixer: return "Mixer";
            case Section::VCF:   return "VCF";
            case Section::VCA:   return "VCA";
            case Section::LFO:   return "LFO";
            case Section::LFO1:  return "LFO-1";
            case Section::LFO2:  return "LFO-2";
            case Section::ENV1:  return "ENV-1";
            case Section::ENV2:  return "ENV-2";
            case Section::ENV3:  return "ENV-3";
            case Section::ENV4:  return "ENV-4";
            case Section::All:   return "All";
            default:             return "";
        }
    }
    
private:
    ToneClipboard() = default;
    ~ToneClipboard() = default;
    
    ToneClipboard(const ToneClipboard&) = delete;
    ToneClipboard& operator=(const ToneClipboard&) = delete;
    
    ToneParams clipboardParams;
    Section copiedSection = Section::All;
    juce::String sourceTone;
    bool hasData = false;
};
