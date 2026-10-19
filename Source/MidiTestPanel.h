/*
  ==============================================================================

    MidiTestPanel.h
    Manual MIDI Address Testing Tool
    
    Allows manual testing of SysEx addresses to discover the correct mapping
    for Vecoven firmware.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiHandler.h"

class MidiTestPanel : public juce::Component,
                      public juce::Button::Listener,
                      public juce::Slider::Listener,
                      public juce::Timer
{
public:
    MidiTestPanel(MidiHandler& midiHandler)
        : midiHandler(midiHandler)
    {
        // Title
        titleLabel.setText("MIDI Address Scanner - Vecoven Mapping Discovery", juce::dontSendNotification);
        titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
        titleLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
        titleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(titleLabel);
        
        // Tone selector
        toneLabel.setText("Tone:", juce::dontSendNotification);
        toneLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(toneLabel);
        
        toneSelector.addItem("Upper (A)", 1);
        toneSelector.addItem("Lower (B)", 2);
        toneSelector.setSelectedId(1);
        addAndMakeVisible(toneSelector);
        
        // Value slider
        valueLabel.setText("Value:", juce::dontSendNotification);
        valueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(valueLabel);
        
        valueSlider.setRange(0, 127, 1);
        valueSlider.setValue(64);
        valueSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        valueSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
        valueSlider.addListener(this);
        addAndMakeVisible(valueSlider);
        
        // Current address display
        currentAddressLabel.setText("Current: 0 (0x00)", juce::dontSendNotification);
        currentAddressLabel.setFont(juce::Font(16.0f, juce::Font::bold));
        currentAddressLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
        addAndMakeVisible(currentAddressLabel);
        
        // Auto-scan controls
        autoScanButton.setButtonText("Start Auto-Scan");
        autoScanButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2E7D32));
        autoScanButton.addListener(this);
        addAndMakeVisible(autoScanButton);
        
        nextAddressButton.setButtonText("Next Address >>");
        nextAddressButton.addListener(this);
        addAndMakeVisible(nextAddressButton);
        
        prevAddressButton.setButtonText("<< Prev Address");
        prevAddressButton.addListener(this);
        addAndMakeVisible(prevAddressButton);
        
        // Parameter name input for current address
        paramNameLabel.setText("Parameter found:", juce::dontSendNotification);
        paramNameLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
        addAndMakeVisible(paramNameLabel);
        
        paramNameInput.setTextToShowWhenEmpty("Type what this address controls...", juce::Colours::grey);
        paramNameInput.onReturnKey = [this]() { saveCurrentAndAdvance(); };
        addAndMakeVisible(paramNameInput);
        
        saveAndNextButton.setButtonText("Save & Next");
        saveAndNextButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1565C0));
        saveAndNextButton.addListener(this);
        addAndMakeVisible(saveAndNextButton);
        
        // Address grid (0-63)
        setupAddressGrid();
        
        // Discovered mappings display
        mappingsLabel.setText("Discovered Mappings:", juce::dontSendNotification);
        mappingsLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(mappingsLabel);
        
        mappingsEditor.setMultiLine(true);
        mappingsEditor.setReadOnly(false);
        mappingsEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::plain));
        mappingsEditor.setText(getDefaultMappingsHeader());
        addAndMakeVisible(mappingsEditor);
        
        // Export/Import buttons
        exportButton.setButtonText("Export JSON");
        exportButton.addListener(this);
        addAndMakeVisible(exportButton);
        
        importButton.setButtonText("Import JSON");
        importButton.addListener(this);
        addAndMakeVisible(importButton);
        
        clearButton.setButtonText("Clear All");
        clearButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFC62828));
        clearButton.addListener(this);
        addAndMakeVisible(clearButton);
        
        // Initialize discovered array
        for (int i = 0; i < 128; i++)
            discoveredParams[i] = "";
        
        // Select address 0
        selectAddress(0);
    }
    
    ~MidiTestPanel() override 
    {
        stopTimer();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xFF1a1a2e));
        
        // Grid section background
        g.setColour(juce::Colour(0xFF2d2d44));
        g.fillRoundedRectangle(10.0f, 70.0f, (float)getWidth() - 20.0f, 200.0f, 8.0f);
        
        // Controls section background
        g.setColour(juce::Colour(0xFF3d3d54));
        g.fillRoundedRectangle(10.0f, 280.0f, (float)getWidth() - 20.0f, 100.0f, 8.0f);
    }
    
    void resized() override
    {
        int y = 10;
        int margin = 15;
        int width = getWidth() - 30;
        
        // Title
        titleLabel.setBounds(margin, y, width, 30);
        y += 40;
        
        // Tone selector and value
        toneLabel.setBounds(margin, y, 40, 25);
        toneSelector.setBounds(margin + 45, y, 100, 25);
        valueLabel.setBounds(margin + 160, y, 45, 25);
        valueSlider.setBounds(margin + 210, y, 200, 25);
        currentAddressLabel.setBounds(margin + 430, y, 200, 25);
        y += 35;
        
        // Address grid (8x8 = 64 buttons)
        int gridStartY = y;
        int btnSize = 38;
        int btnGap = 3;
        int gridWidth = 8 * (btnSize + btnGap);
        int gridX = margin + 10;
        
        for (int i = 0; i < 64; i++)
        {
            int row = i / 8;
            int col = i % 8;
            addressButtons[i]->setBounds(
                gridX + col * (btnSize + btnGap),
                gridStartY + row * (btnSize + btnGap),
                btnSize, btnSize
            );
        }
        
        y = gridStartY + 8 * (btnSize + btnGap) + 10;
        
        // Auto-scan controls
        prevAddressButton.setBounds(margin, y, 120, 28);
        autoScanButton.setBounds(margin + 130, y, 140, 28);
        nextAddressButton.setBounds(margin + 280, y, 120, 28);
        y += 38;
        
        // Parameter input section
        paramNameLabel.setBounds(margin, y, 120, 25);
        paramNameInput.setBounds(margin + 125, y, 300, 28);
        saveAndNextButton.setBounds(margin + 435, y, 100, 28);
        y += 45;
        
        // Mappings display
        mappingsLabel.setBounds(margin, y, 200, 20);
        y += 25;
        
        // Export/Import/Clear buttons at bottom
        int bottomY = getHeight() - 40;
        exportButton.setBounds(margin, bottomY, 100, 28);
        importButton.setBounds(margin + 110, bottomY, 100, 28);
        clearButton.setBounds(margin + 220, bottomY, 100, 28);
        
        // Mappings editor takes remaining space
        int editorHeight = bottomY - y - 10;
        if (editorHeight < 80) editorHeight = 80;
        mappingsEditor.setBounds(margin, y, width, editorHeight);
    }
    
    void buttonClicked(juce::Button* button) override
    {
        if (button == &autoScanButton)
        {
            toggleAutoScan();
        }
        else if (button == &nextAddressButton)
        {
            advanceAddress(1);
        }
        else if (button == &prevAddressButton)
        {
            advanceAddress(-1);
        }
        else if (button == &saveAndNextButton)
        {
            saveCurrentAndAdvance();
        }
        else if (button == &exportButton)
        {
            exportMappings();
        }
        else if (button == &importButton)
        {
            importMappings();
        }
        else if (button == &clearButton)
        {
            clearAllMappings();
        }
        else
        {
            // Address button clicked
            for (int i = 0; i < 64; i++)
            {
                if (button == addressButtons[i].get())
                {
                    selectAddress(i);
                    break;
                }
            }
        }
    }
    
    void sliderValueChanged(juce::Slider* slider) override
    {
        if (slider == &valueSlider)
        {
            sendCurrentMessage();
        }
    }
    
    void timerCallback() override
    {
        // Auto-scan: send current value periodically
        sendCurrentMessage();
    }
    
private:
    MidiHandler& midiHandler;
    int currentAddress = 0;
    bool autoScanning = false;
    juce::String discoveredParams[128];
    
    juce::Label titleLabel;
    juce::Label toneLabel;
    juce::ComboBox toneSelector;
    juce::Label valueLabel;
    juce::Slider valueSlider;
    juce::Label currentAddressLabel;
    
    juce::TextButton autoScanButton;
    juce::TextButton nextAddressButton;
    juce::TextButton prevAddressButton;
    
    juce::Label paramNameLabel;
    juce::TextEditor paramNameInput;
    juce::TextButton saveAndNextButton;
    
    std::unique_ptr<juce::TextButton> addressButtons[64];
    
    juce::Label mappingsLabel;
    juce::TextEditor mappingsEditor;
    
    juce::TextButton exportButton;
    juce::TextButton importButton;
    juce::TextButton clearButton;
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    void setupAddressGrid()
    {
        for (int i = 0; i < 64; i++)
        {
            addressButtons[i] = std::make_unique<juce::TextButton>(juce::String(i));
            addressButtons[i]->addListener(this);
            addressButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404060));
            addAndMakeVisible(addressButtons[i].get());
        }
    }
    
    void selectAddress(int addr)
    {
        if (addr < 0) addr = 0;
        if (addr > 63) addr = 63;
        
        // Update button colors
        for (int i = 0; i < 64; i++)
        {
            if (i == addr)
            {
                addressButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFFF6B00));  // Orange = selected
            }
            else if (!discoveredParams[i].isEmpty())
            {
                addressButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2E7D32));  // Green = discovered
            }
            else
            {
                addressButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404060));  // Default
            }
        }
        
        currentAddress = addr;
        currentAddressLabel.setText("Address: " + juce::String(addr) + " (0x" + 
                                    juce::String::toHexString(addr).toUpperCase().paddedLeft('0', 2) + ")",
                                    juce::dontSendNotification);
        
        // Load existing discovery for this address
        paramNameInput.setText(discoveredParams[addr]);
        
        // Send current value to hear the parameter
        sendCurrentMessage();
    }
    
    void advanceAddress(int delta)
    {
        int newAddr = currentAddress + delta;
        if (newAddr < 0) newAddr = 63;
        if (newAddr > 63) newAddr = 0;
        selectAddress(newAddr);
    }
    
    void saveCurrentAndAdvance()
    {
        // Save current parameter name
        discoveredParams[currentAddress] = paramNameInput.getText().trim();
        
        // Update button color
        if (!discoveredParams[currentAddress].isEmpty())
        {
            addressButtons[currentAddress]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2E7D32));
        }
        
        // Update mappings display
        updateMappingsDisplay();
        
        // Clear input and advance
        paramNameInput.clear();
        advanceAddress(1);
    }
    
    void toggleAutoScan()
    {
        autoScanning = !autoScanning;
        
        if (autoScanning)
        {
            autoScanButton.setButtonText("Stop Auto-Scan");
            autoScanButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFC62828));
            startTimer(200);  // Send value every 200ms
        }
        else
        {
            autoScanButton.setButtonText("Start Auto-Scan");
            autoScanButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2E7D32));
            stopTimer();
        }
    }
    
    void sendCurrentMessage()
    {
        int tone = toneSelector.getSelectedId();
        int value = (int)valueSlider.getValue();
        
        midiHandler.sendToneParameter(tone, currentAddress, value);
    }
    
    void updateMappingsDisplay()
    {
        juce::String text = getDefaultMappingsHeader();
        
        for (int i = 0; i < 64; i++)
        {
            if (!discoveredParams[i].isEmpty())
            {
                text += juce::String(i).paddedLeft(' ', 3) + " (0x" + 
                        juce::String::toHexString(i).toUpperCase().paddedLeft('0', 2) + ") -> " +
                        discoveredParams[i] + "\n";
            }
        }
        
        mappingsEditor.setText(text);
    }
    
    void exportMappings()
    {
        // Build JSON
        juce::String json = "{\n  \"firmware\": \"Vecoven 3.x\",\n  \"date\": \"" + 
                           juce::Time::getCurrentTime().toString(true, false) + "\",\n  \"mappings\": {\n";
        
        bool first = true;
        for (int i = 0; i < 64; i++)
        {
            if (!discoveredParams[i].isEmpty())
            {
                if (!first) json += ",\n";
                json += "    \"" + juce::String(i) + "\": \"" + discoveredParams[i] + "\"";
                first = false;
            }
        }
        
        json += "\n  }\n}\n";
        
        fileChooser = std::make_unique<juce::FileChooser>(
            "Export Mappings",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("vecoven_mapping.json"),
            "*.json");
        
        auto chooseFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
        
        fileChooser->launchAsync(chooseFlags, [this, json](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File{})
            {
                file.replaceWithText(json);
            }
        });
    }
    
    void importMappings()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Import Mappings",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.json");
        
        auto chooseFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        
        fileChooser->launchAsync(chooseFlags, [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File{})
            {
                auto json = juce::JSON::parse(file.loadFileAsString());
                if (json.isObject())
                {
                    auto mappings = json.getProperty("mappings", juce::var());
                    if (mappings.isObject())
                    {
                        auto* obj = mappings.getDynamicObject();
                        if (obj != nullptr)
                        {
                            for (auto& prop : obj->getProperties())
                            {
                                int addr = prop.name.toString().getIntValue();
                                if (addr >= 0 && addr < 128)
                                {
                                    discoveredParams[addr] = prop.value.toString();
                                }
                            }
                            
                            // Update UI
                            for (int i = 0; i < 64; i++)
                            {
                                if (!discoveredParams[i].isEmpty())
                                {
                                    addressButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2E7D32));
                                }
                            }
                            
                            updateMappingsDisplay();
                            selectAddress(currentAddress);
                        }
                    }
                }
            }
        });
    }
    
    void clearAllMappings()
    {
        for (int i = 0; i < 128; i++)
            discoveredParams[i] = "";
        
        for (int i = 0; i < 64; i++)
            addressButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404060));
        
        paramNameInput.clear();
        updateMappingsDisplay();
    }
    
    juce::String getDefaultMappingsHeader()
    {
        return "=== VECOVEN MIDI MAPPING ===\n"
               "Click address -> Move Value slider -> See what changes on synth\n"
               "Type parameter name -> Save & Next\n"
               "----------------------------\n\n";
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiTestPanel)
};
