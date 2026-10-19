/*
  ==============================================================================
    SettingsTab.cpp - Comprehensive Settings with Device/Software tabs
  ==============================================================================
*/

#include "SettingsTab.h"
#include "PluginProcessor.h"

SettingsTab::SettingsTab()
{
    // Tab buttons
    addAndMakeVisible(deviceTabBtn);
    addAndMakeVisible(softwareTabBtn);
    addAndMakeVisible(midiTestTabBtn);
    deviceTabBtn.addListener(this);
    softwareTabBtn.addListener(this);
    midiTestTabBtn.addListener(this);
    styleButton(deviceTabBtn, DS::Color::Accent::cyan());
    styleButton(softwareTabBtn, DS::Color::Accent::cyan());
    styleButton(midiTestTabBtn, DS::Color::Accent::cyan());
    
    // Device settings panel
    addAndMakeVisible(deviceSettingsPanel);
    
    // MIDI Test panel with viewport
    addChildComponent(midiTestViewport);
    // midiTestPanel will be created when setProcessor is called
    
    // Software container with viewport for scrolling
    addChildComponent(softwareViewport);
    softwareViewport.setViewedComponent(&softwareContainer, false);
    softwareViewport.setScrollBarsShown(true, false);
    setupSoftwareSettings();
    
    showDeviceTab();
    loadSettings();
}

SettingsTab::~SettingsTab() 
{
    saveSettings();
}

void SettingsTab::setProcessor(MKS70EditorAudioProcessor* proc)
{
    processor = proc;
    deviceSettingsPanel.setProcessor(proc);
    
    if (proc && !midiTestPanel)
    {
        midiTestPanel = std::make_unique<MidiTestPanel>(proc->getMidiHandler());
        midiTestViewport.setViewedComponent(midiTestPanel.get(), false);
        midiTestViewport.setScrollBarsShown(true, false);
    }

    applySettingsToProcessor();
}

void SettingsTab::paint(juce::Graphics& g)
{
    g.fillAll(DS::Color::bg());
    
    // Tab bar background
    g.setColour(DS::Color::bgElevated());
    g.fillRect(0, 0, getWidth(), 36);
    
    // Subtle separator line
    g.setColour(DS::Color::border());
    g.drawHorizontalLine(36, 0.0f, (float)getWidth());
}

void SettingsTab::resized()
{
    auto bounds = getLocalBounds();
    
    // Tab bar
    auto tabBar = bounds.removeFromTop(36);
    deviceTabBtn.setBounds(tabBar.removeFromLeft(100).reduced(6, 6));
    softwareTabBtn.setBounds(tabBar.removeFromLeft(100).reduced(6, 6));
    midiTestTabBtn.setBounds(tabBar.removeFromLeft(100).reduced(6, 6));
    
    // Content area
    auto content = bounds.reduced(8);
    deviceSettingsPanel.setBounds(content);
    softwareViewport.setBounds(content);
    midiTestViewport.setBounds(content);
    
    // Set size for scrollable content
    layoutSoftwareSettings();
    
    // Set size for MIDI Test panel
    if (midiTestPanel)
        midiTestPanel->setSize(content.getWidth() - 20, 800);
}

void SettingsTab::buttonClicked(juce::Button* button)
{
    if (button == &deviceTabBtn) showDeviceTab();
    else if (button == &softwareTabBtn) showSoftwareTab();
    else if (button == &midiTestTabBtn) showMidiTestTab();
    else if (button == &testMidiBtn) testMidiConnection();
    else if (button == &resetBtn) resetToDefaults();
    else if (button == &importBtn) importSettings();
    else if (button == &exportBtn) exportSettings();
    else if (button == &aboutBtn) showAbout();
    else if (button == &browseLibraryBtn) browseForLibraryPath();
    else if (button == &browseBackupBtn) browseForBackupPath();
}

void SettingsTab::showDeviceTab()
{
    currentTab = 0;
    deviceSettingsPanel.setVisible(true);
    softwareViewport.setVisible(false);
    midiTestViewport.setVisible(false);
    deviceTabBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::primary());
    softwareTabBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
    midiTestTabBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
}

void SettingsTab::showSoftwareTab()
{
    currentTab = 1;
    deviceSettingsPanel.setVisible(false);
    softwareViewport.setVisible(true);
    midiTestViewport.setVisible(false);
    deviceTabBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
    softwareTabBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::primary());
    midiTestTabBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
}

void SettingsTab::showMidiTestTab()
{
    currentTab = 2;
    deviceSettingsPanel.setVisible(false);
    softwareViewport.setVisible(false);
    midiTestViewport.setVisible(true);
    deviceTabBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
    softwareTabBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
    midiTestTabBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::primary());
}

