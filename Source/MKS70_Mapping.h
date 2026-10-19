/*
  ==============================================================================

    MKS70_Mapping.h
    Complete MIDI mapping constants for MKS-70
    
    SUPPORTS: Vecoven 3.x firmware
    
    ═══════════════════════════════════════════════════════════════════════════
    VECOVEN 3.x ADDRESS MAP (DECIMAL) - VERIFIED December 2025
    ───────────────────────────────────────────────────────────────────────────
    DCO-1:    11-15  (Range, WF, Tune, LFO, ENV)
    DCO-2:    16-22  (Range, WF, XMOD, Tune, FineTune, LFO, ENV)
    DCO Dyna: 26-27  (Dynamics, Mode)
    Mixer:    28-32  (DCO1, DCO2, ENV, Dyna, Mode)
    VCF:      33-40  (HPF, Freq, Res, LFO, ENV, Key, Dyna, Mode)
    VCA:      41-43  (Level, Dyna, Chorus)
    LFO:      44-46  (WF, Delay, Rate)
    ENV-1:    47-51  (ATT, DECY, SUS, REL, KEY)
    ENV-2:    52-56  (ATT, DECY, SUS, REL, KEY)
    VCA Mode: 58     (Gate/ENV2)
    
    Extensions: 59-63 (PWM1, PWM2, LFO Fine, VCF Key Fine, LFO Phase) - TODO
    ═══════════════════════════════════════════════════════════════════════════
    
    SysEx Format: F0 41 36 [Channel] 24 20 [Tone] [Address] [Value] F7
    - 41 = Roland ID
    - 36 = IPR (Individual Parameter) command
    - Channel = Device ID (0-15)
    - 24 = MKS-70 Model ID
    - 20 = Tone level
    - Tone: 01 = Tone A (Upper), 02 = Tone B (Lower)
    
    NRPN Format (Vecoven 3.x+):
    - CC 99 (MSB): 00 = Tone A, 01 = Tone B, 02 = Both
    - CC 98 (LSB): Parameter number
    - CC 6: Value (Data)

  ==============================================================================
*/

#pragma once

namespace MKS70Mapping
{
    // SysEx Constants
    constexpr unsigned char SYSEX_START = 0xF0;
    constexpr unsigned char SYSEX_END = 0xF7;
    constexpr unsigned char ROLAND_ID = 0x41;
    constexpr unsigned char DEVICE_ID = 0x39;
    constexpr unsigned char MODEL_ID1 = 0x00;
    constexpr unsigned char MODEL_ID2 = 0x24;
    constexpr unsigned char COMMAND_ID = 0x20;
    
    // Tone IDs
    constexpr unsigned char TONE_UPPER = 0x01;  // Tone A
    constexpr unsigned char TONE_LOWER = 0x02;  // Tone B
    
    // NRPN MSB Values
    constexpr unsigned char NRPN_TONE_A = 0x00;
    constexpr unsigned char NRPN_TONE_B = 0x01;
    constexpr unsigned char NRPN_BOTH = 0x02;
    
    // ============================================================================
    // VECOVEN 3.x FIRMWARE - CORRECT ADDRESS MAPPING
    // Discovered via manual testing on real hardware - December 2025
    // ============================================================================
    namespace V3
    {
        // ═══════════════════════════════════════════════════════════════════════
        // DCO-1 Parameters (Address 11-15)
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char DCO1_RANGE = 11;          // 0-3: 16', 8', 4', 2'
        constexpr unsigned char DCO1_WAVEFORM = 12;       // 0=Noise, 36=Square, 70=Pulse, 127=Saw
        constexpr unsigned char DCO1_TUNE = 13;           // 0=-12, 127=+12
        constexpr unsigned char DCO1_LFO_DEPTH = 14;      // 0-99
        constexpr unsigned char DCO1_ENV_DEPTH = 15;      // 0-99
        
        // ═══════════════════════════════════════════════════════════════════════
        // DCO-2 Parameters (Address 16-22)
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char DCO2_RANGE = 16;          // 0-3: 16', 8', 4', 2'
        constexpr unsigned char DCO2_WAVEFORM = 17;       // Same as DCO1
        constexpr unsigned char DCO2_CROSSMOD = 18;       // 0=Off, 33=Sync1, 70=Sync2, 102=XMod
        constexpr unsigned char DCO2_TUNE = 19;           // 0=-12, 127=+12
        constexpr unsigned char DCO2_FINE_TUNE = 20;      // 0=-50, 127=+50
        constexpr unsigned char DCO2_LFO_DEPTH = 21;      // 0-99
        constexpr unsigned char DCO2_ENV_DEPTH = 22;      // 0-99

        // ═══════════════════════════════════════════════════════════════════════
        // DCO Dynamics (Address 26-27)
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char DCO_DYNAMICS = 26;        // 0=Off, 40=1, 76=2, 104=3
        constexpr unsigned char DCO_MODE = 27;            // DCO Mode

        // ═══════════════════════════════════════════════════════════════════════
        // Mixer (Address 28-32)
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char MIX_DCO1 = 28;            // 0-127
        constexpr unsigned char MIX_DCO2 = 29;            // 0-127
        constexpr unsigned char MIX_ENV_DEPTH = 30;       // MIX ENV
        constexpr unsigned char MIX_DYNAMICS = 31;        // 0=Off, 127=3
        constexpr unsigned char MIX_ENV_MODE = 32;        // MIX MODE

