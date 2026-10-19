/*
  ==============================================================================
    LibrarianTab.cpp - Preset Library Manager Implementation
    
    Redesigned with grid layout and AI explanation panel
  ==============================================================================
*/

#include "LibrarianTab.h"
#include "PresetManager.h"

LibrarianTab::LibrarianTab()
{
    setupUI();
    loadPresetsFromFolder();
    rebuildPresetCards();
}

LibrarianTab::~LibrarianTab()
{
    presetList.setModel(nullptr);
}

void LibrarianTab::setupUI()
{
    // === PRESETS SECTION (Left side) ===
    presetsSection = std::make_unique<GlowingSection>("Preset Library", DS::Color::Accent::primary());
    addAndMakeVisible(presetsSection.get());
    
    // Category filter - EXPANDED list to match scanned folders
    categoryCombo.addItem("All Presets", 1);
    categoryCombo.addItem("Bass", 2);
    categoryCombo.addItem("Lead", 3);
    categoryCombo.addItem("Pad", 4);
    categoryCombo.addItem("Keys", 5);
    categoryCombo.addItem("Strings", 6);
    categoryCombo.addItem("Brass", 7);
    categoryCombo.addItem("Synth", 8);
    categoryCombo.addItem("FX", 9);
    categoryCombo.addItem("Bells", 10);
    categoryCombo.addItem("Drums", 11);
    categoryCombo.addItem("Organ", 12);
    categoryCombo.addItem("Banks", 13);
    categoryCombo.addItem("Tools", 14);
    categoryCombo.addItem("Other", 15);
    categoryCombo.setSelectedId(1);
    categoryCombo.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
    categoryCombo.setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
    categoryCombo.setColour(juce::ComboBox::outlineColourId, DS::Color::border());
    categoryCombo.onChange = [this]() { filterPresets(); rebuildPresetCards(); };
    addAndMakeVisible(categoryCombo);
    
    // Search box
    searchBox.setTextToShowWhenEmpty("Search presets...", juce::Colours::grey);
    searchBox.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
    searchBox.setColour(juce::TextEditor::textColourId, DS::Color::textPrimary());
    searchBox.setColour(juce::TextEditor::outlineColourId, DS::Color::border());
    searchBox.onTextChange = [this]() { filterPresets(); rebuildPresetCards(); };
    addAndMakeVisible(searchBox);
    
    // Grid/List toggle button
    gridToggleBtn.setButtonText("Grid");
    gridToggleBtn.setClickingTogglesState(true);
    gridToggleBtn.setToggleState(true, juce::dontSendNotification);
    gridToggleBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
    gridToggleBtn.setColour(juce::TextButton::buttonOnColourId, DS::Color::Accent::primary().withAlpha(0.3f));
    gridToggleBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
    gridToggleBtn.setColour(juce::TextButton::textColourOnId, DS::Color::Accent::primary());
    gridToggleBtn.onClick = [this]() { 
        useGridView = gridToggleBtn.getToggleState();
        gridToggleBtn.setButtonText(useGridView ? "Grid" : "List");
        resized();
    };
    addAndMakeVisible(gridToggleBtn);
    
    // Preset list (for list view)
    presetList.setModel(this);
    presetList.setRowHeight(32);
    presetList.setColour(juce::ListBox::backgroundColourId, DS::Color::bgCard());
    presetList.setColour(juce::ListBox::outlineColourId, DS::Color::border());
    addAndMakeVisible(presetList);
    
    // Preset grid viewport
    presetGridViewport.setViewedComponent(&presetGridContent, false);
    presetGridViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(presetGridViewport);
    
    // Action buttons
    loadBtn.setButtonText("Load");
    loadBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::primary().withAlpha(0.3f));
    loadBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::primary());
    loadBtn.onClick = [this]() { 
        loadSelectedPreset(); 
        if (onPresetLoaded && selectedPresetIndex >= 0 && selectedPresetIndex < static_cast<int>(filteredPresets.size()))
            onPresetLoaded(selectedPresetIndex, filteredPresets[selectedPresetIndex].name);
    };
    addAndMakeVisible(loadBtn);
    
    saveBtn.setButtonText("Save");
    saveBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::warning().withAlpha(0.3f));
    saveBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::warning());
    saveBtn.onClick = [this]() { saveCurrentPreset(); };
    addAndMakeVisible(saveBtn);
    
    sendBtn.setButtonText("Send");
    sendBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::success().withAlpha(0.3f));
    sendBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::success());
    sendBtn.onClick = [this]() { sendToSynth(); };
    addAndMakeVisible(sendBtn);
    
    requestBtn.setButtonText("Request");
    requestBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::cyan().withAlpha(0.3f));
    requestBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::cyan());
    requestBtn.onClick = [this]() { requestFromSynth(); };
    addAndMakeVisible(requestBtn);
    
    initBtn.setButtonText("Init");
    initBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
    initBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
    initBtn.onClick = [this]() { initPreset(); };
    addAndMakeVisible(initBtn);
    
    // Status label
    statusLabel.setText("Ready", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
    statusLabel.setFont(DS::Font::small());
    addAndMakeVisible(statusLabel);
    
    // Preset count
    presetCountLabel.setText("0 presets", juce::dontSendNotification);
    presetCountLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
    presetCountLabel.setFont(DS::Font::small());
    addAndMakeVisible(presetCountLabel);
    
    // === EXPLANATION SECTION (Right side) ===
    explanationSection = std::make_unique<GlowingSection>("AI Analysis", DS::Color::Accent::cyan());
    addAndMakeVisible(explanationSection.get());
    addAndMakeVisible(explanationPanel);
}

