/*
  ==============================================================================

    MacroSetupDialog.h
    Configuration dialog for macro control parameter mappings

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MacroControl.h"

//==============================================================================
/**
 * MacroSetupDialog - Configure macro control parameter mappings
 * 
 * Features:
 * - Name and MIDI CC configuration
 * - Parameter mappings table (Parameter, Min, Max, Curve, Inverted)
 * - Add/Remove parameter buttons
 * - Learn mode toggle
 * - Preset template buttons
 * - Save/Load functionality
 */
class MacroSetupDialog : public juce::Component,
                        public juce::TableListBoxModel,
                        public juce::Button::Listener
{
public:
    MacroSetupDialog(MacroControl& macro, MacroManager& manager);
    ~MacroSetupDialog() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // TableListBoxModel implementation
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, 
                                            juce::Component* existingComponentToUpdate) override;
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;
    
    // Button::Listener
    void buttonClicked(juce::Button* button) override;
    
    // Callbacks
    std::function<void()> onClose;
    std::function<void()> onMappingsChanged;
    
private:
    MacroControl& macroControl;
    MacroManager& macroManager;
    
    // Top controls
    juce::Label nameLabel;
    juce::TextEditor nameEditor;
    
    juce::Label ccLabel;
    juce::Slider ccSlider;
    juce::Label ccValueLabel;
    
    juce::Label channelLabel;
    juce::Slider channelSlider;
    juce::Label channelValueLabel;
    
    // Table
    juce::TableListBox table;
    std::vector<MacroControl::ParameterMapping*> tableData;
    
    // Table editing components
    class CellEditor : public juce::Label
    {
    public:
        CellEditor(MacroSetupDialog& owner, int row, int col)
            : owner(owner), rowNumber(row), columnId(col)
        {
            setEditable(true, true, false);
        }
        
        void textWasEdited() override;
        
    private:
        MacroSetupDialog& owner;
        int rowNumber;
        int columnId;
    };
    
    class CurveSelectorBox : public juce::ComboBox
    {
    public:
        CurveSelectorBox(MacroSetupDialog& dialogOwner, int row)
            : owner(dialogOwner), rowNumber(row)
        {
            addItem("Linear", 1);
            addItem("Exponential", 2);
            addItem("Logarithmic", 3);
            addItem("S-Curve", 4);
            addItem("Stepped", 5);
            onChange = [this]() { owner.curveSelectorChanged(rowNumber, this->getSelectedId()); };
        }

        void setRow(int row)
        {
            rowNumber = row;
            onChange = [this]() { owner.curveSelectorChanged(rowNumber, this->getSelectedId()); };
        }
        
    private:
        MacroSetupDialog& owner;
        int rowNumber;
    };
    
    class InvertedToggle : public juce::ToggleButton
    {
    public:
        InvertedToggle(MacroSetupDialog& dialogOwner, int row)
            : owner(dialogOwner), rowNumber(row)
        {
            onClick = [this]() { owner.invertedToggled(rowNumber, this->getToggleState()); };
        }

        void setRow(int row)
        {
            rowNumber = row;
            onClick = [this]() { owner.invertedToggled(rowNumber, this->getToggleState()); };
        }
        
    private:
        MacroSetupDialog& owner;
        int rowNumber;
    };
    
    // Buttons
    juce::TextButton addButton;
    juce::TextButton removeButton;
    juce::TextButton learnButton;
    juce::TextButton clearButton;
    
    // Template buttons
    juce::Label templateLabel;
    juce::TextButton brightnessBtn;
    juce::TextButton filterSweepBtn;
    juce::TextButton envelopeBtn;
    juce::TextButton vibratoBtn;
    juce::TextButton movementBtn;
    juce::TextButton attackBtn;
    juce::TextButton decayBtn;
    juce::TextButton resonanceBtn;
    
    // Action buttons
    juce::TextButton okButton;
    juce::TextButton cancelButton;
    
    // Methods
    void updateTableData();
    void cellValueChanged(int rowNumber, int columnId, const juce::String& newValue);
    void curveSelectorChanged(int rowNumber, int curveId);
    void invertedToggled(int rowNumber, bool inverted);
    void showAddParameterMenu();
    void removeSelectedParameter();
    void toggleLearnMode();
    void clearAllMappings();
    void applyTemplate(const juce::String& templateName);
    void saveChanges();
    
    // Available parameters
    juce::StringArray getAvailableParameters();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroSetupDialog)
};
