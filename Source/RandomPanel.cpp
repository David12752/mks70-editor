/*
  ==============================================================================

    RandomPanel.cpp
    Random Generator UI Panel implementation

  ==============================================================================
*/

#include "RandomPanel.h"

//==============================================================================
RandomPanel::RandomPanel()
{
    setupCategoryButtons();
    setupControls();
    setupLockedParams();
    setupVariation();
    setupActions();
    setupHistory();
    
    // Initialize random generator with default settings
    randomGenerator.enableMusicalConstraints(true);
    randomGenerator.setRandomness(0.75f);
    randomGenerator.setComplexity(0.45f);
}

RandomPanel::~RandomPanel()
{
}

void RandomPanel::setProcessor(MKS70EditorAudioProcessor* proc)
{
    processor = proc;
}

//==============================================================================
// Setup Methods

void RandomPanel::setupCategoryButtons()
{
    // Group
    addAndMakeVisible(categoryGroup);
    categoryGroup.setText("CATEGORY");
    categoryGroup.setTextLabelPosition(juce::Justification::centredTop);
    
    addAndMakeVisible(categoryLabel);
    categoryLabel.setText("Select sound type:", juce::dontSendNotification);
    categoryLabel.setJustificationType(juce::Justification::centredLeft);
    categoryLabel.setFont(juce::FontOptions(14.0f));
    
    // Category buttons (10 categories)
    const char* categoryNames[] = {
        "Bass", "Pad", "Lead", "Brass", "Strings",
        "Keys", "Bells", "FX", "Perc", "Experimental"
    };
    
    for (int i = 0; i < 10; i++)
    {
        addAndMakeVisible(categoryButtons[i]);
        categoryButtons[i].setButtonText(categoryNames[i]);
        categoryButtons[i].addListener(this);
        categoryButtons[i].setClickingTogglesState(true);
        categoryButtons[i].setRadioGroupId(1001);
    }
    
    // Select Bass by default
    categoryButtons[0].setToggleState(true, juce::dontSendNotification);
}

void RandomPanel::setupControls()
{
    // Group
    addAndMakeVisible(controlsGroup);
    controlsGroup.setText("CONTROLS");
    controlsGroup.setTextLabelPosition(juce::Justification::centredTop);
    
    // Randomness slider
    addAndMakeVisible(randomnessSlider);
    randomnessSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    randomnessSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    randomnessSlider.setRange(0.0, 1.0, 0.01);
    randomnessSlider.setValue(0.75);
    randomnessSlider.addListener(this);
    
    addAndMakeVisible(randomnessLabel);
    randomnessLabel.setText("Randomness:", juce::dontSendNotification);
    randomnessLabel.setJustificationType(juce::Justification::centredLeft);
    randomnessLabel.setFont(juce::FontOptions(13.0f));
    
    addAndMakeVisible(randomnessValueLabel);
    randomnessValueLabel.setText("75%", juce::dontSendNotification);
    randomnessValueLabel.setJustificationType(juce::Justification::centredRight);
    randomnessValueLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    
    // Complexity slider
    addAndMakeVisible(complexitySlider);
    complexitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    complexitySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    complexitySlider.setRange(0.0, 1.0, 0.01);
    complexitySlider.setValue(0.45);
    complexitySlider.addListener(this);
    
    addAndMakeVisible(complexityLabel);
    complexityLabel.setText("Complexity:", juce::dontSendNotification);
    complexityLabel.setJustificationType(juce::Justification::centredLeft);
    complexityLabel.setFont(juce::FontOptions(13.0f));
    
    addAndMakeVisible(complexityValueLabel);
    complexityValueLabel.setText("45%", juce::dontSendNotification);
    complexityValueLabel.setJustificationType(juce::Justification::centredRight);
    complexityValueLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    
    // Toggle buttons
    addAndMakeVisible(musicalConstraintsToggle);
    musicalConstraintsToggle.setButtonText("Musical Constraints (avoid harsh sounds)");
    musicalConstraintsToggle.setToggleState(true, juce::dontSendNotification);
    musicalConstraintsToggle.addListener(this);
    
    addAndMakeVisible(keepPatchSettingsToggle);
    keepPatchSettingsToggle.setButtonText("Keep current Patch settings");
    keepPatchSettingsToggle.setToggleState(false, juce::dontSendNotification);
    keepPatchSettingsToggle.addListener(this);
    
    addAndMakeVisible(randomizeArpToggle);
    randomizeArpToggle.setButtonText("Randomize Arpeggiator too");
    randomizeArpToggle.setToggleState(false, juce::dontSendNotification);
    randomizeArpToggle.addListener(this);
}