void LibrarianTab::paint(juce::Graphics& g)
{
    g.fillAll(DS::Color::bg());
}

void LibrarianTab::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    const int rightPanelWidth = 320;  // AI explanation panel width - INCREASED from 280
    const int gap = 10;
    
    // === RIGHT SIDE: AI Explanation Panel ===
    auto rightPanel = bounds.removeFromRight(rightPanelWidth);
    explanationSection->setBounds(rightPanel);
    auto explContent = explanationSection->getContentBounds();
    explanationPanel.setBounds(explContent);
    
    bounds.removeFromRight(gap);
    
    // === LEFT SIDE: Presets ===
    presetsSection->setBounds(bounds);
    auto presetsContent = presetsSection->getContentBounds();
    
    // Top row: category, search, grid toggle - INCREASED HEIGHT from 30 to 36
    auto topRow = presetsContent.removeFromTop(36);
    categoryCombo.setBounds(topRow.removeFromLeft(140));  // Slightly wider
    topRow.removeFromLeft(8);
    gridToggleBtn.setBounds(topRow.removeFromRight(55));
    topRow.removeFromRight(8);
    presetCountLabel.setBounds(topRow.removeFromRight(90));
    topRow.removeFromRight(8);
    searchBox.setBounds(topRow);
    
    presetsContent.removeFromTop(10);
    
    // Bottom row: action buttons and status
    auto bottomRow = presetsContent.removeFromBottom(32);
    int btnWidth = 65;
    int btnGap = 5;
    
    loadBtn.setBounds(bottomRow.removeFromLeft(btnWidth));
    bottomRow.removeFromLeft(btnGap);
    saveBtn.setBounds(bottomRow.removeFromLeft(btnWidth));
    bottomRow.removeFromLeft(btnGap);
    sendBtn.setBounds(bottomRow.removeFromLeft(btnWidth));
    bottomRow.removeFromLeft(btnGap);
    requestBtn.setBounds(bottomRow.removeFromLeft(btnWidth));
    bottomRow.removeFromLeft(btnGap);
    initBtn.setBounds(bottomRow.removeFromLeft(btnWidth));
    bottomRow.removeFromLeft(15);
    statusLabel.setBounds(bottomRow);
    
    presetsContent.removeFromBottom(8);
    
    // Preset display area
    if (useGridView)
    {
        presetList.setVisible(false);
        presetGridViewport.setVisible(true);
        presetGridViewport.setBounds(presetsContent);
        layoutPresetGrid();
    }
    else
    {
        presetGridViewport.setVisible(false);
        presetList.setVisible(true);
        presetList.setBounds(presetsContent);
    }
}

