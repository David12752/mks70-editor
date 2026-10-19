#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

class ScrollablePanel : public juce::Component {
public:
    ScrollablePanel()
    {
        viewport.setViewedComponent(&contentContainer, false);
        viewport.setScrollBarsShown(true, false); // Vertical only
        viewport.setScrollBarThickness(10);
        addAndMakeVisible(viewport);
    }

    void setContent(std::unique_ptr<juce::Component> content)
    {
        contentComponent = std::move(content);
        contentContainer.removeAllChildren();
        if (contentComponent)
        {
            contentContainer.addAndMakeVisible(contentComponent.get());
            updateContentSize();
        }
    }

    void resized() override
    {
        viewport.setBounds(getLocalBounds());
        updateContentSize();
    }

    void updateContentSize()
    {
        if (contentComponent)
        {
            const int contentWidth = viewport.getLocalBounds().getWidth();
            const int contentHeight = calculateContentHeight();

            contentContainer.setSize(contentWidth, contentHeight);
            contentComponent->setBounds(0, 0, contentWidth, contentHeight);
        }
    }

    juce::Viewport& getViewport() { return viewport; }

protected:
    virtual int calculateContentHeight()
    {
        if (contentComponent)
            return juce::jmax(600, contentComponent->getHeight());
        return 600;
    }

private:
    juce::Viewport viewport;
    juce::Component contentContainer;
    std::unique_ptr<juce::Component> contentComponent;
};
