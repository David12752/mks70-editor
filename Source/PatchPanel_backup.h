/*
  ==============================================================================
    PatchPanel.h - Complete MKS-70 Patch Panel
    
    Premium design with:
    - Realistic Juno-60 style faders
    - Large bilingual info panel (Hebrew/English)
    - Zoom support
    - Properly sized sections
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"
#include "GlowingSection.h"
#include "StyledSlider.h"
#include "StyledComboBox.h"
#include "Juno60Fader.h"

//==============================================================================
// Parameter Info Database - Bilingual descriptions
//==============================================================================
struct ParameterInfoData {
    juce::String nameEN;
    juce::String nameHE;
    juce::String descriptionEN;
    juce::String descriptionHE;
    juce::String creativeTipEN;
    juce::String creativeTipHE;
    
    static ParameterInfoData getInfo(const juce::String& paramId) {
        // Performance parameters
        if (paramId == "splitPoint") return {
            "Split Point", "נקודת חלוקה",
            "Sets the keyboard split point. Notes below this point play the Lower tone, notes above play the Upper tone.",
            "קובע את נקודת החלוקה של הקלידים. תווים מתחת לנקודה זו מנגנים את הטון התחתון, תווים מעל מנגנים את הטון העליון.",
            "Use split mode to layer bass sounds on the left hand while playing lead on the right. Try C3 (48) for a classic organ-style split.",
            "השתמש במצב חלוקה כדי לשכב צלילי בס ביד שמאל תוך נגינת לידים בימין. נסה C3 (48) לחלוקה קלאסית בסגנון אורגן."
        };
        if (paramId == "balance") return {
            "Balance", "איזון",
            "Controls the volume balance between Upper and Lower tones. 64 = equal mix, 0 = Lower only, 127 = Upper only.",
            "שולט באיזון הווליום בין הטונים העליון והתחתון. 64 = תערובת שווה, 0 = תחתון בלבד, 127 = עליון בלבד.",
            "Automate balance for crossfade effects between two different timbres. Great for evolving pads!",
            "הפעל אוטומציה על האיזון ליצירת אפקטי קרוספייד בין שני גוונים שונים. מעולה לפאדים מתפתחים!"
        };
        if (paramId == "totalVolume") return {
            "Total Volume", "ווליום כללי",
            "Master volume for the entire patch. Does not affect the internal sound - only the output level.",
            "ווליום ראשי לכל הפאץ'. לא משפיע על הצליל הפנימי - רק על רמת הפלט.",
            "Keep at ~100 for consistent levels between patches. Lower for quieter patches in a setlist.",
            "שמור על ~100 לרמות עקביות בין פאצ'ים. הנמך לפאצ'ים שקטים יותר ברשימת השירים."
        };
        if (paramId == "bendRange") return {
            "Bend Range", "טווח עיקול",
            "Sets pitch bend range in semitones (0-12). Affects both Upper and Lower tones.",
            "קובע את טווח עיקול הפיץ' בחצאי טונים (0-12). משפיע על הטונים העליון והתחתון.",
            "Use 2 for subtle vibrato, 12 for dramatic pitch sweeps. For guitar-like bends, try 2-3 semitones.",
            "השתמש ב-2 לוויברטו עדין, 12 לסוויפים דרמטיים. לעיקולים בסגנון גיטרה, נסה 2-3 חצאי טונים."
        };
        if (paramId == "unisonDetune") return {
            "Unison Detune", "דיטון יוניסון",
            "Amount of detuning between voices in Unison mode. Higher values = thicker, chorus-like sound.",
            "כמות הדיטון בין קולות במצב יוניסון. ערכים גבוהים = צליל עבה יותר, דמוי כורוס.",
            "Light detune (10-30) for subtle thickening. Heavy detune (60+) for massive synth leads!",
            "דיטון קל (10-30) לעיבוי עדין. דיטון כבד (60+) ללידים סינטיים מאסיביים!"
        };
        if (paramId == "portaTime") return {
            "Portamento Time", "זמן פורטמנטו",
            "Glide time between notes. 0 = instant pitch change, 127 = very slow glide.",
            "זמן החלקה בין תווים. 0 = שינוי פיץ' מיידי, 127 = החלקה איטית מאוד.",
            "Short times (10-30) for expressive leads. Long times (80+) for dreamy, ambient textures.",
            "זמנים קצרים (10-30) ללידים אקספרסיביים. זמנים ארוכים (80+) לטקסטורות חלומיות ואמביינט."
        };
        if (paramId == "chaseLevel") return {
            "Chase Level", "עומק מרדף",
            "Depth of the stereo auto-pan effect. 0 = no effect, 127 = full left-right panning.",
            "עומק אפקט הפאן האוטומטי הסטריאו. 0 = ללא אפקט, 127 = פאן מלא שמאל-ימין.",
            "Use moderate levels (40-80) for subtle movement. Max level creates dramatic stereo sweeps.",
            "השתמש ברמות מתונות (40-80) לתנועה עדינה. רמה מקסימלית יוצרת סוויפים סטריאו דרמטיים."
        };
        if (paramId == "chaseTime") return {
            "Chase Time", "קצב מרדף",
            "Speed of the stereo chase effect. 0 = fast panning, 127 = slow sweeping.",
            "מהירות אפקט המרדף הסטריאו. 0 = פאן מהיר, 127 = סוויפ איטי.",
            "Sync chase time to your song tempo for rhythmic interest. Fast times work well with stabs!",
            "סנכרן את זמן המרדף לטמפו השיר ליצירת עניין ריתמי. זמנים מהירים עובדים טוב עם סטאבים!"
        };
        // Default
        return {
            "Parameter", "פרמטר",
            "Adjust this parameter to shape your sound.",
            "כוונן פרמטר זה לעיצוב הצליל שלך.",
            "Experiment with different values to discover unique sounds!",
            "נסה ערכים שונים לגילוי צלילים ייחודיים!"
        };
    }
};

//==============================================================================
// Large Info Panel - Bilingual parameter help
//==============================================================================
class LargeInfoPanel : public juce::Component {
public:
    LargeInfoPanel() {
        setOpaque(true);
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Background with gradient
        juce::ColourGradient bgGrad(
            DS::Color::bgCard(), bounds.getX(), bounds.getY(),
            DS::Color::bgCard().darker(0.15f), bounds.getX(), bounds.getBottom(),
            false
        );
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // Top accent bar
        g.setColour(accentColor.withAlpha(0.8f));
        g.fillRoundedRectangle(bounds.removeFromTop(4), 8.0f);
        
        bounds = getLocalBounds().toFloat().reduced(16, 8);
        bounds.removeFromTop(4);
        
        // Parameter name - large
        g.setColour(accentColor);
        g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
        auto nameArea = bounds.removeFromTop(28);
        g.drawText(currentInfo.nameEN + " / " + currentInfo.nameHE, nameArea, juce::Justification::centredLeft);
        
        bounds.removeFromTop(8);
        
        // Split into two columns
        auto leftCol = bounds.removeFromLeft(bounds.getWidth() * 0.5f - 8);
        bounds.removeFromLeft(16);
        auto rightCol = bounds;
        
        // English column
        g.setColour(DS::Color::textPrimary());
        g.setFont(juce::FontOptions(14.0f));
        g.drawFittedText(currentInfo.descriptionEN, leftCol.removeFromTop(60).toNearestInt(), 
                        juce::Justification::topLeft, 4);
        
        leftCol.removeFromTop(8);
        g.setColour(DS::Color::Accent::success());
        g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
        g.drawText("💡 Creative Tip:", leftCol.removeFromTop(18), juce::Justification::topLeft);
        
        g.setColour(DS::Color::textSecondary());
        g.setFont(juce::FontOptions(12.0f));
        g.drawFittedText(currentInfo.creativeTipEN, leftCol.toNearestInt(), 
                        juce::Justification::topLeft, 3);
        
        // Hebrew column (right-to-left)
        g.setColour(DS::Color::textPrimary());
        g.setFont(juce::FontOptions(14.0f));
        g.drawFittedText(currentInfo.descriptionHE, rightCol.removeFromTop(60).toNearestInt(), 
                        juce::Justification::topRight, 4);
        
        rightCol.removeFromTop(8);
        g.setColour(DS::Color::Accent::success());
        g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
        g.drawText(":💡 טיפ יצירתי", rightCol.removeFromTop(18), juce::Justification::topRight);
        
        g.setColour(DS::Color::textSecondary());
        g.setFont(juce::FontOptions(12.0f));
        g.drawFittedText(currentInfo.creativeTipHE, rightCol.toNearestInt(), 
                        juce::Justification::topRight, 3);
    }
    
    void setParameter(const juce::String& paramId, juce::Colour color) {
        currentInfo = ParameterInfoData::getInfo(paramId);
        accentColor = color;
        repaint();
    }
    
private:
    ParameterInfoData currentInfo;
    juce::Colour accentColor = DS::Color::Accent::primary();
};

//==============================================================================
// RealisticPatchFader - Uses Juno60Fader LookAndFeel
//==============================================================================
class RealisticPatchFader : public juce::Component {
public:
    RealisticPatchFader(const juce::String& name, juce::Colour color, const juce::String& paramId = "")
        : labelText(name), accentColor(color), parameterId(paramId)
    {
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        slider.setColour(juce::Slider::textBoxTextColourId, DS::Color::textPrimary());
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider.setLookAndFeel(&faderLookAndFeel);
        addAndMakeVisible(slider);
        
        // Mouse enter/exit for info panel
        slider.onValueChange = [this]() {
            if (onFaderFocused)
                onFaderFocused(parameterId, accentColor);
        };
    }
    
    ~RealisticPatchFader() override {
        slider.setLookAndFeel(nullptr);
    }
    
    void paint(juce::Graphics& g) override {
        // Label at top
        g.setColour(accentColor);
        g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
        g.drawText(labelText, labelBounds, juce::Justification::centred);
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        labelBounds = bounds.removeFromTop(18);
        slider.setBounds(bounds.reduced(2, 0));
    }
    
    void mouseEnter(const juce::MouseEvent&) override {
        if (onFaderFocused)
            onFaderFocused(parameterId, accentColor);
    }
    
    void setRange(double min, double max, double interval = 1.0) {
        slider.setRange(min, max, interval);
    }
    
    void setValue(double val, juce::NotificationType notify = juce::sendNotification) {
        slider.setValue(val, notify);
    }
    
    double getValue() const { return slider.getValue(); }
    juce::Slider& getSlider() { return slider; }
    
    std::function<void(const juce::String&, juce::Colour)> onFaderFocused;
    
private:
    // Realistic fader look and feel
    class RealisticFaderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                              const juce::Slider::SliderStyle /*style*/, juce::Slider& /*slider*/) override
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
            
            // Track dimensions
            const float trackWidth = 10.0f;
            const float trackX = bounds.getCentreX() - trackWidth / 2.0f;
            const float trackY = bounds.getY() + 4.0f;
            const float trackHeight = bounds.getHeight() - 8.0f;
            
            // Slot background
            juce::Rectangle<float> slotBounds(trackX - 3, trackY - 2, trackWidth + 6, trackHeight + 4);
            g.setColour(juce::Colour(0xFF0A0A0A));
            g.fillRoundedRectangle(slotBounds, 3.0f);
            
            // Inner slot
            juce::ColourGradient slotGrad(
                juce::Colour(0xFF151515), slotBounds.getX(), slotBounds.getY(),
                juce::Colour(0xFF252525), slotBounds.getRight(), slotBounds.getY(),
                false
            );
            g.setGradientFill(slotGrad);
            g.fillRoundedRectangle(slotBounds.reduced(1.5f), 2.0f);
            
            // Scale marks
            g.setColour(DS::Color::textSecondary().withAlpha(0.4f));
            for (int i = 0; i <= 10; i++) {
                float yPos = trackY + trackHeight * (float)i / 10.0f;
                float markW = (i == 0 || i == 5 || i == 10) ? 6.0f : 3.0f;
                g.fillRect(trackX - markW - 3, yPos - 0.5f, markW, 1.0f);
                g.fillRect(trackX + trackWidth + 3, yPos - 0.5f, markW, 1.0f);
            }
            
            // Track groove
            juce::Rectangle<float> trackBounds(trackX, trackY, trackWidth, trackHeight);
            g.setColour(juce::Colour(0xFF0D0D0D));
            g.fillRoundedRectangle(trackBounds, 2.0f);
            
            // Value fill
            const float fillTop = sliderPos;
            const float fillBottom = trackY + trackHeight - 2;
            if (fillBottom > fillTop) {
                juce::ColourGradient fillGrad(
                    DS::Color::Accent::cyan().withAlpha(0.2f), trackX, fillTop,
                    DS::Color::Accent::cyan().withAlpha(0.6f), trackX, fillBottom,
                    false
                );
                g.setGradientFill(fillGrad);
                g.fillRoundedRectangle(trackX + 1, fillTop, trackWidth - 2, fillBottom - fillTop, 1.0f);
            }
            
            // Fader cap
            const float capW = 28.0f;
            const float capH = 16.0f;
            const float capX = bounds.getCentreX() - capW / 2.0f;
            const float capY = sliderPos - capH / 2.0f;
            
            // Cap shadow
            g.setColour(juce::Colour(0x30000000));
            g.fillRoundedRectangle(capX, capY + 2, capW, capH, 3.0f);
            
            // Cap body
            juce::ColourGradient capGrad(
                juce::Colour(0xFF555555), capX, capY,
                juce::Colour(0xFF1A1A1A), capX, capY + capH,
                false
            );
            g.setGradientFill(capGrad);
            g.fillRoundedRectangle(capX, capY, capW, capH, 3.0f);
            
            // Highlight
            g.setColour(juce::Colour(0x30FFFFFF));
            g.fillRoundedRectangle(capX + 2, capY + 1, capW - 4, 3.0f, 1.5f);
            
            // Center line
            g.setColour(juce::Colours::white);
            g.fillRect(capX + capW / 2.0f - 0.5f, capY + 3.0f, 1.0f, capH - 6.0f);
            
            // Cap border
            g.setColour(juce::Colour(0xFF0A0A0A));
            g.drawRoundedRectangle(capX, capY, capW, capH, 3.0f, 1.0f);
        }
    };
    
    RealisticFaderLookAndFeel faderLookAndFeel;
    juce::Slider slider;
    juce::String labelText;
    juce::String parameterId;
    juce::Colour accentColor;
    juce::Rectangle<int> labelBounds;
};

