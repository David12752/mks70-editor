/*
  ==============================================================================

    RandomPanel.h
    Random Generator UI Panel with categories, controls, and history

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RandomGenerator.h"
#include "PluginProcessor.h"

//==============================================================================
/**
 * RandomPanel - UI for smart random patch generation
 */
class RandomPanel : public juce::Component,
                    public juce::Button::Listener,
                    public juce::Slider::Listener,
                    public juce::ComboBox::Listener
{
public:
    RandomPanel();
    ~RandomPanel() override;
    
    void setProcessor(MKS70EditorAudioProcessor* proc);
    
    // Close callback
    std::function<void()> onClose;
  // Patch-applied callback (to refresh UI)
  std::function<void()> onPatchApplied;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Component listeners
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;

private:
    MKS70EditorAudioProcessor* processor = nullptr;
    RandomGenerator randomGenerator;
    
    // History and favorites
    struct HistoryEntry
    {
        FullPatch patch;
        juce::String categoryName;
        juce::Time timestamp;
        bool isFavorite = false;
    };
    std::vector<HistoryEntry> history;
    int currentHistoryIndex = -1;
    static constexpr int MAX_HISTORY = 20;
    
    // UI Components - Category Buttons
    juce::GroupComponent categoryGroup;
    juce::Label categoryLabel;
    juce::TextButton categoryButtons[10];
    RandomGenerator::SoundCategory selectedCategory = RandomGenerator::CAT_BASS;
    
    // UI Components - Controls
    juce::GroupComponent controlsGroup;
    juce::Slider randomnessSlider;
    juce::Label randomnessLabel;
    juce::Label randomnessValueLabel;
    
    juce::Slider complexitySlider;
    juce::Label complexityLabel;
    juce::Label complexityValueLabel;
    
    juce::ToggleButton musicalConstraintsToggle;
    juce::ToggleButton keepPatchSettingsToggle;
    juce::ToggleButton randomizeArpToggle;
    
    // UI Components - Locked Parameters
    juce::GroupComponent lockedParamsGroup;
    juce::Label lockedParamsLabel;
    juce::TextButton addLockButton;
    juce::Viewport lockedParamsViewport;
    juce::Component lockedParamsContainer;
    std::vector<std::unique_ptr<juce::TextButton>> lockedParamButtons;
    
    // UI Components - Variation
    juce::GroupComponent variationGroup;
    juce::Label variationLabel;
    juce::TextButton variationButtons[4];
    int selectedVariation = 1;  // 0=10%, 1=30%, 2=70%, 3=100%
    
    // UI Components - Actions
    juce::TextButton generateButton;
    juce::TextButton undoButton;
    juce::TextButton favoriteButton;
    juce::TextButton closeButton;
    
    // UI Components - History
    juce::Label historyLabel;
    juce::TextButton historyPrevButton;
    juce::TextButton historyNextButton;
    juce::Label historyInfoLabel;
    
    // Setup methods
    void setupCategoryButtons();
    void setupControls();
    void setupLockedParams();
    void setupVariation();
    void setupActions();
    void setupHistory();
    
    // Category button handlers
    void selectCategory(RandomGenerator::SoundCategory category);
    void updateCategoryButtons();
    
    // Generation methods
    void generatePatch();
    void generateVariation();
    void applyPatchToProcessor(const FullPatch& patch);
    
    // History methods
    void addToHistory(const FullPatch& patch, const juce::String& categoryName);
    void navigateHistory(int direction);  // -1 = prev, +1 = next
    void updateHistoryUI();
    void toggleFavorite();
    
    // Locked parameters
    void showAddLockDialog();
    void addLockedParam(const juce::String& paramName, int value);
    void removeLockedParam(const juce::String& paramName);
    void refreshLockedParamsUI();
    
    // Utility
    juce::String getCategoryButtonText(RandomGenerator::SoundCategory category);
    float getVariationAmount(int index);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RandomPanel)
};