        // ═══════════════════════════════════════════════════════════════════════
        // VCF (Address 33-40)
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char VCF_HPF = 33;             // HPF FREQ (High Pass Filter)
        constexpr unsigned char VCF_CUTOFF = 34;          // VCF FREQ (Cutoff)
        constexpr unsigned char VCF_RESONANCE = 35;       // VCF RES
        constexpr unsigned char VCF_LFO_DEPTH = 36;       // VCF LFO
        constexpr unsigned char VCF_ENV_DEPTH = 37;       // VCF ENV
        constexpr unsigned char VCF_KEY_FOLLOW = 38;      // VCF KEY
        constexpr unsigned char VCF_DYNAMICS = 39;        // VCF DYNA
        constexpr unsigned char VCF_ENV_MODE = 40;        // VCF MODE

        // ═══════════════════════════════════════════════════════════════════════
        // VCA (Address 41-43)
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char VCA_LEVEL = 41;           // VCA LEVEL
        constexpr unsigned char VCA_DYNAMICS = 42;        // VCA DYNA
        constexpr unsigned char CHORUS = 43;              // CHORUS 0=Off

        // ═══════════════════════════════════════════════════════════════════════
        // LFO (Address 44-46) - VERIFIED December 2025
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char LFO_WAVEFORM = 44;        // LFO WF: 0=Random, 127=Sine
        constexpr unsigned char LFO_DELAY = 45;           // LFO DELAY
        constexpr unsigned char LFO_RATE = 46;            // LFO RATE

        // ═══════════════════════════════════════════════════════════════════════
        // ENV-1 (Address 47-51) - VERIFIED December 2025
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char ENV1_ATTACK = 47;         // ENV-1 ATT
        constexpr unsigned char ENV1_DECAY = 48;          // ENV-1 DECY
        constexpr unsigned char ENV1_SUSTAIN = 49;        // ENV-1 SUS
        constexpr unsigned char ENV1_RELEASE = 50;        // ENV-1 REL
        constexpr unsigned char ENV1_KEY_FOLLOW = 51;     // ENV-1 KEY: 0=Off

        // ═══════════════════════════════════════════════════════════════════════
        // ENV-2 (Address 52-56) - VERIFIED December 2025
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char ENV2_ATTACK = 52;         // ENV-2 ATT
        constexpr unsigned char ENV2_DECAY = 53;          // ENV-2 DECY
        constexpr unsigned char ENV2_SUSTAIN = 54;        // ENV-2 SUS
        constexpr unsigned char ENV2_RELEASE = 55;        // ENV-2 REL
        constexpr unsigned char ENV2_KEY_FOLLOW = 56;     // ENV-2 KEY: 0=Off, 127=3
        
        // ═══════════════════════════════════════════════════════════════════════
        // VCA Mode (Address 58) - VERIFIED December 2025
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char VCA_ENV_MODE = 58;        // VCA MODE: 0=Gate, ~100=ENV2
        
        // ═══════════════════════════════════════════════════════════════════════
        // ⭐ Vecoven V3.x Extensions - TODO: Verify these addresses
        // Address 57 shows DCO1 RANGE (duplicate?), investigate further
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char DCO1_PWM = 59;            // EXT - DCO 1 PWM Depth - TODO: Verify
        constexpr unsigned char DCO2_PWM = 60;            // EXT - DCO 2 PWM Depth - TODO: Verify
        constexpr unsigned char LFO_RATE_FINE = 61;       // EXT - LFO Rate Fine - TODO: Verify
        constexpr unsigned char VCF_KEY_FOLLOW_FINE = 62; // EXT - VCF Key Follow Fine - TODO: Verify
        constexpr unsigned char LFO_PHASE_RESET = 63;     // EXT - LFO Phase Reset Mode - TODO: Verify
        
        // ═══════════════════════════════════════════════════════════════════════
        // Legacy aliases for backward compatibility
        // ═══════════════════════════════════════════════════════════════════════
        constexpr unsigned char ENV1_T1 = ENV1_ATTACK;
        constexpr unsigned char ENV1_L1 = ENV1_DECAY;
        constexpr unsigned char ENV1_T2 = ENV1_SUSTAIN;
        constexpr unsigned char ENV1_L2 = ENV1_RELEASE;
        constexpr unsigned char ENV1_T3 = ENV1_RELEASE;
        constexpr unsigned char ENV1_L3 = ENV1_RELEASE;
        constexpr unsigned char ENV1_T4 = ENV1_KEY_FOLLOW;
        
        constexpr unsigned char ENV2_T1 = ENV2_ATTACK;
        constexpr unsigned char ENV2_L1 = ENV2_DECAY;
        constexpr unsigned char ENV2_T2 = ENV2_SUSTAIN;
        constexpr unsigned char ENV2_L2 = ENV2_RELEASE;
        constexpr unsigned char ENV2_T3 = ENV2_RELEASE;
        constexpr unsigned char ENV2_L3 = ENV2_RELEASE;
        constexpr unsigned char ENV2_T4 = ENV2_KEY_FOLLOW;
        
        constexpr unsigned char DCO_ENV_MODE = DCO_MODE;
        constexpr unsigned char CHORUS_RATE = CHORUS;
    }
    
    // ============================================================================
    // VECOVEN 4.x FIRMWARE - EXTENDED ADDRESS MAPPING
    // V4.x adds: LFO2, PWM depth per DCO, more ENV types, arpeggiator extensions
    // ============================================================================
    namespace V4
    {
        // ═══════════════════════════════════════════════════════════════════════
        // V4.x inherits all V3.x addresses 11-58
        // ═══════════════════════════════════════════════════════════════════════
        
        // DCO-1 (same as V3.x)
        constexpr unsigned char DCO1_RANGE = V3::DCO1_RANGE;
        constexpr unsigned char DCO1_WAVEFORM = V3::DCO1_WAVEFORM;
        constexpr unsigned char DCO1_TUNE = V3::DCO1_TUNE;
        constexpr unsigned char DCO1_LFO_DEPTH = V3::DCO1_LFO_DEPTH;
        constexpr unsigned char DCO1_ENV_DEPTH = V3::DCO1_ENV_DEPTH;
        