//==============================================================================
// PatchPanel - Main panel with realistic faders and info panel
//==============================================================================
class PatchPanel : public juce::Component {
public:
    PatchPanel() {
        // Zoom controls
        zoomInBtn.setButtonText("+");
        zoomInBtn.setTooltip("Zoom In");
        zoomInBtn.onClick = [this]() { setZoom(zoomLevel + 0.1f); };
        addAndMakeVisible(zoomInBtn);
        
        zoomOutBtn.setButtonText("-");
        zoomOutBtn.setTooltip("Zoom Out");
        zoomOutBtn.onClick = [this]() { setZoom(zoomLevel - 0.1f); };
        addAndMakeVisible(zoomOutBtn);
        
        zoomResetBtn.setButtonText("100%");
        zoomResetBtn.setTooltip("Reset Zoom");
        zoomResetBtn.onClick = [this]() { setZoom(1.0f); };
        addAndMakeVisible(zoomResetBtn);
        
        // Style buttons
        auto styleBtn = [](juce::TextButton& btn, juce::Colour col) {
            btn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
            btn.setColour(juce::TextButton::textColourOffId, col);
        };
        styleBtn(zoomInBtn, DS::Color::textPrimary());
        styleBtn(zoomOutBtn, DS::Color::textPrimary());
        styleBtn(zoomResetBtn, DS::Color::textSecondary());
        
        // Info panel at bottom
        infoPanel = std::make_unique<LargeInfoPanel>();
        addAndMakeVisible(infoPanel.get());
        
        // Connect fader focus to info panel
        auto connectFader = [this](RealisticPatchFader& fader) {
            fader.onFaderFocused = [this](const juce::String& paramId, juce::Colour col) {
                infoPanel->setParameter(paramId, col);
            };
        };
        
        // === PATCH NAME ===
        patchNameEditor.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
        patchNameEditor.setJustification(juce::Justification::centred);
        patchNameEditor.setTextToShowWhenEmpty("PATCH NAME", DS::Color::textMuted());
        patchNameEditor.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgCard());
        patchNameEditor.setColour(juce::TextEditor::outlineColourId, DS::Color::Accent::primary().withAlpha(0.5f));
        patchNameEditor.setColour(juce::TextEditor::textColourId, DS::Color::Accent::primary());
        addAndMakeVisible(patchNameEditor);
        
