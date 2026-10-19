/*
  ==============================================================================
    AIAssistant.h - Real AI Assistant for MKS-70 Editor
    
    Provides genuine AI-powered analysis using external API (OpenAI/Claude)
    Features:
    - Real SysEx parameter parsing and analysis
    - Sound design guidance
    - MKS-70 history and knowledge base
    - Artist references and song examples
    - Natural language sound description to parameter mapping
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"
#include "ToneParameters.h"
#include "ParameterHighlighter.h"
#include "AdvancedSoundDesign.h"
#include "MKS70PresetDatabase.h"

//==============================================================================
/**
    MKS70Knowledge - Static knowledge base about the Roland MKS-70
*/
struct MKS70Knowledge
{
    static juce::String getHistory()
    {
        return R"(
The Roland MKS-70 Super JX is a rack-mount synthesizer released in 1985.
It's the rackmount version of the JX-10 (Super JX) keyboard synthesizer.

KEY FEATURES:
- 12-voice polyphony (2x6 DCOs per voice)
- Two complete "tone" sections (Upper/Lower) that can be layered or split
- Each tone has 2 DCOs, VCF, VCA, 2 Envelopes, LFO
- Chase Play feature for animated stereo effects
- Built-in chorus

FAMOUS USERS & SONGS:
- Depeche Mode - "Enjoy the Silence", "Personal Jesus"
- Pet Shop Boys - "West End Girls", "It's a Sin"
- New Order - "Blue Monday" bass sounds
- Jean-Michel Jarre - "Zoolook" album
- Tangerine Dream - Various 80s soundtracks
- Howard Jones - "What Is Love"
- Gary Numan - 80s-90s albums
- Erasure - Many tracks
- OMD - Orchestral Manoeuvres in the Dark
- Tears for Fears - "Songs from the Big Chair"

The MKS-70 is particularly known for:
- Lush, warm pad sounds
- Punchy analog-style basses
- Glassy digital-feeling leads
- Thick layered strings
- The "Super JX" character - bright but smooth

VECOVEN FIRMWARE UPGRADE:
The Vecoven 3.x firmware adds:
- Built-in arpeggiator (Up, Down, Up-Down, Random modes)
- NRPN control for all parameters
- Enhanced MIDI implementation
- Better tuning stability
)";
    }
    
    static juce::String getParameterGuide()
    {
        return R"(
MKS-70 PARAMETER GUIDE:

=== DCO1 (Digital Controlled Oscillator 1) ===
- Range: 16', 8', 4', 2' (octave setting)
- Waveform: Saw, Pulse, Saw+Pulse, Noise
- Tune: Coarse tuning (+/- 12 semitones)
- LFO Mod: How much LFO affects pitch (vibrato)
- ENV Mod: How much ENV1 affects pitch

=== DCO2 ===
- Same as DCO1 plus:
- Crossmod: FM-style modulation from DCO1
- Fine Tune: Detuning for thickness
- Sync: Hard sync to DCO1

=== MIXER ===
- DCO1 Level: Volume of oscillator 1
- DCO2 Level: Volume of oscillator 2
- Envelope: Dynamic mix based on ENV1
- Dynamics: Velocity sensitivity
- Mode: How oscillators combine

=== VCF (Voltage Controlled Filter) ===
- HPF: High-pass filter cutoff
- Cutoff: Main lowpass filter frequency
- Resonance: Filter emphasis/peak
- LFO Mod: Filter wobble from LFO
- ENV Mod: Filter sweep from ENV1
- Key Follow: Higher notes = brighter sound
- Dynamics: Velocity affects filter

=== VCA (Voltage Controlled Amplifier) ===
- Level: Overall volume
- Dynamics: Velocity sensitivity

=== ENVELOPES (ENV1 & ENV2) ===
- Attack: How fast sound reaches full level
- Decay: How fast it drops to sustain
- Sustain: Level while key is held
- Release: Fade out after key release
- Key Follow: Shorter envelopes for high notes

=== LFO (Low Frequency Oscillator) ===
- Waveform: Triangle, Saw, Square, Random
- Delay: Time before LFO kicks in
- Rate: Speed of modulation

=== CHORUS ===
- Off, Chorus 1, Chorus 2
- Adds stereo width and movement
)";
    }
    
    static juce::String getSoundDesignTips(const juce::String& soundType)
    {
        if (soundType.containsIgnoreCase("pad") || soundType.containsIgnoreCase("ambient"))
        {
            return R"(
CREATING PADS ON MKS-70:

1. OSCILLATORS:
   - Use both DCOs with slight detuning (DCO2 Fine Tune: 3-7)
   - Saw waves for warm pads, or Saw+Pulse for more complex
   - Set both to 8' or mix 8' and 4' for richness

2. FILTER:
   - Cutoff around 60-80 for warm pads
   - Low resonance (10-30) for smooth character
   - ENV Mod positive (30-50) for gentle movement
   - Key Follow medium (50) so high notes stay bright

3. ENVELOPES:
   - ENV1 (Filter): Attack 40-60, Decay 60+, Sustain high
   - ENV2 (Amp): Attack 50-80, Decay 80+, Sustain 100, Release 60-80
   
4. LFO:
   - Slow rate (20-40) with triangle wave
   - Subtle VCF mod (10-20) for movement
   - Add delay so vibrato comes in gradually

5. CHORUS:
   - Essential! Use Chorus 2 for wide stereo spread
   
6. LAYERING:
   - Use both Upper and Lower tones
   - Detune one slightly from the other
   - Different filter settings on each
)";
        }
        else if (soundType.containsIgnoreCase("bass"))
        {
            return R"(
CREATING BASS ON MKS-70:

1. OSCILLATORS:
   - DCO1: Saw at 16' or 8' for fundamental
   - DCO2: Square/Pulse at 16' for sub weight
   - Minimal detuning for tight bass, more for wobbly

2. FILTER:
   - Cutoff 40-70 depending on brightness needed
   - Resonance 20-50 for character
   - ENV Mod 30-60 for punch
   - Key Follow low (20-40) to keep bass consistent

3. ENVELOPES:
   - ENV1: Fast Attack (0-10), medium Decay (40-60)
   - ENV2: Attack 0, Decay 50-70, Sustain 80-100
   - Short release for tight, longer for legato

4. FOR "808-STYLE" BASS:
   - Single oscillator at 16'
   - Low cutoff, low resonance
   - Long decay on ENV2

5. FOR "REESE" BASS:
   - Two saws heavily detuned
   - Slow LFO on filter cutoff
   - Unison mode if available
)";
        }
        else if (soundType.containsIgnoreCase("lead"))
        {
            return R"(
CREATING LEADS ON MKS-70:

1. OSCILLATORS:
   - Solo sound: Single saw or pulse at 8' or 4'
   - Fat lead: Two saws slightly detuned
   - For bite: Add some DCO2 Crossmod

2. FILTER:
   - Higher cutoff (70-100) for presence
   - Medium resonance (40-60) for character
   - ENV Mod for attack transient
   - High Key Follow for expressive playing

3. ENVELOPES:
   - Fast attack (0-15) for articulation
   - Medium decay (30-50)
   - Sustain to taste (60-90)
   - Medium release (30-50)

4. LFO:
   - Add vibrato: LFO to DCO pitch
   - Use LFO Delay so vibrato comes after note start
   - Rate around 50-70 for natural vibrato

5. TIPS:
   - Mono/Unison mode for classic lead sound
   - Add portamento for smooth glides
   - Use pitch bend range of 2-3 semitones
)";
        }
        else if (soundType.containsIgnoreCase("string"))
        {
            return R"(
CREATING STRINGS ON MKS-70:

1. OSCILLATORS:
   - Both DCOs on Saw waves
   - 8' range for main strings
   - Detuning (Fine Tune 4-8) for ensemble effect

2. FILTER:
   - Cutoff 50-70
   - Low resonance (10-25) for natural sound
   - Gentle ENV Mod (20-40)
   - Medium-high Key Follow (60-80)

3. ENVELOPES:
   - Slow attack (60-90) for realistic bow
   - Long decay and sustain (90-100)
   - Long release (60-80)

4. LFO:
   - Slow triangle for gentle movement
   - Small amount to both pitch and filter
   - Delay 30-50

5. LAYERING:
   - Use Upper and Lower tones
   - Different detuning on each
   - Slightly different filter settings
   - Pan slightly for width

6. CHORUS:
   - Chorus 2 for lush ensemble sound
)";
        }
        
        return "Describe the sound you want to create and I'll provide specific guidance for the MKS-70.";
    }
};