        // DCO-2 (same as V3.x)
        constexpr unsigned char DCO2_RANGE = V3::DCO2_RANGE;
        constexpr unsigned char DCO2_WAVEFORM = V3::DCO2_WAVEFORM;
        constexpr unsigned char DCO2_CROSSMOD = V3::DCO2_CROSSMOD;
        constexpr unsigned char DCO2_TUNE = V3::DCO2_TUNE;
        constexpr unsigned char DCO2_FINE_TUNE = V3::DCO2_FINE_TUNE;
        constexpr unsigned char DCO2_LFO_DEPTH = V3::DCO2_LFO_DEPTH;
        constexpr unsigned char DCO2_ENV_DEPTH = V3::DCO2_ENV_DEPTH;
        
        // DCO Dynamics (same as V3.x)
        constexpr unsigned char DCO_DYNAMICS = V3::DCO_DYNAMICS;
        constexpr unsigned char DCO_MODE = V3::DCO_MODE;
        
        // Mixer (same as V3.x)
        constexpr unsigned char MIX_DCO1 = V3::MIX_DCO1;
        constexpr unsigned char MIX_DCO2 = V3::MIX_DCO2;
        constexpr unsigned char MIX_ENV_DEPTH = V3::MIX_ENV_DEPTH;
        constexpr unsigned char MIX_DYNAMICS = V3::MIX_DYNAMICS;
        constexpr unsigned char MIX_ENV_MODE = V3::MIX_ENV_MODE;
        
        // VCF (same as V3.x)
        constexpr unsigned char VCF_HPF = V3::VCF_HPF;
        constexpr unsigned char VCF_CUTOFF = V3::VCF_CUTOFF;
        constexpr unsigned char VCF_RESONANCE = V3::VCF_RESONANCE;
        constexpr unsigned char VCF_LFO_DEPTH = V3::VCF_LFO_DEPTH;
        constexpr unsigned char VCF_ENV_DEPTH = V3::VCF_ENV_DEPTH;
        constexpr unsigned char VCF_KEY_FOLLOW = V3::VCF_KEY_FOLLOW;
        constexpr unsigned char VCF_DYNAMICS = V3::VCF_DYNAMICS;
        constexpr unsigned char VCF_ENV_MODE = V3::VCF_ENV_MODE;
        
        // VCA (same as V3.x)
        constexpr unsigned char VCA_LEVEL = V3::VCA_LEVEL;
        constexpr unsigned char VCA_DYNAMICS = V3::VCA_DYNAMICS;
        constexpr unsigned char CHORUS = V3::CHORUS;
        
        // LFO1 (same as V3.x)
        constexpr unsigned char LFO_WAVEFORM = V3::LFO_WAVEFORM;
        constexpr unsigned char LFO_DELAY = V3::LFO_DELAY;
        constexpr unsigned char LFO_RATE = V3::LFO_RATE;
        
        // ENV-1 & ENV-2 (same as V3.x)
        constexpr unsigned char ENV1_ATTACK = V3::ENV1_ATTACK;
        constexpr unsigned char ENV1_DECAY = V3::ENV1_DECAY;
        constexpr unsigned char ENV1_SUSTAIN = V3::ENV1_SUSTAIN;
        constexpr unsigned char ENV1_RELEASE = V3::ENV1_RELEASE;
        constexpr unsigned char ENV1_KEY_FOLLOW = V3::ENV1_KEY_FOLLOW;
        constexpr unsigned char ENV2_ATTACK = V3::ENV2_ATTACK;
        constexpr unsigned char ENV2_DECAY = V3::ENV2_DECAY;
        constexpr unsigned char ENV2_SUSTAIN = V3::ENV2_SUSTAIN;
        constexpr unsigned char ENV2_RELEASE = V3::ENV2_RELEASE;
        constexpr unsigned char ENV2_KEY_FOLLOW = V3::ENV2_KEY_FOLLOW;
        constexpr unsigned char VCA_ENV_MODE = V3::VCA_ENV_MODE;
        
        // ═══════════════════════════════════════════════════════════════════════
        // V4.x EXCLUSIVE - Extended parameters (address 59+)
        // Note: These addresses need verification on real V4.x hardware
        // ═══════════════════════════════════════════════════════════════════════
        
        // PWM Depth per DCO (V4.x only)
        constexpr unsigned char DCO1_PWM = 59;             // DCO 1 PWM Depth 0-127
        constexpr unsigned char DCO2_PWM = 60;             // DCO 2 PWM Depth 0-127
        
        // LFO Extensions (V4.x only)
        constexpr unsigned char LFO_RATE_FINE = 61;        // LFO Rate Fine tune
        constexpr unsigned char LFO_SYNC = 62;             // LFO Sync mode (off/key/tempo)
        constexpr unsigned char LFO_PHASE_RESET = 63;      // LFO Phase Reset Mode
        
        // VCF Extensions (V4.x only)  
        constexpr unsigned char VCF_KEY_FOLLOW_FINE = 64;  // VCF Key Follow Fine
        constexpr unsigned char VCF_LFO2_DEPTH = 65;       // VCF modulated by LFO2
        
        // LFO2 (V4.x only) - Second LFO for more modulation
        constexpr unsigned char LFO2_WAVEFORM = 66;        // LFO2 Waveform
        constexpr unsigned char LFO2_RATE = 67;            // LFO2 Rate
        constexpr unsigned char LFO2_DELAY = 68;           // LFO2 Delay
        constexpr unsigned char LFO2_DEST = 69;            // LFO2 Destination (VCF/VCA/DCO)
        constexpr unsigned char LFO2_DEPTH = 70;           // LFO2 Depth
        
