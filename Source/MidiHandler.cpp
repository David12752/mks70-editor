/*
  ==============================================================================

    MidiHandler.cpp
    MIDI communication handler for MKS-70 with Vecoven V4.x firmware

  ==============================================================================
*/

#include "MidiHandler.h"

//==============================================================================
MidiHandler::MidiHandler()
    : deviceId(0),
      isBulkDumpInProgress(false),
      expectedBulkMessages(0),
      receivedBulkMessages(0)
{
}

MidiHandler::~MidiHandler()
{
    if (midiInput != nullptr)
        midiInput->stop();
    if (controllerInput1 != nullptr)
        controllerInput1->stop();
    if (controllerInput2 != nullptr)
        controllerInput2->stop();
}

//==============================================================================
// Connection
//==============================================================================
void MidiHandler::setInputDevice(int index)
{
    if (midiInput != nullptr)
        midiInput->stop();
    auto devices = juce::MidiInput::getAvailableDevices();
    if (index >= 0 && index < devices.size()) {
        midiInput = juce::MidiInput::openDevice(devices[index].identifier, this);
        if (midiInput != nullptr) {
            midiInput->start();
            DBG("MidiHandler: MIDI IN device set: " + devices[index].name);
        } else {
            DBG("MidiHandler: Failed to open MIDI IN device: " + devices[index].name);
        }
    } else {
        DBG("MidiHandler: MIDI IN index out of range: " + juce::String(index));
    }
}

void MidiHandler::setOutputDevice(int index)
{
    midiOutput.reset();
    auto devices = juce::MidiOutput::getAvailableDevices();
    if (index >= 0 && index < devices.size()) {
        midiOutput = juce::MidiOutput::openDevice(devices[index].identifier);
        if (midiOutput != nullptr) {
            DBG("MidiHandler: MIDI OUT device set: " + devices[index].name);
        } else {
            DBG("MidiHandler: Failed to open MIDI OUT device: " + devices[index].name);
        }
    } else {
        DBG("MidiHandler: MIDI OUT index out of range: " + juce::String(index));
    }
}

void MidiHandler::setControllerInput1(int index)
{
    if (controllerInput1 != nullptr)
        controllerInput1->stop();
    controllerInput1.reset();
    
    auto devices = juce::MidiInput::getAvailableDevices();
    if (index >= 0 && index < devices.size()) {
        controllerInput1 = juce::MidiInput::openDevice(devices[index].identifier, this);
        if (controllerInput1 != nullptr) {
            controllerInput1->start();
            DBG("MidiHandler: Controller 1 MIDI IN device set: " + devices[index].name);
        } else {
            DBG("MidiHandler: Failed to open Controller 1 MIDI IN device: " + devices[index].name);
        }
    } else if (index < 0) {
        DBG("MidiHandler: Controller 1 MIDI IN disabled");
    }
}

void MidiHandler::setControllerInput2(int index)
{
    if (controllerInput2 != nullptr)
        controllerInput2->stop();
    controllerInput2.reset();
    
    auto devices = juce::MidiInput::getAvailableDevices();
    if (index >= 0 && index < devices.size()) {
        controllerInput2 = juce::MidiInput::openDevice(devices[index].identifier, this);
        if (controllerInput2 != nullptr) {
            controllerInput2->start();
            DBG("MidiHandler: Controller 2 MIDI IN device set: " + devices[index].name);
        } else {
            DBG("MidiHandler: Failed to open Controller 2 MIDI IN device: " + devices[index].name);
        }
    } else if (index < 0) {
        DBG("MidiHandler: Controller 2 MIDI IN disabled");
    }
}

bool MidiHandler::isConnected() const
{
    return (midiInput != nullptr && midiOutput != nullptr);
}

juce::StringArray MidiHandler::getAvailableMidiInputs() const
{
    juce::StringArray inputNames;
    auto devices = juce::MidiInput::getAvailableDevices();
    for (auto& device : devices)
        inputNames.add(device.name);
    return inputNames;
}

juce::StringArray MidiHandler::getAvailableMidiOutputs() const
{
    juce::StringArray outputNames;
    auto devices = juce::MidiOutput::getAvailableDevices();
    for (auto& device : devices)
        outputNames.add(device.name);
    return outputNames;
}

void MidiHandler::setDeviceId(int id)
{
    deviceId = juce::jlimit(0, 31, id);
}

//==============================================================================
// SysEx Message Construction Helpers
//==============================================================================
std::vector<uint8_t> MidiHandler::buildRolandSysEx(uint8_t cmd, const std::vector<uint8_t>& data)
{
    // MKS-70 Standard SysEx format (original firmware):
    // F0 41 [DevID] [Model1] [Model2] [Command] [Data...] [Checksum] F7
    // DevID = 0x39 (default), Model = 00 24, Command = 0x20 for tone data
    
    std::vector<uint8_t> message;
    
    message.push_back(MKS70Mapping::SYSEX_START);  // F0
    message.push_back(MKS70Mapping::ROLAND_ID);     // 41
    message.push_back(MKS70Mapping::DEVICE_ID + (deviceId & 0x0F)); // 39 + device offset
    message.push_back(MKS70Mapping::MODEL_ID1);    // 00
    message.push_back(MKS70Mapping::MODEL_ID2);    // 24
    message.push_back(cmd);                         // Command (0x20 for tone params)
    
    // Add data
    for (auto byte : data)
        message.push_back(byte);
    
    // MKS-70 doesn't use checksum for most messages, but we include it for compatibility
    // The checksum covers all data bytes
    int sum = cmd;
    for (auto byte : data)
        sum += byte;
    message.push_back((128 - (sum & 0x7F)) & 0x7F);
    
    message.push_back(MKS70Mapping::SYSEX_END);    // F7
    
    return message;
}

uint8_t MidiHandler::calculateChecksum(const std::vector<uint8_t>& data)
{
    int sum = 0;
    for (auto byte : data)
        sum += byte;
    return (128 - (sum & 0x7F)) & 0x7F;
}

std::vector<uint8_t> MidiHandler::buildDT1Message(uint8_t address1, uint8_t address2, uint8_t address3, uint8_t value)
{
    std::vector<uint8_t> data;
    data.push_back(address1);
    data.push_back(address2);
    data.push_back(address3);
    data.push_back(value & 0x7F);
    return buildRolandSysEx(0x12, data); // DT1 command
}

std::vector<uint8_t> MidiHandler::buildRQ1Message(uint8_t address1, uint8_t address2, uint8_t address3, uint8_t size)
{
    std::vector<uint8_t> data;
    data.push_back(address1);
    data.push_back(address2);
    data.push_back(address3);
    data.push_back(size & 0x7F);
    return buildRolandSysEx(0x11, data); // RQ1 command
}