        // === ACTION BUTTONS ===
        auto styleActionBtn = [](juce::TextButton& btn, juce::Colour accent, const juce::String& tip) {
            btn.setColour(juce::TextButton::buttonColourId, accent.withAlpha(0.2f));
            btn.setColour(juce::TextButton::textColourOffId, accent);
            btn.setTooltip(tip);
        };
        
        sendBtn.setButtonText("SEND"); styleActionBtn(sendBtn, DS::Color::Accent::success(), "Send patch to MKS-70");
        requestBtn.setButtonText("REQUEST"); styleActionBtn(requestBtn, DS::Color::Accent::cyan(), "Request from MKS-70");
        initBtn.setButtonText("INIT"); styleActionBtn(initBtn, DS::Color::Accent::warning(), "Initialize patch");
        copyBtn.setButtonText("COPY"); styleActionBtn(copyBtn, DS::Color::Accent::magenta(), "Copy patch");
        pasteBtn.setButtonText("PASTE"); styleActionBtn(pasteBtn, DS::Color::Accent::magenta(), "Paste patch");
        compareHwBtn.setButtonText("COMPARE"); styleActionBtn(compareHwBtn, DS::Color::Accent::warning(), "Compare with hardware");
        writeMemBtn.setButtonText("WRITE"); styleActionBtn(writeMemBtn, DS::Color::Accent::error(), "Write to MKS-70 memory");
        