        // ENV-3 (V4.x only - for more modulation options)
        constexpr unsigned char ENV3_ATTACK = 71;
        constexpr unsigned char ENV3_DECAY = 72;
        constexpr unsigned char ENV3_SUSTAIN = 73;
        constexpr unsigned char ENV3_RELEASE = 74;
        
        // ENV-4 (V4.x only)
        constexpr unsigned char ENV4_ATTACK = 75;
        constexpr unsigned char ENV4_DECAY = 76;
        constexpr unsigned char ENV4_SUSTAIN = 77;
        constexpr unsigned char ENV4_RELEASE = 78;
        
        // Extended Arpeggiator (V4.x enhanced)
        constexpr unsigned char ARP_BPM = 80;              // BPM 40-300
        constexpr unsigned char ARP_MODE = 81;             // UP/DOWN/U-D/PLAY/RND
        constexpr unsigned char ARP_OCTAVE = 82;           // Octave span 1-4
        constexpr unsigned char ARP_RATE = 83;             // 1/4, 1/8, 1/16, etc.
        constexpr unsigned char ARP_LATCH = 84;            // Latch on/off
        constexpr unsigned char ARP_SWING = 85;            // Swing amount (V4.x only)
        constexpr unsigned char ARP_GATE = 86;             // Gate length (V4.x only)
    }
    
    // ============================================================================
    // NRPN Parameter Numbers (Vecoven 3.x) - Used with CC 98/99
    // Sequential numbering 0-49 for NRPN LSB
    // ============================================================================
    namespace NRPN
    {
        // DCO-1 (0-4)
        constexpr unsigned char DCO1_RANGE = 0;
        constexpr unsigned char DCO1_WAVEFORM = 1;
        constexpr unsigned char DCO1_TUNE = 2;
        constexpr unsigned char DCO1_LFO = 3;
        constexpr unsigned char DCO1_ENV = 4;
        
        // DCO-2 (5-11)
        constexpr unsigned char DCO2_RANGE = 5;
        constexpr unsigned char DCO2_WAVEFORM = 6;
        constexpr unsigned char DCO2_CROSSMOD = 7;
        constexpr unsigned char DCO2_TUNE = 8;
        constexpr unsigned char DCO2_FINE_TUNE = 9;
        constexpr unsigned char DCO2_LFO = 10;
        constexpr unsigned char DCO2_ENV = 11;
        
        // Dynamics (12-13)
        constexpr unsigned char DCO_DYNAMICS = 12;
        constexpr unsigned char DCO_ENV_MODE = 13;
        
        // Mixer (14-18)
        constexpr unsigned char MIX_DCO1 = 14;
        constexpr unsigned char MIX_DCO2 = 15;
        constexpr unsigned char MIX_ENV_DEPTH = 16;
        constexpr unsigned char MIX_DYNAMICS = 17;
        constexpr unsigned char MIX_ENV_MODE = 18;
        
        // VCF (19-26)
        constexpr unsigned char VCF_HPF = 19;
        constexpr unsigned char VCF_CUTOFF = 20;
        constexpr unsigned char VCF_RESONANCE = 21;
        constexpr unsigned char VCF_LFO = 22;
        constexpr unsigned char VCF_ENV = 23;
        constexpr unsigned char VCF_KEY_FOLLOW = 24;
        constexpr unsigned char VCF_DYNAMICS = 25;
        constexpr unsigned char VCF_ENV_MODE = 26;
        
        // VCA (27-29)
        constexpr unsigned char VCA_LEVEL = 27;
        constexpr unsigned char VCA_DYNAMICS = 28;
        constexpr unsigned char CHORUS = 29;
        
        // LFO (30-32)
        constexpr unsigned char LFO_WAVEFORM = 30;
        constexpr unsigned char LFO_DELAY = 31;
        constexpr unsigned char LFO_RATE = 32;
        
        // ENV-1 (33-40)
        constexpr unsigned char ENV1_ATTACK = 33;
        constexpr unsigned char ENV1_DECAY = 34;
        constexpr unsigned char ENV1_SUSTAIN = 35;
        constexpr unsigned char ENV1_RELEASE = 36;
        constexpr unsigned char ENV1_KEY_FOLLOW = 40;
        
        // ENV-2 (41-48)
        constexpr unsigned char ENV2_ATTACK = 41;
        constexpr unsigned char ENV2_DECAY = 42;
        constexpr unsigned char ENV2_SUSTAIN = 43;
        constexpr unsigned char ENV2_RELEASE = 44;
        constexpr unsigned char ENV2_KEY_FOLLOW = 48;
        
        // VCA Mode (49)
        constexpr unsigned char VCA_ENV_MODE = 49;
        
        // Aliases for T/L naming
        constexpr unsigned char ENV1_T1 = ENV1_ATTACK;
        constexpr unsigned char ENV1_L1 = ENV1_DECAY;
        constexpr unsigned char ENV1_T2 = ENV1_SUSTAIN;
        constexpr unsigned char ENV1_L2 = ENV1_RELEASE;
        constexpr unsigned char ENV2_T1 = ENV2_ATTACK;
        constexpr unsigned char ENV2_L1 = ENV2_DECAY;
        constexpr unsigned char ENV2_T2 = ENV2_SUSTAIN;
        constexpr unsigned char ENV2_L2 = ENV2_RELEASE;
    }
    
    // ============================================================================
    // LEGACY SYSEX ALIASES (for backward compatibility with existing code)
    // All mapped to V3 namespace addresses
    // ============================================================================
    
