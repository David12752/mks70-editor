/*
  ==============================================================================
    MacroTab.h - Dedicated Macro Configuration Tab
    
    Full macro control with 8 assignable macros, each controlling multiple
    parameters with individual depth and curve settings.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "GlowingSection.h"
#include "StyledSlider.h"
#include "StyledComboBox.h"
#include "MacroControl.h"

//==============================================================================
/**
 * Single macro editor card - shows one macro with all its mappings
 */
class MacroEditorCard : public juce::Component
{
public:
    MacroEditorCard(int macroIndex) : index(macroIndex)
    {
        // Header with macro name
        nameEditor.setFont(DS::Font::medium());
        nameEditor.setJustification(juce::Justification::centred);
        nameEditor.setTextToShowWhenEmpty("Macro " + juce::String(macroIndex + 1), DS::Color::textMuted());
        nameEditor.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        nameEditor.setColour(juce::TextEditor::textColourId, DS::Color::Accent::primary());
        nameEditor.setColour(juce::TextEditor::outlineColourId, DS::Color::Accent::primary().withAlpha(0.3f));
        addAndMakeVisible(nameEditor);
        
        // Main value slider
        valueSlider = std::make_unique<StyledSlider>("Value", StyledSlider::Knob, 
            DS::Color::Accent::primary());
        valueSlider->setRange(0, 127);
        valueSlider->setValue(64);
        addAndMakeVisible(valueSlider.get());
        
        // MIDI Learn button
        midiLearnBtn.setButtonText("MIDI Learn");
        midiLearnBtn.setClickingTogglesState(true);
        styleButton(midiLearnBtn, DS::Color::Accent::cyan());
        addAndMakeVisible(midiLearnBtn);
        
        // CC display
        ccLabel.setText("CC: --", juce::dontSendNotification);
        ccLabel.setFont(DS::Font::small());
        ccLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        ccLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(ccLabel);
        
        // Add mapping button
        addMappingBtn.setButtonText("+ Add Mapping");
        styleButton(addMappingBtn, DS::Color::Accent::success());
        addMappingBtn.onClick = [this]() { addNewMapping(); };
        addAndMakeVisible(addMappingBtn);
        
        // Clear all button
        clearAllBtn.setButtonText("Clear All");
        styleButton(clearAllBtn, DS::Color::Accent::error());
        clearAllBtn.onClick = [this]() { clearAllMappings(); };
        addAndMakeVisible(clearAllBtn);
        
        // Mappings viewport
        mappingsViewport.setViewedComponent(&mappingsContainer, false);
        mappingsViewport.setScrollBarsShown(true, false);
        addAndMakeVisible(mappingsViewport);
    }
    
    void setMacroControl(MacroControl* macro) {
        macroControl = macro;
        if (macro) {
            nameEditor.setText(macro->getName(), juce::dontSendNotification);
            valueSlider->setValue(macro->getValue());
            if (macro->getMidiCC() >= 0) {
                ccLabel.setText("CC: " + juce::String(macro->getMidiCC()), juce::dontSendNotification);
            }
            refreshMappings();
        }
    }
    
