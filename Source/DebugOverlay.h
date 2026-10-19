#pragma once
#include <JuceHeader.h>

class DebugOverlay : public juce::Component {
public:
    inline static bool showGrid = false;
    inline static bool showBounds = false;
    inline static bool showSpacing = false;

    static void paintDebug(juce::Graphics& g, juce::Component* comp)
    {
        if (!showGrid && !showBounds)
            return;

        if (showBounds) {
            g.setColour(juce::Colours::red.withAlpha(0.3f));
            g.drawRect(comp->getLocalBounds(), 1);
        }

        if (showGrid) {
            g.setColour(juce::Colours::blue.withAlpha(0.1f));
            for (int x = 0; x < comp->getWidth(); x += 8)
                g.drawVerticalLine(x, 0.0f, (float) comp->getHeight());
            for (int y = 0; y < comp->getHeight(); y += 8)
                g.drawHorizontalLine(y, 0.0f, (float) comp->getWidth());
        }
    }
};
