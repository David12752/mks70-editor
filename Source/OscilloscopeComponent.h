#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * Enhanced oscilloscope with waveform and spectrum analyzer modes
 * Click to toggle between modes
 */
class OscilloscopeComponent : public juce::Component, public juce::Timer {
public:
    enum class DisplayMode { Waveform, Spectrum, Both };
    
    OscilloscopeComponent() {
        // Initialize spectrum data
        for (int i = 0; i < numBands; i++) {
            spectrumBands[i] = 0.0f;
            spectrumPeaks[i] = 0.0f;
        }
        startTimerHz(30);
    }
    
    ~OscilloscopeComponent() override { stopTimer(); }
    
    // Toggle display mode
    void setDisplayMode(DisplayMode mode) { displayMode = mode; repaint(); }
    DisplayMode getDisplayMode() const { return displayMode; }
    void cycleDisplayMode() {
        if (displayMode == DisplayMode::Waveform) displayMode = DisplayMode::Spectrum;
        else if (displayMode == DisplayMode::Spectrum) displayMode = DisplayMode::Both;
        else displayMode = DisplayMode::Waveform;
        repaint();
    }
    
    // DCO Parameters
    void setDCO1Wave(int wave) { dco1Wave = wave; }
    void setDCO2Wave(int wave) { dco2Wave = wave; }
    void setDCO1Level(float v) { dco1Level = v / 127.0f; }
    void setDCO2Level(float v) { dco2Level = v / 127.0f; }
    void setDCO1Tune(float v) { dco1Tune = v; }
    void setDCO2Tune(float v) { dco2Tune = v; }
    
    // VCF Parameters
    void setCutoff(float v) { vcfCutoff = v / 127.0f; }
    void setResonance(float v) { vcfReso = v / 127.0f; }
    void setVCFEnvMod(float v) { vcfEnvMod = v / 127.0f; }
    
    // LFO Parameters
    void setLFORate(float v) { lfoRate = v / 127.0f; }
    void setLFOWave(int w) { lfoWave = w; }
    
    // ENV1 (VCF Envelope)
    void setEnv1Attack(float v) { env1A = v / 127.0f; }
    void setEnv1Decay(float v) { env1D = v / 127.0f; }
    void setEnv1Sustain(float v) { env1S = v / 127.0f; }
    void setEnv1Release(float v) { env1R = v / 127.0f; }
    
    // ENV2 (VCA Envelope)
    void setEnv2Attack(float v) { env2A = v / 127.0f; }
    void setEnv2Decay(float v) { env2D = v / 127.0f; }
    void setEnv2Sustain(float v) { env2S = v / 127.0f; }
    void setEnv2Release(float v) { env2R = v / 127.0f; }
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isLeftButtonDown()) {
            cycleDisplayMode();
        }
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Dark background with subtle gradient
        juce::ColourGradient bgGrad(
            juce::Colour(0xFF0A0A0C), bounds.getTopLeft(),
            juce::Colour(0xFF050508), bounds.getBottomLeft(), false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, DS::Radius::M);
        
        // Subtle border
        g.setColour(DS::Color::border());
        g.drawRoundedRectangle(bounds.reduced(0.5f), DS::Radius::M, 1.0f);
        
        // Title with mode indicator
        auto header = bounds.removeFromTop(18);
        g.setColour(DS::Color::textMuted());
        g.setFont(DS::Font::tiny());
        
        juce::String modeStr;
        switch (displayMode) {
            case DisplayMode::Waveform: modeStr = "SCOPE"; break;
            case DisplayMode::Spectrum: modeStr = "SPECTRUM"; break;
            case DisplayMode::Both:     modeStr = "SCOPE+SPECTRUM"; break;
        }
        g.drawText(modeStr, header.reduced(6, 2), juce::Justification::left);
        
        // Click hint
        g.setColour(DS::Color::textMuted().withAlpha(0.4f));
        g.setFont(DS::Font::tiny());
        g.drawText("click to cycle", header.reduced(6, 2), juce::Justification::right);
        
        // Main display area
        auto display = bounds.reduced(4);
        
        // Draw based on mode
        if (displayMode == DisplayMode::Both) {
            auto topHalf = display.removeFromTop(display.getHeight() * 0.5f);
            auto bottomHalf = display;
            
            drawGrid(g, topHalf);
            drawFinalWaveform(g, topHalf);
            
            drawSpectrumGrid(g, bottomHalf);
            drawSpectrum(g, bottomHalf);
        }
        else if (displayMode == DisplayMode::Spectrum) {
            drawSpectrumGrid(g, display);
            drawSpectrum(g, display);
        }
        else {
            drawGrid(g, display);
            drawFinalWaveform(g, display);
        }
    }
    
    void resized() override {}
    
    void timerCallback() override {
        phase += 0.05f + lfoRate * 0.02f;
        if (phase > juce::MathConstants<float>::twoPi * 4) 
            phase -= juce::MathConstants<float>::twoPi * 4;
        
        // Update spectrum analyzer data
        updateSpectrumData();
        
        repaint();
    }

