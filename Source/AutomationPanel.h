/*
  ==============================================================================
    AutomationPanel.h - Premium Redesigned Automation Panel
    
    Major Upgrade Features:
    - Quick Access buttons for popular parameters
    - Parameter Info Panel with bilingual descriptions
    - Clean category organization with color coding
    - Modern zoom controls with timeline
    - Keyboard shortcuts display
    - Professional DAW-style transport
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AutomationRecorder.h"
#include "AutomationLaneComponent.h"
#include "DesignSystem.h"
#include "GlowingSection.h"
#include "StyledComboBox.h"
#include "StyledSlider.h"
#include <map>

//==============================================================================
// Parameter category colors for visual organization
//==============================================================================
namespace AutomationColors {
    inline juce::Colour dco()      { return juce::Colour(0xFF4FC3F7); }  // Light blue - oscillators
    inline juce::Colour vcf()      { return juce::Colour(0xFFFFB74D); }  // Orange - filter
    inline juce::Colour env()      { return juce::Colour(0xFF81C784); }  // Green - envelopes
    inline juce::Colour lfo()      { return juce::Colour(0xFFBA68C8); }  // Purple - modulation
    inline juce::Colour mixer()    { return juce::Colour(0xFFE57373); }  // Red - mixer/VCA
    inline juce::Colour patch()    { return juce::Colour(0xFF64B5F6); }  // Blue - patch settings
    inline juce::Colour chorus()   { return juce::Colour(0xFF4DB6AC); }  // Teal - effects
}

//==============================================================================
// Quick Parameter Button - Fast access to popular automation targets
//==============================================================================
class QuickParamButton : public juce::Component, public juce::SettableTooltipClient {
public:
    QuickParamButton(const juce::String& name, const juce::String& paramId, 
                     juce::Colour color, const juce::String& tooltip)
        : paramName(name), paramIdentifier(paramId), accentColor(color)
    {
        setInterceptsMouseClicks(true, false);
        setTooltip(tooltip);
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(1);
        
        bool isHovered = isMouseOver();
        bool isSelected = selected;
        
        // Background with gradient
        juce::Colour bgCol = isSelected ? accentColor.withAlpha(0.4f) :
                             (isHovered ? accentColor.withAlpha(0.2f) : DS::Color::bgInput());
        g.setColour(bgCol);
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Accent bar on left when selected
        if (isSelected) {
            g.setColour(accentColor);
            g.fillRoundedRectangle(bounds.getX(), bounds.getY(), 3.0f, bounds.getHeight(), 1.5f);
        }
        
        // Border
        g.setColour(isHovered || isSelected ? accentColor : DS::Color::border());
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        
        // Text
        g.setColour(isSelected ? DS::Color::textPrimary() : 
                   (isHovered ? accentColor : DS::Color::textSecondary()));
        g.setFont(juce::FontOptions(10.0f));
        g.drawText(paramName, bounds.reduced(4, 0), juce::Justification::centred);
    }
    
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
    void mouseUp(const juce::MouseEvent& e) override {
        if (e.mouseWasClicked() && onClick)
            onClick(paramIdentifier, paramName);
    }
    
    void setSelected(bool sel) { selected = sel; repaint(); }
    bool isSelected() const { return selected; }
    const juce::String& getParamId() const { return paramIdentifier; }
    
    std::function<void(const juce::String&, const juce::String&)> onClick;
    
private:
    juce::String paramName;
    juce::String paramIdentifier;
    juce::Colour accentColor;
    bool selected = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuickParamButton)
};

//==============================================================================
// Automation Info Panel - Shows parameter details in Hebrew and English
//==============================================================================
class AutomationInfoPanel : public juce::Component {
public:
    AutomationInfoPanel() {
        setOpaque(true);
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Dark gradient background
        juce::ColourGradient bgGrad(DS::Color::bgCard(), 0, 0,
                                     DS::Color::bg(), 0, bounds.getHeight(), false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, 6.0f);
        
        // Top accent line
        g.setColour(currentAccent);
        g.fillRect(bounds.getX() + 8, bounds.getY(), bounds.getWidth() - 16, 2.0f);
        
        // Border
        g.setColour(DS::Color::border());
        g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
        
        if (currentParamName.isEmpty()) {
            // Empty state message
            g.setColour(DS::Color::textSecondary());
            g.setFont(juce::FontOptions(14.0f).withStyle("Italic"));
            g.drawText("Select a parameter to see details / Bhar parameter lir'ot pratim",
                      bounds, juce::Justification::centred);
            return;
        }
        
        auto content = bounds.reduced(12, 8);
        content.removeFromTop(4);  // Below accent line
        
        // English section (left)
        auto leftCol = content.removeFromLeft(content.getWidth() * 0.5f - 8);
        
        // Parameter name (English)
        g.setColour(currentAccent);
        g.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
        g.drawText(englishName, leftCol.removeFromTop(22), juce::Justification::centredLeft);
        
        // English description
        g.setColour(DS::Color::textSecondary());
        g.setFont(juce::FontOptions(11.0f));
        g.drawFittedText(englishDesc, leftCol.removeFromTop(40).toNearestInt(),
                        juce::Justification::topLeft, 3);
        
        // Tip (English)
        if (englishTip.isNotEmpty()) {
            leftCol.removeFromTop(4);
            g.setColour(DS::Color::Accent::cyan());
            g.setFont(juce::FontOptions(10.0f).withStyle("Italic"));
            g.drawText("Tip: " + englishTip, leftCol.removeFromTop(16), juce::Justification::centredLeft);
        }
        
        // Separator
        content.removeFromLeft(8);
        g.setColour(DS::Color::border());
        g.drawVerticalLine(content.getX() - 4, bounds.getY() + 12, bounds.getBottom() - 8);
        content.removeFromLeft(8);
        
        // Hebrew section (right)
        auto rightCol = content;
        
        // Parameter name (Hebrew)
        g.setColour(currentAccent);
        g.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
        g.drawText(hebrewName, rightCol.removeFromTop(22), juce::Justification::centredRight);
        
        // Hebrew description
        g.setColour(DS::Color::textSecondary());
        g.setFont(juce::FontOptions(11.0f));
        g.drawFittedText(hebrewDesc, rightCol.removeFromTop(40).toNearestInt(),
                        juce::Justification::topRight, 3);
        
        // Tip (Hebrew)
        if (hebrewTip.isNotEmpty()) {
            rightCol.removeFromTop(4);
            g.setColour(DS::Color::Accent::cyan());
            g.setFont(juce::FontOptions(10.0f).withStyle("Italic"));
            g.drawText("Tip: " + hebrewTip, rightCol.removeFromTop(16), juce::Justification::centredRight);
        }
    }
    
    void setParameterInfo(const juce::String& paramId, const juce::String& displayName) {
        currentParamName = paramId;
        lookupParameterInfo(paramId, displayName);
        repaint();
    }
    
    void clear() {
        currentParamName = "";
        englishName = hebrewName = "";
        englishDesc = hebrewDesc = "";
        englishTip = hebrewTip = "";
        currentAccent = DS::Color::Accent::primary();
        repaint();
    }
    
private:
    juce::String currentParamName;
    juce::String englishName, hebrewName;
    juce::String englishDesc, hebrewDesc;
    juce::String englishTip, hebrewTip;
    juce::Colour currentAccent = DS::Color::Accent::primary();
    
    void lookupParameterInfo(const juce::String& paramId, const juce::String& displayName) {
        // Set defaults
        englishName = displayName;
        hebrewName = displayName;
        
        // Lookup detailed info based on parameter type
        // Using ASCII transliteration for Hebrew to avoid encoding issues
        if (paramId.containsIgnoreCase("Cutoff") || paramId.containsIgnoreCase("VCF")) {
            currentAccent = AutomationColors::vcf();
            englishName = "VCF Cutoff";
            hebrewName = "Teder Hitukh Filter";
            englishDesc = "Filter cutoff frequency. Automate this for classic filter sweeps and evolving pads. Higher values = brighter sound.";
            hebrewDesc = "Teder hahitukh shel hafilter. Mumlatz le'otomatzia shel sweepim klassi'im upa-dim mitpathim.";
            englishTip = "Try slow sweeps from 20 to 100 for evolving textures";
            hebrewTip = "Nase sweepim iti'im me-20 le-100 leteksturot mitpathot";
        }
        else if (paramId.containsIgnoreCase("Resonance") || paramId.containsIgnoreCase("Reso")) {
            currentAccent = AutomationColors::vcf();
            englishName = "VCF Resonance";
            hebrewName = "Rezonans Filter";
            englishDesc = "Filter resonance/emphasis. Creates peaks at the cutoff frequency. High values create self-oscillation.";
            hebrewDesc = "Rezonans hafilter. Yotzer shi'im beteder hahitukh. Arakhim gvohim yotzrim oscillatzia atzmit.";
            englishTip = "Automate together with cutoff for acid-style sequences";
            hebrewTip = "Shalev im otomatzia cutoff lesigun acid";
        }
        else if (paramId.containsIgnoreCase("DCO1") || paramId.containsIgnoreCase("Osc1")) {
            currentAccent = AutomationColors::dco();
            englishName = "DCO 1 Parameters";
            hebrewName = "Parametrei Oscillator 1";
            englishDesc = "Oscillator 1 controls. The main tone generator. Automating waveform or tune creates movement.";
            hebrewDesc = "Parametrei oscillator 1. Meholel hatslil harashi. Otomatzia shel gal o teder yotzeret tenu'a.";
            englishTip = "Automate LFO Mod depth for vibrato control";
            hebrewTip = "Shlot be'omek vibrato derekh otomatzia LFO Mod";
        }
        else if (paramId.containsIgnoreCase("DCO2") || paramId.containsIgnoreCase("Osc2")) {
            currentAccent = AutomationColors::dco();
            englishName = "DCO 2 Parameters";
            hebrewName = "Parametrei Oscillator 2";
            englishDesc = "Oscillator 2 controls. Combine with DCO1 for thick sounds. Crossmod creates FM-like timbres.";
            hebrewDesc = "Parametrei oscillator 2. Shiluv im DCO1 yotzer tslil aveh. Crossmod yotzer gvanim domim le-FM.";
            englishTip = "Automate Fine Tune for detuning effects";
            hebrewTip = "Otomatzia shel Fine Tune liyetzirat efektei detune";
        }
        else if (paramId.containsIgnoreCase("LFO")) {
            currentAccent = AutomationColors::lfo();
            englishName = "LFO Parameters";
            hebrewName = "Parametrei LFO";
            englishDesc = "Low Frequency Oscillator for modulation. Controls vibrato, tremolo, and filter wobble.";
            hebrewDesc = "Oscillator teder namukh lemoDulatzia. Sholet bevibrato, tremolo ure'idot filter.";
            englishTip = "Automate Rate for tempo-synced to free-running transitions";
            hebrewTip = "Otomatzia Rate lema'avar bein synkron letenu'a hofshit";
        }
        else if (paramId.containsIgnoreCase("ENV") || paramId.containsIgnoreCase("Attack") || 
                 paramId.containsIgnoreCase("Decay") || paramId.containsIgnoreCase("Sustain") ||
                 paramId.containsIgnoreCase("Release")) {
            currentAccent = AutomationColors::env();
            englishName = "Envelope Parameters";
            hebrewName = "Parametrei Ma'atefa";
            englishDesc = "ADSR envelope controls. Shape how sound evolves over time. ENV1=Filter, ENV2=Amplitude.";
            hebrewDesc = "Parametrei ma'atefa ADSR. Me'atzevim et hitpathut hatslil le'orekh zman. ENV1=Filter, ENV2=Otzma.";
            englishTip = "Automate Attack for plucks to pads transitions";
            hebrewTip = "Otomatzia Attack lema'avar bein pluck lepad";
        }
        else if (paramId.containsIgnoreCase("Mixer") || paramId.containsIgnoreCase("VCA") || 
                 paramId.containsIgnoreCase("Level")) {
            currentAccent = AutomationColors::mixer();
            englishName = "Mixer/Level Controls";
            hebrewName = "Bakrei Mixer/Otzma";
            englishDesc = "Volume and mix controls. Balance oscillators and overall level.";
            hebrewDesc = "Bakrei volume umix. Izun bein oscillatorim ve'otzma klal.";
            englishTip = "Automate for dynamic swells and fades";
            hebrewTip = "Otomatzia legalei dinamika ufade";
        }
        else if (paramId.containsIgnoreCase("Chorus")) {
            currentAccent = AutomationColors::chorus();
            englishName = "Chorus Effect";
            hebrewName = "Efekt Chorus";
            englishDesc = "Classic Roland chorus effect. Three modes: Off, I, II. Essential for that 80s sound.";
            hebrewDesc = "Efekt chorus klassi shel Roland. Shlosha matzavim. Hiyuni letslil shnot ha-80.";
            englishTip = "Switch between modes for section changes";
            hebrewTip = "Hahlef bein matzavim beshinuyei sektziot";
        }
        else if (paramId.containsIgnoreCase("Patch") || paramId.containsIgnoreCase("Balance") ||
                 paramId.containsIgnoreCase("Portamento") || paramId.containsIgnoreCase("Split")) {
            currentAccent = AutomationColors::patch();
            englishName = "Patch Settings";
            hebrewName = "Hagdarot Patch";
            englishDesc = "Performance and patch-level controls. Upper/Lower balance, portamento, key modes.";
            hebrewDesc = "Bakrei bitzu'a urvat patch. Izun elyon/tahton, portamento, matzavei mikladet.";
            englishTip = "Automate Balance for morphing between tones";
            hebrewTip = "Otomatzia Balance lemorfing bein tonim";
        }
        else {
            currentAccent = DS::Color::Accent::primary();
            englishDesc = "Control this parameter to shape your sound dynamically.";
            hebrewDesc = "Shlot beparameter ze le'itzuv dinami shel hatslil.";
            englishTip = "";
            hebrewTip = "";
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationInfoPanel)
};

//==============================================================================
// AutomationPanel - Premium Redesigned with Quick Access and Info Panel
//==============================================================================
class AutomationPanel : public juce::Component, private juce::Timer
{
public:
    AutomationPanel();
    ~AutomationPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    AutomationRecorder& getRecorder() { return recorder; }
    std::function<void(const juce::String& paramId, int value)> onPlaybackValue;

private:
    // === SECTIONS ===
    std::unique_ptr<GlowingSection> transportSection;
    std::unique_ptr<GlowingSection> timingSection;
    std::unique_ptr<GlowingSection> quickAccessSection;  // NEW: Quick parameter access
    std::unique_ptr<GlowingSection> lanesSection;
    std::unique_ptr<GlowingSection> toolsSection;
    std::unique_ptr<GlowingSection> parameterSection;
    
    // (InfoPanel removed - space used for larger Quick Access)

    // === TRANSPORT (DAW style) ===
    juce::TextButton btnRew {"<<"};
    juce::TextButton btnStop;
    juce::TextButton btnPlay;
    juce::TextButton btnRec;
    juce::TextButton btnFwd {">>"};
    juce::Label timeLabel;

    // Tempo / Sync / Loop
    std::unique_ptr<StyledSlider> tempoSlider;
    std::unique_ptr<StyledComboBox> syncMode;
    juce::TextButton loopToggle;

    // === QUICK ACCESS BUTTONS ===
    juce::OwnedArray<QuickParamButton> quickButtons;
    juce::Label quickFilterLabel, quickEnvLabel, quickLfoLabel, quickMixerLabel;
    
    void createQuickAccessButtons();
    void onQuickButtonClicked(const juce::String& paramId, const juce::String& name);

    // === PARAMETER TARGETING ===
    std::unique_ptr<StyledComboBox> targetToneCombo;
    std::unique_ptr<StyledComboBox> targetParamCombo;
    juce::TextButton addTargetBtn;

    // === TIMELINE + LANES ===
    double viewTimeStart = 0.0;
    double viewTimeEnd = 45.0;
    double zoomFactor = 1.0;

    juce::Viewport lanesViewport;
    juce::Component lanesContent;
    juce::OwnedArray<AutomationLaneComponent> laneViews;
    juce::TextButton btnAddLane {"+ Add Lane"};
    
    // Zoom controls
    juce::TextButton btnZoomIn, btnZoomOut, btnZoomFit;
    juce::Label zoomLabel;

    // === EDIT TOOLS ===
    juce::TextButton toolDraw;
    juce::TextButton toolLine;
    juce::TextButton toolCurve;
    juce::TextButton toolSelect;
    juce::TextButton toolErase;
    juce::TextButton toolSmooth;
    juce::TextButton toolQuantize;
    std::unique_ptr<StyledComboBox> snapCombo;
    std::unique_ptr<StyledComboBox> curveTypeCombo;
    juce::TextButton btnCopy;
    juce::TextButton btnPaste;
    juce::TextButton btnClear;
    juce::TextButton btnUndo;
    juce::TextButton btnRedo;
    juce::TextButton btnImport;
    juce::TextButton btnExport;

    // === RECORDER ===
    AutomationRecorder recorder;
    std::map<juce::String, int> lastPlaybackValues;

    // === METHODS ===
    void timerCallback() override;
    void emitPlaybackValues();
    void refreshLaneViews();
    void setAllLaneEditMode(AutomationLaneComponent::EditMode mode);
    void updateTimeLabel();
    void updateLoop();
    void styleTransportButton(juce::TextButton& btn, const juce::String& icon, bool isRecButton = false);
    void styleToolButton(juce::TextButton& btn, const juce::String& text);
    void styleZoomButton(juce::TextButton& btn, const juce::String& text);
    void populateParameterList(int toneIndex);
    void applyCurvePreset(int presetIndex);
    void smoothSelectedLanes();
    void quantizeSelectedLanes();
    void setZoom(double newZoom);
    void zoomToFit();
};
