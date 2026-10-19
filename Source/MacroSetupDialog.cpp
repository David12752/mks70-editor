/*
  ==============================================================================

    MacroSetupDialog.cpp
    Implementation of macro setup dialog

  ==============================================================================
*/

#include "MacroSetupDialog.h"

//==============================================================================
// Column IDs for table
enum ColumnId
{
    COL_PARAMETER = 1,
    COL_MIN = 2,
    COL_MAX = 3,
    COL_CURVE = 4,
    COL_INVERTED = 5,
    COL_REMOVE = 6
};

//==============================================================================
MacroSetupDialog::MacroSetupDialog(MacroControl& macro, MacroManager& manager)
    : macroControl(macro), macroManager(manager)
{
    // Name controls
    nameLabel.setText("Name:", juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(nameLabel);
    
    nameEditor.setText(macro.getName());
    nameEditor.setFont(juce::Font(14.0f));
    addAndMakeVisible(nameEditor);
    
    // MIDI CC controls
    ccLabel.setText("MIDI CC:", juce::dontSendNotification);
    ccLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(ccLabel);
    
    ccSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    ccSlider.setRange(-1, 127, 1);
    ccSlider.setValue(macro.getMidiCC());
    ccSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    ccSlider.onValueChange = [this]()
    {
        int ccValue = static_cast<int>(ccSlider.getValue());
        ccValueLabel.setText(ccValue < 0 ? "OFF" : juce::String(ccValue), juce::dontSendNotification);
    };
    addAndMakeVisible(ccSlider);
    
    ccValueLabel.setText(macro.getMidiCC() < 0 ? "OFF" : juce::String(macro.getMidiCC()), juce::dontSendNotification);
    ccValueLabel.setJustificationType(juce::Justification::centred);
    ccValueLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(ccValueLabel);
    
    // MIDI Channel controls
    channelLabel.setText("Channel:", juce::dontSendNotification);
    channelLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(channelLabel);
    
    channelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    channelSlider.setRange(1, 16, 1);
    channelSlider.setValue(macro.getMidiChannel() + 1);
    channelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    channelSlider.onValueChange = [this]()
    {
        int ch = static_cast<int>(channelSlider.getValue());
        channelValueLabel.setText(juce::String(ch), juce::dontSendNotification);
    };
    addAndMakeVisible(channelSlider);
    
    channelValueLabel.setText(juce::String(macro.getMidiChannel() + 1), juce::dontSendNotification);
    channelValueLabel.setJustificationType(juce::Justification::centred);
    channelValueLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(channelValueLabel);
    
    // Table
    table.setModel(this);
    table.setColour(juce::ListBox::backgroundColourId, juce::Colours::darkgrey);
    table.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
    table.setOutlineThickness(1);
    
    table.getHeader().addColumn("Parameter", COL_PARAMETER, 150, 100, 300);
    table.getHeader().addColumn("Min", COL_MIN, 60, 50, 100);
    table.getHeader().addColumn("Max", COL_MAX, 60, 50, 100);
    table.getHeader().addColumn("Curve", COL_CURVE, 120, 100, 150);
    table.getHeader().addColumn("Inverted", COL_INVERTED, 70, 60, 80);
    table.getHeader().addColumn("", COL_REMOVE, 40, 40, 40);
    
    addAndMakeVisible(table);
    updateTableData();
    
    // Action buttons
    addButton.setButtonText("Add Parameter");
    addButton.addListener(this);
    addAndMakeVisible(addButton);
    
    removeButton.setButtonText("Remove Selected");
    removeButton.addListener(this);
    addAndMakeVisible(removeButton);
    
    learnButton.setButtonText("MIDI Learn");
    learnButton.setClickingTogglesState(true);
    learnButton.addListener(this);
    learnButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
    learnButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    addAndMakeVisible(learnButton);
    
    clearButton.setButtonText("Clear All");
    clearButton.addListener(this);
    addAndMakeVisible(clearButton);
    
    // Template buttons
    templateLabel.setText("Templates:", juce::dontSendNotification);
    templateLabel.setJustificationType(juce::Justification::centredLeft);
    templateLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(templateLabel);
    
    brightnessBtn.setButtonText("Brightness");
    brightnessBtn.addListener(this);
    addAndMakeVisible(brightnessBtn);
    
    filterSweepBtn.setButtonText("Filter Sweep");
    filterSweepBtn.addListener(this);
    addAndMakeVisible(filterSweepBtn);
    
    envelopeBtn.setButtonText("Envelope");
    envelopeBtn.addListener(this);
    addAndMakeVisible(envelopeBtn);
    
    vibratoBtn.setButtonText("Vibrato");
    vibratoBtn.addListener(this);
    addAndMakeVisible(vibratoBtn);
    
    movementBtn.setButtonText("Movement");
    movementBtn.addListener(this);
    addAndMakeVisible(movementBtn);
    
    attackBtn.setButtonText("Attack");
    attackBtn.addListener(this);
    addAndMakeVisible(attackBtn);
    
    decayBtn.setButtonText("Decay");
    decayBtn.addListener(this);
    addAndMakeVisible(decayBtn);
    
    resonanceBtn.setButtonText("Resonance");
    resonanceBtn.addListener(this);
    addAndMakeVisible(resonanceBtn);
    
    // OK/Cancel
    okButton.setButtonText("OK");
    okButton.addListener(this);
    addAndMakeVisible(okButton);
    
    cancelButton.setButtonText("Cancel");
    cancelButton.addListener(this);
    addAndMakeVisible(cancelButton);
    
    setSize(700, 600);
}

MacroSetupDialog::~MacroSetupDialog()
{
}

void MacroSetupDialog::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("Macro Setup: " + macroControl.getName(), 10, 10, getWidth() - 20, 30, juce::Justification::centred);
}