juce::File LibrarianTab::getPresetsFolder() const
{
    // Try relative to executable first
    auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    auto presetsFolder = appFile.getParentDirectory().getChildFile("Presets");
    
    if (!presetsFolder.exists())
    {
        // Go up from Builds folder structure
        presetsFolder = appFile.getParentDirectory().getParentDirectory()
                              .getParentDirectory().getParentDirectory()
                              .getChildFile("Presets");
    }
    
    // Fallback to hardcoded dev path
    if (!presetsFolder.exists())
    {
        presetsFolder = juce::File("C:\\Users\\User\\Desktop\\synths editors\\MKS70 Editor\\Presets");
    }
    
    DBG("LibrarianTab: Presets folder = " + presetsFolder.getFullPathName());
    return presetsFolder;
}

void LibrarianTab::loadPresetsFromFolder()
{
    allPresets.clear();
    
    auto presetsFolder = getPresetsFolder();
    if (!presetsFolder.exists())
    {
        DBG("LibrarianTab: Presets folder not found!");
        presetCountLabel.setText("Presets folder not found", juce::dontSendNotification);
        return;
    }
    
    // Scan category folders
    juce::StringArray categories = { "Bass", "Lead", "Pad", "Keys", "Strings", 
                                      "Brass", "Synth", "FX", "Bells", "Drums",
                                      "Organ", "Other", "Banks", "Tools" };

    auto isRealPresetFile = [](const juce::File& file)
    {
        const auto fileName = file.getFileName();
        const auto fullPath = file.getFullPathName();
        return !fileName.startsWith("._") &&
               !fullPath.containsIgnoreCase(juce::File::getSeparatorString() + "__MACOSX" + juce::File::getSeparatorString());
    };
    
    for (const auto& category : categories)
    {
        auto catFolder = presetsFolder.getChildFile(category);
        if (catFolder.exists() && catFolder.isDirectory())
        {
            for (const auto& file : catFolder.findChildFiles(juce::File::findFiles, false, "*.syx;*.SYX"))
            {
                if (!isRealPresetFile(file))
                    continue;

                PresetInfo preset;
                preset.name = file.getFileNameWithoutExtension();
                preset.category = category;
                preset.filePath = file.getFullPathName();
                allPresets.push_back(preset);
            }
        }
    }
    
    // Also scan root Presets folder
    for (const auto& file : presetsFolder.findChildFiles(juce::File::findFiles, false, "*.syx;*.SYX"))
    {
        if (!isRealPresetFile(file))
            continue;

        PresetInfo preset;
        preset.name = file.getFileNameWithoutExtension();
        preset.category = "Other";
        preset.filePath = file.getFullPathName();
        allPresets.push_back(preset);
    }
    
    filteredPresets = allPresets;
    presetList.updateContent();
    presetCountLabel.setText(juce::String(filteredPresets.size()) + " presets", juce::dontSendNotification);
    
    DBG("LibrarianTab: Loaded " + juce::String(allPresets.size()) + " presets");
}

