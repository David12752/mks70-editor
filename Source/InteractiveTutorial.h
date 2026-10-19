#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * InteractiveTutorial.h
 * Onboarding wizard and interactive tutorial system
 * 
 * Part of Improvement #20: Interactive Tutorial
 */

namespace Tutorial
{

//==============================================================================
/**
 * Tutorial step definition
 */
struct TutorialStep
{
    juce::String title;
    juce::String description;
    juce::String highlightComponentId;  // Component to highlight
    juce::Rectangle<int> highlightArea;
    juce::String actionRequired;  // What user should do
    bool requiresAction = false;
    int nextStepDelay = 0;  // Auto-advance after ms (0 = manual)
};

//==============================================================================
/**
 * Tutorial Manager
 */
class TutorialManager
{
public:
    static TutorialManager& getInstance()
    {
        static TutorialManager instance;
        return instance;
    }
    
    void startTutorial(const juce::String& tutorialId)
    {
        auto it = tutorials.find(tutorialId);
        if (it != tutorials.end())
        {
            currentTutorial = &it->second;
            currentStep = 0;
            isRunning = true;
            
            if (onTutorialStarted)
                onTutorialStarted(tutorialId);
            
            showCurrentStep();
        }
    }
    
    void nextStep()
    {
        if (!isRunning || currentTutorial == nullptr)
            return;
        
        currentStep++;
        
        if (currentStep >= static_cast<int>(currentTutorial->size()))
        {
            endTutorial();
        }
        else
        {
            showCurrentStep();
        }
    }
    
    void previousStep()
    {
        if (!isRunning || currentStep <= 0)
            return;
        
        currentStep--;
        showCurrentStep();
    }
    
    void endTutorial()
    {
        isRunning = false;
        currentTutorial = nullptr;
        currentStep = 0;
        
        if (onTutorialEnded)
            onTutorialEnded();
    }
    
    void skipTutorial()
    {
        endTutorial();
    }
    
    bool getIsRunning() const { return isRunning; }
    int getCurrentStep() const { return currentStep; }
    int getTotalSteps() const { return currentTutorial ? static_cast<int>(currentTutorial->size()) : 0; }
    
    const TutorialStep* getCurrentStepData() const
    {
        if (currentTutorial && currentStep < static_cast<int>(currentTutorial->size()))
            return &(*currentTutorial)[currentStep];
        return nullptr;
    }
    
    void registerTutorial(const juce::String& id, const std::vector<TutorialStep>& steps)
    {
        tutorials[id] = steps;
    }
    