//==============================================================================
// SysEx - Parameter Sending
//==============================================================================
void MidiHandler::sendSysEx(const std::vector<uint8_t>& data)
{
    if (midiOutput == nullptr)
        return;
    
    if (data.size() < 2)
        return;
    
    // Debug: Log the exact SysEx bytes being sent
    juce::String hexStr = "SysEx TX: ";
    for (auto byte : data)
        hexStr += juce::String::toHexString(byte).paddedLeft('0', 2).toUpperCase() + " ";
    DBG(hexStr);
    
    // JUCE's createSysExMessage expects data WITHOUT F0 and F7
    // Our data array includes F0 at start and F7 at end, so we skip them
    const uint8_t* dataStart = data.data();
    int dataLen = (int)data.size();
    
    // Skip F0 at start if present
    if (data[0] == 0xF0) {
        dataStart++;
        dataLen--;
    }
    // Skip F7 at end if present
    if (data.back() == 0xF7) {
        dataLen--;
    }
    
    auto message = juce::MidiMessage::createSysExMessage(dataStart, dataLen);
    midiOutput->sendMessageNow(message);
    
    // Add delay for reliable transmission to hardware
    if (sysexDelayMs > 0)
        juce::Thread::sleep(sysexDelayMs);
}

void MidiHandler::sendRawSysEx(const juce::MemoryBlock& data)
{
    if (midiOutput == nullptr)
    {
        DBG("MidiHandler ERROR: No MIDI Output device connected!");
        return;
    }
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data.getData());
    size_t size = data.getSize();
    
    DBG("MidiHandler: Sending " + juce::String(size) + " bytes of raw SysEx data");
    
    // Parse and send each SysEx message in the data
    size_t pos = 0;
    int messageCount = 0;
    
    while (pos < size)
    {
        // Find start of SysEx (0xF0)
        while (pos < size && bytes[pos] != 0xF0)
            pos++;
        
        if (pos >= size)
            break;
        
        size_t start = pos;
        
        // Find end of SysEx (0xF7)
        while (pos < size && bytes[pos] != 0xF7)
            pos++;
        
        if (pos >= size)
            break;
        
        pos++; // Include the 0xF7
        
        // Extract this SysEx message
        size_t msgLen = pos - start;
        if (msgLen > 2) // Must have at least F0 and F7
        {
            // JUCE expects data WITHOUT F0 and F7
            auto message = juce::MidiMessage::createSysExMessage(bytes + start + 1, (int)(msgLen - 2));
            midiOutput->sendMessageNow(message);
            messageCount++;
            
            // Debug first message
            if (messageCount <= 3)
            {
                juce::String hexStr = "SysEx TX[" + juce::String(messageCount) + "]: ";
                for (size_t i = start; i < pos && i < start + 20; i++)
                    hexStr += juce::String::toHexString(bytes[i]).paddedLeft('0', 2).toUpperCase() + " ";
                if (pos - start > 20)
                    hexStr += "...";
                DBG(hexStr);
            }
            
            // Add delay between messages
            if (sysexDelayMs > 0)
                juce::Thread::sleep(sysexDelayMs);
        }
    }
    
    DBG("MidiHandler: Sent " + juce::String(messageCount) + " SysEx messages");
}

void MidiHandler::sendRawSysExToBothTones(const juce::MemoryBlock& data)
{
    if (midiOutput == nullptr)
    {
        DBG("MidiHandler ERROR: No MIDI Output device connected!");
        return;
    }
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data.getData());
    size_t size = data.getSize();
    
    DBG("MidiHandler: Sending SysEx to BOTH tones - " + juce::String(size) + " bytes");
    
    // Parse and send each SysEx message to both tones
    size_t pos = 0;
    int messageCount = 0;
    
    while (pos < size)
    {
        // Find start of SysEx (0xF0)
        while (pos < size && bytes[pos] != 0xF0)
            pos++;
        
        if (pos >= size)
            break;
        
        size_t start = pos;
        
        // Find end of SysEx (0xF7)
        while (pos < size && bytes[pos] != 0xF7)
            pos++;
        
        if (pos >= size)
            break;
        
        pos++; // Include the 0xF7
        
        // Extract this SysEx message
        size_t msgLen = pos - start;
        if (msgLen > 8) // Must be a valid Roland message (F0 41 ... F7)
        {
            // Check if this is a Roland MKS-70 Tone message
            // Format: F0 41 36 [ch] 24 20 [tone] [addr] [val] F7
            // Or APR: F0 41 35 [ch] 24 20 [tone] [data...] F7
            if (bytes[start + 1] == 0x41 && // Roland ID
                (bytes[start + 2] == 0x36 || bytes[start + 2] == 0x35)) // IPR or APR
            {
                // This is a tone parameter message - send to both tones
                std::vector<uint8_t> upperMsg(bytes + start, bytes + pos);
                std::vector<uint8_t> lowerMsg(bytes + start, bytes + pos);
                
                // Find the tone byte position (usually byte 6)
                // Format: F0 41 36/35 [ch] 24 20 [tone] ...
                size_t toneBytePos = 6;
                if (msgLen > toneBytePos)
                {
                    upperMsg[toneBytePos] = MKS70Mapping::TONE_UPPER;  // 0x01
                    lowerMsg[toneBytePos] = MKS70Mapping::TONE_LOWER;  // 0x02
                    
                    // Send to Upper tone
                    auto msgUpper = juce::MidiMessage::createSysExMessage(upperMsg.data() + 1, (int)(upperMsg.size() - 2));
                    midiOutput->sendMessageNow(msgUpper);
                    
                    if (sysexDelayMs > 0)
                        juce::Thread::sleep(sysexDelayMs);
                    
                    // Send to Lower tone
                    auto msgLower = juce::MidiMessage::createSysExMessage(lowerMsg.data() + 1, (int)(lowerMsg.size() - 2));
                    midiOutput->sendMessageNow(msgLower);
                    
                    messageCount += 2;
                    
                    DBG("SysEx sent to BOTH tones");
                }
            }
            else
            {
                // Not a tone message - send as-is
                auto message = juce::MidiMessage::createSysExMessage(bytes + start + 1, (int)(msgLen - 2));
                midiOutput->sendMessageNow(message);
                messageCount++;
            }
            
            // Add delay between messages
            if (sysexDelayMs > 0)
                juce::Thread::sleep(sysexDelayMs);
        }
    }
    
    DBG("MidiHandler: Sent " + juce::String(messageCount) + " SysEx messages to both tones");
}

void MidiHandler::sendNoteOn(int channel, int note, int velocity)
{
    if (midiOutput == nullptr)
        return;
    midiOutput->sendMessageNow(juce::MidiMessage::noteOn(channel, note, (uint8_t)velocity));
}

void MidiHandler::sendNoteOff(int channel, int note)
{
    if (midiOutput == nullptr)
        return;
    midiOutput->sendMessageNow(juce::MidiMessage::noteOff(channel, note));
}

