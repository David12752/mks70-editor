/*
  ==============================================================================

    MidiHandler.h
    MIDI communication handler for MKS-70 with Vecoven V4.x firmware

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>
#include "ToneParams.h"
#include "PatchParameters.h"
#include "ArpParameters.h"
#include "MKS70_Mapping.h"

//==============================================================================
// Bank structure for bulk dumps
//==============================================================================
struct PatchBank
{
    struct PatchData
    {
        ToneParams upperTone;
        ToneParams lowerTone;
        PatchParameters patch;
        juce::String name;
    };
    
    std::vector<PatchData> patches; // 64 patches per bank
    int bankNumber; // 0-3
    
    PatchBank() : bankNumber(0) 
    {
        patches.resize(64);
    }
};

//==============================================================================
// MidiHandler class manages all MIDI communication with the MKS-70
// Supports both SysEx (Roland standard) and NRPN (Vecoven firmware)
//==============================================================================
class MidiHandler : public juce::MidiInputCallback
{
public:
    enum FirmwareVersion {
        Original = 1,
        VecovenX3 = 2,
        Vecoven4x = 3
    };
    void setFirmwareVersion(FirmwareVersion v) { firmwareVersion = v; }
    FirmwareVersion getFirmwareVersion() const { return firmwareVersion; }
    MidiHandler();
    ~MidiHandler() override;
    
    //==============================================================================
    // Connection
    //==============================================================================
    void setInputDevice(int index);
    void setOutputDevice(int index);
    bool isConnected() const;
    juce::StringArray getAvailableMidiInputs() const;
    juce::StringArray getAvailableMidiOutputs() const;
    
    // Controller MIDI inputs (for MIDI Thru to synth)
    void setControllerInput1(int index);
    void setControllerInput2(int index);
    
    // Device ID configuration
    void setDeviceId(int id); // 0-31, default 0
    int getDeviceId() const { return deviceId; }
    
    // MIDI Channel (1-16) - stored as midiChannel internally
    void setMidiChannel(int channel) { midiChannel = juce::jlimit(1, 16, channel); }
    int getMidiChannel() const { return midiChannel; }
    
    // Connection status
    bool isOutputConnected() const { return midiOutput != nullptr; }
    bool isInputConnected() const { return midiInput != nullptr; }
    
    //==============================================================================
    // SysEx - Parameter Sending
    //==============================================================================
    void sendSysEx(const std::vector<uint8_t>& data);
    void sendRawSysEx(const juce::MemoryBlock& data);  // Send raw SysEx file data
    void sendRawSysExToBothTones(const juce::MemoryBlock& data);  // Send to both Upper and Lower
    void sendToneParameter(int tone, int param, int value); // tone: 1=Upper, 2=Lower
    void sendPatchParameter(int param, int value);
    void sendGlobalParameter(int param, int value); // MIDI menu / arpeggiator params
    
    // Bulk sends
    void sendCompleteTone(int tone, const ToneParams& params);
    void sendCompletePatch(const PatchParameters& params);
    void sendBulkDump(const PatchBank& bank);
    
    // Write to synth memory
    void sendPatchToMemory(int bank, int patchNumber, const ToneParams& upperTone, 
                          const ToneParams& lowerTone, const PatchParameters& patch);
    void sendProgramChange(int channel, int program); // Select patch on synth
    void sendCC(int channel, int controller, int value); // Send Control Change
    
    //==============================================================================
    // SysEx - Data Requests
    //==============================================================================
    void requestToneDump(int tone); // 1=Upper, 2=Lower
    void requestPatchDump();
    void requestBulkDump(int bank); // 0-3
    void requestAllParameters(); // IPR (Identity Parameter Request)
    
    //==============================================================================
    // NRPN (Vecoven V4.x firmware)
    //==============================================================================
    void sendNRPN(int channel, int paramMSB, int paramLSB, int value);
    void sendNRPNToneParam(bool isUpper, int param, int value); // Helper for tone params
    
    //==============================================================================
    // Incoming MIDI
    //==============================================================================
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    
    // Load SysEx from file data
    void loadSysExFromData(const void* data, size_t size);
    
    // Callbacks for received data
    std::function<void(ToneParams&, int tone)> onToneReceived;
    std::function<void(int tone, int param, int value)> onToneParameterReceived;
    std::function<void(PatchParameters&)> onPatchReceived;
    std::function<void(int param, int value)> onPatchParameterReceived;
    std::function<void(PatchBank&)> onBulkDumpReceived;
    std::function<void(const juce::String&)> onError;
    // Raw MIDI callback (for MIDI Learn / monitoring)
    std::function<void(const juce::MidiMessage&)> onRawMidi;
    
    // MIDI Thru
    void setMidiThruEnabled(bool enabled) { midiThruEnabled = enabled; }
    bool isMidiThruEnabled() const { return midiThruEnabled; }
    
    // Send Note On/Off directly
    void sendNoteOn(int channel, int note, int velocity);
    void sendNoteOff(int channel, int note);
    
    // SysEx delay for reliable transmission
    void setSysExDelay(int delayMs) { sysexDelayMs = delayMs; }
    
private:
    FirmwareVersion firmwareVersion = Original;
    //==============================================================================
    // MIDI I/O
    //==============================================================================
    std::unique_ptr<juce::MidiInput> midiInput;
    std::unique_ptr<juce::MidiInput> controllerInput1;  // Controller keyboard 1
    std::unique_ptr<juce::MidiInput> controllerInput2;  // Controller keyboard 2
    std::unique_ptr<juce::MidiOutput> midiOutput;
    int deviceId; // Roland Device ID (0-31)
    int midiChannel = 1; // MIDI channel (1-16)
    bool midiThruEnabled = true; // Forward incoming MIDI to output
    int sysexDelayMs = 20; // Delay between SysEx messages
    
    //==============================================================================
    // SysEx Message Construction
    //==============================================================================
    std::vector<uint8_t> buildRolandSysEx(uint8_t cmd, const std::vector<uint8_t>& data);
    uint8_t calculateChecksum(const std::vector<uint8_t>& data);
    
    // Build specific message types
    std::vector<uint8_t> buildDT1Message(uint8_t address1, uint8_t address2, uint8_t address3, uint8_t value);
    std::vector<uint8_t> buildRQ1Message(uint8_t address1, uint8_t address2, uint8_t address3, uint8_t size);
    
    //==============================================================================
    // SysEx Parsing
    //==============================================================================
    void parseSysEx(const juce::MidiMessage& msg);
    void parseAPR(const std::vector<uint8_t>& data); // Auto Parameter Response
    void parseDT1(const std::vector<uint8_t>& data); // Data Set response
    void parseBulkDump(const std::vector<uint8_t>& data);
    
    // Validate SysEx message
    bool validateRolandSysEx(const std::vector<uint8_t>& data);
    
    //==============================================================================
    // Buffer for multi-message dumps
    //==============================================================================
    std::vector<uint8_t> bulkDumpBuffer;
    bool isBulkDumpInProgress;
    int expectedBulkMessages;
    int receivedBulkMessages;
    
    //==============================================================================
    // Parameter mapping helpers
    //==============================================================================
    uint8_t getToneAddress(int tone, int param); // Get SysEx address for tone parameter
    uint8_t getGlobalAddress(int param); // Get SysEx address for global parameter
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiHandler)
};
