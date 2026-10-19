/*
  ==============================================================================

    PatchNavigator.h
    Quick patch navigation component with +/- buttons

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
    PatchNavigator - Component with +/- buttons for quick patch navigation
*/
class PatchNavigator : public juce::Component
{
public:
    PatchNavigator()
    {
        // Previous button
        prevButton.setButtonText("<");
        prevButton.onClick = [this]() { 
            if (onPatchChange)
                onPatchChange(-1); 
        };
        addAndMakeVisible(prevButton);
        
        // Next button
        nextButton.setButtonText(">");
        nextButton.onClick = [this]() { 
            if (onPatchChange)
                onPatchChange(1); 
        };
        addAndMakeVisible(nextButton);
        
        // Patch number display
        patchDisplay.setJustificationType(juce::Justification::centred);
        patchDisplay.setFont(juce::Font(18.0f).boldened());
        patchDisplay.setColour(juce::Label::textColourId, DS::Colors::text);
        addAndMakeVisible(patchDisplay);
        
        // Bank display
        bankDisplay.setJustificationType(juce::Justification::centred);
        bankDisplay.setFont(juce::Font(10.0f));
        bankDisplay.setColour(juce::Label::textColourId, DS::Colors::textMuted);
        addAndMakeVisible(bankDisplay);
        
        setSize(180, 50);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(bounds, 6.0f);
        
        // Border
        g.setColour(DS::Colors::border);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(4);
        
        auto buttonWidth = 35;
        
        prevButton.setBounds(bounds.removeFromLeft(buttonWidth));
        nextButton.setBounds(bounds.removeFromRight(buttonWidth));
        
        auto displayArea = bounds.reduced(5, 0);
        bankDisplay.setBounds(displayArea.removeFromTop(14));
        patchDisplay.setBounds(displayArea);
    }
    
    void setCurrentPatch(int bank, int patch, const juce::String& name = "")
    {
        currentBank = bank;
        currentPatch = patch;
        
        juce::String patchStr = juce::String(bank) + "-" + juce::String(patch + 1).paddedLeft('0', 2);
        patchDisplay.setText(patchStr, juce::dontSendNotification);
        
        juce::String bankName = "Bank " + juce::String(bank);
        bankDisplay.setText(bankName, juce::dontSendNotification);
    }
    
    void setRange(int minPatch, int maxPatch, int minBank = 1, int maxBank = 8)
    {
        this->minPatch = minPatch;
        this->maxPatch = maxPatch;
        this->minBank = minBank;
        this->maxBank = maxBank;
    }
    
    // Callback when patch changes: delta is -1 for prev, +1 for next
    std::function<void(int delta)> onPatchChange;
    
    // Full callback with bank and patch
    std::function<void(int bank, int patch)> onPatchSelected;
    
private:
    juce::TextButton prevButton;
    juce::TextButton nextButton;
    juce::Label patchDisplay;
    juce::Label bankDisplay;
    
    int currentBank = 1;
    int currentPatch = 0;
    int minPatch = 0;
    int maxPatch = 63;
    int minBank = 1;
    int maxBank = 8;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatchNavigator)
};

//==============================================================================
/**
    PatchIncrementButton - Stylized +/- button for patch navigation
*/
class PatchIncrementButton : public juce::Button
{
public:
    enum Direction { Decrement, Increment };
    
    PatchIncrementButton(Direction dir) : Button(""), direction(dir)
    {
        setSize(30, 30);
    }
    
    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        juce::Colour bgColour = DS::Colors::surface;
        if (isButtonDown)
            bgColour = DS::Colors::accent.withAlpha(0.3f);
        else if (isMouseOver)
            bgColour = DS::Colors::surface.brighter(0.1f);
        
        g.setColour(bgColour);
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Border
        g.setColour(isMouseOver ? DS::Colors::accent : DS::Colors::border);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        
        // Arrow symbol
        g.setColour(isMouseOver ? DS::Colors::accent : DS::Colors::text);
        
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float size = 8.0f;
        
        juce::Path arrow;
        if (direction == Decrement)
        {
            // Left arrow / minus
            arrow.startNewSubPath(cx + size * 0.4f, cy - size * 0.5f);
            arrow.lineTo(cx - size * 0.4f, cy);
            arrow.lineTo(cx + size * 0.4f, cy + size * 0.5f);
        }
        else
        {
            // Right arrow / plus
            arrow.startNewSubPath(cx - size * 0.4f, cy - size * 0.5f);
            arrow.lineTo(cx + size * 0.4f, cy);
            arrow.lineTo(cx - size * 0.4f, cy + size * 0.5f);
        }
        
        g.strokePath(arrow, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }
    
private:
    Direction direction;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatchIncrementButton)
};

//==============================================================================
/**
    PatchStepper - Compact stepper control for patch/bank selection
*/
class PatchStepper : public juce::Component
{
public:
    PatchStepper(const juce::String& labelText = "Patch")
    {
        // Label
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(10.0f));
        label.setColour(juce::Label::textColourId, DS::Colors::textMuted);
        addAndMakeVisible(label);
        
        // Value display
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.setFont(juce::Font(16.0f).boldened());
        valueLabel.setColour(juce::Label::textColourId, DS::Colors::text);
        valueLabel.setEditable(false, true, false);  // Double-click to edit
        valueLabel.onTextChange = [this]() { 
            int newVal = valueLabel.getText().getIntValue();
            setValue(newVal);
        };
        addAndMakeVisible(valueLabel);
        
        // Decrement button
        decButton = std::make_unique<PatchIncrementButton>(PatchIncrementButton::Decrement);
        decButton->onClick = [this]() { 
            setValue(currentValue - 1);
            if (onValueChanged)
                onValueChanged(currentValue);
        };
        addAndMakeVisible(decButton.get());
        
        // Increment button
        incButton = std::make_unique<PatchIncrementButton>(PatchIncrementButton::Increment);
        incButton->onClick = [this]() { 
            setValue(currentValue + 1);
            if (onValueChanged)
                onValueChanged(currentValue);
        };
        addAndMakeVisible(incButton.get());
        
        setSize(120, 45);
        setValue(1);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Subtle background
        g.setColour(juce::Colour(0xff1e1e1e));
        g.fillRoundedRectangle(bounds, 4.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(2);
        
        label.setBounds(bounds.removeFromTop(14));
        
        auto controlArea = bounds;
        auto buttonWidth = 28;
        
        decButton->setBounds(controlArea.removeFromLeft(buttonWidth).reduced(0, 2));
        incButton->setBounds(controlArea.removeFromRight(buttonWidth).reduced(0, 2));
        valueLabel.setBounds(controlArea);
    }
    
    void setRange(int min, int max)
    {
        minValue = min;
        maxValue = max;
        setValue(currentValue);  // Re-clamp
    }
    
    void setValue(int value)
    {
        currentValue = juce::jlimit(minValue, maxValue, value);
        valueLabel.setText(juce::String(currentValue), juce::dontSendNotification);
        
        // Update button states
        decButton->setEnabled(currentValue > minValue);
        incButton->setEnabled(currentValue < maxValue);
    }
    
    int getValue() const { return currentValue; }
    
    std::function<void(int)> onValueChanged;
    
private:
    juce::Label label;
    juce::Label valueLabel;
    std::unique_ptr<PatchIncrementButton> decButton;
    std::unique_ptr<PatchIncrementButton> incButton;
    
    int currentValue = 1;
    int minValue = 1;
    int maxValue = 64;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatchStepper)
};