void MidiHandler::sendToneParameter(int tone, int param, int value)
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI Output device connected! Cannot send tone parameter.");
        return;
    }
    
    // Skip parameters marked as "not available" (V4.x-only parameters on V3.x firmware)
    if (param == MKS70Mapping::PARAM_NOT_AVAILABLE || param == 0xFF) {
        DBG("MidiHandler: Skipping unavailable parameter (V4.x only)");
        return;
    }
    
    // MKS-70 IPR (Individual Parameter Request) format for tone parameters:
    // F0 41 36 [Channel] 24 20 [Tone] [Address] [Value] F7
    // Channel = Device ID (0-15, default 0)
    // 24 = Model ID (MKS-70)
    // 20 = Level (Tone level)
    // Tone = 01 for Upper (Tone A), 02 for Lower (Tone B)
    // Address = Parameter address
    // Value = Parameter value (0-127)
    
    // Accept both formats: 0/1 (bool style) or 1/2 (MKS-70 style)
    // 0 or 1 = Upper, 2 = Lower
    uint8_t toneNum = (tone == 0 || tone == 1) ? MKS70Mapping::TONE_UPPER : MKS70Mapping::TONE_LOWER;
    
    std::vector<uint8_t> sysex;
    sysex.push_back(MKS70Mapping::SYSEX_START);    // F0
    sysex.push_back(MKS70Mapping::ROLAND_ID);       // 41 - Roland ID
    sysex.push_back(MKS70Mapping::CMD_IPR);         // 36 - IPR command
    sysex.push_back(static_cast<uint8_t>(deviceId & 0x0F)); // Channel (0-15)
    sysex.push_back(MKS70Mapping::MODEL_ID2);       // 24 - Model ID (MKS-70)
    sysex.push_back(MKS70Mapping::LEVEL_TONE);      // 20 - Tone level
    sysex.push_back(toneNum);                        // 01 or 02
    sysex.push_back(static_cast<uint8_t>(param & 0x7F));  // Parameter address
    sysex.push_back(static_cast<uint8_t>(value & 0x7F));  // Value
    sysex.push_back(MKS70Mapping::SYSEX_END);       // F7
    
    sendSysEx(sysex);
    
    DBG("MidiHandler: Sent IPR Tone - Ch:" + juce::String(deviceId) +
        " Tone:" + juce::String(toneNum) + 
        " Param:0x" + juce::String::toHexString(param) + 
        " Value:" + juce::String(value));
}

void MidiHandler::sendPatchParameter(int param, int value)
{
    sendGlobalParameter(param, value);
}

void MidiHandler::sendGlobalParameter(int param, int value)
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI Output device connected! Cannot send global parameter.");
        return;
    }
    
    // IPR format for MIDI menu parameters (Vecoven firmware):
    // F0 41 36 [CH] 24 30 01 [PARAM] [VALUE] F7
    // 36 = IPR command
    // 30 = Patch level
    // 01 = Group (patch)
    std::vector<uint8_t> sysex;
    sysex.push_back(MKS70Mapping::SYSEX_START);    // F0
    sysex.push_back(MKS70Mapping::ROLAND_ID);       // 41
    sysex.push_back(MKS70Mapping::CMD_IPR);         // 36 = IPR
    sysex.push_back(static_cast<uint8_t>(deviceId)); // Channel (device ID)
    sysex.push_back(MKS70Mapping::MODEL_ID2);       // 24
    sysex.push_back(MKS70Mapping::LEVEL_PATCH);     // 30 = Patch level
    sysex.push_back(MKS70Mapping::GROUP_PATCH);     // 01 = Group
    sysex.push_back(static_cast<uint8_t>(param & 0x7F));
    sysex.push_back(static_cast<uint8_t>(value & 0x7F));
    sysex.push_back(MKS70Mapping::SYSEX_END);       // F7
    
    sendSysEx(sysex);
    DBG("MidiHandler: Sent Global Param IPR - Param:" + juce::String::toHexString(param) + 
        " Value:" + juce::String(value));
}

