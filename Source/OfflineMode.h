#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * OfflineMode.h
 * Work without synth connected
 * 
 * Part of Improvement #14: Offline Mode
 */

namespace Offline
{

//==============================================================================
/**
 * Connection state
 */
enum class ConnectionState
{
    Connected,       // Synth is connected and responding
    Disconnected,    // No synth detected
    Offline,         // User explicitly in offline mode
    Connecting       // Attempting to connect
};

//==============================================================================
/**
 * Pending change - stored for sync when reconnected
 */
struct PendingChange
{
    juce::String paramId;
    int value;
    juce::Time timestamp;
    bool sent = false;
};

//==============================================================================
/**
 * Offline Manager - manages offline operation mode
 */
class OfflineManager
{
public:
    OfflineManager()
    {
        connectionState = ConnectionState::Disconnected;
        offlineMode = false;
    }
    
    // Connection state
    void setConnectionState(ConnectionState state)
    {
        connectionState = state;
        notifyListeners();
        
        // When reconnected, sync pending changes
        if (state == ConnectionState::Connected && !pendingChanges.empty())
        {
            syncPendingChanges();
        }
    }
    
    ConnectionState getConnectionState() const { return connectionState; }
    
    bool isConnected() const { return connectionState == ConnectionState::Connected; }
    
    // Offline mode toggle
    void setOfflineMode(bool offline)
    {
        offlineMode = offline;
        
        if (offline)
            connectionState = ConnectionState::Offline;
        else if (connectionState == ConnectionState::Offline)
            connectionState = ConnectionState::Disconnected;
        
        notifyListeners();
    }
    
    bool isOfflineMode() const { return offlineMode; }
    
    // Can we edit? (Yes, always - changes are queued when offline)
    bool canEdit() const { return true; }
    
    // Queue parameter change
    void queueChange(const juce::String& paramId, int value)
    {
        if (isConnected())
        {
            // Send immediately
            sendToSynth(paramId, value);
        }
        else
        {
            // Queue for later
            PendingChange change;
            change.paramId = paramId;
            change.value = value;
            change.timestamp = juce::Time::getCurrentTime();
            
            // Replace existing change for same param
            for (auto& pc : pendingChanges)
            {
                if (pc.paramId == paramId)
                {
                    pc = change;
                    return;
                }
            }
            
            pendingChanges.push_back(change);
            notifyListeners();
        }
    }
    
    // Get pending changes
    const std::vector<PendingChange>& getPendingChanges() const { return pendingChanges; }
    int getPendingChangeCount() const { return static_cast<int>(pendingChanges.size()); }
    
    // Clear pending changes (e.g., if user discards them)
    void clearPendingChanges()
    {
        pendingChanges.clear();
        notifyListeners();
    }
    
    // Sync pending changes when reconnected
    void syncPendingChanges()
    {
        for (auto& change : pendingChanges)
        {
            sendToSynth(change.paramId, change.value);
            change.sent = true;
        }
        
        pendingChanges.clear();
        notifyListeners();
    }
    
    // Manual sync request
    void requestSync()
    {
        if (isConnected() && !pendingChanges.empty())
        {
            syncPendingChanges();
        }
    }
    
    // Check synth presence
    void pingSync()
    {
        // In real implementation, send identity request
        // For now, just simulate
        if (!offlineMode)
        {
            connectionState = ConnectionState::Connecting;
            notifyListeners();
            
            // Would normally have callback when response received
            // or timeout to set Disconnected
        }
    }
    
    // Status text
    juce::String getStatusText() const
    {
        switch (connectionState)
        {
            case ConnectionState::Connected:
                return "Connected";
            case ConnectionState::Disconnected:
                return "Disconnected";
            case ConnectionState::Offline:
                return "Offline Mode";
            case ConnectionState::Connecting:
                return "Connecting...";
            default:
                return "Unknown";
        }
    }
    
    // Save/load state
    juce::ValueTree saveState() const
    {
        juce::ValueTree state("OfflineMode");
        state.setProperty("offlineMode", offlineMode, nullptr);
        
        for (const auto& change : pendingChanges)
        {
            juce::ValueTree changeState("PendingChange");
            changeState.setProperty("param", change.paramId, nullptr);
            changeState.setProperty("value", change.value, nullptr);
            changeState.setProperty("time", change.timestamp.toMilliseconds(), nullptr);
            state.appendChild(changeState, nullptr);
        }
        
        return state;
    }
    
    void loadState(const juce::ValueTree& state)
    {
        if (!state.hasType("OfflineMode"))
            return;
        
        offlineMode = state.getProperty("offlineMode");
        pendingChanges.clear();
        
        for (int i = 0; i < state.getNumChildren(); i++)
        {
            auto changeState = state.getChild(i);
            if (changeState.hasType("PendingChange"))
            {
                PendingChange change;
                change.paramId = changeState.getProperty("param");
                change.value = changeState.getProperty("value");
                change.timestamp = juce::Time(static_cast<int64_t>(changeState.getProperty("time")));
                pendingChanges.push_back(change);
            }
        }
        
        notifyListeners();
    }
    
