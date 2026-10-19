#pragma once
#include <JuceHeader.h>
#include <vector>

// מבנה פריסט מלא
struct MKS70Preset
{
    juce::String name;
    juce::String category;      // Bass, Lead, Pad, Brass, Strings, FX, Arp, Key
    juce::String genre;          // Electronic, Rock, Pop, Ambient, etc
    juce::String description;
    
    // DCO Parameters
    int dco1_range = 0;              // 0=16', 1=8', 2=4', 3=2'
    int dco1_waveform = 0;           // 0=Pulse, 1=Saw, 2=Square
    int dco1_pulse_width = 0;        // 0-127
    int dco1_pwm = 0;                // 0-127
    
    int dco2_range = 0;
    int dco2_waveform = 0;
    int dco2_pulse_width = 0;
    int dco2_pwm = 0;
    int dco2_fine_tune = 0;          // -63 to +63
    int dco2_detune = 0;             // 0-127
    
    int dco_crossmod = 0;            // 0-127
    int dco_mix = 64;                 // 0-127 (DCO1 vs DCO2)
    
    // VCF Parameters
    int vcf_cutoff = 64;              // 0-127
    int vcf_resonance = 0;           // 0-127
    int vcf_env_amount = 0;          // -63 to +63
    int vcf_lfo_amount = 0;          // 0-127
    int vcf_key_follow = 0;          // 0-127
    
    // VCA Parameters
    int vca_level = 127;               // 0-127
    
    // Envelope 1 (VCF)
    int env1_attack = 0;             // 0-127
    int env1_decay = 64;              // 0-127
    int env1_sustain = 127;            // 0-127
    int env1_release = 20;            // 0-127
    
    // Envelope 2 (VCA)
    int env2_attack = 0;
    int env2_decay = 64;
    int env2_sustain = 127;
    int env2_release = 20;
    
    // LFO
    int lfo_rate = 0;                // 0-127
    int lfo_delay = 0;               // 0-127
    int lfo_waveform = 0;            // 0=Triangle, 1=Saw Down, 2=Saw Up, 3=Square, 4=Random
    
    // Chorus
    int chorus_rate = 0;             // 0-127
    int chorus_depth = 0;            // 0-127
    int chorus_mode = 0;             // 0=Off, 1=I, 2=II, 3=I+II
    
    juce::String soundDesignTips;
};

