/*
  ==============================================================================

    EnvelopeVisualizer.cpp
    7-Stage envelope visualization for MKS-70 Vecoven 3.x

  ==============================================================================
*/

#include "EnvelopeVisualizer.h"

EnvelopeVisualizer::EnvelopeVisualizer() {}
EnvelopeVisualizer::~EnvelopeVisualizer() { stopAnimation(); }

void EnvelopeVisualizer::setEnvelope7Stage(int t1, int l1, int t2, int l2, 
                                            int t3, int l3, int t4)
{
    time1 = juce::jlimit(0, 127, t1);
    level1 = juce::jlimit(0, 127, l1);
    time2 = juce::jlimit(0, 127, t2);
    level2 = juce::jlimit(0, 127, l2);
    time3 = juce::jlimit(0, 127, t3);
    level3 = juce::jlimit(0, 127, l3);
    time4 = juce::jlimit(0, 127, t4);
    repaint();
}

// Legacy ADSR support - map to simplified 7-Stage
void EnvelopeVisualizer::setEnvelope(int attack, int decay, int sustain, int release)
{
    // Map ADSR to 7-Stage:
    // Attack -> Time1 to Level1(127)
    // Decay -> Time2 to Level2(sustain)
    // Sustain -> keep Level2
    // Release -> Time4 to 0
    time1 = juce::jlimit(0, 127, attack);
    level1 = 127;
    time2 = juce::jlimit(0, 127, decay);
    level2 = juce::jlimit(0, 127, sustain);
    time3 = 0;  // No time3
    level3 = level2;  // Same as level2
    time4 = juce::jlimit(0, 127, release);
    repaint();
}

void EnvelopeVisualizer::setKeyFollow(int k)
{
    keyFollow = juce::jlimit(0, 127, k);
    repaint();
}

void EnvelopeVisualizer::triggerEnvelope()
{
    isAnimating = true;
    playheadX = 0.0f;
    startTimerHz(60);
    repaint();
}

void EnvelopeVisualizer::releaseEnvelope()
{
    // Keep it simple: allow playhead to continue moving
}

void EnvelopeVisualizer::stopAnimation()
{
    isAnimating = false;
    stopTimer();
    repaint();
}

void EnvelopeVisualizer::setShowGrid(bool show) { showGrid = show; repaint(); }
void EnvelopeVisualizer::setShowValues(bool show) { showValues = show; repaint(); }
void EnvelopeVisualizer::setShowPlayhead(bool show) { showPlayhead = show; repaint(); }

float EnvelopeVisualizer::timeToX(int time, float sectionWidth) const
{
    return sectionWidth * (juce::jlimit(0, 127, time) / 127.0f);
}

float EnvelopeVisualizer::levelToY(int level) const
{
    const float h = (float)getHeight();
    float t = juce::jlimit(0, 127, level) / 127.0f;
    return juce::jmap(1.0f - t, 0.0f, 1.0f, 4.0f, h - 4.0f);
}

void EnvelopeVisualizer::drawGrid(juce::Graphics& g) const
{
    g.setColour(juce::Colours::darkgrey.withAlpha(0.6f));
    auto w = getWidth();
    auto h = getHeight();
    for (int i = 1; i < 6; ++i)
    {
        int x = (w * i) / 6;
        g.drawVerticalLine(x, 0.0f, (float)h);
    }
    for (int j = 1; j < 4; ++j)
    {
        int y = (h * j) / 4;
        g.drawHorizontalLine(y, 0.0f, (float)w);
    }
}

void EnvelopeVisualizer::drawValues(juce::Graphics& g) const
{
    g.setColour(juce::Colours::lightgrey);
    g.setFont(juce::FontOptions(10.0f));
    juce::String text = "T1:" + juce::String(time1) + " L1:" + juce::String(level1) +
                        " T2:" + juce::String(time2) + " L2:" + juce::String(level2) +
                        " T3:" + juce::String(time3) + " L3:" + juce::String(level3) +
                        " T4:" + juce::String(time4);
    g.drawText(text, getLocalBounds().reduced(4, 2), juce::Justification::bottomLeft);
}

void EnvelopeVisualizer::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    if (showGrid) drawGrid(g);

    // Draw 7-Stage envelope
    juce::Path envPath;
    const float h = (float)getHeight();
    const float w = (float)getWidth();

    // Normalize segment widths to fill the available width
    const int totalSteps = juce::jmax(1, time1 + time2 + time3 + time4);
    const float leftPad = 4.0f;
    const float rightPad = 4.0f;
    const float usableW = juce::jmax(1.0f, w - (leftPad + rightPad));
    const float pixelsPerStep = usableW / (float) totalSteps;

    float x = leftPad;
    envPath.startNewSubPath(x, h); // Start at bottom

    // Time1 -> Level1
    x += pixelsPerStep * juce::jlimit(0, 127, time1);
    float y1 = levelToY(level1);
    envPath.lineTo(x, y1);

    // Time2 -> Level2
    x += pixelsPerStep * juce::jlimit(0, 127, time2);
    float y2 = levelToY(level2);
    envPath.lineTo(x, y2);

    // Time3 -> Level3
    x += pixelsPerStep * juce::jlimit(0, 127, time3);
    float y3 = levelToY(level3);
    envPath.lineTo(x, y3);

    // Time4 -> 0 (release)
    x += pixelsPerStep * juce::jlimit(0, 127, time4);
    envPath.lineTo(juce::jmin(x, w - rightPad), h);

    g.setColour(juce::Colours::orange.withAlpha(0.3f));
    g.fillPath(envPath);
    g.setColour(juce::Colours::orange);
    g.strokePath(envPath, juce::PathStrokeType(2.0f));

    if (showPlayhead && isAnimating)
    {
        g.setColour(juce::Colours::white);
        g.drawVerticalLine((int)playheadX, 0.0f, (float)getHeight());
    }

    if (showValues) drawValues(g);
}

void EnvelopeVisualizer::timerCallback()
{
    playheadX += 2.0f;
    if (playheadX > (float)getWidth())
    {
        stopAnimation();
        return;
    }
    repaint();
}
