/*
  ==============================================================================

    ComparePanel.cpp
    Compare Mode panel implementation

  ==============================================================================
*/

#include "ComparePanel.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ComparePanel::ComparePanel()
{
    setupUI();
    
    // Setup key listener
    setWantsKeyboardFocus(true);
    addKeyListener(this);
}

ComparePanel::~ComparePanel()
{
    removeKeyListener(this);
}

void ComparePanel::paint(juce::Graphics& g)
{
    // Modern dark background with subtle gradient
    juce::ColourGradient bgGrad(juce::Colour(0xFF1A1A24), 0, 0,
                                 juce::Colour(0xFF12121A), 0, (float)getHeight(), false);
    g.setGradientFill(bgGrad);
    g.fillAll();
    
    // Subtle border with glow
    g.setColour(juce::Colour(0xFF6366F1).withAlpha(0.3f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 8.0f, 2.0f);
    
    // Draw differences table
    drawDifferencesTable(g);
}

void ComparePanel::resized()
{
    auto bounds = getLocalBounds().reduced(16);
    
    // Compact title bar
    auto titleBar = bounds.removeFromTop(32);
    titleLabel.setBounds(titleBar.removeFromLeft(150));
    closeButton.setBounds(titleBar.removeFromRight(70).reduced(2));
    
    bounds.removeFromTop(12);
    
    // Slot panels - more compact horizontal layout
    auto slotArea = bounds.removeFromTop(80);
    int slotWidth = (slotArea.getWidth() - 60) / 2;
    
    auto slotAArea = slotArea.removeFromLeft(slotWidth);
    slotAGroup.setBounds(slotAArea);
    auto slotAContent = slotAArea.reduced(8, 6);
    slotANameLabel.setBounds(slotAContent.removeFromTop(22));
    slotAActiveButton.setBounds(slotAContent.reduced(16, 4));
    
    auto swapArea = slotArea.removeFromLeft(60);
    swapButton.setBounds(swapArea.reduced(8, 20));
    
    auto slotBArea = slotArea;
    slotBGroup.setBounds(slotBArea);
    auto slotBContent = slotBArea.reduced(8, 6);
    slotBNameLabel.setBounds(slotBContent.removeFromTop(22));
    slotBActiveButton.setBounds(slotBContent.reduced(16, 4));
    
    bounds.removeFromTop(8);
    
    // Filter and differences in compact row
    auto filterRow = bounds.removeFromTop(28);
    differencesLabel.setBounds(filterRow.removeFromLeft(200));
    filterRow.removeFromLeft(8);
    showAllToggle.setBounds(filterRow.removeFromLeft(50));
    showChangedOnlyToggle.setBounds(filterRow.removeFromLeft(100));
    filterRow.removeFromLeft(8);
    sectionFilterCombo.setBounds(filterRow.removeFromLeft(140));
    filterLabel.setBounds(0, 0, 0, 0);  // Hide old label
    
    bounds.removeFromTop(6);
    
    // Differences table - take remaining space minus buttons
    auto diffTable = bounds.removeFromTop(bounds.getHeight() - 44);
    differencesViewport.setBounds(diffTable);
    differencesContent.setBounds(0, 0, diffTable.getWidth() - 16, 
                                  static_cast<int>(differences.size() * 22 + 24));
    
    bounds.removeFromTop(8);
    
    // Action buttons - compact row
    auto buttonArea = bounds.removeFromTop(32);
    int btnW = (buttonArea.getWidth() - 20) / 5;
    
    storeAButton.setBounds(buttonArea.removeFromLeft(btnW));
    buttonArea.removeFromLeft(4);
    storeBButton.setBounds(buttonArea.removeFromLeft(btnW));
    buttonArea.removeFromLeft(4);
    recallAButton.setBounds(buttonArea.removeFromLeft(btnW));
    buttonArea.removeFromLeft(4);
    recallBButton.setBounds(buttonArea.removeFromLeft(btnW));
    buttonArea.removeFromLeft(4);
    revertButton.setBounds(buttonArea);
}

void ComparePanel::setupUI()
{
    // Title - modern minimal style
    addAndMakeVisible(titleLabel);
    titleLabel.setText("A/B COMPARE", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF94A3B8));
    
    // Close button - minimal X
    addAndMakeVisible(closeButton);
    closeButton.setButtonText("Close");
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF374151));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFE2E8F0));
    closeButton.addListener(this);
    
    setupSlotPanels();
    setupDifferencesTable();
    setupFilterControls();
    setupActionButtons();
}