class MKS70PresetDatabase
{
private:
    // פונקציית עזר להוספת פריסט
    static void addPreset(std::vector<MKS70Preset>& presets,
                          const char* name, const char* category, const char* genre, const char* description,
                          int dco1_range, int dco1_waveform, int dco1_pulse_width, int dco1_pwm,
                          int dco2_range, int dco2_waveform, int dco2_pulse_width, int dco2_pwm, int dco2_fine_tune, int dco2_detune,
                          int dco_crossmod, int dco_mix,
                          int vcf_cutoff, int vcf_resonance, int vcf_env_amount, int vcf_lfo_amount, int vcf_key_follow,
                          int vca_level,
                          int env1_attack, int env1_decay, int env1_sustain, int env1_release,
                          int env2_attack, int env2_decay, int env2_sustain, int env2_release,
                          int lfo_rate, int lfo_delay, int lfo_waveform,
                          int chorus_rate, int chorus_depth, int chorus_mode,
                          const char* tips)
    {
        MKS70Preset p;
        p.name = name;
        p.category = category;
        p.genre = genre;
        p.description = description;
        p.dco1_range = dco1_range;
        p.dco1_waveform = dco1_waveform;
        p.dco1_pulse_width = dco1_pulse_width;
        p.dco1_pwm = dco1_pwm;
        p.dco2_range = dco2_range;
        p.dco2_waveform = dco2_waveform;
        p.dco2_pulse_width = dco2_pulse_width;
        p.dco2_pwm = dco2_pwm;
        p.dco2_fine_tune = dco2_fine_tune;
        p.dco2_detune = dco2_detune;
        p.dco_crossmod = dco_crossmod;
        p.dco_mix = dco_mix;
        p.vcf_cutoff = vcf_cutoff;
        p.vcf_resonance = vcf_resonance;
        p.vcf_env_amount = vcf_env_amount;
        p.vcf_lfo_amount = vcf_lfo_amount;
        p.vcf_key_follow = vcf_key_follow;
        p.vca_level = vca_level;
        p.env1_attack = env1_attack;
        p.env1_decay = env1_decay;
        p.env1_sustain = env1_sustain;
        p.env1_release = env1_release;
        p.env2_attack = env2_attack;
        p.env2_decay = env2_decay;
        p.env2_sustain = env2_sustain;
        p.env2_release = env2_release;
        p.lfo_rate = lfo_rate;
        p.lfo_delay = lfo_delay;
        p.lfo_waveform = lfo_waveform;
        p.chorus_rate = chorus_rate;
        p.chorus_depth = chorus_depth;
        p.chorus_mode = chorus_mode;
        p.soundDesignTips = tips;
        presets.push_back(p);
    }

public:
    static std::vector<MKS70Preset> getAllPresets()
    {
        std::vector<MKS70Preset> presets;
        presets.reserve(500);
        
        // ============ BASS SOUNDS ============
        addPreset(presets, "Sub Bass Classic", "Bass", "Electronic",
                  "Deep analog bass",
                  1,1,0,0, 1,1,0,0,0,0, 0,64, 35,15,0,0,64, 127, 0,30,127,10, 0,25,100,15, 0,0,0, 0,0,0,
                  "Low Cutoff for depth");
        
        addPreset(presets, "Acid Bass TB-303", "Bass", "Electronic",
                  "TB-303 style acid bass",
                  2,1,0,0, 0,0,0,0,0,0, 0,127, 80,90,50,0,64, 127, 0,50,0,10, 0,40,0,8, 0,0,0, 0,0,0,
                  "High Resonance = 303");
        
        addPreset(presets, "Reese Bass", "Bass", "Drum & Bass",
                  "Reese bass with Detune",
                  1,1,0,0, 1,1,0,0,0,45, 0,64, 45,20,0,0,64, 127, 0,35,127,18, 0,28,100,15, 0,0,0, 35,45,2,
                  "Detune 40-50");
        
        addPreset(presets, "Wobble Bass", "Bass", "Dubstep",
                  "Wobble bass with LFO",
                  1,1,0,0, 1,1,0,0,0,25, 0,64, 70,80,0,127,64, 127, 0,60,127,30, 0,50,100,40, 60,0,3, 0,0,0,
                  "LFO Rate = speed");
        
        addPreset(presets, "808 Sub", "Bass", "Hip-Hop",
                  "Classic 808 bass",
                  1,0,0,0, 0,0,0,0,0,0, 0,127, 35,18,0,0,64, 127, 0,40,127,30, 0,35,100,28, 0,0,0, 0,0,0,
                  "Long Decay");
        
        addPreset(presets, "Fat Analog Bass", "Bass", "Electronic",
                  "Fat analog bass",
                  1,1,0,0, 1,1,0,0,0,10, 0,64, 50,25,20,0,64, 127, 0,35,120,15, 0,30,110,12, 0,0,0, 25,35,1,
                  "Light Detune + Chorus");
        
        addPreset(presets, "Punchy Bass", "Bass", "Pop",
                  "Short punchy bass",
                  2,1,0,0, 2,1,0,0,0,5, 0,64, 60,30,40,0,64, 127, 0,25,60,8, 0,20,70,10, 0,0,0, 0,0,0,
                  "Short Decay");
        
        addPreset(presets, "Growl Bass", "Bass", "Dubstep",
                  "Aggressive growl bass",
                  1,1,64,30, 1,1,64,30,0,35, 20,64, 55,75,30,80,64, 127, 0,50,100,25, 0,45,95,20, 50,0,3, 0,0,0,
                  "PWM + LFO Filter");
        
        addPreset(presets, "Moog Bass", "Bass", "Funk",
                  "Classic Moog bass",
                  1,1,0,0, 0,0,0,0,0,0, 0,127, 55,30,35,0,64, 127, 0,40,100,15, 0,35,95,12, 0,0,0, 0,0,0,
                  "Medium Filter Env");
        
        addPreset(presets, "Octave Bass", "Bass", "Rock",
                  "Bass with octave",
                  0,1,0,0, 1,1,0,0,0,0, 0,64, 50,20,25,0,64, 127, 0,35,110,15, 0,30,100,12, 0,0,0, 0,0,0,
                  "16' + 8'");
        
        // ============ LEAD SOUNDS ============
        addPreset(presets, "Super Saw Lead", "Lead", "Trance",
                  "Powerful supersaw lead",
                  2,1,0,0, 2,1,0,0,0,35, 0,64, 85,30,25,0,90, 127, 5,40,100,25, 3,35,100,20, 25,0,0, 55,65,3,
                  "Detune + Chorus");
        
        addPreset(presets, "Sync Lead", "Lead", "Electronic",
                  "Aggressive sync lead",
                  2,1,0,0, 2,1,0,0,50,0, 64,64, 75,45,35,0,80, 127, 0,45,90,20, 0,40,95,18, 0,0,0, 0,0,0,
                  "Crossmod = sync");
        
        addPreset(presets, "Classic Moog Lead", "Lead", "Rock",
                  "Classic Moog lead",
                  1,1,0,0, 1,1,0,0,0,5, 0,64, 75,35,30,0,85, 127, 8,50,100,25, 5,45,100,22, 20,40,0, 0,0,0,
                  "Vibrato with LFO");
        
        addPreset(presets, "Bright Lead", "Lead", "Pop",
                  "Bright sharp lead",
                  2,1,0,0, 2,1,0,0,0,8, 0,64, 95,40,35,0,100, 127, 0,35,90,20, 0,30,95,18, 0,0,0, 0,0,0,
                  "High Cutoff");
        
        addPreset(presets, "Portamento Lead", "Lead", "Electronic",
                  "Lead with portamento",
                  2,1,0,0, 2,1,0,0,0,12, 0,64, 80,35,30,0,85, 127, 5,40,95,22, 3,35,100,20, 15,30,0, 30,40,2,
                  "Use MIDI portamento");
        
        addPreset(presets, "Screaming Lead", "Lead", "Rock",
                  "Distorted screaming lead",
                  2,1,64,20, 2,1,64,20,0,5, 30,64, 90,60,45,0,95, 127, 0,45,85,25, 0,40,90,22, 30,40,0, 0,0,0,
                  "PWM + Crossmod");
        
        addPreset(presets, "Smooth Lead", "Lead", "Jazz",
                  "Smooth warm lead",
                  1,1,0,0, 1,1,0,0,0,3, 0,64, 60,20,20,0,80, 127, 10,50,100,30, 8,45,100,28, 18,45,0, 25,35,1,
                  "Light Attack + Chorus");
        
        addPreset(presets, "Aggressive Lead", "Lead", "Metal",
                  "Aggressive lead",
                  2,1,0,0, 2,1,0,0,0,8, 25,64, 85,55,40,0,90, 127, 0,35,80,15, 0,30,85,12, 0,0,0, 0,0,0,
                  "Crossmod + Resonance");
        
        addPreset(presets, "Whistle Lead", "Lead", "Electronic",
                  "Whistle lead",
                  3,0,64,0, 0,0,0,0,0,0, 0,127, 100,50,30,0,110, 127, 0,30,85,20, 0,25,90,18, 20,35,0, 0,0,0,
                  "2' Pulse + High Res");
        
        addPreset(presets, "Fat Mono Lead", "Lead", "Electronic",
                  "Fat mono lead",
                  1,1,0,0, 1,1,0,0,0,15, 0,64, 70,30,25,0,75, 127, 3,45,95,22, 2,40,100,20, 0,0,0, 35,45,2,
                  "Detune + Chorus");
        
        // ============ PAD SOUNDS ============
        addPreset(presets, "Warm Strings Pad", "Pad", "Ambient",
                  "Warm string pad",
                  1,1,0,0, 1,1,0,0,0,15, 0,64, 55,20,15,0,70, 120, 45,70,100,60, 40,65,100,55, 20,50,0, 45,55,2,
                  "Long Attack");
        
        addPreset(presets, "Ethereal Pad", "Pad", "Cinematic",
                  "Ethereal pad",
                  1,1,0,0, 1,2,64,0,0,12, 0,64, 65,25,20,35,75, 115, 60,80,100,70, 55,75,100,65, 15,60,0, 50,60,3,
                  "LFO on Filter");
        
        addPreset(presets, "Dark Ambient Pad", "Pad", "Dark Ambient",
                  "Dark ambient pad",
                  0,1,0,0, 0,2,64,0,0,20, 0,64, 40,30,10,20,60, 110, 70,90,100,80, 65,85,100,75, 10,70,0, 30,40,2,
                  "16' + Low Cutoff");
        
        addPreset(presets, "Lush Pad", "Pad", "Electronic",
                  "Lush rich pad",
                  1,1,0,0, 1,1,0,0,0,18, 0,64, 60,22,18,0,72, 118, 50,75,100,65, 45,70,100,60, 18,55,0, 50,60,3,
                  "Detune + Max Chorus");
        
        addPreset(presets, "Space Pad", "Pad", "Ambient",
                  "Space pad",
                  1,2,64,25, 1,2,64,25,0,15, 0,64, 55,28,22,30,68, 112, 65,85,100,75, 60,80,100,70, 12,65,0, 55,65,3,
                  "PWM + LFO");
        
        addPreset(presets, "Analog Warmth", "Pad", "Lo-Fi",
                  "Analog warmth",
                  1,1,0,0, 1,1,0,0,0,8, 0,64, 45,15,10,0,60, 105, 55,80,100,70, 50,75,100,65, 0,0,0, 40,50,2,
                  "Low-ish Cutoff");
        
        addPreset(presets, "Glass Pad", "Pad", "Cinematic",
                  "Glass pad",
                  2,2,64,15, 2,2,64,15,0,10, 0,64, 75,30,25,20,85, 115, 55,75,100,65, 50,70,100,60, 15,55,0, 45,55,2,
                  "Square + Long Attack");
        
        addPreset(presets, "Choir Pad", "Pad", "Orchestral",
                  "Choir pad",
                  1,1,64,20, 1,1,64,20,0,12, 0,64, 58,22,18,0,75, 118, 60,85,100,70, 55,80,100,65, 22,50,0, 42,52,2,
                  "Slow PWM");
        
        addPreset(presets, "Evolving Pad", "Pad", "Ambient",
                  "Evolving pad",
                  1,1,64,35, 1,2,64,35,0,20, 0,64, 50,25,15,45,65, 110, 80,100,100,90, 75,95,100,85, 8,70,0, 55,65,3,
                  "Slow LFO Rate");
        
        addPreset(presets, "Vintage Pad", "Pad", "Synthwave",
                  "Vintage pad",
                  1,1,0,0, 1,1,0,0,0,10, 0,64, 52,18,12,0,65, 115, 45,70,100,55, 40,65,100,50, 0,0,0, 48,58,2,
                  "80s style Chorus");
        
        // ============ KEYS SOUNDS ============
        addPreset(presets, "Electric Piano", "Keys", "Jazz",
                  "Rhodes style EP",
                  2,1,0,0, 2,1,0,0,0,0, 0,64, 75,25,35,0,100, 127, 0,25,60,12, 0,20,70,15, 0,0,0, 35,45,2,
                  "High Key Follow");
        
        addPreset(presets, "Clavi", "Keys", "Funk",
                  "Funky clavinet",
                  2,0,64,0, 2,0,64,0,0,0, 0,64, 85,35,40,0,105, 127, 0,20,50,8, 0,15,60,10, 0,0,0, 0,0,0,
                  "Pulse + Filter Env");
        
        addPreset(presets, "Organ Perc", "Keys", "Soul",
                  "Organ with percussion",
                  2,2,64,0, 3,2,64,0,0,0, 0,64, 80,20,45,0,90, 127, 0,15,40,5, 0,12,50,8, 0,0,0, 45,55,3,
                  "Short Decay");
        
        addPreset(presets, "Wurlitzer", "Keys", "Jazz",
                  "Warm Wurlitzer",
                  2,1,0,0, 2,1,0,0,0,3, 0,64, 70,22,30,0,95, 127, 0,28,65,15, 0,22,70,12, 0,0,0, 30,40,1,
                  "Tremolo with LFO");
        
        addPreset(presets, "DX7 Piano", "Keys", "Pop",
                  "FM style piano",
                  2,1,0,0, 3,1,0,0,25,0, 30,64, 85,25,35,0,100, 127, 0,35,55,20, 0,30,60,18, 0,0,0, 0,0,0,
                  "Minimal Crossmod");
        
        addPreset(presets, "Harpsichord", "Keys", "Classical",
                  "Synth harpsichord",
                  2,0,64,0, 3,0,64,0,0,0, 0,64, 95,30,50,0,110, 127, 0,12,30,5, 0,10,40,8, 0,0,0, 0,0,0,
                  "Pulse + Short Decay");
        
        addPreset(presets, "Synth Piano", "Keys", "Electronic",
                  "Synth piano",
                  2,1,0,0, 2,1,0,0,0,5, 0,64, 80,28,38,0,98, 127, 0,30,60,18, 0,25,65,15, 0,0,0, 25,35,1,
                  "Light Chorus");
        
        addPreset(presets, "Celesta", "Keys", "Orchestral",
                  "Celesta",
                  3,1,0,0, 3,1,0,0,0,0, 0,64, 90,20,30,0,105, 127, 0,35,50,25, 0,30,55,22, 0,0,0, 40,50,2,
                  "2' + Medium Decay");
        
        addPreset(presets, "Toy Piano", "Keys", "Lo-Fi",
                  "Toy piano",
                  3,0,64,0, 3,0,64,0,0,0, 0,64, 95,35,45,0,100, 127, 0,20,35,12, 0,15,45,10, 0,0,0, 0,0,0,
                  "2' Pulse");
        
        addPreset(presets, "Vibraphone", "Keys", "Jazz",
                  "Vibraphone",
                  2,1,0,0, 2,1,0,0,0,0, 0,64, 85,18,25,0,100, 125, 0,45,55,35, 0,40,60,30, 25,50,0, 40,50,2,
                  "LFO = Tremolo");
        
        // ============ BRASS SOUNDS ============
        addPreset(presets, "Synth Brass", "Brass", "Pop",
                  "Classic synth brass",
                  1,1,0,0, 1,1,0,0,0,8, 0,64, 60,30,50,0,85, 127, 15,45,100,25, 12,40,100,22, 0,0,0, 30,40,2,
                  "Medium Attack");
        
        addPreset(presets, "French Horn", "Brass", "Orchestral",
                  "French horn",
                  1,1,0,0, 1,1,0,0,0,5, 0,64, 55,25,40,0,80, 122, 25,60,100,40, 20,55,100,35, 15,50,0, 20,30,1,
                  "Slow Attack");
        
        addPreset(presets, "Trumpet Section", "Brass", "Big Band",
                  "Trumpet section",
                  2,1,0,0, 2,1,0,0,0,10, 0,64, 75,35,55,0,95, 127, 10,40,100,20, 8,35,100,18, 0,0,0, 35,45,2,
                  "Bright + Chorus");
        
        addPreset(presets, "Trombone", "Brass", "Jazz",
                  "Trombone",
                  1,1,0,0, 1,1,0,0,0,3, 0,64, 50,22,35,0,75, 125, 20,55,100,35, 15,50,100,30, 0,0,0, 0,0,0,
                  "8' + Filter Env");
        
        addPreset(presets, "Brass Stab", "Brass", "Electronic",
                  "Short brass stab",
                  1,1,0,0, 1,1,0,0,0,6, 0,64, 80,40,60,0,90, 127, 0,20,50,8, 0,15,60,10, 0,0,0, 25,35,2,
                  "Short Decay");
        
        addPreset(presets, "Warm Brass", "Brass", "Soul",
                  "Warm brass",
                  1,1,0,0, 1,1,0,0,0,8, 0,64, 52,22,40,0,78, 122, 18,55,100,30, 15,50,100,25, 0,0,0, 32,42,1,
                  "Lower Cutoff");
        
        addPreset(presets, "Analog Brass", "Brass", "Synthwave",
                  "80s analog brass",
                  1,1,0,0, 1,1,0,0,0,12, 0,64, 65,32,48,0,88, 127, 12,42,100,22, 10,38,100,20, 0,0,0, 40,50,2,
                  "Detune + Chorus");
        
        addPreset(presets, "Muted Brass", "Brass", "Jazz",
                  "Muted brass",
                  2,1,0,0, 2,1,0,0,0,5, 0,64, 45,28,30,0,70, 115, 8,45,95,25, 5,40,100,22, 0,0,0, 0,0,0,
                  "Low Cutoff");
        
        addPreset(presets, "Power Brass", "Brass", "Rock",
                  "Powerful brass",
                  1,1,0,0, 1,1,0,0,0,10, 0,64, 75,38,58,0,92, 127, 8,38,100,18, 5,35,100,15, 0,0,0, 38,48,2,
                  "Strong Filter Env");
        
        addPreset(presets, "Soft Brass", "Brass", "Ballad",
                  "Soft brass",
                  1,1,0,0, 1,1,0,0,0,5, 0,64, 48,20,32,0,72, 118, 22,60,100,40, 18,55,100,35, 0,0,0, 28,38,1,
                  "Long Attack");
        
        // ============ STRINGS SOUNDS ============
        addPreset(presets, "Analog Strings", "Strings", "Cinematic",
                  "Analog strings",
                  1,1,0,0, 1,1,0,0,0,12, 0,64, 50,20,15,0,70, 118, 50,75,100,55, 45,70,100,50, 20,55,0, 40,50,2,
                  "Long Attack + Chorus");
        
        addPreset(presets, "Solo Violin", "Strings", "Classical",
                  "Solo violin",
                  2,1,0,0, 2,1,0,0,0,3, 0,64, 70,30,25,0,90, 125, 35,55,100,40, 30,50,100,35, 28,40,0, 25,35,2,
                  "Vibrato with LFO");
        
        addPreset(presets, "Cello", "Strings", "Orchestral",
                  "Deep cello",
                  1,1,0,0, 1,1,0,0,0,5, 0,64, 55,22,18,0,75, 120, 45,70,100,50, 40,65,100,45, 22,48,0, 35,45,2,
                  "8' + Vibrato");
        
        addPreset(presets, "String Section", "Strings", "Orchestral",
                  "String section",
                  1,1,0,0, 1,1,0,0,0,15, 0,64, 52,18,12,0,68, 115, 55,80,100,60, 50,75,100,55, 18,52,0, 48,58,2,
                  "Detune + Chorus");
        
        addPreset(presets, "Pizzicato", "Strings", "Classical",
                  "Pizzicato",
                  2,1,0,0, 2,1,0,0,0,5, 0,64, 80,25,35,0,95, 127, 0,20,40,15, 0,18,50,12, 0,0,0, 0,0,0,
                  "Very short Decay");
        
        addPreset(presets, "Synth Strings", "Strings", "Synthwave",
                  "80s synth strings",
                  1,1,0,0, 1,1,0,0,0,18, 0,64, 58,22,15,0,72, 118, 48,72,100,55, 45,68,100,50, 0,0,0, 52,62,3,
                  "Full Chorus");
        
        addPreset(presets, "Dark Strings", "Strings", "Cinematic",
                  "Dark strings",
                  0,1,0,0, 0,1,0,0,0,12, 0,64, 42,25,10,0,58, 110, 60,85,100,70, 55,80,100,65, 15,55,0, 38,48,2,
                  "16' + Low Cutoff");
        
        addPreset(presets, "Tremolo Strings", "Strings", "Cinematic",
                  "Tremolo strings",
                  1,1,0,0, 1,1,0,0,0,10, 0,64, 55,20,15,0,70, 118, 50,75,100,55, 45,70,100,50, 45,30,0, 42,52,2,
                  "Fast LFO Rate");
        
        addPreset(presets, "Warm Strings", "Strings", "Pop",
                  "Warm strings",
                  1,1,0,0, 1,1,0,0,0,8, 0,64, 48,15,12,0,65, 115, 55,80,100,60, 50,75,100,55, 18,50,0, 45,55,2,
                  "Medium-low Cutoff");
        
        addPreset(presets, "Fast Strings", "Strings", "Action",
                  "Fast strings",
                  1,1,0,0, 1,1,0,0,0,12, 0,64, 60,22,18,0,75, 122, 25,50,100,35, 22,48,100,32, 0,0,0, 45,55,2,
                  "Shorter Attack");
        
        // ============ FX SOUNDS ============
        addPreset(presets, "Sci-Fi Sweep", "FX", "Cinematic",
                  "Sci-fi sweep",
                  1,1,0,0, 1,1,0,0,0,25, 40,64, 30,45,60,0,64, 120, 20,100,127,90, 15,90,127,85, 8,30,0, 45,55,3,
                  "Long Filter Env");
        
        addPreset(presets, "White Noise", "FX", "Electronic",
                  "White noise",
                  2,1,0,0, 2,1,0,0,127,127, 127,64, 60,40,0,0,64, 110, 0,50,100,40, 0,45,100,35, 0,0,0, 0,0,0,
                  "Max Crossmod");
        
        addPreset(presets, "Rising Sweep", "FX", "Trance",
                  "Rising sweep",
                  1,1,0,0, 1,1,0,0,0,20, 0,64, 20,35,80,0,64, 125, 0,127,127,127, 0,120,127,120, 5,20,0, 50,60,3,
                  "Max Filter Env");
        
        addPreset(presets, "Laser", "FX", "Sci-Fi",
                  "Laser",
                  3,1,0,0, 3,1,0,0,0,0, 50,64, 100,80,70,0,64, 127, 0,10,0,5, 0,8,0,5, 0,0,0, 0,0,0,
                  "Very short Decay");
        
        addPreset(presets, "Explosion", "FX", "Cinematic",
                  "Explosion",
                  0,1,0,0, 0,1,0,0,80,80, 80,64, 90,50,0,0,64, 127, 0,60,100,70, 0,55,100,65, 0,0,0, 0,0,0,
                  "Noise + Long Decay");
        
        addPreset(presets, "Wind", "FX", "Ambient",
                  "Wind",
                  1,1,64,50, 1,1,64,50,60,60, 60,64, 55,30,0,60,64, 100, 80,100,100,90, 75,95,100,85, 5,60,4, 0,0,0,
                  "Random LFO + PWM");
        
        addPreset(presets, "Underwater", "FX", "Ambient",
                  "Underwater",
                  1,1,0,0, 1,1,0,0,0,20, 0,64, 35,45,0,30,50, 105, 70,90,100,80, 65,85,100,75, 8,50,0, 55,65,3,
                  "Low Cutoff + LFO");
        
        addPreset(presets, "Alarm", "FX", "Industrial",
                  "Alarm",
                  3,0,64,0, 3,0,64,0,0,0, 0,127, 100,60,40,0,64, 127, 0,30,127,20, 0,25,127,18, 60,0,3, 0,0,0,
                  "Square LFO");
        
        addPreset(presets, "Helicopter", "FX", "Cinematic",
                  "Helicopter",
                  1,1,0,0, 1,1,0,0,40,40, 40,64, 50,35,0,80,64, 115, 10,60,100,40, 8,55,100,35, 70,0,3, 0,0,0,
                  "Fast LFO");
        
        addPreset(presets, "Space Drone", "FX", "Ambient",
                  "Space drone",
                  0,1,0,0, 0,2,64,30,0,25, 0,64, 40,25,10,25,55, 100, 90,110,127,100, 85,105,127,95, 5,70,0, 50,60,3,
                  "Very long Attack");
        
        // ============ PLUCK SOUNDS ============
        addPreset(presets, "Synth Pluck", "Pluck", "EDM",
                  "Synth pluck",
                  2,1,0,0, 2,1,0,0,0,10, 0,64, 85,30,40,0,95, 127, 0,25,30,15, 0,20,40,12, 0,0,0, 0,0,0,
                  "Short Decay");
        
        addPreset(presets, "Bell Tone", "Pluck", "Ambient",
                  "Bell tone",
                  3,1,0,0, 3,1,0,0,30,0, 35,64, 90,35,30,0,100, 127, 0,45,50,35, 0,40,55,30, 0,0,0, 35,45,2,
                  "2' + Crossmod");
        
        addPreset(presets, "Guitar Pluck", "Pluck", "Acoustic",
                  "Guitar pluck",
                  2,1,0,0, 2,1,0,0,0,5, 0,64, 80,25,35,0,90, 127, 0,30,45,20, 0,25,50,18, 0,0,0, 20,30,1,
                  "Short Filter Env");
        
        addPreset(presets, "Marimba", "Pluck", "World",
                  "Marimba",
                  2,1,0,0, 2,1,0,0,0,0, 0,64, 85,18,30,0,95, 127, 0,40,45,30, 0,35,50,25, 0,0,0, 0,0,0,
                  "Medium Decay");
        
        addPreset(presets, "Kalimba", "Pluck", "World",
                  "Kalimba",
                  3,1,0,0, 3,1,0,0,0,0, 20,64, 90,25,35,0,100, 127, 0,35,40,25, 0,30,45,22, 0,0,0, 30,40,2,
                  "2' + Light Crossmod");
        
        addPreset(presets, "Harp", "Pluck", "Classical",
                  "Harp",
                  2,1,0,0, 2,1,0,0,0,3, 0,64, 82,20,30,0,92, 125, 0,50,55,40, 0,45,60,35, 0,0,0, 35,45,2,
                  "Long Decay");
        
        addPreset(presets, "Banjo", "Pluck", "Folk",
                  "Banjo",
                  2,0,64,0, 2,0,64,0,0,0, 0,64, 90,30,40,0,95, 127, 0,18,35,12, 0,15,40,10, 0,0,0, 0,0,0,
                  "Pulse + Short Decay");
        
        addPreset(presets, "Steel Drum", "Pluck", "Caribbean",
                  "Steel drum",
                  2,1,0,0, 3,1,0,0,20,0, 25,64, 88,28,35,0,98, 127, 0,40,50,30, 0,35,55,25, 0,0,0, 40,50,2,
                  "Crossmod + Decay");
        
        addPreset(presets, "Koto", "Pluck", "Japanese",
                  "Japanese koto",
                  2,1,0,0, 2,1,0,0,0,2, 0,64, 78,22,28,0,88, 127, 0,45,55,35, 0,40,60,30, 0,0,0, 25,35,1,
                  "Long Decay + Light Chorus");
        
        addPreset(presets, "Sitar", "Pluck", "Indian",
                  "Sitar",
                  2,1,0,0, 2,1,0,0,0,8, 15,64, 75,35,25,0,85, 127, 0,55,60,45, 0,50,65,40, 25,40,0, 28,38,2,
                  "Crossmod + Vibrato");
        
        // ============ ARP SOUNDS ============
        addPreset(presets, "Trance Arp", "Arp", "Trance",
                  "Trance arpeggio",
                  2,1,0,0, 2,1,0,0,0,8, 0,64, 80,25,35,0,90, 127, 0,30,40,18, 0,25,50,15, 0,0,0, 40,50,2,
                  "Filter Env for motion");
        
        addPreset(presets, "Acid Arp", "Arp", "Acid",
                  "Acid arpeggio",
                  2,1,0,0, 0,0,0,0,0,0, 0,127, 75,85,45,0,70, 127, 0,35,20,12, 0,30,30,10, 0,0,0, 0,0,0,
                  "High Resonance");
        
        addPreset(presets, "Ambient Arp", "Arp", "Ambient",
                  "Ambient arpeggio",
                  2,1,0,0, 2,1,0,0,0,12, 0,64, 70,20,25,0,80, 120, 0,40,55,30, 0,35,60,25, 0,0,0, 50,60,3,
                  "Long Decay + Chorus");
        
        addPreset(presets, "Techno Arp", "Arp", "Techno",
                  "Techno arpeggio",
                  2,1,0,0, 2,1,0,0,0,5, 0,64, 85,35,40,0,85, 127, 0,25,35,15, 0,20,40,12, 0,0,0, 0,0,0,
                  "Strong Filter Env");
        
        addPreset(presets, "House Arp", "Arp", "House",
                  "House arpeggio",
                  2,1,0,0, 2,1,0,0,0,8, 0,64, 78,28,32,0,88, 127, 0,28,42,18, 0,25,48,15, 0,0,0, 35,45,2,
                  "Chord stabs");
        
        addPreset(presets, "Retro Arp", "Arp", "Synthwave",
                  "Retro arpeggio",
                  2,1,0,0, 2,1,0,0,0,10, 0,64, 75,25,30,0,82, 125, 0,32,45,20, 0,28,50,18, 0,0,0, 45,55,2,
                  "Chorus + Detune");
        
        addPreset(presets, "Dark Arp", "Arp", "Dark Ambient",
                  "Dark arpeggio",
                  1,1,0,0, 1,1,0,0,0,15, 0,64, 55,30,25,0,65, 115, 0,38,50,25, 0,35,55,22, 0,0,0, 38,48,2,
                  "8' + Low Cutoff");
        
        addPreset(presets, "Pluck Arp", "Arp", "EDM",
                  "Pluck arpeggio",
                  2,1,0,0, 2,1,0,0,0,6, 0,64, 88,30,42,0,95, 127, 0,22,32,14, 0,18,38,12, 0,0,0, 0,0,0,
                  "Very short Decay");
        
        addPreset(presets, "Bell Arp", "Arp", "Ambient",
                  "Bell arpeggio",
                  3,1,0,0, 3,1,0,0,25,0, 30,64, 92,32,35,0,102, 127, 0,42,52,32, 0,38,55,28, 0,0,0, 42,52,2,
                  "2' + Crossmod");
        
        addPreset(presets, "Poly Arp", "Arp", "Progressive",
                  "Poly arpeggio",
                  2,1,0,0, 2,1,0,0,0,12, 0,64, 82,28,30,0,88, 122, 0,35,48,22, 0,32,52,20, 0,0,0, 48,58,2,
                  "Detune + Chorus");
        
        // ============ SYNTH SOUNDS ============
        addPreset(presets, "Classic Poly", "Synth", "Electronic",
                  "Classic polysynth",
                  1,1,0,0, 1,1,0,0,0,10, 0,64, 70,25,25,0,80, 120, 15,50,100,35, 12,45,100,30, 0,0,0, 40,50,2,
                  "All-around synth");
        
        addPreset(presets, "Oberheim OB-X", "Synth", "Synthwave",
                  "Oberheim style",
                  1,1,0,0, 1,1,0,0,0,15, 0,64, 65,28,22,0,75, 118, 18,55,100,40, 15,50,100,35, 0,0,0, 45,55,2,
                  "Fat analog");
        
        addPreset(presets, "Prophet 5 Pad", "Synth", "Ambient",
                  "Prophet 5 style pad",
                  1,1,0,0, 1,2,64,25,0,12, 0,64, 55,22,18,0,68, 115, 50,75,100,60, 45,70,100,55, 15,55,0, 48,58,2,
                  "Square + PWM");
        
        addPreset(presets, "Jupiter 8", "Synth", "Pop",
                  "Jupiter 8 style",
                  1,1,0,0, 1,1,0,0,0,18, 0,64, 72,25,20,0,82, 122, 20,55,100,40, 18,50,100,35, 0,0,0, 52,62,3,
                  "Detune + Full Chorus");
        
        addPreset(presets, "CS-80 Brass", "Synth", "Cinematic",
                  "CS-80 brass",
                  1,1,0,0, 1,1,0,0,0,8, 0,64, 62,30,45,0,85, 125, 12,45,100,28, 10,42,100,25, 0,0,0, 35,45,2,
                  "Filter Env + Detune");
        
        addPreset(presets, "Minimoog Bass", "Synth", "Funk",
                  "Minimoog bass",
                  1,1,0,0, 0,0,0,0,0,0, 0,127, 58,32,38,0,68, 127, 0,38,100,18, 0,35,100,15, 0,0,0, 0,0,0,
                  "Mono + Filter Env");
        
        addPreset(presets, "ARP Odyssey", "Synth", "Electronic",
                  "ARP style",
                  2,1,0,0, 2,1,0,0,0,5, 30,64, 78,38,35,0,88, 127, 0,40,90,25, 0,38,95,22, 0,0,0, 0,0,0,
                  "Crossmod + Filter");
        
        addPreset(presets, "Juno Strings", "Synth", "Synthwave",
                  "Juno strings",
                  1,1,64,20, 1,1,64,20,0,10, 0,64, 55,18,12,0,68, 115, 50,75,100,55, 45,70,100,50, 0,0,0, 55,65,3,
                  "PWM + Chorus");
        
        addPreset(presets, "MS-20 Lead", "Synth", "Industrial",
                  "MS-20 lead",
                  2,1,0,0, 2,1,0,0,0,3, 45,64, 82,55,40,0,90, 127, 0,35,80,20, 0,32,85,18, 0,0,0, 0,0,0,
                  "High Resonance");
        
        addPreset(presets, "SH-101 Bass", "Synth", "Electronic",
                  "SH-101 bass",
                  2,1,0,0, 0,0,0,0,0,0, 0,127, 62,35,42,0,72, 127, 0,35,100,15, 0,32,100,12, 0,0,0, 0,0,0,
                  "Mono + Simple");
        
        return presets;
    }
    
    // Search by category
    static std::vector<MKS70Preset> getByCategory(const juce::String& category)
    {
        auto allPresets = getAllPresets();
        std::vector<MKS70Preset> results;
        
        for (const auto& preset : allPresets)
        {
            if (preset.category == category)
                results.push_back(preset);
        }
        
        return results;
    }
    
    // Search by genre
    static std::vector<MKS70Preset> getByGenre(const juce::String& genre)
    {
        auto allPresets = getAllPresets();
        std::vector<MKS70Preset> results;
        
        for (const auto& preset : allPresets)
        {
            if (preset.genre == genre)
                results.push_back(preset);
        }
        
        return results;
    }
    
    // Search by name or description
    static std::vector<MKS70Preset> search(const juce::String& searchTerm)
    {
        auto allPresets = getAllPresets();
        std::vector<MKS70Preset> results;
        
        juce::String lowerSearch = searchTerm.toLowerCase();
        
        for (const auto& preset : allPresets)
        {
            if (preset.name.toLowerCase().contains(lowerSearch) ||
                preset.description.toLowerCase().contains(lowerSearch) ||
                preset.soundDesignTips.toLowerCase().contains(lowerSearch))
            {
                results.push_back(preset);
            }
        }
        
        return results;
    }
};