//==============================================================================
// Bulk Data Send
//==============================================================================
void MidiHandler::sendCompleteTone(int tone, const ToneParams& params)
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI Output device connected! Cannot send complete tone.");
        return;
    }
    
    // Send all tone parameters via SysEx
    // DCO-1
    sendToneParameter(tone, MKS70Mapping::DCO1_RANGE_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::DCO1_RANGE_SYSEX, params.dco1Range));
    sendToneParameter(tone, MKS70Mapping::DCO1_WAVEFORM_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::DCO1_WAVEFORM_SYSEX, params.dco1Waveform));
    sendToneParameter(tone, MKS70Mapping::DCO1_TUNE_SYSEX, params.dco1Tune);
    sendToneParameter(tone, MKS70Mapping::DCO1_LFO_DEPTH_SYSEX, params.dco1LFOMod);
    sendToneParameter(tone, MKS70Mapping::DCO1_ENV_DEPTH_SYSEX, params.dco1ENVMod);
    sendToneParameter(tone, MKS70Mapping::DCO1_DYNAMICS_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::DCO1_DYNAMICS_SYSEX, params.dcoDynamics));
    sendToneParameter(tone, MKS70Mapping::DCO1_ENV_SOURCE_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::DCO1_ENV_SOURCE_SYSEX, params.dcoENVMode));
    
    // DCO-2
    sendToneParameter(tone, MKS70Mapping::DCO2_RANGE_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::DCO2_RANGE_SYSEX, params.dco2Range));
    sendToneParameter(tone, MKS70Mapping::DCO2_WAVEFORM_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::DCO2_WAVEFORM_SYSEX, params.dco2Waveform));
    sendToneParameter(tone, MKS70Mapping::DCO2_TUNE_SYSEX, params.dco2Tune);
    sendToneParameter(tone, MKS70Mapping::DCO2_FINETUNE_SYSEX, params.dco2FineTune);
    sendToneParameter(tone, MKS70Mapping::DCO2_LFO_DEPTH_SYSEX, params.dco2LFOMod);
    sendToneParameter(tone, MKS70Mapping::DCO2_ENV_DEPTH_SYSEX, params.dco2ENVMod);
    sendToneParameter(tone, MKS70Mapping::DCO_CROSSMOD_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::DCO_CROSSMOD_SYSEX, params.dco2CrossMod));
    
    // Mixer
    sendToneParameter(tone, MKS70Mapping::MIXER_DCO1_LEVEL_SYSEX, params.mixerDCO1Level);
    sendToneParameter(tone, MKS70Mapping::MIXER_DCO2_LEVEL_SYSEX, params.mixerDCO2Level);
    sendToneParameter(tone, MKS70Mapping::MIXER_ENV_DEPTH_SYSEX, params.mixerENVMod);
    sendToneParameter(tone, MKS70Mapping::MIXER_DYNAMICS_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::MIXER_DYNAMICS_SYSEX, params.mixerDynamics));
    sendToneParameter(tone, MKS70Mapping::MIXER_ENV_SOURCE_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::MIXER_ENV_SOURCE_SYSEX, params.mixerENVMode));
    
    // HPF & Chorus
    sendToneParameter(tone, MKS70Mapping::HPF_CUTOFF_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::HPF_CUTOFF_SYSEX, params.vcfHPF));
    sendToneParameter(tone, MKS70Mapping::CHORUS_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::CHORUS_SYSEX, params.chorusMode));
    
    // VCF
    sendToneParameter(tone, MKS70Mapping::VCF_CUTOFF_SYSEX, params.vcfCutoff);
    sendToneParameter(tone, MKS70Mapping::VCF_RESONANCE_SYSEX, params.vcfResonance);
    sendToneParameter(tone, MKS70Mapping::VCF_LFO1_DEPTH_SYSEX, params.vcfLFOMod);
    sendToneParameter(tone, MKS70Mapping::VCF_ENV_DEPTH_SYSEX, params.vcfENVMod);
    sendToneParameter(tone, MKS70Mapping::VCF_KEY_FOLLOW_SYSEX, params.vcfKeyFollow);
    sendToneParameter(tone, MKS70Mapping::VCF_DYNAMICS_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::VCF_DYNAMICS_SYSEX, params.vcfDynamics));
    sendToneParameter(tone, MKS70Mapping::VCF_ENV_SOURCE_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::VCF_ENV_SOURCE_SYSEX, params.vcfENVMode));
    
    // VCA
    sendToneParameter(tone, MKS70Mapping::VCA_LEVEL_SYSEX, params.vcaLevel);
    sendToneParameter(tone, MKS70Mapping::VCA_ENV_SOURCE_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::VCA_ENV_SOURCE_SYSEX, params.vcaMode));
    sendToneParameter(tone, MKS70Mapping::VCA_DYNAMICS_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::VCA_DYNAMICS_SYSEX, params.vcaDynamics));
    
    // LFO 1
    sendToneParameter(tone, MKS70Mapping::LFO1_WAVEFORM_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::LFO1_WAVEFORM_SYSEX, params.lfo1Waveform));
    sendToneParameter(tone, MKS70Mapping::LFO1_DELAY_SYSEX, params.lfo1Delay);
    sendToneParameter(tone, MKS70Mapping::LFO1_RATE_SYSEX, params.lfo1Rate);
    
    // ENV 1 (7-Stage)
    sendToneParameter(tone, MKS70Mapping::ENV1_TIME1_SYSEX, params.env1Time1);
    sendToneParameter(tone, MKS70Mapping::ENV1_LEVEL1_SYSEX, params.env1Level1);
    sendToneParameter(tone, MKS70Mapping::ENV1_TIME2_SYSEX, params.env1Time2);
    sendToneParameter(tone, MKS70Mapping::ENV1_LEVEL2_SYSEX, params.env1Level2);
    sendToneParameter(tone, MKS70Mapping::ENV1_TIME3_SYSEX, params.env1Time3);
    sendToneParameter(tone, MKS70Mapping::ENV1_LEVEL3_SYSEX, params.env1Level3);
    sendToneParameter(tone, MKS70Mapping::ENV1_TIME4_SYSEX, params.env1Time4);
    sendToneParameter(tone, MKS70Mapping::ENV1_KEYFOLLOW_LOOP_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::ENV1_KEYFOLLOW_LOOP_SYSEX, params.env1KeyFollow));
    
    // ENV 2 (7-Stage)
    sendToneParameter(tone, MKS70Mapping::ENV2_TIME1_SYSEX, params.env2Time1);
    sendToneParameter(tone, MKS70Mapping::ENV2_LEVEL1_SYSEX, params.env2Level1);
    sendToneParameter(tone, MKS70Mapping::ENV2_TIME2_SYSEX, params.env2Time2);
    sendToneParameter(tone, MKS70Mapping::ENV2_LEVEL2_SYSEX, params.env2Level2);
    sendToneParameter(tone, MKS70Mapping::ENV2_TIME3_SYSEX, params.env2Time3);
    sendToneParameter(tone, MKS70Mapping::ENV2_LEVEL3_SYSEX, params.env2Level3);
    sendToneParameter(tone, MKS70Mapping::ENV2_TIME4_SYSEX, params.env2Time4);
    sendToneParameter(tone, MKS70Mapping::ENV2_KEYFOLLOW_LOOP_SYSEX, MKS70Mapping::mapToneValueForSysex(MKS70Mapping::ENV2_KEYFOLLOW_LOOP_SYSEX, params.env2KeyFollow));
    
    // ENV 3 (ADSR) - VCF
    sendToneParameter(tone, MKS70Mapping::ENV3_ATTACK_SYSEX, params.env3Attack);
    sendToneParameter(tone, MKS70Mapping::ENV3_DECAY_SYSEX, params.env3Decay);
    sendToneParameter(tone, MKS70Mapping::ENV3_SUSTAIN_SYSEX, params.env3Sustain);
    sendToneParameter(tone, MKS70Mapping::ENV3_RELEASE_SYSEX, params.env3Release);
    
    // ENV 4 (ADSR) - VCA
    sendToneParameter(tone, MKS70Mapping::ENV4_ATTACK_SYSEX, params.env4Attack);
    sendToneParameter(tone, MKS70Mapping::ENV4_DECAY_SYSEX, params.env4Decay);
    sendToneParameter(tone, MKS70Mapping::ENV4_SUSTAIN_SYSEX, params.env4Sustain);
    sendToneParameter(tone, MKS70Mapping::ENV4_RELEASE_SYSEX, params.env4Release);
    
    DBG("MidiHandler: Sent complete tone " + juce::String(tone) + " (" + juce::String(82) + " parameters)");
}

void MidiHandler::sendCompletePatch(const PatchParameters& params)
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI Output device connected! Cannot send patch.");
        return;
    }
    
    // Send all global parameters via IPR SysEx
    // Format: F0 41 36 [Ch] 24 30 01 [Param] [Value] F7
    
    // Global Balance & Split
    sendGlobalParameter(MKS70Mapping::GLOBAL_UL_BALANCE_SYSEX, params.upperLowerBalance);
    sendGlobalParameter(MKS70Mapping::GLOBAL_UPPER_SPLIT_SYSEX, params.upperSplit);
    sendGlobalParameter(MKS70Mapping::GLOBAL_LOWER_SPLIT_SYSEX, params.lowerSplit);
    sendGlobalParameter(MKS70Mapping::GLOBAL_TOTAL_VOLUME_SYSEX, params.totalVolume);
    
    // Key Mode & Split
    sendGlobalParameter(0x10, params.toneSel);
    sendGlobalParameter(0x11, params.keyMode);
    sendGlobalParameter(0x12, params.splitPoint);
    sendGlobalParameter(0x13, params.balance);
    
    // Portamento
    sendGlobalParameter(0x14, params.portamentoSwitch);
    sendGlobalParameter(0x15, params.portamentoTime);
    
    // Bender
    sendGlobalParameter(0x16, params.bendRange);
    
    // Assignment Mode
    sendGlobalParameter(0x17, params.assignMode);
    sendGlobalParameter(0x18, params.glide);
    sendGlobalParameter(0x19, params.unisonDetune);
    
    // Chase Play
    sendGlobalParameter(0x1A, params.chasePlay);
    sendGlobalParameter(0x1B, params.chaseLevel);
    sendGlobalParameter(0x1C, params.chaseTime);
    
    DBG("MidiHandler: Sent complete patch (15 parameters)");
}

