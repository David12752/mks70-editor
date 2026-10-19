#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include <cmath>

/**
 * VisualFeedback.h
 * Real-time animated visual feedback components
 * - LFO Animation Display
 * - Envelope ADSR Visualization
 * - VU Meters
 * 
 * Part of Improvement #4: Visual Feedback System
 */

namespace VisualFeedback
{

//==============================================================================
/**
 * LFO Waveform Types
 */
enum class LFOWaveform
{
    Sine,
    Triangle,
    Sawtooth,
    Square,
    Random,
    SampleAndHold
};

//==============================================================================
/**
 * Animated LFO Display
 * Shows real-time LFO waveform with phase indicator
 */
class LFODisplay : public juce::Component, private juce::Timer
{
public:
    LFODisplay()
    {
        startTimerHz(60); // 60 FPS animation
    }
    
    ~LFODisplay() override
    {
        stopTimer();
    }
    
    void setWaveform(LFOWaveform waveform)
    {
        currentWaveform = waveform;
        repaint();
    }
    
    void setRate(float rateHz)
    {
        lfoRate = juce::jlimit(0.01f, 30.0f, rateHz);
    }
    
    void setDepth(float depth)
    {
        lfoDepth = juce::jlimit(0.0f, 1.0f, depth);
    }
    
    void setDelay(float delayMs)
    {
        lfoDelay = juce::jlimit(0.0f, 5000.0f, delayMs);
    }
    
    void setRunning(bool shouldRun)
    {
        isRunning = shouldRun;
        if (!isRunning)
            phase = 0.0f;
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, DS::Radius::small);
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::small, 1.0f);
        
        if (bounds.getWidth() < 20 || bounds.getHeight() < 20)
            return;
        
        auto graphBounds = bounds.reduced(6);
        float centerY = graphBounds.getCentreY();
        float amplitude = graphBounds.getHeight() * 0.4f * lfoDepth;
        
        // Draw waveform
        juce::Path waveformPath;
        bool pathStarted = false;
        
        for (float x = 0; x <= graphBounds.getWidth(); x += 1.0f)
        {
            float normalizedX = x / graphBounds.getWidth();
            float waveValue = getWaveformValue(normalizedX);
            float y = centerY - (waveValue * amplitude);
            
            if (!pathStarted)
            {
                waveformPath.startNewSubPath(graphBounds.getX() + x, y);
                pathStarted = true;
            }
            else
            {
                waveformPath.lineTo(graphBounds.getX() + x, y);
            }
        }
        
        // Gradient stroke for waveform
        juce::ColourGradient waveGradient(
            DS::Color::accent.withAlpha(0.9f),
            graphBounds.getX(), centerY,
            DS::Color::accent.withAlpha(0.5f),
            graphBounds.getRight(), centerY,
            false
        );
        
        g.setGradientFill(waveGradient);
        g.strokePath(waveformPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
        
        // Draw center line
        g.setColour(DS::Color::textMuted.withAlpha(0.3f));
        g.drawHorizontalLine(static_cast<int>(centerY), graphBounds.getX(), graphBounds.getRight());
        
        // Draw phase indicator (moving dot)
        if (isRunning)
        {
            float indicatorX = graphBounds.getX() + (phase * graphBounds.getWidth());
            float indicatorY = centerY - (getWaveformValue(phase) * amplitude);
            
            // Glow
            g.setColour(DS::Color::accent.withAlpha(0.4f));
            g.fillEllipse(indicatorX - 8, indicatorY - 8, 16, 16);
            
            // Main dot
            g.setColour(DS::Color::accent);
            g.fillEllipse(indicatorX - 5, indicatorY - 5, 10, 10);
            
            // Highlight
            g.setColour(juce::Colours::white);
            g.fillEllipse(indicatorX - 2, indicatorY - 3, 4, 4);
        }
        
        // Draw waveform label
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        
        juce::String waveformName;
        switch (currentWaveform)
        {
            case LFOWaveform::Sine:         waveformName = "SIN"; break;
            case LFOWaveform::Triangle:     waveformName = "TRI"; break;
            case LFOWaveform::Sawtooth:     waveformName = "SAW"; break;
            case LFOWaveform::Square:       waveformName = "SQR"; break;
            case LFOWaveform::Random:       waveformName = "RND"; break;
            case LFOWaveform::SampleAndHold: waveformName = "S&H"; break;
        }
        
        g.drawText(waveformName, bounds.reduced(4).removeFromTop(14), juce::Justification::topLeft);
        
        // Draw rate
        g.drawText(juce::String(lfoRate, 1) + " Hz", bounds.reduced(4).removeFromTop(14), juce::Justification::topRight);
    }
    
private:
    void timerCallback() override
    {
        if (isRunning)
        {
            // Advance phase based on rate
            float phaseIncrement = lfoRate / 60.0f; // 60 FPS
            phase += phaseIncrement;
            
            if (phase >= 1.0f)
            {
                phase -= 1.0f;
                
                // Generate new random value for S&H
                if (currentWaveform == LFOWaveform::SampleAndHold || 
                    currentWaveform == LFOWaveform::Random)
                {
                    lastRandomValue = random.nextFloat() * 2.0f - 1.0f;
                }
            }
            
            repaint();
        }
    }
    
