/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToneParams.h"
#include "PatchParameters.h"
#include "ArpParameters.h"
#include "MidiHandler.h"
#include "MappingLoader.h"
#include "CompareManager.h"
#include "UndoManager.h"

//==============================================================================
/**
*/
class MKS70EditorAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MKS70EditorAudioProcessor();
    ~MKS70EditorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Accessors for UI
    MidiHandler& getMidiHandler() { return midiHandler; }
    MappingLoader& getMappingLoader() { return mappingLoader; }
    CompareManager& getCompareManager() { return compareManager; }
    
    ToneParams& getToneUpper() { return toneUpper; }
    ToneParams& getToneLower() { return toneLower; }
    PatchParameters& getPatchParameters() { return patchParameters; }
    ArpParameters& getArpParameters() { return arpParameters; }
    
    void setToneParameter(bool isUpper, int paramIndex, int value);
    void setToneParameters(bool isUpper, const ToneParams& params, bool sendToSynth = false);
    void setPatchParameter(int paramIndex, int value);
    void setPatchParameters(const PatchParameters& params, bool sendToSynth = false);
    void setArpParameter(int paramIndex, int value);
    void setArpParameters(const ArpParameters& params, bool sendToSynth = false);
    
    void requestToneDump(bool isUpper);
    void requestPatchDump();
    void requestBulkDump(int bank);
    
    void sendToneToSynth(bool isUpper);
    void sendPatchToSynth();
    
    // Compare mode operations
    void storeCurrentToSlot(CompareManager::Slot slot);
    void loadSlotToCurrent(CompareManager::Slot slot);
    std::vector<ParameterDiff> compareAB();
    
    // Public compare manager for direct access
    CompareManager compareManager;
    
    // Undo/Redo manager
    MKS70UndoManager& getUndoManager() { return undoManager; }
    
    //==============================================================================
    // Total Recall settings for DAW integration
    bool totalRecallEnabled = true;       // Enable state save/restore
    bool sendSysExOnLoad = true;          // Send SysEx when DAW project loads
    bool requestSysExOnLoad = false;      // Request current state from synth
    bool sendOnEditEnabled = true;        // Send real-time parameter edits to synth
    
    void setTotalRecallEnabled(bool enabled) { totalRecallEnabled = enabled; }
    bool isTotalRecallEnabled() const { return totalRecallEnabled; }
    
    void setSendSysExOnLoad(bool enabled) { sendSysExOnLoad = enabled; }
    bool getSendSysExOnLoad() const { return sendSysExOnLoad; }
    
    void setRequestSysExOnLoad(bool enabled) { requestSysExOnLoad = enabled; }
    bool getRequestSysExOnLoad() const { return requestSysExOnLoad; }

    void setSendOnEditEnabled(bool enabled) { sendOnEditEnabled = enabled; }
    bool getSendOnEditEnabled() const { return sendOnEditEnabled; }
    
private:
    //==============================================================================
    // Core components
    MidiHandler midiHandler;
    MappingLoader mappingLoader;
    MKS70UndoManager undoManager;
    
    // Current edit buffers
    ToneParams toneUpper;
    ToneParams toneLower;
    PatchParameters patchParameters;
    ArpParameters arpParameters;
    
    // Callbacks from MidiHandler
    void handleToneReceived(ToneParams& tone, int toneNumber);
    void handleToneParameterReceived(int toneNumber, int paramIndex, int value);
    void handlePatchReceived(PatchParameters& patch);
    void handlePatchParameterReceived(int paramIndex, int value);
    void handleBulkDumpReceived(const PatchBank& bank);
    void handleMidiError(const juce::String& error);

    int getToneParameterValue(const ToneParams& tone, int paramIndex) const;
    void updateToneParameterValue(ToneParams& tone, int paramIndex, int value);
    void updatePatchParameterValue(PatchParameters& patch, int paramIndex, int value);
    void updateArpParameterValue(ArpParameters& arp, int paramIndex, int value);
    
    //==============================================================================
    // XML Serialization helpers for Total Recall
    void storeToneToXml(juce::XmlElement& xml, const ToneParams& tone);
    void loadToneFromXml(const juce::XmlElement& xml, ToneParams& tone);
    void storePatchToXml(juce::XmlElement& xml, const PatchParameters& patch);
    void loadPatchFromXml(const juce::XmlElement& xml, PatchParameters& patch);
    void storeArpToXml(juce::XmlElement& xml, const ArpParameters& arp);
    void loadArpFromXml(const juce::XmlElement& xml, ArpParameters& arp);
    void storeFullPatchToXml(juce::XmlElement& xml, const FullPatch& fullPatch);
    void loadFullPatchFromXml(const juce::XmlElement& xml, FullPatch& fullPatch);
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MKS70EditorAudioProcessor)
};
