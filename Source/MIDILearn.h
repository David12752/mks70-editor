#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * MIDILearn.h
 * Advanced MIDI Learn system with visual feedback
 * Right-click to learn, visual CC indicators
 * 
 * Part of Improvement #8: MIDI Learn 2.0
 */

namespace MIDILearn
{

//==============================================================================
/**
 * MIDI CC Mapping entry
 */
struct CCMapping
{
    juce::String parameterId;
    int ccNumber = -1;
    int channel = 0;  // 0 = any channel
    float minValue = 0.0f;
    float maxValue = 1.0f;
    bool inverted = false;
    bool isRelative = false;  // For endless encoders
    
    bool isValid() const { return ccNumber >= 0 && ccNumber < 128; }
};

//==============================================================================
/**
 * MIDI Learn State Manager
 */
class LearnManager
{
public:
    LearnManager()
    {
    }
    
    //==========================================================================
    // Learning Mode
    //==========================================================================
    
    void startLearning(const juce::String& parameterId)
    {
        learningParameter = parameterId;
        isLearning = true;
        
        if (onLearningStarted)
            onLearningStarted(parameterId);
    }
    
    void stopLearning()
    {
        juce::String wasLearning = learningParameter;
        learningParameter = "";
        isLearning = false;
        
        if (onLearningStopped)
            onLearningStopped(wasLearning);
    }
    
    bool getIsLearning() const { return isLearning; }
    juce::String getLearningParameter() const { return learningParameter; }
    
    //==========================================================================
    // CC Handling
    //==========================================================================
    
    void handleCC(int channel, int ccNumber, int value)
    {
        lastReceivedCC = ccNumber;
        lastReceivedChannel = channel;
        lastReceivedValue = value;
        
        if (isLearning && learningParameter.isNotEmpty())
        {
            // Create new mapping
            CCMapping mapping;
            mapping.parameterId = learningParameter;
            mapping.ccNumber = ccNumber;
            mapping.channel = channel;
            
            addMapping(mapping);
            stopLearning();
            return;
        }
        
        // Check existing mappings
        for (const auto& mapping : mappings)
        {
            if (mapping.ccNumber == ccNumber)
            {
                if (mapping.channel == 0 || mapping.channel == channel)
                {
                    // Calculate parameter value
                    float normalizedValue = value / 127.0f;
                    
                    if (mapping.inverted)
                        normalizedValue = 1.0f - normalizedValue;
                    
                    float paramValue = mapping.minValue + normalizedValue * (mapping.maxValue - mapping.minValue);
                    
                    if (onCCValueChanged)
                        onCCValueChanged(mapping.parameterId, paramValue);
                }
            }
        }
        
        if (onCCReceived)
            onCCReceived(channel, ccNumber, value);
    }
    
    //==========================================================================
    // Mapping Management
    //==========================================================================
    
    void addMapping(const CCMapping& mapping)
    {
        // Remove any existing mapping for this parameter
        removeMapping(mapping.parameterId);
        
        mappings.push_back(mapping);
        
        if (onMappingAdded)
            onMappingAdded(mapping);
    }
    
    void removeMapping(const juce::String& parameterId)
    {
        auto it = std::remove_if(mappings.begin(), mappings.end(),
            [&](const CCMapping& m) { return m.parameterId == parameterId; });
        
        if (it != mappings.end())
        {
            mappings.erase(it, mappings.end());
            
            if (onMappingRemoved)
                onMappingRemoved(parameterId);
        }
    }
    
    void removeMappingByCC(int ccNumber)
    {
        auto it = std::remove_if(mappings.begin(), mappings.end(),
            [&](const CCMapping& m) { return m.ccNumber == ccNumber; });
        
        mappings.erase(it, mappings.end());
    }
    
    void clearAllMappings()
    {
        mappings.clear();
        
        if (onMappingsCleared)
            onMappingsCleared();
    }
    
    const CCMapping* getMappingForParameter(const juce::String& parameterId) const
    {
        for (const auto& mapping : mappings)
        {
            if (mapping.parameterId == parameterId)
                return &mapping;
        }
        return nullptr;
    }
    
    const CCMapping* getMappingForCC(int ccNumber) const
    {
        for (const auto& mapping : mappings)
        {
            if (mapping.ccNumber == ccNumber)
                return &mapping;
        }
        return nullptr;
    }
    