    float getWaveformValue(float normalizedPhase) const
    {
        switch (currentWaveform)
        {
            case LFOWaveform::Sine:
                return std::sin(normalizedPhase * juce::MathConstants<float>::twoPi);
                
            case LFOWaveform::Triangle:
            {
                float triPhase = std::fmod(normalizedPhase * 4.0f, 4.0f);
                if (triPhase < 1.0f) return triPhase;
                if (triPhase < 3.0f) return 2.0f - triPhase;
                return triPhase - 4.0f;
            }
                
            case LFOWaveform::Sawtooth:
                return 2.0f * normalizedPhase - 1.0f;
                
            case LFOWaveform::Square:
                return normalizedPhase < 0.5f ? 1.0f : -1.0f;
                
            case LFOWaveform::Random:
            {
                // Smooth random (interpolated)
                float segment = normalizedPhase * 8.0f;
                int segmentIndex = static_cast<int>(segment) % 8;
                float segmentPhase = segment - std::floor(segment);
                
                // Use deterministic random based on segment
                juce::Random segRandom(segmentIndex * 12345);
                float val1 = segRandom.nextFloat() * 2.0f - 1.0f;
                segRandom.setSeed((segmentIndex + 1) * 12345);
                float val2 = segRandom.nextFloat() * 2.0f - 1.0f;
                
                // Smooth interpolation
                float t = segmentPhase * segmentPhase * (3.0f - 2.0f * segmentPhase);
                return val1 + (val2 - val1) * t;
            }
                
            case LFOWaveform::SampleAndHold:
                return lastRandomValue;
                
            default:
                return 0.0f;
        }
    }
    
    LFOWaveform currentWaveform = LFOWaveform::Sine;
    float lfoRate = 1.0f;
    float lfoDepth = 1.0f;
    float lfoDelay = 0.0f;
    float phase = 0.0f;
    bool isRunning = true;
    
    juce::Random random;
    float lastRandomValue = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFODisplay)
};

//==============================================================================
/**
 * Envelope ADSR Display
 * Shows envelope shape with adjustable ADSR parameters
 */
class EnvelopeDisplay : public juce::Component
{
public:
    EnvelopeDisplay()
    {
    }
    
    void setADSR(float attack, float decay, float sustain, float release)
    {
        attackTime = juce::jlimit(0.0f, 1.0f, attack);
        decayTime = juce::jlimit(0.0f, 1.0f, decay);
        sustainLevel = juce::jlimit(0.0f, 1.0f, sustain);
        releaseTime = juce::jlimit(0.0f, 1.0f, release);
        repaint();
    }
    
    void setAttack(float attack)
    {
        attackTime = juce::jlimit(0.0f, 1.0f, attack);
        repaint();
    }
    
    void setDecay(float decay)
    {
        decayTime = juce::jlimit(0.0f, 1.0f, decay);
        repaint();
    }
    