void SettingsTab::setupSoftwareSettings()
{
    //==========================================================================
    // SECTION 1: Appearance
    //==========================================================================
    softwareContainer.addAndMakeVisible(appearanceHeader);
    styleSectionHeader(appearanceHeader);
    
    // Theme
    softwareContainer.addAndMakeVisible(themeLabel); styleLabel(themeLabel);
    softwareContainer.addAndMakeVisible(themeCombo); styleCombo(themeCombo, DS::Color::Accent::magenta());
    themeCombo.addItem("Roland Dark", 1);
    themeCombo.addItem("Modern Slate", 2);
    themeCombo.addItem("Modern Ember", 3);
    themeCombo.addItem("Light", 4);
    themeCombo.addItem("Modern Breeze", 5);
    themeCombo.addItem("High Contrast", 6);
    themeCombo.addItem("Vintage", 7);
    themeCombo.addItem("Cyberpunk", 8);
    themeCombo.setSelectedId(1);
    themeCombo.onChange = [this]() {
        int themeId = themeCombo.getSelectedId();
        DBG("Theme changed to: " << themeId);
        if (onThemeChanged)
            onThemeChanged(themeId);
        saveSettings();
    };
    
    // UI Scale
    softwareContainer.addAndMakeVisible(uiScaleLabel); styleLabel(uiScaleLabel);
    softwareContainer.addAndMakeVisible(uiScaleCombo); styleCombo(uiScaleCombo, DS::Color::Accent::cyan());
    uiScaleCombo.addItem("80%", 1);
    uiScaleCombo.addItem("90%", 2);
    uiScaleCombo.addItem("100%", 3);
    uiScaleCombo.addItem("110%", 4);
    uiScaleCombo.addItem("125%", 5);
    uiScaleCombo.addItem("150%", 6);
    uiScaleCombo.setSelectedId(3);
    uiScaleCombo.onChange = [this]() {
        float scales[] = {0.8f, 0.9f, 1.0f, 1.1f, 1.25f, 1.5f};
        int idx = uiScaleCombo.getSelectedId() - 1;
        if (idx >= 0 && idx < 6 && onUIScaleChanged)
            onUIScaleChanged(scales[idx]);
        saveSettings();
    };
    
    // Animation Speed
    softwareContainer.addAndMakeVisible(animSpeedLabel); styleLabel(animSpeedLabel);
    softwareContainer.addAndMakeVisible(animSpeedCombo); styleCombo(animSpeedCombo, DS::Color::Accent::cyan());
    animSpeedCombo.addItem("Off", 1);
    animSpeedCombo.addItem("Slow", 2);
    animSpeedCombo.addItem("Normal", 3);
    animSpeedCombo.addItem("Fast", 4);
    animSpeedCombo.setSelectedId(3);
    animSpeedCombo.onChange = [this]() {
        float speeds[] = {0.0f, 0.5f, 1.0f, 2.0f};
        int idx = animSpeedCombo.getSelectedId() - 1;
        if (idx >= 0 && idx < 4 && onAnimationSpeedChanged)
            onAnimationSpeedChanged(speeds[idx]);
        saveSettings();
    };
    
    // Tooltips toggle
    softwareContainer.addAndMakeVisible(showTooltipsToggle); styleToggle(showTooltipsToggle);
    showTooltipsToggle.setToggleState(true, juce::dontSendNotification);
    
    // Show values toggle
    softwareContainer.addAndMakeVisible(showParameterValuesToggle); styleToggle(showParameterValuesToggle);
    showParameterValuesToggle.setToggleState(true, juce::dontSendNotification);
    
    //==========================================================================
    // SECTION 2: Workflow & Sync
    //==========================================================================
    softwareContainer.addAndMakeVisible(workflowHeader);
    styleSectionHeader(workflowHeader);
    
    softwareContainer.addAndMakeVisible(sendOnEditToggle); styleToggle(sendOnEditToggle, DS::Color::Accent::success());
    sendOnEditToggle.setToggleState(true, juce::dontSendNotification);
    sendOnEditToggle.onClick = [this]() {
        if (processor)
            processor->setSendOnEditEnabled(sendOnEditToggle.getToggleState());
        saveSettings();
    };
    
    softwareContainer.addAndMakeVisible(requestOnLoadToggle); styleToggle(requestOnLoadToggle, DS::Color::Accent::success());
    requestOnLoadToggle.setToggleState(true, juce::dontSendNotification);
    requestOnLoadToggle.onClick = [this]() {
        if (processor)
            processor->setRequestSysExOnLoad(requestOnLoadToggle.getToggleState());
        saveSettings();
    };
    
    softwareContainer.addAndMakeVisible(autoBackupToggle); styleToggle(autoBackupToggle, DS::Color::Accent::warning());
    autoBackupToggle.setToggleState(false, juce::dontSendNotification);
    
    softwareContainer.addAndMakeVisible(confirmActionsToggle); styleToggle(confirmActionsToggle);
    confirmActionsToggle.setToggleState(true, juce::dontSendNotification);
    
    softwareContainer.addAndMakeVisible(rememberWindowToggle); styleToggle(rememberWindowToggle);
    rememberWindowToggle.setToggleState(true, juce::dontSendNotification);
    
    //==========================================================================
    // SECTION 3: File Paths
    //==========================================================================
    softwareContainer.addAndMakeVisible(pathsHeader);
    styleSectionHeader(pathsHeader);
    
    softwareContainer.addAndMakeVisible(libraryLabel); styleLabel(libraryLabel);
    softwareContainer.addAndMakeVisible(libraryPath); styleTextEditor(libraryPath);
    libraryPath.setReadOnly(true);
    libraryPath.setText(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("MKS70 Presets").getFullPathName());
    softwareContainer.addAndMakeVisible(browseLibraryBtn); styleButton(browseLibraryBtn); 
    browseLibraryBtn.addListener(this);
    
    softwareContainer.addAndMakeVisible(backupLabel); styleLabel(backupLabel);
    softwareContainer.addAndMakeVisible(backupPathEdit); styleTextEditor(backupPathEdit);
    backupPathEdit.setReadOnly(true);
    backupPathEdit.setText(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("MKS70 Backups").getFullPathName());
    softwareContainer.addAndMakeVisible(browseBackupBtn); styleButton(browseBackupBtn); 
    browseBackupBtn.addListener(this);
    
    softwareContainer.addAndMakeVisible(backupIntLabel); styleLabel(backupIntLabel);
    softwareContainer.addAndMakeVisible(backupIntSlider); styleSlider(backupIntSlider);
    backupIntSlider.setRange(1, 60, 1);
    backupIntSlider.setValue(5);
    backupIntSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    softwareContainer.addAndMakeVisible(backupIntValueLabel); styleLabel(backupIntValueLabel);
    backupIntSlider.onValueChange = [this]() {
        int val = (int)backupIntSlider.getValue();
        backupIntValueLabel.setText(juce::String(val) + " min", juce::dontSendNotification);
        saveSettings();
    };
    
    //==========================================================================
    // SECTION 4: Vecoven Firmware
    //==========================================================================
    softwareContainer.addAndMakeVisible(vecovenHeader);
    styleSectionHeader(vecovenHeader);
    
    softwareContainer.addAndMakeVisible(fwVersionLabel); styleLabel(fwVersionLabel);
    softwareContainer.addAndMakeVisible(fwVersionCombo); styleCombo(fwVersionCombo, DS::Color::Accent::magenta());
    fwVersionCombo.addItem("Original ROM", 1);
    fwVersionCombo.addItem("Vecoven 3.x", 2);
    fwVersionCombo.addItem("Vecoven 4.x", 3);
    fwVersionCombo.setSelectedId(2);  // Default to V3.x (most common)
    fwVersionCombo.onChange = [this]() {
        int fwId = fwVersionCombo.getSelectedId();
        DBG("SettingsTab: Firmware version selected: " + juce::String(fwId) + 
            (fwId == 1 ? " (Original ROM)" : fwId == 2 ? " (Vecoven 3.x)" : " (Vecoven 4.x)"));
        if (processor) {
            auto& midiHandler = processor->getMidiHandler();
            midiHandler.setFirmwareVersion(static_cast<MidiHandler::FirmwareVersion>(fwId));
        }
        // Notify UI to update panels
        if (onFirmwareVersionChanged)
            onFirmwareVersionChanged(fwId);
        saveSettings();
    };
    
    softwareContainer.addAndMakeVisible(pwmToggle); styleToggle(pwmToggle, DS::Color::Accent::magenta());
    softwareContainer.addAndMakeVisible(aprToggle); styleToggle(aprToggle, DS::Color::Accent::magenta());
    
    softwareContainer.addAndMakeVisible(sysexRedirLabel); styleLabel(sysexRedirLabel);
    softwareContainer.addAndMakeVisible(sysexRedirCombo); styleCombo(sysexRedirCombo, DS::Color::Accent::magenta());
    sysexRedirCombo.addItem("Off", 1);
    sysexRedirCombo.addItem("Upper Tone", 2);
    sysexRedirCombo.addItem("Lower Tone", 3);
    sysexRedirCombo.setSelectedId(1);
    
    //==========================================================================
    // SECTION 5: Advanced/Timing
    //==========================================================================
    softwareContainer.addAndMakeVisible(advancedHeader);
    styleSectionHeader(advancedHeader);
    
    softwareContainer.addAndMakeVisible(sysexDelayLabel); styleLabel(sysexDelayLabel);
    softwareContainer.addAndMakeVisible(sysexDelaySlider); styleSlider(sysexDelaySlider, DS::Color::Accent::warning());
    sysexDelaySlider.setRange(0, 200, 1);
    sysexDelaySlider.setValue(20);
    sysexDelaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 20);
    sysexDelaySlider.setTextValueSuffix(" ms");
    sysexDelaySlider.onValueChange = [this]() {
        if (processor)
            processor->getMidiHandler().setSysExDelay((int)sysexDelaySlider.getValue());
        saveSettings();
    };
    
    softwareContainer.addAndMakeVisible(bulkDelayLabel); styleLabel(bulkDelayLabel);
    softwareContainer.addAndMakeVisible(bulkDelaySlider); styleSlider(bulkDelaySlider, DS::Color::Accent::warning());
    bulkDelaySlider.setRange(0, 500, 1);
    bulkDelaySlider.setValue(50);
    bulkDelaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 20);
    bulkDelaySlider.setTextValueSuffix(" ms");
    
    softwareContainer.addAndMakeVisible(midiThruToggle); styleToggle(midiThruToggle);
    midiThruToggle.setToggleState(true, juce::dontSendNotification);
    midiThruToggle.onClick = [this]() {
        if (processor)
            processor->getMidiHandler().setMidiThruEnabled(midiThruToggle.getToggleState());
        saveSettings();
    };
    
    softwareContainer.addAndMakeVisible(debugModeToggle); styleToggle(debugModeToggle, DS::Color::Accent::warning());
    softwareContainer.addAndMakeVisible(logToFileToggle); styleToggle(logToFileToggle, DS::Color::Accent::warning());
    
    //==========================================================================
    // Action Buttons
    //==========================================================================
    softwareContainer.addAndMakeVisible(testMidiBtn); styleButton(testMidiBtn, DS::Color::Accent::cyan()); 
    testMidiBtn.addListener(this);
    softwareContainer.addAndMakeVisible(resetBtn); styleButton(resetBtn, DS::Color::Accent::error()); 
    resetBtn.addListener(this);
    softwareContainer.addAndMakeVisible(importBtn); styleButton(importBtn, DS::Color::Accent::success()); 
    importBtn.addListener(this);
    softwareContainer.addAndMakeVisible(exportBtn); styleButton(exportBtn, DS::Color::Accent::success()); 
    exportBtn.addListener(this);
    softwareContainer.addAndMakeVisible(aboutBtn); styleButton(aboutBtn); 
    aboutBtn.addListener(this);
}

