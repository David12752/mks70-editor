#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * MultiSynthSupport.h
 * Support for multiple MKS-70 units
 * 
 * Part of Improvement #13: Multi-Synth Support
 */

namespace MultiSynth
{

//==============================================================================
/**
 * Synth unit info
 */
struct SynthUnit
{
    juce::String id;
    juce::String name;
    juce::String midiInputDevice;
    juce::String midiOutputDevice;
    int midiChannel = 1;
    int deviceId = 0;  // Roland Device ID
    bool connected = false;
    juce::Time lastSeen;
    
    // Unit-specific settings
    bool enabled = true;
    juce::Colour color = juce::Colour(0xFF6366f1);
};

//==============================================================================
/**
 * Multi-Synth Manager - manages multiple MKS-70 units
 */
class MultiSynthManager
{
public:
    MultiSynthManager()
    {
        // Create default unit
        addUnit("Default MKS-70");
    }
    
    // Unit management
    void addUnit(const juce::String& name)
    {
        SynthUnit unit;
        unit.id = juce::Uuid().toString();
        unit.name = name;
        unit.midiChannel = 1;
        unit.deviceId = units.size();
        unit.color = getColorForIndex(static_cast<int>(units.size()));
        
        units.push_back(unit);
        notifyListeners();
    }
    
    void removeUnit(const juce::String& unitId)
    {
        units.erase(
            std::remove_if(units.begin(), units.end(),
                [&](const SynthUnit& u) { return u.id == unitId; }),
            units.end()
        );
        
        if (activeUnitId == unitId && !units.empty())
            activeUnitId = units[0].id;
        
        notifyListeners();
    }
    
    void renameUnit(const juce::String& unitId, const juce::String& newName)
    {
        auto* unit = getUnit(unitId);
        if (unit)
        {
            unit->name = newName;
            notifyListeners();
        }
    }
    
    // Active unit
    void setActiveUnit(const juce::String& unitId)
    {
        if (getUnit(unitId) != nullptr)
        {
            activeUnitId = unitId;
            notifyListeners();
        }
    }
    
    SynthUnit* getActiveUnit()
    {
        return getUnit(activeUnitId);
    }
    
    const SynthUnit* getActiveUnit() const
    {
        return getUnit(activeUnitId);
    }
    
    juce::String getActiveUnitId() const { return activeUnitId; }
    
    // Unit access
    SynthUnit* getUnit(const juce::String& unitId)
    {
        for (auto& unit : units)
            if (unit.id == unitId)
                return &unit;
        return nullptr;
    }
    
    const SynthUnit* getUnit(const juce::String& unitId) const
    {
        for (const auto& unit : units)
            if (unit.id == unitId)
                return &unit;
        return nullptr;
    }
    
    const std::vector<SynthUnit>& getUnits() const { return units; }
    int getUnitCount() const { return static_cast<int>(units.size()); }
    
    // MIDI configuration
    void setUnitMidiInput(const juce::String& unitId, const juce::String& deviceName)
    {
        auto* unit = getUnit(unitId);
        if (unit)
        {
            unit->midiInputDevice = deviceName;
            notifyListeners();
        }
    }
    
    void setUnitMidiOutput(const juce::String& unitId, const juce::String& deviceName)
    {
        auto* unit = getUnit(unitId);
        if (unit)
        {
            unit->midiOutputDevice = deviceName;
            notifyListeners();
        }
    }
    
    void setUnitMidiChannel(const juce::String& unitId, int channel)
    {
        auto* unit = getUnit(unitId);
        if (unit)
        {
            unit->midiChannel = juce::jlimit(1, 16, channel);
            notifyListeners();
        }
    }
    
    void setUnitDeviceId(const juce::String& unitId, int devId)
    {
        auto* unit = getUnit(unitId);
        if (unit)
        {
            unit->deviceId = juce::jlimit(0, 127, devId);
            notifyListeners();
        }
    }
    