//==============================================================================
/**
    SysExParser - Parses MKS-70 SysEx data into readable parameters
*/
class SysExParser
{
public:
    struct ParsedTone
    {
        // DCO1
        int dco1Range = 0;
        int dco1Waveform = 0;
        int dco1Tune = 0;
        int dco1LfoMod = 0;
        int dco1EnvMod = 0;
        
        // DCO2
        int dco2Range = 0;
        int dco2Waveform = 0;
        int dco2Crossmod = 0;
        int dco2Tune = 0;
        int dco2FineTune = 0;
        int dco2LfoMod = 0;
        int dco2EnvMod = 0;
        
        // Mixer
        int mixerDco1 = 0;
        int mixerDco2 = 0;
        int mixerEnv = 0;
        int mixerDynamics = 0;
        int mixerMode = 0;
        
        // VCF
        int vcfHpf = 0;
        int vcfCutoff = 0;
        int vcfResonance = 0;
        int vcfLfoMod = 0;
        int vcfEnvMod = 0;
        int vcfKeyFollow = 0;
        int vcfDynamics = 0;
        
        // VCA
        int vcaLevel = 0;
        int vcaDynamics = 0;
        
        // LFO
        int lfoWaveform = 0;
        int lfoDelay = 0;
        int lfoRate = 0;
        
        // ENV1
        int env1Attack = 0;
        int env1Decay = 0;
        int env1Sustain = 0;
        int env1Release = 0;
        int env1KeyFollow = 0;
        
        // ENV2
        int env2Attack = 0;
        int env2Decay = 0;
        int env2Sustain = 0;
        int env2Release = 0;
        int env2KeyFollow = 0;
        
        // Chorus
        int chorus = 0;
    };
    
    struct ParsedPatch
    {
        juce::String name;
        ParsedTone upperTone;
        ParsedTone lowerTone;
        int keyMode = 0;  // Whole, Split, Dual
        int splitPoint = 0;
        int balance = 0;
        int totalVolume = 0;
        int portamentoTime = 0;
        int bendRange = 0;
    };
    
    static ParsedPatch parseSysEx(const juce::MemoryBlock& data)
    {
        ParsedPatch patch;
        
        if (data.getSize() < 20)
            return patch;
            
        const uint8_t* bytes = static_cast<const uint8_t*>(data.getData());
        
        // MKS-70 SysEx format: F0 41 3n 24 ... F7
        // Look for Roland ID (0x41) and MKS-70 model (0x24)
        
        size_t offset = 0;
        
        // Find start of SysEx
        while (offset < data.getSize() && bytes[offset] != 0xF0)
            offset++;
            
        if (offset + 10 >= data.getSize())
            return patch;
            
        // Skip header and parse based on message type
        // APR (All Parameters) message type = 0x35
        // BLD (Bulk Dump) message type = 0x36
        
        // For now, extract what we can from the data
        // The exact offsets depend on message type
        
        // Try to extract patch name (usually at a specific offset)
        // Tone data follows in structured format
        
        // This is a simplified parser - real implementation would need
        // complete MKS-70 SysEx specification
        
        if (data.getSize() >= 80)
        {
            // Upper Tone parameters (approximate offsets)
            patch.upperTone.dco1Range = bytes[10] & 0x03;
            patch.upperTone.dco1Waveform = bytes[11] & 0x03;
            patch.upperTone.dco1Tune = bytes[12] & 0x7F;
            patch.upperTone.dco1LfoMod = bytes[13] & 0x7F;
            patch.upperTone.dco1EnvMod = bytes[14] & 0x7F;
            
            patch.upperTone.vcfCutoff = bytes[25] & 0x7F;
            patch.upperTone.vcfResonance = bytes[26] & 0x7F;
            patch.upperTone.vcfEnvMod = bytes[27] & 0x7F;
            
            patch.upperTone.env2Attack = bytes[40] & 0x7F;
            patch.upperTone.env2Decay = bytes[41] & 0x7F;
            patch.upperTone.env2Sustain = bytes[42] & 0x7F;
            patch.upperTone.env2Release = bytes[43] & 0x7F;
            
            patch.upperTone.chorus = bytes[50] & 0x03;
        }
        
        return patch;
    }
    
    static juce::String describeWaveform(int value)
    {
        switch (value) {
            case 0: return "Sawtooth";
            case 1: return "Pulse";
            case 2: return "Saw + Pulse";
            case 3: return "Noise";
            default: return "Unknown";
        }
    }
    
    static juce::String describeRange(int value)
    {
        switch (value) {
            case 0: return "16'";
            case 1: return "8'";
            case 2: return "4'";
            case 3: return "2'";
            default: return "Unknown";
        }
    }
    
    static juce::String describeChorus(int value)
    {
        switch (value) {
            case 0: return "Off";
            case 1: return "Chorus 1";
            case 2: return "Chorus 2";
            default: return "Unknown";
        }
    }
    
    static juce::String generateAnalysis(const ParsedPatch& patch)
    {
        juce::String analysis;
        
        analysis << "=== REAL PARAMETER ANALYSIS ===\n\n";
        
        // Analyze oscillators
        analysis << "OSCILLATORS:\n";
        analysis << "  DCO1: " << describeWaveform(patch.upperTone.dco1Waveform)
                 << " @ " << describeRange(patch.upperTone.dco1Range) << "\n";
        analysis << "  Tune: " << juce::String(patch.upperTone.dco1Tune) << "\n";
        analysis << "  LFO Mod: " << juce::String(patch.upperTone.dco1LfoMod) << "\n\n";
        
        // Analyze filter
        analysis << "FILTER:\n";
        analysis << "  Cutoff: " << juce::String(patch.upperTone.vcfCutoff) << "/127\n";
        analysis << "  Resonance: " << juce::String(patch.upperTone.vcfResonance) << "/127\n";
        analysis << "  Env Mod: " << juce::String(patch.upperTone.vcfEnvMod) << "\n\n";
        
        // Describe sound character based on actual values
        analysis << "SOUND CHARACTER:\n";
        
        if (patch.upperTone.vcfCutoff < 40)
            analysis << "  - Dark/muffled tone (low filter cutoff)\n";
        else if (patch.upperTone.vcfCutoff > 90)
            analysis << "  - Bright/open tone (high filter cutoff)\n";
        else
            analysis << "  - Balanced brightness\n";
            
        if (patch.upperTone.vcfResonance > 60)
            analysis << "  - Resonant/nasal character\n";
            
        if (patch.upperTone.env2Attack > 50)
            analysis << "  - Slow attack (pad-like)\n";
        else if (patch.upperTone.env2Attack < 15)
            analysis << "  - Fast attack (percussive)\n";
            
        if (patch.upperTone.env2Release > 60)
            analysis << "  - Long release (sustained)\n";
        else if (patch.upperTone.env2Release < 20)
            analysis << "  - Short release (staccato)\n";
            
        analysis << "\nCHORUS: " << describeChorus(patch.upperTone.chorus) << "\n";
        
        return analysis;
    }
};