    // DCO-1 Parameters (11-15)
    constexpr unsigned char DCO1_RANGE_SYSEX = V3::DCO1_RANGE;           // 11
    constexpr unsigned char DCO1_WAVEFORM_SYSEX = V3::DCO1_WAVEFORM;     // 12
    constexpr unsigned char DCO1_TUNE_SYSEX = V3::DCO1_TUNE;             // 13
    constexpr unsigned char DCO1_LFO_DEPTH_SYSEX = V3::DCO1_LFO_DEPTH;   // 14
    constexpr unsigned char DCO1_ENV_DEPTH_SYSEX = V3::DCO1_ENV_DEPTH;   // 15
    constexpr unsigned char DCO1_DYNAMICS_SYSEX = V3::DCO_DYNAMICS;      // 31
    constexpr unsigned char DCO1_ENV_SOURCE_SYSEX = V3::DCO_ENV_MODE;
    
    // DCO-2 Parameters
    constexpr unsigned char DCO2_RANGE_SYSEX = V3::DCO2_RANGE;
    constexpr unsigned char DCO2_WAVEFORM_SYSEX = V3::DCO2_WAVEFORM;
    constexpr unsigned char DCO2_TUNE_SYSEX = V3::DCO2_TUNE;
    constexpr unsigned char DCO2_FINETUNE_SYSEX = V3::DCO2_FINE_TUNE;
    constexpr unsigned char DCO2_LFO_DEPTH_SYSEX = V3::DCO2_LFO_DEPTH;
    constexpr unsigned char DCO2_ENV_DEPTH_SYSEX = V3::DCO2_ENV_DEPTH;
    constexpr unsigned char DCO_CROSSMOD_SYSEX = V3::DCO2_CROSSMOD;
    
    // Mixer
    constexpr unsigned char MIXER_DCO1_LEVEL_SYSEX = V3::MIX_DCO1;
    constexpr unsigned char MIXER_DCO2_LEVEL_SYSEX = V3::MIX_DCO2;
    constexpr unsigned char MIXER_ENV_DEPTH_SYSEX = V3::MIX_ENV_DEPTH;
    constexpr unsigned char MIXER_DYNAMICS_SYSEX = V3::MIX_DYNAMICS;
    constexpr unsigned char MIXER_ENV_SOURCE_SYSEX = V3::MIX_ENV_MODE;
    
    // HPF & Chorus
    constexpr unsigned char HPF_CUTOFF_SYSEX = V3::VCF_HPF;
    constexpr unsigned char CHORUS_SYSEX = V3::CHORUS;
    
    // VCF
    constexpr unsigned char VCF_CUTOFF_SYSEX = V3::VCF_CUTOFF;       // 34 - VCF Frequency
    constexpr unsigned char VCF_RESONANCE_SYSEX = V3::VCF_RESONANCE;
    constexpr unsigned char VCF_LFO1_DEPTH_SYSEX = V3::VCF_LFO_DEPTH;
    constexpr unsigned char VCF_ENV_DEPTH_SYSEX = V3::VCF_ENV_DEPTH;
    constexpr unsigned char VCF_KEY_FOLLOW_SYSEX = V3::VCF_KEY_FOLLOW;
    constexpr unsigned char VCF_DYNAMICS_SYSEX = V3::VCF_DYNAMICS;
    constexpr unsigned char VCF_ENV_SOURCE_SYSEX = V3::VCF_ENV_MODE;
    
    // ============================================================================
    // V4.x-ONLY PARAMETERS MARKER
    // Set to 0xFF to indicate "not available" in V3.x firmware
    // ============================================================================
    constexpr unsigned char PARAM_NOT_AVAILABLE = 0xFF;
    
    // VCA
    constexpr unsigned char VCA_LEVEL_SYSEX = V3::VCA_LEVEL;           // 41
    constexpr unsigned char VCA_ENV_SOURCE_SYSEX = V3::VCA_ENV_MODE;       // 58 - VCA Mode (0=Gate, ~100=Env2)
    constexpr unsigned char VCA_DYNAMICS_SYSEX = V3::VCA_DYNAMICS;         // 42
    
    // LFO 1
    constexpr unsigned char LFO1_WAVEFORM_SYSEX = V3::LFO_WAVEFORM;
    constexpr unsigned char LFO1_DELAY_SYSEX = V3::LFO_DELAY;
    constexpr unsigned char LFO1_RATE_SYSEX = V3::LFO_RATE;
    
    // ENV 1 (ADSR - V3.x firmware uses ADSR, not 7-stage)
    // Map legacy 7-stage names to ADSR equivalents for compatibility
    constexpr unsigned char ENV1_TIME1_SYSEX = V3::ENV1_ATTACK;      // Attack
    constexpr unsigned char ENV1_LEVEL1_SYSEX = PARAM_NOT_AVAILABLE; // Not used in ADSR
    constexpr unsigned char ENV1_TIME2_SYSEX = V3::ENV1_DECAY;       // Decay
    constexpr unsigned char ENV1_LEVEL2_SYSEX = PARAM_NOT_AVAILABLE; // Not used in ADSR
    constexpr unsigned char ENV1_TIME3_SYSEX = PARAM_NOT_AVAILABLE;  // Not used in ADSR
    constexpr unsigned char ENV1_LEVEL3_SYSEX = V3::ENV1_SUSTAIN;    // Sustain
    constexpr unsigned char ENV1_TIME4_SYSEX = V3::ENV1_RELEASE;     // Release
    constexpr unsigned char ENV1_KEYFOLLOW_LOOP_SYSEX = V3::ENV1_KEY_FOLLOW;
    
