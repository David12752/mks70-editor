#pragma once

#include "WaveformDisplay.h"

class MixerWaveformDisplay : public WaveformDisplay
{
public:
    void setDCO1Params(int wave, int level)
    {
        dco1Wave = juce::jlimit(0, 3, wave);
        dco1Level = juce::jlimit(0, 127, level);
        repaint();
    }

    void setDCO2Params(int wave, int level, int cross, int fineDetune)
    {
        dco2Wave = juce::jlimit(0, 3, wave);
        dco2Level = juce::jlimit(0, 127, level);
        crossMod = juce::jlimit(0, 3, cross);
        detune = juce::jlimit(0, 127, fineDetune);
        repaint();
    }

protected:
    float getSample(float phase) override
    {
        const float d1 = renderOsc(dco1Wave, phase);
        const float detunePhase = phase + detuneToPhaseOffset();
        float d2 = renderOsc(dco2Wave, detunePhase);

        if (crossMod > 0)
            d2 = applyCrossMod(d1, d2, crossMod);

        const float mix = ((d1 * (float)dco1Level) + (d2 * (float)dco2Level)) / 254.0f;
        return juce::jlimit(-1.0f, 1.0f, mix);
    }

private:
    static float renderOsc(int wave, float phase)
    {
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float p = std::fmod(phase, twoPi);
        switch (wave)
        {
            case 1: // Saw
                return 1.0f - 2.0f * (p / twoPi);
            case 2: // Pulse (50%)
                return (p < juce::MathConstants<float>::pi * 0.5f ? 1.0f : -1.0f);
            case 3: // Square (50%)
                return (p < juce::MathConstants<float>::pi ? 1.0f : -1.0f);
            case 0: // Noise (deterministic-ish)
            default:
            {
                const uint32_t k = (uint32_t)std::floor(phase * 1000.0f) * 2654435761u;
                const float nf = ((k >> 9) & 0xFFFF) / 32767.5f;
                return juce::jlimit(-1.0f, 1.0f, nf - 1.0f);
            }
        }
    }

    static float applyCrossMod(float d1, float d2, int mode)
    {
        switch (mode)
        {
            case 1: // Sync-like shaping
                return d2 * juce::jlimit(0.0f, 1.0f, std::abs(d1));
            case 2: // Ring/Metal
                return d1 * d2;
            case 3: // Both (average)
                return 0.5f * (d2 * juce::jlimit(0.0f, 1.0f, std::abs(d1)) + (d1 * d2));
            default:
                return d2;
        }
    }

    float detuneToPhaseOffset() const
    {
        // Map detune (0..127, 64=none) to a small phase offset up to +/- pi/4
        const float centered = ((float)detune - 64.0f) / 64.0f; // ~ -1..+1
        return centered * (juce::MathConstants<float>::pi * 0.25f);
    }

    int dco1Wave = 1;
    int dco2Wave = 1;
};