    // Listener support
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void connectionStateChanged() = 0;
    };
    
    void addListener(Listener* l) { listeners.push_back(l); }
    void removeListener(Listener* l) 
    {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), l), listeners.end());
    }
    
private:
    void sendToSynth(const juce::String& /*paramId*/, int /*value*/)
    {
        // In real implementation, send MIDI to synth
        // This would be connected to MidiHandler
    }
    
    void notifyListeners()
    {
        for (auto* l : listeners)
            l->connectionStateChanged();
    }
    
    ConnectionState connectionState;
    bool offlineMode;
    std::vector<PendingChange> pendingChanges;
    std::vector<Listener*> listeners;
};

//==============================================================================
/**
 * Connection Status Indicator
 */
class ConnectionIndicator : public juce::Component, public OfflineManager::Listener
{
public:
    ConnectionIndicator(OfflineManager& mgr)
        : manager(mgr)
    {
        manager.addListener(this);
    }
    
    ~ConnectionIndicator() override
    {
        manager.removeListener(this);
    }
    
    void connectionStateChanged() override
    {
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, DS::Radius::small);
        
        // Status LED
        auto state = manager.getConnectionState();
        juce::Colour ledColor;
        
        switch (state)
        {
            case ConnectionState::Connected:
                ledColor = juce::Colour(0xFF10b981);  // Green
                break;
            case ConnectionState::Connecting:
                ledColor = juce::Colour(0xFFf59e0b);  // Yellow
                break;
            case ConnectionState::Offline:
                ledColor = juce::Colour(0xFF6366f1);  // Purple
                break;
            default:
                ledColor = juce::Colour(0xFFef4444);  // Red
                break;
        }
        
        float ledSize = juce::jmin(bounds.getHeight() - 8, 12.0f);
        auto ledBounds = juce::Rectangle<float>(
            bounds.getX() + 6,
            bounds.getCentreY() - ledSize / 2,
            ledSize, ledSize
        );
        
        // LED glow
        g.setColour(ledColor.withAlpha(0.3f));
        g.fillEllipse(ledBounds.expanded(3));
        
        g.setColour(ledColor);
        g.fillEllipse(ledBounds);
        
        g.setColour(ledColor.brighter(0.5f));
        g.fillEllipse(ledBounds.reduced(ledSize * 0.25f));
        
        // Status text
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::label);
        g.drawText(manager.getStatusText(),
                   bounds.withTrimmedLeft(ledSize + 12).withTrimmedRight(4),
                   juce::Justification::centredLeft);
        
        // Pending count
        if (manager.getPendingChangeCount() > 0)
        {
            auto badge = juce::Rectangle<float>(
                bounds.getRight() - 20,
                bounds.getCentreY() - 8,
                16, 16
            );
            
            g.setColour(juce::Colour(0xFFf59e0b));
            g.fillEllipse(badge);
            
            g.setColour(DS::Color::surface);
            g.setFont(DS::Font::label.withHeight(10.0f));
            g.drawText(juce::String(manager.getPendingChangeCount()),
                       badge, juce::Justification::centred);
        }
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::small, 1.0f);
    }
    
    void mouseDown(const juce::MouseEvent&) override
    {
        // Toggle offline mode or try to reconnect
        if (manager.isOfflineMode())
            manager.setOfflineMode(false);
        else if (!manager.isConnected())
            manager.pingSync();
    }
    
private:
    OfflineManager& manager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionIndicator)
};

//==============================================================================
/**
 * Pending Changes Panel - shows queued changes
 */
class PendingChangesPanel : public juce::Component, public OfflineManager::Listener
{
public:
    PendingChangesPanel(OfflineManager& mgr)
        : manager(mgr)
    {
        manager.addListener(this);
        
        // Sync button
        syncButton.setButtonText("Sync Now");
        syncButton.onClick = [this]() { manager.requestSync(); };
        addAndMakeVisible(syncButton);
        
        // Clear button
        clearButton.setButtonText("Discard Changes");
        clearButton.onClick = [this]() { manager.clearPendingChanges(); };
        addAndMakeVisible(clearButton);
        
        updateVisibility();
    }
    
    ~PendingChangesPanel() override
    {
        manager.removeListener(this);
    }
    
    void connectionStateChanged() override
    {
        updateVisibility();
        repaint();
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        
        // Title
        bounds.removeFromTop(24);
        bounds.removeFromTop(DS::Space::sm);
        
        // List area
        bounds.removeFromBottom(32 + DS::Space::sm);
        listBounds = bounds;
        
        // Buttons
        bounds = getLocalBounds().reduced(DS::Space::md);
        auto buttonRow = bounds.removeFromBottom(32);
        syncButton.setBounds(buttonRow.removeFromLeft(100));
        buttonRow.removeFromLeft(DS::Space::sm);
        clearButton.setBounds(buttonRow.removeFromLeft(120));
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
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading3);
        g.drawText("Pending Changes", contentBounds.removeFromTop(24), juce::Justification::centredLeft);
        contentBounds.removeFromTop(DS::Space::sm);
        