void SettingsTab::layoutSoftwareSettings()
{
    const int contentWidth = softwareViewport.getWidth() - 16; // Account for scrollbar
    const int rowH = 32;
    const int sectionH = 28;
    const int labelW = 120;
    const int comboW = 150;
    const int toggleW = 200;
    const int pathW = 280;
    const int btnW = 32;
    const int sliderW = 140;
    const int gap = 8;
    const int sectionGap = 16;
    
    int y = 8;
    
    //==========================================================================
    // SECTION 1: Appearance
    //==========================================================================
    appearanceHeader.setBounds(8, y, contentWidth - 16, sectionH);
    y += sectionH + gap;
    
    // Theme row
    themeLabel.setBounds(8, y, labelW, rowH);
    themeCombo.setBounds(labelW + 8, y, comboW, rowH - 4);
    y += rowH;
    
    // UI Scale row
    uiScaleLabel.setBounds(8, y, labelW, rowH);
    uiScaleCombo.setBounds(labelW + 8, y, comboW, rowH - 4);
    y += rowH;
    
    // Animation row
    animSpeedLabel.setBounds(8, y, labelW, rowH);
    animSpeedCombo.setBounds(labelW + 8, y, comboW, rowH - 4);
    y += rowH;
    
    // Toggle row
    showTooltipsToggle.setBounds(8, y, toggleW, rowH);
    showParameterValuesToggle.setBounds(toggleW + 16, y, toggleW, rowH);
    y += rowH + sectionGap;
    
    //==========================================================================
    // SECTION 2: Workflow & Sync
    //==========================================================================
    workflowHeader.setBounds(8, y, contentWidth - 16, sectionH);
    y += sectionH + gap;
    
    sendOnEditToggle.setBounds(8, y, toggleW, rowH);
    requestOnLoadToggle.setBounds(toggleW + 16, y, toggleW, rowH);
    y += rowH;
    
    autoBackupToggle.setBounds(8, y, toggleW, rowH);
    confirmActionsToggle.setBounds(toggleW + 16, y, toggleW, rowH);
    y += rowH;
    
    rememberWindowToggle.setBounds(8, y, toggleW, rowH);
    y += rowH + sectionGap;
    
    //==========================================================================
    // SECTION 3: File Paths
    //==========================================================================
    pathsHeader.setBounds(8, y, contentWidth - 16, sectionH);
    y += sectionH + gap;
    
    libraryLabel.setBounds(8, y, labelW, rowH);
    libraryPath.setBounds(labelW + 8, y, pathW, rowH - 4);
    browseLibraryBtn.setBounds(labelW + pathW + 12, y, btnW, rowH - 4);
    y += rowH;
    
    backupLabel.setBounds(8, y, labelW, rowH);
    backupPathEdit.setBounds(labelW + 8, y, pathW, rowH - 4);
    browseBackupBtn.setBounds(labelW + pathW + 12, y, btnW, rowH - 4);
    y += rowH;
    
    backupIntLabel.setBounds(8, y, labelW, rowH);
    backupIntSlider.setBounds(labelW + 8, y, sliderW, rowH - 4);
    backupIntValueLabel.setBounds(labelW + sliderW + 12, y, 60, rowH);
    y += rowH + sectionGap;
    
    //==========================================================================
    // SECTION 4: Vecoven Firmware
    //==========================================================================
    vecovenHeader.setBounds(8, y, contentWidth - 16, sectionH);
    y += sectionH + gap;
    
    fwVersionLabel.setBounds(8, y, labelW, rowH);
    fwVersionCombo.setBounds(labelW + 8, y, comboW, rowH - 4);
    y += rowH;
    
    pwmToggle.setBounds(8, y, 100, rowH);
    aprToggle.setBounds(116, y, 100, rowH);
    y += rowH;
    
    sysexRedirLabel.setBounds(8, y, labelW, rowH);
    sysexRedirCombo.setBounds(labelW + 8, y, comboW, rowH - 4);
    y += rowH + sectionGap;
    
    //==========================================================================
    // SECTION 5: Advanced/Timing
    //==========================================================================
    advancedHeader.setBounds(8, y, contentWidth - 16, sectionH);
    y += sectionH + gap;
    
    sysexDelayLabel.setBounds(8, y, labelW, rowH);
    sysexDelaySlider.setBounds(labelW + 8, y, sliderW + 50, rowH - 4);
    y += rowH;
    
    bulkDelayLabel.setBounds(8, y, labelW, rowH);
    bulkDelaySlider.setBounds(labelW + 8, y, sliderW + 50, rowH - 4);
    y += rowH;
    
    midiThruToggle.setBounds(8, y, 120, rowH);
    debugModeToggle.setBounds(136, y, 120, rowH);
    logToFileToggle.setBounds(264, y, 120, rowH);
    y += rowH + sectionGap;
    
    //==========================================================================
    // Action Buttons (centered)
    //==========================================================================
    const int btnCount = 5;
    const int btnWidth = 80;
    const int totalBtnWidth = btnCount * btnWidth + (btnCount - 1) * gap;
    int btnX = (contentWidth - totalBtnWidth) / 2;
    int btnY = y + 8;
    
    testMidiBtn.setBounds(btnX, btnY, btnWidth, 36);
    btnX += btnWidth + gap;
    resetBtn.setBounds(btnX, btnY, btnWidth, 36);
    btnX += btnWidth + gap;
    importBtn.setBounds(btnX, btnY, btnWidth, 36);
    btnX += btnWidth + gap;
    exportBtn.setBounds(btnX, btnY, btnWidth, 36);
    btnX += btnWidth + gap;
    aboutBtn.setBounds(btnX, btnY, btnWidth, 36);
    
    y = btnY + 50;
    
    // Set total size for scrolling
    softwareContainer.setSize(contentWidth, y);
}