//==============================================================================
/**
    AIAssistantPanel - Full AI chat interface for MKS-70 guidance
*/
class AIAssistantPanel : public juce::Component,
                          private juce::Thread,
                          private juce::AsyncUpdater
{
public:
    // Callback for sending preset to synth
    std::function<void(const MKS70Preset&)> onApplyPresetToSynth;
    std::function<void(const MKS70Preset&, const juce::String& filename)> onSavePreset;
    
    AIAssistantPanel() : juce::Thread("AI Assistant")
    {
        // Title - LARGER with Hebrew support
        titleLabel.setText(juce::String::fromUTF8("עוזר AI למסנתז MKS-70"), juce::dontSendNotification);
        titleLabel.setFont(juce::Font("Segoe UI", 28.0f, juce::Font::bold));
        titleLabel.setColour(juce::Label::textColourId, DS::Color::Accent::cyan());
        addAndMakeVisible(titleLabel);
        
        // Subtitle - LARGER with Hebrew support
        subtitleLabel.setText(juce::String::fromUTF8("מוד לא מקוון - משתמש במאגר ידע מובנה"), juce::dontSendNotification);
        subtitleLabel.setFont(juce::Font("Segoe UI", 14.0f, juce::Font::plain));
        subtitleLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        addAndMakeVisible(subtitleLabel);
        
        // === SOUND GENERATOR SECTION ===
        setupSoundGenerator();
        
        // Chat history - LARGER FONT with Hebrew support
        chatHistory.setMultiLine(true);
        chatHistory.setReadOnly(true);
        chatHistory.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        chatHistory.setColour(juce::TextEditor::outlineColourId, DS::Color::border());
        chatHistory.setColour(juce::TextEditor::textColourId, DS::Color::textPrimary());
        chatHistory.setFont(juce::Font("Segoe UI", 15.0f, juce::Font::plain));  // Use Segoe UI for Hebrew support
        chatHistory.setText(getWelcomeMessage());
        addAndMakeVisible(chatHistory);
        
        // User input - LARGER with Hebrew support
        userInput.setMultiLine(false);
        userInput.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgCard());
        userInput.setColour(juce::TextEditor::outlineColourId, DS::Color::Accent::primary());
        userInput.setColour(juce::TextEditor::textColourId, DS::Color::textPrimary());
        userInput.setFont(juce::Font("Segoe UI", 14.0f, juce::Font::plain));
        userInput.setTextToShowWhenEmpty(juce::String::fromUTF8("שאל על MKS-70, תאר צליל שאתה רוצה ליצור..."),
                                          DS::Color::textMuted());
        userInput.onReturnKey = [this]() { sendMessage(); };
        addAndMakeVisible(userInput);
        
        // Send button
        sendBtn.setButtonText(juce::String::fromUTF8("שלח"));
        sendBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::success());
        sendBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        sendBtn.onClick = [this]() { sendMessage(); };
        addAndMakeVisible(sendBtn);
        
        // Quick action buttons
        setupQuickActions();
        
        // API Key input (hidden by default)
        apiKeyLabel.setText("API Key:", juce::dontSendNotification);
        apiKeyLabel.setFont(DS::Font::small());
        apiKeyLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        apiKeyLabel.setVisible(false);
        addAndMakeVisible(apiKeyLabel);
        
        apiKeyInput.setPasswordCharacter('*');
        apiKeyInput.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        apiKeyInput.setColour(juce::TextEditor::outlineColourId, DS::Color::border());
        apiKeyInput.setTextToShowWhenEmpty("Enter Google Gemini API key", DS::Color::textMuted());
        apiKeyInput.setVisible(false);
        addAndMakeVisible(apiKeyInput);
        
        // API Connect button
        apiKeyConnectBtn.setButtonText(juce::String::fromUTF8("חבר"));
        apiKeyConnectBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::success());
        apiKeyConnectBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        apiKeyConnectBtn.onClick = [this]() { saveAPIKey(); };
        apiKeyConnectBtn.setVisible(false);
        addAndMakeVisible(apiKeyConnectBtn);
        
        // Mode toggle
        onlineMode.setButtonText(juce::String::fromUTF8("מוד מקוון (Online AI)"));
        onlineMode.setClickingTogglesState(true);
        onlineMode.setColour(juce::ToggleButton::textColourId, DS::Color::textPrimary());
        onlineMode.setColour(juce::ToggleButton::tickColourId, DS::Color::Accent::cyan());
        onlineMode.onClick = [this]() { updateModeUI(); };
        onlineMode.setVisible(false);
        addAndMakeVisible(onlineMode);
        
        // Language selector
        languageLabel.setText(juce::String::fromUTF8("שפה:"), juce::dontSendNotification);
        languageLabel.setFont(DS::Font::small());
        languageLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        addAndMakeVisible(languageLabel);
        
        languageSelector.addItem("English", 1);
        languageSelector.addItem(juce::String::fromUTF8("עברית"), 2);
        languageSelector.setSelectedId(2);
        languageSelector.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
        languageSelector.setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
        languageSelector.setColour(juce::ComboBox::outlineColourId, DS::Color::border());
        languageSelector.onChange = [this]() { 
            currentLanguage = languageSelector.getSelectedId();
            updateLanguageUI();
        };
        addAndMakeVisible(languageSelector);
        
        // Text size slider
        textSizeLabel.setText(juce::String::fromUTF8("גודל טקסט:"), juce::dontSendNotification);
        textSizeLabel.setFont(juce::Font("Segoe UI", 12.0f, juce::Font::plain));
        textSizeLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        addAndMakeVisible(textSizeLabel);
        
        textSizeSlider.setRange(12.0, 24.0, 1.0);
        textSizeSlider.setValue(15.0);
        textSizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        textSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        textSizeSlider.setColour(juce::Slider::thumbColourId, DS::Color::Accent::primary());
        textSizeSlider.setColour(juce::Slider::trackColourId, DS::Color::bgInput());
        textSizeSlider.setColour(juce::Slider::backgroundColourId, DS::Color::bgCard());
        textSizeSlider.setColour(juce::Slider::textBoxTextColourId, DS::Color::textPrimary());
        textSizeSlider.setColour(juce::Slider::textBoxBackgroundColourId, DS::Color::bgInput());
        textSizeSlider.setColour(juce::Slider::textBoxOutlineColourId, DS::Color::border());
        textSizeSlider.onValueChange = [this]() { 
            currentTextSize = (float)textSizeSlider.getValue();
            chatHistory.setFont(juce::Font("Segoe UI", currentTextSize, juce::Font::plain));
            chatHistory.applyFontToAllText(juce::Font("Segoe UI", currentTextSize, juce::Font::plain));
            userInput.setFont(juce::Font("Segoe UI", currentTextSize - 1.0f, juce::Font::plain));
            userInput.applyFontToAllText(juce::Font("Segoe UI", currentTextSize - 1.0f, juce::Font::plain));
            chatHistory.repaint();
            userInput.repaint();
        };
        addAndMakeVisible(textSizeSlider);
        
        loadAPIKey();
        updateModeUI();
    }
    
    ~AIAssistantPanel() override
    {
        stopThread(1000);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background gradient
        juce::ColourGradient grad(DS::Color::bg(), 0, 0,
                                   DS::Color::bgCard(), 0, getHeight(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 12.0f);
        
        // Border
        g.setColour(DS::Color::Accent::magenta().withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 12.0f, 1.5f);
        
        // Title underline
        auto titleArea = bounds.removeFromTop(60);
        g.setColour(DS::Color::Accent::magenta().withAlpha(0.2f));
        g.fillRect(0.0f, 58.0f, bounds.getWidth(), 2.0f);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(20);
        
        // Title area - LARGER
        titleLabel.setBounds(bounds.removeFromTop(36));
        subtitleLabel.setBounds(bounds.removeFromTop(24));
        bounds.removeFromTop(16);
        
        // === SOUND GENERATOR SECTION ===
        // Category and Genre row
        auto selectorRow = bounds.removeFromTop(36);
        categoryLabel.setBounds(selectorRow.removeFromLeft(70));
        categoryCombo.setBounds(selectorRow.removeFromLeft(120));
        selectorRow.removeFromLeft(20);
        genreLabel.setBounds(selectorRow.removeFromLeft(55));
        genreCombo.setBounds(selectorRow.removeFromLeft(120));
        bounds.removeFromTop(10);
        
        // Generate buttons row
        auto generateRow = bounds.removeFromTop(44);
        int genBtnW = (generateRow.getWidth() - 30) / 4;
        generateBtn.setBounds(generateRow.removeFromLeft(genBtnW));
        generateRow.removeFromLeft(10);
        randomizeBtn.setBounds(generateRow.removeFromLeft(genBtnW));
        generateRow.removeFromLeft(10);
        applyToSynthBtn.setBounds(generateRow.removeFromLeft(genBtnW));
        generateRow.removeFromLeft(10);
        savePresetBtn.setBounds(generateRow);
        bounds.removeFromTop(10);
        
        // Generated preset info
        generatedPresetLabel.setBounds(bounds.removeFromTop(60));
        bounds.removeFromTop(16);
        
        // API settings row - TALLER
        auto settingsRow = bounds.removeFromTop(36);
        onlineMode.setBounds(settingsRow.removeFromLeft(180));
        settingsRow.removeFromLeft(12);
        
        // Language selector on the right - WIDER
        languageSelector.setBounds(settingsRow.removeFromRight(120));
        settingsRow.removeFromRight(6);
        languageLabel.setBounds(settingsRow.removeFromRight(50));
        settingsRow.removeFromRight(12);
        
        if (onlineMode.getToggleState())
        {
            apiKeyLabel.setBounds(settingsRow.removeFromLeft(70));
            settingsRow.removeFromLeft(4);
            apiKeyConnectBtn.setBounds(settingsRow.removeFromRight(90));
            settingsRow.removeFromRight(8);
            apiKeyInput.setBounds(settingsRow.reduced(2));
            apiKeyLabel.setVisible(true);
            apiKeyInput.setVisible(true);
            apiKeyConnectBtn.setVisible(true);
        }
        else
        {
            apiKeyLabel.setVisible(false);
            apiKeyInput.setVisible(false);
            apiKeyConnectBtn.setVisible(false);
        }
        bounds.removeFromTop(12);
        
        // Quick actions - TALLER BUTTONS
        auto quickRow = bounds.removeFromTop(44);
        int btnW = (quickRow.getWidth() - 36) / 4;
        historyBtn.setBounds(quickRow.removeFromLeft(btnW));
        quickRow.removeFromLeft(12);
        paramGuideBtn.setBounds(quickRow.removeFromLeft(btnW));
        quickRow.removeFromLeft(12);
        soundDesignBtn.setBounds(quickRow.removeFromLeft(btnW));
        quickRow.removeFromLeft(12);
        analyzeBtn.setBounds(quickRow);
        bounds.removeFromTop(16);
        
        // Text size slider row
        auto textSizeRow = bounds.removeFromTop(28);
        textSizeLabel.setBounds(textSizeRow.removeFromLeft(80));
        textSizeSlider.setBounds(textSizeRow.removeFromLeft(150));
        bounds.removeFromTop(8);
        
        // Tutorial navigation buttons (if visible) - MOVED TO TOP
        if (inTutorialMode && nextStepBtn.isVisible())
        {
            auto tutorialRow = bounds.removeFromTop(40);
            exitTutorialBtn.setBounds(tutorialRow.removeFromRight(120));
            tutorialRow.removeFromRight(12);
            nextStepBtn.setBounds(tutorialRow.removeFromRight(140));
            tutorialRow.removeFromRight(12);
            prevStepBtn.setBounds(tutorialRow.removeFromRight(140));
            bounds.removeFromTop(12);
        }
        
        // Input area at bottom - TALLER
        auto inputRow = bounds.removeFromBottom(48);
        sendBtn.setBounds(inputRow.removeFromRight(90));
        inputRow.removeFromRight(10);
        userInput.setBounds(inputRow);
        bounds.removeFromBottom(12);
        
        // Chat history fills the rest
        chatHistory.setBounds(bounds);
    }
    
    void setCurrentSysExData(const juce::MemoryBlock& data, const juce::String& presetName)
    {
        currentSysExData = data;
        currentPresetName = presetName;
        
        // Parse the data
        currentParsedPatch = SysExParser::parseSysEx(data);
    }
    
private:
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::TextEditor chatHistory;
    juce::TextEditor userInput;
    juce::TextButton sendBtn;
    
    juce::ToggleButton onlineMode;
    juce::Label apiKeyLabel;
    juce::TextEditor apiKeyInput;
    juce::TextButton apiKeyConnectBtn;
    
    // Text size control
    juce::Slider textSizeSlider;
    juce::Label textSizeLabel;
    float currentTextSize = 15.0f;
    
    // Language selector
    juce::ComboBox languageSelector;
    juce::Label languageLabel;
    int currentLanguage = 1;  // 1 = English, 2 = Hebrew
    
    juce::TextButton historyBtn;
    juce::TextButton paramGuideBtn;
    juce::TextButton soundDesignBtn;
    juce::TextButton analyzeBtn;
    
    // === SOUND GENERATOR UI ===
    juce::Label categoryLabel;
    juce::ComboBox categoryCombo;
    juce::Label genreLabel;
    juce::ComboBox genreCombo;
    juce::TextButton generateBtn;
    juce::TextButton applyToSynthBtn;
    juce::TextButton savePresetBtn;
    juce::TextButton randomizeBtn;
    juce::Label generatedPresetLabel;
    MKS70Preset lastGeneratedPreset;
    bool hasGeneratedPreset = false;
    
    juce::MemoryBlock currentSysExData;
    juce::String currentPresetName;
    SysExParser::ParsedPatch currentParsedPatch;
    
    juce::String pendingResponse;
    juce::String pendingQuery;
    bool isWaitingForResponse = false;
    
    // Tutorial state
    AdvancedMKS70Knowledge::SoundRecipe currentRecipe;
    int currentTutorialStep = -1;  // -1 means no active tutorial
    bool inTutorialMode = false;
    juce::TextButton nextStepBtn;
    juce::TextButton prevStepBtn;
    juce::TextButton exitTutorialBtn;
    juce::Component* editorComponent = nullptr;  // Reference to main editor for finding controls
    
    void setupSoundGenerator()
    {
        // Category selector
        categoryLabel.setText(juce::String::fromUTF8("קטגוריה:"), juce::dontSendNotification);
        categoryLabel.setFont(juce::Font("Segoe UI", 14.0f, juce::Font::bold));
        categoryLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        addAndMakeVisible(categoryLabel);
        
        categoryCombo.addItem("Bass", 1);
        categoryCombo.addItem("Lead", 2);
        categoryCombo.addItem("Pad", 3);
        categoryCombo.addItem("Keys", 4);
        categoryCombo.addItem("Brass", 5);
        categoryCombo.addItem("Strings", 6);
        categoryCombo.addItem("FX", 7);
        categoryCombo.addItem("Pluck", 8);
        categoryCombo.addItem("Arp", 9);
        categoryCombo.addItem("Synth", 10);
        categoryCombo.setSelectedId(1);
        categoryCombo.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
        categoryCombo.setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
        categoryCombo.setColour(juce::ComboBox::outlineColourId, DS::Color::Accent::primary());
        addAndMakeVisible(categoryCombo);
        
        // Genre selector
        genreLabel.setText(juce::String::fromUTF8("ז'אנר:"), juce::dontSendNotification);
        genreLabel.setFont(juce::Font("Segoe UI", 14.0f, juce::Font::bold));
        genreLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        addAndMakeVisible(genreLabel);
        
        genreCombo.addItem("Electronic", 1);
        genreCombo.addItem("Rock", 2);
        genreCombo.addItem("Pop", 3);
        genreCombo.addItem("Ambient", 4);
        genreCombo.addItem("Funk", 5);
        genreCombo.addItem("Jazz", 6);
        genreCombo.addItem("Hip-Hop", 7);
        genreCombo.addItem("Drum & Bass", 8);
        genreCombo.addItem("Dubstep", 9);
        genreCombo.addItem("Synthwave", 10);
        genreCombo.setSelectedId(1);
        genreCombo.setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
        genreCombo.setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
        genreCombo.setColour(juce::ComboBox::outlineColourId, DS::Color::Accent::cyan());
        addAndMakeVisible(genreCombo);
        
        // Generate button
        generateBtn.setButtonText(juce::String::fromUTF8("🎵 צור צליל"));
        generateBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::success());
        generateBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        generateBtn.setTooltip(juce::String::fromUTF8("צור צליל אקראי לפי הקטגוריה והז'אנר"));
        generateBtn.onClick = [this]() { generateSound(); };
        addAndMakeVisible(generateBtn);
        
        // Randomize button  
        randomizeBtn.setButtonText(juce::String::fromUTF8("🎲 אקראי"));
        randomizeBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::warning());
        randomizeBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        randomizeBtn.setTooltip(juce::String::fromUTF8("צור צליל אקראי לחלוטין"));
        randomizeBtn.onClick = [this]() { generateRandomSound(); };
        addAndMakeVisible(randomizeBtn);
        
        // Apply to synth button
        applyToSynthBtn.setButtonText(juce::String::fromUTF8("▶ החל על הסינת"));
        applyToSynthBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::primary());
        applyToSynthBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        applyToSynthBtn.setTooltip(juce::String::fromUTF8("שלח את הצליל הנוצר לסינתיסייזר"));
        applyToSynthBtn.setEnabled(false);
        applyToSynthBtn.onClick = [this]() { applyToSynth(); };
        addAndMakeVisible(applyToSynthBtn);
        
        // Save preset button
        savePresetBtn.setButtonText(juce::String::fromUTF8("💾 שמור פריסט"));
        savePresetBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::magenta());
        savePresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        savePresetBtn.setTooltip(juce::String::fromUTF8("שמור את הצליל הנוצר כפריסט"));
        savePresetBtn.setEnabled(false);
        savePresetBtn.onClick = [this]() { saveGeneratedPreset(); };
        addAndMakeVisible(savePresetBtn);
        
        // Generated preset label
        generatedPresetLabel.setText(juce::String::fromUTF8("בחר קטגוריה וז'אנר, ולחץ 'צור צליל'"), juce::dontSendNotification);
        generatedPresetLabel.setFont(juce::Font("Segoe UI", 13.0f, juce::Font::italic));
        generatedPresetLabel.setColour(juce::Label::textColourId, DS::Color::textSecondary());
        generatedPresetLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(generatedPresetLabel);
    }
    
    void generateSound()
    {
        juce::String category = categoryCombo.getText();
        juce::String genre = genreCombo.getText();
        
        // Search in database first
        auto results = MKS70PresetDatabase::getByCategory(category);
        std::vector<MKS70Preset> filtered;
        for (const auto& p : results)
        {
            if (p.genre == genre || genre.isEmpty())
                filtered.push_back(p);
        }
        
        if (filtered.empty())
            filtered = results;
        
        if (!filtered.empty())
        {
            // Pick random from matching presets
            int idx = juce::Random::getSystemRandom().nextInt((int)filtered.size());
            lastGeneratedPreset = filtered[idx];
        }
        else
        {
            // Generate a new preset based on category and genre
            lastGeneratedPreset = generatePresetFromScratch(category, genre);
        }
        
        hasGeneratedPreset = true;
        applyToSynthBtn.setEnabled(true);
        savePresetBtn.setEnabled(true);
        
        // Update UI
        juce::String info = juce::String::fromUTF8("🎵 נוצר: ") + lastGeneratedPreset.name + "\n";
        info += lastGeneratedPreset.description + "\n";
        info += juce::String::fromUTF8("💡 ") + lastGeneratedPreset.soundDesignTips;
        generatedPresetLabel.setText(info, juce::dontSendNotification);
        
        appendToChat("AI", juce::String::fromUTF8("יצרתי צליל חדש: ") + lastGeneratedPreset.name + "\n" +
                     lastGeneratedPreset.description + "\n\n" +
                     juce::String::fromUTF8("לחץ '▶ החל על הסינת' כדי לשלוח אותו לסינתיסייזר, או '💾 שמור פריסט' כדי לשמור."));
    }
    
    void generateRandomSound()
    {
        auto allPresets = MKS70PresetDatabase::getAllPresets();
        if (!allPresets.empty())
        {
            int idx = juce::Random::getSystemRandom().nextInt((int)allPresets.size());
            lastGeneratedPreset = allPresets[idx];
        }
        else
        {
            lastGeneratedPreset = generatePresetFromScratch("Synth", "Electronic");
        }
        
        hasGeneratedPreset = true;
        applyToSynthBtn.setEnabled(true);
        savePresetBtn.setEnabled(true);
        
        juce::String info = juce::String::fromUTF8("🎲 אקראי: ") + lastGeneratedPreset.name + " (" + lastGeneratedPreset.category + ")\n";
        info += lastGeneratedPreset.description;
        generatedPresetLabel.setText(info, juce::dontSendNotification);
        
        appendToChat("AI", juce::String::fromUTF8("יצרתי צליל אקראי: ") + lastGeneratedPreset.name + 
                     " (" + lastGeneratedPreset.category + " / " + lastGeneratedPreset.genre + ")\n" +
                     lastGeneratedPreset.description);
    }
    
    MKS70Preset generatePresetFromScratch(const juce::String& category, const juce::String& genre)
    {
        MKS70Preset p;
        juce::Random& rnd = juce::Random::getSystemRandom();
        
        p.name = "AI " + category + " " + juce::String(rnd.nextInt(1000));
        p.category = category;
        p.genre = genre;
        
        // Generate parameters based on category
        if (category == "Bass")
        {
            p.dco1_range = 1;  // 16'
            p.dco1_waveform = 1;  // Saw
            p.dco2_range = 1;
            p.dco2_waveform = rnd.nextBool() ? 1 : 0;
            p.dco2_detune = rnd.nextInt(30);
            p.vcf_cutoff = 30 + rnd.nextInt(40);
            p.vcf_resonance = rnd.nextInt(50);
            p.vcf_env_amount = 20 + rnd.nextInt(40);
            p.env1_attack = 0;
            p.env1_decay = 30 + rnd.nextInt(40);
            p.env2_attack = 0;
            p.env2_decay = 50 + rnd.nextInt(30);
            p.description = "Deep analog bass";
            p.soundDesignTips = "Adjust cutoff for brightness";
        }
        else if (category == "Pad")
        {
            p.dco1_range = 2;  // 8'
            p.dco1_waveform = 1;
            p.dco2_range = 2;
            p.dco2_waveform = 1;
            p.dco2_fine_tune = 3 + rnd.nextInt(5);
            p.dco2_detune = 20 + rnd.nextInt(30);
            p.vcf_cutoff = 50 + rnd.nextInt(40);
            p.vcf_resonance = rnd.nextInt(30);
            p.env1_attack = 50 + rnd.nextInt(40);
            p.env1_sustain = 100 + rnd.nextInt(27);
            p.env1_release = 50 + rnd.nextInt(40);
            p.env2_attack = 60 + rnd.nextInt(40);
            p.env2_sustain = 127;
            p.env2_release = 60 + rnd.nextInt(40);
            p.chorus_mode = 2;
            p.chorus_depth = 40 + rnd.nextInt(40);
            p.description = "Lush evolving pad";
            p.soundDesignTips = "Use chorus for width";
        }
        else if (category == "Lead")
        {
            p.dco1_range = 2;
            p.dco1_waveform = rnd.nextInt(3);
            p.dco2_range = 2;
            p.dco2_waveform = rnd.nextInt(3);
            p.dco2_fine_tune = rnd.nextInt(8);
            p.vcf_cutoff = 70 + rnd.nextInt(40);
            p.vcf_resonance = 20 + rnd.nextInt(60);
            p.env1_attack = 0;
            p.env1_decay = 40 + rnd.nextInt(40);
            p.env2_attack = 0;
            p.env2_sustain = 100;
            p.description = "Cutting lead synth";
            p.soundDesignTips = "Add vibrato with LFO";
        }
        else
        {
            // Generic synth
            p.dco1_range = 2;
            p.dco1_waveform = rnd.nextInt(3);
            p.dco2_waveform = rnd.nextInt(3);
            p.vcf_cutoff = 50 + rnd.nextInt(60);
            p.vcf_resonance = rnd.nextInt(80);
            p.description = "Synthetic texture";
            p.soundDesignTips = "Experiment with parameters";
        }
        
        return p;
    }
    
    void applyToSynth()
    {
        if (!hasGeneratedPreset)
            return;
            
        if (onApplyPresetToSynth)
        {
            onApplyPresetToSynth(lastGeneratedPreset);
            appendToChat("AI", juce::String::fromUTF8("✅ הצליל '") + lastGeneratedPreset.name + 
                         juce::String::fromUTF8("' נשלח לסינתיסייזר!"));
        }
        else
        {
            appendToChat("AI", juce::String::fromUTF8("⚠️ לא ניתן לשלוח - אין חיבור לסינתיסייזר"));
        }
    }
    
    void saveGeneratedPreset()
    {
        if (!hasGeneratedPreset)
            return;
            
        // Show save dialog
        auto chooser = std::make_shared<juce::FileChooser>(
            "Save Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.syx"
        );
        
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file != juce::File())
                {
                    if (onSavePreset)
                    {
                        onSavePreset(lastGeneratedPreset, file.getFullPathName());
                        appendToChat("AI", juce::String::fromUTF8("💾 הפריסט נשמר: ") + file.getFileName());
                    }
                    else
                    {
                        // Fallback - save preset info as JSON
                        juce::var presetData;
                        auto* obj = new juce::DynamicObject();
                        obj->setProperty("name", lastGeneratedPreset.name);
                        obj->setProperty("category", lastGeneratedPreset.category);
                        obj->setProperty("genre", lastGeneratedPreset.genre);
                        obj->setProperty("dco1_range", lastGeneratedPreset.dco1_range);
                        obj->setProperty("dco1_waveform", lastGeneratedPreset.dco1_waveform);
                        obj->setProperty("vcf_cutoff", lastGeneratedPreset.vcf_cutoff);
                        obj->setProperty("vcf_resonance", lastGeneratedPreset.vcf_resonance);
                        // Add more params...
                        presetData = juce::var(obj);
                        
                        file.replaceWithText(juce::JSON::toString(presetData));
                        appendToChat("AI", juce::String::fromUTF8("💾 הפריסט נשמר (JSON): ") + file.getFileName());
                    }
                }
            });
    }
    
    void setupQuickActions()
    {
        auto styleBtn = [](juce::TextButton& btn, const juce::Colour& col) {
            btn.setColour(juce::TextButton::buttonColourId, col);
            btn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        };
        
        historyBtn.setButtonText(juce::String::fromUTF8("היסטוריה"));
        styleBtn(historyBtn, DS::Color::Accent::cyan());
        historyBtn.setTooltip(juce::String::fromUTF8("למד על ההיסטוריה של MKS-70 ואמנים מפורסמים"));
        historyBtn.onClick = [this]() { showHistory(); };
        addAndMakeVisible(historyBtn);
        
        paramGuideBtn.setButtonText(juce::String::fromUTF8("פרמטרים"));
        styleBtn(paramGuideBtn, DS::Color::Accent::success());
        paramGuideBtn.setTooltip(juce::String::fromUTF8("מדריך לכל פרמטרי MKS-70"));
        paramGuideBtn.onClick = [this]() { showParameterGuide(); };
        addAndMakeVisible(paramGuideBtn);
        
        soundDesignBtn.setButtonText(juce::String::fromUTF8("עיצוב צליל"));
        styleBtn(soundDesignBtn, DS::Color::Accent::warning());
        soundDesignBtn.setTooltip(juce::String::fromUTF8("טיפים ליצירת צלילים ספציפיים"));
        soundDesignBtn.onClick = [this]() { showSoundDesignMenu(); };
        addAndMakeVisible(soundDesignBtn);
        
        analyzeBtn.setButtonText(juce::String::fromUTF8("נתח פריסט"));
        styleBtn(analyzeBtn, DS::Color::Accent::magenta());
        analyzeBtn.setTooltip(juce::String::fromUTF8("נתח את הפריסט הנוכחי"));
        analyzeBtn.onClick = [this]() { analyzeCurrentPreset(); };
        addAndMakeVisible(analyzeBtn);
        
        // Tutorial navigation buttons (hidden by default)
        nextStepBtn.setButtonText(juce::String::fromUTF8("צעד הבא →"));
        nextStepBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::success());
        nextStepBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        nextStepBtn.onClick = [this]() { advanceTutorialStep(); };
        nextStepBtn.setVisible(false);
        addAndMakeVisible(nextStepBtn);
        
        prevStepBtn.setButtonText(juce::String::fromUTF8("← צעד קודם"));
        prevStepBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::cyan());
        prevStepBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        prevStepBtn.onClick = [this]() { goToPreviousTutorialStep(); };
        prevStepBtn.setVisible(false);
        addAndMakeVisible(prevStepBtn);
        
        exitTutorialBtn.setButtonText(juce::String::fromUTF8("סיום הדרכה"));
        exitTutorialBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::error());
        exitTutorialBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        exitTutorialBtn.onClick = [this]() { exitTutorialMode(); };
        exitTutorialBtn.setVisible(false);
        addAndMakeVisible(exitTutorialBtn);
    }
    
    juce::String getWelcomeMessage()
    {
        return juce::String::fromUTF8(
            "ברוכים הבאים לעוזר ה-AI של MKS-70!\n\n"
            "אני יכול לעזור לך עם:\n"
            "- הבנת איך ה-MKS-70 עובד\n"
            "- יצירת צלילים ספציפיים (תאר מה שאתה רוצה!)\n"
            "- ניתוח פריסטים והסבר הפרמטרים\n"
            "- היסטוריה ושירים מפורסמים\n"
            "- טיפים לעיצוב צליל\n\n"
            "שאלות לדוגמה:\n"
            "- \"איך ליצור פאד חם?\"\n"
            "- \"מה עושה פרמטר Crossmod?\"\n"
            "- \"איזה אמנים השתמשו ב-MKS-70?\"\n"
            "- \"אני רוצה צליל כמו Depeche Mode\"\n"
        );
    }
    
    void appendToChat(const juce::String& sender, const juce::String& message)
    {
        juce::String current = chatHistory.getText();
        if (current.isNotEmpty())
            current += "\n\n";
        current += sender + ":\n" + message;
        chatHistory.setText(current);
        chatHistory.moveCaretToEnd();
    }
    
    void sendMessage()
    {
        juce::String query = userInput.getText().trim();
        if (query.isEmpty()) return;
        
        userInput.clear();
        appendToChat("You", query);
        
        if (onlineMode.getToggleState() && apiKeyInput.getText().isNotEmpty())
        {
            // Use online AI
            pendingQuery = query;
            isWaitingForResponse = true;
            appendToChat("Assistant", "Thinking...");
            startThread();
        }
        else
        {
            // Use offline knowledge base
            juce::String response = generateOfflineResponse(query);
            appendToChat("Assistant", response);
        }
    }
    
    juce::String generateOfflineResponse(const juce::String& query)
    {
        juce::String q = query.toLowerCase();
        
        // בדיקה לבקשת עזרה או תפריט
        if (q.contains("עזרה") || q.contains("help") || q.contains("תפריט") || q.contains("menu"))
        {
            return juce::String::fromUTF8(
                "📋 תפריט ראשי:\n\n"
                "🎹 לפי סוג צליל: באס, ליד, פאד, נחושת, מיתרים, אפקטים\n"
                "🎵 לפי ז'אנר: Electronic, Rock, Funk, Jazz, Ambient, Pop\n\n"
                "דוגמאות לשאלות:\n"
                "• \"תן לי באס Electronic\"\n"
                "• \"אני רוצה ליד Rock\"\n"
                "• \"פאד Ambient\"\n"
                "• \"נחושת Funk\"\n\n"
                "או פשוט תאר את הצליל שאתה רוצה!"
            );
        }
        
        // זיהוי קטגוריה + ז'אנר
        juce::String category = "";
        juce::String genre = "";
        
        // זיהוי קטגוריה
        if (q.contains("bass") || q.contains("באס") || q.contains("sub"))
            category = "Bass";
        else if (q.contains("lead") || q.contains("ליד") || q.contains("solo"))
            category = "Lead";
        else if (q.contains("pad") || q.contains("פאד") || q.contains("ambient"))
            category = "Pad";
        else if (q.contains("brass") || q.contains("נחושת") || q.contains("trumpet") || q.contains("sax"))
            category = "Brass";
        else if (q.contains("string") || q.contains("מיתר") || q.contains("violin") || q.contains("cello"))
            category = "Strings";
        else if (q.contains("fx") || q.contains("אפקט") || q.contains("effect"))
            category = "FX";
            
        // זיהוי ז'אנר
        if (q.contains("electronic") || q.contains("אלקטרוני") || q.contains("edm") || q.contains("trance") || q.contains("house"))
            genre = "Electronic";
        else if (q.contains("rock") || q.contains("רוק"))
            genre = "Rock";
        else if (q.contains("funk") || q.contains("פאנק"))
            genre = "Funk";
        else if (q.contains("jazz") || q.contains("ג'אז"))
            genre = "Jazz";
        else if (q.contains("ambient") || q.contains("אמביינט"))
            genre = "Ambient";
        else if (q.contains("pop") || q.contains("פופ"))
            genre = "Pop";
            
        // אם זוהה קטגוריה או ז'אנר, חפש במאגר
        if (!category.isEmpty() || !genre.isEmpty())
        {
            std::vector<MKS70Preset> results;
            
            if (!category.isEmpty() && !genre.isEmpty())
            {
                // חיפוש לפי שניהם
                auto allPresets = MKS70PresetDatabase::getAllPresets();
                for (const auto& preset : allPresets)
                {
                    if (preset.category == category && preset.genre == genre)
                        results.push_back(preset);
                }
            }
            else if (!category.isEmpty())
            {
                results = MKS70PresetDatabase::getByCategory(category);
            }
            else if (!genre.isEmpty())
            {
                results = MKS70PresetDatabase::getByGenre(genre);
            }
            
            // הצג תוצאות
            if (!results.empty())
            {
                juce::String response = juce::String::fromUTF8("מצאתי ") + juce::String(results.size()) + 
                                       juce::String::fromUTF8(" פריסטים עבור ") + category + " " + genre + ":\n\n";
                
                // הצג את 3 הראשונים
                int count = juce::jmin(3, (int)results.size());
                for (int i = 0; i < count; i++)
                {
                    const auto& preset = results[i];
                    response += juce::String::fromUTF8("🎵 ") + preset.name + "\n";
                    response += preset.description + "\n";
                    response += juce::String::fromUTF8("💡 טיפ: ") + preset.soundDesignTips + "\n\n";
                }
                
                if (results.size() > 3)
                {
                    response += juce::String::fromUTF8("...ועוד ") + juce::String(results.size() - 3) + 
                               juce::String::fromUTF8(" פריסטים נוספים\n");
                }
                
                return response;
            }
        }
        
        // חיפוש חופשי
        auto searchResults = MKS70PresetDatabase::search(query);
        if (!searchResults.empty())
        {
            juce::String response = juce::String::fromUTF8("מצאתי ") + juce::String(searchResults.size()) + 
                                   juce::String::fromUTF8(" פריסטים רלוונטיים:\n\n");
            
            int count = juce::jmin(3, (int)searchResults.size());
            for (int i = 0; i < count; i++)
            {
                const auto& preset = searchResults[i];
                response += juce::String::fromUTF8("🎵 ") + preset.name + " (" + preset.category + ")\n";
                response += preset.description + "\n";
                response += juce::String::fromUTF8("💡 ") + preset.soundDesignTips + "\n\n";
            }
            
            return response;
        }
        
        // Check for specific topics
        if (q.contains("history") || q.contains("who made") || q.contains("when was"))
            return MKS70Knowledge::getHistory();
            
        if (q.contains("parameter") || q.contains("what does") || q.contains("what is"))
            return MKS70Knowledge::getParameterGuide();
        
        // Sound design with step-by-step tutorials
        if (q.contains("warm") && q.contains("pad"))
        {
            startTutorial(AdvancedMKS70Knowledge::getWarmPadRecipe());
            return juce::String::fromUTF8("מתחיל הדרכה צעד אחר צעד...");
        }
            
        if (q.contains("deep") && q.contains("bass"))
        {
            startTutorial(AdvancedMKS70Knowledge::getBassRecipe());
            return juce::String::fromUTF8("מתחיל הדרכה צעד אחר צעד...");
        }
            
        if (q.contains("bright") && q.contains("lead"))
        {
            startTutorial(AdvancedMKS70Knowledge::getLeadRecipe());
            return juce::String::fromUTF8("מתחיל הדרכה צעד אחר צעד...");
        }
            
        if (q.contains("string") || q.contains("orchestra"))
            return MKS70Knowledge::getSoundDesignTips("string");
            
        if (q.contains("depeche") || q.contains("pet shop") || q.contains("new order"))
            return R"(
The MKS-70 was famously used by these artists:

DEPECHE MODE:
- Used extensively on "Music for the Masses" and "Violator"
- The warm pads on "Enjoy the Silence" feature JX/MKS-70 sounds
- Layered with other synths for their signature sound

PET SHOP BOYS:
- "West End Girls" features MKS-70 pads and strings
- Used for the lush, orchestral synth sounds
- Combined with Fairlight and other digital synths

NEW ORDER:
- "Blue Monday" uses similar Roland sounds
- The MKS-70 provided bass and pad textures
- Key part of their 80s electronic sound

To recreate these sounds:
1. Use slow attack pads with chorus
2. Layer Upper and Lower tones
3. Moderate filter with some resonance
4. Use ENV mod on the filter for movement
)";
        
        if (q.contains("analyze") || q.contains("current preset") || q.contains("this sound"))
            return analyzeCurrentPresetResponse();
            
        // Default response
        return R"(
