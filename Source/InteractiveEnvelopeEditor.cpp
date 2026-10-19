/*
  ==============================================================================

    InteractiveEnvelopeEditor.cpp
    Interactive editing disabled until full 7-Stage support implemented

  ==============================================================================
*/

#include "InteractiveEnvelopeEditor.h"

InteractiveEnvelopeEditor::InteractiveEnvelopeEditor() {}
InteractiveEnvelopeEditor::~InteractiveEnvelopeEditor() {}

InteractiveEnvelopeEditor::DragPoint InteractiveEnvelopeEditor::findNearestPoint(juce::Point<int> pos) const
{
    // TODO: Implement 7-Stage point detection
    return NONE;
}

int InteractiveEnvelopeEditor::xToTime(float x, float sectionWidth) const
{
    if (sectionWidth <= 0.0f) return 0;
    float v = juce::jlimit(0.0f, sectionWidth, x);
    return (int)juce::jlimit(0.0f, 127.0f, (v / sectionWidth) * 127.0f);
}

int InteractiveEnvelopeEditor::yToLevel(float y) const
{
    float h = (float)getHeight();
    float clampedY = juce::jlimit(0.0f, h, y);
    float t = juce::jmap(clampedY, 4.0f, h - 4.0f, 1.0f, 0.0f);
    return (int)juce::jlimit(0.0f, 127.0f, t * 127.0f);
}

void InteractiveEnvelopeEditor::mouseDown(const juce::MouseEvent& e)
{
    // TODO: Enable interactive editing for 7-Stage
    draggedPoint = NONE;
}

void InteractiveEnvelopeEditor::mouseDrag(const juce::MouseEvent& e)
{
    // TODO: Implement 7-Stage dragging
}

void InteractiveEnvelopeEditor::mouseUp(const juce::MouseEvent&)
{
    draggedPoint = NONE;
}