    // ENV 2 (ADSR - V3.x firmware uses ADSR, not 7-stage)
    constexpr unsigned char ENV2_TIME1_SYSEX = V3::ENV2_ATTACK;      // Attack
    constexpr unsigned char ENV2_LEVEL1_SYSEX = PARAM_NOT_AVAILABLE; // Not used in ADSR
    constexpr unsigned char ENV2_TIME2_SYSEX = V3::ENV2_DECAY;       // Decay
    constexpr unsigned char ENV2_LEVEL2_SYSEX = PARAM_NOT_AVAILABLE; // Not used in ADSR
    constexpr unsigned char ENV2_TIME3_SYSEX = PARAM_NOT_AVAILABLE;  // Not used in ADSR
    constexpr unsigned char ENV2_LEVEL3_SYSEX = V3::ENV2_SUSTAIN;    // Sustain
    constexpr unsigned char ENV2_TIME4_SYSEX = V3::ENV2_RELEASE;     // Release
    constexpr unsigned char ENV2_KEYFOLLOW_LOOP_SYSEX = V3::ENV2_KEY_FOLLOW;
    
    // ============================================================================
    // V4.x-ONLY PARAMETERS (PWM, LFO2, ENV3, ENV4)
    // These do NOT exist in V3.x firmware - they will be ignored
    // ============================================================================
    
    // PWM DCO-1 (V4.x only - requires PWM kit hardware)
    constexpr unsigned char PWM_DCO1_WIDTH_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO1_ENV_DEPTH_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO1_LFO_DEPTH_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO1_LFO_SOURCE_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO1_DYNAMICS_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO1_ENV_SOURCE_SYSEX = PARAM_NOT_AVAILABLE;
    
    // PWM DCO-2 (V4.x only)
    constexpr unsigned char PWM_DCO2_WIDTH_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO2_ENV_DEPTH_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO2_LFO_DEPTH_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO2_LFO_SOURCE_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO2_DYNAMICS_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char PWM_DCO2_ENV_SOURCE_SYSEX = PARAM_NOT_AVAILABLE;
    
    // LFO 2 (V4.x only)
    constexpr unsigned char LFO2_WAVEFORM_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char LFO2_DELAY_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char LFO2_RATE_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char LFO1_LFO2_DEPTH_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char LFO1_SYNC_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char LFO2_LFO1_DEPTH_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char LFO2_SYNC_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char VCF_LFO2_DEPTH_SYSEX = PARAM_NOT_AVAILABLE;
    
    // ENV 3 (V4.x only - ADSR)
    constexpr unsigned char ENV3_ATTACK_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char ENV3_DECAY_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char ENV3_SUSTAIN_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char ENV3_RELEASE_SYSEX = PARAM_NOT_AVAILABLE;
    
    // ENV 4 (V4.x only - ADSR)
    constexpr unsigned char ENV4_ATTACK_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char ENV4_DECAY_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char ENV4_SUSTAIN_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char ENV4_RELEASE_SYSEX = PARAM_NOT_AVAILABLE;
    
    // IPR/APR Command IDs
    constexpr unsigned char CMD_IPR = 0x36;                // Individual Parameter Request/Send
    constexpr unsigned char CMD_APR = 0x35;                // All Parameters Request/Send  
    constexpr unsigned char CMD_PGR = 0x34;                // Program Change Request
    constexpr unsigned char LEVEL_PATCH = 0x30;            // Patch level
    constexpr unsigned char LEVEL_TONE = 0x20;             // Tone level
    constexpr unsigned char GROUP_PATCH = 0x01;            // Patch group
    
    // CC Numbers
    constexpr unsigned char CC_BANK_SELECT_MSB = 0;
    constexpr unsigned char CC_BANK_SELECT_LSB = 32;
    constexpr unsigned char CC_DATA_ENTRY_MSB = 6;
    constexpr unsigned char CC_DATA_ENTRY_LSB = 38;
    constexpr unsigned char CC_NRPN_MSB = 99;
    constexpr unsigned char CC_NRPN_LSB = 98;
    
    // ============================================================================
    // ADDITIONAL ALIASES FOR BACKWARD COMPATIBILITY
    // ============================================================================
    
    // DCO2 has its own Dynamics/EnvMode in V3 (same as DCO1)
    constexpr unsigned char DCO2_DYNAMICS_SYSEX = V3::DCO_DYNAMICS;
    constexpr unsigned char DCO2_ENV_SOURCE_SYSEX = V3::DCO_ENV_MODE;
    
    // DCO LFO Source (not used in V3.x - set to NOT_AVAILABLE)
    constexpr unsigned char DCO1_LFO_SOURCE_SYSEX = PARAM_NOT_AVAILABLE;
    constexpr unsigned char DCO2_LFO_SOURCE_SYSEX = PARAM_NOT_AVAILABLE;
    
    // Global/System Parameters (Patch Level)
    constexpr unsigned char GLOBAL_UL_BALANCE_SYSEX = 0x12;
    constexpr unsigned char GLOBAL_UPPER_SPLIT_SYSEX = 0x19;
    constexpr unsigned char GLOBAL_LOWER_SPLIT_SYSEX = 0x1A;
    constexpr unsigned char GLOBAL_TOTAL_VOLUME_SYSEX = 0x1C;
    
    // Keyboard/Performance Parameters
    constexpr unsigned char PARAM_KEY_MODE = 0x12;         // Key Mode: 0=Whole, 1=Dual, 2=Split, 3=Whole/Split
    constexpr unsigned char PARAM_SPLIT_POINT = 0x13;      // Split point (MIDI note 0-127)
    