    // Connection status
    void updateConnectionStatus(const juce::String& unitId, bool connected)
    {
        auto* unit = getUnit(unitId);
        if (unit)
        {
            unit->connected = connected;
            if (connected)
                unit->lastSeen = juce::Time::getCurrentTime();
            notifyListeners();
        }
    }
    
    // Send to all units
    void sendToAllUnits(const juce::MidiMessage& message)
    {
        for (const auto& unit : units)
        {
            if (unit.enabled && unit.connected)
            {
                // In real implementation, send to each unit's MIDI output
                juce::ignoreUnused(message);
            }
        }
    }
    
    // Save/load state
    juce::ValueTree saveState() const
    {
        juce::ValueTree state("MultiSynthManager");
        state.setProperty("activeUnit", activeUnitId, nullptr);
        
        for (const auto& unit : units)
        {
            juce::ValueTree unitState("Unit");
            unitState.setProperty("id", unit.id, nullptr);
            unitState.setProperty("name", unit.name, nullptr);
            unitState.setProperty("midiInput", unit.midiInputDevice, nullptr);
            unitState.setProperty("midiOutput", unit.midiOutputDevice, nullptr);
            unitState.setProperty("midiChannel", unit.midiChannel, nullptr);
            unitState.setProperty("deviceId", unit.deviceId, nullptr);
            unitState.setProperty("enabled", unit.enabled, nullptr);
            unitState.setProperty("color", unit.color.toString(), nullptr);
            state.appendChild(unitState, nullptr);
        }
        
        return state;
    }
    
    void loadState(const juce::ValueTree& state)
    {
        if (!state.hasType("MultiSynthManager"))
            return;
        
        units.clear();
        
        for (int i = 0; i < state.getNumChildren(); i++)
        {
            auto unitState = state.getChild(i);
            if (unitState.hasType("Unit"))
            {
                SynthUnit unit;
                unit.id = unitState.getProperty("id");
                unit.name = unitState.getProperty("name");
                unit.midiInputDevice = unitState.getProperty("midiInput");
                unit.midiOutputDevice = unitState.getProperty("midiOutput");
                unit.midiChannel = unitState.getProperty("midiChannel");
                unit.deviceId = unitState.getProperty("deviceId");
                unit.enabled = unitState.getProperty("enabled");
                unit.color = juce::Colour::fromString(unitState.getProperty("color").toString());
                units.push_back(unit);
            }
        }
        
        activeUnitId = state.getProperty("activeUnit");
        
        if (units.empty())
            addUnit("Default MKS-70");
        
        notifyListeners();
    }
    
    // Listener support
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void synthUnitsChanged() = 0;
    };
    
    void addListener(Listener* l) { listeners.push_back(l); }
    void removeListener(Listener* l) 
    {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), l), listeners.end());
    }
    
private:
    juce::Colour getColorForIndex(int index)
    {
        const juce::Colour colors[] = {
            juce::Colour(0xFF6366f1),  // Indigo
            juce::Colour(0xFF10b981),  // Green
            juce::Colour(0xFFf59e0b),  // Orange
            juce::Colour(0xFFef4444),  // Red
            juce::Colour(0xFF8b5cf6),  // Purple
            juce::Colour(0xFF06b6d4),  // Cyan
        };
        return colors[index % 6];
    }
    
    void notifyListeners()
    {
        for (auto* l : listeners)
            l->synthUnitsChanged();
    }
    
    std::vector<SynthUnit> units;
    juce::String activeUnitId;
    std::vector<Listener*> listeners;
};

//==============================================================================
/**
 * Unit Selector - dropdown for selecting active synth unit
 */
class UnitSelector : public juce::Component, public MultiSynthManager::Listener
{
public:
    UnitSelector(MultiSynthManager& mgr)
        : manager(mgr)
    {
        manager.addListener(this);
        updateItems();
        
        selector.onChange = [this]() {
            int idx = selector.getSelectedItemIndex();
            if (idx >= 0 && idx < static_cast<int>(manager.getUnits().size()))
            {
                manager.setActiveUnit(manager.getUnits()[idx].id);
            }
        };
        addAndMakeVisible(selector);
    }
    
    ~UnitSelector() override
    {
        manager.removeListener(this);
    }
    
