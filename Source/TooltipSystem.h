/*
  ==============================================================================

    TooltipSystem.h
    Advanced tooltip system for MKS-70 Editor
    
    Features:
    - Rich tooltips with parameter name, value, range
    - Contextual help descriptions
    - Keyboard shortcut hints
    - Parameter-specific documentation

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
    ParameterTooltipInfo - Stores all tooltip data for a parameter
*/
struct ParameterTooltipInfo
{
    juce::String name;
    juce::String description;
    juce::String section;
    int minValue = 0;
    int maxValue = 127;
    juce::String unit;
    juce::StringArray valueLabels;  // For combo boxes or specific value names
    juce::String shortcut;          // Keyboard shortcut if any
    
    // Generate full tooltip text
    juce::String getTooltipText(int currentValue) const
    {
        juce::String text;
        
        // Name and section
        text << name;
        if (section.isNotEmpty())
            text << " (" << section << ")";
        text << "\n";
        
        // Current value
        if (valueLabels.isEmpty())
        {
            text << "Value: " << currentValue;
            if (unit.isNotEmpty())
                text << " " << unit;
            text << " [" << minValue << "-" << maxValue << "]";
        }
        else if (currentValue >= 0 && currentValue < valueLabels.size())
        {
            text << "Value: " << valueLabels[currentValue];
        }
        text << "\n";
        
        // Description
        if (description.isNotEmpty())
        {
            text << "\n" << description;
        }
        
        // Shortcut
        if (shortcut.isNotEmpty())
        {
            text << "\n\nShortcut: " << shortcut;
        }
        
        return text;
    }
};

//==============================================================================
/**
    MKS70TooltipDatabase - Database of all parameter tooltips
*/
class MKS70TooltipDatabase
{
public:
    static MKS70TooltipDatabase& getInstance()
    {
        static MKS70TooltipDatabase instance;
        return instance;
    }
    
    const ParameterTooltipInfo* getTooltip(const juce::String& paramId) const
    {
        auto it = tooltips.find(paramId);
        if (it != tooltips.end())
            return &it->second;
        return nullptr;
    }
    
private:
    MKS70TooltipDatabase()
    {
        initializeTooltips();
    }
    