    // MIDI Menu Parameters (Vecoven firmware)
    // SysEx format: F0 41 36 [CH] 24 30 01 [PARAM] [VALUE] F7 (IPR = Individual Parameter)
    constexpr unsigned char PARAM_CTRL_CHANNEL = 0x00;     // Control channel 1-16 (value 0-15)
    constexpr unsigned char PARAM_UPPER_CHANNEL = 0x01;    // Upper tone channel 1-16
    constexpr unsigned char PARAM_LOWER_CHANNEL = 0x02;    // Lower tone channel 1-16  
    constexpr unsigned char PARAM_SYSEX_MODE = 0x03;       // SysEx: OFF=0, RECV=1, SEND=2, ON=3
    constexpr unsigned char PARAM_SYSEX_IPR = 0x04;        // IPR mode
    constexpr unsigned char PARAM_SYSEX_APR = 0x05;        // APR mode
    constexpr unsigned char PARAM_PATCH_PROG_CHG = 0x06;   // Patch prog change: OFF=0, ON=1
    constexpr unsigned char PARAM_UPPER_PROG_CHG = 0x07;   // Upper prog change
    constexpr unsigned char PARAM_LOWER_PROG_CHG = 0x08;   // Lower prog change
    constexpr unsigned char PARAM_MIDI_CC = 0x09;          // MIDI CC: OFF=0, RECV=1, SEND=2, ON=3
    constexpr unsigned char PARAM_NOTE_MSG = 0x0A;         // Note message: ALL=0, ODD=1, EVEN=2
    constexpr unsigned char PARAM_PG800 = 0x0B;            // PG-800: OFF=0, ON=1
    constexpr unsigned char PARAM_EDIT_DISPLAY = 0x0C;     // Edit display: OFF=0, SYSEX=1, PG800=2, CC=3, ALL=4
    constexpr unsigned char PARAM_COMPAT_MODE = 0x0D;      // Compat mode: OFF=0, ON=1
    constexpr unsigned char PARAM_GU280 = 0x0E;            // GU-280 display: OFF=0, ON=1
    
    // Arpeggiator Parameters (Vecoven V4.x - may not work on V3.x)
    constexpr unsigned char ARP_BPM = 0x10;                // BPM 40-300
    constexpr unsigned char ARP_MODE = 0x11;               // UP=0, DOWN=1, U-D=2, PLAY=3, RND=4
    constexpr unsigned char ARP_OCTAVE = 0x12;             // Octave span 1-4
    constexpr unsigned char ARP_RATE = 0x13;               // Rate: 1/4=0, 1/8=1, 1/16=2
    constexpr unsigned char ARP_LATCH = 0x14;              // Latch: OFF=0, ON=1
    constexpr unsigned char ARP_PATCH = 0x15;              // ARP patch 1-8 (value 0-7)
    
    // ============================================================================
    // VALUE MAPPING FUNCTIONS - Convert UI values to MKS-70 SysEx values
    // ============================================================================
    
    // DCO Range: UI 0-3 -> MKS-70 values
    // 0=16' (0), 1=8' (33), 2=4' (70), 3=2' (102)
    inline int mapRangeValue(int uiValue) {
        constexpr int rangeValues[] = { 0, 33, 70, 102 };
        if (uiValue >= 0 && uiValue < 4) return rangeValues[uiValue];
        return 33;  // Default to 8'
    }
    
    // DCO Waveform: UI 0-3 -> MKS-70 values
    // 0=Noise, 36=Square, 70=Pulse, 127=Saw
    inline int mapWaveformValue(int uiValue) {
        constexpr int waveformValues[] = { 0, 127, 36, 70 };  // Noise, Saw, Square, Pulse
        if (uiValue >= 0 && uiValue < 4) return waveformValues[uiValue];
        return 127;  // Default to Saw
    }
    
    // DCO2 Cross Mod: UI 0-3 -> MKS-70 values
    // 0=Off, 33=Sync1, 70=Sync2, 102=XMod
    inline int mapCrossModValue(int uiValue) {
        constexpr int crossmodValues[] = { 0, 33, 70, 102 };
        if (uiValue >= 0 && uiValue < 4) return crossmodValues[uiValue];
        return 0;  // Default to Off
    }
    
    // Dynamics: UI 0-3 -> MKS-70 values
    // 0=Off(0), 1=Dyna1(40), 2=Dyna2(76), 3=Dyna3(104)
    inline int mapDynamicsValue(int uiValue) {
        constexpr int dynamicsValues[] = { 0, 40, 76, 104 };
        if (uiValue >= 0 && uiValue < 4) return dynamicsValues[uiValue];
        return 0;  // Default to Off
    }
    
    // Env Mode/Source: UI 0-3 -> MKS-70 values
    // 0=E1(0), 1=E2(127), 2=E1+E2(64), 3=E1xE2(32)
    inline int mapEnvModeValue(int uiValue) {
        constexpr int envModeValues[] = { 0, 127, 64, 32 };
        if (uiValue >= 0 && uiValue < 4) return envModeValues[uiValue];
        return 0;  // Default to E1
    }
    
    // LFO Waveform: UI 0-3 -> MKS-70 values
    // 0=Random(0), 1=Saw(43), 2=Square(85), 3=Sine(127)
    inline int mapLfoWaveformValue(int uiValue) {
        constexpr int lfoWaveValues[] = { 0, 43, 85, 127 };  // Random, Saw, Square, Sine
        if (uiValue >= 0 && uiValue < 4) return lfoWaveValues[uiValue];
        return 127;  // Default to Sine
    }
    
