/*
  ==============================================================================
    FocusMode.h
    Full-screen expansion for synth sections
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
 * FocusModeOverlay - Full-screen overlay that displays a focused section
 */
class FocusModeOverlay : public juce::Component
{
public:
    FocusModeOverlay()
    {
        setInterceptsMouseClicks(true, true);
        
        // Close button
        closeBtn.setButtonText(juce::CharPointer_UTF8("\xC3\x97"));  // × symbol
        closeBtn.onClick = [this]() { exitFocusMode(); };
        addAndMakeVisible(closeBtn);
        
        // Title
        titleLabel.setFont(DS::Font::heading());
        titleLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        titleLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(titleLabel);
        
        // Help text
        helpLabel.setText("Press ESC or click outside to exit focus mode", juce::dontSendNotification);
        helpLabel.setFont(DS::Font::small());
        helpLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        helpLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(helpLabel);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Semi-transparent background
        g.fillAll(juce::Colours::black.withAlpha(0.85f));
        
        // Content area background
        auto contentBounds = getContentBounds();
        g.setColour(DS::Color::bgPanel());
        g.fillRoundedRectangle(contentBounds.toFloat(), DS::Dim::cornerRadiusLarge());
        
        g.setColour(DS::Color::border());
        g.drawRoundedRectangle(contentBounds.toFloat(), DS::Dim::cornerRadiusLarge(), 2.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        auto headerBounds = getContentBounds().removeFromTop(40);
        
        titleLabel.setBounds(headerBounds.removeFromLeft(300).reduced(10, 5));
        closeBtn.setBounds(headerBounds.removeFromRight(40).reduced(5));
        
        helpLabel.setBounds(bounds.removeFromBottom(30));
        
        // Position the focused component
        if (focusedComponent != nullptr)
        {
            auto contentBounds = getContentBounds();
            contentBounds.removeFromTop(40);
            contentBounds.reduce(20, 20);
            focusedComponent->setBounds(contentBounds);
        }
    }
    
    juce::Rectangle<int> getContentBounds() const
    {
        auto bounds = getLocalBounds();
        int margin = 40;
        return bounds.reduced(margin);
    }
    
    //==========================================================================
    // Focus Mode Control
    //==========================================================================
    
    void enterFocusMode(juce::Component* componentToFocus, const juce::String& title)
    {
        if (componentToFocus == nullptr) return;
        
        // Store original parent and bounds
        originalParent = componentToFocus->getParentComponent();
        originalBounds = componentToFocus->getBounds();
        sectionTitle = title;
        
        titleLabel.setText(title + " - FOCUS MODE", juce::dontSendNotification);
        
        // Remove from original parent and add to overlay
        if (originalParent != nullptr)
            originalParent->removeChildComponent(componentToFocus);
            
        focusedComponent = componentToFocus;
        addAndMakeVisible(focusedComponent);
        
        // Show overlay
        setVisible(true);
        toFront(true);
        resized();
        
        if (onFocusModeEntered)
            onFocusModeEntered(title);
    }
    
    void exitFocusMode()
    {
        if (focusedComponent == nullptr) return;
        
        // Remove from overlay
        removeChildComponent(focusedComponent);
        
        // Return to original parent
        if (originalParent != nullptr)
        {
            originalParent->addAndMakeVisible(focusedComponent);
            focusedComponent->setBounds(originalBounds);
        }
        
        focusedComponent = nullptr;
        originalParent = nullptr;
        
        // Hide overlay
        setVisible(false);
        
        if (onFocusModeExited)
            onFocusModeExited();
    }
    
    bool isInFocusMode() const { return focusedComponent != nullptr; }
    
    //==========================================================================
    // Input Handling
    //==========================================================================
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        // Click outside content area to exit
        if (!getContentBounds().contains(e.getPosition()))
            exitFocusMode();
    }
    
    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::escapeKey)
        {
            exitFocusMode();
            return true;
        }
        return false;
    }
    
    std::function<void(const juce::String&)> onFocusModeEntered;
    std::function<void()> onFocusModeExited;
    
private:
    juce::Component* focusedComponent = nullptr;
    juce::Component* originalParent = nullptr;
    juce::Rectangle<int> originalBounds;
    juce::String sectionTitle;
    
    juce::TextButton closeBtn;
    juce::Label titleLabel;
    juce::Label helpLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FocusModeOverlay)
};

//==============================================================================
/**
 * FocusModeManager - Singleton manager for focus mode
 */
class FocusModeManager
{
public:
    static FocusModeManager& getInstance()
    {
        static FocusModeManager instance;
        return instance;
    }
    
    void setOverlay(FocusModeOverlay* overlay) { focusOverlay = overlay; }
    
    void enterFocusMode(juce::Component* component, const juce::String& title)
    {
        if (focusOverlay != nullptr)
            focusOverlay->enterFocusMode(component, title);
    }
    
    void exitFocusMode()
    {
        if (focusOverlay != nullptr)
            focusOverlay->exitFocusMode();
    }
    
    bool isInFocusMode() const 
    { 
        return focusOverlay != nullptr && focusOverlay->isInFocusMode(); 
    }
    
private:
    FocusModeManager() = default;
    FocusModeOverlay* focusOverlay = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FocusModeManager)
};

//==============================================================================
/**
 * FocusableSection - Wrapper that adds focus mode capability to any component
 * 
 * Usage: Add a FocusableHeader above your section component
 */
class FocusableHeader : public juce::Component
{
public:
    FocusableHeader(const juce::String& title, juce::Component* sectionToFocus)
        : sectionTitle(title), targetSection(sectionToFocus)
    {
        setTooltip("Double-click to enter focus mode");
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Header background
        g.setColour(isMouseOver() ? DS::Color::bgHover() : DS::Color::bgElevated());
        g.fillRoundedRectangle(bounds, DS::Dim::cornerRadius());
        
        // Title
        g.setColour(DS::Color::textPrimary());
        g.setFont(DS::Font::label().withStyle("Bold"));
        g.drawText(sectionTitle, bounds.reduced(8, 0), juce::Justification::centredLeft);
        
        // Focus mode hint icon
        if (isMouseOver())
        {
            g.setColour(DS::Color::textMuted());
            g.setFont(DS::Font::tiny());
            g.drawText(juce::CharPointer_UTF8("\xe2\x87\xb1"), // ⇱ expand icon
                      bounds.removeFromRight(24), juce::Justification::centred);
        }
    }
    
    void mouseDoubleClick(const juce::MouseEvent&) override
    {
        if (targetSection != nullptr)
            FocusModeManager::getInstance().enterFocusMode(targetSection, sectionTitle);
    }
    
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
    
    void setTargetSection(juce::Component* section) { targetSection = section; }
    
private:
    juce::String sectionTitle;
    juce::Component* targetSection = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FocusableHeader)
};
