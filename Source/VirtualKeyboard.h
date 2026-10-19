/*
  ==============================================================================
    VirtualKeyboard.h
    Virtual MIDI keyboard for testing sounds without hardware
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "MidiHandler.h"

//==============================================================================
/**
 * VirtualKeyboard - Collapsible MIDI keyboard for sound testing
 */
class VirtualKeyboard : public juce::Component,
                        public juce::MidiKeyboardState::Listener
{
public:
    VirtualKeyboard(MidiHandler& handler)
        : midiHandler(handler),
          keyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        // Setup keyboard appearance
        keyboard.setAvailableRange(36, 96);  // C2 to C7
        keyboard.setOctaveForMiddleC(4);
        keyboard.setKeyWidth(18.0f);
        keyboard.setScrollButtonWidth(12);
        
        // Style the keyboard
        keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, 
                          juce::Colour(0xFFE8E8E8));
        keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId,
                          juce::Colour(0xFF2A2A2A));
        keyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId,
                          juce::Colour(0xFF666666));
        keyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                          DS::Color::Accent::primary().withAlpha(0.3f));
        keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId,
                          DS::Color::Accent::primary().withAlpha(0.6f));
        
        addAndMakeVisible(keyboard);
        
        // Listen for keyboard events
        keyboardState.addListener(this);
        
        // Collapse/Expand button
        collapseBtn.setButtonText("▼");
        collapseBtn.setTooltip("Show/Hide Virtual Keyboard");
        collapseBtn.onClick = [this]() { toggleCollapsed(); };
        collapseBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        collapseBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        addAndMakeVisible(collapseBtn);
        
        // Velocity slider
        velocitySlider.setRange(1, 127, 1);
        velocitySlider.setValue(100);
        velocitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
        velocitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 35, 18);
        velocitySlider.setColour(juce::Slider::trackColourId, DS::Color::Accent::primary());
        addAndMakeVisible(velocitySlider);
        
        velocityLabel.setText("Vel:", juce::dontSendNotification);
        velocityLabel.setFont(DS::Font::small());
        velocityLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        addAndMakeVisible(velocityLabel);
        
        // Octave buttons
        octaveDownBtn.setButtonText("-");
        octaveDownBtn.setTooltip("Octave Down");
        octaveDownBtn.onClick = [this]() { shiftOctave(-1); };
        addAndMakeVisible(octaveDownBtn);
        
        octaveUpBtn.setButtonText("+");
        octaveUpBtn.setTooltip("Octave Up");
        octaveUpBtn.onClick = [this]() { shiftOctave(1); };
        addAndMakeVisible(octaveUpBtn);
        
        octaveLabel.setText("Oct: 4", juce::dontSendNotification);
        octaveLabel.setFont(DS::Font::small());
        octaveLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(octaveLabel);
        
        // Sustain pedal toggle
        sustainBtn.setButtonText("Sus");
        sustainBtn.setTooltip("Sustain Pedal (Hold)");
        sustainBtn.setClickingTogglesState(true);
        sustainBtn.onStateChange = [this]() { 
            sendSustain(sustainBtn.getToggleState()); 
        };
        addAndMakeVisible(sustainBtn);
        
        // Channel selector
        channelCombo.addItem("Ch 1", 1);
        channelCombo.addItem("Ch 2", 2);
        channelCombo.addItem("Ch 3", 3);
        channelCombo.addItem("Ch 4", 4);
        channelCombo.setSelectedId(1);
        channelCombo.setTooltip("MIDI Channel");
        addAndMakeVisible(channelCombo);
        
        // Start collapsed
        isCollapsed = true;
    }
    
    ~VirtualKeyboard() override
    {
        keyboardState.removeListener(this);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bgPanel());
        
        // Top bar
        auto topBar = getLocalBounds().removeFromTop(24);
        g.setColour(DS::Color::bgElevated());
        g.fillRect(topBar);
        
        // Label
        g.setColour(DS::Color::textSecondary());
        g.setFont(DS::Font::small());
        g.drawText("VIRTUAL KEYBOARD", topBar.reduced(30, 0), 
                   juce::Justification::centredLeft);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        auto topBar = bounds.removeFromTop(24);
        
        // Collapse button
        collapseBtn.setBounds(topBar.removeFromLeft(24).reduced(2));
        
        // Channel selector (right side)
        channelCombo.setBounds(topBar.removeFromRight(60).reduced(2));
        
        // Sustain button
        sustainBtn.setBounds(topBar.removeFromRight(35).reduced(2));
        
        if (!isCollapsed)
        {
            // Controls row
            auto controlsRow = bounds.removeFromTop(26);
            controlsRow.removeFromLeft(4);
            
            // Octave controls
            octaveDownBtn.setBounds(controlsRow.removeFromLeft(24).reduced(2));
            octaveLabel.setBounds(controlsRow.removeFromLeft(45).reduced(2));
            octaveUpBtn.setBounds(controlsRow.removeFromLeft(24).reduced(2));
            
            controlsRow.removeFromLeft(10);
            
            // Velocity
            velocityLabel.setBounds(controlsRow.removeFromLeft(25).reduced(2));
            velocitySlider.setBounds(controlsRow.removeFromLeft(120).reduced(2));
            
            // Keyboard
            keyboard.setBounds(bounds.reduced(2));
        }
    }
    
    void toggleCollapsed()
    {
        isCollapsed = !isCollapsed;
        collapseBtn.setButtonText(isCollapsed ? "▶" : "▼");
        
        if (onCollapsedChanged)
            onCollapsedChanged(isCollapsed);
            
        resized();
        repaint();
    }
    
    bool getIsCollapsed() const { return isCollapsed; }
    
    int getPreferredHeight() const
    {
        return isCollapsed ? 24 : 100;
    }
    
    // Callback when collapsed state changes
    std::function<void(bool)> onCollapsedChanged;
    
    // Send a test note (for preview)
    void triggerTestNote(int note = 60, int velocity = -1)
    {
        if (velocity < 0)
            velocity = (int)velocitySlider.getValue();
            
        int channel = channelCombo.getSelectedId();
        
        // Note on
        midiHandler.sendNoteOn(channel, note, velocity);
        
        // Schedule note off after 200ms
        juce::Timer::callAfterDelay(200, [this, channel, note]() {
            midiHandler.sendNoteOff(channel, note);
        });
    }
    