private:
    DisplayMode displayMode = DisplayMode::Waveform;
    
    // Spectrum analyzer
    static constexpr int numBands = 24;
    float spectrumBands[numBands];
    float spectrumPeaks[numBands];  // Peak hold values
    // Synth parameters
    int dco1Wave = 1, dco2Wave = 1, lfoWave = 3;
    float dco1Level = 0.8f, dco2Level = 0.6f;
    float dco1Tune = 64.0f, dco2Tune = 64.0f;
    float vcfCutoff = 0.8f, vcfReso = 0.2f, vcfEnvMod = 0.5f;
    float lfoRate = 0.3f;
    float env1A = 0.1f, env1D = 0.3f, env1S = 0.7f, env1R = 0.4f;
    float env2A = 0.05f, env2D = 0.2f, env2S = 0.8f, env2R = 0.3f;
    
    float phase = 0.0f;
    float filterState1 = 0.0f, filterState2 = 0.0f;
    
    void drawGrid(juce::Graphics& g, juce::Rectangle<float> b) {
        g.setColour(juce::Colour(0xFF151518));
        
        // Horizontal center line (zero crossing)
        float centerY = b.getCentreY();
        g.drawHorizontalLine((int)centerY, b.getX(), b.getRight());
        
        // Faint grid lines
        g.setColour(juce::Colour(0xFF101012));
        for (int i = 1; i < 4; i++) {
            float x = b.getX() + b.getWidth() * i / 4.0f;
            g.drawVerticalLine((int)x, b.getY(), b.getBottom());
        }
    }
    
    // Oscillator waveform calculation
    float calcOsc(int wave, float p) {
        float n = std::fmod(p, juce::MathConstants<float>::twoPi);
        if (n < 0) n += juce::MathConstants<float>::twoPi;
        float pi = juce::MathConstants<float>::pi;
        
        switch (wave) {
            case 0: // Noise
                return juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
            case 1: // Saw
                return (n / pi) - 1.0f;
            case 2: // Pulse
                return n < pi * 1.0f ? 0.85f : -0.85f;
            case 3: // Square
                return n < pi ? 0.85f : -0.85f;
            default: return std::sin(p);
        }
    }
    
    // Simple resonant lowpass filter - more responsive to cutoff/reso
    float applyFilter(float input, float cutoff, float reso) {
        // Make cutoff more responsive (quadratic response)
        float fc = cutoff * cutoff * cutoff * 0.5f + 0.01f;
        // Resonance feedback - more pronounced
        float fb = reso * reso * 4.0f;
        
        filterState1 += fc * (input - filterState1 + fb * (filterState1 - filterState2));
        filterState2 += fc * (filterState1 - filterState2);
        
        // Resonance can cause clipping - soft clip
        float out = filterState2;
        if (out > 1.0f) out = 1.0f - 1.0f / (out + 1.0f);
        else if (out < -1.0f) out = -1.0f + 1.0f / (-out + 1.0f);
        
        return out;
    }
    
    void drawFinalWaveform(juce::Graphics& g, juce::Rectangle<float> b) {
        if (b.getWidth() < 10) return;
        
        juce::Path path;
        int n = (int)b.getWidth();
        float centerY = b.getCentreY();
        float amp = b.getHeight() * 0.4f;
        
        // Pitch ratio from DCO2 tune - more dramatic range
        float pitchRatio2 = std::pow(2.0f, (dco2Tune - 64.0f) / 60.0f);
        
        // LFO value for modulation - more visible
        float lfoVal = std::sin(phase * 0.2f) * 0.15f * lfoRate;
        
        // Envelope values (simplified - always in sustain-like state for visualization)
        float envVal = env1S * 0.8f + 0.2f;
        float vcaEnv = env2S * 0.7f + 0.3f;
        
        // Reset filter states for each frame to prevent drift
        filterState1 = 0.0f;
        filterState2 = 0.0f;
        
        for (int i = 0; i < n; i++) {
            float x = b.getX() + i;
            float t = (float)i / n * juce::MathConstants<float>::twoPi * 4.0f;
            
            // Mix both DCOs with more visible wave shapes
            float osc1 = calcOsc(dco1Wave, t + phase * 0.5f) * dco1Level;
            float osc2 = calcOsc(dco2Wave, (t + phase * 0.4f) * pitchRatio2) * dco2Level;
            float mix = (osc1 + osc2) * 0.6f;
            
            // Apply filter with envelope/LFO modulation - more responsive
            float dynCutoff = juce::jlimit(0.02f, 0.98f, vcfCutoff + envVal * vcfEnvMod * 0.4f + lfoVal);
            float filtered = applyFilter(mix, dynCutoff, vcfReso);
            
            // Apply VCA envelope
            float sample = filtered * vcaEnv;
            
            float y = centerY - sample * amp;
            if (i == 0) path.startNewSubPath(x, y);
            else path.lineTo(x, y);
        }
        
        // Accent color based on primary oscillator
        juce::Colour waveColor = DS::Color::Accent::cyan();
        
        // Outer glow
        g.setColour(waveColor.withAlpha(0.08f));
        g.strokePath(path, juce::PathStrokeType(6.0f));
        
        // Medium glow
        g.setColour(waveColor.withAlpha(0.2f));
        g.strokePath(path, juce::PathStrokeType(3.0f));
        
        // Main line
        g.setColour(waveColor.withAlpha(0.85f));
        g.strokePath(path, juce::PathStrokeType(1.5f));
        
        // Bright core
        g.setColour(waveColor.brighter(0.3f));
        g.strokePath(path, juce::PathStrokeType(0.8f));
    }
    
    // Generate simulated spectrum data based on synth parameters
    void updateSpectrumData() {
        // Decay peaks
        for (int i = 0; i < numBands; i++) {
            spectrumPeaks[i] *= 0.96f;  // Slow peak decay
        }
        
        // Base frequency bands - simulate harmonic content based on oscillator waves
        float baseLevel = (dco1Level + dco2Level) * 0.5f;
        
        for (int i = 0; i < numBands; i++) {
            float freq = (float)(i + 1) / numBands;  // 0 to 1 frequency range
            float level = 0.0f;
            
            // Saw wave has all harmonics with 1/n falloff
            if (dco1Wave == 1) {  // Saw
                level += dco1Level * (0.8f / (i + 1));
            }
            // Pulse/Square has odd harmonics
            else if (dco1Wave == 2 || dco1Wave == 3) {
                if (i % 2 == 0) level += dco1Level * (0.7f / (i + 1));
            }
            // Noise has flat spectrum
            else if (dco1Wave == 0) {
                level += dco1Level * 0.3f * (juce::Random::getSystemRandom().nextFloat() * 0.5f + 0.5f);
            }
            
            // Same for DCO2
            if (dco2Wave == 1) {
                level += dco2Level * (0.7f / (i + 1));
            }
            else if (dco2Wave == 2 || dco2Wave == 3) {
                if (i % 2 == 0) level += dco2Level * (0.6f / (i + 1));
            }
            else if (dco2Wave == 0) {
                level += dco2Level * 0.25f * (juce::Random::getSystemRandom().nextFloat() * 0.5f + 0.5f);
            }
            
            // Apply VCF effect - attenuate frequencies above cutoff
            float cutoffBand = vcfCutoff * numBands;
            float cutoffDist = (float)i - cutoffBand;
            
            if (cutoffDist > 0) {
                // Steep rolloff above cutoff (12dB/oct simulation)
                float atten = std::exp(-cutoffDist * 0.4f);
                level *= atten;
            }
            
            // Strong resonance peak at cutoff - Q spike
            float resoPeak = std::exp(-cutoffDist * cutoffDist / (4.0f + (1.0f - vcfReso) * 20.0f));
            level += vcfReso * vcfReso * 1.5f * resoPeak;
            
            // Add some animation
            level *= 0.7f + 0.3f * std::sin(phase * 0.1f + i * 0.2f);
            
            // Smooth transition
            spectrumBands[i] = spectrumBands[i] * 0.7f + level * 0.3f;
            
            // Update peaks
            if (spectrumBands[i] > spectrumPeaks[i]) {
                spectrumPeaks[i] = spectrumBands[i];
            }
        }
    }
    
    void drawSpectrumGrid(juce::Graphics& g, juce::Rectangle<float> b) {
        // Horizontal dB lines
        g.setColour(juce::Colour(0xFF101012));
        for (int i = 1; i < 4; i++) {
            float y = b.getY() + b.getHeight() * i / 4.0f;
            g.drawHorizontalLine((int)y, b.getX(), b.getRight());
        }
        
        // Frequency markers
        g.setColour(juce::Colour(0xFF0E0E10));
        for (int i = 1; i < 8; i++) {
            float x = b.getX() + b.getWidth() * i / 8.0f;
            g.drawVerticalLine((int)x, b.getY(), b.getBottom());
        }
    }
    
    void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> b) {
        if (b.getWidth() < 10 || b.getHeight() < 10) return;
        
        float barWidth = b.getWidth() / numBands;
        float gap = 2.0f;
        
        for (int i = 0; i < numBands; i++) {
            float x = b.getX() + i * barWidth;
            float level = juce::jlimit(0.0f, 1.0f, spectrumBands[i]);
            float barHeight = level * b.getHeight() * 0.9f;
            
            // Color gradient from cyan to magenta based on frequency
            float hue = 0.5f - (float)i / numBands * 0.2f;  // Cyan to blue-ish
            juce::Colour barColor = juce::Colour::fromHSV(hue, 0.7f, 0.9f, 1.0f);
            
            // Draw bar with gradient
            juce::Rectangle<float> bar(x + gap * 0.5f, 
                                        b.getBottom() - barHeight, 
                                        barWidth - gap, 
                                        barHeight);
            
            juce::ColourGradient grad(barColor.withAlpha(0.9f), bar.getBottomLeft(),
                                      barColor.withAlpha(0.4f), bar.getTopLeft(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bar, 2.0f);
            
            // Glow effect
            g.setColour(barColor.withAlpha(0.15f));
            g.fillRoundedRectangle(bar.expanded(2, 0), 3.0f);
            
            // Peak indicator
            float peakY = b.getBottom() - spectrumPeaks[i] * b.getHeight() * 0.9f;
            if (peakY < b.getBottom() - 2) {
                g.setColour(barColor.brighter(0.5f).withAlpha(0.8f));
                g.fillRect(x + gap * 0.5f, peakY - 2, barWidth - gap, 2.0f);
            }
        }
        
        // Frequency labels
        g.setColour(DS::Color::textMuted().withAlpha(0.4f));
        g.setFont(DS::Font::tiny());
        g.drawText("LOW", b.removeFromLeft(30).translated(0, -2), juce::Justification::bottomLeft);
        g.drawText("HIGH", b.removeFromRight(30).translated(0, -2), juce::Justification::bottomRight);
    }
};