void LibrarianTab::filterPresets()
{
    filteredPresets.clear();
    
    auto searchText = searchBox.getText().toLowerCase();
    auto categoryId = categoryCombo.getSelectedId();
    juce::String categoryFilter;
    
    // Map combo IDs to category names - EXPANDED to match all scanned folders
    switch (categoryId)
    {
        case 2: categoryFilter = "Bass"; break;
        case 3: categoryFilter = "Lead"; break;
        case 4: categoryFilter = "Pad"; break;
        case 5: categoryFilter = "Keys"; break;
        case 6: categoryFilter = "Strings"; break;
        case 7: categoryFilter = "Brass"; break;
        case 8: categoryFilter = "Synth"; break;
        case 9: categoryFilter = "FX"; break;
        case 10: categoryFilter = "Bells"; break;
        case 11: categoryFilter = "Drums"; break;
        case 12: categoryFilter = "Organ"; break;
        case 13: categoryFilter = "Banks"; break;
        case 14: categoryFilter = "Tools"; break;
        case 15: categoryFilter = "Other"; break;
        default: categoryFilter = ""; // All (id=1)
    }
    
    DBG("LibrarianTab::filterPresets() - Category: " + categoryFilter + ", Search: " + searchText);
    
    for (const auto& preset : allPresets)
    {
        bool matchesCategory = categoryFilter.isEmpty() || preset.category == categoryFilter;
        bool matchesSearch = searchText.isEmpty() || preset.name.toLowerCase().contains(searchText);
        
        if (matchesCategory && matchesSearch)
            filteredPresets.push_back(preset);
    }
    
    DBG("  -> Filtered " + juce::String(filteredPresets.size()) + " presets from " + juce::String(allPresets.size()) + " total");
    
    presetList.updateContent();
    presetCountLabel.setText(juce::String(filteredPresets.size()) + " presets", juce::dontSendNotification);
}

// ListBoxModel implementation
int LibrarianTab::getNumRows()
{
    return static_cast<int>(filteredPresets.size());
}

void LibrarianTab::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= static_cast<int>(filteredPresets.size()))
        return;
    
    auto& preset = filteredPresets[rowNumber];
    
    if (rowIsSelected)
        g.fillAll(DS::Color::Accent::primary().withAlpha(0.3f));
    else if (rowNumber % 2 == 0)
        g.fillAll(DS::Color::bgElevated().withAlpha(0.3f));
    
    g.setColour(DS::Color::textPrimary());
    g.setFont(14.0f);
    g.drawText(preset.name, 10, 0, width - 100, height, juce::Justification::centredLeft);
    
    g.setColour(DS::Color::textSecondary());
    g.setFont(12.0f);
    g.drawText(preset.category, width - 90, 0, 80, height, juce::Justification::centredRight);
}

void LibrarianTab::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    selectPreset(row);
}

void LibrarianTab::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    selectPreset(row);
    loadSelectedPreset();
    sendToSynth();
}

void LibrarianTab::selectPreset(int index)
{
    if (index >= 0 && index < static_cast<int>(filteredPresets.size()))
    {
        selectedPresetIndex = index;
        statusLabel.setText("Selected: " + filteredPresets[index].name, juce::dontSendNotification);
    }
}

void LibrarianTab::loadSelectedPreset()
{
    if (selectedPresetIndex < 0 || selectedPresetIndex >= static_cast<int>(filteredPresets.size()))
    {
        statusLabel.setText("No preset selected", juce::dontSendNotification);
        return;
    }
    
    auto& preset = filteredPresets[selectedPresetIndex];
    juce::File file(preset.filePath);
    
    if (!file.existsAsFile())
    {
        statusLabel.setText("File not found", juce::dontSendNotification);
        return;
    }
    
    juce::MemoryBlock data;
    if (file.loadFileAsData(data))
    {
        // Store the loaded SysEx data
        loadedSysexData = data;
        currentPresetName = preset.name;
        
        PresetManager presetManager;
        auto tones = presetManager.importSysExFile(file);
        if (!tones.empty() && onLoadToneToEditor)
        {
            onLoadToneToEditor(tones[0], true);
            if (tones.size() > 1)
                onLoadToneToEditor(tones[1], false);
        }

        statusLabel.setText("Loaded: " + preset.name, juce::dontSendNotification);
        DBG("LibrarianTab: Loaded " + juce::String(data.getSize()) + " bytes from " + file.getFullPathName());
    }
    else
    {
        statusLabel.setText("Failed to load file", juce::dontSendNotification);
    }
}

