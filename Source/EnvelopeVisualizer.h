/*
  ==============================================================================

    EnvelopeVisualizer.h
    7-Stage envelope visualizer with optional animation
    (MKS-70 Vecoven 3.x: Time1, Level1, Time2, Level2, Time3, Level3, Time4)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class EnvelopeVisualizer : public juce::Component, private juce::Timer
{
public:
    EnvelopeVisualizer();
    ~EnvelopeVisualizer() override;

    void paint(juce::Graphics& g) override;

    // 7-Stage envelope
    void setEnvelope7Stage(int time1, int level1, int time2, int level2, 
                           int time3, int level3, int time4);
    
    // Legacy ADSR support (maps to simplified 7-Stage)
    void setEnvelope(int attack, int decay, int sustain, int release);
    
    void setKeyFollow(int keyFollow);

    void triggerEnvelope();
    void releaseEnvelope();
    void stopAnimation();

    void setShowGrid(bool show);
    void setShowValues(bool show);
    void setShowPlayhead(bool show);

protected:
    // Geometry helpers for 7-Stage
    float timeToX(int time, float sectionWidth) const;
    float levelToY(int level) const;

    void drawGrid(juce::Graphics& g) const;
    void drawValues(juce::Graphics& g) const;

    // Animation
    void timerCallback() override;

    // 7-Stage envelope parameters
    int time1 = 0, level1 = 127, time2 = 64, level2 = 100;
    int time3 = 64, level3 = 80, time4 = 64;
    int keyFollow = 0;
    
    bool showGrid = true;
    bool showValues = true;
    bool showPlayhead = true;
    bool isAnimating = false;
    float playheadX = 0.0f;
};