void MacroSetupDialog::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(40);  // Title
    
    // Top controls
    auto topArea = bounds.removeFromTop(80);
    
    auto nameRow = topArea.removeFromTop(30);
    nameLabel.setBounds(nameRow.removeFromLeft(80));
    nameEditor.setBounds(nameRow.reduced(5, 2));
    
    topArea.removeFromTop(5);
    
    auto ccRow = topArea.removeFromTop(30);
    ccLabel.setBounds(ccRow.removeFromLeft(80));
    ccSlider.setBounds(ccRow.removeFromLeft(150));
    ccRow.removeFromLeft(5);
    ccValueLabel.setBounds(ccRow.removeFromLeft(50));
    
    ccRow.removeFromLeft(20);
    channelLabel.setBounds(ccRow.removeFromLeft(70));
    channelSlider.setBounds(ccRow.removeFromLeft(100));
    ccRow.removeFromLeft(5);
    channelValueLabel.setBounds(ccRow.removeFromLeft(40));
    
    bounds.removeFromTop(10);
    
    // Action buttons
    auto buttonRow = bounds.removeFromTop(30);
    addButton.setBounds(buttonRow.removeFromLeft(120));
    buttonRow.removeFromLeft(5);
    removeButton.setBounds(buttonRow.removeFromLeft(130));
    buttonRow.removeFromLeft(5);
    learnButton.setBounds(buttonRow.removeFromLeft(100));
    buttonRow.removeFromLeft(5);
    clearButton.setBounds(buttonRow.removeFromLeft(80));
    
    bounds.removeFromTop(5);
    
    // Table
    table.setBounds(bounds.removeFromTop(250));
    
    bounds.removeFromTop(10);
    
    // Templates
    auto templateArea = bounds.removeFromTop(90);
    templateLabel.setBounds(templateArea.removeFromTop(25));
    
    auto template1 = templateArea.removeFromTop(30);
    brightnessBtn.setBounds(template1.removeFromLeft(120));
    template1.removeFromLeft(5);
    filterSweepBtn.setBounds(template1.removeFromLeft(120));
    template1.removeFromLeft(5);
    envelopeBtn.setBounds(template1.removeFromLeft(100));
    template1.removeFromLeft(5);
    vibratoBtn.setBounds(template1.removeFromLeft(100));
    
    templateArea.removeFromTop(5);
    
    auto template2 = templateArea.removeFromTop(30);
    movementBtn.setBounds(template2.removeFromLeft(110));
    template2.removeFromLeft(5);
    attackBtn.setBounds(template2.removeFromLeft(80));
    template2.removeFromLeft(5);
    decayBtn.setBounds(template2.removeFromLeft(80));
    template2.removeFromLeft(5);
    resonanceBtn.setBounds(template2.removeFromLeft(110));
    
    bounds.removeFromTop(10);
    
    // OK/Cancel
    auto okCancelRow = bounds.removeFromTop(35);
    cancelButton.setBounds(okCancelRow.removeFromRight(100));
    okCancelRow.removeFromRight(10);
    okButton.setBounds(okCancelRow.removeFromRight(100));
}