    const std::vector<CCMapping>& getAllMappings() const { return mappings; }
    int getMappingCount() const { return static_cast<int>(mappings.size()); }
    
    //==========================================================================
    // Last Received
    //==========================================================================
    
    int getLastReceivedCC() const { return lastReceivedCC; }
    int getLastReceivedChannel() const { return lastReceivedChannel; }
    int getLastReceivedValue() const { return lastReceivedValue; }
    
    //==========================================================================
    // Save/Load
    //==========================================================================
    
    juce::String saveToXml() const
    {
        juce::XmlElement root("MIDIMappings");
        
        for (const auto& mapping : mappings)
        {
            auto* element = root.createNewChildElement("Mapping");
            element->setAttribute("parameterId", mapping.parameterId);
            element->setAttribute("cc", mapping.ccNumber);
            element->setAttribute("channel", mapping.channel);
            element->setAttribute("min", mapping.minValue);
            element->setAttribute("max", mapping.maxValue);
            element->setAttribute("inverted", mapping.inverted);
            element->setAttribute("relative", mapping.isRelative);
        }
        
        return root.toString();
    }
    
    void loadFromXml(const juce::String& xmlString)
    {
        auto root = juce::XmlDocument::parse(xmlString);
        
        if (root == nullptr || root->getTagName() != "MIDIMappings")
            return;
        
        mappings.clear();
        
        for (auto* element : root->getChildIterator())
        {
            if (element->getTagName() == "Mapping")
            {
                CCMapping mapping;
                mapping.parameterId = element->getStringAttribute("parameterId");
                mapping.ccNumber = element->getIntAttribute("cc", -1);
                mapping.channel = element->getIntAttribute("channel", 0);
                mapping.minValue = static_cast<float>(element->getDoubleAttribute("min", 0.0));
                mapping.maxValue = static_cast<float>(element->getDoubleAttribute("max", 1.0));
                mapping.inverted = element->getBoolAttribute("inverted", false);
                mapping.isRelative = element->getBoolAttribute("relative", false);
                
                if (mapping.isValid())
                    mappings.push_back(mapping);
            }
        }
    }
    
    //==========================================================================
    // Callbacks
    //==========================================================================
    
    std::function<void(const juce::String&)> onLearningStarted;
    std::function<void(const juce::String&)> onLearningStopped;
    std::function<void(int, int, int)> onCCReceived;
    std::function<void(const juce::String&, float)> onCCValueChanged;
    std::function<void(const CCMapping&)> onMappingAdded;
    std::function<void(const juce::String&)> onMappingRemoved;
    std::function<void()> onMappingsCleared;
    
private:
    std::vector<CCMapping> mappings;
    juce::String learningParameter;
    bool isLearning = false;
    
    int lastReceivedCC = -1;
    int lastReceivedChannel = 0;
    int lastReceivedValue = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LearnManager)
};

//==============================================================================
/**
 * MIDI Learn Overlay
 * Visual indicator when learning mode is active
 */
class LearnOverlay : public juce::Component, private juce::Timer
{
public:
    LearnOverlay()
    {
        setInterceptsMouseClicks(false, false);
    }
    
    void setLearning(bool learning, const juce::String& paramName = "")
    {
        isLearning = learning;
        parameterName = paramName;
        
        if (learning)
        {
            startTimerHz(30);
            pulsePhase = 0.0f;
        }
        else
        {
            stopTimer();
        }
        
        setVisible(learning);
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        if (!isLearning)
            return;
        
        auto bounds = getLocalBounds();
        
        // Pulsing border
        float alpha = 0.3f + 0.3f * std::sin(pulsePhase);
        g.setColour(juce::Colour(0xFFFF6B00).withAlpha(alpha));
        g.drawRect(bounds, 3);
        
        // Semi-transparent overlay
        g.setColour(juce::Colour(0xFF000000).withAlpha(0.3f));
        g.fillRect(bounds);
        
        // Learning message
        auto messageBounds = bounds.withSizeKeepingCentre(300, 100);
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(messageBounds.toFloat(), DS::Radius::large);
        
        g.setColour(juce::Colour(0xFFFF6B00));
        g.drawRoundedRectangle(messageBounds.toFloat(), DS::Radius::large, 2.0f);
        
        g.setFont(DS::Font::heading.withHeight(18.0f));
        g.setColour(DS::Color::text);
        g.drawText("MIDI Learn Active", messageBounds.removeFromTop(30), juce::Justification::centred);
        
        g.setFont(DS::Font::body);
        g.setColour(DS::Color::textMuted);
        g.drawText("Waiting for CC...", messageBounds.removeFromTop(25), juce::Justification::centred);
        
        if (parameterName.isNotEmpty())
        {
            g.setColour(juce::Colour(0xFFFF6B00));
            g.drawText(parameterName, messageBounds.removeFromTop(25), juce::Justification::centred);
        }
        
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText("Press ESC to cancel", messageBounds, juce::Justification::centred);
    }
    
private:
    void timerCallback() override
    {
        pulsePhase += 0.15f;
        if (pulsePhase > juce::MathConstants<float>::twoPi)
            pulsePhase -= juce::MathConstants<float>::twoPi;
        repaint();
    }
    