    void registerDefaultTutorials()
    {
        // Getting Started tutorial
        std::vector<TutorialStep> gettingStarted;
        
        gettingStarted.push_back({
            "Welcome to MKS-70 Editor!",
            "This editor lets you control your Roland MKS-70 synthesizer with a modern interface. "
            "Let's take a quick tour of the main features.",
            "", {}, "", false, 0
        });
        
        gettingStarted.push_back({
            "Header Bar",
            "The header bar shows the current patch name and provides quick access to save, load, "
            "and sync functions. You can also switch between compare slots A/B/C/D here.",
            "headerBar", {}, "", false, 0
        });
        
        gettingStarted.push_back({
            "Tone Tabs",
            "Use the Upper Tone and Lower Tone tabs to edit the two tone generators. "
            "The MKS-70 is a dual-tone synth, allowing for rich layered sounds.",
            "toneUpperTab", {}, "Click on 'TONE UPPER' tab", true, 0
        });
        
        gettingStarted.push_back({
            "DCO Section",
            "The DCO (Digitally Controlled Oscillator) section contains the sound sources. "
            "Adjust waveforms, pitch, and modulation here.",
            "dcoSection", {}, "", false, 0
        });
        
        gettingStarted.push_back({
            "VCF Section",
            "The VCF (Voltage Controlled Filter) shapes the tone. "
            "Cutoff and resonance are the main controls for sculpting your sound.",
            "vcfSection", {}, "", false, 0
        });
        
        gettingStarted.push_back({
            "Envelope Section",
            "Envelopes control how parameters change over time. "
            "ENV1 typically controls the filter, ENV2 controls the amplifier.",
            "envSection", {}, "", false, 0
        });
        
        gettingStarted.push_back({
            "Librarian",
            "The Librarian tab lets you browse, organize, and manage your presets. "
            "You can tag favorites, search, and organize into banks.",
            "librarianTab", {}, "", false, 0
        });
        
        gettingStarted.push_back({
            "MIDI Setup",
            "Make sure your MKS-70 is connected and the MIDI ports are configured in Settings. "
            "The editor will sync changes in real-time.",
            "settingsTab", {}, "", false, 0
        });
        
        gettingStarted.push_back({
            "You're Ready!",
            "That's the basics! Explore the interface, try the randomizer, "
            "and don't forget you can use Ctrl+Z to undo any changes. Have fun!",
            "", {}, "", false, 0
        });
        
        tutorials["getting_started"] = gettingStarted;
        
        // Sound Design tutorial
        std::vector<TutorialStep> soundDesign;
        
        soundDesign.push_back({
            "Sound Design Basics",
            "Let's learn how to create sounds on the MKS-70. "
            "We'll cover the signal flow and key parameters.",
            "", {}, "", false, 0
        });
        
        soundDesign.push_back({
            "Start with DCO1",
            "Select a waveform for DCO1. Sawtooth is great for pads and leads, "
            "Square/Pulse for hollow or woody sounds.",
            "dco1Waveform", {}, "Select SAW waveform", true, 0
        });
        
        soundDesign.push_back({
            "Add DCO2",
            "DCO2 adds thickness. Try detuning it slightly from DCO1 for a wider sound.",
            "dco2Detune", {}, "Adjust the detune", true, 0
        });
        
        soundDesign.push_back({
            "Shape with Filter",
            "Lower the cutoff and raise resonance to create movement. "
            "Add envelope modulation for dynamic sounds.",
            "vcfCutoff", {}, "Lower the cutoff", true, 0
        });
        
        soundDesign.push_back({
            "Animate with LFO",
            "Use the LFO to add vibrato, filter wobble, or PWM. "
            "Adjust rate and delay to taste.",
            "lfoRate", {}, "", false, 0
        });
        
        tutorials["sound_design"] = soundDesign;
    }
    
    std::function<void(const juce::String&)> onTutorialStarted;
    std::function<void()> onTutorialEnded;
    std::function<void(const TutorialStep&)> onStepChanged;
    
private:
    TutorialManager() { registerDefaultTutorials(); }
    
    void showCurrentStep()
    {
        if (currentTutorial && currentStep < static_cast<int>(currentTutorial->size()))
        {
            if (onStepChanged)
                onStepChanged((*currentTutorial)[currentStep]);
        }
    }
    
    std::map<juce::String, std::vector<TutorialStep>> tutorials;
    std::vector<TutorialStep>* currentTutorial = nullptr;
    int currentStep = 0;
    bool isRunning = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TutorialManager)
};

//==============================================================================
/**
 * Tutorial Overlay - Shows current step with highlighting
 */
class TutorialOverlay : public juce::Component
{
public:
    TutorialOverlay()
    {
        setInterceptsMouseClicks(true, true);
        
        nextButton.setButtonText("Next >");
        nextButton.onClick = [] { TutorialManager::getInstance().nextStep(); };
        addAndMakeVisible(nextButton);
        
        prevButton.setButtonText("< Back");
        prevButton.onClick = [] { TutorialManager::getInstance().previousStep(); };
        addAndMakeVisible(prevButton);
        
        skipButton.setButtonText("Skip Tutorial");
        skipButton.onClick = [] { TutorialManager::getInstance().skipTutorial(); };
        addAndMakeVisible(skipButton);
    }
    