    void paint(juce::Graphics& g) override {
        g.setColour(DS::Color::bgCard());
        g.fillRoundedRectangle(getLocalBounds().toFloat(), DS::Radius::M);
        
        g.setColour(DS::Color::Accent::primary().withAlpha(0.3f));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), DS::Radius::M, 1.0f);
        
        // Header glow
        auto headerArea = getLocalBounds().removeFromTop(40);
        juce::ColourGradient grad(DS::Color::Accent::primary().withAlpha(0.15f),
                                   headerArea.getCentreX(), headerArea.getY(),
                                   DS::Color::Accent::primary().withAlpha(0.0f),
                                   headerArea.getCentreX(), headerArea.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRect(headerArea);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(DS::Space::S);
        
        // Header: name editor
        nameEditor.setBounds(bounds.removeFromTop(32));
        bounds.removeFromTop(DS::Space::S);
        
        // Value and MIDI learn row
        auto controlRow = bounds.removeFromTop(90);
        valueSlider->setBounds(controlRow.removeFromLeft(90));
        controlRow.removeFromLeft(DS::Space::S);
        
        auto midiCol = controlRow;
        midiLearnBtn.setBounds(midiCol.removeFromTop(28));
        midiCol.removeFromTop(4);
        ccLabel.setBounds(midiCol.removeFromTop(20));
        
        bounds.removeFromTop(DS::Space::S);
        
        // Buttons row
        auto btnRow = bounds.removeFromTop(28);
        addMappingBtn.setBounds(btnRow.removeFromLeft(100));
        btnRow.removeFromLeft(DS::Space::S);
        clearAllBtn.setBounds(btnRow.removeFromLeft(80));
        
        bounds.removeFromTop(DS::Space::S);
        
        // Mappings list
        mappingsViewport.setBounds(bounds);
        layoutMappings();
    }
    
    std::function<void(int macroIndex)> onMappingChanged;
    