void SettingsTab::styleSectionHeader(juce::Label& l)
{
    l.setFont(juce::Font(13.0f, juce::Font::bold));
    l.setColour(juce::Label::textColourId, DS::Color::Accent::cyan());
    l.setColour(juce::Label::backgroundColourId, DS::Color::bgElevated().withAlpha(0.5f));
    l.setJustificationType(juce::Justification::centredLeft);
}

void SettingsTab::styleCombo(juce::ComboBox& c, juce::Colour accent)
{
    c.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
    c.setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
    c.setColour(juce::ComboBox::outlineColourId, accent.withAlpha(0.5f));
    c.setColour(juce::ComboBox::arrowColourId, accent);
}

void SettingsTab::styleLabel(juce::Label& l)
{
    l.setColour(juce::Label::textColourId, DS::Color::textSecondary());
    l.setFont(DS::Font::small());
}

void SettingsTab::styleSlider(juce::Slider& s, juce::Colour accent)
{
    s.setColour(juce::Slider::backgroundColourId, DS::Color::bgInput());
    s.setColour(juce::Slider::thumbColourId, accent);
    s.setColour(juce::Slider::trackColourId, accent.withAlpha(0.4f));
    s.setColour(juce::Slider::textBoxTextColourId, DS::Color::textPrimary());
    s.setColour(juce::Slider::textBoxBackgroundColourId, DS::Color::bgInput());
    s.setColour(juce::Slider::textBoxOutlineColourId, accent.withAlpha(0.3f));
}

