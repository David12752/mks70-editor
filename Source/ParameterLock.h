/*
  ==============================================================================

    ParameterLock.h
    System for locking individual parameters from modification

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
    ParameterLockManager - Singleton to track locked parameters
*/
class ParameterLockManager
{
public:
    static ParameterLockManager& getInstance()
    {
        static ParameterLockManager instance;
        return instance;
    }
    
    // Check if a parameter is locked
    bool isLocked(const juce::String& paramId) const
    {
        return lockedParams.count(paramId) > 0;
    }
    
    // Toggle lock state
    void toggleLock(const juce::String& paramId)
    {
        if (lockedParams.count(paramId) > 0)
            lockedParams.erase(paramId);
        else
            lockedParams.insert(paramId);
            
        if (onLockChanged)
            onLockChanged(paramId);
    }
    
    // Set lock state
    void setLocked(const juce::String& paramId, bool locked)
    {
        if (locked)
            lockedParams.insert(paramId);
        else
            lockedParams.erase(paramId);
            
        if (onLockChanged)
            onLockChanged(paramId);
    }
    
    // Clear all locks
    void clearAllLocks()
    {
        lockedParams.clear();
        if (onLockChanged)
            onLockChanged("");
    }
    
    // Get number of locked params
    int getLockedCount() const { return (int)lockedParams.size(); }
    
    // Callback for lock changes
    std::function<void(const juce::String&)> onLockChanged;
    
private:
    ParameterLockManager() = default;
    ~ParameterLockManager() = default;
    
    ParameterLockManager(const ParameterLockManager&) = delete;
    ParameterLockManager& operator=(const ParameterLockManager&) = delete;
    
    std::set<juce::String> lockedParams;
};

//==============================================================================
/**
    LockableSlider - Slider with lock indicator
*/
class LockableSlider : public juce::Slider
{
public:
    LockableSlider(const juce::String& paramId)
        : parameterID(paramId)
    {
        setPopupDisplayEnabled(true, true, nullptr);
    }
    
    void paint(juce::Graphics& g) override
    {
        juce::Slider::paint(g);
        
        // Draw lock icon if locked
        if (ParameterLockManager::getInstance().isLocked(parameterID))
        {
            auto bounds = getLocalBounds();
            auto lockArea = juce::Rectangle<int>(bounds.getRight() - 14, 2, 12, 12);
            
            g.setColour(DS::Colors::warning.withAlpha(0.9f));
            
            // Lock body
            g.fillRoundedRectangle(lockArea.toFloat().reduced(1), 2.0f);
            
            // Lock shackle
            juce::Path shackle;
            auto cx = lockArea.getCentreX();
            shackle.addArc((float)cx - 3.0f, (float)lockArea.getY() - 3.0f, 6.0f, 6.0f, 
                          juce::MathConstants<float>::pi, 0.0f, true);
            g.setColour(DS::Colors::warning);
            g.strokePath(shackle, juce::PathStrokeType(1.5f));
            
            // Lock keyhole
            g.setColour(DS::Colors::background);
            g.fillEllipse((float)cx - 1.5f, (float)lockArea.getCentreY() - 1.0f, 3.0f, 3.0f);
        }
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            showLockMenu(e);
            return;
        }
        
        // If locked, show tooltip but don't allow change
        if (ParameterLockManager::getInstance().isLocked(parameterID))
        {
            // Flash red to indicate locked
            flashLocked();
            return;
        }
        
        juce::Slider::mouseDown(e);
    }
    
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (ParameterLockManager::getInstance().isLocked(parameterID))
            return;  // Don't allow drag if locked
            
        juce::Slider::mouseDrag(e);
    }
    
    void mouseDoubleClick(const juce::MouseEvent& e) override
    {
        if (ParameterLockManager::getInstance().isLocked(parameterID))
        {
            flashLocked();
            return;
        }
        
        juce::Slider::mouseDoubleClick(e);
    }
    
    const juce::String& getParameterID() const { return parameterID; }
    
private:
    juce::String parameterID;
    
    void showLockMenu(const juce::MouseEvent& e)
    {
        auto& lockMgr = ParameterLockManager::getInstance();
        bool isLocked = lockMgr.isLocked(parameterID);
        
        juce::PopupMenu menu;
        menu.addItem(1, isLocked ? "Unlock Parameter" : "Lock Parameter", true, isLocked);
        menu.addSeparator();
        menu.addItem(2, "Clear All Locks", lockMgr.getLockedCount() > 0);
        
        menu.showMenuAsync(juce::PopupMenu::Options()
            .withTargetScreenArea(juce::Rectangle<int>(e.getScreenX(), e.getScreenY(), 1, 1)),
            [this](int result)
            {
                if (result == 1)
                {
                    ParameterLockManager::getInstance().toggleLock(parameterID);
                    repaint();
                }
                else if (result == 2)
                {
                    ParameterLockManager::getInstance().clearAllLocks();
                    // Note: parent should repaint all sliders
                }
            });
    }
    
    void flashLocked()
    {
        // Quick visual feedback that parameter is locked
        auto& animator = juce::Desktop::getInstance().getAnimator();
        
        // Create flash overlay effect by repainting with alpha
        startTimer(50);
        flashAlpha = 1.0f;
    }
    
    void timerCallback()
    {
        flashAlpha -= 0.15f;
        if (flashAlpha <= 0)
        {
            flashAlpha = 0;
            stopTimer();
        }
        repaint();
    }
    
    float flashAlpha = 0.0f;
    
    void startTimer(int intervalMs) { juce::Timer::callAfterDelay(intervalMs, [this]() { timerCallback(); }); }
    void stopTimer() { }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockableSlider)
};

//==============================================================================
/**
    LockIndicatorComponent - Shows lock status badge for a section
*/
class LockIndicatorComponent : public juce::Component
{
public:
    LockIndicatorComponent()
    {
        setSize(20, 20);
    }
    
    void setLockedCount(int count)
    {
        lockedCount = count;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        if (lockedCount == 0) return;
        
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background circle
        g.setColour(DS::Colors::warning.withAlpha(0.8f));
        g.fillEllipse(bounds);
        
        // Lock icon
        g.setColour(DS::Colors::background);
        
        auto cx = bounds.getCentreX();
        auto cy = bounds.getCentreY();
        
        // Lock body
        g.fillRoundedRectangle(cx - 3, cy - 1, 6, 5, 1);
        
        // Lock shackle
        juce::Path shackle;
        shackle.addArc(cx - 2, cy - 4, 4, 4, 
                      juce::MathConstants<float>::pi, 0.0f, true);
        g.strokePath(shackle, juce::PathStrokeType(1.0f));
        
        // Number badge if multiple
        if (lockedCount > 1)
        {
            g.setColour(DS::Colors::warning);
            g.setFont(8.0f);
            g.drawText(juce::String(lockedCount), bounds.translated(5, -5).withSize(12, 12),
                      juce::Justification::centred);
        }
    }
    
private:
    int lockedCount = 0;
};
