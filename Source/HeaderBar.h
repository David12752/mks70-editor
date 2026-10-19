/*
  ==============================================================================
    HeaderBar.h - Modern Header Bar for MKS-70 Editor
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "DCOSection.h"  // For FirmwareVersion enum

class MKS70EditorAudioProcessor;

//==============================================================================
// Minimal expand bar shown when HeaderBar is collapsed
class ExpandBar : public juce::Component
{
public:
    ExpandBar()
    {
        expandBtn.setButtonText("v MKS-70 Editor v");
        expandBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgCard());
        expandBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::primary());
        expandBtn.onClick = [this]() { if (onExpand) onExpand(); };
        expandBtn.setTooltip("Click to expand header bar");
        addAndMakeVisible(expandBtn);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bgCard());
        g.setColour(DS::Color::Accent::primary().withAlpha(0.3f));
        g.drawLine(0, (float)getHeight() - 1, (float)getWidth(), (float)getHeight() - 1, 1.0f);
    }
    
    void resized() override
    {
        expandBtn.setBounds(getLocalBounds().withSizeKeepingCentre(160, 14));
    }
    
    std::function<void()> onExpand;
    
private:
    juce::TextButton expandBtn;
};

//==============================================================================
class HeaderBar : public juce::Component, public juce::Button::Listener
{
public:
    HeaderBar()
    {
        // Logo/Title compact
        titleLabel.setText("MKS-70", juce::dontSendNotification);
        titleLabel.setFont(DS::Font::large());
        titleLabel.setColour(juce::Label::textColourId, DS::Color::Accent::primary());
        titleLabel.setTooltip("Roland MKS-70 Super JX Editor");
        addAndMakeVisible(titleLabel);
        
        // === U-HE STYLE PRESET NAVIGATION ===
        // Previous preset button
        prevPresetBtn.setButtonText("<");
        prevPresetBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        prevPresetBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::cyan());
        prevPresetBtn.setTooltip("Previous preset");
        prevPresetBtn.onClick = [this]() { navigatePreset(-1); };
        addAndMakeVisible(prevPresetBtn);
        
        // Patch name (clickable to open preset menu)
        patchName.setText("INIT PATCH", juce::dontSendNotification);
        patchName.setFont(DS::Font::medium());
        patchName.setColour(juce::Label::textColourId, DS::Color::Accent::cyan());
        patchName.setJustificationType(juce::Justification::centred);
        patchName.setEditable(false); // Click opens menu instead
        patchName.setTooltip("Click to open preset browser");
        patchName.addMouseListener(this, false);
        addAndMakeVisible(patchName);
        
        // Next preset button
        nextPresetBtn.setButtonText(">");
        nextPresetBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        nextPresetBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::cyan());
        nextPresetBtn.setTooltip("Next preset");
        nextPresetBtn.onClick = [this]() { navigatePreset(1); };
        addAndMakeVisible(nextPresetBtn);
        
        // Preset menu button (hamburger style)
        presetMenuBtn.setButtonText("...");
        presetMenuBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        presetMenuBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        presetMenuBtn.setTooltip("Open preset browser");
        presetMenuBtn.onClick = [this]() { showPresetMenu(); };
        addAndMakeVisible(presetMenuBtn);
        
        // Initialize preset list
        loadPresetList();
        
        // MIDI status indicator (compact)
        midiStatusLabel.setText("MIDI", juce::dontSendNotification);
        midiStatusLabel.setFont(DS::Font::tiny());
        midiStatusLabel.setColour(juce::Label::textColourId, DS::Color::Accent::success());
        midiStatusLabel.setTooltip("MIDI connection status");
        addAndMakeVisible(midiStatusLabel);
        
        // Compare A/B button (small)
        compareBtn.setButtonText("A/B");
        compareBtn.setClickingTogglesState(true);
        compareBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        compareBtn.setColour(juce::TextButton::buttonOnColourId, DS::Color::Accent::magenta().darker(0.2f));
        compareBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textPrimary());
        compareBtn.setColour(juce::TextButton::textColourOnId, DS::Color::Accent::magenta());
        compareBtn.setTooltip("Compare A/B - Store and compare two patch versions");
        compareBtn.addListener(this);
        addAndMakeVisible(compareBtn);
        
        // Settings quick access
        settingsBtn.setButtonText("[S]");
        settingsBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        settingsBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        settingsBtn.setTooltip("Open Settings tab");
        settingsBtn.addListener(this);
        addAndMakeVisible(settingsBtn);
        
        // Collapse button
        collapseBtn.setButtonText("^");
        collapseBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        collapseBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textMuted());
        collapseBtn.setTooltip("Collapse header bar");
        collapseBtn.addListener(this);
        addAndMakeVisible(collapseBtn);
        
        // Firmware version selector
        firmwareCombo.addItem("Original ROM", 1);
        firmwareCombo.addItem("Vecoven V3.x", 2);
        firmwareCombo.addItem("Vecoven V4.x", 3);
        firmwareCombo.setSelectedId(2); // Default to V3.x
        firmwareCombo.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgElevated());
        firmwareCombo.setColour(juce::ComboBox::textColourId, DS::Color::Accent::cyan());
        firmwareCombo.setColour(juce::ComboBox::outlineColourId, DS::Color::border());
        firmwareCombo.setTooltip("Select MKS-70 firmware version\n\nOriginal ROM = Standard MKS-70\nVecoven V3.x = Vecoven firmware 3.x\nVecoven V4.x = Extended features (LFO2, PWM, etc.)");
        firmwareCombo.onChange = [this]() {
            int selected = firmwareCombo.getSelectedId();
            FirmwareVersion fw = FirmwareVersion::Vecoven3;
            if (selected == 1) fw = FirmwareVersion::Original;
            else if (selected == 2) fw = FirmwareVersion::Vecoven3;
            else if (selected == 3) fw = FirmwareVersion::Vecoven4;
            if (onFirmwareChanged) onFirmwareChanged(fw);
        };
        addAndMakeVisible(firmwareCombo);
        
        // === BANK / PROGRAM SELECTOR ===
        // Bank selector (A-H = 1-8 for MIDI Bank Select)
        bankCombo.addItem("Bank A", 1);
        bankCombo.addItem("Bank B", 2);
        bankCombo.addItem("Bank C", 3);
        bankCombo.addItem("Bank D", 4);
        bankCombo.addItem("Bank E", 5);
        bankCombo.addItem("Bank F", 6);
        bankCombo.addItem("Bank G", 7);
        bankCombo.addItem("Bank H", 8);
        bankCombo.setSelectedId(1); // Default Bank A
        bankCombo.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgElevated());
        bankCombo.setColour(juce::ComboBox::textColourId, DS::Color::Accent::warning());
        bankCombo.setColour(juce::ComboBox::outlineColourId, DS::Color::border());
        bankCombo.setTooltip("Select MKS-70 Patch Bank (A-H)\nSends MIDI Bank Select CC#0");
        bankCombo.onChange = [this]() {
            sendBankProgram();
        };
        addAndMakeVisible(bankCombo);
        
        // Program prev button
        prevProgramBtn.setButtonText("<");
        prevProgramBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        prevProgramBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::warning());
        prevProgramBtn.setTooltip("Previous program");
        prevProgramBtn.onClick = [this]() { navigateProgram(-1); };
        addAndMakeVisible(prevProgramBtn);
        
        // Program number label
        programLabel.setText("1", juce::dontSendNotification);
        programLabel.setFont(DS::Font::medium());
        programLabel.setColour(juce::Label::textColourId, DS::Color::Accent::warning());
        programLabel.setJustificationType(juce::Justification::centred);
        programLabel.setTooltip("Current program (1-8)");
        addAndMakeVisible(programLabel);
        
        // Program next button
        nextProgramBtn.setButtonText(">");
        nextProgramBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        nextProgramBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::warning());
        nextProgramBtn.setTooltip("Next program");
        nextProgramBtn.onClick = [this]() { navigateProgram(1); };
        addAndMakeVisible(nextProgramBtn);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Simple dark background
        g.setColour(DS::Color::bgCard());
        g.fillRect(bounds);
        
        // Subtle bottom border
        g.setColour(DS::Color::Accent::primary().withAlpha(0.3f));
        g.fillRect(0.0f, bounds.getBottom() - 1.0f, bounds.getWidth(), 1.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::S, 2);
        
        // Left side - Logo (compact)
        titleLabel.setBounds(bounds.removeFromLeft(70).withSizeKeepingCentre(70, 24));
        bounds.removeFromLeft(DS::Space::S);
        
        // Firmware selector (after logo)
        firmwareCombo.setBounds(bounds.removeFromLeft(110).withSizeKeepingCentre(105, 22));
        bounds.removeFromLeft(DS::Space::S);
        
        // === BANK / PROGRAM SELECTOR ===
        bankCombo.setBounds(bounds.removeFromLeft(75).withSizeKeepingCentre(70, 22));
        bounds.removeFromLeft(4);
        prevProgramBtn.setBounds(bounds.removeFromLeft(20).withSizeKeepingCentre(18, 22));
        programLabel.setBounds(bounds.removeFromLeft(24).withSizeKeepingCentre(22, 22));
        nextProgramBtn.setBounds(bounds.removeFromLeft(20).withSizeKeepingCentre(18, 22));
        bounds.removeFromLeft(DS::Space::S);
        
        // Right side - Controls (compact)
        collapseBtn.setBounds(bounds.removeFromRight(24).withSizeKeepingCentre(22, 22));
        bounds.removeFromRight(DS::Space::XS);
        settingsBtn.setBounds(bounds.removeFromRight(28).withSizeKeepingCentre(26, 22));
        bounds.removeFromRight(DS::Space::XS);
        compareBtn.setBounds(bounds.removeFromRight(40).withSizeKeepingCentre(38, 22));
        bounds.removeFromRight(DS::Space::XS);
        midiStatusLabel.setBounds(bounds.removeFromRight(36).withSizeKeepingCentre(36, 18));
        bounds.removeFromRight(DS::Space::S);
        
        // === U-HE STYLE PRESET NAVIGATION ===
        // Preset menu button (rightmost of preset area)
        presetMenuBtn.setBounds(bounds.removeFromRight(26).withSizeKeepingCentre(24, 22));
        bounds.removeFromRight(2);
        
        // Next button
        nextPresetBtn.setBounds(bounds.removeFromRight(22).withSizeKeepingCentre(20, 22));
        bounds.removeFromRight(4);
        
        // Patch name (center, clickable)
        auto patchArea = bounds.removeFromRight(140);
        patchName.setBounds(patchArea.withSizeKeepingCentre(136, 24));
        bounds.removeFromRight(4);
        
        // Prev button
        prevPresetBtn.setBounds(bounds.removeFromRight(22).withSizeKeepingCentre(20, 22));
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.eventComponent == &patchName)
        {
            showPresetMenu();
        }
    }
    
    void navigatePreset(int direction)
    {
        DBG("HeaderBar::navigatePreset called with direction=" + juce::String(direction));
        DBG("  presetList.size() = " + juce::String(presetList.size()));
        
        // First try using the callback to get preset count from Librarian
        int presetCount = 0;
        if (onGetPresetCount)
        {
            presetCount = onGetPresetCount();
            DBG("HeaderBar::navigatePreset - Got preset count from callback: " + juce::String(presetCount));
        }
        else if (!presetList.empty())
        {
            presetCount = static_cast<int>(presetList.size());
            DBG("HeaderBar::navigatePreset - Using local preset list: " + juce::String(presetCount));
        }
        else
        {
            DBG("HeaderBar::navigatePreset - NO preset count available! onGetPresetCount=" + 
                juce::String(onGetPresetCount ? "set" : "null") + 
                " presetList.size=" + juce::String(presetList.size()));
        }
        
        if (presetCount == 0) 
        {
            DBG("HeaderBar::navigatePreset - presetCount is 0, returning");
            return;
        }
        
        currentPresetIndex += direction;
        if (currentPresetIndex < 0) 
            currentPresetIndex = presetCount - 1;
        if (currentPresetIndex >= presetCount) 
            currentPresetIndex = 0;
        
        // Update local display if we have the list
        if (!presetList.empty() && currentPresetIndex < static_cast<int>(presetList.size()))
            patchName.setText(presetList[currentPresetIndex], juce::dontSendNotification);
        
        // Notify callbacks - onNavigatePreset is the main one that connects to Librarian
        if (onNavigatePreset)
            onNavigatePreset(currentPresetIndex);
        if (onPresetSelected)
            onPresetSelected(currentPresetIndex);
    }
    
    void navigateProgram(int direction)
    {
        int newProgram = currentProgram + direction;
        if (newProgram < 1) newProgram = 8;
        if (newProgram > 8) newProgram = 1;
        currentProgram = newProgram;
        programLabel.setText(juce::String(currentProgram), juce::dontSendNotification);
        sendBankProgram();
    }
    
    void sendBankProgram()
    {
        int bank = bankCombo.getSelectedId() - 1;  // 0-7 for Bank A-H
        int program = currentProgram - 1;          // 0-7 for Program 1-8
        
        DBG("HeaderBar::sendBankProgram - Bank=" + juce::String(bank) + " Program=" + juce::String(program));
        
        if (onBankProgramChange)
            onBankProgramChange(bank, program);
    }
    
    void showPresetMenu()
    {
        juce::PopupMenu menu;
        
        // Factory presets section
        juce::PopupMenu factoryMenu;
        factoryMenu.addItem(1, "A11 - Strings");
        factoryMenu.addItem(2, "A12 - Brass");
        factoryMenu.addItem(3, "A13 - Organ");
        factoryMenu.addItem(4, "A14 - Piano");
        factoryMenu.addItem(5, "A15 - Bass");
        factoryMenu.addItem(6, "A16 - Pad");
        factoryMenu.addItem(7, "A17 - Lead");
        factoryMenu.addItem(8, "A18 - Sync");
        menu.addSubMenu("Factory", factoryMenu);
        
        menu.addSeparator();
        
        // User presets section
        juce::PopupMenu userMenu;
        userMenu.addItem(101, "B11 - My Pad");
        userMenu.addItem(102, "B12 - Lead");
        userMenu.addItem(103, "B13 - Bass");
        userMenu.addItem(104, "B14 - Strings 2");
        menu.addSubMenu("User", userMenu);
        
        menu.addSeparator();
        
        // Actions
        menu.addItem(1000, "Save Preset...");
        menu.addItem(1001, "Save As...");
        menu.addItem(1002, "Load Preset...");
        
        menu.showMenuAsync(juce::PopupMenu::Options()
            .withTargetComponent(&patchName)
            .withMinimumWidth(180),
            [this](int result) {
                if (result > 0 && result < 100) {
                    // Factory preset selected
                    currentPresetIndex = result - 1;
                    patchName.setText(presetList[currentPresetIndex], juce::dontSendNotification);
                    if (onPresetSelected) onPresetSelected(currentPresetIndex);
                } else if (result >= 101 && result < 200) {
                    // User preset selected
                    currentPresetIndex = 8 + (result - 101);
                    if (currentPresetIndex < static_cast<int>(presetList.size())) {
                        patchName.setText(presetList[currentPresetIndex], juce::dontSendNotification);
                        if (onPresetSelected) onPresetSelected(currentPresetIndex);
                    }
                }
            });
    }
    
    void loadPresetList()
    {
        presetList.clear();
        // Factory presets
        presetList.push_back("A11 - Strings");
        presetList.push_back("A12 - Brass");
        presetList.push_back("A13 - Organ");
        presetList.push_back("A14 - Piano");
        presetList.push_back("A15 - Bass");
        presetList.push_back("A16 - Pad");
        presetList.push_back("A17 - Lead");
        presetList.push_back("A18 - Sync");
        // User presets
        presetList.push_back("B11 - My Pad");
        presetList.push_back("B12 - Lead");
        presetList.push_back("B13 - Bass");
        presetList.push_back("B14 - Strings 2");
    }
    
    void buttonClicked(juce::Button* button) override
    {
        if (button == &collapseBtn)
        {
            if (onCollapseToggle)
                onCollapseToggle();
        }
        else if (button == &settingsBtn)
        {
            if (onSettingsClick)
                onSettingsClick();
        }
        else if (button == &compareBtn)
        {
            if (onCompareToggle)
                onCompareToggle(compareBtn.getToggleState());
        }
    }
    
    void setPatchName(const juce::String& name)
    {
        patchName.setText(name, juce::dontSendNotification);
    }
    
    juce::String getPatchName() const
    {
        return patchName.getText();
    }
    
    void setMidiStatus(bool connected)
    {
        midiStatusLabel.setColour(juce::Label::textColourId, 
            connected ? DS::Color::Accent::success() : DS::Color::Accent::error());
        midiStatusLabel.setText(connected ? "MIDI OK" : "NO MIDI", juce::dontSendNotification);
    }
    
    void setProcessor(MKS70EditorAudioProcessor* p) { processor = p; }
    
    // Set preset list from external source (e.g., from scanned files)
    void setPresetList(const juce::StringArray& names)
    {
        DBG("HeaderBar::setPresetList called with " + juce::String(names.size()) + " presets");
        presetList.clear();
        for (auto& name : names)
            presetList.push_back(name);
        currentPresetIndex = 0;
        if (!presetList.empty())
        {
            patchName.setText(presetList[0], juce::dontSendNotification);
            DBG("HeaderBar: First preset = " + presetList[0]);
        }
        else
        {
            DBG("HeaderBar: Preset list is EMPTY!");
        }
    }
    
    // Get current preset index
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    
    // Set current preset index externally
    void setCurrentPresetIndex(int idx)
    {
        if (idx >= 0 && idx < static_cast<int>(presetList.size()))
        {
            currentPresetIndex = idx;
            patchName.setText(presetList[currentPresetIndex], juce::dontSendNotification);
        }
    }
    
    // Update preset list from Librarian
    void setPresetList(const std::vector<juce::String>& names)
    {
        presetList = names;
        if (currentPresetIndex >= static_cast<int>(presetList.size()))
            currentPresetIndex = 0;
        if (!presetList.empty())
            patchName.setText(presetList[currentPresetIndex], juce::dontSendNotification);
        else
            patchName.setText("No Presets", juce::dontSendNotification);
    }
    
    void setPresetName(const juce::String& name)
    {
        patchName.setText(name, juce::dontSendNotification);
    }
    
    void setFirmwareVersion(FirmwareVersion fw)
    {
        int id = 2; // Default V3.x
        if (fw == FirmwareVersion::Original) id = 1;
        else if (fw == FirmwareVersion::Vecoven3) id = 2;
        else if (fw == FirmwareVersion::Vecoven4) id = 3;
        firmwareCombo.setSelectedId(id, juce::dontSendNotification);
    }
    
    FirmwareVersion getFirmwareVersion() const
    {
        int id = firmwareCombo.getSelectedId();
        if (id == 1) return FirmwareVersion::Original;
        if (id == 3) return FirmwareVersion::Vecoven4;
        return FirmwareVersion::Vecoven3;
    }
    
    // Callbacks
    std::function<void()> onCollapseToggle;
    std::function<void()> onSettingsClick;
    std::function<void(bool)> onCompareToggle;
    std::function<void(int)> onPresetSelected;
    std::function<void(int)> onNavigatePreset;  // Called when user clicks prev/next
    std::function<int()> onGetPresetCount;      // Get preset count from Librarian
    std::function<void(FirmwareVersion)> onFirmwareChanged;  // Firmware version changed
    std::function<void(int, int)> onBankProgramChange;  // Bank (0-7) and Program (0-7) changed
    
private:
    MKS70EditorAudioProcessor* processor = nullptr;
    
    juce::Label titleLabel;
    juce::Label patchName;
    juce::TextButton prevPresetBtn, nextPresetBtn, presetMenuBtn;
    juce::Label midiStatusLabel;
    juce::TextButton compareBtn, settingsBtn, collapseBtn;
    juce::ComboBox firmwareCombo;
    
    // Bank/Program selector
    juce::ComboBox bankCombo;
    juce::TextButton prevProgramBtn, nextProgramBtn;
    juce::Label programLabel;
    int currentProgram = 1;  // 1-8
    
    // Preset navigation
    std::vector<juce::String> presetList;
    int currentPresetIndex = 0;
};