//==============================================================================
// TableListBoxModel implementation

int MacroSetupDialog::getNumRows()
{
    return static_cast<int>(tableData.size());
}

void MacroSetupDialog::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue.withAlpha(0.3f));
    else if (rowNumber % 2 == 0)
        g.fillAll(juce::Colours::black.withAlpha(0.2f));
}

void MacroSetupDialog::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= tableData.size())
        return;
    
    auto* mapping = tableData[rowNumber];
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f));
    
    juce::String text;
    
    switch (columnId)
    {
        case COL_PARAMETER:
            text = mapping->paramId;
            break;
        case COL_MIN:
            text = juce::String(static_cast<int>(mapping->minValue));
            break;
        case COL_MAX:
            text = juce::String(static_cast<int>(mapping->maxValue));
            break;
    }
    
    if (!text.isEmpty())
        g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
}

juce::Component* MacroSetupDialog::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected,
                                                           juce::Component* existingComponentToUpdate)
{
    if (rowNumber >= tableData.size())
        return nullptr;
    
    auto* mapping = tableData[rowNumber];
    
    // Curve selector
    if (columnId == COL_CURVE)
    {
        auto* selector = dynamic_cast<CurveSelectorBox*>(existingComponentToUpdate);
        if (selector == nullptr)
            selector = new CurveSelectorBox(*this, rowNumber);
        else
            selector->setRow(rowNumber);

        // Set current curve
        int curveId = 1;  // Default LINEAR
        switch (mapping->curve)
        {
            case MorphEngine::CURVE_LINEAR: curveId = 1; break;
            case MorphEngine::CURVE_EXPONENTIAL: curveId = 2; break;
            case MorphEngine::CURVE_LOGARITHMIC: curveId = 3; break;
            case MorphEngine::CURVE_SCURVE: curveId = 4; break;
            case MorphEngine::CURVE_STEPPED: curveId = 5; break;
        }
        selector->setSelectedId(curveId, juce::dontSendNotification);

        return selector;
    }
    
    // Inverted toggle
    if (columnId == COL_INVERTED)
    {
        auto* toggle = dynamic_cast<InvertedToggle*>(existingComponentToUpdate);
        if (toggle == nullptr)
            toggle = new InvertedToggle(*this, rowNumber);
        else
            toggle->setRow(rowNumber);

        toggle->setToggleState(mapping->inverted, juce::dontSendNotification);

        return toggle;
    }
    
    // Remove button
    if (columnId == COL_REMOVE)
    {
        auto* removeBtn = dynamic_cast<juce::TextButton*>(existingComponentToUpdate);
        if (removeBtn == nullptr)
            removeBtn = new juce::TextButton("X");

        const juce::String currentParam = mapping->paramId; // capture stable id
        removeBtn->onClick = [this, currentParam]()
        {
            macroControl.removeMapping(currentParam);
            updateTableData();
            if (onMappingsChanged)
                onMappingsChanged();
        };
        return removeBtn;
    }
    
    return nullptr;
}

void MacroSetupDialog::cellClicked(int rowNumber, int columnId, const juce::MouseEvent&)
{
    table.selectRow(rowNumber);
}

//==============================================================================
// Button handlers

