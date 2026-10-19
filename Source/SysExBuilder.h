/*
  ==============================================================================

    SysExBuilder.h
    Roland MKS-70 SysEx and NRPN Message Builder
    
    Provides functions for building:
    - IPR (Individual Parameter Request/Send) messages
    - APR (All Parameters) messages  
    - PGR (Program Number) messages
    - NRPN messages (Vecoven 3.x)
    - Bulk dump requests

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

namespace MKS70
{
namespace SysEx
{

//==============================================================================
// Constants
//==============================================================================

// Roland SysEx identifiers
constexpr uint8_t SYSEX_START = 0xF0;
constexpr uint8_t SYSEX_END = 0xF7;
constexpr uint8_t ROLAND_ID = 0x41;

// Operation codes
constexpr uint8_t OP_WST = 0x31;  // Want to Send
constexpr uint8_t OP_RQS = 0x32;  // Request
constexpr uint8_t OP_ACK = 0x33;  // Acknowledge
constexpr uint8_t OP_PGR = 0x34;  // Program Number
constexpr uint8_t OP_APR = 0x35;  // All Parameters
constexpr uint8_t OP_IPR = 0x36;  // Individual Parameter

// MKS-70 Format type
constexpr uint8_t MKS70_FORMAT = 0x24;

// Level bytes
constexpr uint8_t LEVEL_TONE = 0x20;
constexpr uint8_t LEVEL_PATCH = 0x30;

// Group bytes
constexpr uint8_t GROUP_PATCH = 0x01;
constexpr uint8_t GROUP_TONE_A = 0x01;
constexpr uint8_t GROUP_TONE_B = 0x02;

//==============================================================================
// NRPN Target for Vecoven 3.x
//==============================================================================

enum class NRPNTarget : uint8_t
{
    UPPER = 0x00,   // Upper / Tone A
    LOWER = 0x01,   // Lower / Tone B
    BOTH = 0x02     // Both tones
};

//==============================================================================
// Message Building Functions
//==============================================================================

/**
 * Build an IPR (Individual Parameter) message.
 * Used to set a single parameter on the MKS-70.
 * 
 * @param channel   MIDI channel (1-16)
 * @param isPatches true for patch parameters, false for tone parameters
 * @param group     Group number (1 for patch, 1-2 for tone A/B)
 * @param param     Parameter number
 * @param value     Parameter value (0-127)
 * @return          SysEx message as MidiMessage
 */
inline juce::MidiMessage buildIPR(int channel, bool isPatch, int group, int param, int value)
{
    uint8_t data[] = {
        SYSEX_START,
        ROLAND_ID,
        OP_IPR,
        static_cast<uint8_t>((channel - 1) & 0x0F),
        MKS70_FORMAT,
        isPatch ? LEVEL_PATCH : LEVEL_TONE,
        static_cast<uint8_t>(group & 0x7F),
        static_cast<uint8_t>(param & 0x7F),
        static_cast<uint8_t>(value & 0x7F),
        SYSEX_END
    };
    
    return juce::MidiMessage::createSysExMessage(data + 1, sizeof(data) - 2);
}

/**
 * Build an APR request message.
 * Used to request all parameters from the MKS-70.
 * 
 * @param channel   MIDI channel (1-16)
 * @param isPatch   true for patch, false for tone
 * @param group     Group number
 * @return          SysEx message as MidiMessage
 */
inline juce::MidiMessage buildAPRRequest(int channel, bool isPatch, int group)
{
    uint8_t data[] = {
        SYSEX_START,
        ROLAND_ID,
        OP_RQS,
        static_cast<uint8_t>((channel - 1) & 0x0F),
        MKS70_FORMAT,
        isPatch ? LEVEL_PATCH : LEVEL_TONE,
        static_cast<uint8_t>(group & 0x7F),
        SYSEX_END
    };
    
    return juce::MidiMessage::createSysExMessage(data + 1, sizeof(data) - 2);
}

/**
 * Build a PGR (Program Number) message.
 * Used to select a patch or tone by number.
 * 
 * @param channel   MIDI channel (1-16)
 * @param isPatch   true for patch (0-63), false for tone (0-99)
 * @param group     Group number
 * @param program   Program number
 * @return          SysEx message as MidiMessage
 */