void ComparePanel::setupSlotPanels()
{
    // Slot A - cyan accent
    addAndMakeVisible(slotAGroup);
    slotAGroup.setText("A");
    slotAGroup.setTextLabelPosition(juce::Justification::centredTop);
    slotAGroup.setColour(juce::GroupComponent::outlineColourId, juce::Colour(0xFF22D3EE));
    slotAGroup.setColour(juce::GroupComponent::textColourId, juce::Colour(0xFF22D3EE));
    
    addAndMakeVisible(slotANameLabel);
    slotANameLabel.setText("(Empty)", juce::dontSendNotification);
    slotANameLabel.setFont(juce::Font(11.0f));
    slotANameLabel.setJustificationType(juce::Justification::centred);
    slotANameLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFE2E8F0));
    
    addAndMakeVisible(slotAActiveButton);
    slotAActiveButton.setButtonText("ACTIVE");
    slotAActiveButton.setToggleable(true);
    slotAActiveButton.setClickingTogglesState(true);
    slotAActiveButton.setToggleState(true, juce::dontSendNotification);
    slotAActiveButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1E293B));
    slotAActiveButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF22D3EE));
    slotAActiveButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF94A3B8));
    slotAActiveButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF0F172A));
    slotAActiveButton.addListener(this);
    
    // Slot B - orange accent
    addAndMakeVisible(slotBGroup);
    slotBGroup.setText("B");
    slotBGroup.setTextLabelPosition(juce::Justification::centredTop);
    slotBGroup.setColour(juce::GroupComponent::outlineColourId, juce::Colour(0xFFFB923C));
    slotBGroup.setColour(juce::GroupComponent::textColourId, juce::Colour(0xFFFB923C));
    
    addAndMakeVisible(slotBNameLabel);
    slotBNameLabel.setText("(Empty)", juce::dontSendNotification);
    slotBNameLabel.setFont(juce::Font(11.0f));
    slotBNameLabel.setJustificationType(juce::Justification::centred);
    slotBNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFE2E8F0));
    
    addAndMakeVisible(slotBActiveButton);
    slotBActiveButton.setButtonText("SELECT");
    slotBActiveButton.setToggleable(true);
    slotBActiveButton.setClickingTogglesState(true);
    slotBActiveButton.setToggleState(false, juce::dontSendNotification);
    slotBActiveButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1E293B));
    slotBActiveButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFFB923C));
    slotBActiveButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF94A3B8));
    slotBActiveButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF0F172A));
    slotBActiveButton.addListener(this);
    
    // Swap button - minimal arrow style
    addAndMakeVisible(swapButton);
    swapButton.setButtonText("<>");
    swapButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF374151));
    swapButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFE2E8F0));
    swapButton.setTooltip("Swap A and B");
    swapButton.addListener(this);
}

void ComparePanel::setupDifferencesTable()
{
    addAndMakeVisible(differencesLabel);
    differencesLabel.setText("DIFFERENCES (showing 0 of 0 parameters):", juce::dontSendNotification);
    differencesLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    differencesLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF94A3B8));
    
    addAndMakeVisible(differencesViewport);
    differencesViewport.setViewedComponent(&differencesContent, false);
    differencesViewport.setScrollBarsShown(true, false);
    differencesViewport.setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xFF475569));
}

void ComparePanel::setupFilterControls()
{
    addAndMakeVisible(filterLabel);
    filterLabel.setText("", juce::dontSendNotification);  // Hidden
    
    addAndMakeVisible(showAllToggle);
    showAllToggle.setButtonText("All");
    showAllToggle.setToggleState(true, juce::dontSendNotification);
    showAllToggle.setRadioGroupId(1001);
    showAllToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xFFE2E8F0));
    showAllToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFF6366F1));
    showAllToggle.onClick = [this]() { 
        showChangedOnly = false; 
        refreshDifferences(); 
    };
    
    addAndMakeVisible(showChangedOnlyToggle);
    showChangedOnlyToggle.setButtonText("Changed");
    showChangedOnlyToggle.setToggleState(false, juce::dontSendNotification);
    showChangedOnlyToggle.setRadioGroupId(1001);
    showChangedOnlyToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xFFE2E8F0));
    showChangedOnlyToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFF6366F1));
    showChangedOnlyToggle.onClick = [this]() { 
        showChangedOnly = true; 
        refreshDifferences(); 
    };
    
    addAndMakeVisible(sectionFilterCombo);
    sectionFilterCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1E293B));
    sectionFilterCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xFFE2E8F0));
    sectionFilterCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF475569));
    sectionFilterCombo.addItem("All Sections", 1);
    sectionFilterCombo.addItem("DCO", 2);
    sectionFilterCombo.addItem("Mixer", 3);
    sectionFilterCombo.addItem("VCF", 4);
    sectionFilterCombo.addItem("VCA", 5);
    sectionFilterCombo.addItem("ENV", 6);
    sectionFilterCombo.addItem("LFO", 7);
    sectionFilterCombo.addItem("Patch", 8);
    sectionFilterCombo.addItem("Arp", 9);
    sectionFilterCombo.setSelectedId(1, juce::dontSendNotification);
    sectionFilterCombo.addListener(this);
}