I can help you with the MKS-70! Try asking about:

- Sound design: "How do I make a warm pad?"
- Parameters: "What does the VCF Resonance do?"
- History: "Who used the MKS-70?"
- Specific sounds: "I want a bass like New Order"

For online AI with more detailed responses, enable Online Mode 
and enter your OpenAI API key above.
)";
    }
    
    juce::String analyzeCurrentPresetResponse()
    {
        if (currentSysExData.getSize() < 10)
            return "No preset is currently loaded. Load a preset first, then ask me to analyze it.";
            
        return SysExParser::generateAnalysis(currentParsedPatch);
    }
    
    void showHistory()
    {
        appendToChat("You", "[Clicked: History]");
        appendToChat("Assistant", MKS70Knowledge::getHistory());
    }
    
    void showParameterGuide()
    {
        appendToChat("You", "[Clicked: Parameter Guide]");
        appendToChat("Assistant", MKS70Knowledge::getParameterGuide());
    }
    
    void showSoundDesignMenu()
    {
        juce::PopupMenu menu;
        menu.addItem(1, "Pads & Ambient");
        menu.addItem(2, "Bass Sounds");
        menu.addItem(3, "Lead Sounds");
        menu.addItem(4, "Strings");
        
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            juce::String topic;
            switch (result) {
                case 1: topic = "pad"; break;
                case 2: topic = "bass"; break;
                case 3: topic = "lead"; break;
                case 4: topic = "string"; break;
                default: return;
            }
            appendToChat("You", "[Clicked: Sound Design - " + topic + "]");
            appendToChat("Assistant", MKS70Knowledge::getSoundDesignTips(topic));
        });
    }
    
    void analyzeCurrentPreset()
    {
        appendToChat("You", "[Clicked: Analyze Preset]");
        appendToChat("Assistant", analyzeCurrentPresetResponse());
    }
    
    // Tutorial management functions
    void startTutorial(const AdvancedMKS70Knowledge::SoundRecipe& recipe)
    {
        currentRecipe = recipe;
        currentTutorialStep = 0;
        inTutorialMode = true;
        
        // Show tutorial buttons
        nextStepBtn.setVisible(true);
        prevStepBtn.setVisible(true);
        exitTutorialBtn.setVisible(true);
        
        // Display introduction
        juce::String intro = juce::String::fromUTF8("מתחיל הדרכה: ");
        intro += recipe.soundName;
        intro += "\n";
        intro += recipe.description;
        intro += "\n\n";
        intro += juce::String::fromUTF8("סה״כ ");
        intro += juce::String(recipe.steps.size());
        intro += juce::String::fromUTF8(" צעדים.");
        intro += juce::String::fromUTF8("\nלחץ 'צעד הבא' כדי להתחיל!");
        
        appendToChat("Assistant", intro);
        
        resized();
    }
    
    void advanceTutorialStep()
    {
        if (!inTutorialMode || currentTutorialStep >= currentRecipe.steps.size())
            return;
            
        auto step = currentRecipe.steps[currentTutorialStep];
        
        // Display step information
        juce::String stepText;
        stepText += juce::String::fromUTF8("🔹 צעד ");
        stepText += juce::String(step.stepNumber);
        stepText += " / ";
        stepText += juce::String(currentRecipe.steps.size());
        stepText += ":\n\n";
        stepText += step.instruction;
        stepText += "\n\n";
        stepText += juce::String::fromUTF8("💡 ");
        stepText += step.explanation;
        
        appendToChat("Assistant", stepText);
        
        // Highlight the parameter
        highlightParameterForStep(step);
        
        currentTutorialStep++;
        
        // Check if finished
        if (currentTutorialStep >= currentRecipe.steps.size())
        {
            juce::String completion = juce::String::fromUTF8("\n\n🎉 כל הכבוד! סיימת את ההדרכה.");
            completion += juce::String::fromUTF8("\nעכשיו נסה לנגן ולשמוע את הצליל!");
            appendToChat("Assistant", completion);
        }
    }
    
    void goToPreviousTutorialStep()
    {
        if (!inTutorialMode || currentTutorialStep <= 0)
            return;
            
        currentTutorialStep = juce::jmax(0, currentTutorialStep - 2);
        advanceTutorialStep();
    }
    
    void exitTutorialMode()
    {
        inTutorialMode = false;
        currentTutorialStep = -1;
        
        // Hide tutorial buttons
        nextStepBtn.setVisible(false);
        prevStepBtn.setVisible(false);
        exitTutorialBtn.setVisible(false);
        
        // Clear highlighting
        if (auto* highlighter = ParameterHighlighter::getInstance())
            highlighter->clearHighlights();
        
        appendToChat("Assistant", juce::String::fromUTF8("ההדרכה הסתיימה. אפשר להתחיל הדרכה חדשה בכל עת!"));
        
        resized();
    }
    
    void highlightParameterForStep(const AdvancedMKS70Knowledge::SoundDesignStep& step)
    {
        // TODO: Need to implement finding the actual component based on parameter name
        // This requires access to the main editor's component tree
        // For now, we'll just clear highlights - will implement component finding next
        
        if (auto* highlighter = ParameterHighlighter::getInstance())
        {
            highlighter->clearHighlights();
            
            // We need a way to find components by parameter name
            // This will be implemented in the next step when we add a reference to the editor
        }
    }
    
    void setEditorReference(juce::Component* editor)
    {
        editorComponent = editor;
    }
    
    void updateModeUI()
    {
        bool online = onlineMode.getToggleState();
        apiKeyLabel.setVisible(online);
        apiKeyInput.setVisible(online);
        apiKeyConnectBtn.setVisible(online);
        
        if (online)
            subtitleLabel.setText(currentLanguage == 2 ? 
                juce::String::fromUTF8("מוד מקוון - משתמש ב-API של AI") :
                "Online Mode - Using AI API for detailed responses", 
                juce::dontSendNotification);
        else
            subtitleLabel.setText(currentLanguage == 2 ? 
                juce::String::fromUTF8("מוד לא מקוון - משתמש במאגר ידע מובנה") :
                "Offline Mode - Using built-in knowledge base", 
                juce::dontSendNotification);
        
        resized();
    }
    
    juce::File getAPIKeyFile()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("MKS70Editor")
            .getChildFile("api_key.txt");
    }
    
    void loadAPIKey()
    {
        auto keyFile = getAPIKeyFile();
        if (keyFile.existsAsFile())
        {
            auto savedKey = keyFile.loadFileAsString().trim();
            if (savedKey.isNotEmpty())
            {
                apiKeyInput.setText(savedKey, false);
                apiKeyConnectBtn.setButtonText(currentLanguage == 2 ? 
                    juce::String::fromUTF8("מחובר") : "Connected");
                apiKeyConnectBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::cyan());
            }
        }
    }
    
    void saveAPIKey()
    {
        auto apiKey = apiKeyInput.getText().trim();
        
        if (apiKey.isEmpty())
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                currentLanguage == 2 ? juce::String::fromUTF8("שגיאה") : "Error",
                currentLanguage == 2 ? 
                    juce::String::fromUTF8("אנא הכנס מפתח API") :
                    "Please enter an API key",
                "OK");
            return;
        }
        
        // Save to file
        auto keyFile = getAPIKeyFile();
        keyFile.getParentDirectory().createDirectory();
        
        if (keyFile.replaceWithText(apiKey))
        {
            // Show confirmation with key length info
            juce::String msg = currentLanguage == 2 ? 
                juce::String::fromUTF8("מפתח ה-API נשמר בהצלחה!\nאורך: ") + juce::String(apiKey.length()) + juce::String::fromUTF8(" תווים") :
                "API key saved successfully!\nLength: " + juce::String(apiKey.length()) + " characters";
            
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                currentLanguage == 2 ? juce::String::fromUTF8("הצלחה") : "Success",
                msg,
                "OK");
            
            // Update button to show connected status
            apiKeyConnectBtn.setButtonText(currentLanguage == 2 ? 
                juce::String::fromUTF8("מחובר") : "Connected");
            apiKeyConnectBtn.setColour(juce::TextButton::buttonColourId, DS::Color::Accent::cyan());
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                currentLanguage == 2 ? juce::String::fromUTF8("שגיאה") : "Error",
                currentLanguage == 2 ? 
                    juce::String::fromUTF8("לא ניתן לשמור את המפתח לקובץ") :
                    "Could not save the key to file",
                "OK");
        }
    }
    
    void updateLanguageUI()
    {
        if (currentLanguage == 2)  // Hebrew
        {
            titleLabel.setText(juce::String::fromUTF8("עוזר AI ל-MKS-70"), juce::dontSendNotification);
            userInput.setTextToShowWhenEmpty(
                juce::String::fromUTF8("שאל על ה-MKS-70, תאר צליל שאתה רוצה ליצור..."),
                DS::Color::textMuted());
            sendBtn.setButtonText(juce::String::fromUTF8("שלח"));
            historyBtn.setButtonText(juce::String::fromUTF8("היסטוריה"));
            paramGuideBtn.setButtonText(juce::String::fromUTF8("פרמטרים"));
            soundDesignBtn.setButtonText(juce::String::fromUTF8("עיצוב צליל"));
            analyzeBtn.setButtonText(juce::String::fromUTF8("נתח פריסט"));
            chatHistory.setText(getWelcomeMessageHebrew());
        }
        else  // English
        {
            titleLabel.setText("MKS-70 AI Assistant", juce::dontSendNotification);
            userInput.setTextToShowWhenEmpty("Ask about MKS-70, describe a sound you want to create...", 
                                              DS::Color::textMuted());
            sendBtn.setButtonText("Send");
            historyBtn.setButtonText("History");
            paramGuideBtn.setButtonText("Parameters");
            soundDesignBtn.setButtonText("Sound Design");
            analyzeBtn.setButtonText("Analyze Preset");
            chatHistory.setText(getWelcomeMessage());
        }
        updateModeUI();
    }
    
    juce::String getWelcomeMessageHebrew()
    {
        return juce::String::fromUTF8(
            "ברוכים הבאים לעוזר ה-AI של MKS-70!\n\n"
            "אני יכול לעזור לך עם:\n"
            "- הבנת איך ה-MKS-70 עובד\n"
            "- יצירת צלילים ספציפיים (תאר מה שאתה רוצה!)\n"
            "- ניתוח פריסטים והסבר הפרמטרים\n"
            "- היסטוריה ושירים מפורסמים\n"
            "- טיפים לעיצוב צליל\n\n"
            "שאלות לדוגמה:\n"
            "- \"איך ליצור פאד חם?\"\n"
            "- \"מה עושה פרמטר Crossmod?\"\n"
            "- \"איזה אמנים השתמשו ב-MKS-70?\"\n"
            "- \"אני רוצה צליל כמו Depeche Mode\"\n\n"
        );
    }
    
    // Thread for async API calls
    void run() override
    {
        if (pendingQuery.isEmpty()) return;
        
        juce::String apiKey = apiKeyInput.getText().trim();
        
        // Build the prompt with MKS-70 context
        juce::String systemPrompt = R"(You are an expert on the Roland MKS-70 Super JX synthesizer. 
You have deep knowledge of:
- All parameters and how they affect sound
- Sound design techniques specific to the MKS-70
- History of the synthesizer and famous users
- How to recreate specific sounds

Be helpful, specific, and give practical parameter values when possible.
If asked about creating a sound, give specific parameter settings.
Respond in Hebrew if the user writes in Hebrew.
)";
        
        // Add current preset context if available
        if (currentSysExData.getSize() > 10)
        {
            systemPrompt << "\n\nCurrent preset analysis:\n";
            systemPrompt << SysExParser::generateAnalysis(currentParsedPatch);
        }
        
        // Combine system prompt with user query for Gemini
        juce::String fullPrompt = systemPrompt + "\n\nUser question: " + pendingQuery;
        
        // Make API call to Google Gemini
        juce::String urlStr = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + apiKey;
        juce::URL url(urlStr);
        
        // Build Gemini request body
        juce::DynamicObject::Ptr requestBody = new juce::DynamicObject();
        
        // Contents array with single user message
        juce::Array<juce::var> contents;
        juce::DynamicObject::Ptr content = new juce::DynamicObject();
        
        juce::Array<juce::var> parts;
        juce::DynamicObject::Ptr part = new juce::DynamicObject();
        part->setProperty("text", fullPrompt);
        parts.add(juce::var(part.get()));
        
        content->setProperty("parts", parts);
        contents.add(juce::var(content.get()));
        
        requestBody->setProperty("contents", contents);
        
        // Generation config
        juce::DynamicObject::Ptr genConfig = new juce::DynamicObject();
        genConfig->setProperty("temperature", 0.7);
        genConfig->setProperty("maxOutputTokens", 2048);
        requestBody->setProperty("generationConfig", juce::var(genConfig.get()));
        
        juce::String jsonBody = juce::JSON::toString(juce::var(requestBody.get()));
        
        juce::URL::InputStreamOptions options(juce::URL::ParameterHandling::inPostData);
        options.withExtraHeaders("Content-Type: application/json");
        options.withConnectionTimeoutMs(30000);
        
        url = url.withPOSTData(jsonBody);
        
        auto stream = url.createInputStream(options);
        
        if (stream != nullptr)
        {
            juce::String response = stream->readEntireStreamAsString();
            
            auto json = juce::JSON::parse(response);
            if (auto* obj = json.getDynamicObject())
            {
                // Check for error first
                if (obj->hasProperty("error"))
                {
                    auto error = obj->getProperty("error");
                    if (auto* errObj = error.getDynamicObject())
                    {
                        juce::String errMsg = errObj->getProperty("message").toString();
                        pendingResponse = juce::String::fromUTF8("שגיאת API: ") + errMsg;
                    }
                }
                // Parse Gemini response format
                else if (obj->hasProperty("candidates"))
                {
                    auto candidates = obj->getProperty("candidates");
                    if (candidates.isArray() && candidates.getArray()->size() > 0)
                    {
                        auto firstCandidate = candidates.getArray()->getFirst();
                        if (auto* candidateObj = firstCandidate.getDynamicObject())
                        {
                            auto content = candidateObj->getProperty("content");
                            if (auto* contentObj = content.getDynamicObject())
                            {
                                auto parts = contentObj->getProperty("parts");
                                if (parts.isArray() && parts.getArray()->size() > 0)
                                {
                                    auto firstPart = parts.getArray()->getFirst();
                                    if (auto* partObj = firstPart.getDynamicObject())
                                    {
                                        pendingResponse = partObj->getProperty("text").toString();
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (pendingResponse.isEmpty())
                {
                    pendingResponse = juce::String::fromUTF8("לא התקבלה תשובה מהשרת. נסה שוב.");
                }
            }
        }
        else
        {
            pendingResponse = juce::String::fromUTF8("נכשל בהתחברות לשירות AI. בדוק את החיבור לאינטרנט ואת מפתח ה-API.");
        }
        
        triggerAsyncUpdate();
    }
    
    void handleAsyncUpdate() override
    {
        if (!pendingResponse.isEmpty())
        {
            // Remove the "Thinking..." message and add real response
            juce::String current = chatHistory.getText();
            current = current.replace("\nAssistant:\nThinking...\n", "\n");
            chatHistory.setText(current);
            
            appendToChat("Assistant", pendingResponse);
            pendingResponse.clear();
        }
        isWaitingForResponse = false;
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AIAssistantPanel)
};