void MacroSetupDialog::buttonClicked(juce::Button* button)
{
    if (button == &addButton)
    {
        showAddParameterMenu();
    }
    else if (button == &removeButton)
    {
        removeSelectedParameter();
    }
    else if (button == &learnButton)
    {
        toggleLearnMode();
    }
    else if (button == &clearButton)
    {
        clearAllMappings();
    }
    else if (button == &brightnessBtn)
    {
        applyTemplate("Brightness");
    }
    else if (button == &filterSweepBtn)
    {
        applyTemplate("Filter Sweep");
    }
    else if (button == &envelopeBtn)
    {
        applyTemplate("Envelope");
    }
    else if (button == &vibratoBtn)
    {
        applyTemplate("Vibrato");
    }
    else if (button == &movementBtn)
    {
        applyTemplate("Movement");
    }
    else if (button == &attackBtn)
    {
        applyTemplate("Attack");
    }
    else if (button == &decayBtn)
    {
        applyTemplate("Decay");
    }
    else if (button == &resonanceBtn)
    {
        applyTemplate("Resonance");
    }
    else if (button == &okButton)
    {
        saveChanges();
        if (onClose)
            onClose();
    }
    else if (button == &cancelButton)
    {
        if (onClose)
            onClose();
    }
}

//==============================================================================
// Helper methods

void MacroSetupDialog::updateTableData()
{
    tableData.clear();
    auto& mappings = macroControl.getMappings();
    for (auto& m : mappings)
        tableData.push_back(&m);
    
    table.updateContent();
    table.repaint();
}

void MacroSetupDialog::cellValueChanged(int rowNumber, int columnId, const juce::String& newValue)
{
    if (rowNumber >= tableData.size())
        return;
    
    auto* mapping = tableData[rowNumber];
    
    switch (columnId)
    {
        case COL_MIN:
            mapping->minValue = newValue.getFloatValue();
            break;
        case COL_MAX:
            mapping->maxValue = newValue.getFloatValue();
            break;
    }
    
    if (onMappingsChanged)
        onMappingsChanged();
}

void MacroSetupDialog::curveSelectorChanged(int rowNumber, int curveId)
{
    if (rowNumber >= tableData.size())
        return;
    
    auto* mapping = tableData[rowNumber];
    
    switch (curveId)
    {
        case 1: mapping->curve = MorphEngine::CURVE_LINEAR; break;
        case 2: mapping->curve = MorphEngine::CURVE_EXPONENTIAL; break;
        case 3: mapping->curve = MorphEngine::CURVE_LOGARITHMIC; break;
        case 4: mapping->curve = MorphEngine::CURVE_SCURVE; break;
        case 5: mapping->curve = MorphEngine::CURVE_STEPPED; break;
    }
    
    if (onMappingsChanged)
        onMappingsChanged();
}

void MacroSetupDialog::invertedToggled(int rowNumber, bool inverted)
{
    if (rowNumber >= tableData.size())
        return;
    
    tableData[rowNumber]->inverted = inverted;
    
    if (onMappingsChanged)
        onMappingsChanged();
}

void MacroSetupDialog::showAddParameterMenu()
{
    juce::PopupMenu menu;
    auto params = getAvailableParameters();
    
    for (int i = 0; i < params.size(); ++i)
        menu.addItem(i + 1, params[i]);
    
    menu.showMenuAsync(juce::PopupMenu::Options(),
        [this, params](int result)
        {
            if (result > 0 && result <= params.size())
            {
                MacroControl::ParameterMapping mapping(params[result - 1], 0.0f, 127.0f);
                macroControl.addMapping(mapping);
                updateTableData();
                if (onMappingsChanged)
                    onMappingsChanged();
            }
        });
}

void MacroSetupDialog::removeSelectedParameter()
{
    int selectedRow = table.getSelectedRow();
    if (selectedRow >= 0 && selectedRow < tableData.size())
    {
        macroControl.removeMapping(tableData[selectedRow]->paramId);
        updateTableData();
        if (onMappingsChanged)
            onMappingsChanged();
    }
}

