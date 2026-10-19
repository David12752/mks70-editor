#include "AutomationLaneComponent.h"

using namespace juce;

AutomationLaneComponent::AutomationLaneComponent()
{
    setRepaintsOnMouseActivity(true);
}

void AutomationLaneComponent::setLane(AutomationRecorder::AutomationLane* l)
{
    lane = l;
    repaint();
}

void AutomationLaneComponent::setTimeRange(double start, double end)
{
    timeStart = start;
    timeEnd = jmax(start + 0.001, end);
    repaint();
}

void AutomationLaneComponent::setValueRange(int minValue, int maxValue)
{
    valueMin = jmin(minValue, maxValue);
    valueMax = jmax(minValue, maxValue);
    repaint();
}

void AutomationLaneComponent::setEditMode(EditMode mode)
{
    editMode = mode;
}

Point<float> AutomationLaneComponent::timeValueToXY(double time, int value) const
{
    auto r = getLocalBounds().toFloat();
    const double tNorm = juce::jlimit<double>(0.0, 1.0, (time - timeStart) / (timeEnd - timeStart));
    const double vNorm = juce::jlimit<double>(0.0, 1.0, ((double)value - (double)valueMin) / (double)(valueMax - valueMin));
    float x = r.getX() + (float) (tNorm * r.getWidth());
    float y = r.getBottom() - (float) (vNorm * r.getHeight());
    return { x, y };
}

std::pair<double, int> AutomationLaneComponent::xyToTimeValue(Point<float> pos) const
{
    auto r = getLocalBounds().toFloat();
    const double tNorm = juce::jlimit<double>(0.0, 1.0, (double)(pos.x - r.getX()) / (double)r.getWidth());
    const double vNorm = juce::jlimit<double>(0.0, 1.0, (double)(r.getBottom() - pos.y) / (double)r.getHeight());
    const double time = timeStart + tNorm * (timeEnd - timeStart);
    const int value = (int) std::round((double)valueMin + vNorm * (double)(valueMax - valueMin));
    return { time, value };
}

int AutomationLaneComponent::findNearestPointIndex(Point<float> xy, float radiusPx) const
{
    if (!lane) return -1;
    int bestIdx = -1;
    float bestDist2 = radiusPx * radiusPx;
    for (int i = 0; i < (int) lane->points.size(); ++i)
    {
        auto pxy = timeValueToXY(lane->points[i].timestamp, lane->points[i].value);
        auto dx = pxy.x - xy.x;
        auto dy = pxy.y - xy.y;
        float d2 = dx*dx + dy*dy;
        if (d2 <= bestDist2)
        {
            bestDist2 = d2;
            bestIdx = i;
        }
    }
    return bestIdx;
}

void AutomationLaneComponent::paint(Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    
    // Modern dark background
    g.setColour(Colour(0xFF1E293B));
    g.fillRoundedRectangle(r, 4.0f);

    // Background grid - subtle
    g.setColour(Colour(0xFF334155));
    for (int i = 0; i <= 4; ++i)
    {
        float y = r.getY() + r.getHeight() * (float)i / 4.0f;
        g.drawLine(r.getX() + 80, y, r.getRight() - 4, y, 0.5f);
    }

    // Parameter name label area (left side)
    auto labelArea = r.removeFromLeft(80);
    g.setColour(Colour(0xFF0F172A));
    g.fillRoundedRectangle(labelArea.reduced(2), 4.0f);
    
    if (lane)
    {
        // Parameter name
        g.setColour(lane->enabled ? Colour(0xFF22D3EE) : Colour(0xFF64748B));
        g.setFont(Font(11.0f, Font::bold));
        g.drawText(lane->paramId, labelArea.reduced(4), Justification::centred, true);
        
        // Armed indicator
        if (lane->armed)
        {
            g.setColour(Colour(0xFFEF4444));
            g.fillEllipse(labelArea.getX() + 4, labelArea.getY() + 4, 8, 8);
        }
        
        // Enable indicator
        g.setColour(lane->enabled ? Colour(0xFF22C55E) : Colour(0xFF475569));
        g.fillEllipse(labelArea.getRight() - 12, labelArea.getY() + 4, 8, 8);
    }

    // Border
    g.setColour(Colour(0xFF475569));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 4.0f, 1.0f);

    if (!lane || lane->points.empty())
    {
        // Empty lane message
        g.setColour(Colour(0xFF64748B));
        g.setFont(Font(10.0f));
        g.drawText("No automation data - use draw tool to add points", 
                   r.reduced(8), Justification::centred);
        return;
    }

    // Automation path with gradient fill
    Path path;
    Path fillPath;
    bool first = true;
    float firstX = 0, lastX = 0;
    
    for (const auto& pt : lane->points)
    {
        auto xy = timeValueToXY(pt.timestamp, pt.value);
        if (first) { 
            path.startNewSubPath(xy); 
            fillPath.startNewSubPath(xy.x, r.getBottom());
            fillPath.lineTo(xy);
            firstX = xy.x;
            first = false; 
        }
        else { 
            path.lineTo(xy); 
            fillPath.lineTo(xy);
        }
        lastX = xy.x;
    }
    
    // Complete fill path
    fillPath.lineTo(lastX, r.getBottom());
    fillPath.lineTo(firstX, r.getBottom());
    fillPath.closeSubPath();
    
    // Fill under curve
    Colour laneColor = lane->enabled ? Colour(0xFF6366F1) : Colour(0xFF64748B);
    g.setColour(laneColor.withAlpha(0.2f));
    g.fillPath(fillPath);
    
    // Stroke path
    g.setColour(laneColor);
    g.strokePath(path, PathStrokeType(2.0f));

    // Points with hover effect
    for (int i = 0; i < (int) lane->points.size(); ++i)
    {
        auto xy = timeValueToXY(lane->points[i].timestamp, lane->points[i].value);
        bool selected = std::find(selectedPoints.begin(), selectedPoints.end(), i) != selectedPoints.end();
        
        if (selected)
        {
            // Glow effect for selected
            g.setColour(Colour(0xFFFB923C).withAlpha(0.3f));
            g.fillEllipse(xy.x - 8.0f, xy.y - 8.0f, 16.0f, 16.0f);
            g.setColour(Colour(0xFFFB923C));
        }
        else
        {
            g.setColour(laneColor.brighter(0.3f));
        }
        g.fillEllipse(xy.x - 4.0f, xy.y - 4.0f, 8.0f, 8.0f);
        
        // Border
        g.setColour(Colours::white.withAlpha(0.5f));
        g.drawEllipse(xy.x - 4.0f, xy.y - 4.0f, 8.0f, 8.0f, 1.0f);
    }
}