void RandomPanel::setupLockedParams()
{
    // Group
    addAndMakeVisible(lockedParamsGroup);
    lockedParamsGroup.setText("LOCKED PARAMETERS");
    lockedParamsGroup.setTextLabelPosition(juce::Justification::centredTop);
    
    addAndMakeVisible(lockedParamsLabel);
    lockedParamsLabel.setText("Click to lock current value:", juce::dontSendNotification);
    lockedParamsLabel.setJustificationType(juce::Justification::centredLeft);
    lockedParamsLabel.setFont(juce::FontOptions(13.0f));
    
    // Add lock button
    addAndMakeVisible(addLockButton);
    addLockButton.setButtonText("+ Add Lock...");
    addLockButton.addListener(this);
    
    // Viewport for locked params
    addAndMakeVisible(lockedParamsViewport);
    lockedParamsViewport.setViewedComponent(&lockedParamsContainer, false);
    addChildComponent(lockedParamsContainer);
}

void RandomPanel::setupVariation()
{
    // Group
    addAndMakeVisible(variationGroup);
    variationGroup.setText("VARIATION FROM CURRENT");
    variationGroup.setTextLabelPosition(juce::Justification::centredTop);
    
    addAndMakeVisible(variationLabel);
    variationLabel.setText("Amount of change:", juce::dontSendNotification);
    variationLabel.setJustificationType(juce::Justification::centredLeft);
    variationLabel.setFont(juce::FontOptions(13.0f));
    
    // Variation buttons
    const char* variationNames[] = { "Subtle 10%", "Medium 30%", "Wild 70%", "Total 100%" };
    
    for (int i = 0; i < 4; i++)
    {
        addAndMakeVisible(variationButtons[i]);
        variationButtons[i].setButtonText(variationNames[i]);
        variationButtons[i].addListener(this);
        variationButtons[i].setClickingTogglesState(true);
        variationButtons[i].setRadioGroupId(1002);
    }
    
    // Select Medium by default
    variationButtons[1].setToggleState(true, juce::dontSendNotification);
}

void RandomPanel::setupActions()
{
    // Generate button
    addAndMakeVisible(generateButton);
    generateButton.setButtonText("GENERATE");
    generateButton.addListener(this);
    generateButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff228B22));  // Green
    generateButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    
    // Undo button
    addAndMakeVisible(undoButton);
    undoButton.setButtonText("UNDO");
    undoButton.addListener(this);
    undoButton.setEnabled(false);
    
    // Favorite button
    addAndMakeVisible(favoriteButton);
    favoriteButton.setButtonText("FAVORITE");
    favoriteButton.addListener(this);
    favoriteButton.setEnabled(false);
    
    // Close button
    addAndMakeVisible(closeButton);
    closeButton.setButtonText("Close");
    closeButton.addListener(this);
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff8B0000));  // Dark red
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void RandomPanel::setupHistory()
{
    addAndMakeVisible(historyLabel);
    historyLabel.setText("History:", juce::dontSendNotification);
    historyLabel.setJustificationType(juce::Justification::centredLeft);
    historyLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    
    addAndMakeVisible(historyPrevButton);
    historyPrevButton.setButtonText("<");
    historyPrevButton.addListener(this);
    historyPrevButton.setEnabled(false);
    
    addAndMakeVisible(historyNextButton);
    historyNextButton.setButtonText(">");
    historyNextButton.addListener(this);
    historyNextButton.setEnabled(false);
    
    addAndMakeVisible(historyInfoLabel);
    historyInfoLabel.setText("No patches yet", juce::dontSendNotification);
    historyInfoLabel.setJustificationType(juce::Justification::centred);
    historyInfoLabel.setFont(juce::FontOptions(13.0f));
}