void MidiHandler::sendBulkDump(const PatchBank& bank)
{
    if (midiOutput == nullptr)
        return;
    
    // Send all 64 patches in bank
    for (int i = 0; i < 64 && i < bank.patches.size(); ++i)
    {
        const auto& patch = bank.patches[i];
        
        // Send upper tone
        sendCompleteTone(MKS70Mapping::TONE_UPPER, patch.upperTone);
        
        // Send lower tone
        sendCompleteTone(MKS70Mapping::TONE_LOWER, patch.lowerTone);
        
        // Send patch parameters
        sendCompletePatch(patch.patch);
        
        // Small delay between patches
        juce::Thread::sleep(50);
    }
}

void MidiHandler::sendPatchToMemory(int bank, int patchNumber, const ToneParams& upperTone, 
                                    const ToneParams& lowerTone, const PatchParameters& patch)
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI Output device connected! Cannot write to synth memory.");
        return;
    }
    
    // To write a patch to synth memory on MKS-70:
    // 1. First send the complete patch data (tones + patch params)
    // 2. The patch is now in the edit buffer
    // 3. Use Program Change to select the target slot
    // 4. On Vecoven firmware, use Write command to store
    
    DBG("MidiHandler: Writing patch to Bank " + juce::String(bank) + 
        " Slot " + juce::String(patchNumber));
    
    // First, send the complete patch to edit buffer
    sendCompleteTone(MKS70Mapping::TONE_UPPER, upperTone);
    juce::Thread::sleep(50);
    sendCompleteTone(MKS70Mapping::TONE_LOWER, lowerTone);
    juce::Thread::sleep(50);
    sendCompletePatch(patch);
    juce::Thread::sleep(50);
    
    // Select the target patch slot using Program Change
    // Bank A = patches 1-64 (PC 0-63)
    // Bank B = patches 65-128 (PC 0-63 with Bank Select)
    int programNumber = patchNumber & 0x3F; // 0-63
    
    // Send Bank Select if needed (for banks B, C, D)
    if (bank > 0) {
        midiOutput->sendMessageNow(juce::MidiMessage::controllerEvent(
            1, MKS70Mapping::CC_BANK_SELECT_MSB, bank & 0x03));
        juce::Thread::sleep(10);
    }
    
    // Send Program Change to select slot
    sendProgramChange(1, programNumber);
    
    DBG("MidiHandler: Patch sent to edit buffer. Select Write on synth to store.");
}

void MidiHandler::sendProgramChange(int channel, int program)
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI Output device for Program Change");
        return;
    }
    
    channel = juce::jlimit(1, 16, channel);
    program = juce::jlimit(0, 127, program);
    
    midiOutput->sendMessageNow(juce::MidiMessage::programChange(channel, program));
    DBG("MidiHandler: Sent Program Change Ch:" + juce::String(channel) + 
        " Prog:" + juce::String(program));
}

void MidiHandler::sendCC(int channel, int controller, int value)
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI Output device for CC");
        return;
    }
    
    channel = juce::jlimit(1, 16, channel);
    controller = juce::jlimit(0, 127, controller);
    value = juce::jlimit(0, 127, value);
    
    midiOutput->sendMessageNow(juce::MidiMessage::controllerEvent(channel, controller, value));
    DBG("MidiHandler: Sent CC Ch:" + juce::String(channel) + 
        " CC:" + juce::String(controller) + " Val:" + juce::String(value));
}

//==============================================================================
// SysEx - Data Requests
//==============================================================================
void MidiHandler::requestToneDump(int tone)
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI output device for requestToneDump");
        return;
    }
    
    // MKS-70 APR (All Parameters Request) for Tone:
    // F0 41 35 [Channel] 24 [Level=20 for tone] [Tone] F7
    // Channel = deviceId (0-15)
    // Level = 20 (Tone level)
    // Tone = 01 (Upper/A) or 02 (Lower/B)
    uint8_t toneNum = (tone == 0) ? MKS70Mapping::TONE_UPPER : MKS70Mapping::TONE_LOWER;
    
    std::vector<uint8_t> sysex;
    sysex.push_back(MKS70Mapping::SYSEX_START);    // F0
    sysex.push_back(MKS70Mapping::ROLAND_ID);       // 41
    sysex.push_back(MKS70Mapping::CMD_APR);         // 35 - APR command
    sysex.push_back(static_cast<uint8_t>(deviceId & 0x0F)); // Channel
    sysex.push_back(MKS70Mapping::MODEL_ID2);       // 24 - Model ID
    sysex.push_back(MKS70Mapping::LEVEL_TONE);      // 20 - Tone level
    sysex.push_back(toneNum);                        // 01 or 02
    sysex.push_back(MKS70Mapping::SYSEX_END);       // F7
    
    sendSysEx(sysex);
    DBG("MidiHandler: Sent APR request for Tone " + juce::String(toneNum == 1 ? "A (Upper)" : "B (Lower)"));
}

void MidiHandler::requestPatchDump()
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI output device for requestPatchDump");
        return;
    }
    
    // MKS-70 APR (All Parameters Request) for Patch:
    // F0 41 35 [Channel] 24 [Level=30 for patch] [Group=01] F7
    std::vector<uint8_t> sysex;
    sysex.push_back(MKS70Mapping::SYSEX_START);    // F0
    sysex.push_back(MKS70Mapping::ROLAND_ID);       // 41
    sysex.push_back(MKS70Mapping::CMD_APR);         // 35 - APR command
    sysex.push_back(static_cast<uint8_t>(deviceId & 0x0F)); // Channel
    sysex.push_back(MKS70Mapping::MODEL_ID2);       // 24 - Model ID
    sysex.push_back(MKS70Mapping::LEVEL_PATCH);     // 30 - Patch level
    sysex.push_back(MKS70Mapping::GROUP_PATCH);     // 01 - Group
    sysex.push_back(MKS70Mapping::SYSEX_END);       // F7
    
    sendSysEx(sysex);
    DBG("MidiHandler: Sent APR request for Patch");
}

void MidiHandler::requestBulkDump(int bank)
{
    if (midiOutput == nullptr) {
        DBG("MidiHandler ERROR: No MIDI output device for requestBulkDump");
        return;
    }
    
    // Request entire bank (64 patches)
    // This will initiate a multi-message dump
    isBulkDumpInProgress = true;
    expectedBulkMessages = 64;
    receivedBulkMessages = 0;
    bulkDumpBuffer.clear();
    
    // MKS-70 BLD (Bulk Dump Request) Format:
    // F0 41 34 [Channel] 24 [Bank] F7
    // Bank: A=0, B=1 (Internal), C=2, D=3 (Cartridge)
    std::vector<uint8_t> sysex;
    sysex.push_back(MKS70Mapping::SYSEX_START);    // F0
    sysex.push_back(MKS70Mapping::ROLAND_ID);       // 41
    sysex.push_back(MKS70Mapping::CMD_PGR);         // 34 - Bulk/Program request
    sysex.push_back(static_cast<uint8_t>(deviceId & 0x0F)); // Channel
    sysex.push_back(MKS70Mapping::MODEL_ID2);       // 24 - Model ID
    sysex.push_back(static_cast<uint8_t>(bank & 0x03)); // Bank 0-3
    sysex.push_back(MKS70Mapping::SYSEX_END);       // F7
    
    sendSysEx(sysex);
    DBG("MidiHandler: Sent Bulk Dump request for Bank " + juce::String(bank));
}