void ComparePanel::setupActionButtons()
{
    auto styleBtn = [](juce::TextButton& btn, juce::Colour accent) {
        btn.setColour(juce::TextButton::buttonColourId, accent.withAlpha(0.2f));
        btn.setColour(juce::TextButton::buttonOnColourId, accent);
        btn.setColour(juce::TextButton::textColourOffId, accent);
        btn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF0F172A));
    };
    
    addAndMakeVisible(storeAButton);
    storeAButton.setButtonText("Save A");
    styleBtn(storeAButton, juce::Colour(0xFF22D3EE));
    storeAButton.addListener(this);
    
    addAndMakeVisible(storeBButton);
    storeBButton.setButtonText("Save B");
    styleBtn(storeBButton, juce::Colour(0xFFFB923C));
    storeBButton.addListener(this);
    
    addAndMakeVisible(recallAButton);
    recallAButton.setButtonText("Load A");
    styleBtn(recallAButton, juce::Colour(0xFF22D3EE));
    recallAButton.addListener(this);
    
    addAndMakeVisible(recallBButton);
    recallBButton.setButtonText("Load B");
    styleBtn(recallBButton, juce::Colour(0xFFFB923C));
    recallBButton.addListener(this);
    
    addAndMakeVisible(revertButton);
    revertButton.setButtonText("Revert");
    styleBtn(revertButton, juce::Colour(0xFFEF4444));
    revertButton.addListener(this);
}

void ComparePanel::buttonClicked(juce::Button* button)
{
    if (button == &closeButton)
    {
        setVisible(false);
    }
    else if (button == &slotAActiveButton)
    {
        setActiveSlot(CompareManager::Slot::SLOT_A);
        recallSlotA();
    }
    else if (button == &slotBActiveButton)
    {
        setActiveSlot(CompareManager::Slot::SLOT_B);
        recallSlotB();
    }
    else if (button == &swapButton)
    {
        swapSlots();
    }
    else if (button == &storeAButton)
    {
        storeToSlotA();
    }
    else if (button == &storeBButton)
    {
        storeToSlotB();
    }
    else if (button == &recallAButton)
    {
        recallSlotA();
    }
    else if (button == &recallBButton)
    {
        recallSlotB();
    }
    else if (button == &revertButton)
    {
        revertToOriginal();
    }
}

void ComparePanel::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &sectionFilterCombo)
    {
        switch (sectionFilterCombo.getSelectedId())
        {
            case 1: filterSection = "All"; break;
            case 2: filterSection = "DCO"; break;
            case 3: filterSection = "Mixer"; break;
            case 4: filterSection = "VCF"; break;
            case 5: filterSection = "VCA"; break;
            case 6: filterSection = "ENV"; break;
            case 7: filterSection = "LFO"; break;
            case 8: filterSection = "Patch"; break;
            case 9: filterSection = "Arp"; break;
            default: filterSection = "All"; break;
        }
        refreshDifferences();
    }
}

bool ComparePanel::keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    if (key.getKeyCode() == 'A' || key.getKeyCode() == 'a')
    {
        setActiveSlot(CompareManager::Slot::SLOT_A);
        recallSlotA();
        return true;
    }
    else if (key.getKeyCode() == 'B' || key.getKeyCode() == 'b')
    {
        setActiveSlot(CompareManager::Slot::SLOT_B);
        recallSlotB();
        return true;
    }
    else if (key.getKeyCode() == juce::KeyPress::spaceKey)
    {
        swapSlots();
        return true;
    }
    else if (key.getKeyCode() == juce::KeyPress::escapeKey)
    {
        setVisible(false);
        return true;
    }
    
    return false;
}

void ComparePanel::storeToSlotA()
{
    if (processor != nullptr)
    {
        processor->storeCurrentToSlot(CompareManager::Slot::SLOT_A);
        updateSlotNames();
        refreshDifferences();
        DBG("ComparePanel: Stored to Slot A");
    }
}

