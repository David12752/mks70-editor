/*
  ==============================================================================

    LibrarianPanel.h
    MKS-70 Patch Librarian - Bank & Patch Management
    
    Features:
    - Bank list (Internal A/B, Cartridge, User banks)
    - Patch list with search and category filtering
    - Load/Save patch files (.syx, .mid)
    - Request/Send banks via SysEx
    - Drag & drop patch organization

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "GlowingSection.h"

//==============================================================================
/**
 * Single patch item in the list
 */
class PatchListItem : public juce::Component
{
public:
    PatchListItem(int index, const juce::String& name, const juce::String& category)
        : patchIndex(index), patchName(name), patchCategory(category) 
    {
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Hover/selected state
        if (isSelected)
        {
            g.setColour(DS::Color::Accent::primary().withAlpha(0.2f));
            g.fillRoundedRectangle(bounds, (float)DS::Radius::S);
            
            g.setColour(DS::Color::Accent::primary());
            g.drawRoundedRectangle(bounds.reduced(0.5f), (float)DS::Radius::S, 1.0f);
        }
        else if (isMouseOver())
        {
            g.setColour(DS::Color::bgHover());
            g.fillRoundedRectangle(bounds, (float)DS::Radius::S);
        }
        
        // Index number
        g.setColour(DS::Color::textMuted());
        g.setFont(DS::Font::mono());
        auto indexBounds = bounds.removeFromLeft(40);
        g.drawText(juce::String(patchIndex).paddedLeft('0', 3), 
                   indexBounds.toNearestInt(), juce::Justification::centred);
        
        bounds.removeFromLeft((float)DS::Space::S);
        
        // Name
        g.setColour(DS::Color::textPrimary());
        g.setFont(DS::Font::normal());
        auto nameBounds = bounds.removeFromLeft(200);
        g.drawText(patchName, nameBounds.toNearestInt(), juce::Justification::centredLeft);
        
        // Category tag
        if (patchCategory.isNotEmpty())
        {
            auto catBounds = bounds.removeFromRight(80).reduced(4, 6);
            
            // Get color based on category
            juce::Colour catColor = getCategoryColor(patchCategory);
            
            g.setColour(catColor.withAlpha(0.2f));
            g.fillRoundedRectangle(catBounds, (float)DS::Radius::S);
            g.setColour(catColor);
            g.setFont(DS::Font::tiny());
            g.drawText(patchCategory, catBounds.toNearestInt(), juce::Justification::centred);
        }
    }
    
    void setSelected(bool selected)
    {
        isSelected = selected;
        repaint();
    }
    
    bool getSelected() const { return isSelected; }
    int getIndex() const { return patchIndex; }
    juce::String getName() const { return patchName; }
    juce::String getCategory() const { return patchCategory; }
    
    void setName(const juce::String& name) { patchName = name; repaint(); }
    void setCategory(const juce::String& category) { patchCategory = category; repaint(); }
    
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
    
    void mouseDown(const juce::MouseEvent&) override
    {
        if (onSelected)
            onSelected(this);
    }
    
    void mouseDoubleClick(const juce::MouseEvent&) override
    {
        if (onDoubleClick)
            onDoubleClick(this);
    }
    
    std::function<void(PatchListItem*)> onSelected;
    std::function<void(PatchListItem*)> onDoubleClick;
    
private:
    int patchIndex;
    juce::String patchName;
    juce::String patchCategory;
    bool isSelected = false;
    
    juce::Colour getCategoryColor(const juce::String& cat) const
    {
        if (cat == "Bass")   return DS::Color::Accent::magenta();
        if (cat == "Pad")    return DS::Color::Accent::cyan();
        if (cat == "Lead")   return DS::Color::Section::env1();
        if (cat == "Keys")   return DS::Color::Section::vca();
        if (cat == "FX")     return DS::Color::Accent::error();
        if (cat == "Strings") return DS::Color::Section::lfo();
        if (cat == "Brass")  return DS::Color::Section::dco1();
        if (cat == "Synth")  return DS::Color::Accent::primary();
        return DS::Color::textSecondary();
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatchListItem)
};

