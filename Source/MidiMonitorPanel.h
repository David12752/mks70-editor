/*
  ==============================================================================
    MidiMonitorPanel.h
    Real-time MIDI message viewer for debugging and learning
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
 * MidiMessageEntry - Single MIDI message display
 */
struct MidiMessageEntry
{
    juce::Time timestamp;
    juce::String direction;  // "IN" or "OUT"
    juce::String type;       // "Note On", "CC", "SysEx", etc.
    juce::String channel;
    juce::String data;
    juce::String raw;        // Raw hex
    juce::Colour color;
};

//==============================================================================
/**
 * MidiMonitorPanel - Real-time MIDI message viewer
 */
class MidiMonitorPanel : public juce::Component,
                         public juce::Timer
{
public:
    MidiMonitorPanel()
    {
        // Title
        titleLabel.setText("MIDI MONITOR", juce::dontSendNotification);
        titleLabel.setFont(DS::Font::heading());
        titleLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        addAndMakeVisible(titleLabel);
        
        // Clear button
        clearBtn.setButtonText("Clear");
        clearBtn.onClick = [this]() { clearMessages(); };
        addAndMakeVisible(clearBtn);
        
        // Pause button
        pauseBtn.setButtonText("Pause");
        pauseBtn.setClickingTogglesState(true);
        pauseBtn.onClick = [this]() { 
            isPaused = pauseBtn.getToggleState();
            pauseBtn.setButtonText(isPaused ? "Resume" : "Pause");
        };
        addAndMakeVisible(pauseBtn);
        
        // Filter toggles
        showNotesToggle.setButtonText("Notes");
        showNotesToggle.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(showNotesToggle);
        
        showCCToggle.setButtonText("CC");
        showCCToggle.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(showCCToggle);
        
        showSysExToggle.setButtonText("SysEx");
        showSysExToggle.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(showSysExToggle);
        
        showOtherToggle.setButtonText("Other");
        showOtherToggle.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(showOtherToggle);
        
        // Message list
        messageList.setModel(this);
        messageList.setRowHeight(20);
        messageList.setColour(juce::ListBox::backgroundColourId, DS::Color::bgPanel());
        addAndMakeVisible(messageList);
        
        // Start timer for auto-scroll
        startTimer(100);
    }
    
    ~MidiMonitorPanel() override
    {
        stopTimer();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bgPanel());
        
        // Header background
        auto headerArea = getLocalBounds().removeFromTop(60);
        g.setColour(DS::Color::bgElevated());
        g.fillRect(headerArea);
        
        // Column headers
        auto listBounds = getLocalBounds();
        listBounds.removeFromTop(60);
        auto headerRow = listBounds.removeFromTop(20);
        
        g.setColour(DS::Color::bgHover());
        g.fillRect(headerRow);
        
        g.setColour(DS::Color::textSecondary());
        g.setFont(DS::Font::small().withStyle("Bold"));
        
        int x = 5;
        g.drawText("Time", x, headerRow.getY(), 60, 20, juce::Justification::centredLeft);
        x += 65;
        g.drawText("Dir", x, headerRow.getY(), 30, 20, juce::Justification::centredLeft);
        x += 35;
        g.drawText("Type", x, headerRow.getY(), 70, 20, juce::Justification::centredLeft);
        x += 75;
        g.drawText("Ch", x, headerRow.getY(), 25, 20, juce::Justification::centredLeft);
        x += 30;
        g.drawText("Data", x, headerRow.getY(), 150, 20, juce::Justification::centredLeft);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        auto header = bounds.removeFromTop(30);
        
        titleLabel.setBounds(header.removeFromLeft(120));
        clearBtn.setBounds(header.removeFromRight(60).reduced(2));
        pauseBtn.setBounds(header.removeFromRight(60).reduced(2));
        
        auto filterRow = bounds.removeFromTop(26);
        filterRow.removeFromLeft(5);
        showNotesToggle.setBounds(filterRow.removeFromLeft(60).reduced(2));
        showCCToggle.setBounds(filterRow.removeFromLeft(50).reduced(2));
        showSysExToggle.setBounds(filterRow.removeFromLeft(60).reduced(2));
        showOtherToggle.setBounds(filterRow.removeFromLeft(60).reduced(2));
        
        bounds.removeFromTop(24);  // Column headers
        messageList.setBounds(bounds);
    }
    
    //==========================================================================
    // Message Handling
    //==========================================================================
    
    void addIncomingMessage(const juce::MidiMessage& msg)
    {
        if (isPaused) return;
        addMessage(msg, "IN");
    }
    
    void addOutgoingMessage(const juce::MidiMessage& msg)
    {
        if (isPaused) return;
        addMessage(msg, "OUT");
    }
    
    void addMessage(const juce::MidiMessage& msg, const juce::String& direction)
    {
        MidiMessageEntry entry;
        entry.timestamp = juce::Time::getCurrentTime();
        entry.direction = direction;
        entry.color = (direction == "IN") ? DS::Color::Accent::cyan() : DS::Color::Accent::magenta();
        
        // Parse message type
        if (msg.isNoteOn())
        {
            if (!showNotesToggle.getToggleState()) return;
            entry.type = "Note On";
            entry.channel = juce::String(msg.getChannel());
            entry.data = juce::MidiMessage::getMidiNoteName(msg.getNoteNumber(), true, true, 4) 
                       + " Vel:" + juce::String(msg.getVelocity());
            entry.color = juce::Colours::limegreen;
        }
        else if (msg.isNoteOff())
        {
            if (!showNotesToggle.getToggleState()) return;
            entry.type = "Note Off";
            entry.channel = juce::String(msg.getChannel());
            entry.data = juce::MidiMessage::getMidiNoteName(msg.getNoteNumber(), true, true, 4);
            entry.color = juce::Colours::orange;
        }
        else if (msg.isController())
        {
            if (!showCCToggle.getToggleState()) return;
            entry.type = "CC";
            entry.channel = juce::String(msg.getChannel());
            entry.data = "CC" + juce::String(msg.getControllerNumber()) 
                       + " = " + juce::String(msg.getControllerValue());
            entry.color = DS::Color::Accent::primary();
        }
        else if (msg.isSysEx())
        {
            if (!showSysExToggle.getToggleState()) return;
            entry.type = "SysEx";
            entry.channel = "-";
            
            // Show first few bytes
            auto* data = msg.getSysExData();
            int size = msg.getSysExDataSize();
            juce::String hexStr;
            for (int i = 0; i < juce::jmin(8, size); ++i)
                hexStr += juce::String::toHexString(data[i]).paddedLeft('0', 2) + " ";
            if (size > 8) hexStr += "...";
            entry.data = hexStr + " (" + juce::String(size) + " bytes)";
            entry.color = DS::Color::Accent::warning();
        }
        else if (msg.isProgramChange())
        {
            if (!showOtherToggle.getToggleState()) return;
            entry.type = "Prog Chg";
            entry.channel = juce::String(msg.getChannel());
            entry.data = "Program " + juce::String(msg.getProgramChangeNumber());
        }
        else if (msg.isPitchWheel())
        {
            if (!showOtherToggle.getToggleState()) return;
            entry.type = "Pitch";
            entry.channel = juce::String(msg.getChannel());
            entry.data = juce::String(msg.getPitchWheelValue() - 8192);
        }
        else
        {
            if (!showOtherToggle.getToggleState()) return;
            entry.type = "Other";
            entry.channel = "-";
            entry.data = "";
        }
        
        // Generate raw hex
        entry.raw = "";
        auto* rawData = msg.getRawData();
        for (int i = 0; i < juce::jmin(12, msg.getRawDataSize()); ++i)
            entry.raw += juce::String::toHexString(rawData[i]).paddedLeft('0', 2) + " ";
        
        // Add to list (thread-safe)
        {
            const juce::ScopedLock lock(messageLock);
            messages.push_back(entry);
            
            // Keep only last 500 messages
            while (messages.size() > 500)
                messages.erase(messages.begin());
        }
        
        needsUpdate = true;
    }
    
    void clearMessages()
    {
        const juce::ScopedLock lock(messageLock);
        messages.clear();
        messageList.updateContent();
    }
    
    // ListBox callbacks
    int getNumRows() { return (int)messages.size(); }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) 
    {
        const juce::ScopedLock lock(messageLock);
        
        if (rowNumber < 0 || rowNumber >= (int)messages.size())
            return;
            
        auto& entry = messages[rowNumber];
        
        // Background
        if (rowIsSelected)
            g.fillAll(DS::Color::bgHover());
        else if (rowNumber % 2 == 0)
            g.fillAll(DS::Color::bgPanel().brighter(0.02f));
            
        g.setFont(DS::Font::tiny());
        
        int x = 5;
        
        // Time
        g.setColour(DS::Color::textMuted());
        g.drawText(entry.timestamp.toString(false, true, true, true).substring(11), 
                   x, 0, 60, height, juce::Justification::centredLeft);
        x += 65;
        
        // Direction
        g.setColour(entry.direction == "IN" ? DS::Color::Accent::cyan() : DS::Color::Accent::magenta());
        g.drawText(entry.direction, x, 0, 30, height, juce::Justification::centredLeft);
        x += 35;
        
        // Type
        g.setColour(entry.color);
        g.drawText(entry.type, x, 0, 70, height, juce::Justification::centredLeft);
        x += 75;
        
        // Channel
        g.setColour(DS::Color::textSecondary());
        g.drawText(entry.channel, x, 0, 25, height, juce::Justification::centredLeft);
        x += 30;
        
        // Data
        g.setColour(DS::Color::textPrimary());
        g.drawText(entry.data, x, 0, width - x - 5, height, juce::Justification::centredLeft);
    }
    
    void timerCallback() override
    {
        if (needsUpdate)
        {
            messageList.updateContent();
            if (!isPaused)
                messageList.scrollToEnsureRowIsOnscreen((int)messages.size() - 1);
            needsUpdate = false;
        }
    }
    
private:
    juce::Label titleLabel;
    juce::TextButton clearBtn, pauseBtn;
    juce::ToggleButton showNotesToggle, showCCToggle, showSysExToggle, showOtherToggle;
    
    // Custom ListBox that uses this class for data
    class MessageListBox : public juce::ListBox
    {
    public:
        void setModel(MidiMonitorPanel* panel) { owner = panel; }
        int getNumRows() override { return owner ? owner->getNumRows() : 0; }
        void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool sel) override
        {
            if (owner) owner->paintListBoxItem(row, g, w, h, sel);
        }
    private:
        MidiMonitorPanel* owner = nullptr;
    };
    
    MessageListBox messageList;
    
    std::vector<MidiMessageEntry> messages;
    juce::CriticalSection messageLock;
    bool isPaused = false;
    bool needsUpdate = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMonitorPanel)
};
