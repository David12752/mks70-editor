/*
  ==============================================================================
    SoundPreview.h
    Auto-trigger notes when browsing presets for instant preview
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
 * SoundPreviewSettings - Configuration for sound preview behavior
 */
struct SoundPreviewSettings
{
    bool enabled = true;
    int noteNumber = 60;      // Middle C
    int velocity = 100;
    int duration = 500;       // ms
    int delayAfterChange = 100;  // ms delay before triggering
    int channel = 1;
    
    juce::var toVar() const
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("enabled", enabled);
        obj->setProperty("noteNumber", noteNumber);
        obj->setProperty("velocity", velocity);
        obj->setProperty("duration", duration);
        obj->setProperty("delayAfterChange", delayAfterChange);
        obj->setProperty("channel", channel);
        return juce::var(obj.get());
    }
    
    void fromVar(const juce::var& v)
    {
        if (!v.isObject()) return;
        enabled = v.getProperty("enabled", enabled);
        noteNumber = v.getProperty("noteNumber", noteNumber);
        velocity = v.getProperty("velocity", velocity);
        duration = v.getProperty("duration", duration);
        delayAfterChange = v.getProperty("delayAfterChange", delayAfterChange);
        channel = v.getProperty("channel", channel);
    }
};

//==============================================================================
/**
 * SoundPreviewManager - Manages automatic sound preview on preset changes
 */
class SoundPreviewManager : public juce::Timer
{
public:
    using SendNoteFunc = std::function<void(int note, int velocity, int channel, bool isNoteOn)>;
    
    SoundPreviewManager() {}
    
    void setNoteCallback(SendNoteFunc callback) { sendNoteCallback = callback; }
    
    void setSettings(const SoundPreviewSettings& newSettings) 
    { 
        settings = newSettings; 
    }
    
    SoundPreviewSettings& getSettings() { return settings; }
    
    void triggerPreview()
    {
        if (!settings.enabled || !sendNoteCallback) return;
        
        // Cancel any pending preview
        stopTimer();
        
        // Stop any currently playing note
        if (noteIsPlaying)
        {
            sendNoteCallback(currentNote, 0, settings.channel, false);
            noteIsPlaying = false;
        }
        
        // Start delay timer before triggering
        pendingTrigger = true;
        startTimer(settings.delayAfterChange);
    }
    
    void stopPreview()
    {
        stopTimer();
        pendingTrigger = false;
        
        if (noteIsPlaying && sendNoteCallback)
        {
            sendNoteCallback(currentNote, 0, settings.channel, false);
            noteIsPlaying = false;
        }
    }
    
    void timerCallback() override
    {
        if (pendingTrigger)
        {
            // Time to trigger the note
            pendingTrigger = false;
            currentNote = settings.noteNumber;
            
            if (sendNoteCallback)
            {
                sendNoteCallback(currentNote, settings.velocity, settings.channel, true);
                noteIsPlaying = true;
            }
            
            // Set timer for note off
            startTimer(settings.duration);
        }
        else if (noteIsPlaying)
        {
            // Time to stop the note
            stopTimer();
            
            if (sendNoteCallback)
                sendNoteCallback(currentNote, 0, settings.channel, false);
                
            noteIsPlaying = false;
        }
    }
    
    bool isEnabled() const { return settings.enabled; }
    void setEnabled(bool enabled) { settings.enabled = enabled; }
    
private:
    SoundPreviewSettings settings;
    SendNoteFunc sendNoteCallback;
    
    bool pendingTrigger = false;
    bool noteIsPlaying = false;
    int currentNote = 60;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundPreviewManager)
};

//==============================================================================
/**
 * SoundPreviewSettingsPanel - UI for configuring sound preview
 */