//==============================================================================
// Paint and Resized

void RandomPanel::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Title
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("RANDOM GENERATOR", 10, 10, getWidth() - 20, 30, juce::Justification::centred);
}

void RandomPanel::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    // Close button at top-right
    auto topBar = bounds.removeFromTop(40);
    closeButton.setBounds(topBar.removeFromRight(100).reduced(5));
    
    const int groupSpacing = 10;
    
    // Category buttons (top section)
    auto categoryArea = bounds.removeFromTop(120);
    categoryGroup.setBounds(categoryArea);
    
    auto categoryContent = categoryArea.reduced(10, 25);
    categoryLabel.setBounds(categoryContent.removeFromTop(20));
    categoryContent.removeFromTop(5);
    
    // 5 buttons per row
    auto row1 = categoryContent.removeFromTop(35);
    for (int i = 0; i < 5; i++)
    {
        categoryButtons[i].setBounds(row1.removeFromLeft(row1.getWidth() / (5 - i)));
        if (i < 4) row1.removeFromLeft(5);
    }
    
    categoryContent.removeFromTop(5);
    auto row2 = categoryContent.removeFromTop(35);
    for (int i = 5; i < 10; i++)
    {
        categoryButtons[i].setBounds(row2.removeFromLeft(row2.getWidth() / (10 - i)));
        if (i < 9) row2.removeFromLeft(5);
    }
    
    bounds.removeFromTop(groupSpacing);
    
    // Controls section
    auto controlsArea = bounds.removeFromTop(150);
    controlsGroup.setBounds(controlsArea);
    
    auto controlsContent = controlsArea.reduced(10, 25);
    
    // Randomness slider
    auto randomnessArea = controlsContent.removeFromTop(25);
    randomnessLabel.setBounds(randomnessArea.removeFromLeft(100));
    randomnessValueLabel.setBounds(randomnessArea.removeFromRight(50));
    randomnessArea.removeFromLeft(5);
    randomnessArea.removeFromRight(5);
    randomnessSlider.setBounds(randomnessArea);
    
    controlsContent.removeFromTop(10);
    
    // Complexity slider
    auto complexityArea = controlsContent.removeFromTop(25);
    complexityLabel.setBounds(complexityArea.removeFromLeft(100));
    complexityValueLabel.setBounds(complexityArea.removeFromRight(50));
    complexityArea.removeFromLeft(5);
    complexityArea.removeFromRight(5);
    complexitySlider.setBounds(complexityArea);
    
    controlsContent.removeFromTop(15);
    
    // Toggle buttons
    musicalConstraintsToggle.setBounds(controlsContent.removeFromTop(25));
    controlsContent.removeFromTop(5);
    keepPatchSettingsToggle.setBounds(controlsContent.removeFromTop(25));
    controlsContent.removeFromTop(5);
    randomizeArpToggle.setBounds(controlsContent.removeFromTop(25));
    
    bounds.removeFromTop(groupSpacing);
    
    // Locked parameters section
    auto lockedArea = bounds.removeFromTop(100);
    lockedParamsGroup.setBounds(lockedArea);
    
    auto lockedContent = lockedArea.reduced(10, 25);
    lockedParamsLabel.setBounds(lockedContent.removeFromTop(20));
    lockedContent.removeFromTop(5);
    
    auto lockedButtonArea = lockedContent.removeFromTop(30);
    addLockButton.setBounds(lockedButtonArea.removeFromLeft(120));
    lockedButtonArea.removeFromLeft(10);
    lockedParamsViewport.setBounds(lockedButtonArea);
    
    bounds.removeFromTop(groupSpacing);
    
    // Variation section
    auto variationArea = bounds.removeFromTop(90);
    variationGroup.setBounds(variationArea);
    
    auto variationContent = variationArea.reduced(10, 25);
    variationLabel.setBounds(variationContent.removeFromTop(20));
    variationContent.removeFromTop(5);
    
    auto varButtonsArea = variationContent.removeFromTop(35);
    for (int i = 0; i < 4; i++)
    {
        variationButtons[i].setBounds(varButtonsArea.removeFromLeft(varButtonsArea.getWidth() / (4 - i)));
        if (i < 3) varButtonsArea.removeFromLeft(5);
    }
    
    bounds.removeFromTop(groupSpacing);
    
    // Actions section
    auto actionsArea = bounds.removeFromTop(80);
    auto actionButtonsArea = actionsArea.removeFromTop(40);
    
    generateButton.setBounds(actionButtonsArea.removeFromLeft(150));
    actionButtonsArea.removeFromLeft(10);
    undoButton.setBounds(actionButtonsArea.removeFromLeft(100));
    actionButtonsArea.removeFromLeft(10);
    favoriteButton.setBounds(actionButtonsArea.removeFromLeft(120));
    
    actionsArea.removeFromTop(10);
    
    // History navigation
    auto historyArea = actionsArea.removeFromTop(30);
    historyLabel.setBounds(historyArea.removeFromLeft(60));
    historyPrevButton.setBounds(historyArea.removeFromLeft(30));
    historyArea.removeFromLeft(5);
    historyInfoLabel.setBounds(historyArea.removeFromRight(historyArea.getWidth() - 35));
    historyNextButton.setBounds(historyArea.removeFromRight(30));
}