    void initializeTooltips()
    {
        // === DCO 1 ===
        addTooltip("dco1_range", {
            "DCO1 Range", 
            "Sets the octave range of DCO1. Lower values produce deeper bass tones.",
            "DCO1", 0, 3, "",
            {"16'", "8'", "4'", "2'"}
        });
        
        addTooltip("dco1_waveform", {
            "DCO1 Waveform",
            "Selects the oscillator waveform. Each has a distinct harmonic character:\n"
            "- Noise: White noise for effects and percussion\n"
            "- Square: Hollow, clarinet-like tone\n"
            "- Pulse: Variable width for phasing effects\n"
            "- Saw: Bright, rich in harmonics",
            "DCO1", 0, 3, "",
            {"Noise", "Square", "Pulse", "Saw"}
        });
        
        addTooltip("dco1_tune", {
            "DCO1 Tune",
            "Fine tunes DCO1 pitch. Center (64) is normal pitch. "
            "Use for detuning effects between oscillators.",
            "DCO1", 0, 127, "cents"
        });
        
        addTooltip("dco1_lfo", {
            "DCO1 LFO Mod",
            "Amount of LFO modulation applied to DCO1 pitch. "
            "Creates vibrato effect at low rates or FM-like tones at high rates.",
            "DCO1", 0, 127, ""
        });
        
        addTooltip("dco1_env", {
            "DCO1 Envelope Mod",
            "Amount of ENV1 modulation applied to DCO1 pitch. "
            "Positive values sweep pitch up, negative values sweep down.",
            "DCO1", 0, 127, ""
        });
        
        // === DCO 2 ===
        addTooltip("dco2_range", {
            "DCO2 Range",
            "Sets the octave range of DCO2. Can be offset from DCO1 for layering.",
            "DCO2", 0, 3, "",
            {"16'", "8'", "4'", "2'"}
        });
        
        addTooltip("dco2_waveform", {
            "DCO2 Waveform",
            "Selects DCO2 waveform. Combine with DCO1 for complex timbres.",
            "DCO2", 0, 3, "",
            {"Noise", "Square", "Pulse", "Saw"}
        });
        
        addTooltip("dco2_crossmod", {
            "Cross Modulation",
            "DCO1 modulates DCO2 frequency for metallic, bell-like tones. "
            "Higher values create more complex harmonics.",
            "DCO2", 0, 3, "",
            {"Off", "Sync", "Metal", "Ring"}
        });
        
        addTooltip("dco2_tune", {
            "DCO2 Tune",
            "Fine tunes DCO2 pitch. Detune from DCO1 for chorus/unison effects. "
            "+/- 7 semitones range.",
            "DCO2", 0, 127, "cents"
        });
        
        addTooltip("dco2_finetune", {
            "DCO2 Fine Tune",
            "Very fine pitch adjustment for DCO2. Use for subtle detuning "
            "that adds warmth and width.",
            "DCO2", 0, 127, "cents"
        });
        
        // === MIXER ===
        addTooltip("mixer_dco1", {
            "DCO1 Level",
            "Volume level of DCO1 in the mix. Balance with DCO2 for desired tone.",
            "Mixer", 0, 127, ""
        });
        
        addTooltip("mixer_dco2", {
            "DCO2 Level",
            "Volume level of DCO2 in the mix. Set to 0 for single-oscillator sounds.",
            "Mixer", 0, 127, ""
        });
        
        addTooltip("mixer_env", {
            "Env Mod Depth",
            "Amount of envelope modulation on mixer balance. "
            "Creates dynamic timbral changes over time.",
            "Mixer", 0, 127, ""
        });
        
        // === VCF ===
        addTooltip("vcf_cutoff", {
            "Filter Cutoff",
            "Sets the cutoff frequency of the low-pass filter. "
            "Lower values create darker, muffled tones. "
            "Higher values allow more harmonics through.",
            "VCF", 0, 127, ""
        });
        
        addTooltip("vcf_resonance", {
            "Filter Resonance",
            "Emphasizes frequencies around the cutoff point. "
            "High values create a sharp, whistling peak. "
            "At maximum, the filter self-oscillates.",
            "VCF", 0, 127, ""
        });
        
        addTooltip("vcf_env", {
            "Filter Envelope",
            "Amount of ENV1 modulation on filter cutoff. "
            "Positive values sweep filter open, negative values close it.",
            "VCF", 0, 127, ""
        });
        
        addTooltip("vcf_lfo", {
            "Filter LFO",
            "Amount of LFO modulation on filter cutoff. "
            "Creates wah-wah effects at low rates.",
            "VCF", 0, 127, ""
        });
        
        addTooltip("vcf_keyfollow", {
            "Filter Key Follow",
            "Makes filter cutoff track keyboard position. "
            "Higher notes have brighter tone (like acoustic instruments).",
            "VCF", 0, 127, ""
        });
        
        addTooltip("vcf_hpf", {
            "High Pass Filter",
            "Removes low frequencies. Higher values thin out the bass. "
            "Useful for cutting through a mix.",
            "VCF", 0, 3, "",
            {"Off", "1", "2", "3"}
        });
        
        // === VCA ===
        addTooltip("vca_level", {
            "VCA Level",
            "Master volume level of the voice. "
            "Controls overall loudness before effects.",
            "VCA", 0, 127, ""
        });
        
        addTooltip("vca_env", {
            "VCA Envelope Mode",
            "Selects which envelope controls amplitude:\n"
            "- Gate: Instant on/off (organ-like)\n"
            "- ENV2: Full ADSR envelope shaping",
            "VCA", 0, 1, "",
            {"Gate", "ENV2"}
        });
        
        // === ENV 1 ===
        addTooltip("env1_attack", {
            "ENV1 Attack",
            "Time for envelope to reach peak after key press. "
            "0 = instant attack, 127 = slow fade in (~10 sec).",
            "ENV1", 0, 127, "ms"
        });
        
        addTooltip("env1_decay", {
            "ENV1 Decay",
            "Time to fall from peak to sustain level. "
            "Short decay for percussive sounds, long for pads.",
            "ENV1", 0, 127, "ms"
        });
        
        addTooltip("env1_sustain", {
            "ENV1 Sustain",
            "Level held while key is pressed. "
            "0 = no sustain (percussive), 127 = full level (organ-like).",
            "ENV1", 0, 127, ""
        });
        
        addTooltip("env1_release", {
            "ENV1 Release",
            "Time to fade to zero after key release. "
            "Short for staccato, long for ambient tails.",
            "ENV1", 0, 127, "ms"
        });
        
        addTooltip("env1_keyfollow", {
            "ENV1 Key Follow",
            "Makes envelope times shorter for higher notes. "
            "Mimics acoustic instrument behavior.",
            "ENV1", 0, 127, ""
        });
        
        // === ENV 2 ===
        addTooltip("env2_attack", {
            "ENV2 Attack",
            "Amplitude envelope attack time. "
            "Controls how quickly the sound reaches full volume.",
            "ENV2", 0, 127, "ms"
        });
        
        addTooltip("env2_decay", {
            "ENV2 Decay",
            "Amplitude envelope decay time. "
            "How quickly volume drops to sustain level.",
            "ENV2", 0, 127, "ms"
        });
        
        addTooltip("env2_sustain", {
            "ENV2 Sustain",
            "Amplitude sustain level. "
            "Volume held while key is down.",
            "ENV2", 0, 127, ""
        });
        
        addTooltip("env2_release", {
            "ENV2 Release",
            "Amplitude envelope release time. "
            "How long the sound fades after releasing the key.",
            "ENV2", 0, 127, "ms"
        });
        
        // === LFO ===
        addTooltip("lfo_rate", {
            "LFO Rate",
            "Speed of the LFO oscillation. "
            "Slow for vibrato/tremolo, fast for FM effects.",
            "LFO", 0, 127, "Hz"
        });
        
        addTooltip("lfo_delay", {
            "LFO Delay",
            "Time before LFO starts after key press. "
            "Useful for delayed vibrato effect.",
            "LFO", 0, 127, "ms"
        });
        
        addTooltip("lfo_waveform", {
            "LFO Waveform",
            "Shape of the LFO:\n"
            "- Random: Sample & hold for robotic effects\n"
            "- Square: Abrupt switching\n"
            "- Triangle: Smooth back-and-forth\n"
            "- Saw: Ramp up/down patterns",
            "LFO", 0, 3, "",
            {"Random", "Square", "Triangle", "Saw"}
        });
        
        // === CHORUS ===
        addTooltip("chorus_rate", {
            "Chorus Rate",
            "Speed of the chorus modulation. "
            "Slow for gentle movement, fast for Leslie-like effects.",
            "Chorus", 0, 127, "Hz"
        });
        
        addTooltip("chorus_depth", {
            "Chorus Depth",
            "Intensity of the chorus effect. "
            "Higher values create wider, more dramatic modulation.",
            "Chorus", 0, 127, ""
        });
        
        // === PATCH ===
        addTooltip("patch_keymode", {
            "Key Mode",
            "How the keyboard is split between tones:\n"
            "- Whole: Both tones across entire keyboard\n"
            "- Split: Upper tone above split point\n"
            "- Dual: Layer both tones together",
            "Patch", 0, 2, "",
            {"Whole", "Split", "Dual"}
        });
        
        addTooltip("patch_splitpoint", {
            "Split Point",
            "MIDI note number where keyboard splits. "
            "60 = Middle C. Only active in Split mode.",
            "Patch", 0, 127, ""
        });
        
        addTooltip("patch_balance", {
            "Tone Balance",
            "Volume balance between Upper and Lower tones. "
            "64 = equal, <64 favors Lower, >64 favors Upper.",
            "Patch", 0, 127, ""
        });
        
        addTooltip("patch_portamento", {
            "Portamento Time",
            "Glide time between notes. "
            "0 = instant, higher = slower glide.",
            "Patch", 0, 127, "ms"
        });
        
        addTooltip("patch_bendrange", {
            "Bend Range",
            "Pitch bend range in semitones. "
            "12 = one octave up/down.",
            "Patch", 0, 12, "semitones"
        });
    }
    