    void setSustain(float sustain)
    {
        sustainLevel = juce::jlimit(0.0f, 1.0f, sustain);
        repaint();
    }
    
    void setRelease(float release)
    {
        releaseTime = juce::jlimit(0.0f, 1.0f, release);
        repaint();
    }
    
    void setKeyVelocity(float velocity)
    {
        keyVelocity = juce::jlimit(0.0f, 1.0f, velocity);
        repaint();
    }
    
    void setEnvelopeColor(juce::Colour color)
    {
        envelopeColor = color;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, DS::Radius::small);
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::small, 1.0f);
        
        if (bounds.getWidth() < 40 || bounds.getHeight() < 30)
            return;
        
        auto graphBounds = bounds.reduced(8, 12);
        float baseY = graphBounds.getBottom();
        float height = graphBounds.getHeight() * keyVelocity;
        
        // Calculate segment widths based on time parameters
        float totalTime = attackTime + decayTime + 0.3f + releaseTime; // 0.3 = sustain hold time
        float attackWidth = (attackTime / totalTime) * graphBounds.getWidth();
        float decayWidth = (decayTime / totalTime) * graphBounds.getWidth();
        float sustainWidth = (0.3f / totalTime) * graphBounds.getWidth();
        float releaseWidth = (releaseTime / totalTime) * graphBounds.getWidth();
        
        // Minimum widths for visibility
        attackWidth = juce::jmax(attackWidth, 8.0f);
        decayWidth = juce::jmax(decayWidth, 8.0f);
        sustainWidth = juce::jmax(sustainWidth, 20.0f);
        releaseWidth = juce::jmax(releaseWidth, 8.0f);
        
        // Normalize to fit
        float totalWidth = attackWidth + decayWidth + sustainWidth + releaseWidth;
        float scale = graphBounds.getWidth() / totalWidth;
        attackWidth *= scale;
        decayWidth *= scale;
        sustainWidth *= scale;
        releaseWidth *= scale;
        
        // Calculate key points
        float x0 = graphBounds.getX();
        float x1 = x0 + attackWidth;                    // End of attack
        float x2 = x1 + decayWidth;                     // End of decay
        float x3 = x2 + sustainWidth;                   // End of sustain
        float x4 = x3 + releaseWidth;                   // End of release
        
        float y0 = baseY;                               // Start
        float y1 = baseY - height;                      // Peak (after attack)
        float y2 = baseY - (height * sustainLevel);    // Sustain level
        float y3 = baseY;                               // End (after release)
        
        // Create envelope path with curves
        juce::Path envelopePath;
        envelopePath.startNewSubPath(x0, y0);
        
        // Attack (exponential curve)
        juce::Point<float> attackControl(x0 + attackWidth * 0.3f, y1);
        envelopePath.quadraticTo(attackControl.x, attackControl.y, x1, y1);
        
        // Decay (exponential curve)
        juce::Point<float> decayControl(x1 + decayWidth * 0.3f, y1);
        envelopePath.quadraticTo(decayControl.x, decayControl.y, x2, y2);
        
        // Sustain (flat line)
        envelopePath.lineTo(x3, y2);
        
        // Release (exponential curve)
        juce::Point<float> releaseControl(x3 + releaseWidth * 0.7f, y2);
        envelopePath.quadraticTo(releaseControl.x, releaseControl.y, x4, y3);
        
        // Fill under curve
        juce::Path fillPath = envelopePath;
        fillPath.lineTo(x4, baseY);
        fillPath.lineTo(x0, baseY);
        fillPath.closeSubPath();
        
        // Gradient fill
        juce::ColourGradient fillGradient(
            envelopeColor.withAlpha(0.4f),
            graphBounds.getX(), graphBounds.getY(),
            envelopeColor.withAlpha(0.05f),
            graphBounds.getX(), baseY,
            false
        );
        g.setGradientFill(fillGradient);
        g.fillPath(fillPath);
        
        // Draw envelope line
        g.setColour(envelopeColor);
        g.strokePath(envelopePath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved));
        
        // Draw stage markers
        g.setColour(DS::Color::textMuted.withAlpha(0.5f));
        g.drawVerticalLine(static_cast<int>(x1), graphBounds.getY(), baseY);
        g.drawVerticalLine(static_cast<int>(x2), graphBounds.getY(), baseY);
        g.drawVerticalLine(static_cast<int>(x3), graphBounds.getY(), baseY);
        
        // Draw key points
        auto drawPoint = [&](float x, float y, bool isSelected = false)
        {
            if (isSelected)
            {
                g.setColour(envelopeColor.withAlpha(0.4f));
                g.fillEllipse(x - 6, y - 6, 12, 12);
            }
            g.setColour(envelopeColor);
            g.fillEllipse(x - 4, y - 4, 8, 8);
            g.setColour(juce::Colours::white);
            g.fillEllipse(x - 2, y - 2, 4, 4);
        };
        
        drawPoint(x1, y1);  // Attack peak
        drawPoint(x2, y2);  // Decay end
        drawPoint(x3, y2);  // Sustain end
        
        // Draw labels
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        
        float labelY = bounds.getBottom() - 3;
        g.drawText("A", juce::Rectangle<float>(x0, labelY - 10, attackWidth, 10), juce::Justification::centred);
        g.drawText("D", juce::Rectangle<float>(x1, labelY - 10, decayWidth, 10), juce::Justification::centred);
        g.drawText("S", juce::Rectangle<float>(x2, labelY - 10, sustainWidth, 10), juce::Justification::centred);
        g.drawText("R", juce::Rectangle<float>(x3, labelY - 10, releaseWidth, 10), juce::Justification::centred);
    }
    