    bool isLearning = false;
    juce::String parameterName;
    float pulsePhase = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LearnOverlay)
};

//==============================================================================
/**
 * CC Badge
 * Small indicator showing CC assignment on a control
 */
class CCBadge : public juce::Component
{
public:
    CCBadge()
    {
        setRepaintsOnMouseActivity(true);
    }
    
    void setCC(int cc)
    {
        ccNumber = cc;
        repaint();
    }
    
    void clearCC()
    {
        ccNumber = -1;
        repaint();
    }
    
    void setActive(bool active)
    {
        isActive = active;
        repaint();
    }
    
    void setLearning(bool learning)
    {
        isLearningMode = learning;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        if (ccNumber < 0 && !isLearningMode)
            return;
        
        auto bounds = getLocalBounds().toFloat().reduced(1);
        
        // Background
        juce::Colour bgColor;
        if (isLearningMode)
            bgColor = juce::Colour(0xFFFF6B00);
        else if (isActive)
            bgColor = juce::Colour(0xFF00C853);
        else
            bgColor = DS::Color::accent;
        
        g.setColour(bgColor.withAlpha(0.9f));
        g.fillRoundedRectangle(bounds, 3);
        
        // Text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        
        if (isLearningMode)
            g.drawText("...", bounds, juce::Justification::centred);
        else
            g.drawText(juce::String(ccNumber), bounds, juce::Justification::centred);
    }
    
private:
    int ccNumber = -1;
    bool isActive = false;
    bool isLearningMode = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CCBadge)
};

//==============================================================================
/**
 * MIDI Learn Context Menu Helper
 */
class LearnMenuHelper
{
public:
    static void showContextMenu(LearnManager& manager, const juce::String& parameterId, 
                                juce::Component* anchor)
    {
        juce::PopupMenu menu;
        
        const CCMapping* existing = manager.getMappingForParameter(parameterId);
        
        if (existing != nullptr)
        {
            menu.addItem(1, "Clear MIDI Learn (CC" + juce::String(existing->ccNumber) + ")");
            menu.addSeparator();
        }
        
        menu.addItem(2, "Learn MIDI CC...");
        
        if (existing != nullptr)
        {
            menu.addSeparator();
            
            juce::PopupMenu rangeMenu;
            rangeMenu.addItem(10, "Full Range (0-127)");
            rangeMenu.addItem(11, "Half Range (0-63)");
            rangeMenu.addItem(12, "Upper Half (64-127)");
            menu.addSubMenu("Set Range", rangeMenu);
            
            menu.addItem(3, "Invert", true, existing->inverted);
        }
        
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(anchor),
            [&manager, parameterId, existing](int result)
            {
                switch (result)
                {
                    case 1:
                        manager.removeMapping(parameterId);
                        break;
                    case 2:
                        manager.startLearning(parameterId);
                        break;
                    case 3:
                        if (existing)
                        {
                            CCMapping modified = *existing;
                            modified.inverted = !modified.inverted;
                            manager.addMapping(modified);
                        }
                        break;
                    case 10:
                        if (existing)
                        {
                            CCMapping modified = *existing;
                            modified.minValue = 0.0f;
                            modified.maxValue = 1.0f;
                            manager.addMapping(modified);
                        }
                        break;
                    case 11:
                        if (existing)
                        {
                            CCMapping modified = *existing;
                            modified.minValue = 0.0f;
                            modified.maxValue = 0.5f;
                            manager.addMapping(modified);
                        }
                        break;
                    case 12:
                        if (existing)
                        {
                            CCMapping modified = *existing;
                            modified.minValue = 0.5f;
                            modified.maxValue = 1.0f;
                            manager.addMapping(modified);
                        }
                        break;
                }
            });
    }
};