class SoundPreviewSettingsPanel : public juce::Component
{
public:
    SoundPreviewSettingsPanel(SoundPreviewManager& manager)
        : previewManager(manager)
    {
        auto& settings = manager.getSettings();
        
        // Enable toggle
        enableToggle.setButtonText("Enable Sound Preview");
        enableToggle.setToggleState(settings.enabled, juce::dontSendNotification);
        enableToggle.onClick = [this, &manager]() {
            manager.getSettings().enabled = enableToggle.getToggleState();
            updateEnabledState();
        };
        addAndMakeVisible(enableToggle);
        
        // Note selector
        noteLabel.setText("Note:", juce::dontSendNotification);
        noteLabel.setFont(DS::Font::label());
        addAndMakeVisible(noteLabel);
        
        for (int i = 36; i <= 84; ++i)  // C2 to C6
            noteCombo.addItem(juce::MidiMessage::getMidiNoteName(i, true, true, 4), i);
        noteCombo.setSelectedId(settings.noteNumber, juce::dontSendNotification);
        noteCombo.onChange = [this, &manager]() {
            manager.getSettings().noteNumber = noteCombo.getSelectedId();
        };
        addAndMakeVisible(noteCombo);
        
        // Velocity slider
        velocityLabel.setText("Velocity:", juce::dontSendNotification);
        velocityLabel.setFont(DS::Font::label());
        addAndMakeVisible(velocityLabel);
        
        velocitySlider.setRange(1, 127, 1);
        velocitySlider.setValue(settings.velocity, juce::dontSendNotification);
        velocitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        velocitySlider.onValueChange = [this, &manager]() {
            manager.getSettings().velocity = (int)velocitySlider.getValue();
        };
        addAndMakeVisible(velocitySlider);
        
        // Duration slider
        durationLabel.setText("Duration:", juce::dontSendNotification);
        durationLabel.setFont(DS::Font::label());
        addAndMakeVisible(durationLabel);
        
        durationSlider.setRange(100, 2000, 50);
        durationSlider.setValue(settings.duration, juce::dontSendNotification);
        durationSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
        durationSlider.setTextValueSuffix(" ms");
        durationSlider.onValueChange = [this, &manager]() {
            manager.getSettings().duration = (int)durationSlider.getValue();
        };
        addAndMakeVisible(durationSlider);
        
        // Delay slider
        delayLabel.setText("Delay:", juce::dontSendNotification);
        delayLabel.setFont(DS::Font::label());
        addAndMakeVisible(delayLabel);
        
        delaySlider.setRange(0, 500, 25);
        delaySlider.setValue(settings.delayAfterChange, juce::dontSendNotification);
        delaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
        delaySlider.setTextValueSuffix(" ms");
        delaySlider.onValueChange = [this, &manager]() {
            manager.getSettings().delayAfterChange = (int)delaySlider.getValue();
        };
        addAndMakeVisible(delaySlider);
        
        // Test button
        testBtn.setButtonText("Test Preview");
        testBtn.onClick = [&manager]() {
            manager.triggerPreview();
        };
        addAndMakeVisible(testBtn);
        
        updateEnabledState();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bgPanel());
        
        g.setColour(DS::Color::textPrimary());
        g.setFont(DS::Font::label().withStyle("Bold"));
        g.drawText("Sound Preview Settings", 10, 5, 200, 20, juce::Justification::centredLeft);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(10);
        bounds.removeFromTop(25);  // Title
        
        enableToggle.setBounds(bounds.removeFromTop(25));
        bounds.removeFromTop(10);
        
        auto row = bounds.removeFromTop(25);
        noteLabel.setBounds(row.removeFromLeft(60));
        noteCombo.setBounds(row.removeFromLeft(80));
        bounds.removeFromTop(5);
        
        row = bounds.removeFromTop(25);
        velocityLabel.setBounds(row.removeFromLeft(60));
        velocitySlider.setBounds(row);
        bounds.removeFromTop(5);
        
        row = bounds.removeFromTop(25);
        durationLabel.setBounds(row.removeFromLeft(60));
        durationSlider.setBounds(row);
        bounds.removeFromTop(5);
        
        row = bounds.removeFromTop(25);
        delayLabel.setBounds(row.removeFromLeft(60));
        delaySlider.setBounds(row);
        bounds.removeFromTop(10);
        
        testBtn.setBounds(bounds.removeFromTop(28).removeFromLeft(100));
    }
    
private:
    void updateEnabledState()
    {
        bool enabled = enableToggle.getToggleState();
        noteCombo.setEnabled(enabled);
        velocitySlider.setEnabled(enabled);
        durationSlider.setEnabled(enabled);
        delaySlider.setEnabled(enabled);
        testBtn.setEnabled(enabled);
    }
    
    SoundPreviewManager& previewManager;
    
    juce::ToggleButton enableToggle;
    juce::Label noteLabel, velocityLabel, durationLabel, delayLabel;
    juce::ComboBox noteCombo;
    juce::Slider velocitySlider, durationSlider, delaySlider;
    juce::TextButton testBtn;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundPreviewSettingsPanel)
};