private:
    float attackTime = 0.1f;
    float decayTime = 0.2f;
    float sustainLevel = 0.7f;
    float releaseTime = 0.3f;
    float keyVelocity = 1.0f;
    
    juce::Colour envelopeColor = DS::Color::accent;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeDisplay)
};

//==============================================================================
/**
 * VU Meter Display
 * Shows signal level with peak hold
 */
class VUMeter : public juce::Component, private juce::Timer
{
public:
    VUMeter()
    {
        startTimerHz(30);
    }
    
    ~VUMeter() override
    {
        stopTimer();
    }
    
    void setLevel(float newLevel)
    {
        targetLevel = juce::jlimit(0.0f, 1.0f, newLevel);
        
        // Update peak
        if (targetLevel > peakLevel)
        {
            peakLevel = targetLevel;
            peakHoldTime = 1.5f; // Hold for 1.5 seconds
        }
    }
    
    void setVertical(bool isVertical)
    {
        vertical = isVertical;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1);
        
        // Background
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, 2);
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, 2, 1);
        
        auto meterBounds = bounds.reduced(2);
        
        if (vertical)
        {
            // Vertical meter
            float meterHeight = meterBounds.getHeight() * currentLevel;
            float peakY = meterBounds.getBottom() - (meterBounds.getHeight() * peakLevel);
            
            // Level gradient (green to yellow to red)
            juce::ColourGradient levelGradient(
                juce::Colour(0xFF00C853),  // Green
                meterBounds.getX(), meterBounds.getBottom(),
                juce::Colour(0xFFFF5252),  // Red
                meterBounds.getX(), meterBounds.getY(),
                false
            );
            levelGradient.addColour(0.7, juce::Colour(0xFFFFD740)); // Yellow at 70%
            
            g.setGradientFill(levelGradient);
            g.fillRoundedRectangle(
                meterBounds.getX(),
                meterBounds.getBottom() - meterHeight,
                meterBounds.getWidth(),
                meterHeight,
                2
            );
            
            // Peak indicator
            if (peakLevel > 0.01f)
            {
                g.setColour(juce::Colours::white);
                g.fillRect(meterBounds.getX(), peakY - 1, meterBounds.getWidth(), 2.0f);
            }
        }
        else
        {
            // Horizontal meter
            float meterWidth = meterBounds.getWidth() * currentLevel;
            float peakX = meterBounds.getX() + (meterBounds.getWidth() * peakLevel);
            
            // Level gradient
            juce::ColourGradient levelGradient(
                juce::Colour(0xFF00C853),  // Green
                meterBounds.getX(), meterBounds.getY(),
                juce::Colour(0xFFFF5252),  // Red
                meterBounds.getRight(), meterBounds.getY(),
                false
            );
            levelGradient.addColour(0.7, juce::Colour(0xFFFFD740));
            
            g.setGradientFill(levelGradient);
            g.fillRoundedRectangle(
                meterBounds.getX(),
                meterBounds.getY(),
                meterWidth,
                meterBounds.getHeight(),
                2
            );
            
            // Peak indicator
            if (peakLevel > 0.01f)
            {
                g.setColour(juce::Colours::white);
                g.fillRect(peakX - 1, meterBounds.getY(), 2.0f, meterBounds.getHeight());
            }
        }
    }
    