void MidiHandler::requestAllParameters()
{
    if (midiOutput == nullptr)
        return;
    
    // IPR (Identity Parameter Request) - request all current settings
    // This is vendor-specific - send standard identity request
    std::vector<uint8_t> ipr;
    ipr.push_back(0xF0);
    ipr.push_back(0x7E); // Universal Non-Real Time
    ipr.push_back(deviceId & 0x7F);
    ipr.push_back(0x06); // General Information
    ipr.push_back(0x01); // Identity Request
    ipr.push_back(0xF7);
    
    sendSysEx(ipr);
}

//==============================================================================
// NRPN (Vecoven V4.x)
//==============================================================================
void MidiHandler::sendNRPN(int channel, int paramMSB, int paramLSB, int value)
{
    if (midiOutput == nullptr)
        return;
    
    channel = juce::jlimit(1, 16, channel);
    
    // NRPN parameter MSB (CC 99)
    midiOutput->sendMessageNow(juce::MidiMessage::controllerEvent(channel, 99, paramMSB & 0x7F));
    
    // NRPN parameter LSB (CC 98)
    midiOutput->sendMessageNow(juce::MidiMessage::controllerEvent(channel, 98, paramLSB & 0x7F));
    
    // Data Entry MSB (CC 6)
    midiOutput->sendMessageNow(juce::MidiMessage::controllerEvent(channel, 6, (value >> 7) & 0x7F));
    
    // Data Entry LSB (CC 38)
    midiOutput->sendMessageNow(juce::MidiMessage::controllerEvent(channel, 38, value & 0x7F));
}

void MidiHandler::sendNRPNToneParam(bool isUpper, int param, int value)
{
    int paramMSB = isUpper ? MKS70Mapping::NRPN_TONE_A : MKS70Mapping::NRPN_TONE_B;
    sendNRPN(1, paramMSB, param, value);
}

//==============================================================================
// Incoming MIDI Message Handler
//==============================================================================
void MidiHandler::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    // Debug: Log which input received the message
    juce::String sourceName = "Unknown";
    if (source == midiInput.get())
        sourceName = "Synth Input";
    else if (source == controllerInput1.get())
        sourceName = "Controller 1";
    else if (source == controllerInput2.get())
        sourceName = "Controller 2";
    
    DBG("MIDI IN from " + sourceName + ": " + message.getDescription());
    
    if (onRawMidi) onRawMidi(message);
    
    // MIDI Thru - forward non-SysEx messages to output (from any input)
    if (midiThruEnabled && midiOutput != nullptr && !message.isSysEx())
    {
        midiOutput->sendMessageNow(message);
        
        // Debug: Log MIDI Thru messages
        if (message.isNoteOn())
            DBG("MIDI Thru -> Synth: Note On Ch" + juce::String(message.getChannel()) + " Note:" + juce::String(message.getNoteNumber()) + " Vel:" + juce::String(message.getVelocity()));
        else if (message.isNoteOff())
            DBG("MIDI Thru -> Synth: Note Off Ch" + juce::String(message.getChannel()) + " Note:" + juce::String(message.getNoteNumber()));
        else if (message.isController())
            DBG("MIDI Thru -> Synth: CC" + juce::String(message.getControllerNumber()) + " Val:" + juce::String(message.getControllerValue()));
    }
    
    if (message.isSysEx())
    {
        parseSysEx(message);
    }
    // Could also handle NRPN responses here if needed
}

void MidiHandler::loadSysExFromData(const void* data, size_t size)
{
    // Parse multiple SysEx messages from raw data (file dump)
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    size_t pos = 0;
    
    while (pos < size)
    {
        // Find SysEx start
        while (pos < size && bytes[pos] != 0xF0)
            pos++;
        
        if (pos >= size)
            break;
        
        // Find SysEx end
        size_t start = pos;
        while (pos < size && bytes[pos] != 0xF7)
            pos++;
        
        if (pos >= size)
            break;
        
        pos++; // Include the F7
        
        // Create MidiMessage and parse it
        juce::MidiMessage sysexMsg(bytes + start, (int)(pos - start));
        parseSysEx(sysexMsg);
    }
    
    DBG("Loaded SysEx file with " + juce::String(size) + " bytes");
}

//==============================================================================
// SysEx Parsing
//==============================================================================
bool MidiHandler::validateRolandSysEx(const std::vector<uint8_t>& data)
{
    if (data.size() < 8)
        return false;
    
    // Check Roland SysEx header
    if (data[0] != MKS70Mapping::SYSEX_START)
        return false;
    if (data[1] != MKS70Mapping::ROLAND_ID)
        return false;
    if (data[3] != MKS70Mapping::MODEL_ID1)
        return false;
    if (data[4] != MKS70Mapping::MODEL_ID2)
        return false;
    if (data[data.size() - 1] != MKS70Mapping::SYSEX_END)
        return false;
    
    // Verify checksum
    std::vector<uint8_t> checksumData(data.begin() + 5, data.end() - 2);
    uint8_t expectedChecksum = data[data.size() - 2];
    uint8_t actualChecksum = calculateChecksum(checksumData);
    
    return (expectedChecksum == actualChecksum);
}

void MidiHandler::parseSysEx(const juce::MidiMessage& msg)
{
    auto rawData = msg.getSysExData();
    auto size = msg.getSysExDataSize();
    
    // Convert to vector for easier handling
    std::vector<uint8_t> data;
    data.push_back(0xF0); // Add SysEx start
    for (int i = 0; i < size; ++i)
        data.push_back(rawData[i]);
    data.push_back(0xF7); // Add SysEx end
    
    if (data.size() < 5 || data[0] != MKS70Mapping::SYSEX_START ||
        data[1] != MKS70Mapping::ROLAND_ID || data.back() != MKS70Mapping::SYSEX_END)
    {
        if (onError)
            onError("Invalid Roland SysEx message received");
        return;
    }

    const uint8_t vecovenCmd = data[2];
    if (vecovenCmd == MKS70Mapping::CMD_IPR ||
        vecovenCmd == MKS70Mapping::CMD_APR ||
        vecovenCmd == MKS70Mapping::CMD_PGR)
    {
        parseAPR(data);
        return;
    }
    
    // Validate standard Roland DT1/RQ1 style messages.
    if (!validateRolandSysEx(data))
    {
        if (onError)
            onError("Invalid SysEx message received");
        return;
    }
    
    // Check command byte
    uint8_t cmd = data[5];
    
    switch (cmd)
    {
        case 0x12: // DT1 - Data Set (response to request)
            parseDT1(data);
            break;
            
        case 0x11: // RQ1 - Request (shouldn't receive this as editor)
            break;
            
        default:
            if (onError)
                onError("Unknown SysEx command: " + juce::String::toHexString(cmd));
            break;
    }
}