void MacroSetupDialog::toggleLearnMode()
{
    bool isLearning = learnButton.getToggleState();
    
    if (isLearning)
    {
        // Start learning mode
        int macroIndex = 0;
        for (int i = 0; i < MacroManager::NUM_MACROS; i++)
        {
            if (&macroManager.getMacro(i) == &macroControl)
            {
                macroIndex = i;
                break;
            }
        }
        
        macroManager.startLearn(macroIndex);
        
        // Show info message
        auto options = juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::InfoIcon)
            .withTitle("MIDI Learn Active")
            .withMessage("MIDI Learn is now active!\n\n"
                        "Move any parameter in the editor to add it to this macro.\n"
                        "The parameter will be mapped with its current value ±30.\n\n"
                        "Click 'MIDI Learn' again to stop learning.")
            .withButton("OK");
        
        juce::AlertWindow::showAsync(options, nullptr);
        
        DBG("MIDI Learn started for macro " + juce::String(macroIndex));
    }
    else
    {
        // Stop learning mode
        macroManager.stopLearn();
        
        // Update table to show new mappings
        updateTableData();
        table.updateContent();
        
        if (onMappingsChanged)
            onMappingsChanged();
        
        DBG("MIDI Learn stopped");
    }
}

void MacroSetupDialog::clearAllMappings()
{
    auto options = juce::MessageBoxOptions()
        .withIconType(juce::MessageBoxIconType::QuestionIcon)
        .withTitle("Clear All Mappings")
        .withMessage("Are you sure you want to clear all parameter mappings?")
        .withButton("Yes")
        .withButton("No");
    
    juce::AlertWindow::showAsync(options, [this](int result)
    {
        if (result == 1)  // Yes button
        {
            macroControl.clearMappings();
            updateTableData();
            if (onMappingsChanged)
                onMappingsChanged();
        }
    });
}

void MacroSetupDialog::applyTemplate(const juce::String& templateName)
{
    // Find macro index in manager
    int macroIndex = -1;
    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        if (&macroManager.getMacro(i) == &macroControl)
        {
            macroIndex = i;
            break;
        }
    }
    
    if (macroIndex >= 0)
    {
        macroManager.loadTemplate(macroIndex, templateName);
        nameEditor.setText(macroControl.getName());
        updateTableData();
        if (onMappingsChanged)
            onMappingsChanged();
    }
}

void MacroSetupDialog::saveChanges()
{
    // Save name
    macroControl.setName(nameEditor.getText());
    
    // Save MIDI CC and channel
    macroControl.setMidiCC(static_cast<int>(ccSlider.getValue()));
    macroControl.setMidiChannel(static_cast<int>(channelSlider.getValue()) - 1);
}

juce::StringArray MacroSetupDialog::getAvailableParameters()
{
    juce::StringArray params;
    
    // DCO parameters
    params.add("dco1Range");
    params.add("dco1Waveform");
    params.add("dco1Tune");
    params.add("dco1LFOMod");
    params.add("dco1ENVMod");
    params.add("dco2Range");
    params.add("dco2Waveform");
    params.add("dco2Tune");
    params.add("dco2FineTune");
    params.add("dco2LFOMod");
    params.add("dco2ENVMod");
    
    // Mixer parameters
    params.add("mixerDCO1");
    params.add("mixerDCO2");
    params.add("mixerENVMod");
    
    // VCF parameters
    params.add("vcfCutoff");
    params.add("vcfResonance");
    params.add("vcfLFOMod");
    params.add("vcfENVMod");
    params.add("vcfKeyFollow");
    
    // VCA parameters
    params.add("vcaLevel");
    
    // Envelope parameters
    params.add("env1Attack");
    params.add("env1Decay");
    params.add("env1Sustain");
    params.add("env1Release");
    params.add("env2Attack");
    params.add("env2Decay");
    params.add("env2Sustain");
    params.add("env2Release");
    
    // LFO parameters
    params.add("lfo1Rate");
    params.add("lfo1Delay");
    params.add("lfo1Waveform");
    
    // Chorus parameters
    params.add("chorusRate");
    
    return params;
}

//==============================================================================
// CellEditor implementation

void MacroSetupDialog::CellEditor::textWasEdited()
{
    owner.cellValueChanged(rowNumber, columnId, getText());
}