//==============================================================================
// Component Listeners

void RandomPanel::buttonClicked(juce::Button* button)
{
    // Category buttons
    for (int i = 0; i < 10; i++)
    {
        if (button == &categoryButtons[i])
        {
            selectCategory(static_cast<RandomGenerator::SoundCategory>(i));
            return;
        }
    }
    
    // Variation buttons
    for (int i = 0; i < 4; i++)
    {
        if (button == &variationButtons[i])
        {
            selectedVariation = i;
            return;
        }
    }
    
    // Action buttons
    if (button == &generateButton)
    {
        generatePatch();
    }
    else if (button == &undoButton)
    {
        navigateHistory(-1);
    }
    else if (button == &favoriteButton)
    {
        toggleFavorite();
    }
    else if (button == &closeButton)
    {
        // Call close callback
        if (onClose)
            onClose();
    }
    else if (button == &historyPrevButton)
    {
        navigateHistory(-1);
    }
    else if (button == &historyNextButton)
    {
        navigateHistory(1);
    }
    else if (button == &addLockButton)
    {
        showAddLockDialog();
    }
    else if (button == &musicalConstraintsToggle)
    {
        randomGenerator.enableMusicalConstraints(musicalConstraintsToggle.getToggleState());
    }
    
    // Locked param buttons (remove)
    for (auto& lockBtn : lockedParamButtons)
    {
        if (button == lockBtn.get())
        {
            juce::String paramName = lockBtn->getButtonText();
            // Remove "" prefix and value suffix
            paramName = paramName.fromFirstOccurrenceOf(" ", false, false);
            paramName = paramName.upToFirstOccurrenceOf(":", false, false);
            removeLockedParam(paramName);
            return;
        }
    }
}

void RandomPanel::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &randomnessSlider)
    {
        float value = static_cast<float>(slider->getValue());
        randomGenerator.setRandomness(value);
        randomnessValueLabel.setText(juce::String(static_cast<int>(value * 100)) + "%", 
                                     juce::dontSendNotification);
    }
    else if (slider == &complexitySlider)
    {
        float value = static_cast<float>(slider->getValue());
        randomGenerator.setComplexity(value);
        complexityValueLabel.setText(juce::String(static_cast<int>(value * 100)) + "%", 
                                     juce::dontSendNotification);
    }
}