void SettingsTab::styleToggle(juce::ToggleButton& t, juce::Colour accent)
{
    t.setColour(juce::ToggleButton::textColourId, DS::Color::textPrimary());
    t.setColour(juce::ToggleButton::tickColourId, accent);
    t.setColour(juce::ToggleButton::tickDisabledColourId, DS::Color::textSecondary());
}

void SettingsTab::styleButton(juce::TextButton& b, juce::Colour accent)
{
    b.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
    b.setColour(juce::TextButton::buttonOnColourId, accent);
    b.setColour(juce::TextButton::textColourOffId, DS::Color::textPrimary());
    b.setColour(juce::TextButton::textColourOnId, DS::Color::textPrimary());
}

void SettingsTab::styleTextEditor(juce::TextEditor& e)
{
    e.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
    e.setColour(juce::TextEditor::textColourId, DS::Color::textPrimary());
    e.setColour(juce::TextEditor::outlineColourId, DS::Color::Accent::primary().withAlpha(0.3f));
}

void SettingsTab::testMidiConnection()
{
    if (processor) {
        auto& midiHandler = processor->getMidiHandler();
        midiHandler.sendToneParameter(1, 37, 64); // Upper, VCF Cutoff, mid value
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
            "MIDI Test", 
            "Sent SysEx test message: VCF Cutoff change to Upper Tone.\n\n"
            "If your MKS-70 is connected, you should hear the filter change briefly.");
    } else {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, 
            "MIDI Test", "Processor not available for MIDI test.");
    }
}

