#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * ToastNotification - Animated toast messages for feedback
 * Shows brief notifications that auto-dismiss
 */
class ToastNotification : public juce::Component, private juce::Timer {
public:
    enum class Type {
        Info,
        Success,
        Warning,
        Error
    };
    
    ToastNotification() {
        setAlwaysOnTop(true);
        setInterceptsMouseClicks(false, false);
    }
    
    void show(const juce::String& message, Type type = Type::Info, int durationMs = 2000) {
        currentMessage = message;
        currentType = type;
        opacity = 1.0f;
        slideOffset = 30.0f; // Start 30px below
        fadingOut = false;
        
        displayDuration = durationMs;
        displayStartTime = juce::Time::getCurrentTime();
        
        setVisible(true);
        startTimerHz(60);
        repaint();
    }
    
    void paint(juce::Graphics& g) override {
        if (currentMessage.isEmpty()) return;
        
        auto bounds = getLocalBounds().toFloat();
        
        // Slide offset animation
        bounds = bounds.translated(0, slideOffset);
        
        // Choose color based on type
        juce::Colour bgColor, accentColor;
        switch (currentType) {
            case Type::Success:
                bgColor = juce::Colour(0xE51A2A1A);
                accentColor = DS::Color::Accent::success();
                break;
            case Type::Warning:
                bgColor = juce::Colour(0xE52A2A1A);
                accentColor = DS::Color::Accent::warning();
                break;
            case Type::Error:
                bgColor = juce::Colour(0xE52A1A1A);
                accentColor = DS::Color::Accent::error();
                break;
            default:
                bgColor = juce::Colour(0xE5101520);
                accentColor = DS::Color::Accent::primary();
        }
        
        // Background with opacity
        g.setColour(bgColor.withAlpha(opacity));
        g.fillRoundedRectangle(bounds, DS::Radius::M);
        
        // Accent left bar
        g.setColour(accentColor.withAlpha(opacity));
        g.fillRoundedRectangle(bounds.getX(), bounds.getY(), 4, bounds.getHeight(), 2);
        
        // Border
        g.setColour(accentColor.withAlpha(opacity * 0.4f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), DS::Radius::M, 1.0f);
        
        // Icon
        juce::String icon;
        switch (currentType) {
            case Type::Success: icon = juce::CharPointer_UTF8("\xe2\x9c\x93"); break; // ✓
            case Type::Warning: icon = juce::CharPointer_UTF8("\xe2\x9a\xa0"); break; // ⚠
            case Type::Error: icon = juce::CharPointer_UTF8("\xe2\x9c\x97"); break;   // ✗
            default: icon = juce::CharPointer_UTF8("\xe2\x84\xb9"); break;            // ℹ
        }
        
        g.setColour(accentColor.withAlpha(opacity));
        g.setFont(DS::Font::large());
        g.drawText(icon, bounds.withWidth(40).translated(8, 0), juce::Justification::centred);
        
        // Message text
        g.setColour(DS::Color::textPrimary().withAlpha(opacity));
        g.setFont(DS::Font::normal());
        g.drawText(currentMessage, bounds.reduced(50, 0), juce::Justification::centredLeft);
    }
    
private:
    juce::String currentMessage;
    Type currentType = Type::Info;
    float opacity = 0.0f;
    float slideOffset = 30.0f;
    bool fadingOut = false;
    int displayDuration = 2000;
    juce::Time displayStartTime;
    
    void timerCallback() override {
        auto elapsed = (juce::Time::getCurrentTime() - displayStartTime).inMilliseconds();
        
        // Slide in animation
        if (slideOffset > 0) {
            slideOffset = juce::jmax(0.0f, slideOffset - 3.0f);
        }
        
        // Check if we should start fading
        if (!fadingOut && elapsed > displayDuration) {
            fadingOut = true;
        }
        
        // Fade out animation
        if (fadingOut) {
            opacity -= 0.05f;
            if (opacity <= 0) {
                stopTimer();
                setVisible(false);
                currentMessage.clear();
            }
        }
        
        repaint();
    }
};

/**
 * ToastManager - Global toast notification manager
 * Use ToastManager::getInstance().show(...) to display toasts
 */
class ToastManager : public juce::DeletedAtShutdown {
public:
    ToastManager() = default;
    
    static ToastManager& getInstance() {
        static ToastManager instance;
        return instance;
    }
    
    void setParentComponent(juce::Component* parent) {
        parentComponent = parent;
        if (parent) {
            parent->addChildComponent(toast);
        }
    }
    
    void show(const juce::String& message, ToastNotification::Type type = ToastNotification::Type::Info) {
        if (parentComponent) {
            // Position at bottom center
            int toastW = 350;
            int toastH = 50;
            int x = (parentComponent->getWidth() - toastW) / 2;
            int y = parentComponent->getHeight() - toastH - 60;
            toast.setBounds(x, y, toastW, toastH);
            toast.show(message, type);
        }
    }
    
    void showUndo(const juce::String& actionName) {
        show("Undo: " + actionName, ToastNotification::Type::Info);
    }
    
    void showRedo(const juce::String& actionName) {
        show("Redo: " + actionName, ToastNotification::Type::Info);
    }
    
    void showSuccess(const juce::String& message) {
        show(message, ToastNotification::Type::Success);
    }
    
    void showError(const juce::String& message) {
        show(message, ToastNotification::Type::Error);
    }
    
    void showWarning(const juce::String& message) {
        show(message, ToastNotification::Type::Warning);
    }
    
private:
    juce::Component* parentComponent = nullptr;
    ToastNotification toast;
};
