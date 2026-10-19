/*
  ==============================================================================

    InteractiveEnvelopeEditor.h
    Draggable 7-Stage envelope editor (TODO: Full interactive support)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EnvelopeVisualizer.h"

class InteractiveEnvelopeEditor : public EnvelopeVisualizer
{
public:
    // TODO: Update for 7-Stage interactive editing
    enum DragPoint { NONE, TIME1, LEVEL1, TIME2, LEVEL2, TIME3, LEVEL3, TIME4 };

    InteractiveEnvelopeEditor();
    ~InteractiveEnvelopeEditor() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    std::function<void(const juce::String&, int)> onParameterChanged;

private:
    DragPoint draggedPoint = NONE;

    DragPoint findNearestPoint(juce::Point<int> pos) const;
    int xToTime(float x, float sectionWidth) const;
    int yToLevel(float y) const;
};