void SettingsTab::resetToDefaults()
{
    auto callback = juce::ModalCallbackFunction::create([this](int result) {
        if (result == 1) {
            // Reset all values to defaults
            themeCombo.setSelectedId(1, juce::sendNotification);
            uiScaleCombo.setSelectedId(3, juce::dontSendNotification);
            animSpeedCombo.setSelectedId(3, juce::dontSendNotification);
            showTooltipsToggle.setToggleState(true, juce::dontSendNotification);
            showParameterValuesToggle.setToggleState(true, juce::dontSendNotification);
            sendOnEditToggle.setToggleState(true, juce::dontSendNotification);
            requestOnLoadToggle.setToggleState(true, juce::dontSendNotification);
            autoBackupToggle.setToggleState(false, juce::dontSendNotification);
            confirmActionsToggle.setToggleState(true, juce::dontSendNotification);
            rememberWindowToggle.setToggleState(true, juce::dontSendNotification);
            fwVersionCombo.setSelectedId(1, juce::sendNotification);
            pwmToggle.setToggleState(false, juce::dontSendNotification);
            aprToggle.setToggleState(false, juce::dontSendNotification);
            sysexRedirCombo.setSelectedId(1, juce::dontSendNotification);
            sysexDelaySlider.setValue(20, juce::sendNotification);
            bulkDelaySlider.setValue(50, juce::dontSendNotification);
            midiThruToggle.setToggleState(true, juce::sendNotification);
            debugModeToggle.setToggleState(false, juce::dontSendNotification);
            logToFileToggle.setToggleState(false, juce::dontSendNotification);
            backupIntSlider.setValue(5, juce::sendNotification);
            
            saveSettings();
            
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                "Reset Complete", "All settings have been reset to defaults.");
        }
    });
    
    juce::AlertWindow::showOkCancelBox(juce::AlertWindow::WarningIcon, 
        "Reset Settings", 
        "Are you sure you want to reset all settings to defaults?\n\nThis cannot be undone.",
        "Reset", "Cancel", this, callback);
}

void SettingsTab::importSettings()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Import Settings", juce::File(), "*.xml");
    
    chooser->launchAsync(juce::FileBrowserComponent::openMode | 
                         juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) {
                auto xml = juce::parseXML(file);
                if (xml != nullptr && xml->hasTagName("MKS70Settings")) {
                    // Load settings from XML
                    themeCombo.setSelectedId(xml->getIntAttribute("theme", 1), juce::sendNotification);
                    uiScaleCombo.setSelectedId(xml->getIntAttribute("uiScale", 3), juce::dontSendNotification);
                    animSpeedCombo.setSelectedId(xml->getIntAttribute("animSpeed", 3), juce::dontSendNotification);
                    fwVersionCombo.setSelectedId(xml->getIntAttribute("firmware", 1), juce::sendNotification);
                    sysexDelaySlider.setValue(xml->getDoubleAttribute("sysexDelay", 20), juce::sendNotification);
                    bulkDelaySlider.setValue(xml->getDoubleAttribute("bulkDelay", 50), juce::dontSendNotification);
                    
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                        "Import Complete", "Settings imported successfully.");
                } else {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                        "Import Failed", "Invalid settings file.");
                }
            }
        });
}