    void showStep(const TutorialStep& step)
    {
        currentStep = step;
        setVisible(true);
        repaint();
    }
    
    void hide()
    {
        setVisible(false);
    }
    
    void setHighlightArea(juce::Rectangle<int> area)
    {
        highlightRect = area;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        // Semi-transparent overlay
        g.fillAll(juce::Colour(0xCC000000));
        
        // Cut out highlight area
        if (!highlightRect.isEmpty())
        {
            g.setColour(juce::Colours::transparentBlack);
            g.reduceClipRegion(highlightRect);
            g.fillAll(juce::Colours::white);
            
            g.resetToDefaultState();
            
            // Highlight border with glow
            g.setColour(DS::Color::accent.withAlpha(0.5f));
            g.drawRect(highlightRect.expanded(4), 4);
            g.setColour(DS::Color::accent);
            g.drawRect(highlightRect.expanded(2), 2);
        }
        
        // Tooltip box
        auto tooltipBounds = calculateTooltipBounds();
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(tooltipBounds.toFloat(), DS::Radius::large);
        
        g.setColour(DS::Color::accent);
        g.drawRoundedRectangle(tooltipBounds.toFloat(), DS::Radius::large, 2.0f);
        
        auto contentBounds = tooltipBounds.reduced(DS::Space::md);
        
        // Progress indicator
        auto& manager = TutorialManager::getInstance();
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText("Step " + juce::String(manager.getCurrentStep() + 1) + 
                   " of " + juce::String(manager.getTotalSteps()),
                   contentBounds.removeFromTop(16), juce::Justification::centredLeft);
        
        contentBounds.removeFromTop(DS::Space::sm);
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading.withHeight(20.0f));
        g.drawText(currentStep.title, contentBounds.removeFromTop(28), juce::Justification::centredLeft);
        
        contentBounds.removeFromTop(DS::Space::sm);
        
        // Description
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::body);
        g.drawFittedText(currentStep.description, contentBounds.removeFromTop(80),
                         juce::Justification::topLeft, 4);
        
        // Action hint
        if (currentStep.actionRequired.isNotEmpty())
        {
            contentBounds.removeFromTop(DS::Space::sm);
            g.setColour(DS::Color::accent);
            g.setFont(DS::Font::body.italicised());
            g.drawText("→ " + currentStep.actionRequired, contentBounds.removeFromTop(20),
                      juce::Justification::centredLeft);
        }
    }
    
    void resized() override
    {
        auto tooltipBounds = calculateTooltipBounds();
        auto buttonArea = tooltipBounds.reduced(DS::Space::md).removeFromBottom(35);
        
        skipButton.setBounds(buttonArea.removeFromLeft(100));
        nextButton.setBounds(buttonArea.removeFromRight(80));
        prevButton.setBounds(buttonArea.removeFromRight(80));
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        // Click outside tooltip to advance (if not requiring action)
        auto tooltipBounds = calculateTooltipBounds();
        if (!tooltipBounds.contains(e.getPosition()) && !currentStep.requiresAction)
        {
            TutorialManager::getInstance().nextStep();
        }
    }
    