        addAndMakeVisible(sendBtn);
        addAndMakeVisible(requestBtn);
        addAndMakeVisible(initBtn);
        addAndMakeVisible(copyBtn);
        addAndMakeVisible(pasteBtn);
        addAndMakeVisible(compareHwBtn);
        addAndMakeVisible(writeMemBtn);
        
        // === TONE NAMES ===
        toneANameEditor.setFont(juce::FontOptions(14.0f));
        toneANameEditor.setTextToShowWhenEmpty("Upper Tone", DS::Color::textMuted());
        toneANameEditor.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        toneANameEditor.setColour(juce::TextEditor::textColourId, DS::Color::Section::dco1());
        addAndMakeVisible(toneANameEditor);
        
        toneBNameEditor.setFont(juce::FontOptions(14.0f));
        toneBNameEditor.setTextToShowWhenEmpty("Lower Tone", DS::Color::textMuted());
        toneBNameEditor.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        toneBNameEditor.setColour(juce::TextEditor::textColourId, DS::Color::Section::dco2());
        addAndMakeVisible(toneBNameEditor);
        
        // === SECTIONS ===
        performanceSection = std::make_unique<GlowingSection>("PERFORMANCE", DS::Color::Accent::primary());
        pitchSection = std::make_unique<GlowingSection>("PITCH / ASSIGN", DS::Color::Accent::magenta());
        portamentoSection = std::make_unique<GlowingSection>("PORTAMENTO", DS::Color::Accent::cyan());
        chaseSection = std::make_unique<GlowingSection>("CHASE PLAY", DS::Color::Section::lfo());
        bankSection = std::make_unique<GlowingSection>("BANK / PROGRAM", DS::Color::textSecondary());
        