    void addTooltip(const juce::String& id, const ParameterTooltipInfo& info)
    {
        tooltips[id] = info;
    }
    
    std::map<juce::String, ParameterTooltipInfo> tooltips;
};

//==============================================================================
/**
    EnhancedTooltipWindow - Custom tooltip with rich formatting
*/
class EnhancedTooltipWindow : public juce::TooltipWindow
{
public:
    EnhancedTooltipWindow(juce::Component* parent = nullptr)
        : juce::TooltipWindow(parent, 500)  // 500ms delay
    {
        setOpaque(false);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background with glow
        g.setColour(DS::Color::bgElevated());
        g.fillRoundedRectangle(bounds.reduced(2), DS::Radius::M);
        
        // Border
        g.setColour(DS::Color::Accent::primary().withAlpha(0.5f));
        g.drawRoundedRectangle(bounds.reduced(2), DS::Radius::M, 1.0f);
        
        // Text
        g.setColour(DS::Color::textPrimary());
        g.setFont(DS::Font::small());
        
        auto textBounds = getLocalBounds().reduced(10, 8);
        g.drawFittedText(getTipFor(*this), textBounds, 
                         juce::Justification::topLeft, 10);
    }
    
    juce::Rectangle<int> getTooltipBounds(const juce::String& tipText,
                                          juce::Point<int> screenPos,
                                          juce::Rectangle<int> parentArea) override
    {
        const int maxWidth = 350;
        const int padding = 20;
        
        juce::AttributedString s;
        s.append(tipText, DS::Font::small(), DS::Color::textPrimary());
        
        juce::TextLayout tl;
        tl.createLayout(s, static_cast<float>(maxWidth - padding));
        
        auto width = juce::jmin(maxWidth, static_cast<int>(tl.getWidth()) + padding);
        auto height = static_cast<int>(tl.getHeight()) + 16;
        
        auto x = screenPos.x;
        auto y = screenPos.y + 20;
        
        // Keep on screen
        if (x + width > parentArea.getRight())
            x = parentArea.getRight() - width;
        if (y + height > parentArea.getBottom())
            y = screenPos.y - height - 10;
        
        return {x, y, width, height};
    }
};

//==============================================================================
/**
    TooltipHelper - Static helper to set tooltips on components
*/
class TooltipHelper
{
public:
    static void setParameterTooltip(juce::Component& component, 
                                    const juce::String& paramId,
                                    int currentValue = 0)
    {
        auto* info = MKS70TooltipDatabase::getInstance().getTooltip(paramId);
        if (info)
        {
            if (auto* settable = dynamic_cast<juce::SettableTooltipClient*>(&component))
            {
                settable->setTooltip(info->getTooltipText(currentValue));
            }
        }
    }
    
    static void setSimpleTooltip(juce::Component& component,
                                 const juce::String& name,
                                 const juce::String& description)
    {
        if (auto* settable = dynamic_cast<juce::SettableTooltipClient*>(&component))
        {
            juce::String tip = name;
            if (description.isNotEmpty())
                tip << "\n\n" << description;
            settable->setTooltip(tip);
        }
    }
};