private:
    void timerCallback() override
    {
        // Smooth level animation
        float diff = targetLevel - currentLevel;
        if (std::abs(diff) > 0.001f)
        {
            currentLevel += diff * 0.3f;
            repaint();
        }
        else if (currentLevel != targetLevel)
        {
            currentLevel = targetLevel;
            repaint();
        }
        
        // Decay peak hold
        if (peakHoldTime > 0)
        {
            peakHoldTime -= 1.0f / 30.0f; // 30 FPS
        }
        else if (peakLevel > currentLevel)
        {
            peakLevel -= 0.02f;
            if (peakLevel < currentLevel)
                peakLevel = currentLevel;
            repaint();
        }
    }
    
    float targetLevel = 0.0f;
    float currentLevel = 0.0f;
    float peakLevel = 0.0f;
    float peakHoldTime = 0.0f;
    bool vertical = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};

//==============================================================================
/**
 * Stereo VU Meter
 * Two VU meters with labels
 */
class StereoVUMeter : public juce::Component
{
public:
    StereoVUMeter()
    {
        addAndMakeVisible(leftMeter);
        addAndMakeVisible(rightMeter);
        leftMeter.setVertical(true);
        rightMeter.setVertical(true);
    }
    
    void setLevels(float left, float right)
    {
        leftMeter.setLevel(left);
        rightMeter.setLevel(right);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(2);
        
        int meterWidth = (bounds.getWidth() - 4) / 2;
        leftMeter.setBounds(bounds.removeFromLeft(meterWidth));
        bounds.removeFromLeft(4);
        rightMeter.setBounds(bounds.removeFromLeft(meterWidth));
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        
        // Labels
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        
        auto labelBounds = bounds.removeFromBottom(12);
        g.drawText("L", labelBounds.removeFromLeft(bounds.getWidth() / 2), juce::Justification::centred);
        g.drawText("R", labelBounds, juce::Justification::centred);
    }
    
private:
    VUMeter leftMeter;
    VUMeter rightMeter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoVUMeter)
};

//==============================================================================
/**
 * Waveform Display
 * Shows DCO waveform shape
 */
class WaveformDisplay : public juce::Component
{
public:
    enum class Waveform
    {
        Saw,
        Pulse,
        Sub,
        Noise
    };
    
    WaveformDisplay()
    {
    }
    
    void setWaveform(Waveform wf)
    {
        currentWaveform = wf;
        repaint();
    }
    
    void setPulseWidth(float pw)
    {
        pulseWidth = juce::jlimit(0.0f, 1.0f, pw);
        repaint();
    }
    
    void setSubLevel(float level)
    {
        subLevel = juce::jlimit(0.0f, 1.0f, level);
        repaint();
    }
    