void SettingsTab::exportSettings()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Export Settings", juce::File(), "*.xml");
    
    chooser->launchAsync(juce::FileBrowserComponent::saveMode,
        [this, chooser](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File()) {
                juce::XmlElement xml("MKS70Settings");
                xml.setAttribute("theme", themeCombo.getSelectedId());
                xml.setAttribute("uiScale", uiScaleCombo.getSelectedId());
                xml.setAttribute("animSpeed", animSpeedCombo.getSelectedId());
                xml.setAttribute("firmware", fwVersionCombo.getSelectedId());
                xml.setAttribute("sysexDelay", sysexDelaySlider.getValue());
                xml.setAttribute("bulkDelay", bulkDelaySlider.getValue());
                xml.setAttribute("sendOnEdit", sendOnEditToggle.getToggleState());
                xml.setAttribute("requestOnLoad", requestOnLoadToggle.getToggleState());
                xml.setAttribute("autoBackup", autoBackupToggle.getToggleState());
                xml.setAttribute("midiThru", midiThruToggle.getToggleState());
                
                if (xml.writeTo(file.withFileExtension(".xml"))) {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                        "Export Complete", "Settings exported successfully.");
                }
            }
        });
}

void SettingsTab::showAbout()
{
    juce::String aboutText = 
        "MKS-70 Editor v1.0\n\n"
        "A modern editor for the Roland MKS-70 Super JX synthesizer.\n\n"
        "Features:\n"
        "- Full SysEx control of all parameters\n"
        "- Support for Vecoven firmware\n"
        "- Preset management and morphing\n"
        "- Real-time parameter display\n\n"
        "Created with JUCE Framework\n"
        "2024";
    
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
        "About MKS-70 Editor", aboutText);
}

void SettingsTab::browseForLibraryPath()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select Preset Library Folder", 
        juce::File(libraryPath.getText()));
    
    chooser->launchAsync(juce::FileBrowserComponent::openMode | 
                         juce::FileBrowserComponent::canSelectDirectories,
        [this, chooser](const juce::FileChooser& fc) {
            auto folder = fc.getResult();
            if (folder.isDirectory()) {
                libraryPath.setText(folder.getFullPathName());
                saveSettings();
            }
        });
}

void SettingsTab::browseForBackupPath()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select Backup Folder", 
        juce::File(backupPathEdit.getText()));
    
    chooser->launchAsync(juce::FileBrowserComponent::openMode | 
                         juce::FileBrowserComponent::canSelectDirectories,
        [this, chooser](const juce::FileChooser& fc) {
            auto folder = fc.getResult();
            if (folder.isDirectory()) {
                backupPathEdit.setText(folder.getFullPathName());
                saveSettings();
            }
        });
}

void SettingsTab::applySettingsToProcessor()
{
    if (processor)
    {
        processor->setSendOnEditEnabled(sendOnEditToggle.getToggleState());
        processor->setRequestSysExOnLoad(requestOnLoadToggle.getToggleState());
        processor->getMidiHandler().setFirmwareVersion(
            static_cast<MidiHandler::FirmwareVersion>(fwVersionCombo.getSelectedId()));
        processor->getMidiHandler().setSysExDelay((int)sysexDelaySlider.getValue());
        processor->getMidiHandler().setMidiThruEnabled(midiThruToggle.getToggleState());
    }

    if (onThemeChanged)
        onThemeChanged(themeCombo.getSelectedId());

    if (onUIScaleChanged)
    {
        const float scales[] = { 0.8f, 0.9f, 1.0f, 1.1f, 1.25f, 1.5f };
        const int scaleIdx = juce::jlimit(0, 5, uiScaleCombo.getSelectedId() - 1);
        onUIScaleChanged(scales[scaleIdx]);
    }

    if (onAnimationSpeedChanged)
    {
        const float speeds[] = { 0.0f, 0.5f, 1.0f, 2.0f };
        const int speedIdx = juce::jlimit(0, 3, animSpeedCombo.getSelectedId() - 1);
        onAnimationSpeedChanged(speeds[speedIdx]);
    }

    if (onFirmwareVersionChanged)
        onFirmwareVersionChanged(fwVersionCombo.getSelectedId());
}

