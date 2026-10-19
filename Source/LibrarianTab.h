/*
  ==============================================================================
    LibrarianTab.h - Preset Library Manager for MKS-70 Editor
    
    Redesigned with:
    - Grid/card layout for presets
    - Real AI-powered preset analysis (parses actual SysEx data)
    - AI Assistant for sound design guidance
    - Better organization by category
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "ToneParams.h"
#include "PatchParameters.h"
#include "GlowingSection.h"
#include "AIAssistant.h"

//==============================================================================
// Simple preset information structure
struct PresetInfo
{
    juce::String name;
    juce::String category;
    juce::String filePath;
    bool isFavorite = false;
};

//==============================================================================
// Preset Card Component for grid display
class PresetCard : public juce::Component
{
public:
    PresetCard(const PresetInfo& info) : presetInfo(info) {}
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        if (isSelected)
        {
            g.setColour(DS::Color::Accent::primary().withAlpha(0.3f));
            g.fillRoundedRectangle(bounds, 6.0f);
            g.setColour(DS::Color::Accent::primary());
            g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 2.0f);
        }
        else if (isMouseOver())
        {
            g.setColour(DS::Color::bgHover());
            g.fillRoundedRectangle(bounds, 6.0f);
            g.setColour(DS::Color::border());
            g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
        }
        else
        {
            g.setColour(DS::Color::bgCard());
            g.fillRoundedRectangle(bounds, 6.0f);
            g.setColour(DS::Color::border().withAlpha(0.5f));
            g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
        }
        
        auto inner = bounds.reduced(8);
        
        // Category color bar at top
        auto catBar = inner.removeFromTop(4);
        g.setColour(getCategoryColor(presetInfo.category));
        g.fillRoundedRectangle(catBar, 2.0f);
        
        inner.removeFromTop(6);
        
        // Preset name
        g.setColour(DS::Color::textPrimary());
        g.setFont(DS::Font::normal());
        g.drawText(presetInfo.name, inner.removeFromTop(20), juce::Justification::centredLeft);
        
        // Category label
        g.setColour(DS::Color::textSecondary());
        g.setFont(DS::Font::small());
        g.drawText(presetInfo.category, inner.removeFromTop(16), juce::Justification::centredLeft);
    }
    
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
    void mouseDown(const juce::MouseEvent&) override { if (onClick) onClick(this); }
    void mouseDoubleClick(const juce::MouseEvent&) override { if (onDoubleClick) onDoubleClick(this); }
    
    void setSelected(bool sel) { isSelected = sel; repaint(); }
    bool getSelected() const { return isSelected; }
    const PresetInfo& getPresetInfo() const { return presetInfo; }
    
    std::function<void(PresetCard*)> onClick;
    std::function<void(PresetCard*)> onDoubleClick;
    
private:
    PresetInfo presetInfo;
    bool isSelected = false;
    
    juce::Colour getCategoryColor(const juce::String& cat) const
    {
        if (cat == "Bass")    return DS::Color::Accent::magenta();
        if (cat == "Pad")     return DS::Color::Accent::cyan();
        if (cat == "Lead")    return DS::Color::Section::env1();
        if (cat == "Keys")    return DS::Color::Section::vca();
        if (cat == "FX")      return DS::Color::Accent::error();
        if (cat == "Strings") return DS::Color::Section::lfo();
        if (cat == "Brass")   return DS::Color::Section::dco1();
        if (cat == "Synth")   return DS::Color::Accent::primary();
        return DS::Color::textSecondary();
    }
};

//==============================================================================
// AI Explanation Panel
class PresetExplanationPanel : public juce::Component
{
public:
    PresetExplanationPanel()
    {
        titleLabel.setText("Preset Analysis", juce::dontSendNotification);
        titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        titleLabel.setColour(juce::Label::textColourId, DS::Color::Accent::cyan());
        addAndMakeVisible(titleLabel);
        
        presetNameLabel.setText("Select a preset", juce::dontSendNotification);
        presetNameLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        presetNameLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        addAndMakeVisible(presetNameLabel);
        
        explanationText.setMultiLine(true);
        explanationText.setReadOnly(true);
        explanationText.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        explanationText.setColour(juce::TextEditor::outlineColourId, DS::Color::border());
        explanationText.setColour(juce::TextEditor::textColourId, DS::Color::textPrimary());
        explanationText.setFont(DS::Font::small());
        explanationText.setText("Click on a preset to see its analysis.\n\n"
                               "The analyzer will parse actual SysEx data and show:\n"
                               "- Real parameter values from the preset\n"
                               "- Sound character based on actual settings\n"
                               "- How oscillators, filter, and envelopes are configured\n\n"
                               "Use 'AI Assistant' for interactive help!");
        addAndMakeVisible(explanationText);
        
        analyzeBtn.setButtonText("Analyze Parameters");
        analyzeBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::primary().withAlpha(0.3f));
        analyzeBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::primary());
        analyzeBtn.setEnabled(false);
        analyzeBtn.onClick = [this]() { analyzePreset(); };
        addAndMakeVisible(analyzeBtn);
        
        // AI Assistant button
        aiAssistantBtn.setButtonText("AI Assistant");
        aiAssistantBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::magenta().withAlpha(0.3f));
        aiAssistantBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::magenta());
        aiAssistantBtn.setTooltip("Open AI Assistant for sound design help");
        aiAssistantBtn.onClick = [this]() { 
            if (onOpenAIAssistant) 
                onOpenAIAssistant(currentSysexData, currentPreset.name); 
        };
        addAndMakeVisible(aiAssistantBtn);
        
        // Parameter summary section
        paramSummaryLabel.setText("Key Parameters:", juce::dontSendNotification);
        paramSummaryLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        paramSummaryLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        addAndMakeVisible(paramSummaryLabel);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(DS::Color::bgCard());
        g.fillRoundedRectangle(bounds, 8.0f);
        g.setColour(DS::Color::Accent::cyan().withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(12);
        
        // INCREASED heights for better visibility
        titleLabel.setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(10);
        presetNameLabel.setBounds(bounds.removeFromTop(26));
        bounds.removeFromTop(10);
        
        auto btnRow = bounds.removeFromTop(34);
        analyzeBtn.setBounds(btnRow.removeFromLeft(btnRow.getWidth() / 2 - 4));
        btnRow.removeFromLeft(8);
        aiAssistantBtn.setBounds(btnRow);
        
        bounds.removeFromTop(14);
        paramSummaryLabel.setBounds(bounds.removeFromTop(24));
        bounds.removeFromTop(6);
        explanationText.setBounds(bounds);
    }
    
    void setPreset(const PresetInfo& info, const juce::MemoryBlock& sysexData)
    {
        currentPreset = info;
        currentSysexData = sysexData;
        
        presetNameLabel.setText(info.name + " (" + info.category + ")", juce::dontSendNotification);
        analyzeBtn.setEnabled(true);
        
        // Parse and show real data immediately
        if (sysexData.getSize() > 10)
        {
            auto parsed = SysExParser::parseSysEx(sysexData);
            juce::String basicInfo;
            basicInfo << "File: " << juce::File(info.filePath).getFileName() << "\n";
            basicInfo << "Size: " << juce::String(sysexData.getSize()) << " bytes\n\n";
            basicInfo << "Quick Preview:\n";
            basicInfo << "  DCO1: " << SysExParser::describeWaveform(parsed.upperTone.dco1Waveform) << "\n";
            basicInfo << "  Filter Cutoff: " << juce::String(parsed.upperTone.vcfCutoff) << "\n";
            basicInfo << "  Chorus: " << SysExParser::describeChorus(parsed.upperTone.chorus) << "\n\n";
            basicInfo << "Click 'Analyze Parameters' for full analysis.";
            
            explanationText.setText(basicInfo);
        }
        else
        {
            juce::String basicInfo;
            basicInfo << "Category: " << info.category << "\n";
            basicInfo << "File: " << juce::File(info.filePath).getFileName() << "\n\n";
            basicInfo << "Click 'Analyze Parameters' for detailed analysis.";
            explanationText.setText(basicInfo);
        }
    }
    
    void clearPreset()
    {
        presetNameLabel.setText("Select a preset", juce::dontSendNotification);
        analyzeBtn.setEnabled(false);
        explanationText.setText("Click on a preset to see its analysis.");
    }
    
    // Callback to open AI Assistant
    std::function<void(const juce::MemoryBlock&, const juce::String&)> onOpenAIAssistant;
    
private:
    juce::Label titleLabel;
    juce::Label presetNameLabel;
    juce::TextEditor explanationText;
    juce::TextButton analyzeBtn;
    juce::TextButton aiAssistantBtn;
    juce::Label paramSummaryLabel;
    
    PresetInfo currentPreset;
    juce::MemoryBlock currentSysexData;
    
    void analyzePreset()
    {
        if (currentSysexData.getSize() < 10)
        {
            explanationText.setText("Unable to analyze - SysEx data not loaded.\nTry loading the preset first.");
            return;
        }
        
        // Use the real SysEx parser
        auto parsed = SysExParser::parseSysEx(currentSysexData);
        juce::String analysis = SysExParser::generateAnalysis(parsed);
        
        // Add extra context based on category
        analysis << "\n\n=== CATEGORY CONTEXT ===\n";
        if (currentPreset.category == "Pad")
            analysis << "As a PAD preset, expect slow attacks and long releases.\n";
        else if (currentPreset.category == "Bass")
            analysis << "As a BASS preset, expect emphasized low frequencies.\n";
        else if (currentPreset.category == "Lead")
            analysis << "As a LEAD preset, expect clear attack and presence.\n";
            
        analysis << "\nFor more help, open the AI Assistant!";
        
        explanationText.setText(analysis);
    }
};

//==============================================================================
class LibrarianTab : public juce::Component,
                     public juce::ListBoxModel
{
public:
    LibrarianTab();
    ~LibrarianTab() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ListBoxModel interface
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;
    
    // Public API for HeaderBar navigation
    int getPresetCount() const;
    juce::String getPresetName(int index) const;
    std::vector<juce::String> getPresetNames() const;
    void selectPresetByIndex(int index);
    int getSelectedIndex() const;
    
    // Callbacks for editor integration
    std::function<void(const ToneParams& upper, const ToneParams& lower, const PatchParameters& patch)> onSendPatchToSynth;
    std::function<void(const juce::MemoryBlock& sysexData)> onSendSysExToSynth;
    std::function<void()> onRequestPatchFromSynth;
    std::function<void()> onRequestToneUpperFromSynth;
    std::function<void()> onRequestToneLowerFromSynth;
    std::function<ToneParams()> onGetCurrentToneUpper;
    std::function<ToneParams()> onGetCurrentToneLower;
    std::function<PatchParameters()> onGetCurrentPatch;
    std::function<void(const ToneParams& tone, bool isUpper)> onLoadToneToEditor;
    std::function<void(const PatchParameters& patch)> onLoadPatchToEditor;
    std::function<void(int index, const juce::String& name)> onPresetLoaded; // Notify when preset loaded

private:
    // UI Components - Left side (presets)
    std::unique_ptr<GlowingSection> presetsSection;
    juce::ListBox presetList;
    juce::Viewport presetGridViewport;
    juce::Component presetGridContent;
    juce::OwnedArray<PresetCard> presetCards;
    bool useGridView = true;  // Toggle between list and grid
    
    juce::TextButton loadBtn, saveBtn, sendBtn, requestBtn;
    juce::TextButton initBtn, gridToggleBtn;
    juce::ComboBox categoryCombo;
    juce::TextEditor searchBox;
    juce::Label statusLabel;
    juce::Label presetCountLabel;
    
    // UI Components - Right side (explanation)
    std::unique_ptr<GlowingSection> explanationSection;
    PresetExplanationPanel explanationPanel;
    
    // Data
    std::vector<PresetInfo> allPresets;
    std::vector<PresetInfo> filteredPresets;
    int selectedPresetIndex = -1;
    juce::String currentCategory;
    juce::MemoryBlock loadedSysexData;  // Loaded SysEx data to send to synth
    juce::String currentPresetName;     // Name of currently loaded preset
    PresetCard* selectedCard = nullptr;
    
    // Helper methods
    void setupUI();
    void loadPresetsFromFolder();
    juce::File getPresetsFolder() const;
    void filterPresets();
    void selectPreset(int index);
    void loadSelectedPreset();
    void saveCurrentPreset();
    void sendToSynth();
    void requestFromSynth();
    void initPreset();
    void rebuildPresetCards();
    void layoutPresetGrid();
    void selectCard(PresetCard* card);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibrarianTab)
};