    void setMix(float saw, float pulse, float sub, float noise)
    {
        sawLevel = juce::jlimit(0.0f, 1.0f, saw);
        pulseLevel = juce::jlimit(0.0f, 1.0f, pulse);
        subLevel = juce::jlimit(0.0f, 1.0f, sub);
        noiseLevel = juce::jlimit(0.0f, 1.0f, noise);
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, DS::Radius::small);
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::small, 1.0f);
        
        if (bounds.getWidth() < 30 || bounds.getHeight() < 20)
            return;
        
        auto graphBounds = bounds.reduced(6);
        float centerY = graphBounds.getCentreY();
        float amplitude = graphBounds.getHeight() * 0.4f;
        
        juce::Path waveformPath;
        
        switch (currentWaveform)
        {
            case Waveform::Saw:
                drawSawWave(waveformPath, graphBounds, centerY, amplitude);
                break;
            case Waveform::Pulse:
                drawPulseWave(waveformPath, graphBounds, centerY, amplitude, pulseWidth);
                break;
            case Waveform::Sub:
                drawSubWave(waveformPath, graphBounds, centerY, amplitude);
                break;
            case Waveform::Noise:
                drawNoiseWave(waveformPath, graphBounds, centerY, amplitude);
                break;
        }
        
        // Draw center line
        g.setColour(DS::Color::textMuted.withAlpha(0.2f));
        g.drawHorizontalLine(static_cast<int>(centerY), graphBounds.getX(), graphBounds.getRight());
        
        // Draw waveform with glow
        g.setColour(DS::Color::accent.withAlpha(0.3f));
        g.strokePath(waveformPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved));
        
        g.setColour(DS::Color::accent);
        g.strokePath(waveformPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
    }
    
private:
    void drawSawWave(juce::Path& path, juce::Rectangle<float> bounds, float centerY, float amplitude)
    {
        float cycleWidth = bounds.getWidth() / 2;
        
        for (int i = 0; i < 2; i++)
        {
            float startX = bounds.getX() + i * cycleWidth;
            
            if (i == 0)
                path.startNewSubPath(startX, centerY + amplitude);
            else
                path.startNewSubPath(startX, centerY + amplitude);
            
            path.lineTo(startX + cycleWidth, centerY - amplitude);
            path.lineTo(startX + cycleWidth, centerY + amplitude);
        }
    }
    
    void drawPulseWave(juce::Path& path, juce::Rectangle<float> bounds, float centerY, float amplitude, float pw)
    {
        float cycleWidth = bounds.getWidth() / 2;
        float highWidth = cycleWidth * pw;
        
        for (int i = 0; i < 2; i++)
        {
            float startX = bounds.getX() + i * cycleWidth;
            
            if (i == 0)
                path.startNewSubPath(startX, centerY - amplitude);
            else
                path.startNewSubPath(startX, centerY + amplitude);
            
            // High part
            path.lineTo(startX, centerY - amplitude);
            path.lineTo(startX + highWidth, centerY - amplitude);
            
            // Transition
            path.lineTo(startX + highWidth, centerY + amplitude);
            
            // Low part
            path.lineTo(startX + cycleWidth, centerY + amplitude);
        }
    }
    
    void drawSubWave(juce::Path& path, juce::Rectangle<float> bounds, float centerY, float amplitude)
    {
        // Sub is one octave below - half frequency (one cycle instead of two)
        path.startNewSubPath(bounds.getX(), centerY - amplitude);
        path.lineTo(bounds.getX() + bounds.getWidth() * 0.5f, centerY - amplitude);
        path.lineTo(bounds.getX() + bounds.getWidth() * 0.5f, centerY + amplitude);
        path.lineTo(bounds.getRight(), centerY + amplitude);
    }
    
    void drawNoiseWave(juce::Path& path, juce::Rectangle<float> bounds, float centerY, float amplitude)
    {
        juce::Random random(42); // Fixed seed for consistent display
        
        path.startNewSubPath(bounds.getX(), centerY);
        
        for (float x = bounds.getX(); x <= bounds.getRight(); x += 3)
        {
            float y = centerY + (random.nextFloat() * 2.0f - 1.0f) * amplitude;
            path.lineTo(x, y);
        }
    }
    
    Waveform currentWaveform = Waveform::Saw;
    float pulseWidth = 0.5f;
    float sawLevel = 1.0f;
    float pulseLevel = 0.0f;
    float subLevel = 0.0f;
    float noiseLevel = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

//==============================================================================
/**
 * Filter Curve Display
 * Shows VCF frequency response
 */