void MidiHandler::parseDT1(const std::vector<uint8_t>& data)
{
    if (data.size() < 10)
        return;
    
    // Extract address
    uint8_t addr1 = data[6];
    uint8_t addr2 = data[7];
    uint8_t addr3 = data[8];
    
    // Check if this is a tone dump
    if (addr1 == 0x00 && (addr2 == MKS70Mapping::TONE_UPPER || addr2 == MKS70Mapping::TONE_LOWER))
    {
        ToneParams params;
        
        // Parse all 93 parameters from data[9] onwards
        // This is a simplified version - actual implementation would need
        // to map each byte to the correct parameter
        
        if (onToneReceived)
            onToneReceived(params, addr2);
    }
    // Check if this is a patch dump
    else if (addr1 == 0x00 && addr2 == 0x00)
    {
        PatchParameters params;
        
        // Parse all 10 global parameters
        
        if (onPatchReceived)
            onPatchReceived(params);
    }
}

void MidiHandler::parseAPR(const std::vector<uint8_t>& data)
{
    if (data.size() < 8)
        return;
    
    const uint8_t cmd = data[2];
    const uint8_t level = data[5];
    const uint8_t target = data[6];

    if (cmd == MKS70Mapping::CMD_IPR)
    {
        if (data.size() < 10)
            return;

        const int param = data[7];
        const int sysexValue = data[8];

        if (level == MKS70Mapping::LEVEL_TONE)
        {
            const int uiValue = MKS70Mapping::unmapToneValueFromSysex(param, sysexValue);
            if (onToneParameterReceived)
                onToneParameterReceived(target, param, uiValue);
        }
        else if (level == MKS70Mapping::LEVEL_PATCH)
        {
            if (onPatchParameterReceived)
                onPatchParameterReceived(param, sysexValue);
        }

        return;
    }

    if (cmd != MKS70Mapping::CMD_APR)
        return;

    // Short APR messages are requests, not responses.
    if (data.size() <= 8)
        return;

    if (level == MKS70Mapping::LEVEL_TONE)
    {
        constexpr int dataStart = 7;
        constexpr int nameLength = 10;
        constexpr int paramCount = 50;
        const int paramStart = dataStart + nameLength;

        if (data.size() < static_cast<size_t>(paramStart + paramCount + 1))
            return;

        ToneParams params;
        params.dco1Range = data[paramStart + 0];
        params.dco1Waveform = data[paramStart + 1];
        params.dco1Tune = data[paramStart + 2];
        params.dco1LFOMod = data[paramStart + 3];
        params.dco1ENVMod = data[paramStart + 4];

        params.dco2Range = data[paramStart + 5];
        params.dco2Waveform = data[paramStart + 6];
        params.dco2CrossMod = data[paramStart + 7];
        params.dco2Tune = data[paramStart + 8];
        params.dco2FineTune = data[paramStart + 9];
        params.dco2LFOMod = data[paramStart + 10];
        params.dco2ENVMod = data[paramStart + 11];
        params.dcoDynamics = data[paramStart + 12];
        params.dcoENVMode = data[paramStart + 13];

        params.mixerDCO1Level = data[paramStart + 14];
        params.mixerDCO2Level = data[paramStart + 15];
        params.mixerENVMod = data[paramStart + 16];
        params.mixerDynamics = data[paramStart + 17];
        params.mixerENVMode = data[paramStart + 18];

        params.vcfHPF = data[paramStart + 19];
        params.vcfCutoff = data[paramStart + 20];
        params.vcfResonance = data[paramStart + 21];
        params.vcfLFOMod = data[paramStart + 22];
        params.vcfENVMod = data[paramStart + 23];
        params.vcfKeyFollow = data[paramStart + 24];
        params.vcfDynamics = data[paramStart + 25];
        params.vcfENVMode = data[paramStart + 26];

        params.vcaLevel = data[paramStart + 27];
        params.vcaDynamics = data[paramStart + 28];
        params.chorusMode = data[paramStart + 29];

        params.lfo1Waveform = data[paramStart + 30];
        params.lfo1Delay = data[paramStart + 31];
        params.lfo1Rate = data[paramStart + 32];

        params.env1Time1 = data[paramStart + 33];
        params.env1Level1 = data[paramStart + 34];
        params.env1Time2 = data[paramStart + 35];
        params.env1Level2 = data[paramStart + 36];
        params.env1Time3 = data[paramStart + 37];
        params.env1Level3 = data[paramStart + 38];
        params.env1Time4 = data[paramStart + 39];
        params.env1KeyFollow = data[paramStart + 40];

        params.env2Time1 = data[paramStart + 41];
        params.env2Level1 = data[paramStart + 42];
        params.env2Time2 = data[paramStart + 43];
        params.env2Level2 = data[paramStart + 44];
        params.env2Time3 = data[paramStart + 45];
        params.env2Level3 = data[paramStart + 46];
        params.env2Time4 = data[paramStart + 47];
        params.env2KeyFollow = data[paramStart + 48];

        params.vcaMode = data[paramStart + 49];

        if (onToneReceived)
            onToneReceived(params, target);
    }
}

void MidiHandler::parseBulkDump(const std::vector<uint8_t>& data)
{
    if (!isBulkDumpInProgress)
        return;
    
    // Accumulate bulk dump data
    bulkDumpBuffer.insert(bulkDumpBuffer.end(), data.begin(), data.end());
    receivedBulkMessages++;
    
    // Check if we've received all expected messages
    if (receivedBulkMessages >= expectedBulkMessages)
    {
        isBulkDumpInProgress = false;
        
        // Parse complete bank data
        PatchBank bank;
        
        // Extract 64 patches from buffer
        // This would need proper parsing logic
        
        if (onBulkDumpReceived)
            onBulkDumpReceived(bank);
        
        bulkDumpBuffer.clear();
    }
}