        addAndMakeVisible(performanceSection.get());
        addAndMakeVisible(pitchSection.get());
        addAndMakeVisible(portamentoSection.get());
        addAndMakeVisible(chaseSection.get());
        addAndMakeVisible(bankSection.get());
        
        // === COMBO BOXES ===
        toneSelCombo = std::make_unique<StyledComboBox>("Tone");
        toneSelCombo->addItems({"Upper", "Lower", "Both"});
        addAndMakeVisible(toneSelCombo.get());
        
        keyModeCombo = std::make_unique<StyledComboBox>("Key Mode");
        keyModeCombo->addItems({"Whole", "Split", "Dual"});
        addAndMakeVisible(keyModeCombo.get());
        
        assignModeCombo = std::make_unique<StyledComboBox>("Assign");
        assignModeCombo->addItems({"Poly 1", "Poly 2", "Unison", "Solo"});
        addAndMakeVisible(assignModeCombo.get());
        
        holdModeCombo = std::make_unique<StyledComboBox>("Hold");
        holdModeCombo->addItems({"Off", "On"});
        addAndMakeVisible(holdModeCombo.get());
        
        portaModeCombo = std::make_unique<StyledComboBox>("Mode");
        portaModeCombo->addItems({"Normal", "Legato"});
        addAndMakeVisible(portaModeCombo.get());
        
        chaseModeCombo = std::make_unique<StyledComboBox>("Mode");
        chaseModeCombo->addItems({"Normal", "Reverse"});
        addAndMakeVisible(chaseModeCombo.get());
        
        bankCombo = std::make_unique<StyledComboBox>("Bank");
        for (int i = 1; i <= 8; ++i)
            bankCombo->getComboBox().addItem("Bank " + juce::String(i), i);
        bankCombo->setSelectedId(1);
        addAndMakeVisible(bankCombo.get());
        
        // === REALISTIC FADERS ===
        auto faderColor1 = DS::Color::Accent::primary();
        auto faderColor2 = DS::Color::Accent::magenta();
        auto faderColor3 = DS::Color::Accent::cyan();
        auto faderColor4 = DS::Color::Section::lfo();
        
        splitPointSlider = std::make_unique<RealisticPatchFader>("SPLIT PT", faderColor1, "splitPoint");
        splitPointSlider->setRange(0, 127, 1);
        splitPointSlider->setValue(60);
        connectFader(*splitPointSlider);
        addAndMakeVisible(splitPointSlider.get());
        
        balanceSlider = std::make_unique<RealisticPatchFader>("BALANCE", faderColor1, "balance");
        balanceSlider->setRange(0, 127, 1);
        balanceSlider->setValue(64);
        connectFader(*balanceSlider);
        addAndMakeVisible(balanceSlider.get());
        
        totalVolumeSlider = std::make_unique<RealisticPatchFader>("VOLUME", faderColor1, "totalVolume");
        totalVolumeSlider->setRange(0, 127, 1);
        totalVolumeSlider->setValue(100);
        connectFader(*totalVolumeSlider);
        addAndMakeVisible(totalVolumeSlider.get());
        
        bendRangeSlider = std::make_unique<RealisticPatchFader>("BEND", faderColor2, "bendRange");
        bendRangeSlider->setRange(0, 12, 1);
        bendRangeSlider->setValue(2);
        connectFader(*bendRangeSlider);
        addAndMakeVisible(bendRangeSlider.get());
        
        unisonDetuneSlider = std::make_unique<RealisticPatchFader>("DETUNE", faderColor2, "unisonDetune");
        unisonDetuneSlider->setRange(0, 127, 1);
        unisonDetuneSlider->setValue(0);
        connectFader(*unisonDetuneSlider);
        addAndMakeVisible(unisonDetuneSlider.get());
        