    void synthUnitsChanged() override
    {
        updateItems();
    }
    
    void resized() override
    {
        selector.setBounds(getLocalBounds());
    }
    
private:
    void updateItems()
    {
        selector.clear();
        int selectedIdx = 0;
        
        const auto& units = manager.getUnits();
        for (int i = 0; i < static_cast<int>(units.size()); i++)
        {
            const auto& unit = units[i];
            juce::String itemText = unit.name;
            if (unit.connected)
                itemText += " ●";
            
            selector.addItem(itemText, i + 1);
            
            if (unit.id == manager.getActiveUnitId())
                selectedIdx = i;
        }
        
        selector.setSelectedItemIndex(selectedIdx, juce::dontSendNotification);
    }
    
    MultiSynthManager& manager;
    juce::ComboBox selector;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnitSelector)
};

//==============================================================================
/**
 * Unit Configuration Panel
 */
class UnitConfigPanel : public juce::Component
{
public:
    UnitConfigPanel(MultiSynthManager& mgr)
        : manager(mgr)
    {
        // Name field
        nameField.setTextToShowWhenEmpty("Unit name...", DS::Color::textDim);
        nameField.onTextChange = [this]() {
            if (currentUnitId.isNotEmpty())
                manager.renameUnit(currentUnitId, nameField.getText());
        };
        addAndMakeVisible(nameField);
        
        // MIDI Input
        midiInputSelector.onChange = [this]() {
            if (currentUnitId.isNotEmpty())
            {
                int idx = midiInputSelector.getSelectedItemIndex();
                if (idx >= 0)
                    manager.setUnitMidiInput(currentUnitId, midiInputSelector.getItemText(idx));
            }
        };
        addAndMakeVisible(midiInputSelector);
        
        // MIDI Output
        midiOutputSelector.onChange = [this]() {
            if (currentUnitId.isNotEmpty())
            {
                int idx = midiOutputSelector.getSelectedItemIndex();
                if (idx >= 0)
                    manager.setUnitMidiOutput(currentUnitId, midiOutputSelector.getItemText(idx));
            }
        };
        addAndMakeVisible(midiOutputSelector);
        
        // MIDI Channel
        for (int i = 1; i <= 16; i++)
            midiChannelSelector.addItem("Channel " + juce::String(i), i);
        midiChannelSelector.onChange = [this]() {
            if (currentUnitId.isNotEmpty())
                manager.setUnitMidiChannel(currentUnitId, midiChannelSelector.getSelectedId());
        };
        addAndMakeVisible(midiChannelSelector);
        
        // Device ID
        for (int i = 0; i < 128; i++)
            deviceIdSelector.addItem("ID " + juce::String(i), i + 1);
        deviceIdSelector.onChange = [this]() {
            if (currentUnitId.isNotEmpty())
                manager.setUnitDeviceId(currentUnitId, deviceIdSelector.getSelectedId() - 1);
        };
        addAndMakeVisible(deviceIdSelector);
        
        refreshMidiDevices();
    }
    
    void setUnit(const juce::String& unitId)
    {
        currentUnitId = unitId;
        
        const auto* unit = manager.getUnit(unitId);
        if (unit)
        {
            nameField.setText(unit->name, false);
            midiChannelSelector.setSelectedId(unit->midiChannel, juce::dontSendNotification);
            deviceIdSelector.setSelectedId(unit->deviceId + 1, juce::dontSendNotification);
            
            // Select MIDI devices
            for (int i = 0; i < midiInputSelector.getNumItems(); i++)
            {
                if (midiInputSelector.getItemText(i) == unit->midiInputDevice)
                    midiInputSelector.setSelectedItemIndex(i, juce::dontSendNotification);
            }
            for (int i = 0; i < midiOutputSelector.getNumItems(); i++)
            {
                if (midiOutputSelector.getItemText(i) == unit->midiOutputDevice)
                    midiOutputSelector.setSelectedItemIndex(i, juce::dontSendNotification);
            }
        }
    }
    