inline juce::MidiMessage buildPGR(int channel, bool isPatch, int group, int program)
{
    uint8_t data[] = {
        SYSEX_START,
        ROLAND_ID,
        OP_PGR,
        static_cast<uint8_t>((channel - 1) & 0x0F),
        MKS70_FORMAT,
        isPatch ? LEVEL_PATCH : LEVEL_TONE,
        static_cast<uint8_t>(group & 0x7F),
        static_cast<uint8_t>(program & 0x7F),
        0x00,
        SYSEX_END
    };
    
    return juce::MidiMessage::createSysExMessage(data + 1, sizeof(data) - 2);
}

/**
 * Build a WST (Want to Send / Bulk Dump Request) message.
 * Used to request a bulk dump from the MKS-70.
 * 
 * @param channel   MIDI channel (1-16)
 * @param isPatch   true for patch level, false for tone level
 * @param group     Group number
 * @return          SysEx message as MidiMessage
 */
inline juce::MidiMessage buildBulkDumpRequest(int channel, bool isPatch, int group)
{
    uint8_t data[] = {
        SYSEX_START,
        ROLAND_ID,
        OP_WST,
        static_cast<uint8_t>((channel - 1) & 0x0F),
        MKS70_FORMAT,
        isPatch ? LEVEL_PATCH : LEVEL_TONE,
        static_cast<uint8_t>(group & 0x7F),
        SYSEX_END
    };
    
    return juce::MidiMessage::createSysExMessage(data + 1, sizeof(data) - 2);
}

/**
 * Build an ACK (Acknowledge) message.
 * Used to acknowledge receipt of data.
 * 
 * @param channel   MIDI channel (1-16)
 * @return          SysEx message as MidiMessage
 */
inline juce::MidiMessage buildACK(int channel)
{
    uint8_t data[] = {
        SYSEX_START,
        ROLAND_ID,
        OP_ACK,
        static_cast<uint8_t>((channel - 1) & 0x0F),
        MKS70_FORMAT,
        SYSEX_END
    };
    
    return juce::MidiMessage::createSysExMessage(data + 1, sizeof(data) - 2);
}

/**
 * Build MIDI Identity Request (Universal SysEx).
 * Used to test connection and identify device.
 * 
 * @return          Identity Request message
 */
inline juce::MidiMessage buildIdentityRequest()
{
    // Universal SysEx: F0 7E 7F 06 01 F7
    uint8_t data[] = {
        SYSEX_START,
        0x7E,       // Universal Non-Realtime
        0x7F,       // All devices
        0x06,       // General Information
        0x01,       // Identity Request
        SYSEX_END
    };
    
    return juce::MidiMessage::createSysExMessage(data + 1, sizeof(data) - 2);
}

//==============================================================================
// NRPN Message Building (Vecoven 3.x)
//==============================================================================

/**
 * Build NRPN messages for Vecoven 3.x tone parameter control.
 * Returns a vector of CC messages to send in sequence.
 * 
 * @param channel   MIDI channel (1-16)
 * @param target    NRPNTarget (UPPER, LOWER, or BOTH)
 * @param paramCC   Parameter CC number
 * @param value     Parameter value (0-127)
 * @return          Vector of MidiMessages (CC messages)
 */
inline std::vector<juce::MidiMessage> buildNRPN(int channel, NRPNTarget target, int paramCC, int value)
{
    std::vector<juce::MidiMessage> messages;
    int ch = (channel - 1) & 0x0F;
    
    // CC 99: NRPN MSB (Target)
    messages.push_back(juce::MidiMessage::controllerEvent(ch + 1, 99, static_cast<int>(target)));
    
    // CC 98: NRPN LSB (Parameter)
    messages.push_back(juce::MidiMessage::controllerEvent(ch + 1, 98, paramCC & 0x7F));
    
    // CC 6: Data Entry MSB (Value)
    messages.push_back(juce::MidiMessage::controllerEvent(ch + 1, 6, value & 0x7F));
    
    return messages;
}