        portaTimeSlider = std::make_unique<RealisticPatchFader>("TIME", faderColor3, "portaTime");
        portaTimeSlider->setRange(0, 127, 1);
        portaTimeSlider->setValue(0);
        connectFader(*portaTimeSlider);
        addAndMakeVisible(portaTimeSlider.get());
        
        chaseLevelSlider = std::make_unique<RealisticPatchFader>("LEVEL", faderColor4, "chaseLevel");
        chaseLevelSlider->setRange(0, 127, 1);
        chaseLevelSlider->setValue(0);
        connectFader(*chaseLevelSlider);
        addAndMakeVisible(chaseLevelSlider.get());
        
        chaseTimeSlider = std::make_unique<RealisticPatchFader>("TIME", faderColor4, "chaseTime");
        chaseTimeSlider->setRange(0, 127, 1);
        chaseTimeSlider->setValue(64);
        connectFader(*chaseTimeSlider);
        addAndMakeVisible(chaseTimeSlider.get());
        
        programSlider = std::make_unique<RealisticPatchFader>("PROGRAM", DS::Color::textSecondary(), "program");
        programSlider->setRange(1, 64, 1);
        programSlider->setValue(1);
        addAndMakeVisible(programSlider.get());
        
        // === TOGGLE BUTTONS ===
        auto styleToggle = [](juce::TextButton& btn, juce::Colour accent) {
            btn.setClickingTogglesState(true);
            btn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
            btn.setColour(juce::TextButton::buttonOnColourId, accent);
            btn.setColour(juce::TextButton::textColourOffId, DS::Color::textSecondary());
            btn.setColour(juce::TextButton::textColourOnId, DS::Color::textPrimary());
        };
        
        portaSwitch.setButtonText("PORTA ON");
        styleToggle(portaSwitch, DS::Color::Accent::cyan());
        addAndMakeVisible(portaSwitch);
        
        glideSwitch.setButtonText("GLIDE");
        styleToggle(glideSwitch, DS::Color::Accent::cyan());
        addAndMakeVisible(glideSwitch);
        
        chaseSwitch.setButtonText("CHASE ON");
        styleToggle(chaseSwitch, DS::Color::Section::lfo());
        addAndMakeVisible(chaseSwitch);
        
        // Navigation
        prevBtn.setButtonText("< PREV");
        prevBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        addAndMakeVisible(prevBtn);
        
        nextBtn.setButtonText("NEXT >");
        nextBtn.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        addAndMakeVisible(nextBtn);
        