void RandomPanel::comboBoxChanged(juce::ComboBox* comboBox)
{
    // Future: parameter lock combo boxes
}

//==============================================================================
// Category Methods

void RandomPanel::selectCategory(RandomGenerator::SoundCategory category)
{
    selectedCategory = category;
    updateCategoryButtons();
    DBG("Selected category: " + RandomGenerator::getCategoryName(category));
}

void RandomPanel::updateCategoryButtons()
{
    for (int i = 0; i < 10; i++)
    {
        bool isSelected = (static_cast<int>(selectedCategory) == i);
        categoryButtons[i].setToggleState(isSelected, juce::dontSendNotification);
    }
}

juce::String RandomPanel::getCategoryButtonText(RandomGenerator::SoundCategory category)
{
    return RandomGenerator::getCategoryName(category);
}

//==============================================================================
// Generation Methods

void RandomPanel::generatePatch()
{
    if (!processor)
    {
        DBG("RandomPanel: No processor set");
        return;
    }
    
    FullPatch patch;
    
    // Check if we should generate variation or new patch
    if (keepPatchSettingsToggle.getToggleState() || selectedVariation < 3)
    {
        // Generate variation from current
        FullPatch currentPatch;
        currentPatch.toneUpper = processor->getToneUpper();
        currentPatch.toneLower = processor->getToneLower();
        currentPatch.patch = processor->getPatchParameters();
        currentPatch.arp = processor->getArpParameters();
        
        float variation = getVariationAmount(selectedVariation);
        patch = randomGenerator.generateFromTemplate(currentPatch, variation);
        
        // Keep patch settings if requested
        if (keepPatchSettingsToggle.getToggleState())
        {
            patch.patch = currentPatch.patch;
        }
        
        // Don't randomize arp unless requested
        if (!randomizeArpToggle.getToggleState())
        {
            patch.arp = currentPatch.arp;
        }
    }
    else
    {
        // Generate completely new patch from category
        patch = randomGenerator.generate(selectedCategory);
        
        // Keep patch settings if requested
        if (keepPatchSettingsToggle.getToggleState())
        {
            FullPatch currentPatch;
            currentPatch.patch = processor->getPatchParameters();
            patch.patch = currentPatch.patch;
        }
        
        // Don't randomize arp unless requested
        if (!randomizeArpToggle.getToggleState())
        {
            FullPatch currentPatch;
            currentPatch.arp = processor->getArpParameters();
            patch.arp = currentPatch.arp;
        }
    }
    
    // Apply to processor
    applyPatchToProcessor(patch);
    
    // Add to history
    addToHistory(patch, RandomGenerator::getCategoryName(selectedCategory));
    
    DBG("RandomPanel: Generated " + RandomGenerator::getCategoryName(selectedCategory) + " patch");
}

void RandomPanel::applyPatchToProcessor(const FullPatch& patch)
{
    if (!processor)
        return;
    
    processor->setToneParameters(true, patch.toneUpper, true);
    processor->setToneParameters(false, patch.toneLower, true);
    processor->setPatchParameters(patch.patch, true);
    processor->setArpParameters(patch.arp, false);

    // Ask host UI to refresh
    if (onPatchApplied)
        onPatchApplied();
}

float RandomPanel::getVariationAmount(int index)
{
    const float amounts[] = { 0.1f, 0.3f, 0.7f, 1.0f };
    return amounts[juce::jlimit(0, 3, index)];
}

//==============================================================================
// History Methods

void RandomPanel::addToHistory(const FullPatch& patch, const juce::String& categoryName)
{
    // If we're not at the end of history, remove everything after current position
    if (currentHistoryIndex >= 0 && currentHistoryIndex < static_cast<int>(history.size()) - 1)
    {
        history.erase(history.begin() + currentHistoryIndex + 1, history.end());
    }
    
    // Add new entry
    HistoryEntry entry;
    entry.patch = patch;
    entry.categoryName = categoryName;
    entry.timestamp = juce::Time::getCurrentTime();
    entry.isFavorite = false;
    
    history.push_back(entry);
    
    // Limit history size
    if (history.size() > MAX_HISTORY)
    {
        history.erase(history.begin());
    }
    
    // Move to new patch
    currentHistoryIndex = static_cast<int>(history.size()) - 1;
    
    updateHistoryUI();
}