private:
    juce::Rectangle<int> calculateTooltipBounds()
    {
        int tooltipWidth = 400;
        int tooltipHeight = 220;
        
        auto bounds = getLocalBounds();
        
        // Position relative to highlight area
        if (!highlightRect.isEmpty())
        {
            // Try below
            if (highlightRect.getBottom() + tooltipHeight + 20 < bounds.getHeight())
            {
                return juce::Rectangle<int>(
                    juce::jlimit(20, bounds.getWidth() - tooltipWidth - 20, highlightRect.getCentreX() - tooltipWidth / 2),
                    highlightRect.getBottom() + 20,
                    tooltipWidth, tooltipHeight);
            }
            // Try above
            else if (highlightRect.getY() - tooltipHeight - 20 > 0)
            {
                return juce::Rectangle<int>(
                    juce::jlimit(20, bounds.getWidth() - tooltipWidth - 20, highlightRect.getCentreX() - tooltipWidth / 2),
                    highlightRect.getY() - tooltipHeight - 20,
                    tooltipWidth, tooltipHeight);
            }
        }
        
        // Center
        return bounds.withSizeKeepingCentre(tooltipWidth, tooltipHeight);
    }
    
    TutorialStep currentStep;
    juce::Rectangle<int> highlightRect;
    
    juce::TextButton nextButton;
    juce::TextButton prevButton;
    juce::TextButton skipButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TutorialOverlay)
};

//==============================================================================
/**
 * Welcome Dialog - First-run experience
 */
class WelcomeDialog : public juce::Component
{
public:
    WelcomeDialog()
    {
        startTutorialButton.setButtonText("Start Tutorial");
        startTutorialButton.onClick = [this]
        {
            TutorialManager::getInstance().startTutorial("getting_started");
            if (onClose)
                onClose();
        };
        addAndMakeVisible(startTutorialButton);
        
        skipButton.setButtonText("Skip, I know what I'm doing");
        skipButton.onClick = [this]
        {
            if (onClose)
                onClose();
        };
        addAndMakeVisible(skipButton);
        
        dontShowAgain.setButtonText("Don't show this again");
        addAndMakeVisible(dontShowAgain);
    }
    
    std::function<void()> onClose;
    
    bool getDontShowAgain() const { return dontShowAgain.getToggleState(); }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.fillAll(juce::Colour(0xEE000000));
        
        auto dialogBounds = bounds.withSizeKeepingCentre(500, 400);
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(dialogBounds, DS::Radius::large);
        
        g.setColour(DS::Color::accent);
        g.drawRoundedRectangle(dialogBounds, DS::Radius::large, 2.0f);
        
        auto contentBounds = dialogBounds.reduced(DS::Space::lg);
        
        // Logo/Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading.withHeight(32.0f));
        g.drawText("Welcome to", contentBounds.removeFromTop(40), juce::Justification::centred);
        
        g.setColour(DS::Color::accent);
        g.setFont(DS::Font::heading.withHeight(48.0f));
        g.drawText("MKS-70 Editor", contentBounds.removeFromTop(60), juce::Justification::centred);
        
        contentBounds.removeFromTop(DS::Space::lg);
        
        // Description
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::body.withHeight(16.0f));
        g.drawFittedText(
            "A professional editor for the Roland MKS-70 synthesizer.\n\n"
            "Would you like a quick tour of the interface?",
            contentBounds.removeFromTop(100),
            juce::Justification::centred, 4);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        auto dialogBounds = bounds.withSizeKeepingCentre(500, 400);
        auto contentBounds = dialogBounds.reduced(DS::Space::lg);
        
        // Position buttons at bottom
        auto buttonArea = contentBounds.removeFromBottom(100);
        
        dontShowAgain.setBounds(buttonArea.removeFromBottom(24).withSizeKeepingCentre(200, 24));
        buttonArea.removeFromBottom(DS::Space::md);
        
        auto buttonRow = buttonArea.removeFromBottom(40);
        int buttonWidth = 180;
        int spacing = 20;
        
        startTutorialButton.setBounds(buttonRow.withSizeKeepingCentre(buttonWidth * 2 + spacing, 40)
                                           .removeFromLeft(buttonWidth));
        skipButton.setBounds(buttonRow.withSizeKeepingCentre(buttonWidth * 2 + spacing, 40)
                                  .removeFromRight(buttonWidth));
    }
    
private:
    juce::TextButton startTutorialButton;
    juce::TextButton skipButton;
    juce::ToggleButton dontShowAgain;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WelcomeDialog)
};

} // namespace Tutorial
