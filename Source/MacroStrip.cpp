/*
  ==============================================================================

    MacroStrip.cpp
    Horizontal strip with 8 macro controls implementation

  ==============================================================================
*/

#include "MacroStrip.h"

//==============================================================================
// MacroSlider Implementation

MacroSlider::MacroSlider()
    : macroIndex(0), macroControl(nullptr)
{
    // Vertical slider
    addAndMakeVisible(slider);
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setRange(0, 127, 1);
    slider.setValue(64);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.addListener(this);
    
    // Name label
    addAndMakeVisible(nameLabel);
    nameLabel.setText("Macro", juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::FontOptions(11.0f));
    
    // Value label
    addAndMakeVisible(valueLabel);
    valueLabel.setText("64", juce::dontSendNotification);
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    
    // CC label
    addAndMakeVisible(ccLabel);
    ccLabel.setText("CC:--", juce::dontSendNotification);
    ccLabel.setJustificationType(juce::Justification::centred);
    ccLabel.setFont(juce::FontOptions(10.0f));
    ccLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    
    // Setup button (small)
    addAndMakeVisible(setupButton);
    setupButton.setButtonText("[+]");
    setupButton.setTooltip("Setup macro mappings");
    setupButton.onClick = [this]() {
        if (onSetupClick)
            onSetupClick(macroIndex);
    };
}

MacroSlider::~MacroSlider()
{
}

void MacroSlider::setMacroIndex(int index)
{
    macroIndex = index;
}

void MacroSlider::setMacroControl(MacroControl* macro)
{
    macroControl = macro;
    updateLabels();
    
    if (macroControl)
    {
        slider.setValue(macroControl->getValue(), juce::dontSendNotification);
    }
}

void MacroSlider::paint(juce::Graphics& g)
{
    // Draw border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);
    
    // Draw background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(getLocalBounds().reduced(1));
}

void MacroSlider::resized()
{
    auto bounds = getLocalBounds().reduced(1);
    
    // Setup button at top-right
    setupButton.setBounds(bounds.removeFromTop(18).removeFromRight(18));
    
    // Name label at top
    nameLabel.setBounds(bounds.removeFromTop(18));
    
    bounds.removeFromTop(2);
    
    // Slider in middle (most space)
    auto sliderArea = bounds.removeFromTop(bounds.getHeight() - 32);
    slider.setBounds(sliderArea);
    
    bounds.removeFromTop(2);
    
    // Value label
    valueLabel.setBounds(bounds.removeFromTop(16));
    
    // CC label at bottom
    ccLabel.setBounds(bounds);
}

void MacroSlider::sliderValueChanged(juce::Slider* /*slider*/)
{
    int value = static_cast<int>(slider.getValue());
    
    if (macroControl)
    {
        macroControl->setValue(value);
    }
    
    valueLabel.setText(juce::String(value), juce::dontSendNotification);
    
    if (onValueChange)
        onValueChange(macroIndex, value);
}

void MacroSlider::updateLabels()
{
    if (macroControl)
    {
        nameLabel.setText(macroControl->getName(), juce::dontSendNotification);
        valueLabel.setText(juce::String(macroControl->getValue()), juce::dontSendNotification);
        
        int cc = macroControl->getMidiCC();
        if (cc >= 0)
            ccLabel.setText("CC:" + juce::String(cc), juce::dontSendNotification);
        else
            ccLabel.setText("CC:--", juce::dontSendNotification);
    }
}

//==============================================================================
// MacroStrip Implementation

