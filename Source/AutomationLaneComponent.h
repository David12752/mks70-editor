#pragma once

#include <JuceHeader.h>
#include "AutomationRecorder.h"

class AutomationLaneComponent : public juce::Component
{
public:
    AutomationLaneComponent();

    void setLane(AutomationRecorder::AutomationLane* lane);
    void setTimeRange(double start, double end);
    void setValueRange(int minValue, int maxValue);

    enum EditMode { DRAW, LINE, SELECT, ERASE };
    void setEditMode(EditMode mode);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    AutomationRecorder::AutomationLane* lane = nullptr;
    std::vector<int> selectedPoints;
    EditMode editMode = DRAW;

    double timeStart = 0.0;
    double timeEnd = 60.0;
    int valueMin = 0;
    int valueMax = 127;

    bool linePending = false;
    juce::Point<float> lineStartXY{};
    std::pair<double,int> lineStartTV{0.0, 0};

    juce::Point<float> timeValueToXY(double time, int value) const;
    std::pair<double, int> xyToTimeValue(juce::Point<float> pos) const;
    int findNearestPointIndex(juce::Point<float> xy, float radiusPx) const;
};