void LibrarianTab::saveCurrentPreset()
{
    // File chooser for saving
    auto chooser = std::make_shared<juce::FileChooser>(
        "Save Preset",
        getPresetsFolder(),
        "*.syx"
    );
    
    chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File())
            {
                if (!onGetCurrentToneUpper || !onGetCurrentToneLower)
                {
                    statusLabel.setText("Save callbacks not set", juce::dontSendNotification);
                    return;
                }

                ToneData upper(onGetCurrentToneUpper());
                ToneData lower(onGetCurrentToneLower());
                upper.toneName = file.getFileNameWithoutExtension().substring(0, 10);
                lower.toneName = (file.getFileNameWithoutExtension() + " L").substring(0, 10);

                PresetManager presetManager;
                std::vector<ToneData> tones { upper, lower };
                auto syxFile = file.withFileExtension(".syx");

                if (presetManager.exportToSysEx(tones, syxFile))
                {
                    statusLabel.setText("Saved: " + syxFile.getFileNameWithoutExtension(), juce::dontSendNotification);
                    loadPresetsFromFolder();
                    filterPresets();
                    rebuildPresetCards();
                }
                else
                {
                    statusLabel.setText("Save failed", juce::dontSendNotification);
                }
            }
        });
}

void LibrarianTab::sendToSynth()
{
    if (selectedPresetIndex < 0 || selectedPresetIndex >= static_cast<int>(filteredPresets.size()))
    {
        statusLabel.setText("No preset selected", juce::dontSendNotification);
        return;
    }
    
    // Prefer parsed tone data so editor-generated files go through the normal value mapping.
    if (loadedSysexData.getSize() > 0 && onSendPatchToSynth && onGetCurrentToneUpper && onGetCurrentToneLower && onGetCurrentPatch)
    {
        PresetManager presetManager;
        auto tones = presetManager.importSysExFile(juce::File(filteredPresets[selectedPresetIndex].filePath));
        if (!tones.empty())
        {
            ToneParams upper = tones[0];
            ToneParams lower = tones.size() > 1 ? ToneParams(tones[1]) : onGetCurrentToneLower();
            auto patch = onGetCurrentPatch();
            onSendPatchToSynth(upper, lower, patch);
            statusLabel.setText("Sent: " + currentPresetName, juce::dontSendNotification);
            return;
        }
    }

    // Fallback for external/raw SysEx that the editor cannot parse yet.
    if (loadedSysexData.getSize() > 0 && onSendSysExToSynth)
    {
        onSendSysExToSynth(loadedSysexData);
        statusLabel.setText("Sent: " + currentPresetName, juce::dontSendNotification);
        DBG("LibrarianTab: Sent " + juce::String(loadedSysexData.getSize()) + " bytes to synth");
    }
    else if (onGetCurrentToneUpper && onGetCurrentToneLower && onGetCurrentPatch && onSendPatchToSynth)
    {
        // Fallback: send current editor parameters
        auto upper = onGetCurrentToneUpper();
        auto lower = onGetCurrentToneLower();
        auto patch = onGetCurrentPatch();
        onSendPatchToSynth(upper, lower, patch);
        statusLabel.setText("Sent current settings", juce::dontSendNotification);
    }
    else
    {
        statusLabel.setText("Send callbacks not set", juce::dontSendNotification);
        DBG("LibrarianTab: ERROR - No send callbacks available!");
    }
}

void LibrarianTab::requestFromSynth()
{
    if (onRequestPatchFromSynth)
    {
        onRequestPatchFromSynth();
        statusLabel.setText("Requested from synth", juce::dontSendNotification);
    }
}

void LibrarianTab::initPreset()
{
    statusLabel.setText("Initialized preset", juce::dontSendNotification);
}