    void refreshMidiDevices()
    {
        midiInputSelector.clear();
        midiOutputSelector.clear();
        
        auto inputs = juce::MidiInput::getAvailableDevices();
        for (const auto& device : inputs)
            midiInputSelector.addItem(device.name, midiInputSelector.getNumItems() + 1);
        
        auto outputs = juce::MidiOutput::getAvailableDevices();
        for (const auto& device : outputs)
            midiOutputSelector.addItem(device.name, midiOutputSelector.getNumItems() + 1);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        int rowHeight = 28;
        int labelWidth = 100;
        
        // Title
        bounds.removeFromTop(24);
        bounds.removeFromTop(DS::Space::sm);
        
        // Name
        auto row = bounds.removeFromTop(rowHeight);
        row.removeFromLeft(labelWidth);
        nameField.setBounds(row);
        bounds.removeFromTop(DS::Space::sm);
        
        // MIDI Input
        row = bounds.removeFromTop(rowHeight);
        row.removeFromLeft(labelWidth);
        midiInputSelector.setBounds(row);
        bounds.removeFromTop(DS::Space::sm);
        
        // MIDI Output
        row = bounds.removeFromTop(rowHeight);
        row.removeFromLeft(labelWidth);
        midiOutputSelector.setBounds(row);
        bounds.removeFromTop(DS::Space::sm);
        
        // MIDI Channel
        row = bounds.removeFromTop(rowHeight);
        row.removeFromLeft(labelWidth);
        midiChannelSelector.setBounds(row.removeFromLeft(150));
        bounds.removeFromTop(DS::Space::sm);
        
        // Device ID
        row = bounds.removeFromTop(rowHeight);
        row.removeFromLeft(labelWidth);
        deviceIdSelector.setBounds(row.removeFromLeft(150));
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        auto contentBounds = bounds.reduced(DS::Space::md);
        int labelWidth = 100;
        int rowHeight = 28;
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading3);
        g.drawText("Unit Configuration", contentBounds.removeFromTop(24), juce::Justification::centredLeft);
        contentBounds.removeFromTop(DS::Space::sm);
        
        // Labels
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::body);
        
        g.drawText("Name:", contentBounds.removeFromTop(rowHeight).removeFromLeft(labelWidth), juce::Justification::centredLeft);
        contentBounds.removeFromTop(DS::Space::sm);
        
        g.drawText("MIDI Input:", contentBounds.removeFromTop(rowHeight).removeFromLeft(labelWidth), juce::Justification::centredLeft);
        contentBounds.removeFromTop(DS::Space::sm);
        
        g.drawText("MIDI Output:", contentBounds.removeFromTop(rowHeight).removeFromLeft(labelWidth), juce::Justification::centredLeft);
        contentBounds.removeFromTop(DS::Space::sm);
        
        g.drawText("MIDI Channel:", contentBounds.removeFromTop(rowHeight).removeFromLeft(labelWidth), juce::Justification::centredLeft);
        contentBounds.removeFromTop(DS::Space::sm);
        
        g.drawText("Device ID:", contentBounds.removeFromTop(rowHeight).removeFromLeft(labelWidth), juce::Justification::centredLeft);
    }
    
private:
    MultiSynthManager& manager;
    juce::String currentUnitId;
    
    juce::TextEditor nameField;
    juce::ComboBox midiInputSelector;
    juce::ComboBox midiOutputSelector;
    juce::ComboBox midiChannelSelector;
    juce::ComboBox deviceIdSelector;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnitConfigPanel)
};

//==============================================================================
/**
 * Multi-Synth Panel - full management UI
 */
class MultiSynthPanel : public juce::Component, public MultiSynthManager::Listener
{
public:
    MultiSynthPanel(MultiSynthManager& mgr)
        : manager(mgr), unitConfig(mgr)
    {
        manager.addListener(this);
        
        // Add unit button
        addButton.setButtonText("+ Add Unit");
        addButton.onClick = [this]() {
            manager.addUnit("MKS-70 " + juce::String(manager.getUnitCount() + 1));
        };
        addAndMakeVisible(addButton);
        
        // Remove unit button
        removeButton.setButtonText("Remove");
        removeButton.onClick = [this]() {
            if (manager.getUnitCount() > 1)
                manager.removeUnit(manager.getActiveUnitId());
        };
        addAndMakeVisible(removeButton);
        
        addAndMakeVisible(unitConfig);
        
        updateUnitList();
    }
    
