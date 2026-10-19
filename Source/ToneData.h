/*
  ==============================================================================

    ToneData.h
    Wrapper for ToneParams with serialization support for PresetManager

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToneParams.h"

//==============================================================================
// ToneData - extends ToneParams with serialization and metadata
//==============================================================================
struct ToneData : public ToneParams
{
    juce::String toneName = "Init";
    int toneNumber = 0;  // 0-49 for internal, 0-63 for cartridge
    
    ToneData() : ToneParams() {}
    
    // Copy from ToneParams
    ToneData(const ToneParams& params) : ToneParams(params) {}
    
    // Serialize to JSON
    juce::var toVar() const
    {
        auto obj = new juce::DynamicObject();
        
        obj->setProperty("toneName", toneName);
        obj->setProperty("toneNumber", toneNumber);
        
        // DCO1
        obj->setProperty("dco1Range", dco1Range);
        obj->setProperty("dco1Waveform", dco1Waveform);
        obj->setProperty("dco1Tune", dco1Tune);
        obj->setProperty("dco1LFOMod", dco1LFOMod);
        obj->setProperty("dco1ENVMod", dco1ENVMod);
        
        // DCO2
        obj->setProperty("dco2Range", dco2Range);
        obj->setProperty("dco2Waveform", dco2Waveform);
        obj->setProperty("dco2CrossMod", dco2CrossMod);
        obj->setProperty("dco2Tune", dco2Tune);
        obj->setProperty("dco2FineTune", dco2FineTune);
        obj->setProperty("dco2LFOMod", dco2LFOMod);
        obj->setProperty("dco2ENVMod", dco2ENVMod);
        
        // DCO Common
        obj->setProperty("dcoDynamics", dcoDynamics);
        obj->setProperty("dcoENVMode", dcoENVMode);
        
        // Mixer
        obj->setProperty("mixerDCO1Level", mixerDCO1Level);
        obj->setProperty("mixerDCO2Level", mixerDCO2Level);
        obj->setProperty("mixerENVMod", mixerENVMod);
        obj->setProperty("mixerDynamics", mixerDynamics);
        obj->setProperty("mixerENVMode", mixerENVMode);
        
        // VCF
        obj->setProperty("vcfHPF", vcfHPF);
        obj->setProperty("vcfCutoff", vcfCutoff);
        obj->setProperty("vcfResonance", vcfResonance);
        obj->setProperty("vcfLFOMod", vcfLFOMod);
        obj->setProperty("vcfENVMod", vcfENVMod);
        obj->setProperty("vcfKeyFollow", vcfKeyFollow);
        obj->setProperty("vcfDynamics", vcfDynamics);
        obj->setProperty("vcfENVMode", vcfENVMode);
        
        // VCA
        obj->setProperty("vcaMode", vcaMode);
        obj->setProperty("vcaLevel", vcaLevel);
        obj->setProperty("vcaDynamics", vcaDynamics);
        
        // ENV1
        obj->setProperty("env1Time1", env1Time1);
        obj->setProperty("env1Level1", env1Level1);
        obj->setProperty("env1Time2", env1Time2);
        obj->setProperty("env1Level2", env1Level2);
        obj->setProperty("env1Time3", env1Time3);
        obj->setProperty("env1Level3", env1Level3);
        obj->setProperty("env1Time4", env1Time4);
        obj->setProperty("env1KeyFollow", env1KeyFollow);
        
        // ENV2
        obj->setProperty("env2Time1", env2Time1);
        obj->setProperty("env2Level1", env2Level1);
        obj->setProperty("env2Time2", env2Time2);
        obj->setProperty("env2Level2", env2Level2);
        obj->setProperty("env2Time3", env2Time3);
        obj->setProperty("env2Level3", env2Level3);
        obj->setProperty("env2Time4", env2Time4);
        obj->setProperty("env2KeyFollow", env2KeyFollow);
        
        // ENV3 (VCF ADSR)
        obj->setProperty("env3Attack", env3Attack);
        obj->setProperty("env3Decay", env3Decay);
        obj->setProperty("env3Sustain", env3Sustain);
        obj->setProperty("env3Release", env3Release);
        
        // ENV4 (VCA ADSR)
        obj->setProperty("env4Attack", env4Attack);
        obj->setProperty("env4Decay", env4Decay);
        obj->setProperty("env4Sustain", env4Sustain);
        obj->setProperty("env4Release", env4Release);
        
        // LFO1
        obj->setProperty("lfo1Waveform", lfo1Waveform);
        obj->setProperty("lfo1Delay", lfo1Delay);
        obj->setProperty("lfo1Rate", lfo1Rate);
        
        // LFO2
        obj->setProperty("lfo2Waveform", lfo2Waveform);
        obj->setProperty("lfo2Delay", lfo2Delay);
        obj->setProperty("lfo2Rate", lfo2Rate);
        
        // Chorus
        obj->setProperty("chorusMode", chorusMode);
        
        return juce::var(obj);
    }
    
    // Deserialize from JSON
    static ToneData fromVar(const juce::var& v)
    {
        ToneData data;
        
        if (auto* obj = v.getDynamicObject())
        {
            data.toneName = obj->getProperty("toneName").toString();
            data.toneNumber = obj->getProperty("toneNumber");
            
            // DCO1
            data.dco1Range = obj->getProperty("dco1Range");
            data.dco1Waveform = obj->getProperty("dco1Waveform");
            data.dco1Tune = obj->getProperty("dco1Tune");
            data.dco1LFOMod = obj->getProperty("dco1LFOMod");
            data.dco1ENVMod = obj->getProperty("dco1ENVMod");
            
            // DCO2
            data.dco2Range = obj->getProperty("dco2Range");
            data.dco2Waveform = obj->getProperty("dco2Waveform");
            data.dco2CrossMod = obj->getProperty("dco2CrossMod");
            data.dco2Tune = obj->getProperty("dco2Tune");
            data.dco2FineTune = obj->getProperty("dco2FineTune");
            data.dco2LFOMod = obj->getProperty("dco2LFOMod");
            data.dco2ENVMod = obj->getProperty("dco2ENVMod");
            
            // DCO Common
            data.dcoDynamics = obj->getProperty("dcoDynamics");
            data.dcoENVMode = obj->getProperty("dcoENVMode");
            
            // Mixer
            data.mixerDCO1Level = obj->getProperty("mixerDCO1Level");
            data.mixerDCO2Level = obj->getProperty("mixerDCO2Level");
            data.mixerENVMod = obj->getProperty("mixerENVMod");
            data.mixerDynamics = obj->getProperty("mixerDynamics");
            data.mixerENVMode = obj->getProperty("mixerENVMode");
            
            // VCF
            data.vcfHPF = obj->getProperty("vcfHPF");
            data.vcfCutoff = obj->getProperty("vcfCutoff");
            data.vcfResonance = obj->getProperty("vcfResonance");
            data.vcfLFOMod = obj->getProperty("vcfLFOMod");
            data.vcfENVMod = obj->getProperty("vcfENVMod");
            data.vcfKeyFollow = obj->getProperty("vcfKeyFollow");
            data.vcfDynamics = obj->getProperty("vcfDynamics");
            data.vcfENVMode = obj->getProperty("vcfENVMode");
            
            // VCA
            data.vcaMode = obj->getProperty("vcaMode");
            data.vcaLevel = obj->getProperty("vcaLevel");
            data.vcaDynamics = obj->getProperty("vcaDynamics");
            
            // ENV1
            data.env1Time1 = obj->getProperty("env1Time1");
            data.env1Level1 = obj->getProperty("env1Level1");
            data.env1Time2 = obj->getProperty("env1Time2");
            data.env1Level2 = obj->getProperty("env1Level2");
            data.env1Time3 = obj->getProperty("env1Time3");
            data.env1Level3 = obj->getProperty("env1Level3");
            data.env1Time4 = obj->getProperty("env1Time4");
            data.env1KeyFollow = obj->getProperty("env1KeyFollow");
            
            // ENV2
            data.env2Time1 = obj->getProperty("env2Time1");
            data.env2Level1 = obj->getProperty("env2Level1");
            data.env2Time2 = obj->getProperty("env2Time2");
            data.env2Level2 = obj->getProperty("env2Level2");
            data.env2Time3 = obj->getProperty("env2Time3");
            data.env2Level3 = obj->getProperty("env2Level3");
            data.env2Time4 = obj->getProperty("env2Time4");
            data.env2KeyFollow = obj->getProperty("env2KeyFollow");
            
            // ENV3
            data.env3Attack = obj->getProperty("env3Attack");
            data.env3Decay = obj->getProperty("env3Decay");
            data.env3Sustain = obj->getProperty("env3Sustain");
            data.env3Release = obj->getProperty("env3Release");
            
            // ENV4
            data.env4Attack = obj->getProperty("env4Attack");
            data.env4Decay = obj->getProperty("env4Decay");
            data.env4Sustain = obj->getProperty("env4Sustain");
            data.env4Release = obj->getProperty("env4Release");
            
            // LFO1
            data.lfo1Waveform = obj->getProperty("lfo1Waveform");
            data.lfo1Delay = obj->getProperty("lfo1Delay");
            data.lfo1Rate = obj->getProperty("lfo1Rate");
            
            // LFO2
            data.lfo2Waveform = obj->getProperty("lfo2Waveform");
            data.lfo2Delay = obj->getProperty("lfo2Delay");
            data.lfo2Rate = obj->getProperty("lfo2Rate");
            
            // Chorus
            data.chorusMode = obj->getProperty("chorusMode");
        }
        
        return data;
    }
};
