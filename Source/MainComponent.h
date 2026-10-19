/*
  ==============================================================================

    MainComponent.h
    Main component that assembles all MKS-70 Editor UI parts
    
    Uses the modern DS:: design system for consistent styling
    
    Layout:
    - Mini title bar at top (26px) with logo + preset navigation
    - Vertical tabs on left (icons/short names)
    - Bottom toolbar with MIDI, A/B, Settings etc.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "MKS70LookAndFeel.h"
#include "TonePanel.h"
#include "PatchPanel.h"
#include "ArpPanel.h"
#include "LibrarianTab.h"
#include "SettingsTab.h"
#include "ComparePanel.h"
#include "UndoManager.h"
#include "KeyboardShortcuts.h"

//==============================================================================
// Minimal Title Bar - Only logo + preset navigation
//==============================================================================
class MiniTitleBar : public juce::Component
{
public:
    MiniTitleBar()
    {
        // Logo/Title
        titleLabel.setText("MKS-70", juce::dontSendNotification);
        titleLabel.setFont(DS::Font::bold(14.0f));
        titleLabel.setColour(juce::Label::textColourId, DS::Color::Accent::primary());
        addAndMakeVisible(titleLabel);
        
        // Preset navigation
        prevBtn.setButtonText("<");
        prevBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        prevBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::cyan());
        prevBtn.setTooltip("Previous preset");
        prevBtn.onClick = [this]() { navigatePreset(-1); };
        addAndMakeVisible(prevBtn);
        
        patchName.setText("INIT PATCH", juce::dontSendNotification);
        patchName.setFont(DS::Font::medium());
        patchName.setColour(juce::Label::textColourId, DS::Color::Accent::cyan());
        patchName.setJustificationType(juce::Justification::centred);
        patchName.addMouseListener(this, false);
        addAndMakeVisible(patchName);
        
        nextBtn.setButtonText(">");
        nextBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        nextBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::cyan());
        nextBtn.setTooltip("Next preset");
        nextBtn.onClick = [this]() { navigatePreset(1); };
        addAndMakeVisible(nextBtn);
        
        menuBtn.setButtonText("...");
        menuBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        menuBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        menuBtn.setTooltip("Preset browser");
        menuBtn.onClick = [this]() { showPresetMenu(); };
        addAndMakeVisible(menuBtn);
        
        loadPresetList();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bgCard());
        // Bottom accent line
        g.setColour(DS::Color::Accent::primary().withAlpha(0.3f));
        g.fillRect(0.0f, (float)getHeight() - 1.0f, (float)getWidth(), 1.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(8, 2);
        
        titleLabel.setBounds(bounds.removeFromLeft(60));
        bounds.removeFromLeft(12);
        
        // Preset area centered
        auto presetArea = bounds.withSizeKeepingCentre(240, bounds.getHeight());
        prevBtn.setBounds(presetArea.removeFromLeft(22).reduced(0, 2));
        presetArea.removeFromLeft(4);
        menuBtn.setBounds(presetArea.removeFromRight(22).reduced(0, 2));
        presetArea.removeFromRight(4);
        nextBtn.setBounds(presetArea.removeFromRight(22).reduced(0, 2));
        presetArea.removeFromRight(4);
        patchName.setBounds(presetArea.reduced(0, 1));
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.eventComponent == &patchName)
            showPresetMenu();
    }
    
    void navigatePreset(int direction)
    {
        currentPresetIndex += direction;
        if (currentPresetIndex < 0) 
            currentPresetIndex = static_cast<int>(presetList.size()) - 1;
        if (currentPresetIndex >= static_cast<int>(presetList.size())) 
            currentPresetIndex = 0;
        
        if (!presetList.empty())
        {
            patchName.setText(presetList[currentPresetIndex], juce::dontSendNotification);
            if (onPresetSelected)
                onPresetSelected(currentPresetIndex);
        }
    }
    
    void showPresetMenu()
    {
        juce::PopupMenu menu;
        juce::PopupMenu factoryMenu;
        for (size_t i = 0; i < 8 && i < presetList.size(); ++i)
            factoryMenu.addItem((int)i + 1, presetList[i]);
        menu.addSubMenu("Factory", factoryMenu);
        menu.addSeparator();
        juce::PopupMenu userMenu;
        for (size_t i = 8; i < presetList.size(); ++i)
            userMenu.addItem(100 + (int)i, presetList[i]);
        menu.addSubMenu("User", userMenu);
        menu.addSeparator();
        menu.addItem(1000, "Save Preset...");
        menu.addItem(1001, "Load Preset...");
        
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&patchName));
    }
    
    void loadPresetList()
    {
        presetList = {"A11-Strings", "A12-Brass", "A13-Organ", "A14-Piano",
                      "A15-Bass", "A16-Pad", "A17-Lead", "A18-Sync",
                      "B11-MyPad", "B12-Lead", "B13-Bass", "B14-Strings2"};
    }
    
    void setPatchName(const juce::String& name) { patchName.setText(name, juce::dontSendNotification); }
    juce::String getPatchName() const { return patchName.getText(); }
    
    std::function<void(int)> onPresetSelected;
    
private:
    juce::Label titleLabel, patchName;
    juce::TextButton prevBtn, nextBtn, menuBtn;
    std::vector<juce::String> presetList;
    int currentPresetIndex = 0;
};

//==============================================================================
// Balance/Morph Strip - Shows when viewing Upper/Lower tabs
// Controls the global Upper/Lower balance via GLOBAL_UL_BALANCE_SYSEX (0x12)
//==============================================================================
class BalanceMorphStrip : public juce::Component
{
public:
    BalanceMorphStrip()
    {
        // Upper label
        upperLabel.setText("UPPER", juce::dontSendNotification);
        upperLabel.setFont(DS::Font::bold(10.0f));
        upperLabel.setColour(juce::Label::textColourId, DS::Color::Accent::cyan());
        upperLabel.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(upperLabel);
        
        // Balance slider (center-notched)
        balanceSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        balanceSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        balanceSlider.setRange(0.0, 127.0, 1.0);
        balanceSlider.setValue(64.0);  // Center = equal balance
        balanceSlider.setDoubleClickReturnValue(true, 64.0);
        balanceSlider.setColour(juce::Slider::thumbColourId, DS::Color::Accent::primary());
        balanceSlider.setColour(juce::Slider::trackColourId, DS::Color::bgElevated());
        balanceSlider.setColour(juce::Slider::backgroundColourId, DS::Color::bgCard());
        balanceSlider.setTooltip("Upper/Lower Balance (64 = equal mix)");
        balanceSlider.onValueChange = [this]()
        {
            int value = (int)balanceSlider.getValue();
            valueLabel.setText(juce::String(value), juce::dontSendNotification);
            if (onBalanceChanged)
                onBalanceChanged(value);
        };
        addAndMakeVisible(balanceSlider);
        
        // Lower label
        lowerLabel.setText("LOWER", juce::dontSendNotification);
        lowerLabel.setFont(DS::Font::bold(10.0f));
        lowerLabel.setColour(juce::Label::textColourId, DS::Color::Accent::magenta());
        lowerLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(lowerLabel);
        
        // Value display
        valueLabel.setText("64", juce::dontSendNotification);
        valueLabel.setFont(DS::Font::mono(10.0f));
        valueLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        valueLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(valueLabel);
        
        // Title/indicator
        titleLabel.setText("MIX", juce::dontSendNotification);
        titleLabel.setFont(DS::Font::tiny());
        titleLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        titleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(titleLabel);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(DS::Color::bgCard().darker(0.1f));
        g.fillRect(bounds);
        
        // Top border accent
        g.setColour(DS::Color::Accent::primary().withAlpha(0.2f));
        g.fillRect(0.0f, 0.0f, bounds.getWidth(), 1.0f);
        
        // Draw center notch indicator on slider
        float centerX = getWidth() / 2.0f;
        g.setColour(DS::Color::textMuted().withAlpha(0.5f));
        g.fillRect(centerX - 0.5f, 4.0f, 1.0f, (float)getHeight() - 8.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(4, 2);
        
        // Title on far left
        titleLabel.setBounds(bounds.removeFromLeft(30));
        bounds.removeFromLeft(4);
        
        // Upper label
        upperLabel.setBounds(bounds.removeFromLeft(40));
        bounds.removeFromLeft(4);
        
        // Value display on far right
        valueLabel.setBounds(bounds.removeFromRight(24));
        bounds.removeFromRight(4);
        
        // Lower label
        lowerLabel.setBounds(bounds.removeFromRight(40));
        bounds.removeFromRight(4);
        
        // Slider takes remaining space
        balanceSlider.setBounds(bounds);
    }
    
    void setBalance(int value)
    {
        balanceSlider.setValue(value, juce::dontSendNotification);
        valueLabel.setText(juce::String(value), juce::dontSendNotification);
    }
    
    int getBalance() const { return (int)balanceSlider.getValue(); }
    
    // Callback when balance changes (should send SysEx)
    std::function<void(int)> onBalanceChanged;
    
private:
    juce::Label upperLabel, lowerLabel, valueLabel, titleLabel;
    juce::Slider balanceSlider;
};

//==============================================================================
// Bottom Toolbar - MIDI, A/B Compare, Settings, etc.
//==============================================================================
class BottomToolbar : public juce::Component
{
public:
    BottomToolbar()
    {
        // MIDI status LED (animated)
        midiLED.setColour(juce::Label::backgroundColourId, DS::Color::Accent::success().darker(0.8f));
        midiLED.setJustificationType(juce::Justification::centred);
        midiLED.setText("", juce::dontSendNotification);
        addAndMakeVisible(midiLED);
        
        // MIDI status text
        midiLabel.setText("MIDI", juce::dontSendNotification);
        midiLabel.setFont(DS::Font::tiny());
        midiLabel.setColour(juce::Label::textColourId, DS::Color::Accent::success());
        addAndMakeVisible(midiLabel);
        
        // Undo button
        undoBtn.setButtonText("Undo");
        undoBtn.setTooltip("Undo (Ctrl+Z)");
        undoBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        undoBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        undoBtn.setEnabled(false);
        undoBtn.onClick = [this]() { if (onUndoClick) onUndoClick(); };
        addAndMakeVisible(undoBtn);
        
        // Redo button
        redoBtn.setButtonText("Redo");
        redoBtn.setTooltip("Redo (Ctrl+Y)");
        redoBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        redoBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        redoBtn.setEnabled(false);
        redoBtn.onClick = [this]() { if (onRedoClick) onRedoClick(); };
        addAndMakeVisible(redoBtn);
        
        // A/B Compare
        compareBtn.setButtonText("A/B");
        compareBtn.setClickingTogglesState(true);
        compareBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        compareBtn.setColour(juce::TextButton::buttonOnColourId, DS::Color::Accent::magenta().darker(0.2f));
        compareBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        compareBtn.setColour(juce::TextButton::textColourOnId, DS::Color::Accent::magenta());
        compareBtn.setTooltip("Compare A/B patches");
        addAndMakeVisible(compareBtn);
        
        // Settings
        settingsBtn.setButtonText("Settings");
        settingsBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        settingsBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        addAndMakeVisible(settingsBtn);
        
        // MIDI In/Out selectors (compact)
        midiInLabel.setText("IN:", juce::dontSendNotification);
        midiInLabel.setFont(DS::Font::tiny());
        midiInLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        addAndMakeVisible(midiInLabel);
        
        midiInCombo.addItem("None", 1);
        midiInCombo.addItem("MIDI Port 1", 2);
        midiInCombo.setSelectedId(1);
        midiInCombo.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
        midiInCombo.setColour(juce::ComboBox::textColourId, DS::Color::textSecondary());
        midiInCombo.setColour(juce::ComboBox::outlineColourId, DS::Color::border());
        addAndMakeVisible(midiInCombo);
        
        midiOutLabel.setText("OUT:", juce::dontSendNotification);
        midiOutLabel.setFont(DS::Font::tiny());
        midiOutLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        addAndMakeVisible(midiOutLabel);
        
        midiOutCombo.addItem("None", 1);
        midiOutCombo.addItem("MIDI Port 1", 2);
        midiOutCombo.setSelectedId(1);
        midiOutCombo.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
        midiOutCombo.setColour(juce::ComboBox::textColourId, DS::Color::textSecondary());
        midiOutCombo.setColour(juce::ComboBox::outlineColourId, DS::Color::border());
        addAndMakeVisible(midiOutCombo);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bgCard());
        // Top border
        g.setColour(DS::Color::border());
        g.fillRect(0.0f, 0.0f, (float)getWidth(), 1.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(8, 3);
        
        // Left - MIDI LED and status
        midiLED.setBounds(bounds.removeFromLeft(8).reduced(0, 4));
        bounds.removeFromLeft(4);
        midiLabel.setBounds(bounds.removeFromLeft(32).reduced(0, 2));
        bounds.removeFromLeft(8);
        midiInLabel.setBounds(bounds.removeFromLeft(20).reduced(0, 3));
        midiInCombo.setBounds(bounds.removeFromLeft(90).reduced(0, 2));
        bounds.removeFromLeft(8);
        midiOutLabel.setBounds(bounds.removeFromLeft(26).reduced(0, 3));
        midiOutCombo.setBounds(bounds.removeFromLeft(90).reduced(0, 2));
        
        // Right - Settings, A/B, Undo/Redo
        settingsBtn.setBounds(bounds.removeFromRight(60).reduced(0, 2));
        bounds.removeFromRight(6);
        compareBtn.setBounds(bounds.removeFromRight(36).reduced(0, 2));
        bounds.removeFromRight(6);
        redoBtn.setBounds(bounds.removeFromRight(24).reduced(0, 2));
        bounds.removeFromRight(2);
        undoBtn.setBounds(bounds.removeFromRight(24).reduced(0, 2));
    }
    
    void setMidiStatus(bool connected)
    {
        midiLabel.setColour(juce::Label::textColourId, 
            connected ? DS::Color::Accent::success() : DS::Color::Accent::error());
        midiLabel.setText(connected ? "MIDI OK" : "NO MIDI", juce::dontSendNotification);
        midiLED.setColour(juce::Label::backgroundColourId,
            connected ? DS::Color::Accent::success().darker(0.8f) : DS::Color::Accent::error().darker(0.8f));
    }
    
    void flashMidiActivity(bool isInput)
    {
        // Flash the LED briefly
        midiLED.setColour(juce::Label::backgroundColourId,
            isInput ? DS::Color::Accent::cyan() : DS::Color::Accent::magenta());
        juce::Timer::callAfterDelay(50, [this]() {
            midiLED.setColour(juce::Label::backgroundColourId, DS::Color::Accent::success().darker(0.8f));
        });
    }
    
    void setUndoEnabled(bool enabled) { undoBtn.setEnabled(enabled); }
    void setRedoEnabled(bool enabled) { redoBtn.setEnabled(enabled); }
    
    std::function<void(bool)> onCompareToggle;
    std::function<void()> onSettingsClick;
    std::function<void()> onUndoClick;
    std::function<void()> onRedoClick;
    
private:
    juce::Label midiLabel, midiInLabel, midiOutLabel, midiLED;
    juce::ComboBox midiInCombo, midiOutCombo;
    juce::TextButton compareBtn, settingsBtn, undoBtn, redoBtn;
};

//==============================================================================
// Vertical Tab Button - Modern icon/text style
//==============================================================================
class VerticalTabButton : public juce::Button
{
public:
    VerticalTabButton(const juce::String& name, const juce::String& shortName, int idx)
        : juce::Button(name), displayName(shortName), tabIndex(idx) {}
    
    void paintButton(juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        if (getToggleState())
        {
            // Active tab - accent background
            g.setColour(DS::Color::Accent::primary().withAlpha(0.15f));
            g.fillRoundedRectangle(bounds.reduced(2, 1), 4.0f);
            
            // Left accent bar
            g.setColour(DS::Color::Accent::primary());
            g.fillRoundedRectangle(bounds.getX(), bounds.getY() + 4, 3.0f, bounds.getHeight() - 8, 1.5f);
        }
        else if (isMouseOver)
        {
            g.setColour(DS::Color::bgHover());
            g.fillRoundedRectangle(bounds.reduced(2, 1), 4.0f);
        }
        
        // Text
        g.setFont(DS::Font::bold(11.0f));
        g.setColour(getToggleState() ? DS::Color::Accent::primary() : 
                    (isMouseOver ? DS::Color::textPrimary() : DS::Color::textSecondary()));
        g.drawText(displayName, bounds.reduced(6, 0), juce::Justification::centredLeft, true);
    }
    
    int getTabIndex() const { return tabIndex; }
    
private:
    juce::String displayName;
    int tabIndex;
};

//==============================================================================
/**
    MainComponent - Central assembly of all MKS-70 Editor components
    
    New Layout:
    - MiniTitleBar at top (26px) - logo + preset nav only
    - Vertical tabs on left (50px) - modern icons
    - Content area fills rest
    - BottomToolbar (24px) - MIDI, A/B, Settings
*/
class MainComponent : public juce::Component,
                      public juce::KeyListener
{
public:
    MainComponent()
    {
        setLookAndFeel(&lookAndFeel);
        
        // === MINI TITLE BAR ===
        titleBar = std::make_unique<MiniTitleBar>();
        addAndMakeVisible(titleBar.get());
        
        // === BOTTOM TOOLBAR ===
        bottomToolbar = std::make_unique<BottomToolbar>();
        bottomToolbar->onSettingsClick = [this]() { selectTab(5); };
        bottomToolbar->onUndoClick = [this]() { if (undoManager) undoManager->undo(); };
        bottomToolbar->onRedoClick = [this]() { if (undoManager) undoManager->redo(); };
        addAndMakeVisible(bottomToolbar.get());
        
        // === BALANCE/MORPH STRIP (shows on Upper/Lower tabs) ===
        balanceStrip = std::make_unique<BalanceMorphStrip>();
        balanceStrip->onBalanceChanged = [this](int value)
        {
            // TODO: Send SysEx for GLOBAL_UL_BALANCE_SYSEX (0x12)
            // Format: F0 41 39 00 24 36 12 [value] F7 (IPR OpCode 36)
            DBG("Balance changed to: " + juce::String(value));
        };
        addAndMakeVisible(balanceStrip.get());
        
        // === CREATE PANELS ===
        panels.push_back(std::make_unique<TonePanel>(true));   // Upper
        panels.push_back(std::make_unique<TonePanel>(false));  // Lower
        panels.push_back(std::make_unique<PatchPanel>());      // Patch
        panels.push_back(std::make_unique<ArpPanel>());        // Arp
        panels.push_back(std::make_unique<LibrarianTab>());    // Library
        panels.push_back(std::make_unique<SettingsTab>());     // Settings
        
        // === ADVANCED PANELS ===
        undoManager = std::make_unique<MKS70UndoManager>();
        comparePanel = std::make_unique<ComparePanel>();
        undoHistoryPanel = std::make_unique<UndoHistoryPanel>(*undoManager);
        panels.push_back(std::move(comparePanel));
        panels.push_back(std::move(undoHistoryPanel));
        
        for (auto& panel : panels)
            addChildComponent(panel.get());
        
        // === VERTICAL TAB BUTTONS ===
        const char* tabNames[] = {"Upper", "Lower", "Patch", "Arp", "Library", "Settings", "Compare", "Undo"};
        const char* shortNames[] = {"UPPER", "LOWER", "PATCH", "ARP", "LIB", "SET", "COMPARE", "UNDO"};
        
        for (int i = 0; i < 8; ++i)
        {
            auto btn = std::make_unique<VerticalTabButton>(tabNames[i], shortNames[i], i);
            btn->setRadioGroupId(1);
            btn->setClickingTogglesState(true);
            btn->onClick = [this, i]() { selectTab(i); };
            addAndMakeVisible(btn.get());
            tabButtons.push_back(std::move(btn));
        }
        
        // Select first tab
        tabButtons[0]->setToggleState(true, juce::dontSendNotification);
        panels[0]->setVisible(true);
        currentTab = 0;
        
        setSize(1280, 800);

        // Ensure this component can receive keyboard events
        setWantsKeyboardFocus(true);
        addKeyListener(this);

        // Wire shortcut actions to the undo manager
        Shortcuts::ShortcutManager::getInstance().setAction("undo", [this]() {
            if (undoManager) undoManager->undo();
        });
        Shortcuts::ShortcutManager::getInstance().setAction("redo", [this]() {
            if (undoManager) undoManager->redo();
        });
        
        // Update undo/redo button states when history changes
        undoManager->onHistoryChanged = [this]() {
            if (bottomToolbar)
            {
                bottomToolbar->setUndoEnabled(undoManager->canUndo());
                bottomToolbar->setRedoEnabled(undoManager->canRedo());
                bottomToolbar->undoBtn.setTooltip(undoManager->canUndo() ? 
                    "Undo: " + undoManager->getUndoDescription() : "Undo (Ctrl+Z)");
                bottomToolbar->redoBtn.setTooltip(undoManager->canRedo() ? 
                    "Redo: " + undoManager->getRedoDescription() : "Redo (Ctrl+Y)");
            }
            if (undoHistoryPanel)
                undoHistoryPanel->repaint();
        };
    }
    
    ~MainComponent() override
    {
        removeKeyListener(this);
        setLookAndFeel(nullptr);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bg());
        
        // Tab bar background
        auto tabBarArea = getLocalBounds();
        tabBarArea.removeFromTop(26);  // title bar
        tabBarArea.removeFromBottom(24); // bottom toolbar
        tabBarArea = tabBarArea.removeFromLeft(50);
        
        g.setColour(DS::Color::bgCard());
        g.fillRect(tabBarArea);
        
        // Right border of tab bar
        g.setColour(DS::Color::border());
        g.fillRect(tabBarArea.getRight() - 1, tabBarArea.getY(), 1, tabBarArea.getHeight());
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        
        // Title bar at top (26px)
        titleBar->setBounds(bounds.removeFromTop(26));
        
        // Balance strip (20px) - only visible on Upper/Lower tabs
        if (balanceStrip->isVisible())
        {
            const int balanceStripHeight = 20;
            auto balanceArea = bounds.removeFromTop(balanceStripHeight);
            balanceStrip->setBounds(balanceArea);
        }
        
        // Bottom toolbar (24px)
        bottomToolbar->setBounds(bounds.removeFromBottom(24));
        
        // Vertical tab bar on left (50px)
        auto tabBar = bounds.removeFromLeft(50);
        tabBar.removeFromTop(8);
        
        for (auto& btn : tabButtons)
        {
            btn->setBounds(tabBar.removeFromTop(32).reduced(2, 1));
            tabBar.removeFromTop(2);
        }
        
        // Content area
        for (auto& panel : panels)
            panel->setBounds(bounds);
    }
    
    void selectTab(int index)
    {
        if (index < 0 || index >= (int)panels.size()) return;
        
        for (auto& panel : panels)
            panel->setVisible(false);
        currentTab = index;
        panels[currentTab]->setVisible(true);
        
        // Show balance strip only on Upper (0) or Lower (1) tabs
        bool showBalanceStrip = (index == 0 || index == 1);
        balanceStrip->setVisible(showBalanceStrip);
        
        for (size_t i = 0; i < tabButtons.size(); ++i)
            tabButtons[i]->setToggleState((int)i == index, juce::dontSendNotification);
        
        // Trigger layout update for balance strip visibility
        resized();
        repaint();
    }
    
    MiniTitleBar* getTitleBar() { return titleBar.get(); }
    BottomToolbar* getBottomToolbar() { return bottomToolbar.get(); }
    BalanceMorphStrip* getBalanceStrip() { return balanceStrip.get(); }
    
private:
    // KeyListener implementation - forward keys to the ShortcutManager
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override
    {
        return Shortcuts::ShortcutManager::getInstance().handleKeyPress(key);
    }

    MKS70LookAndFeel lookAndFeel;
    std::unique_ptr<MiniTitleBar> titleBar;
    std::unique_ptr<BottomToolbar> bottomToolbar;
    std::unique_ptr<BalanceMorphStrip> balanceStrip;
    std::vector<std::unique_ptr<juce::Component>> panels;
    std::vector<std::unique_ptr<VerticalTabButton>> tabButtons;
    int currentTab = 0;
    std::unique_ptr<ComparePanel> comparePanel;
    std::unique_ptr<UndoHistoryPanel> undoHistoryPanel;
    std::unique_ptr<MKS70UndoManager> undoManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