private:
    MidiHandler& midiHandler;
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboard;
    
    juce::TextButton collapseBtn;
    juce::Slider velocitySlider;
    juce::Label velocityLabel;
    juce::TextButton octaveDownBtn, octaveUpBtn;
    juce::Label octaveLabel;
    juce::TextButton sustainBtn;
    juce::ComboBox channelCombo;
    
    bool isCollapsed = true;
    int currentOctave = 4;
    
    void shiftOctave(int delta)
    {
        currentOctave = juce::jlimit(0, 8, currentOctave + delta);
        octaveLabel.setText("Oct: " + juce::String(currentOctave), juce::dontSendNotification);
        
        int lowestNote = (currentOctave - 2) * 12;
        keyboard.setAvailableRange(lowestNote, lowestNote + 60);
    }
    
    void sendSustain(bool on)
    {
        int channel = channelCombo.getSelectedId();
        midiHandler.sendCC(channel, 64, on ? 127 : 0);
    }
    
    // MidiKeyboardState::Listener
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        int channel = channelCombo.getSelectedId();
        int vel = (int)(velocity * velocitySlider.getValue());
        midiHandler.sendNoteOn(channel, midiNoteNumber, vel);
    }
    
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        int channel = channelCombo.getSelectedId();
        midiHandler.sendNoteOff(channel, midiNoteNumber);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualKeyboard)
};