        // Changes list
        const auto& changes = manager.getPendingChanges();
        
        if (changes.empty())
        {
            g.setColour(DS::Color::textMuted);
            g.setFont(DS::Font::body);
            g.drawText("No pending changes", listBounds.toFloat(), juce::Justification::centred);
        }
        else
        {
            int rowHeight = 24;
            g.setFont(DS::Font::label);
            
            for (int i = 0; i < static_cast<int>(changes.size()) && i < 10; i++)
            {
                const auto& change = changes[i];
                auto rowBounds = juce::Rectangle<float>(
                    listBounds.getX(), static_cast<float>(listBounds.getY() + i * rowHeight),
                    static_cast<float>(listBounds.getWidth()), static_cast<float>(rowHeight)
                );
                
                // Parameter name
                g.setColour(DS::Color::text);
                g.drawText(change.paramId, rowBounds.removeFromLeft(150), juce::Justification::centredLeft);
                
                // Value
                g.setColour(DS::Color::accent);
                g.drawText(juce::String(change.value), rowBounds.removeFromLeft(60), juce::Justification::centredLeft);
                
                // Time
                g.setColour(DS::Color::textMuted);
                g.drawText(change.timestamp.toString(false, true, false),
                           rowBounds, juce::Justification::centredRight);
            }
            
            if (static_cast<int>(changes.size()) > 10)
            {
                g.setColour(DS::Color::textMuted);
                g.drawText("... and " + juce::String(changes.size() - 10) + " more",
                           listBounds.toFloat().removeFromBottom(24), juce::Justification::centred);
            }
        }
    }
    
private:
    void updateVisibility()
    {
        bool hasPending = manager.getPendingChangeCount() > 0;
        syncButton.setEnabled(hasPending && manager.isConnected());
        clearButton.setEnabled(hasPending);
    }
    
    OfflineManager& manager;
    juce::TextButton syncButton;
    juce::TextButton clearButton;
    juce::Rectangle<int> listBounds;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PendingChangesPanel)
};

//==============================================================================
/**
 * Offline Mode Panel - full offline management UI
 */
class OfflineModePanel : public juce::Component, public OfflineManager::Listener
{
public:
    OfflineModePanel(OfflineManager& mgr)
        : manager(mgr), indicator(mgr), pendingPanel(mgr)
    {
        manager.addListener(this);
        
        addAndMakeVisible(indicator);
        addAndMakeVisible(pendingPanel);
        
        // Offline mode toggle
        offlineToggle.setButtonText("Enable Offline Mode");
        offlineToggle.setToggleState(manager.isOfflineMode(), juce::dontSendNotification);
        offlineToggle.onClick = [this]() {
            manager.setOfflineMode(offlineToggle.getToggleState());
        };
        addAndMakeVisible(offlineToggle);
        
        // Reconnect button
        reconnectButton.setButtonText("Try Reconnect");
        reconnectButton.onClick = [this]() { manager.pingSync(); };
        addAndMakeVisible(reconnectButton);
    }
    
    ~OfflineModePanel() override
    {
        manager.removeListener(this);
    }
    
    void connectionStateChanged() override
    {
        offlineToggle.setToggleState(manager.isOfflineMode(), juce::dontSendNotification);
        reconnectButton.setEnabled(!manager.isConnected() && !manager.isOfflineMode());
        repaint();
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        
        // Title
        bounds.removeFromTop(30);
        bounds.removeFromTop(DS::Space::sm);
        
        // Status indicator
        indicator.setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(DS::Space::sm);
        
        // Toggle and reconnect
        auto row = bounds.removeFromTop(28);
        offlineToggle.setBounds(row.removeFromLeft(180));
        row.removeFromLeft(DS::Space::sm);
        reconnectButton.setBounds(row.removeFromLeft(120));
        bounds.removeFromTop(DS::Space::md);
        
        // Description
        bounds.removeFromTop(60);
        bounds.removeFromTop(DS::Space::md);
        
        // Pending changes panel
        pendingPanel.setBounds(bounds);
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
        g.drawText("Connection & Offline Mode", contentBounds.removeFromTop(30), juce::Justification::centredLeft);
        
        contentBounds.removeFromTop(DS::Space::sm);
        contentBounds.removeFromTop(28); // indicator
        contentBounds.removeFromTop(DS::Space::sm);
        contentBounds.removeFromTop(28); // toggle row
        contentBounds.removeFromTop(DS::Space::md);
        
        // Description
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::body);
        g.drawMultiLineText(
            "Offline mode allows you to edit patches without a connected synth. "
            "Changes will be queued and sent automatically when the synth is reconnected. "
            "You can also manually sync pending changes at any time.",
            static_cast<int>(contentBounds.getX()),
            static_cast<int>(contentBounds.getY()) + 12,
            static_cast<int>(contentBounds.getWidth())
        );
    }
    
private:
    OfflineManager& manager;
    ConnectionIndicator indicator;
    PendingChangesPanel pendingPanel;
    juce::ToggleButton offlineToggle;
    juce::TextButton reconnectButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OfflineModePanel)
};

} // namespace Offline