//==============================================================================
/**
 * Bank list item model
 */
class BankListBoxModel : public juce::ListBoxModel
{
public:
    BankListBoxModel() 
    {
        bankNames = { 
            "Internal A", 
            "Internal B", 
            "Cartridge", 
            "User Bank 1", 
            "User Bank 2",
            "User Bank 3",
            "User Bank 4"
        };
        
        bankIcons = {
            juce::CharPointer_UTF8("\xf0\x9f\x8e\xb9"),  // 🎹
            juce::CharPointer_UTF8("\xf0\x9f\x8e\xb9"),  // 🎹
            juce::CharPointer_UTF8("\xf0\x9f\x92\xbe"),  // 💾
            juce::CharPointer_UTF8("\xf0\x9f\x93\x81"),  // 📁
            juce::CharPointer_UTF8("\xf0\x9f\x93\x81"),  // 📁
            juce::CharPointer_UTF8("\xf0\x9f\x93\x81"),  // 📁
            juce::CharPointer_UTF8("\xf0\x9f\x93\x81")   // 📁
        };
    }
    
    int getNumRows() override { return bankNames.size(); }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, 
                          int width, int height, bool rowIsSelected) override
    {
        if (rowNumber < 0 || rowNumber >= bankNames.size())
            return;
            
        auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
        
        if (rowIsSelected)
        {
            g.setColour(DS::Color::Accent::primary().withAlpha(0.3f));
            g.fillRoundedRectangle(bounds.reduced(2), (float)DS::Radius::S);
            
            g.setColour(DS::Color::Accent::primary().withAlpha(0.6f));
            g.drawRoundedRectangle(bounds.reduced(2), (float)DS::Radius::S, 1.0f);
        }
        else if (rowNumber == hoveredRow)
        {
            g.setColour(DS::Color::bgHover());
            g.fillRoundedRectangle(bounds.reduced(2), (float)DS::Radius::S);
        }
        
        // Icon
        g.setColour(rowIsSelected ? DS::Color::Accent::primary() : DS::Color::textSecondary());
        g.setFont(14.0f);
        g.drawText(bankIcons[rowNumber], 
                   juce::Rectangle<int>(DS::Space::S, 0, 24, height), 
                   juce::Justification::centred);
        
        // Name
        g.setColour(DS::Color::textPrimary());
        g.setFont(DS::Font::normal());
        g.drawText(bankNames[rowNumber], 
                   juce::Rectangle<int>(32, 0, width - 40, height), 
                   juce::Justification::centredLeft);
        
        // Patch count badge
        g.setColour(DS::Color::textMuted());
        g.setFont(DS::Font::tiny());
        g.drawText("64", 
                   juce::Rectangle<int>(width - 30, 0, 25, height), 
                   juce::Justification::centredRight);
    }
    
    void setHoveredRow(int row) { hoveredRow = row; }
    juce::String getBankName(int index) const { return bankNames[index]; }
    
    std::function<void(int)> onBankSelected;
    
private:
    juce::StringArray bankNames;
    juce::StringArray bankIcons;
    int hoveredRow = -1;
};

//==============================================================================
/**
 * Main Librarian Panel - Patch and Bank Management
 */