    // Chorus Type: UI 0-2 -> MKS-70 values
    // 0=Off(0), 1=Type I(64), 2=Type II(127)
    inline int mapChorusValue(int uiValue) {
        constexpr int chorusValues[] = { 0, 64, 127 };
        if (uiValue >= 0 && uiValue < 3) return chorusValues[uiValue];
        return 0;  // Default to Off
    }
    
    // VCA Env Mode: UI 0-1 -> MKS-70 values
    // 0=Gate(0), 1=Env2(~100)
    inline int mapVcaEnvModeValue(int uiValue) {
        return uiValue == 0 ? 0 : 100;
    }
    
    // HPF: UI 0-3 -> MKS-70 values
    // 0=Off(0), 1=Low(43), 2=Mid(85), 3=High(127)
    inline int mapHpfValue(int uiValue) {
        constexpr int hpfValues[] = { 0, 43, 85, 127 };
        if (uiValue >= 0 && uiValue < 4) return hpfValues[uiValue];
        return 0;  // Default to Off
    }
    
    // Key Follow: UI 0-3 -> MKS-70 values (for VCF/ENV key follow)
    inline int mapKeyFollowValue(int uiValue) {
        constexpr int kfValues[] = { 0, 43, 85, 127 };
        if (uiValue >= 0 && uiValue < 4) return kfValues[uiValue];
        return 0;
    }

    inline int clampMidiValue(int value) {
        if (value < 0) return 0;
        if (value > 127) return 127;
        return value;
    }

    inline int mapToneValueForSysex(int sysexAddress, int uiValue) {
        switch (sysexAddress) {
            case DCO1_RANGE_SYSEX:
            case DCO2_RANGE_SYSEX:
                return mapRangeValue(uiValue);

            case DCO1_WAVEFORM_SYSEX:
            case DCO2_WAVEFORM_SYSEX:
                return mapWaveformValue(uiValue);

            case DCO_CROSSMOD_SYSEX:
                return mapCrossModValue(uiValue);

            case DCO1_DYNAMICS_SYSEX:
            case MIXER_DYNAMICS_SYSEX:
            case VCF_DYNAMICS_SYSEX:
            case VCA_DYNAMICS_SYSEX:
                return mapDynamicsValue(uiValue);

            case DCO1_ENV_SOURCE_SYSEX:
            case MIXER_ENV_SOURCE_SYSEX:
            case VCF_ENV_SOURCE_SYSEX:
                return mapEnvModeValue(uiValue);

            case VCA_ENV_SOURCE_SYSEX:
                return mapVcaEnvModeValue(uiValue);

            case HPF_CUTOFF_SYSEX:
                return mapHpfValue(uiValue);

            case LFO1_WAVEFORM_SYSEX:
                return mapLfoWaveformValue(uiValue);

            case CHORUS_SYSEX:
                return mapChorusValue(uiValue);

            case ENV1_KEYFOLLOW_LOOP_SYSEX:
            case ENV2_KEYFOLLOW_LOOP_SYSEX:
                return mapKeyFollowValue(uiValue);

            default:
                return clampMidiValue(uiValue);
        }
    }

    inline int nearestMappedIndex(int sysexValue, const int* values, int count) {
        int bestIndex = 0;
        int bestDistance = 128;
        for (int i = 0; i < count; ++i) {
            int distance = sysexValue - values[i];
            if (distance < 0) distance = -distance;
            if (distance < bestDistance) {
                bestDistance = distance;
                bestIndex = i;
            }
        }
        return bestIndex;
    }

    inline int unmapToneValueFromSysex(int sysexAddress, int sysexValue) {
        sysexValue = clampMidiValue(sysexValue);

        switch (sysexAddress) {
            case DCO1_RANGE_SYSEX:
            case DCO2_RANGE_SYSEX: {
                constexpr int values[] = { 0, 33, 70, 102 };
                return nearestMappedIndex(sysexValue, values, 4);
            }

            case DCO1_WAVEFORM_SYSEX:
            case DCO2_WAVEFORM_SYSEX: {
                constexpr int values[] = { 0, 127, 36, 70 };
                return nearestMappedIndex(sysexValue, values, 4);
            }

            case DCO_CROSSMOD_SYSEX: {
                constexpr int values[] = { 0, 33, 70, 102 };
                return nearestMappedIndex(sysexValue, values, 4);
            }

            case DCO1_DYNAMICS_SYSEX:
            case MIXER_DYNAMICS_SYSEX:
            case VCF_DYNAMICS_SYSEX:
            case VCA_DYNAMICS_SYSEX: {
                constexpr int values[] = { 0, 40, 76, 104 };
                return nearestMappedIndex(sysexValue, values, 4);
            }

            case DCO1_ENV_SOURCE_SYSEX:
            case MIXER_ENV_SOURCE_SYSEX:
            case VCF_ENV_SOURCE_SYSEX: {
                constexpr int values[] = { 0, 127, 64, 32 };
                return nearestMappedIndex(sysexValue, values, 4);
            }

            case VCA_ENV_SOURCE_SYSEX:
                return sysexValue < 50 ? 0 : 1;

            case HPF_CUTOFF_SYSEX:
            case ENV1_KEYFOLLOW_LOOP_SYSEX:
            case ENV2_KEYFOLLOW_LOOP_SYSEX: {
                constexpr int values[] = { 0, 43, 85, 127 };
                return nearestMappedIndex(sysexValue, values, 4);
            }

            case LFO1_WAVEFORM_SYSEX: {
                constexpr int values[] = { 0, 43, 85, 127 };
                return nearestMappedIndex(sysexValue, values, 4);
            }

            case CHORUS_SYSEX: {
                constexpr int values[] = { 0, 64, 127 };
                return nearestMappedIndex(sysexValue, values, 3);
            }

            default:
                return sysexValue;
        }
    }
}