MacroStrip::MacroStrip()
    : macroManager(nullptr), expanded(false)
{
    // Title label
    addAndMakeVisible(titleLabel);
    titleLabel.setText("MACROS", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFaaaaaa));

    // Expand/collapse button
    addAndMakeVisible(expandButton);
    expandButton.setButtonText("[+] Show Macros");
    expandButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0x00000000));
    expandButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF6366f1));
    expandButton.onClick = [this]() { toggleExpanded(); };

    // Help button
    addAndMakeVisible(helpButton);
    helpButton.setButtonText("?");
    helpButton.setTooltip("What are macros?");
    helpButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0x00000000));
    helpButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF888888));
    helpButton.onClick = [this]() { showHelp(); };

    // Create macro sliders (initially hidden)
    for (int i = 0; i < MacroManager::NUM_MACROS; i++)
    {
        addChildComponent(macroSliders[i]);  // Start hidden
        macroSliders[i].setMacroIndex(i);
        
        macroSliders[i].onValueChange = [this](int macroIndex, int value) {
            if (macroManager)
            {
                macroManager->setMacroValue(macroIndex, value);
            }
        };
        
        macroSliders[i].onSetupClick = [this](int macroIndex) {
            if (onSetupMacro)
                onSetupMacro(macroIndex);
        };
    }
}

MacroStrip::~MacroStrip()
{
    if (macroManager)
        macroManager->removeListener(this);
}

void MacroStrip::setMacroManager(MacroManager* manager)
{
    if (macroManager)
        macroManager->removeListener(this);
    
    macroManager = manager;
    
    if (macroManager)
    {
        macroManager->addListener(this);
        updateAllSliders();
    }
}

void MacroStrip::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff1e1e1e));
    
    // Draw a subtle border at bottom when collapsed
    if (!expanded)
    {
        g.setColour(juce::Colour(0xFF3a3a3a));
        g.drawHorizontalLine(getHeight() - 1, 0.0f, (float)getWidth());
    }
}

void MacroStrip::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    
    // Header bar (always visible)
    auto header = bounds.removeFromTop(24);
    titleLabel.setBounds(header.removeFromLeft(60));
    expandButton.setBounds(header.removeFromLeft(120));
    helpButton.setBounds(header.removeFromRight(24));

    // Show/hide sliders based on expanded state
    if (expanded)
    {
        bounds.removeFromTop(4);  // Small gap
        int sliderWidth = juce::jmax(40, bounds.getWidth() / MacroManager::NUM_MACROS);
        
        for (int i = 0; i < MacroManager::NUM_MACROS; i++)
        {
            macroSliders[i].setVisible(true);
            macroSliders[i].setBounds(bounds.removeFromLeft(sliderWidth).reduced(2));
        }
    }
    else
    {
        for (int i = 0; i < MacroManager::NUM_MACROS; i++)
        {
            macroSliders[i].setVisible(false);
        }
    }
}

void MacroStrip::toggleExpanded()
{
    expanded = !expanded;
    expandButton.setButtonText(expanded ? "[-] Hide Macros" : "[+] Show Macros");
    
    // Notify listener
    if (onExpandedChanged)
        onExpandedChanged(expanded);
    
    // Notify parent to resize
    if (auto* parent = getParentComponent())
        parent->resized();
    
    resized();
    repaint();
}

void MacroStrip::setExpanded(bool shouldExpand)
{
    if (expanded != shouldExpand)
    {
        expanded = shouldExpand;
        expandButton.setButtonText(expanded ? "[-] Hide Macros" : "[+] Show Macros");
        resized();
        repaint();
    }
}

void MacroStrip::macroValueChanged(int macroIndex, int newValue)
{
    if (macroIndex >= 0 && macroIndex < MacroManager::NUM_MACROS)
    {
        macroSliders[macroIndex].setMacroControl(&macroManager->getMacro(macroIndex));
    }
}

void MacroStrip::macroMappingsChanged(int macroIndex)
{
    if (macroIndex >= 0 && macroIndex < MacroManager::NUM_MACROS)
    {
        macroSliders[macroIndex].setMacroControl(&macroManager->getMacro(macroIndex));
    }
}

void MacroStrip::updateAllSliders()
{
    if (!macroManager) return;
    
    for (int i = 0; i < MacroManager::NUM_MACROS; i++)
    {
        macroSliders[i].setMacroControl(&macroManager->getMacro(i));
    }
}

void MacroStrip::showHelp()
{
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Macros",
        "Macros let you control multiple parameters together.\n\n"
        "- Click the gear on a macro to choose which parameters it affects.\n"
        "- Move the slider to send those changes.\n"
        "- The CC shown is the external MIDI CC (if assigned).",
        "OK");
}