class LibrarianPanel : public juce::Component,
                       public juce::FileDragAndDropTarget
{
public:
    LibrarianPanel()
    {
        // === TITLE ===
        titleLabel.setText(juce::CharPointer_UTF8("\xf0\x9f\x93\x9a Patch Librarian"), 
                          juce::dontSendNotification);
        titleLabel.setFont(juce::FontOptions(22.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centredLeft);
        titleLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        addAndMakeVisible(titleLabel);
        
        // === BANK SECTION ===
        bankSection = std::make_unique<GlowingSection>("Banks", DS::Color::Accent::primary());
        addAndMakeVisible(bankSection.get());
        
        bankListBox.setModel(&bankListModel);
        bankListBox.setRowHeight(36);
        bankListBox.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
        bankListBox.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
        bankListModel.onBankSelected = [this](int bank) { loadBankPatches(bank); };
        addAndMakeVisible(bankListBox);
        
        // === PATCH SECTION ===
        patchSection = std::make_unique<GlowingSection>("Patches", DS::Color::Accent::cyan());
        addAndMakeVisible(patchSection.get());
        
        // Search box
        searchBox.setTextToShowWhenEmpty("Search patches...", DS::Color::textMuted());
        searchBox.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        searchBox.setColour(juce::TextEditor::outlineColourId, DS::Color::border());
        searchBox.setColour(juce::TextEditor::textColourId, DS::Color::textPrimary());
        searchBox.setColour(juce::TextEditor::focusedOutlineColourId, DS::Color::Accent::primary());
        searchBox.onTextChange = [this]() { filterPatches(); };
        addAndMakeVisible(searchBox);
        
        // Category filter
        categoryFilter.addItem("All Categories", 1);
        categoryFilter.addItem("Bass", 2);
        categoryFilter.addItem("Pad", 3);
        categoryFilter.addItem("Lead", 4);
        categoryFilter.addItem("Keys", 5);
        categoryFilter.addItem("Strings", 6);
        categoryFilter.addItem("Brass", 7);
        categoryFilter.addItem("Synth", 8);
        categoryFilter.addItem("FX", 9);
        categoryFilter.setSelectedId(1);
        categoryFilter.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
        categoryFilter.setColour(juce::ComboBox::outlineColourId, DS::Color::border());
        categoryFilter.setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
        categoryFilter.onChange = [this]() { filterPatches(); };
        addAndMakeVisible(categoryFilter);
        
        // Patch count label
        patchCountLabel.setFont(DS::Font::small());
        patchCountLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        addAndMakeVisible(patchCountLabel);
        
        // Patch list viewport
        patchListContent = std::make_unique<juce::Component>();
        patchViewport.setViewedComponent(patchListContent.get(), false);
        patchViewport.setScrollBarsShown(true, false);
        patchViewport.setColour(juce::ScrollBar::thumbColourId, DS::Color::Accent::primary().withAlpha(0.5f));
        addAndMakeVisible(patchViewport);
        
        // Create sample patches for demo
        createDemoPatches();
        
        // === PREVIEW SECTION ===
        previewSection = std::make_unique<GlowingSection>("Preview", DS::Color::Section::mixer());
        addAndMakeVisible(previewSection.get());
        
        selectedPatchLabel.setText("No patch selected", juce::dontSendNotification);
        selectedPatchLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        selectedPatchLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        addAndMakeVisible(selectedPatchLabel);
        
        patchInfoLabel.setText("Select a patch to see details", juce::dontSendNotification);
        patchInfoLabel.setFont(DS::Font::small());
        patchInfoLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        addAndMakeVisible(patchInfoLabel);
        
        // === ACTIONS SECTION ===
        actionsSection = std::make_unique<GlowingSection>("Actions", DS::Color::Section::env1());
        addAndMakeVisible(actionsSection.get());
        
        setupButton(requestBankBtn, juce::CharPointer_UTF8("\xe2\xac\x87 Request Bank"), 
                   DS::Color::Accent::cyan());
        setupButton(sendBankBtn, juce::CharPointer_UTF8("\xe2\xac\x86 Send Bank"), 
                   DS::Color::Accent::primary());
        setupButton(loadFileBtn, juce::CharPointer_UTF8("\xf0\x9f\x93\x82 Load File"), 
                   DS::Color::Section::vca());
        setupButton(saveFileBtn, juce::CharPointer_UTF8("\xf0\x9f\x92\xbe Save File"), 
                   DS::Color::Section::vca());
        setupButton(sendPatchBtn, juce::CharPointer_UTF8("\xf0\x9f\x8e\xb9 Send Patch"), 
                   DS::Color::Accent::magenta());
        setupButton(initPatchBtn, juce::CharPointer_UTF8("\xe2\x9c\xa8 Init Patch"), 
                   DS::Color::textSecondary());
        
        // Button callbacks
        requestBankBtn.onClick = [this]() { requestBank(); };
        sendBankBtn.onClick = [this]() { sendBank(); };
        loadFileBtn.onClick = [this]() { loadFile(); };
        saveFileBtn.onClick = [this]() { saveFile(); };
        sendPatchBtn.onClick = [this]() { sendSelectedPatch(); };
        initPatchBtn.onClick = [this]() { initPatch(); };
        
        // === DROP ZONE LABEL ===
        dropZoneLabel.setText("Drop .syx or .mid files here", juce::dontSendNotification);
        dropZoneLabel.setFont(DS::Font::small());
        dropZoneLabel.setColour(juce::Label::textColourId, DS::Color::textMuted());
        dropZoneLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(dropZoneLabel);
        
        updatePatchCount();
    }
    
    ~LibrarianPanel() override = default;
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bg());
        
        // Draw drop zone indicator when dragging
        if (isDraggingOver)
        {
            g.setColour(DS::Color::Accent::primary().withAlpha(0.1f));
            g.fillAll();
            
            g.setColour(DS::Color::Accent::primary().withAlpha(0.5f));
            g.drawRect(getLocalBounds().reduced(4), 2);
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        
        // Title
        auto titleArea = bounds.removeFromTop(50);
        titleLabel.setBounds(titleArea.reduced(DS::Space::L, DS::Space::M));
        
        bounds = bounds.reduced(DS::Space::L);
        
        // Three column layout
        const int bankWidth = 180;
        const int actionsWidth = 160;
        const int previewHeight = 120;
        const int gap = DS::Space::L;
        
        // Left: Banks
        auto bankCol = bounds.removeFromLeft(bankWidth);
        bounds.removeFromLeft(gap);
        
        // Right: Actions
        auto actionsCol = bounds.removeFromRight(actionsWidth);
        bounds.removeFromRight(gap);
        
        // Center: Patches
        auto patchCol = bounds;
        
        // === BANK SECTION ===
        bankSection->setBounds(bankCol);
        auto bankContent = bankCol.reduced(DS::Space::M);
        bankContent.removeFromTop(DS::Size::SectionHeaderHeight);
        bankListBox.setBounds(bankContent);
        
        // === PATCH SECTION ===
        // Bottom preview area
        auto previewCol = patchCol.removeFromBottom(previewHeight);
        patchCol.removeFromBottom(gap);
        
        patchSection->setBounds(patchCol);
        auto patchContent = patchCol.reduced(DS::Space::M);
        patchContent.removeFromTop(DS::Size::SectionHeaderHeight);
        
        // Search row
        auto searchRow = patchContent.removeFromTop(DS::Size::TextFieldHeight);
        searchBox.setBounds(searchRow.removeFromLeft((int)(searchRow.getWidth() * 0.5f)));
        searchRow.removeFromLeft(DS::Space::S);
        categoryFilter.setBounds(searchRow.removeFromLeft(120));
        searchRow.removeFromLeft(DS::Space::S);
        patchCountLabel.setBounds(searchRow);
        
        patchContent.removeFromTop(DS::Space::M);
        
        // Drop zone label at bottom
        dropZoneLabel.setBounds(patchContent.removeFromBottom(20));
        patchContent.removeFromBottom(DS::Space::XS);
        
        // Patch list viewport
        patchViewport.setBounds(patchContent);
        layoutPatchList();
        
        // === PREVIEW SECTION ===
        previewSection->setBounds(previewCol);
        auto previewContent = previewCol.reduced(DS::Space::M);
        previewContent.removeFromTop(DS::Size::SectionHeaderHeight);
        selectedPatchLabel.setBounds(previewContent.removeFromTop(24));
        patchInfoLabel.setBounds(previewContent);
        
        // === ACTIONS SECTION ===
        actionsSection->setBounds(actionsCol);
        auto actionContent = actionsCol.reduced(DS::Space::M);
        actionContent.removeFromTop(DS::Size::SectionHeaderHeight);
        
        const int btnHeight = DS::Size::ButtonHeight;
        const int btnGap = DS::Space::S;
        
        requestBankBtn.setBounds(actionContent.removeFromTop(btnHeight));
        actionContent.removeFromTop(btnGap);
        sendBankBtn.setBounds(actionContent.removeFromTop(btnHeight));
        actionContent.removeFromTop(DS::Space::L);
        
        loadFileBtn.setBounds(actionContent.removeFromTop(btnHeight));
        actionContent.removeFromTop(btnGap);
        saveFileBtn.setBounds(actionContent.removeFromTop(btnHeight));
        actionContent.removeFromTop(DS::Space::L);
        
        sendPatchBtn.setBounds(actionContent.removeFromTop(btnHeight));
        actionContent.removeFromTop(btnGap);
        initPatchBtn.setBounds(actionContent.removeFromTop(btnHeight));
    }
    
    // FileDragAndDropTarget interface
    bool isInterestedInFileDrag(const juce::StringArray& files) override
    {
        for (const auto& file : files)
        {
            if (file.endsWithIgnoreCase(".syx") || 
                file.endsWithIgnoreCase(".mid") ||
                file.endsWithIgnoreCase(".midi"))
                return true;
        }
        return false;
    }
    
    void fileDragEnter(const juce::StringArray&, int, int) override
    {
        isDraggingOver = true;
        repaint();
    }
    
    void fileDragExit(const juce::StringArray&) override
    {
        isDraggingOver = false;
        repaint();
    }
    
    void filesDropped(const juce::StringArray& files, int, int) override
    {
        isDraggingOver = false;
        repaint();
        
        for (const auto& filePath : files)
        {
            juce::File file(filePath);
            if (file.existsAsFile())
            {
                loadPatchFile(file);
            }
        }
    }
    
private:
    // UI Components
    juce::Label titleLabel;
    
    std::unique_ptr<GlowingSection> bankSection;
    BankListBoxModel bankListModel;
    juce::ListBox bankListBox { "Banks", nullptr };
    
    std::unique_ptr<GlowingSection> patchSection;
    juce::TextEditor searchBox;
    juce::ComboBox categoryFilter;
    juce::Label patchCountLabel;
    juce::Viewport patchViewport;
    std::unique_ptr<juce::Component> patchListContent;
    juce::OwnedArray<PatchListItem> patchItems;
    juce::OwnedArray<PatchListItem> filteredItems;
    
    std::unique_ptr<GlowingSection> previewSection;
    juce::Label selectedPatchLabel;
    juce::Label patchInfoLabel;
    
    std::unique_ptr<GlowingSection> actionsSection;
    juce::TextButton requestBankBtn, sendBankBtn;
    juce::TextButton loadFileBtn, saveFileBtn;
    juce::TextButton sendPatchBtn, initPatchBtn;
    
    juce::Label dropZoneLabel;
    
    bool isDraggingOver = false;
    PatchListItem* selectedPatch = nullptr;
    int currentBank = 0;
    
    void setupButton(juce::TextButton& btn, const juce::String& text, juce::Colour color)
    {
        btn.setButtonText(text);
        btn.setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        btn.setColour(juce::TextButton::buttonOnColourId, color.withAlpha(0.3f));
        btn.setColour(juce::TextButton::textColourOffId, DS::Color::textPrimary());
        btn.setColour(juce::TextButton::textColourOnId, color);
        addAndMakeVisible(btn);
    }
    
    void createDemoPatches()
    {
        // Sample patch data for demo
        struct DemoPatch { const char* name; const char* category; };
        DemoPatch demos[] = {
            {"Warm Pad", "Pad"}, {"Deep Bass", "Bass"}, {"Bright Lead", "Lead"},
            {"Soft Strings", "Strings"}, {"Analog Brass", "Brass"}, {"Synth Bell", "Synth"},
            {"PWM Pad", "Pad"}, {"Sub Bass", "Bass"}, {"Screaming Lead", "Lead"},
            {"Lush Strings", "Strings"}, {"Fat Brass", "Brass"}, {"Crystal", "Synth"},
            {"Atmosphere", "Pad"}, {"Punchy Bass", "Bass"}, {"Solo Lead", "Lead"},
            {"Orchestra", "Strings"}, {"Soft Brass", "Brass"}, {"Digital", "Synth"},
            {"Sweep Pad", "Pad"}, {"Acid Bass", "Bass"}, {"Portamento", "Lead"},
            {"Ensemble", "Strings"}, {"Power Brass", "Brass"}, {"Retro", "Synth"},
            {"Space Pad", "Pad"}, {"Wobble Bass", "Bass"}, {"Sync Lead", "Lead"},
            {"Violin", "Strings"}, {"Trombone", "Brass"}, {"Chiptune", "Synth"},
            {"Evolving", "Pad"}, {"Reese Bass", "Bass"}, {"Dist Lead", "Lead"},
            {"Cello", "Strings"}, {"Horn", "Brass"}, {"Arpeggio", "Synth"},
            {"Dreamy", "Pad"}, {"808 Bass", "Bass"}, {"Fizzy Lead", "Lead"},
            {"Chamber", "Strings"}, {"Fanfare", "Brass"}, {"Glitch", "FX"},
            {"Ambient", "Pad"}, {"Growl Bass", "Bass"}, {"Whistle", "Lead"},
            {"Pizzicato", "Strings"}, {"Muted Brass", "Brass"}, {"Noise", "FX"},
            {"Shimmer", "Pad"}, {"Pluck Bass", "Bass"}, {"Vibrato", "Lead"},
            {"Spiccato", "Strings"}, {"Stab Brass", "Brass"}, {"Riser", "FX"},
            {"Glass Pad", "Pad"}, {"Finger Bass", "Bass"}, {"Feedback", "Lead"},
            {"Tremolo", "Strings"}, {"Swell Brass", "Brass"}, {"Impact", "FX"},
            {"Choir Pad", "Pad"}, {"Slap Bass", "Bass"}, {"Wah Lead", "Lead"},
            {"Legato", "Strings"}, {"Tuba", "Brass"}, {"Sweep", "FX"}
        };
        
        for (int i = 0; i < 64; ++i)
        {
            auto* item = new PatchListItem(i + 1, demos[i].name, demos[i].category);
            item->onSelected = [this](PatchListItem* p) { selectPatch(p); };
            item->onDoubleClick = [this](PatchListItem* p) { sendPatch(p); };
            patchItems.add(item);
            patchListContent->addAndMakeVisible(item);
        }
        
        // Initially all patches are visible
        filterPatches();
    }
    
    void layoutPatchList()
    {
        const int itemHeight = 36;
        const int itemGap = 2;
        int contentWidth = patchViewport.getWidth() - 12;
        
        int y = 0;
        for (auto* item : filteredItems)
        {
            item->setBounds(0, y, contentWidth, itemHeight);
            item->setVisible(true);
            y += itemHeight + itemGap;
        }
        
        patchListContent->setBounds(0, 0, contentWidth, y);
    }
    
    void filterPatches()
    {
        filteredItems.clear();
        
        juce::String searchText = searchBox.getText().toLowerCase();
        int categoryId = categoryFilter.getSelectedId();
        juce::String categoryName = categoryId > 1 ? categoryFilter.getText() : "";
        
        for (auto* item : patchItems)
        {
            bool matchesSearch = searchText.isEmpty() || 
                                 item->getName().toLowerCase().contains(searchText);
            bool matchesCategory = categoryName.isEmpty() || 
                                   item->getCategory() == categoryName;
            
            if (matchesSearch && matchesCategory)
            {
                filteredItems.add(item);
            }
            else
            {
                item->setVisible(false);
            }
        }
        
        updatePatchCount();
        layoutPatchList();
    }
    
    void updatePatchCount()
    {
        patchCountLabel.setText(juce::String(filteredItems.size()) + " / " + 
                               juce::String(patchItems.size()) + " patches",
                               juce::dontSendNotification);
    }
    
    void selectPatch(PatchListItem* patch)
    {
        // Deselect previous
        if (selectedPatch)
            selectedPatch->setSelected(false);
        
        selectedPatch = patch;
        
        if (patch)
        {
            patch->setSelected(true);
            selectedPatchLabel.setText(juce::String(patch->getIndex()).paddedLeft('0', 3) + 
                                       " - " + patch->getName(), 
                                       juce::dontSendNotification);
            patchInfoLabel.setText("Category: " + patch->getCategory() + 
                                   "\nBank: " + bankListModel.getBankName(currentBank),
                                   juce::dontSendNotification);
        }
        else
        {
            selectedPatchLabel.setText("No patch selected", juce::dontSendNotification);
            patchInfoLabel.setText("Select a patch to see details", juce::dontSendNotification);
        }
    }
    
    void sendPatch(PatchListItem* patch)
    {
        if (patch)
        {
            DBG("Sending patch: " + patch->getName());
            // TODO: Implement actual SysEx send
        }
    }
    
    void loadBankPatches(int bankIndex)
    {
        currentBank = bankIndex;
        DBG("Loading bank: " + bankListModel.getBankName(bankIndex));
        // TODO: Load actual bank data
    }
    
    // Action handlers
    void requestBank()
    {
        int selectedBank = bankListBox.getSelectedRow();
        if (selectedBank >= 0)
        {
            DBG("Requesting bank: " + bankListModel.getBankName(selectedBank));
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                "Request Bank",
                "Requesting " + bankListModel.getBankName(selectedBank) + " from MKS-70...\n\n"
                "Make sure your MIDI connection is configured correctly."
            );
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "No Bank Selected",
                "Please select a bank from the list first."
            );
        }
    }
    
    void sendBank()
    {
        int selectedBank = bankListBox.getSelectedRow();
        if (selectedBank >= 0)
        {
            DBG("Sending bank: " + bankListModel.getBankName(selectedBank));
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Send Bank",
                "This will overwrite " + bankListModel.getBankName(selectedBank) + " on the MKS-70.\n\n"
                "Make sure PROTECT is OFF on the synthesizer."
            );
        }
    }
    
    void loadFile()
    {
        auto chooser = std::make_shared<juce::FileChooser>(
            "Load Patch File",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.syx;*.mid;*.midi"
        );
        
        chooser->launchAsync(juce::FileBrowserComponent::openMode | 
                            juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    loadPatchFile(file);
                }
            }
        );
    }
    
    void loadPatchFile(const juce::File& file)
    {
        DBG("Loading file: " + file.getFullPathName());
        
        // TODO: Parse SysEx/MIDI file and load patches
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "File Loaded",
            "Loaded: " + file.getFileName() + "\n\n"
            "Parsing patch data..."
        );
    }
    
    void saveFile()
    {
        auto chooser = std::make_shared<juce::FileChooser>(
            "Save Patch File",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.syx"
        );
        
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | 
                            juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file != juce::File())
                {
                    DBG("Saving to: " + file.getFullPathName());
                    // TODO: Save patches to SysEx file
                }
            }
        );
    }
    
    void sendSelectedPatch()
    {
        if (selectedPatch)
        {
            sendPatch(selectedPatch);
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "No Patch Selected",
                "Please select a patch from the list first."
            );
        }
    }
    
    void initPatch()
    {
        DBG("Initializing patch to default values");
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Initialize Patch",
            "Patch initialized to default values.\n\n"
            "Use 'Send Patch' to send to the MKS-70."
        );
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibrarianPanel)
};