private:
    int index;
    MacroControl* macroControl = nullptr;
    
    juce::TextEditor nameEditor;
    std::unique_ptr<StyledSlider> valueSlider;
    juce::TextButton midiLearnBtn;
    juce::Label ccLabel;
    juce::TextButton addMappingBtn;
    juce::TextButton clearAllBtn;
    
    juce::Viewport mappingsViewport;
    juce::Component mappingsContainer;
    
    // Mapping rows
    struct MappingRow : public juce::Component {
        juce::ComboBox paramCombo;
        juce::Slider depthSlider;
        juce::ComboBox curveCombo;
        juce::TextButton removeBtn { "X" };
        
        MappingRow() {
            paramCombo.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
            paramCombo.setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
            addAndMakeVisible(paramCombo);
            
            depthSlider.setRange(-100, 100, 1);
            depthSlider.setValue(100);
            depthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
            depthSlider.setColour(juce::Slider::thumbColourId, DS::Color::Accent::primary());
            addAndMakeVisible(depthSlider);
            
            curveCombo.addItem("Linear", 1);
            curveCombo.addItem("Exponential", 2);
            curveCombo.addItem("Logarithmic", 3);
            curveCombo.addItem("S-Curve", 4);
            curveCombo.setSelectedId(1);
            curveCombo.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
            addAndMakeVisible(curveCombo);
            
            removeBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::error().withAlpha(0.3f));
            removeBtn.setColour(juce::TextButton::textColourOffId, DS::Color::Accent::error());
            addAndMakeVisible(removeBtn);
        }
        
        void resized() override {
            auto bounds = getLocalBounds();
            removeBtn.setBounds(bounds.removeFromRight(24));
            bounds.removeFromRight(4);
            curveCombo.setBounds(bounds.removeFromRight(90));
            bounds.removeFromRight(4);
            depthSlider.setBounds(bounds.removeFromRight(120));
            bounds.removeFromRight(4);
            paramCombo.setBounds(bounds);
        }
    };
    
    juce::OwnedArray<MappingRow> mappingRows;
    
    void styleButton(juce::TextButton& btn, juce::Colour accent) {
        btn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        btn.setColour(juce::TextButton::buttonOnColourId, accent);
        btn.setColour(juce::TextButton::textColourOffId, accent);
        btn.setColour(juce::TextButton::textColourOnId, DS::Color::textPrimary());
    }
    
    void addNewMapping() {
        auto* row = mappingRows.add(new MappingRow());
        mappingsContainer.addAndMakeVisible(row);
        
        // Populate parameter list
        populateParameterCombo(row->paramCombo);
        
        // Use weak reference pattern to avoid crash on deletion
        row->removeBtn.onClick = [this, rowPtr = juce::Component::SafePointer<MappingRow>(row)]() {
            if (rowPtr != nullptr) {
                // Post the removal to avoid deleting during callback
                juce::MessageManager::callAsync([this, rowPtr]() {
                    if (rowPtr != nullptr) {
                        mappingRows.removeObject(rowPtr.getComponent());
                        layoutMappings();
                        if (onMappingChanged) onMappingChanged(index);
                    }
                });
            }
        };
        
        layoutMappings();
        if (onMappingChanged) onMappingChanged(index);
    }
    
    void clearAllMappings() {
        mappingRows.clear();
        layoutMappings();
        if (onMappingChanged) onMappingChanged(index);
    }
    
    void refreshMappings() {
        mappingRows.clear();
        if (macroControl) {
            for (size_t i = 0; i < macroControl->getMappings().size(); ++i) {
                addNewMapping();
                // TODO: populate from macro control
            }
        }
        layoutMappings();
    }
    
    void layoutMappings() {
        int y = 0;
        const int rowH = 28;
        const int gap = 4;
        for (auto* row : mappingRows) {
            row->setBounds(0, y, mappingsViewport.getWidth() - 10, rowH);
            y += rowH + gap;
        }
        mappingsContainer.setSize(mappingsViewport.getWidth() - 10, juce::jmax(y, mappingsViewport.getHeight()));
    }
    
    void populateParameterCombo(juce::ComboBox& combo) {
        combo.clear();
        int id = 1;
        
        // DCO parameters
        combo.addSectionHeading("DCO-1");
        combo.addItem("DCO1 Tune", id++);
        combo.addItem("DCO1 LFO Depth", id++);
        combo.addItem("DCO1 ENV Depth", id++);
        
        combo.addSectionHeading("DCO-2");
        combo.addItem("DCO2 Tune", id++);
        combo.addItem("DCO2 LFO Depth", id++);
        combo.addItem("DCO2 ENV Depth", id++);
        combo.addItem("DCO2 Fine Tune", id++);
        
        combo.addSectionHeading("VCF");
        combo.addItem("VCF Cutoff", id++);
        combo.addItem("VCF Resonance", id++);
        combo.addItem("VCF LFO Depth", id++);
        combo.addItem("VCF ENV Depth", id++);
        combo.addItem("VCF Key Follow", id++);
        
        combo.addSectionHeading("VCA");
        combo.addItem("VCA Level", id++);
        
        combo.addSectionHeading("Mixer");
        combo.addItem("DCO1 Level", id++);
        combo.addItem("DCO2 Level", id++);
        
        combo.addSectionHeading("LFO");
        combo.addItem("LFO1 Rate", id++);
        combo.addItem("LFO1 Delay", id++);
        combo.addItem("LFO2 Rate", id++);
        combo.addItem("LFO2 Delay", id++);
        
        combo.addSectionHeading("ENV1");
        combo.addItem("ENV1 Attack", id++);
        combo.addItem("ENV1 Decay", id++);
        combo.addItem("ENV1 Sustain", id++);
        combo.addItem("ENV1 Release", id++);
        
        combo.addSectionHeading("ENV2");
        combo.addItem("ENV2 Attack", id++);
        combo.addItem("ENV2 Decay", id++);
        combo.addItem("ENV2 Sustain", id++);
        combo.addItem("ENV2 Release", id++);
        
        combo.addSectionHeading("Chorus");
        combo.addItem("Chorus Rate", id++);
        
        combo.setSelectedId(1);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroEditorCard)
};


//==============================================================================
/**
 * MacroTab - Full macro configuration interface
 */