        // Set initial info
        infoPanel->setParameter("balance", DS::Color::Accent::primary());
    }
    
    void setZoom(float newZoom) {
        zoomLevel = juce::jlimit(0.7f, 1.5f, newZoom);
        zoomResetBtn.setButtonText(juce::String(int(zoomLevel * 100)) + "%");
        resized();
        repaint();
    }
    
    float getZoom() const { return zoomLevel; }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(10);
        
        const int gap = 10;
        const int smallGap = 6;
        const int faderH = int(140 * zoomLevel);
        const int comboH = 28;
        const int btnH = 28;
        
        // === TOP: Zoom + Patch Name + Buttons ===
        auto topRow = bounds.removeFromTop(36);
        
        // Zoom controls on the right
        auto zoomArea = topRow.removeFromRight(120);
        zoomOutBtn.setBounds(zoomArea.removeFromLeft(30).reduced(2));
        zoomResetBtn.setBounds(zoomArea.removeFromLeft(50).reduced(2));
        zoomInBtn.setBounds(zoomArea.removeFromLeft(30).reduced(2));
        
        topRow.removeFromRight(gap);
        
        // Patch name
        patchNameEditor.setBounds(topRow.removeFromLeft(200).reduced(0, 2));
        topRow.removeFromLeft(gap);
        
        // Action buttons
        int btnW = 65;
        sendBtn.setBounds(topRow.removeFromLeft(btnW).withHeight(btnH));
        topRow.removeFromLeft(4);
        requestBtn.setBounds(topRow.removeFromLeft(btnW + 10).withHeight(btnH));
        topRow.removeFromLeft(4);
        initBtn.setBounds(topRow.removeFromLeft(50).withHeight(btnH));
        topRow.removeFromLeft(8);
        copyBtn.setBounds(topRow.removeFromLeft(50).withHeight(btnH));
        topRow.removeFromLeft(4);
        pasteBtn.setBounds(topRow.removeFromLeft(55).withHeight(btnH));
        topRow.removeFromLeft(8);
        compareHwBtn.setBounds(topRow.removeFromLeft(btnW + 15).withHeight(btnH));
        topRow.removeFromLeft(4);
        writeMemBtn.setBounds(topRow.removeFromLeft(55).withHeight(btnH));
        
        bounds.removeFromTop(smallGap);
        
        // === TONE NAMES ===
        auto toneRow = bounds.removeFromTop(26);
        int toneW = (toneRow.getWidth() - gap) / 2;
        toneANameEditor.setBounds(toneRow.removeFromLeft(toneW));
        toneRow.removeFromLeft(gap);
        toneBNameEditor.setBounds(toneRow);
        
        bounds.removeFromTop(gap);
        
        // === BOTTOM: Info Panel (large) ===
        infoPanel->setBounds(bounds.removeFromBottom(int(140 * zoomLevel)));
        bounds.removeFromBottom(gap);
        
        // === MAIN CONTENT: 5 Sections ===
        const int sectionW = (bounds.getWidth() - 4 * gap) / 5;
        const int sectionH = faderH + comboH * 2 + 60;
        
        // PERFORMANCE
        auto sec1 = bounds.removeFromLeft(sectionW);
        performanceSection->setBounds(sec1.withHeight(sectionH));
        auto perf = performanceSection->getContentBounds();
        
        toneSelCombo->setBounds(perf.removeFromTop(comboH));
        perf.removeFromTop(4);
        keyModeCombo->setBounds(perf.removeFromTop(comboH));
        perf.removeFromTop(smallGap);
        
        auto perfFaders = perf.removeFromTop(faderH);
        int faderW = (perfFaders.getWidth() - 8) / 3;
        splitPointSlider->setBounds(perfFaders.removeFromLeft(faderW));
        perfFaders.removeFromLeft(4);
        balanceSlider->setBounds(perfFaders.removeFromLeft(faderW));
        perfFaders.removeFromLeft(4);
        totalVolumeSlider->setBounds(perfFaders);
        
        bounds.removeFromLeft(gap);
        
        // PITCH / ASSIGN
        auto sec2 = bounds.removeFromLeft(sectionW);
        pitchSection->setBounds(sec2.withHeight(sectionH));
        auto pitch = pitchSection->getContentBounds();
        
        assignModeCombo->setBounds(pitch.removeFromTop(comboH));
        pitch.removeFromTop(4);
        holdModeCombo->setBounds(pitch.removeFromTop(comboH));
        pitch.removeFromTop(smallGap);
        
        auto pitchFaders = pitch.removeFromTop(faderH);
        int pitchFaderW = (pitchFaders.getWidth() - 4) / 2;
        bendRangeSlider->setBounds(pitchFaders.removeFromLeft(pitchFaderW));
        pitchFaders.removeFromLeft(4);
        unisonDetuneSlider->setBounds(pitchFaders);
        
        bounds.removeFromLeft(gap);
        
        // PORTAMENTO
        auto sec3 = bounds.removeFromLeft(sectionW);
        portamentoSection->setBounds(sec3.withHeight(sectionH));
        auto porta = portamentoSection->getContentBounds();
        
        auto portaSw = porta.removeFromTop(btnH);
        int halfSw = (portaSw.getWidth() - 4) / 2;
        portaSwitch.setBounds(portaSw.removeFromLeft(halfSw));
        portaSw.removeFromLeft(4);
        glideSwitch.setBounds(portaSw);
        
        porta.removeFromTop(4);
        portaModeCombo->setBounds(porta.removeFromTop(comboH));
        porta.removeFromTop(smallGap);
        
        auto portaFader = porta.removeFromTop(faderH);
        portaTimeSlider->setBounds(portaFader.withSizeKeepingCentre(60, faderH));
        
        bounds.removeFromLeft(gap);
        
        // CHASE PLAY
        auto sec4 = bounds.removeFromLeft(sectionW);
        chaseSection->setBounds(sec4.withHeight(sectionH));
        auto chase = chaseSection->getContentBounds();
        
        chaseSwitch.setBounds(chase.removeFromTop(btnH));
        chase.removeFromTop(4);
        chaseModeCombo->setBounds(chase.removeFromTop(comboH));
        chase.removeFromTop(smallGap);
        
        auto chaseFaders = chase.removeFromTop(faderH);
        int chaseFaderW = (chaseFaders.getWidth() - 4) / 2;
        chaseLevelSlider->setBounds(chaseFaders.removeFromLeft(chaseFaderW));
        chaseFaders.removeFromLeft(4);
        chaseTimeSlider->setBounds(chaseFaders);
        
        bounds.removeFromLeft(gap);
        
        // BANK / PROGRAM
        auto sec5 = bounds;
        bankSection->setBounds(sec5.withHeight(sectionH));
        auto bank = bankSection->getContentBounds();
        
        bankCombo->setBounds(bank.removeFromTop(comboH));
        bank.removeFromTop(smallGap);
        
        auto progFader = bank.removeFromTop(faderH);
        programSlider->setBounds(progFader.withSizeKeepingCentre(60, faderH));
        
        bank.removeFromTop(smallGap);
        auto navRow = bank.removeFromTop(btnH);
        int navHalf = (navRow.getWidth() - 4) / 2;
        prevBtn.setBounds(navRow.removeFromLeft(navHalf));
        navRow.removeFromLeft(4);
        nextBtn.setBounds(navRow);
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DS::Color::bg());
    }
    
    // === GETTERS (compatible with existing code) ===
    StyledComboBox& getToneSelectCombo() { return *toneSelCombo; }
    StyledComboBox& getKeyModeCombo() { return *keyModeCombo; }
    RealisticPatchFader& getSplitPointSlider() { return *splitPointSlider; }
    RealisticPatchFader& getBalanceSlider() { return *balanceSlider; }
    RealisticPatchFader& getTotalVolumeSlider() { return *totalVolumeSlider; }
    juce::TextButton& getPortaSwitch() { return portaSwitch; }
    juce::TextButton& getGlideSwitch() { return glideSwitch; }
    RealisticPatchFader& getPortaTimeSlider() { return *portaTimeSlider; }
    StyledComboBox& getPortaModeCombo() { return *portaModeCombo; }
    RealisticPatchFader& getBendRangeSlider() { return *bendRangeSlider; }
    RealisticPatchFader& getUnisonDetuneSlider() { return *unisonDetuneSlider; }
    StyledComboBox& getAssignModeCombo() { return *assignModeCombo; }
    StyledComboBox& getHoldModeCombo() { return *holdModeCombo; }
    juce::TextButton& getChaseSwitch() { return chaseSwitch; }
    RealisticPatchFader& getChaseLevelSlider() { return *chaseLevelSlider; }
    RealisticPatchFader& getChaseTimeSlider() { return *chaseTimeSlider; }
    StyledComboBox& getChaseModeCombo() { return *chaseModeCombo; }
    juce::TextEditor& getPatchNameEditor() { return patchNameEditor; }
    juce::TextEditor& getToneANameEditor() { return toneANameEditor; }
    juce::TextEditor& getToneBNameEditor() { return toneBNameEditor; }
    StyledComboBox& getBankCombo() { return *bankCombo; }
    RealisticPatchFader& getProgramSlider() { return *programSlider; }
    juce::TextButton& getSendBtn() { return sendBtn; }
    juce::TextButton& getRequestBtn() { return requestBtn; }
    juce::TextButton& getInitBtn() { return initBtn; }
    juce::TextButton& getCopyBtn() { return copyBtn; }
    juce::TextButton& getPasteBtn() { return pasteBtn; }
    juce::TextButton& getPrevBtn() { return prevBtn; }
    juce::TextButton& getNextBtn() { return nextBtn; }
    juce::TextButton& getCompareHwBtn() { return compareHwBtn; }
    juce::TextButton& getWriteMemBtn() { return writeMemBtn; }
    LargeInfoPanel& getInfoPanel() { return *infoPanel; }
    