void SettingsTab::saveSettings()
{
    auto settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("MKS70Editor")
        .getChildFile("settings.xml");
    
    settingsFile.getParentDirectory().createDirectory();
    
    juce::XmlElement xml("MKS70EditorSettings");
    xml.setAttribute("theme", themeCombo.getSelectedId());
    xml.setAttribute("uiScale", uiScaleCombo.getSelectedId());
    xml.setAttribute("animSpeed", animSpeedCombo.getSelectedId());
    xml.setAttribute("showTooltips", showTooltipsToggle.getToggleState());
    xml.setAttribute("showValues", showParameterValuesToggle.getToggleState());
    xml.setAttribute("sendOnEdit", sendOnEditToggle.getToggleState());
    xml.setAttribute("requestOnLoad", requestOnLoadToggle.getToggleState());
    xml.setAttribute("autoBackup", autoBackupToggle.getToggleState());
    xml.setAttribute("confirmActions", confirmActionsToggle.getToggleState());
    xml.setAttribute("rememberWindow", rememberWindowToggle.getToggleState());
    xml.setAttribute("libraryPath", libraryPath.getText());
    xml.setAttribute("backupPath", backupPathEdit.getText());
    xml.setAttribute("backupInterval", backupIntSlider.getValue());
    xml.setAttribute("firmware", fwVersionCombo.getSelectedId());
    xml.setAttribute("pwm", pwmToggle.getToggleState());
    xml.setAttribute("apr", aprToggle.getToggleState());
    xml.setAttribute("sysexRedirect", sysexRedirCombo.getSelectedId());
    xml.setAttribute("sysexDelay", sysexDelaySlider.getValue());
    xml.setAttribute("bulkDelay", bulkDelaySlider.getValue());
    xml.setAttribute("midiThru", midiThruToggle.getToggleState());
    xml.setAttribute("debug", debugModeToggle.getToggleState());
    xml.setAttribute("log", logToFileToggle.getToggleState());
    
    xml.writeTo(settingsFile);
}

void SettingsTab::loadSettings()
{
    auto settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("MKS70Editor")
        .getChildFile("settings.xml");
    
    if (settingsFile.existsAsFile()) {
        auto xml = juce::parseXML(settingsFile);
        if (xml != nullptr && xml->hasTagName("MKS70EditorSettings")) {
            themeCombo.setSelectedId(xml->getIntAttribute("theme", 1), juce::sendNotification);
            uiScaleCombo.setSelectedId(xml->getIntAttribute("uiScale", 3), juce::dontSendNotification);
            animSpeedCombo.setSelectedId(xml->getIntAttribute("animSpeed", 3), juce::dontSendNotification);
            showTooltipsToggle.setToggleState(xml->getBoolAttribute("showTooltips", true), juce::dontSendNotification);
            showParameterValuesToggle.setToggleState(xml->getBoolAttribute("showValues", true), juce::dontSendNotification);
            sendOnEditToggle.setToggleState(xml->getBoolAttribute("sendOnEdit", true), juce::dontSendNotification);
            requestOnLoadToggle.setToggleState(xml->getBoolAttribute("requestOnLoad", true), juce::dontSendNotification);
            autoBackupToggle.setToggleState(xml->getBoolAttribute("autoBackup", false), juce::dontSendNotification);
            confirmActionsToggle.setToggleState(xml->getBoolAttribute("confirmActions", true), juce::dontSendNotification);
            rememberWindowToggle.setToggleState(xml->getBoolAttribute("rememberWindow", true), juce::dontSendNotification);
            
            juce::String lib = xml->getStringAttribute("libraryPath");
            if (lib.isNotEmpty()) libraryPath.setText(lib);
            
            juce::String backup = xml->getStringAttribute("backupPath");
            if (backup.isNotEmpty()) backupPathEdit.setText(backup);
            
            backupIntSlider.setValue(xml->getDoubleAttribute("backupInterval", 5), juce::sendNotification);
            fwVersionCombo.setSelectedId(xml->getIntAttribute("firmware", 1), juce::sendNotification);
            pwmToggle.setToggleState(xml->getBoolAttribute("pwm", false), juce::dontSendNotification);
            aprToggle.setToggleState(xml->getBoolAttribute("apr", false), juce::dontSendNotification);
            sysexRedirCombo.setSelectedId(xml->getIntAttribute("sysexRedirect", 1), juce::dontSendNotification);
            sysexDelaySlider.setValue(xml->getDoubleAttribute("sysexDelay", 20), juce::sendNotification);
            bulkDelaySlider.setValue(xml->getDoubleAttribute("bulkDelay", 50), juce::dontSendNotification);
            midiThruToggle.setToggleState(xml->getBoolAttribute("midiThru", true), juce::sendNotification);
            debugModeToggle.setToggleState(xml->getBoolAttribute("debug", false), juce::dontSendNotification);
            logToFileToggle.setToggleState(xml->getBoolAttribute("log", false), juce::dontSendNotification);
        }
    }

    applySettingsToProcessor();
}