    ~MultiSynthPanel() override
    {
        manager.removeListener(this);
    }
    
    void synthUnitsChanged() override
    {
        updateUnitList();
        unitConfig.setUnit(manager.getActiveUnitId());
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        
        // Title
        bounds.removeFromTop(30);
        bounds.removeFromTop(DS::Space::sm);
        
        // Unit list on left
        auto leftPanel = bounds.removeFromLeft(200);
        
        auto buttonRow = leftPanel.removeFromTop(28);
        addButton.setBounds(buttonRow.removeFromLeft(100));
        buttonRow.removeFromLeft(DS::Space::sm);
        removeButton.setBounds(buttonRow);
        
        leftPanel.removeFromTop(DS::Space::sm);
        unitListBounds = leftPanel;
        
        bounds.removeFromLeft(DS::Space::md);
        
        // Config panel on right
        unitConfig.setBounds(bounds);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::large);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::large, 1.0f);
        
        auto contentBounds = bounds.reduced(DS::Space::md);
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading2);
        g.drawText("Multi-Synth Manager", contentBounds.removeFromTop(30), juce::Justification::centredLeft);
        contentBounds.removeFromTop(DS::Space::sm);
        
        // Unit list
        auto listBounds = contentBounds.removeFromLeft(200);
        listBounds.removeFromTop(28 + DS::Space::sm);  // Button row
        
        g.setFont(DS::Font::body);
        
        const auto& units = manager.getUnits();
        int itemHeight = 40;
        
        for (int i = 0; i < static_cast<int>(units.size()); i++)
        {
            const auto& unit = units[i];
            auto itemBounds = juce::Rectangle<float>(
                listBounds.getX(), listBounds.getY() + i * (itemHeight + 4),
                listBounds.getWidth(), static_cast<float>(itemHeight)
            );
            
            bool isActive = unit.id == manager.getActiveUnitId();
            
            // Background
            if (isActive)
                g.setColour(DS::Color::accent.withAlpha(0.2f));
            else
                g.setColour(DS::Color::surfaceAlt);
            
            g.fillRoundedRectangle(itemBounds, DS::Radius::small);
            
            // Border
            if (isActive)
                g.setColour(DS::Color::accent);
            else
                g.setColour(DS::Color::border);
            g.drawRoundedRectangle(itemBounds, DS::Radius::small, 1.0f);
            
            // Color indicator
            g.setColour(unit.color);
            g.fillEllipse(itemBounds.getX() + 8, itemBounds.getCentreY() - 6, 12, 12);
            
            // Name
            g.setColour(DS::Color::text);
            g.drawText(unit.name, itemBounds.withTrimmedLeft(28).withTrimmedRight(24),
                       juce::Justification::centredLeft);
            
            // Connection status
            if (unit.connected)
            {
                g.setColour(juce::Colour(0xFF10b981));
                g.fillEllipse(itemBounds.getRight() - 16, itemBounds.getCentreY() - 4, 8, 8);
            }
        }
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        // Check if click is in unit list
        auto localPos = e.getPosition();
        
        if (unitListBounds.contains(localPos))
        {
            int itemHeight = 40;
            int index = (localPos.y - unitListBounds.getY()) / (itemHeight + 4);
            
            const auto& units = manager.getUnits();
            if (index >= 0 && index < static_cast<int>(units.size()))
            {
                manager.setActiveUnit(units[index].id);
            }
        }
    }
    
private:
    void updateUnitList()
    {
        repaint();
    }
    
    MultiSynthManager& manager;
    UnitConfigPanel unitConfig;
    juce::TextButton addButton;
    juce::TextButton removeButton;
    juce::Rectangle<int> unitListBounds;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiSynthPanel)
};

} // namespace MultiSynth