class FilterCurveDisplay : public juce::Component
{
public:
    FilterCurveDisplay()
    {
    }
    
    void setCutoff(float cutoffNormalized)
    {
        cutoff = juce::jlimit(0.0f, 1.0f, cutoffNormalized);
        repaint();
    }
    
    void setResonance(float resonanceNormalized)
    {
        resonance = juce::jlimit(0.0f, 1.0f, resonanceNormalized);
        repaint();
    }
    
    void setFilterType(int type)
    {
        filterType = type; // 0 = LP, 1 = HP, 2 = BP
        repaint();
    }
    
    void setEnvAmount(float amount)
    {
        envAmount = juce::jlimit(-1.0f, 1.0f, amount);
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, DS::Radius::small);
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::small, 1.0f);
        
        if (bounds.getWidth() < 40 || bounds.getHeight() < 30)
            return;
        
        auto graphBounds = bounds.reduced(6);
        
        // Draw frequency grid
        g.setColour(DS::Color::textMuted.withAlpha(0.1f));
        for (int i = 1; i < 4; i++)
        {
            float x = graphBounds.getX() + (i / 4.0f) * graphBounds.getWidth();
            g.drawVerticalLine(static_cast<int>(x), graphBounds.getY(), graphBounds.getBottom());
        }
        
        // Draw filter curve
        juce::Path filterPath;
        float cutoffX = graphBounds.getX() + cutoff * graphBounds.getWidth();
        float baseY = graphBounds.getBottom();
        float topY = graphBounds.getY();
        float peakHeight = graphBounds.getHeight() * (0.3f + resonance * 0.5f);
        
        // Low-pass filter curve
        filterPath.startNewSubPath(graphBounds.getX(), topY + 4);
        
        // Flat response before cutoff
        filterPath.lineTo(cutoffX - 20, topY + 4);
        
        // Resonance peak
        if (resonance > 0.1f)
        {
            filterPath.quadraticTo(cutoffX, topY - peakHeight * 0.5f, cutoffX + 5, topY + 4);
        }
        else
        {
            filterPath.lineTo(cutoffX, topY + 4);
        }
        
        // Slope after cutoff
        float slopeEndX = juce::jmin(cutoffX + graphBounds.getWidth() * 0.3f, graphBounds.getRight());
        filterPath.quadraticTo(cutoffX + 30, baseY * 0.6f, slopeEndX, baseY - 4);
        
        // Continue to end
        filterPath.lineTo(graphBounds.getRight(), baseY - 4);
        
        // Fill under curve
        juce::Path fillPath = filterPath;
        fillPath.lineTo(graphBounds.getRight(), baseY);
        fillPath.lineTo(graphBounds.getX(), baseY);
        fillPath.closeSubPath();
        
        // Gradient fill
        juce::ColourGradient fillGradient(
            DS::Color::accent.withAlpha(0.3f),
            graphBounds.getX(), graphBounds.getY(),
            DS::Color::accent.withAlpha(0.05f),
            graphBounds.getX(), baseY,
            false
        );
        g.setGradientFill(fillGradient);
        g.fillPath(fillPath);
        
        // Draw curve line
        g.setColour(DS::Color::accent);
        g.strokePath(filterPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
        
        // Draw cutoff indicator
        g.setColour(DS::Color::accent.withAlpha(0.6f));
        g.drawVerticalLine(static_cast<int>(cutoffX), graphBounds.getY(), graphBounds.getBottom());
        
        // Cutoff marker
        g.setColour(DS::Color::accent);
        g.fillEllipse(cutoffX - 4, topY, 8, 8);
        
        // Labels
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText("20Hz", graphBounds.removeFromLeft(30).removeFromBottom(12), juce::Justification::left);
        g.drawText("20kHz", bounds.reduced(4).removeFromRight(30).removeFromBottom(12), juce::Justification::right);
    }
    
private:
    float cutoff = 0.5f;
    float resonance = 0.0f;
    int filterType = 0; // 0 = LP
    float envAmount = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterCurveDisplay)
};

} // namespace VisualFeedback