void AutomationLaneComponent::mouseDown(const MouseEvent& e)
{
    if (!lane) return;

    auto tv = xyToTimeValue(e.position);

    if (editMode == DRAW)
    {
        AutomationRecorder::AutomationPoint p;
        p.timestamp = tv.first;
        p.value = juce::jlimit(0, 127, tv.second);
        p.paramId = lane->paramId;
        auto it = std::lower_bound(lane->points.begin(), lane->points.end(), p.timestamp,
            [](const AutomationRecorder::AutomationPoint& a, double t){ return a.timestamp < t; });
        lane->points.insert(it, p);
        repaint();
    }
    else if (editMode == ERASE)
    {
        int idx = findNearestPointIndex(e.position, 8.0f);
        if (idx >= 0)
        {
            lane->points.erase(lane->points.begin() + idx);
            repaint();
        }
    }
    else if (editMode == SELECT)
    {
        int idx = findNearestPointIndex(e.position, 8.0f);
        if (idx >= 0)
        {
            auto it = std::find(selectedPoints.begin(), selectedPoints.end(), idx);
            if (it == selectedPoints.end()) selectedPoints.push_back(idx);
            else selectedPoints.erase(it);
            repaint();
        }
    }
    else if (editMode == LINE)
    {
        if (!linePending)
        {
            linePending = true;
            lineStartXY = e.position;
            lineStartTV = tv;
        }
        else
        {
            linePending = false;
            auto tv2 = tv;
            const int segments = 32;
            const double t0 = jmin(lineStartTV.first, tv2.first);
            const double t1 = jmax(lineStartTV.first, tv2.first);
            const int v0 = lineStartTV.second;
            const int v1 = tv2.second;
            if (t1 > t0)
            {
                for (int i = 0; i <= segments; ++i)
                {
                    const double a = (double)i / (double)segments;
                    AutomationRecorder::AutomationPoint p;
                    p.timestamp = t0 + a * (t1 - t0);
                    p.value = roundToInt((1.0 - a) * v0 + a * v1);
                    p.value = jlimit(0, 127, p.value);
                    p.paramId = lane->paramId;
                    auto it = std::lower_bound(lane->points.begin(), lane->points.end(), p.timestamp,
                        [](const AutomationRecorder::AutomationPoint& ap, double t){ return ap.timestamp < t; });
                    lane->points.insert(it, p);
                }
                repaint();
            }
        }
    }
}

void AutomationLaneComponent::mouseDrag(const MouseEvent& e)
{
    if (!lane) return;
    if (editMode == DRAW)
    {
        auto tv = xyToTimeValue(e.position);
        AutomationRecorder::AutomationPoint p;
        p.timestamp = tv.first;
        p.value = juce::jlimit(0, 127, tv.second);
        p.paramId = lane->paramId;
        auto it = std::lower_bound(lane->points.begin(), lane->points.end(), p.timestamp,
            [](const AutomationRecorder::AutomationPoint& a, double t){ return a.timestamp < t; });
        lane->points.insert(it, p);
        repaint();
    }
    else if (editMode == ERASE)
    {
        int idx = findNearestPointIndex(e.position, 8.0f);
        if (idx >= 0)
        {
            lane->points.erase(lane->points.begin() + idx);
            repaint();
        }
    }
}