void RandomPanel::navigateHistory(int direction)
{
    if (history.empty())
        return;
    
    int newIndex = currentHistoryIndex + direction;
    
    if (newIndex < 0 || newIndex >= static_cast<int>(history.size()))
        return;
    
    currentHistoryIndex = newIndex;
    
    // Apply patch
    applyPatchToProcessor(history[currentHistoryIndex].patch);
    
    updateHistoryUI();
    
    DBG("RandomPanel: Navigated to history index " + juce::String(currentHistoryIndex));
}

void RandomPanel::updateHistoryUI()
{
    bool hasHistory = !history.empty();
    bool canGoPrev = currentHistoryIndex > 0;
    bool canGoNext = currentHistoryIndex < static_cast<int>(history.size()) - 1;
    
    undoButton.setEnabled(hasHistory);
    favoriteButton.setEnabled(hasHistory);
    historyPrevButton.setEnabled(canGoPrev);
    historyNextButton.setEnabled(canGoNext);
    
    if (hasHistory)
    {
        juce::String info = "Patch " + 
                           juce::String(currentHistoryIndex + 1) + 
                           " of " + 
                           juce::String(history.size());
        
        if (history[currentHistoryIndex].isFavorite)
            info += " [FAV]";
        
        historyInfoLabel.setText(info, juce::dontSendNotification);
        
        // Update favorite button color
        auto& currentEntry = history[currentHistoryIndex];
        favoriteButton.setColour(juce::TextButton::buttonColourId, 
                                currentEntry.isFavorite ? juce::Colour(0xffff1493) : juce::Colour(0xff404040));
    }
    else
    {
        historyInfoLabel.setText("No patches yet", juce::dontSendNotification);
    }
}

void RandomPanel::toggleFavorite()
{
    if (currentHistoryIndex < 0 || currentHistoryIndex >= static_cast<int>(history.size()))
        return;
    
    history[currentHistoryIndex].isFavorite = !history[currentHistoryIndex].isFavorite;
    updateHistoryUI();
    
    DBG("RandomPanel: Toggled favorite for patch " + juce::String(currentHistoryIndex));
}

//==============================================================================
// Locked Parameters Methods

void RandomPanel::showAddLockDialog()
{
    // For now, show a simple alert - in production, show a parameter selector dialog
    juce::AlertWindow::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        "Add Lock",
        "Parameter locking dialog coming soon!\n\n"
        "This will allow you to lock specific parameters\n"
        "to their current values during generation.",
        "OK"
    );
}

void RandomPanel::addLockedParam(const juce::String& paramName, int value)
{
    randomGenerator.lockParameter(paramName, value);
    refreshLockedParamsUI();
    
    DBG("RandomPanel: Locked " + paramName + " to " + juce::String(value));
}

void RandomPanel::removeLockedParam(const juce::String& paramName)
{
    randomGenerator.unlockParameter(paramName);
    refreshLockedParamsUI();
    
    DBG("RandomPanel: Unlocked " + paramName);
}

void RandomPanel::refreshLockedParamsUI()
{
    // Clear existing buttons
    lockedParamButtons.clear();
    
    // Rebuild from randomGenerator's locked params
    // (In production, would query randomGenerator for locked params)
    
    // Layout locked param buttons
    int xPos = 0;
    for (auto& btn : lockedParamButtons)
    {
        btn->setBounds(xPos, 0, 150, 25);
        xPos += 155;
    }
    
    lockedParamsContainer.setSize(xPos, 25);
    lockedParamsViewport.setViewedComponent(&lockedParamsContainer, false);
}