private:
    float zoomLevel = 1.0f;
    
    // Zoom controls
    juce::TextButton zoomInBtn, zoomOutBtn, zoomResetBtn;
    
    // Info panel
    std::unique_ptr<LargeInfoPanel> infoPanel;
    
    // Sections
    std::unique_ptr<GlowingSection> performanceSection, pitchSection;
    std::unique_ptr<GlowingSection> portamentoSection, chaseSection, bankSection;
    
    // Combo boxes
    std::unique_ptr<StyledComboBox> toneSelCombo, keyModeCombo;
    std::unique_ptr<StyledComboBox> assignModeCombo, holdModeCombo;
    std::unique_ptr<StyledComboBox> portaModeCombo, chaseModeCombo;
    std::unique_ptr<StyledComboBox> bankCombo;
    
    // Realistic faders
    std::unique_ptr<RealisticPatchFader> splitPointSlider, balanceSlider, totalVolumeSlider;
    std::unique_ptr<RealisticPatchFader> bendRangeSlider, unisonDetuneSlider;
    std::unique_ptr<RealisticPatchFader> portaTimeSlider;
    std::unique_ptr<RealisticPatchFader> chaseLevelSlider, chaseTimeSlider;
    std::unique_ptr<RealisticPatchFader> programSlider;
    
    // Toggle buttons
    juce::TextButton portaSwitch, glideSwitch, chaseSwitch;
    
    // Text editors
    juce::TextEditor patchNameEditor, toneANameEditor, toneBNameEditor;
    
    // Navigation
    juce::TextButton prevBtn, nextBtn;
    
    // Action buttons
    juce::TextButton sendBtn, requestBtn, initBtn, copyBtn, pasteBtn;
    juce::TextButton compareHwBtn, writeMemBtn;
};