//==============================================================================
/**
 * MIDI Mappings Panel
 * Shows all current mappings in a list
 */
class MappingsPanel : public juce::Component, public juce::TableListBoxModel
{
public:
    MappingsPanel(LearnManager& manager) : learnManager(manager)
    {
        table.setModel(this);
        table.setHeader(new juce::TableHeaderComponent());
        
        table.getHeader().addColumn("Parameter", 1, 150);
        table.getHeader().addColumn("CC#", 2, 50);
        table.getHeader().addColumn("Channel", 3, 60);
        table.getHeader().addColumn("Range", 4, 80);
        table.getHeader().addColumn("", 5, 40); // Delete button
        
        table.setHeaderHeight(25);
        table.setRowHeight(24);
        
        addAndMakeVisible(table);
        
        // Clear all button
        clearAllButton.setButtonText("Clear All");
        clearAllButton.onClick = [this] 
        { 
            learnManager.clearAllMappings();
            table.updateContent();
        };
        addAndMakeVisible(clearAllButton);
    }
    
    void refresh()
    {
        table.updateContent();
    }
    
    int getNumRows() override
    {
        return learnManager.getMappingCount();
    }
    
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll(DS::Color::accent.withAlpha(0.3f));
        else if (rowNumber % 2)
            g.fillAll(DS::Color::surfaceAlt.withAlpha(0.3f));
    }
    
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override
    {
        const auto& mappings = learnManager.getAllMappings();
        if (rowNumber >= static_cast<int>(mappings.size()))
            return;
        
        const auto& mapping = mappings[rowNumber];
        
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::body);
        
        juce::String text;
        switch (columnId)
        {
            case 1: text = mapping.parameterId; break;
            case 2: text = juce::String(mapping.ccNumber); break;
            case 3: text = mapping.channel == 0 ? "Any" : juce::String(mapping.channel); break;
            case 4: text = juce::String(static_cast<int>(mapping.minValue * 127)) + "-" + 
                           juce::String(static_cast<int>(mapping.maxValue * 127)); break;
        }
        
        g.drawText(text, 4, 0, width - 8, height, juce::Justification::centredLeft);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        clearAllButton.setBounds(bounds.removeFromBottom(30).reduced(4));
        table.setBounds(bounds);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), DS::Radius::medium);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), DS::Radius::medium, 1.0f);
    }
    
private:
    LearnManager& learnManager;
    juce::TableListBox table;
    juce::TextButton clearAllButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingsPanel)
};

//==============================================================================
/**
 * CC Activity Display
 * Shows incoming MIDI CC activity
 */
class CCActivityDisplay : public juce::Component, private juce::Timer
{
public:
    CCActivityDisplay()
    {
        startTimerHz(15);
    }
    
    ~CCActivityDisplay() override
    {
        stopTimer();
    }
    
    void handleCC(int channel, int cc, int value)
    {
        lastCC = cc;
        lastChannel = channel;
        lastValue = value;
        activityTime = 1.0f;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, DS::Radius::small);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::small, 1.0f);
        
        // Activity indicator
        if (activityTime > 0)
        {
            float alpha = activityTime;
            g.setColour(juce::Colour(0xFF00C853).withAlpha(alpha));
            g.fillEllipse(bounds.getX() + 4, bounds.getCentreY() - 4, 8, 8);
        }
        
        // CC info
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::body);
        
        if (lastCC >= 0)
        {
            juce::String text = "CC" + juce::String(lastCC) + ": " + juce::String(lastValue);
            g.drawText(text, bounds.reduced(16, 0), juce::Justification::centredLeft);
            
            g.setColour(DS::Color::textMuted);
            g.setFont(DS::Font::label);
            g.drawText("Ch " + juce::String(lastChannel), bounds.reduced(4), juce::Justification::centredRight);
        }
        else
        {
            g.setColour(DS::Color::textMuted);
            g.drawText("No CC activity", bounds, juce::Justification::centred);
        }
    }
    
private:
    void timerCallback() override
    {
        if (activityTime > 0)
        {
            activityTime -= 0.1f;
            repaint();
        }
    }
    
    int lastCC = -1;
    int lastChannel = 0;
    int lastValue = 0;
    float activityTime = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CCActivityDisplay)
};

} // namespace MIDILearn