//==============================================================================
// Parameter Mapping Helpers
//==============================================================================
uint8_t MidiHandler::getToneAddress(int tone, int param)
{
    // Map parameter index to SysEx address
    // This maps the 0-92 parameter indices to actual SysEx addresses
    
    static const uint8_t addressMap[] = {
        // DCO1 (0-7)
        MKS70Mapping::DCO1_RANGE_SYSEX,
        MKS70Mapping::DCO1_WAVEFORM_SYSEX,
        MKS70Mapping::DCO1_TUNE_SYSEX,
        MKS70Mapping::DCO1_LFO_DEPTH_SYSEX,
        MKS70Mapping::DCO1_LFO_SOURCE_SYSEX,
        MKS70Mapping::DCO1_ENV_DEPTH_SYSEX,
        MKS70Mapping::DCO1_DYNAMICS_SYSEX,
        MKS70Mapping::DCO1_ENV_SOURCE_SYSEX,
        
        // DCO2 (8-15)
        MKS70Mapping::DCO2_RANGE_SYSEX,
        MKS70Mapping::DCO2_WAVEFORM_SYSEX,
        MKS70Mapping::DCO2_TUNE_SYSEX,
        MKS70Mapping::DCO2_LFO_DEPTH_SYSEX,
        MKS70Mapping::DCO2_LFO_SOURCE_SYSEX,
        MKS70Mapping::DCO2_ENV_DEPTH_SYSEX,
        MKS70Mapping::DCO2_DYNAMICS_SYSEX,
        MKS70Mapping::DCO2_ENV_SOURCE_SYSEX,
        
        // DCO Mods (16-17)
        MKS70Mapping::DCO_CROSSMOD_SYSEX,
        MKS70Mapping::DCO2_FINETUNE_SYSEX,
        
        // HPF & Chorus (18-19)
        MKS70Mapping::HPF_CUTOFF_SYSEX,
        MKS70Mapping::CHORUS_SYSEX,
        
        // PWM DCO1 (20-25)
        MKS70Mapping::PWM_DCO1_WIDTH_SYSEX,
        MKS70Mapping::PWM_DCO1_ENV_DEPTH_SYSEX,
        MKS70Mapping::PWM_DCO1_LFO_DEPTH_SYSEX,
        MKS70Mapping::PWM_DCO1_LFO_SOURCE_SYSEX,
        MKS70Mapping::PWM_DCO1_DYNAMICS_SYSEX,
        MKS70Mapping::PWM_DCO1_ENV_SOURCE_SYSEX,
        
        // PWM DCO2 (26-31)
        MKS70Mapping::PWM_DCO2_WIDTH_SYSEX,
        MKS70Mapping::PWM_DCO2_ENV_DEPTH_SYSEX,
        MKS70Mapping::PWM_DCO2_LFO_DEPTH_SYSEX,
        MKS70Mapping::PWM_DCO2_LFO_SOURCE_SYSEX,
        MKS70Mapping::PWM_DCO2_DYNAMICS_SYSEX,
        MKS70Mapping::PWM_DCO2_ENV_SOURCE_SYSEX,
        
        // Mixer (32-36)
        MKS70Mapping::MIXER_DCO1_LEVEL_SYSEX,
        MKS70Mapping::MIXER_DCO2_LEVEL_SYSEX,
        MKS70Mapping::MIXER_ENV_DEPTH_SYSEX,
        MKS70Mapping::MIXER_DYNAMICS_SYSEX,
        MKS70Mapping::MIXER_ENV_SOURCE_SYSEX,
        
        // VCF (37-44)
        MKS70Mapping::VCF_CUTOFF_SYSEX,
        MKS70Mapping::VCF_RESONANCE_SYSEX,
        MKS70Mapping::VCF_LFO1_DEPTH_SYSEX,
        MKS70Mapping::VCF_LFO2_DEPTH_SYSEX,
        MKS70Mapping::VCF_ENV_DEPTH_SYSEX,
        MKS70Mapping::VCF_KEY_FOLLOW_SYSEX,
        MKS70Mapping::VCF_DYNAMICS_SYSEX,
        MKS70Mapping::VCF_ENV_SOURCE_SYSEX,
        
        // VCA (45-47)
        MKS70Mapping::VCA_LEVEL_SYSEX,
        MKS70Mapping::VCA_ENV_SOURCE_SYSEX,
        MKS70Mapping::VCA_DYNAMICS_SYSEX,
        
        // LFO1 (48-52)
        MKS70Mapping::LFO1_WAVEFORM_SYSEX,
        MKS70Mapping::LFO1_DELAY_SYSEX,
        MKS70Mapping::LFO1_RATE_SYSEX,
        MKS70Mapping::LFO1_LFO2_DEPTH_SYSEX,
        MKS70Mapping::LFO1_SYNC_SYSEX,
        
        // LFO2 (53-57)
        MKS70Mapping::LFO2_WAVEFORM_SYSEX,
        MKS70Mapping::LFO2_DELAY_SYSEX,
        MKS70Mapping::LFO2_RATE_SYSEX,
        MKS70Mapping::LFO2_LFO1_DEPTH_SYSEX,
        MKS70Mapping::LFO2_SYNC_SYSEX,
        
        // ENV1 (58-65)
        MKS70Mapping::ENV1_TIME1_SYSEX,
        MKS70Mapping::ENV1_LEVEL1_SYSEX,
        MKS70Mapping::ENV1_TIME2_SYSEX,
        MKS70Mapping::ENV1_LEVEL2_SYSEX,
        MKS70Mapping::ENV1_TIME3_SYSEX,
        MKS70Mapping::ENV1_LEVEL3_SYSEX,
        MKS70Mapping::ENV1_TIME4_SYSEX,
        MKS70Mapping::ENV1_KEYFOLLOW_LOOP_SYSEX,
        
        // ENV2 (66-73)
        MKS70Mapping::ENV2_TIME1_SYSEX,
        MKS70Mapping::ENV2_LEVEL1_SYSEX,
        MKS70Mapping::ENV2_TIME2_SYSEX,
        MKS70Mapping::ENV2_LEVEL2_SYSEX,
        MKS70Mapping::ENV2_TIME3_SYSEX,
        MKS70Mapping::ENV2_LEVEL3_SYSEX,
        MKS70Mapping::ENV2_TIME4_SYSEX,
        MKS70Mapping::ENV2_KEYFOLLOW_LOOP_SYSEX,
        
        // ENV3 (74-77)
        MKS70Mapping::ENV3_ATTACK_SYSEX,
        MKS70Mapping::ENV3_DECAY_SYSEX,
        MKS70Mapping::ENV3_SUSTAIN_SYSEX,
        MKS70Mapping::ENV3_RELEASE_SYSEX,
        
        // ENV4 (78-81)
        MKS70Mapping::ENV4_ATTACK_SYSEX,
        MKS70Mapping::ENV4_DECAY_SYSEX,
        MKS70Mapping::ENV4_SUSTAIN_SYSEX,
        MKS70Mapping::ENV4_RELEASE_SYSEX
    };
    
    if (param >= 0 && param < 82)
        return addressMap[param];
    
    return 0;
}

uint8_t MidiHandler::getGlobalAddress(int param)
{
    // Map global parameter index to SysEx address
    static const uint8_t globalAddressMap[] = {
        0x10, // toneSelect
        0x11, // keyMode
        MKS70Mapping::GLOBAL_UPPER_SPLIT_SYSEX, // splitPoint
        MKS70Mapping::GLOBAL_UL_BALANCE_SYSEX,  // balance
        0x13, // portamentoSwitch
        0x14, // portamentoTime
        0x15, // pitchBendRange
        0x16, // unisonDetune
        0x17, // assignMode
        0x18  // glide
    };
    
    if (param >= 0 && param < 10)
        return globalAddressMap[param];
    
    return 0;
}