/**
 * Build NRPN messages with fine value (14-bit).
 * 
 * @param channel   MIDI channel (1-16)
 * @param target    NRPNTarget (UPPER, LOWER, or BOTH)
 * @param paramCC   Parameter CC number
 * @param valueMSB  Parameter value MSB (0-127)
 * @param valueLSB  Parameter value LSB (0-127)
 * @return          Vector of MidiMessages (CC messages)
 */
inline std::vector<juce::MidiMessage> buildNRPN14Bit(int channel, NRPNTarget target, int paramCC, int valueMSB, int valueLSB)
{
    std::vector<juce::MidiMessage> messages;
    int ch = (channel - 1) & 0x0F;
    
    // CC 99: NRPN MSB (Target)
    messages.push_back(juce::MidiMessage::controllerEvent(ch + 1, 99, static_cast<int>(target)));
    
    // CC 98: NRPN LSB (Parameter)
    messages.push_back(juce::MidiMessage::controllerEvent(ch + 1, 98, paramCC & 0x7F));
    
    // CC 6: Data Entry MSB
    messages.push_back(juce::MidiMessage::controllerEvent(ch + 1, 6, valueMSB & 0x7F));
    
    // CC 38: Data Entry LSB
    messages.push_back(juce::MidiMessage::controllerEvent(ch + 1, 38, valueLSB & 0x7F));
    
    return messages;
}

//==============================================================================
// Parsing Functions
//==============================================================================

/**
 * Check if a SysEx message is from Roland MKS-70.
 */
inline bool isMKS70SysEx(const juce::MidiMessage& msg)
{
    if (!msg.isSysEx())
        return false;
    
    auto data = msg.getSysExData();
    auto size = msg.getSysExDataSize();
    
    if (size < 3)
        return false;
    
    return data[0] == ROLAND_ID && data[3] == MKS70_FORMAT;
}

/**
 * Get the operation code from a Roland SysEx message.
 */
inline uint8_t getOperationCode(const juce::MidiMessage& msg)
{
    if (!msg.isSysEx())
        return 0;
    
    auto data = msg.getSysExData();
    auto size = msg.getSysExDataSize();
    
    if (size < 2)
        return 0;
    
    return data[1];
}

/**
 * Check if a SysEx message is an APR (All Parameters) message.
 */
inline bool isAPRMessage(const juce::MidiMessage& msg)
{
    return isMKS70SysEx(msg) && getOperationCode(msg) == OP_APR;
}

/**
 * Check if a SysEx message is an IPR (Individual Parameter) message.
 */
inline bool isIPRMessage(const juce::MidiMessage& msg)
{
    return isMKS70SysEx(msg) && getOperationCode(msg) == OP_IPR;
}

/**
 * Parse an IPR message to extract parameter info.
 * 
 * @param msg       The MIDI message to parse
 * @param channel   Output: MIDI channel (1-16)
 * @param isPatch   Output: true if patch, false if tone
 * @param group     Output: Group number
 * @param param     Output: Parameter number
 * @param value     Output: Parameter value
 * @return          true if successfully parsed
 */
inline bool parseIPR(const juce::MidiMessage& msg, int& channel, bool& isPatch, int& group, int& param, int& value)
{
    if (!isIPRMessage(msg))
        return false;
    
    auto data = msg.getSysExData();
    auto size = msg.getSysExDataSize();
    
    if (size < 8)
        return false;
    
    channel = (data[2] & 0x0F) + 1;
    isPatch = (data[4] == LEVEL_PATCH);
    group = data[5];
    param = data[6];
    value = data[7];
    
    return true;
}

//==============================================================================
// Utility Functions
//==============================================================================

/**
 * Format a SysEx message as a hex string for debugging.
 */
inline juce::String toHexString(const juce::MidiMessage& msg)
{
    if (!msg.isSysEx())
        return "[Not SysEx]";
    
    juce::String hex;
    auto data = msg.getSysExData();
    auto size = msg.getSysExDataSize();
    
    hex << "F0 ";
    for (int i = 0; i < size; ++i)
        hex << juce::String::toHexString(data[i]).paddedLeft('0', 2).toUpperCase() << " ";
    hex << "F7";
    
    return hex;
}

} // namespace SysEx
} // namespace MKS70