class MacroTab : public juce::Component
{
public:
    MacroTab()
    {
        // Header section
        headerSection = std::make_unique<GlowingSection>("MACRO CONTROL", 
            DS::Color::Accent::primary());
        addAndMakeVisible(headerSection.get());
        
        // Global controls in header
        globalEnableBtn.setButtonText("MACROS ON");
        globalEnableBtn.setClickingTogglesState(true);
        globalEnableBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        globalEnableBtn.setColour(juce::TextButton::buttonOnColourId, DS::Color::Accent::success());
        globalEnableBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
        globalEnableBtn.setColour(juce::TextButton::textColourOnId, DS::Color::textPrimary());
        addAndMakeVisible(globalEnableBtn);
        
        savePresetBtn.setButtonText("Save Preset");
        styleButton(savePresetBtn, DS::Color::Accent::cyan());
        addAndMakeVisible(savePresetBtn);
        
        loadPresetBtn.setButtonText("Load Preset");
        styleButton(loadPresetBtn, DS::Color::Accent::cyan());
        addAndMakeVisible(loadPresetBtn);
        
        initAllBtn.setButtonText("Init All");
        styleButton(initAllBtn, DS::Color::Accent::warning());
        addAndMakeVisible(initAllBtn);
        
        // Create 8 macro editor cards
        for (int i = 0; i < 8; ++i) {
            auto* card = new MacroEditorCard(i);
            macroCards.add(card);
            addAndMakeVisible(card);
        }
        
        // Help text
        helpLabel.setText("Macros allow you to control multiple parameters with a single control. "
                         "Each macro can modulate up to 8 parameters with individual depth and curve settings.",
                         juce::dontSendNotification);
        helpLabel.setFont(DS::Font::small());
        helpLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        helpLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(helpLabel);
    }
    
    void setMacroManager(MacroManager* manager) {
        macroManager = manager;
        if (manager) {
            for (int i = 0; i < 8; ++i) {
                macroCards[i]->setMacroControl(&manager->getMacro(i));
            }
        }
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DS::Color::bg());
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(DS::Space::M);
        
        // Header row
        auto headerRow = bounds.removeFromTop(50);
        headerSection->setBounds(headerRow.removeFromLeft(200));
        headerRow.removeFromLeft(DS::Space::L);
        
        globalEnableBtn.setBounds(headerRow.removeFromLeft(100).withHeight(32).withY(headerRow.getY() + 9));
        headerRow.removeFromLeft(DS::Space::M);
        savePresetBtn.setBounds(headerRow.removeFromLeft(90).withHeight(28).withY(headerRow.getY() + 11));
        headerRow.removeFromLeft(DS::Space::S);
        loadPresetBtn.setBounds(headerRow.removeFromLeft(90).withHeight(28).withY(headerRow.getY() + 11));
        headerRow.removeFromLeft(DS::Space::S);
        initAllBtn.setBounds(headerRow.removeFromLeft(70).withHeight(28).withY(headerRow.getY() + 11));
        
        bounds.removeFromTop(DS::Space::M);
        
        // Help text
        helpLabel.setBounds(bounds.removeFromTop(36));
        bounds.removeFromTop(DS::Space::M);
        
        // Macro cards grid (2 rows x 4 columns)
        const int numCols = 4;
        const int numRows = 2;
        const int gap = DS::Space::M;
        const int cardW = (bounds.getWidth() - (numCols - 1) * gap) / numCols;
        const int cardH = (bounds.getHeight() - (numRows - 1) * gap) / numRows;
        
        for (int row = 0; row < numRows; ++row) {
            auto rowBounds = bounds.removeFromTop(cardH);
            for (int col = 0; col < numCols; ++col) {
                int idx = row * numCols + col;
                if (idx < macroCards.size()) {
                    macroCards[idx]->setBounds(rowBounds.removeFromLeft(cardW));
                    if (col < numCols - 1) rowBounds.removeFromLeft(gap);
                }
            }
            if (row < numRows - 1) bounds.removeFromTop(gap);
        }
    }
    
private:
    MacroManager* macroManager = nullptr;
    
    std::unique_ptr<GlowingSection> headerSection;
    juce::TextButton globalEnableBtn;
    juce::TextButton savePresetBtn;
    juce::TextButton loadPresetBtn;
    juce::TextButton initAllBtn;
    juce::Label helpLabel;
    
    juce::OwnedArray<MacroEditorCard> macroCards;
    
    void styleButton(juce::TextButton& btn, juce::Colour accent) {
        btn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        btn.setColour(juce::TextButton::textColourOffId, accent);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroTab)
};