// Public API for HeaderBar navigation
int LibrarianTab::getPresetCount() const
{
    DBG("LibrarianTab::getPresetCount() = " + juce::String(filteredPresets.size()));
    return static_cast<int>(filteredPresets.size());
}

juce::String LibrarianTab::getPresetName(int index) const
{
    if (index >= 0 && index < static_cast<int>(filteredPresets.size()))
        return filteredPresets[index].name;
    return "";
}

std::vector<juce::String> LibrarianTab::getPresetNames() const
{
    std::vector<juce::String> names;
    for (const auto& preset : filteredPresets)
        names.push_back(preset.name);
    return names;
}

void LibrarianTab::selectPresetByIndex(int index)
{
    DBG("LibrarianTab::selectPresetByIndex(" + juce::String(index) + ")");
    
    if (index < 0 || index >= static_cast<int>(filteredPresets.size()))
    {
        DBG("  -> Index out of range! filteredPresets.size() = " + juce::String(filteredPresets.size()));
        return;
    }
    
    selectedPresetIndex = index;
    presetList.selectRow(index);
    presetList.scrollToEnsureRowIsOnscreen(index);
    
    // Load and send the preset
    loadSelectedPreset();
    sendToSynth();
    
    statusLabel.setText("Playing: " + filteredPresets[index].name, juce::dontSendNotification);
    DBG("  -> Selected and sent: " + filteredPresets[index].name);
    
    // Notify HeaderBar that preset was loaded
    if (onPresetLoaded)
        onPresetLoaded(index, filteredPresets[index].name);
}

int LibrarianTab::getSelectedIndex() const
{
    return selectedPresetIndex;
}

void LibrarianTab::rebuildPresetCards()
{
    presetCards.clear();
    selectedCard = nullptr;
    
    for (size_t i = 0; i < filteredPresets.size(); ++i)
    {
        auto* card = new PresetCard(filteredPresets[i]);
        card->onClick = [this, i](PresetCard* c) { 
            selectCard(c); 
            selectedPresetIndex = static_cast<int>(i);
            selectPreset(static_cast<int>(i));
            
            // Update explanation panel
            loadSelectedPreset();
            if (loadedSysexData.getSize() > 0)
                explanationPanel.setPreset(filteredPresets[selectedPresetIndex], loadedSysexData);
        };
        card->onDoubleClick = [this](PresetCard*) { 
            sendToSynth();
            if (onPresetLoaded && selectedPresetIndex >= 0 && selectedPresetIndex < static_cast<int>(filteredPresets.size()))
                onPresetLoaded(selectedPresetIndex, filteredPresets[selectedPresetIndex].name);
        };
        presetCards.add(card);
        presetGridContent.addAndMakeVisible(card);
    }
    
    layoutPresetGrid();
}

void LibrarianTab::layoutPresetGrid()
{
    const int cardWidth = 150;
    const int cardHeight = 70;
    const int gap = 8;
    
    int viewportWidth = presetGridViewport.getWidth() - presetGridViewport.getScrollBarThickness();
    int cols = juce::jmax(1, viewportWidth / (cardWidth + gap));
    
    int x = 0, y = 0;
    int col = 0;
    
    for (auto* card : presetCards)
    {
        card->setBounds(x, y, cardWidth, cardHeight);
        
        col++;
        if (col >= cols)
        {
            col = 0;
            x = 0;
            y += cardHeight + gap;
        }
        else
        {
            x += cardWidth + gap;
        }
    }
    
    int totalHeight = y + (col > 0 ? cardHeight + gap : 0);
    presetGridContent.setSize(viewportWidth, juce::jmax(totalHeight, presetGridViewport.getHeight()));
}

void LibrarianTab::selectCard(PresetCard* card)
{
    if (selectedCard)
        selectedCard->setSelected(false);
    
    selectedCard = card;
    
    if (card)
        card->setSelected(true);
}