void ComparePanel::storeToSlotB()
{
    if (processor != nullptr)
    {
        processor->storeCurrentToSlot(CompareManager::Slot::SLOT_B);
        updateSlotNames();
        refreshDifferences();
        DBG("ComparePanel: Stored to Slot B");
    }
}

void ComparePanel::recallSlotA()
{
    if (processor != nullptr)
    {
        processor->loadSlotToCurrent(CompareManager::Slot::SLOT_A);
        processor->compareManager.setActiveSlot(CompareManager::Slot::SLOT_A);
        setActiveSlot(CompareManager::Slot::SLOT_A);
        
        // Update editor's indicator
        if (auto* editor = dynamic_cast<MKS70EditorAudioProcessorEditor*>(processor->getActiveEditor()))
            editor->updateCompareIndicator();
            
        DBG("ComparePanel: Recalled Slot A");
    }
}

void ComparePanel::recallSlotB()
{
    if (processor != nullptr)
    {
        processor->loadSlotToCurrent(CompareManager::Slot::SLOT_B);
        processor->compareManager.setActiveSlot(CompareManager::Slot::SLOT_B);
        setActiveSlot(CompareManager::Slot::SLOT_B);
        
        // Update editor's indicator
        if (auto* editor = dynamic_cast<MKS70EditorAudioProcessorEditor*>(processor->getActiveEditor()))
            editor->updateCompareIndicator();
            
        DBG("ComparePanel: Recalled Slot B");
    }
}

void ComparePanel::swapSlots()
{
    if (processor != nullptr)
    {
        processor->compareManager.swapAB();
        updateSlotNames();
        refreshDifferences();
        DBG("ComparePanel: Swapped A/B slots");
    }
}

void ComparePanel::revertToOriginal()
{
    if (processor != nullptr)
    {
        processor->loadSlotToCurrent(CompareManager::Slot::SLOT_ORIGINAL);
        refreshDifferences();
        DBG("ComparePanel: Reverted to original");
    }
}

void ComparePanel::setActiveSlot(CompareManager::Slot slot)
{
    activeSlot = slot;
    
    slotAActiveButton.setToggleState(slot == CompareManager::Slot::SLOT_A, juce::dontSendNotification);
    slotBActiveButton.setToggleState(slot == CompareManager::Slot::SLOT_B, juce::dontSendNotification);
    
    slotAActiveButton.setButtonText(slot == CompareManager::Slot::SLOT_A ? "ACTIVE" : "SELECT");
    slotBActiveButton.setButtonText(slot == CompareManager::Slot::SLOT_B ? "ACTIVE" : "SELECT");
}

void ComparePanel::refreshDifferences()
{
    if (processor != nullptr)
    {
        differences = processor->compareAB();
        
        // Update differences count in label
        auto filteredDiffs = getFilteredDifferences();
        differencesLabel.setText(
            "DIFFERENCES (showing " + juce::String(filteredDiffs.size()) + 
            " of " + juce::String(differences.size()) + " parameters):",
            juce::dontSendNotification
        );
        
        repaint();
    }
}

void ComparePanel::updateSlotNames()
{
    if (processor != nullptr)
    {
        auto slotA = processor->compareManager.getSlot(CompareManager::Slot::SLOT_A);
        auto slotB = processor->compareManager.getSlot(CompareManager::Slot::SLOT_B);
        
        slotANameLabel.setText(
            slotA.name.isEmpty() ? "(Empty)" : slotA.name,
            juce::dontSendNotification
        );
        
        slotBNameLabel.setText(
            slotB.name.isEmpty() ? "(Empty)" : slotB.name,
            juce::dontSendNotification
        );
    }
}

void ComparePanel::drawDifferencesTable(juce::Graphics& g)
{
    auto filteredDiffs = getFilteredDifferences();
    
    // Note: Table is drawn in the viewport's content component
    // This method is called from paint() but we can't draw here
    // Need to refactor to draw in a custom component
}

std::vector<ParameterDiff> ComparePanel::getFilteredDifferences()
{
    std::vector<ParameterDiff> filtered;
    
    for (const auto& diff : differences)
    {
        // Filter by changed only
        if (showChangedOnly && diff.difference == 0)
            continue;
        
        // Filter by section
        if (filterSection != "All")
        {
            if (!diff.section.contains(filterSection))
                continue;
        }
        
        filtered.push_back(diff);
    }
    
    return filtered;
}

juce::Colour ComparePanel::getDifferenceColour(int diffValue)
{
    if (diffValue == 0)
        return juce::Colour(0xff808080);  // Grey for no change
    else if (diffValue > 0)
        return juce::Colour(0xff00ff00);  // Green for positive
    else
        return juce::Colour(0xffff4444);  // Red for negative
}
