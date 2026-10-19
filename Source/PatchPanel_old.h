/*
  ==============================================================================
    PatchPanel.h - Complete MKS-70 Patch Panel
    
    All global patch parameters for MKS-70 with Vecoven firmware
    Updated: Vertical faders (5cm), larger text, info boxes
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "GlowingSection.h"
#include "StyledSlider.h"
#include "StyledComboBox.h"

//==============================================================================
// InfoBox - Small description box for each parameter
//==============================================================================
class InfoBox : public juce::Component {
public:
    InfoBox(const juce::String& text, juce::Colour accentColor = DS::Color::textMuted())
        : infoText(text), accent(accentColor) {}
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Background with subtle gradient
        g.setColour(DS::Color::bgCard().darker(0.1f));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Left accent bar
        g.setColour(accent.withAlpha(0.6f));
        g.fillRoundedRectangle(bounds.removeFromLeft(3), 4.0f);
        
        // Text
        g.setColour(DS::Color::textMuted());
        g.setFont(juce::FontOptions(11.0f));
        g.drawFittedText(infoText, getLocalBounds().reduced(8, 2), 
                        juce::Justification::centredLeft, 3, 0.8f);
    }
    
    void setText(const juce::String& text) { infoText = text; repaint(); }
    
private:
    juce::String infoText;
    juce::Colour accent;
};

//==============================================================================
// VerticalFader - Custom 5cm vertical slider with large label
//==============================================================================
class VerticalFader : public juce::Component {
public:
    VerticalFader(const juce::String& name, juce::Colour color, const juce::String& info = "")
        : labelText(name), accentColor(color), infoText(info)
    {
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 22);
        slider.setColour(juce::Slider::trackColourId, color.withAlpha(0.3f));
        slider.setColour(juce::Slider::thumbColourId, color);
        slider.setColour(juce::Slider::backgroundColourId, DS::Color::bgInput());
        slider.setColour(juce::Slider::textBoxTextColourId, DS::Color::textPrimary());
        slider.setColour(juce::Slider::textBoxBackgroundColourId, DS::Color::bgInput());
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(slider);
        
        // Info box
        if (info.isNotEmpty()) {
            infoBox = std::make_unique<InfoBox>(info, color);
            addAndMakeVisible(infoBox.get());
        }
    }
    
    void paint(juce::Graphics& g) override {
        // Label at top - larger font
        g.setColour(accentColor);
        g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
        g.drawText(labelText, labelBounds, juce::Justification::centred);
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        
        // Label at top (26px)
        labelBounds = bounds.removeFromTop(26);
        
        // Info box at bottom if exists (44px)
        if (infoBox) {
            infoBox->setBounds(bounds.removeFromBottom(44).reduced(2, 2));
            bounds.removeFromBottom(4);
        }
        
        // Slider takes remaining space
        slider.setBounds(bounds.reduced(4, 0));
    }
    
    void setRange(double min, double max, double interval = 0) {
        slider.setRange(min, max, interval);
    }
    
    void setValue(double val, juce::NotificationType notify = juce::sendNotification) {
        slider.setValue(val, notify);
    }
    
    double getValue() const { return slider.getValue(); }
    
    juce::Slider& getSlider() { return slider; }
    
    void setTooltip(const juce::String& tip) { slider.setTooltip(tip); }
    
private:
    juce::Slider slider;
    juce::String labelText;
    juce::Colour accentColor;
    juce::String infoText;
    juce::Rectangle<int> labelBounds;
    std::unique_ptr<InfoBox> infoBox;
};

//==============================================================================
// PatchPanel - Main panel with faders layout
//==============================================================================
class PatchPanel : public juce::Component {
public:
    PatchPanel() {
        // Style toggle buttons
        auto styleToggleButton = [](juce::TextButton& btn, juce::Colour accent, const juce::String& tooltip = "") {
            btn.setClickingTogglesState(true);
            btn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
            btn.setColour(juce::TextButton::buttonOnColourId, accent);
            btn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
            btn.setColour(juce::TextButton::textColourOnId, DS::Color::textPrimary());
            if (tooltip.isNotEmpty()) btn.setTooltip(tooltip);
        };
        
        auto styleActionButton = [](juce::TextButton& btn, juce::Colour accent, const juce::String& tooltip = "") {
            btn.setColour(juce::TextButton::buttonColourId, accent.withAlpha(0.2f));
            btn.setColour(juce::TextButton::textColourOffId, accent);
            if (tooltip.isNotEmpty()) btn.setTooltip(tooltip);
        };
        
        // === PATCH NAME HEADER ===
        patchNameEditor.setFont(juce::FontOptions(24.0f).withStyle("Bold"));
        patchNameEditor.setJustification(juce::Justification::centred);
        patchNameEditor.setTextToShowWhenEmpty("PATCH NAME", DS::Color::textMuted());
        patchNameEditor.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgCard());
        patchNameEditor.setColour(juce::TextEditor::outlineColourId, DS::Color::Accent::primary().withAlpha(0.5f));
        patchNameEditor.setColour(juce::TextEditor::textColourId, DS::Color::Accent::primary());
        patchNameEditor.setColour(juce::TextEditor::focusedOutlineColourId, DS::Color::Accent::primary());
        patchNameEditor.setTooltip("Patch name (up to 10 characters)");
        addAndMakeVisible(patchNameEditor);
        
        // === ACTION BUTTONS ===
        sendBtn.setButtonText("SEND ALL");
        styleActionButton(sendBtn, DS::Color::Accent::success(), "Send entire patch to MKS-70");
        addAndMakeVisible(sendBtn);
        
        requestBtn.setButtonText("REQUEST");
        styleActionButton(requestBtn, DS::Color::Accent::cyan(), "Request patch from MKS-70 memory");
        addAndMakeVisible(requestBtn);
        
        initBtn.setButtonText("INIT");
        styleActionButton(initBtn, DS::Color::Accent::warning(), "Reset patch to initial values");
        addAndMakeVisible(initBtn);
        
        copyBtn.setButtonText("COPY");
        styleActionButton(copyBtn, DS::Color::Accent::magenta(), "Copy patch to clipboard");
        addAndMakeVisible(copyBtn);
        
        pasteBtn.setButtonText("PASTE");
        styleActionButton(pasteBtn, DS::Color::Accent::magenta(), "Paste patch from clipboard");
        addAndMakeVisible(pasteBtn);
        
        compareHwBtn.setButtonText("COMPARE");
        styleActionButton(compareHwBtn, DS::Color::Accent::warning(), "Compare with current hardware state");
        addAndMakeVisible(compareHwBtn);
        
        writeMemBtn.setButtonText("WRITE");
        styleActionButton(writeMemBtn, DS::Color::Accent::error(), "Write patch to MKS-70 memory (permanent)");
        addAndMakeVisible(writeMemBtn);
        
        // === TONE NAMES ===
        toneANameEditor.setFont(juce::FontOptions(16.0f));
        toneANameEditor.setTextToShowWhenEmpty("Upper Tone", DS::Color::textMuted());
        toneANameEditor.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        toneANameEditor.setColour(juce::TextEditor::outlineColourId, DS::Color::Section::dco1().withAlpha(0.5f));
        toneANameEditor.setColour(juce::TextEditor::textColourId, DS::Color::Section::dco1());
        toneANameEditor.setTooltip("Upper tone name");
        addAndMakeVisible(toneANameEditor);
        
        toneBNameEditor.setFont(juce::FontOptions(16.0f));
        toneBNameEditor.setTextToShowWhenEmpty("Lower Tone", DS::Color::textMuted());
        toneBNameEditor.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        toneBNameEditor.setColour(juce::TextEditor::outlineColourId, DS::Color::Section::dco2().withAlpha(0.5f));
        toneBNameEditor.setColour(juce::TextEditor::textColourId, DS::Color::Section::dco2());
        toneBNameEditor.setTooltip("Lower tone name");
        addAndMakeVisible(toneBNameEditor);
        
        // === SECTION HEADERS ===
        performanceSection = std::make_unique<GlowingSection>("PERFORMANCE", DS::Color::Accent::primary());
        addAndMakeVisible(performanceSection.get());
        
        pitchSection = std::make_unique<GlowingSection>("PITCH / ASSIGN", DS::Color::Accent::magenta());
        addAndMakeVisible(pitchSection.get());
        
        portamentoSection = std::make_unique<GlowingSection>("PORTAMENTO", DS::Color::Accent::cyan());
        addAndMakeVisible(portamentoSection.get());
        
        chaseSection = std::make_unique<GlowingSection>("CHASE PLAY", DS::Color::Section::lfo());
        addAndMakeVisible(chaseSection.get());
        
        bankSection = std::make_unique<GlowingSection>("BANK / PROGRAM", DS::Color::textSecondary());
        addAndMakeVisible(bankSection.get());
        
        // === COMBO BOXES (larger font) ===
        toneSelCombo = std::make_unique<StyledComboBox>("Tone Select");
        toneSelCombo->addItems({"Upper", "Lower", "Both"});
        toneSelCombo->setTooltip("Select which tone to edit");
        addAndMakeVisible(toneSelCombo.get());
        
        keyModeCombo = std::make_unique<StyledComboBox>("Key Mode");
        keyModeCombo->addItems({"Whole", "Split", "Dual"});
        keyModeCombo->setTooltip("Whole: full keyboard, Split: zones, Dual: layer");
        addAndMakeVisible(keyModeCombo.get());
        
        assignModeCombo = std::make_unique<StyledComboBox>("Assign Mode");
        assignModeCombo->addItems({"Poly 1", "Poly 2", "Unison", "Solo"});
        assignModeCombo->setTooltip("Voice assignment mode");
        addAndMakeVisible(assignModeCombo.get());
        
        holdModeCombo = std::make_unique<StyledComboBox>("Hold");
        holdModeCombo->addItems({"Off", "On"});
        holdModeCombo->setTooltip("Hold notes when keys released");
        addAndMakeVisible(holdModeCombo.get());
        
        portaModeCombo = std::make_unique<StyledComboBox>("Porta Mode");
        portaModeCombo->addItems({"Normal", "Legato"});
        portaModeCombo->setTooltip("Normal: always glide, Legato: overlapping only");
        addAndMakeVisible(portaModeCombo.get());
        
        chaseModeCombo = std::make_unique<StyledComboBox>("Chase Mode");
        chaseModeCombo->addItems({"Normal", "Reverse"});
        chaseModeCombo->setTooltip("Direction of stereo chase");
        addAndMakeVisible(chaseModeCombo.get());
        
        bankCombo = std::make_unique<StyledComboBox>("Bank");
        for (int i = 1; i <= 8; ++i)
            bankCombo->getComboBox().addItem("Bank " + juce::String(i), i);
        bankCombo->getComboBox().setSelectedId(1);
        bankCombo->setTooltip("Select memory bank (1-8)");
        addAndMakeVisible(bankCombo.get());
        
        // === VERTICAL FADERS (5cm = 189px) with info boxes ===
        auto faderColor1 = DS::Color::Accent::primary();
        auto faderColor2 = DS::Color::Accent::magenta();
        auto faderColor3 = DS::Color::Accent::cyan();
        auto faderColor4 = DS::Color::Section::lfo();
        
        // Performance faders
        splitPointSlider = std::make_unique<VerticalFader>("SPLIT PT", faderColor1, 
            "Keyboard split point.\nC4 = 60, C3 = 48");
        splitPointSlider->setRange(0, 127, 1);
        splitPointSlider->setValue(60);
        addAndMakeVisible(splitPointSlider.get());
        
        balanceSlider = std::make_unique<VerticalFader>("BALANCE", faderColor1,
            "Upper/Lower balance.\n64 = equal mix");
        balanceSlider->setRange(0, 127, 1);
        balanceSlider->setValue(64);
        addAndMakeVisible(balanceSlider.get());
        
        totalVolumeSlider = std::make_unique<VerticalFader>("VOLUME", faderColor1,
            "Master patch volume.\n100 = default");
        totalVolumeSlider->setRange(0, 127, 1);
        totalVolumeSlider->setValue(100);
        addAndMakeVisible(totalVolumeSlider.get());
        
        // Pitch faders
        bendRangeSlider = std::make_unique<VerticalFader>("BEND", faderColor2,
            "Pitch bend range.\nIn semitones (0-12)");
        bendRangeSlider->setRange(0, 12, 1);
        bendRangeSlider->setValue(2);
        addAndMakeVisible(bendRangeSlider.get());
        
        unisonDetuneSlider = std::make_unique<VerticalFader>("DETUNE", faderColor2,
            "Unison voice detune.\n0 = no detune");
        unisonDetuneSlider->setRange(0, 127, 1);
        unisonDetuneSlider->setValue(0);
        addAndMakeVisible(unisonDetuneSlider.get());
        
        // Portamento faders
        portaTimeSlider = std::make_unique<VerticalFader>("TIME", faderColor3,
            "Portamento time.\n0 = instant glide");
        portaTimeSlider->setRange(0, 127, 1);
        portaTimeSlider->setValue(0);
        addAndMakeVisible(portaTimeSlider.get());
        
        // Chase faders
        chaseLevelSlider = std::make_unique<VerticalFader>("LEVEL", faderColor4,
            "Chase depth.\n127 = full stereo pan");
        chaseLevelSlider->setRange(0, 127, 1);
        chaseLevelSlider->setValue(0);
        addAndMakeVisible(chaseLevelSlider.get());
        
        chaseTimeSlider = std::make_unique<VerticalFader>("TIME", faderColor4,
            "Chase rate.\n0 = fast, 127 = slow");
        chaseTimeSlider->setRange(0, 127, 1);
        chaseTimeSlider->setValue(64);
        addAndMakeVisible(chaseTimeSlider.get());
        
        // Program fader
        programSlider = std::make_unique<VerticalFader>("PROGRAM", DS::Color::textSecondary(),
            "Program number.\n1-64 per bank");
        programSlider->setRange(1, 64, 1);
        programSlider->setValue(1);
        addAndMakeVisible(programSlider.get());
        
        // === TOGGLE BUTTONS ===
        portaSwitch.setButtonText("PORTA ON");
        styleToggleButton(portaSwitch, DS::Color::Accent::cyan(), "Enable portamento");
        addAndMakeVisible(portaSwitch);
        
        glideSwitch.setButtonText("GLIDE");
        styleToggleButton(glideSwitch, DS::Color::Accent::cyan(), "Constant rate vs constant time");
        addAndMakeVisible(glideSwitch);
        
        chaseSwitch.setButtonText("CHASE ON");
        styleToggleButton(chaseSwitch, DS::Color::Section::lfo(), "Enable stereo auto-pan");
        addAndMakeVisible(chaseSwitch);
        
        // Program navigation
        prevBtn.setButtonText("◀ PREV");
        prevBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        prevBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textPrimary());
        prevBtn.setTooltip("Previous program");
        addAndMakeVisible(prevBtn);
        
        nextBtn.setButtonText("NEXT ▶");
        nextBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        nextBtn.setColour(juce::TextButton::textColourOffId, DS::Color::textPrimary());
        nextBtn.setTooltip("Next program");
        addAndMakeVisible(nextBtn);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(12);
        
        const int gap = 12;
        const int smallGap = 6;
        constexpr int faderHeight = 189;  // 5cm at 96 DPI
        constexpr int faderWidth = 70;
        constexpr int comboH = 32;
        constexpr int toggleH = 30;
        constexpr int btnH = 32;
        constexpr int sectionHeaderH = 28;
        
        // === ROW 1: Patch Name + Action Buttons ===
        auto row1 = bounds.removeFromTop(45);
        patchNameEditor.setBounds(row1.removeFromLeft(250).reduced(0, 4));
        row1.removeFromLeft(gap);
        
        // Action buttons with larger size
        int btnWidth = 80;
        sendBtn.setBounds(row1.removeFromLeft(btnWidth).withHeight(btnH).withY(row1.getY() + 6));
        row1.removeFromLeft(4);
        requestBtn.setBounds(row1.removeFromLeft(btnWidth).withHeight(btnH).withY(row1.getY() + 6));
        row1.removeFromLeft(4);
        initBtn.setBounds(row1.removeFromLeft(60).withHeight(btnH).withY(row1.getY() + 6));
        row1.removeFromLeft(gap);
        copyBtn.setBounds(row1.removeFromLeft(60).withHeight(btnH).withY(row1.getY() + 6));
        row1.removeFromLeft(4);
        pasteBtn.setBounds(row1.removeFromLeft(60).withHeight(btnH).withY(row1.getY() + 6));
        row1.removeFromLeft(gap);
        compareHwBtn.setBounds(row1.removeFromLeft(btnWidth).withHeight(btnH).withY(row1.getY() + 6));
        row1.removeFromLeft(4);
        writeMemBtn.setBounds(row1.removeFromLeft(70).withHeight(btnH).withY(row1.getY() + 6));
        
        bounds.removeFromTop(smallGap);
        
        // === ROW 2: Tone Names ===
        auto row2 = bounds.removeFromTop(30);
        int toneW = (row2.getWidth() - gap) / 2;
        toneANameEditor.setBounds(row2.removeFromLeft(toneW));
        row2.removeFromLeft(gap);
        toneBNameEditor.setBounds(row2);
        
        bounds.removeFromTop(gap);
        
        // === MAIN CONTENT: 5 Sections ===
        // Calculate section width for horizontal layout
        const int numSections = 5;
        const int sectionWidth = (bounds.getWidth() - (numSections - 1) * gap) / numSections;
        
        // === SECTION 1: PERFORMANCE ===
        auto sec1 = bounds.removeFromLeft(sectionWidth);
        performanceSection->setBounds(sec1.removeFromTop(sectionHeaderH + faderHeight + comboH * 2 + gap * 3 + 70));
        auto perf = performanceSection->getContentBounds();
        
        toneSelCombo->setBounds(perf.removeFromTop(comboH));
        perf.removeFromTop(4);
        keyModeCombo->setBounds(perf.removeFromTop(comboH));
        perf.removeFromTop(smallGap);
        
        // 3 faders in row
        auto perfFaders = perf.removeFromTop(faderHeight + 70);  // Extra for info box
        int faderW = (perfFaders.getWidth() - 8) / 3;
        splitPointSlider->setBounds(perfFaders.removeFromLeft(faderW));
        perfFaders.removeFromLeft(4);
        balanceSlider->setBounds(perfFaders.removeFromLeft(faderW));
        perfFaders.removeFromLeft(4);
        totalVolumeSlider->setBounds(perfFaders);
        
        bounds.removeFromLeft(gap);
        
        // === SECTION 2: PITCH / ASSIGN ===
        auto sec2 = bounds.removeFromLeft(sectionWidth);
        pitchSection->setBounds(sec2.removeFromTop(sectionHeaderH + faderHeight + comboH * 2 + gap * 3 + 70));
        auto pitch = pitchSection->getContentBounds();
        
        assignModeCombo->setBounds(pitch.removeFromTop(comboH));
        pitch.removeFromTop(4);
        holdModeCombo->setBounds(pitch.removeFromTop(comboH));
        pitch.removeFromTop(smallGap);
        
        // 2 faders
        auto pitchFaders = pitch.removeFromTop(faderHeight + 70);
        int pitchFaderW = (pitchFaders.getWidth() - 4) / 2;
        bendRangeSlider->setBounds(pitchFaders.removeFromLeft(pitchFaderW));
        pitchFaders.removeFromLeft(4);
        unisonDetuneSlider->setBounds(pitchFaders);
        
        bounds.removeFromLeft(gap);
        
        // === SECTION 3: PORTAMENTO ===
        auto sec3 = bounds.removeFromLeft(sectionWidth);
        portamentoSection->setBounds(sec3.removeFromTop(sectionHeaderH + faderHeight + comboH + toggleH * 2 + gap * 4 + 70));
        auto porta = portamentoSection->getContentBounds();
        
        auto portaSwitchRow = porta.removeFromTop(toggleH);
        int halfSw = (portaSwitchRow.getWidth() - 4) / 2;
        portaSwitch.setBounds(portaSwitchRow.removeFromLeft(halfSw));
        portaSwitchRow.removeFromLeft(4);
        glideSwitch.setBounds(portaSwitchRow);
        
        porta.removeFromTop(4);
        portaModeCombo->setBounds(porta.removeFromTop(comboH));
        porta.removeFromTop(smallGap);
        
        // Center the single fader
        auto portaFaderArea = porta.removeFromTop(faderHeight + 70);
        portaTimeSlider->setBounds(portaFaderArea.withSizeKeepingCentre(faderWidth + 20, faderHeight + 70));
        
        bounds.removeFromLeft(gap);
        
        // === SECTION 4: CHASE PLAY ===
        auto sec4 = bounds.removeFromLeft(sectionWidth);
        chaseSection->setBounds(sec4.removeFromTop(sectionHeaderH + faderHeight + comboH + toggleH + gap * 4 + 70));
        auto chase = chaseSection->getContentBounds();
        
        chaseSwitch.setBounds(chase.removeFromTop(toggleH));
        chase.removeFromTop(4);
        chaseModeCombo->setBounds(chase.removeFromTop(comboH));
        chase.removeFromTop(smallGap);
        
        // 2 faders
        auto chaseFaders = chase.removeFromTop(faderHeight + 70);
        int chaseFaderW = (chaseFaders.getWidth() - 4) / 2;
        chaseLevelSlider->setBounds(chaseFaders.removeFromLeft(chaseFaderW));
        chaseFaders.removeFromLeft(4);
        chaseTimeSlider->setBounds(chaseFaders);
        
        bounds.removeFromLeft(gap);
        
        // === SECTION 5: BANK / PROGRAM ===
        auto sec5 = bounds;
        bankSection->setBounds(sec5.removeFromTop(sectionHeaderH + faderHeight + comboH + toggleH + gap * 4 + 70));
        auto bank = bankSection->getContentBounds();
        
        bankCombo->setBounds(bank.removeFromTop(comboH));
        bank.removeFromTop(smallGap);
        
        // Center program fader
        auto progFaderArea = bank.removeFromTop(faderHeight + 70);
        programSlider->setBounds(progFaderArea.withSizeKeepingCentre(faderWidth + 20, faderHeight + 70));
        
        bank.removeFromTop(smallGap);
        auto navRow = bank.removeFromTop(toggleH);
        int navHalf = (navRow.getWidth() - 4) / 2;
        prevBtn.setBounds(navRow.removeFromLeft(navHalf));
        navRow.removeFromLeft(4);
        nextBtn.setBounds(navRow);
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DS::Color::bg());
    }
    
    // === GETTERS ===
    StyledComboBox& getToneSelectCombo() { return *toneSelCombo; }
    StyledComboBox& getKeyModeCombo() { return *keyModeCombo; }
    VerticalFader& getSplitPointSlider() { return *splitPointSlider; }
    VerticalFader& getBalanceSlider() { return *balanceSlider; }
    VerticalFader& getTotalVolumeSlider() { return *totalVolumeSlider; }
    juce::TextButton& getPortaSwitch() { return portaSwitch; }
    juce::TextButton& getGlideSwitch() { return glideSwitch; }
    VerticalFader& getPortaTimeSlider() { return *portaTimeSlider; }
    StyledComboBox& getPortaModeCombo() { return *portaModeCombo; }
    VerticalFader& getBendRangeSlider() { return *bendRangeSlider; }
    VerticalFader& getUnisonDetuneSlider() { return *unisonDetuneSlider; }
    StyledComboBox& getAssignModeCombo() { return *assignModeCombo; }
    StyledComboBox& getHoldModeCombo() { return *holdModeCombo; }
    juce::TextButton& getChaseSwitch() { return chaseSwitch; }
    VerticalFader& getChaseLevelSlider() { return *chaseLevelSlider; }
    VerticalFader& getChaseTimeSlider() { return *chaseTimeSlider; }
    StyledComboBox& getChaseModeCombo() { return *chaseModeCombo; }
    juce::TextEditor& getPatchNameEditor() { return patchNameEditor; }
    juce::TextEditor& getToneANameEditor() { return toneANameEditor; }
    juce::TextEditor& getToneBNameEditor() { return toneBNameEditor; }
    StyledComboBox& getBankCombo() { return *bankCombo; }
    VerticalFader& getProgramSlider() { return *programSlider; }
    juce::TextButton& getSendBtn() { return sendBtn; }
    juce::TextButton& getRequestBtn() { return requestBtn; }
    juce::TextButton& getInitBtn() { return initBtn; }
    juce::TextButton& getCopyBtn() { return copyBtn; }
    juce::TextButton& getPasteBtn() { return pasteBtn; }
    juce::TextButton& getPrevBtn() { return prevBtn; }
    juce::TextButton& getNextBtn() { return nextBtn; }
    juce::TextButton& getCompareHwBtn() { return compareHwBtn; }
    juce::TextButton& getWriteMemBtn() { return writeMemBtn; }
    
private:
    // Sections
    std::unique_ptr<GlowingSection> performanceSection, portamentoSection;
    std::unique_ptr<GlowingSection> pitchSection, chaseSection, bankSection;
    
    // Combo boxes
    std::unique_ptr<StyledComboBox> toneSelCombo, keyModeCombo;
    std::unique_ptr<StyledComboBox> assignModeCombo, holdModeCombo;
    std::unique_ptr<StyledComboBox> portaModeCombo, chaseModeCombo;
    std::unique_ptr<StyledComboBox> bankCombo;
    
    // Vertical faders (5cm)
    std::unique_ptr<VerticalFader> splitPointSlider, balanceSlider, totalVolumeSlider;
    std::unique_ptr<VerticalFader> bendRangeSlider, unisonDetuneSlider;
    std::unique_ptr<VerticalFader> portaTimeSlider;
    std::unique_ptr<VerticalFader> chaseLevelSlider, chaseTimeSlider;
    std::unique_ptr<VerticalFader> programSlider;
    
    // Toggle buttons
    juce::TextButton portaSwitch, glideSwitch, chaseSwitch;
    
    // Text editors
    juce::TextEditor patchNameEditor, toneANameEditor, toneBNameEditor;
    
    // Navigation buttons
    juce::TextButton prevBtn, nextBtn;
    
    // Action buttons
    juce::TextButton sendBtn, requestBtn, initBtn, copyBtn, pasteBtn;
    juce::TextButton compareHwBtn, writeMemBtn;
};
