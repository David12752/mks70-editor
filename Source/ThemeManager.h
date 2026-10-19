#pragma once

#include <JuceHeader.h>

/**
 * ThemeManager.h
 * Complete theme system with Dark, Light, High Contrast and custom themes
 * 
 * Part of Improvement #9: Dark/Light Themes
 */

namespace Theme
{

//==============================================================================
/**
 * Theme color palette
 */
struct ColorPalette
{
    // Backgrounds
    juce::Colour background;
    juce::Colour surface;
    juce::Colour surfaceAlt;
    juce::Colour surfaceHover;
    
    // Text
    juce::Colour text;
    juce::Colour textMuted;
    juce::Colour textDim;
    
    // Borders
    juce::Colour border;
    juce::Colour borderFocus;
    
    // Accent
    juce::Colour accent;
    juce::Colour accentHover;
    juce::Colour accentDim;
    
    // Semantic colors
    juce::Colour success;
    juce::Colour warning;
    juce::Colour error;
    juce::Colour info;
    
    // Section colors
    juce::Colour dco;
    juce::Colour vcf;
    juce::Colour vca;
    juce::Colour env;
    juce::Colour lfo;
    juce::Colour patch;
    juce::Colour arp;
    
    // Glow effects
    juce::Colour glow;
    float glowIntensity = 0.3f;
};

//==============================================================================
/**
 * Theme definition
 */
struct ThemeDefinition
{
    juce::String name;
    juce::String description;
    ColorPalette colors;
    bool isDark;
};

//==============================================================================
/**
 * Built-in themes
 */
namespace Themes
{
    // Roland Dark - Default professional dark theme
    inline ThemeDefinition RolandDark()
    {
        ThemeDefinition theme;
        theme.name = "Roland Dark";
        theme.description = "Professional dark theme inspired by Roland synths";
        theme.isDark = true;
        
        auto& c = theme.colors;
        
        // Backgrounds
        c.background   = juce::Colour(0xFF1a1a2e);
        c.surface      = juce::Colour(0xFF252542);
        c.surfaceAlt   = juce::Colour(0xFF2d2d4a);
        c.surfaceHover = juce::Colour(0xFF353560);
        
        // Text
        c.text         = juce::Colour(0xFFeaeaea);
        c.textMuted    = juce::Colour(0xFF8888aa);
        c.textDim      = juce::Colour(0xFF555577);
        
        // Borders
        c.border       = juce::Colour(0xFF3a3a5c);
        c.borderFocus  = juce::Colour(0xFF5555aa);
        
        // Accent
        c.accent       = juce::Colour(0xFF6366f1);
        c.accentHover  = juce::Colour(0xFF818cf8);
        c.accentDim    = juce::Colour(0xFF4f46e5);
        
        // Semantic
        c.success      = juce::Colour(0xFF10b981);
        c.warning      = juce::Colour(0xFFf59e0b);
        c.error        = juce::Colour(0xFFef4444);
        c.info         = juce::Colour(0xFF3b82f6);
        
        // Sections
        c.dco          = juce::Colour(0xFF6366f1);
        c.vcf          = juce::Colour(0xFFec4899);
        c.vca          = juce::Colour(0xFF10b981);
        c.env          = juce::Colour(0xFFf59e0b);
        c.lfo          = juce::Colour(0xFF8b5cf6);
        c.patch        = juce::Colour(0xFF06b6d4);
        c.arp          = juce::Colour(0xFFef4444);
        
        // Glow
        c.glow         = juce::Colour(0xFF6366f1);
        c.glowIntensity = 0.3f;
        
        return theme;
    }
    
    // Light Theme - Clean and bright
    inline ThemeDefinition Light()
    {
        ThemeDefinition theme;
        theme.name = "Light";
        theme.description = "Clean, bright theme for well-lit environments";
        theme.isDark = false;
        
        auto& c = theme.colors;
        
        // Backgrounds
        c.background   = juce::Colour(0xFFf5f5f7);
        c.surface      = juce::Colour(0xFFffffff);
        c.surfaceAlt   = juce::Colour(0xFFf0f0f2);
        c.surfaceHover = juce::Colour(0xFFe8e8ec);
        
        // Text
        c.text         = juce::Colour(0xFF1a1a1a);
        c.textMuted    = juce::Colour(0xFF666666);
        c.textDim      = juce::Colour(0xFF999999);
        
        // Borders
        c.border       = juce::Colour(0xFFd0d0d5);
        c.borderFocus  = juce::Colour(0xFF6366f1);
        
        // Accent (keep same as dark for consistency)
        c.accent       = juce::Colour(0xFF5558e3);
        c.accentHover  = juce::Colour(0xFF6366f1);
        c.accentDim    = juce::Colour(0xFF4f46e5);
        
        // Semantic
        c.success      = juce::Colour(0xFF059669);
        c.warning      = juce::Colour(0xFFd97706);
        c.error        = juce::Colour(0xFFdc2626);
        c.info         = juce::Colour(0xFF2563eb);
        
        // Sections
        c.dco          = juce::Colour(0xFF5558e3);
        c.vcf          = juce::Colour(0xFFdb2777);
        c.vca          = juce::Colour(0xFF059669);
        c.env          = juce::Colour(0xFFd97706);
        c.lfo          = juce::Colour(0xFF7c3aed);
        c.patch        = juce::Colour(0xFF0891b2);
        c.arp          = juce::Colour(0xFFdc2626);
        
        // Glow
        c.glow         = juce::Colour(0xFF5558e3);
        c.glowIntensity = 0.15f;
        
        return theme;
    }
    
    // High Contrast - Accessibility focused
    inline ThemeDefinition HighContrast()
    {
        ThemeDefinition theme;
        theme.name = "High Contrast";
        theme.description = "High contrast theme for accessibility";
        theme.isDark = true;
        
        auto& c = theme.colors;
        
        // Backgrounds - Pure black
        c.background   = juce::Colour(0xFF000000);
        c.surface      = juce::Colour(0xFF0a0a0a);
        c.surfaceAlt   = juce::Colour(0xFF151515);
        c.surfaceHover = juce::Colour(0xFF202020);
        
        // Text - Pure white
        c.text         = juce::Colour(0xFFffffff);
        c.textMuted    = juce::Colour(0xFFcccccc);
        c.textDim      = juce::Colour(0xFF888888);
        
        // Borders - High visibility
        c.border       = juce::Colour(0xFF555555);
        c.borderFocus  = juce::Colour(0xFFffff00);
        
        // Accent - Yellow for high visibility
        c.accent       = juce::Colour(0xFFffff00);
        c.accentHover  = juce::Colour(0xFFffff66);
        c.accentDim    = juce::Colour(0xFFcccc00);
        
        // Semantic - High saturation
        c.success      = juce::Colour(0xFF00ff00);
        c.warning      = juce::Colour(0xFFffaa00);
        c.error        = juce::Colour(0xFFff0000);
        c.info         = juce::Colour(0xFF00aaff);
        
        // Sections - Bright and distinct
        c.dco          = juce::Colour(0xFF00ffff);
        c.vcf          = juce::Colour(0xFFff00ff);
        c.vca          = juce::Colour(0xFF00ff00);
        c.env          = juce::Colour(0xFFffaa00);
        c.lfo          = juce::Colour(0xFFaa00ff);
        c.patch        = juce::Colour(0xFF00aaff);
        c.arp          = juce::Colour(0xFFff0000);
        
        // Glow
        c.glow         = juce::Colour(0xFFffff00);
        c.glowIntensity = 0.5f;
        
        return theme;
    }
    
    // Vintage - Classic synth look
    inline ThemeDefinition Vintage()
    {
        ThemeDefinition theme;
        theme.name = "Vintage";
        theme.description = "Classic vintage synthesizer aesthetics";
        theme.isDark = true;
        
        auto& c = theme.colors;
        
        // Backgrounds - Warm browns
        c.background   = juce::Colour(0xFF1a1612);
        c.surface      = juce::Colour(0xFF2a241e);
        c.surfaceAlt   = juce::Colour(0xFF352e26);
        c.surfaceHover = juce::Colour(0xFF453c32);
        
        // Text - Warm cream
        c.text         = juce::Colour(0xFFf5e6d3);
        c.textMuted    = juce::Colour(0xFFa89580);
        c.textDim      = juce::Colour(0xFF706050);
        
        // Borders
        c.border       = juce::Colour(0xFF4a4036);
        c.borderFocus  = juce::Colour(0xFFd4a574);
        
        // Accent - Warm orange
        c.accent       = juce::Colour(0xFFd4a574);
        c.accentHover  = juce::Colour(0xFFe8c090);
        c.accentDim    = juce::Colour(0xFFb08050);
        
        // Semantic
        c.success      = juce::Colour(0xFF88b060);
        c.warning      = juce::Colour(0xFFd4a020);
        c.error        = juce::Colour(0xFFc05040);
        c.info         = juce::Colour(0xFF5090c0);
        
        // Sections - Muted vintage colors
        c.dco          = juce::Colour(0xFFd4a574);
        c.vcf          = juce::Colour(0xFFc07080);
        c.vca          = juce::Colour(0xFF88b060);
        c.env          = juce::Colour(0xFFd4a020);
        c.lfo          = juce::Colour(0xFF9080b0);
        c.patch        = juce::Colour(0xFF5090c0);
        c.arp          = juce::Colour(0xFFc05040);
        
        // Glow
        c.glow         = juce::Colour(0xFFd4a574);
        c.glowIntensity = 0.25f;
        
        return theme;
    }
    
    // Cyberpunk - Neon and futuristic
    inline ThemeDefinition Cyberpunk()
    {
        ThemeDefinition theme;
        theme.name = "Cyberpunk";
        theme.description = "Neon-lit futuristic theme";
        theme.isDark = true;
        
        auto& c = theme.colors;
        
        // Backgrounds - Deep purple/black
        c.background   = juce::Colour(0xFF0d0d1a);
        c.surface      = juce::Colour(0xFF15152a);
        c.surfaceAlt   = juce::Colour(0xFF1a1a35);
        c.surfaceHover = juce::Colour(0xFF252545);
        
        // Text - Cool cyan
        c.text         = juce::Colour(0xFFe0f0ff);
        c.textMuted    = juce::Colour(0xFF7090b0);
        c.textDim      = juce::Colour(0xFF405060);
        
        // Borders
        c.border       = juce::Colour(0xFF303050);
        c.borderFocus  = juce::Colour(0xFF00ffff);
        
        // Accent - Hot pink/magenta
        c.accent       = juce::Colour(0xFFff00aa);
        c.accentHover  = juce::Colour(0xFFff44cc);
        c.accentDim    = juce::Colour(0xFFcc0088);
        
        // Semantic
        c.success      = juce::Colour(0xFF00ff88);
        c.warning      = juce::Colour(0xFFffaa00);
        c.error        = juce::Colour(0xFFff0044);
        c.info         = juce::Colour(0xFF00aaff);
        
        // Sections - Neon colors
        c.dco          = juce::Colour(0xFF00ffff);
        c.vcf          = juce::Colour(0xFFff00aa);
        c.vca          = juce::Colour(0xFF00ff88);
        c.env          = juce::Colour(0xFFffaa00);
        c.lfo          = juce::Colour(0xFFaa00ff);
        c.patch        = juce::Colour(0xFF00aaff);
        c.arp          = juce::Colour(0xFFff0044);
        
        // Glow - Strong neon glow
        c.glow         = juce::Colour(0xFFff00aa);
        c.glowIntensity = 0.5f;
        
        return theme;
    }

    // Modern Slate - Contemporary pro dark look
    inline ThemeDefinition ModernSlate()
    {
        ThemeDefinition theme;
        theme.name = "Modern Slate";
        theme.description = "Contemporary dark theme with slate blues";
        theme.isDark = true;
        
        auto& c = theme.colors;
        
        c.background   = juce::Colour(0xFF12161d);
        c.surface      = juce::Colour(0xFF1a2029);
        c.surfaceAlt   = juce::Colour(0xFF202833);
        c.surfaceHover = juce::Colour(0xFF27313d);
        
        c.text         = juce::Colour(0xFFeef2f8);
        c.textMuted    = juce::Colour(0xFF9aa8bb);
        c.textDim      = juce::Colour(0xFF5e6978);
        
        c.border       = juce::Colour(0xFF2d3643);
        c.borderFocus  = juce::Colour(0xFF4aa8ff);
        
        c.accent       = juce::Colour(0xFF4aa8ff);
        c.accentHover  = juce::Colour(0xFF6cb8ff);
        c.accentDim    = juce::Colour(0xFF2e7ad3);
        
        c.success      = juce::Colour(0xFF4caf50);
        c.warning      = juce::Colour(0xFFffb74d);
        c.error        = juce::Colour(0xFFef5350);
        c.info         = juce::Colour(0xFF42a5f5);
        
        c.dco          = juce::Colour(0xFF4aa8ff);
        c.vcf          = juce::Colour(0xFFf06292);
        c.vca          = juce::Colour(0xFF66bb6a);
        c.env          = juce::Colour(0xFFffb74d);
        c.lfo          = juce::Colour(0xFF9575cd);
        c.patch        = juce::Colour(0xFF26c6da);
        c.arp          = juce::Colour(0xFFef5350);
        
        c.glow         = juce::Colour(0xFF4aa8ff);
        c.glowIntensity = 0.22f;
        
        return theme;
    }

    // Modern Breeze - Airy minimal light mode
    inline ThemeDefinition ModernBreeze()
    {
        ThemeDefinition theme;
        theme.name = "Modern Breeze";
        theme.description = "Minimal light theme with cool blues";
        theme.isDark = false;

        auto& c = theme.colors;

        c.background   = juce::Colour(0xFFf3f6fb);
        c.surface      = juce::Colour(0xFFffffff);
        c.surfaceAlt   = juce::Colour(0xFFe8eef7);
        c.surfaceHover = juce::Colour(0xFFdde6f3);

        c.text         = juce::Colour(0xFF1c2230);
        c.textMuted    = juce::Colour(0xFF5c6578);
        c.textDim      = juce::Colour(0xFF8c95a8);

        c.border       = juce::Colour(0xFFccd4e2);
        c.borderFocus  = juce::Colour(0xFF3b82f6);

        c.accent       = juce::Colour(0xFF3b82f6);
        c.accentHover  = juce::Colour(0xFF5a94f7);
        c.accentDim    = juce::Colour(0xFF2c5fcc);

        c.success      = juce::Colour(0xFF2e8b57);
        c.warning      = juce::Colour(0xFFdb8b2b);
        c.error        = juce::Colour(0xFFd64545);
        c.info         = juce::Colour(0xFF1f7ae0);

        c.dco          = juce::Colour(0xFF3b82f6);
        c.vcf          = juce::Colour(0xFFec4899);
        c.vca          = juce::Colour(0xFF34a853);
        c.env          = juce::Colour(0xFFf59e0b);
        c.lfo          = juce::Colour(0xFF7c3aed);
        c.patch        = juce::Colour(0xFF0ea5e9);
        c.arp          = juce::Colour(0xFFd64545);

        c.glow         = juce::Colour(0xFF3b82f6);
        c.glowIntensity = 0.12f;

        return theme;
    }

    // Modern Ember - Warm studio vibe
    inline ThemeDefinition ModernEmber()
    {
        ThemeDefinition theme;
        theme.name = "Modern Ember";
        theme.description = "Warm dark theme with copper accents";
        theme.isDark = true;

        auto& c = theme.colors;

        c.background   = juce::Colour(0xFF181211);
        c.surface      = juce::Colour(0xFF201714);
        c.surfaceAlt   = juce::Colour(0xFF271d19);
        c.surfaceHover = juce::Colour(0xFF302421);

        c.text         = juce::Colour(0xFFF5EEE7);
        c.textMuted    = juce::Colour(0xFFcbb6a4);
        c.textDim      = juce::Colour(0xFF8c7565);

        c.border       = juce::Colour(0xFF3b2d26);
        c.borderFocus  = juce::Colour(0xFFf28c52);

        c.accent       = juce::Colour(0xFFf28c52);
        c.accentHover  = juce::Colour(0xFFff9a61);
        c.accentDim    = juce::Colour(0xFFc45d28);

        c.success      = juce::Colour(0xFF9dcc63);
        c.warning      = juce::Colour(0xFFf4b642);
        c.error        = juce::Colour(0xFFe3605f);
        c.info         = juce::Colour(0xFFf28c52);

        c.dco          = juce::Colour(0xFFf28c52);
        c.vcf          = juce::Colour(0xFFf76d82);
        c.vca          = juce::Colour(0xFF9dcc63);
        c.env          = juce::Colour(0xFFf4b642);
        c.lfo          = juce::Colour(0xFFb38adf);
        c.patch        = juce::Colour(0xFFf0a87a);
        c.arp          = juce::Colour(0xFFe3605f);

        c.glow         = juce::Colour(0xFFf28c52);
        c.glowIntensity = 0.28f;

        return theme;
    }
}

//==============================================================================
/**
 * Theme Manager - Singleton
 */
class ThemeManager
{
public:
    static ThemeManager& getInstance()
    {
        static ThemeManager instance;
        return instance;
    }
    
    //==========================================================================
    // Theme Selection
    //==========================================================================
    
    void setTheme(const juce::String& themeName)
    {
        for (const auto& theme : availableThemes)
        {
            if (theme.name == themeName)
            {
                currentTheme = theme;
                notifyListeners();
                return;
            }
        }
    }
    
    void setThemeByIndex(int index)
    {
        if (index >= 0 && index < static_cast<int>(availableThemes.size()))
        {
            currentTheme = availableThemes[index];
            notifyListeners();
        }
    }
    
    const ThemeDefinition& getCurrentTheme() const { return currentTheme; }
    const ColorPalette& getColors() const { return currentTheme.colors; }
    
    //==========================================================================
    // Available Themes
    //==========================================================================
    
    const std::vector<ThemeDefinition>& getAvailableThemes() const { return availableThemes; }
    
    int getCurrentThemeIndex() const
    {
        for (size_t i = 0; i < availableThemes.size(); i++)
        {
            if (availableThemes[i].name == currentTheme.name)
                return static_cast<int>(i);
        }
        return 0;
    }
    
    juce::StringArray getThemeNames() const
    {
        juce::StringArray names;
        for (const auto& theme : availableThemes)
            names.add(theme.name);
        return names;
    }
    
    //==========================================================================
    // Listeners
    //==========================================================================
    
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void themeChanged(const ThemeDefinition& newTheme) = 0;
    };
    
    void addListener(Listener* listener) { listeners.push_back(listener); }
    void removeListener(Listener* listener)
    {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }
    
    //==========================================================================
    // Custom Theme
    //==========================================================================
    
    void addCustomTheme(const ThemeDefinition& theme)
    {
        availableThemes.push_back(theme);
    }
    
    //==========================================================================
    // Helper - Apply to LookAndFeel
    //==========================================================================
    
    void applyToLookAndFeel(juce::LookAndFeel_V4& laf)
    {
        const auto& c = currentTheme.colors;
        const bool isDark = currentTheme.isDark;
        
        laf.setColour(juce::ResizableWindow::backgroundColourId, c.background);
        laf.setColour(juce::TextEditor::backgroundColourId, c.surface);
        laf.setColour(juce::TextEditor::textColourId, c.text);
        laf.setColour(juce::TextEditor::outlineColourId, c.border);
        laf.setColour(juce::TextEditor::focusedOutlineColourId, c.borderFocus);
        
        laf.setColour(juce::ComboBox::backgroundColourId, c.surface);
        laf.setColour(juce::ComboBox::textColourId, c.text);
        laf.setColour(juce::ComboBox::outlineColourId, c.border);
        laf.setColour(juce::ComboBox::arrowColourId, c.text);
        
        laf.setColour(juce::TextButton::buttonColourId, c.surface);
        laf.setColour(juce::TextButton::textColourOnId, c.text);
        laf.setColour(juce::TextButton::textColourOffId, c.text);
        laf.setColour(juce::TextButton::buttonOnColourId, c.accent.withAlpha(0.25f));
        
        laf.setColour(juce::Slider::backgroundColourId, c.surfaceAlt);
        laf.setColour(juce::Slider::thumbColourId, c.accent);
        laf.setColour(juce::Slider::trackColourId, c.accentDim);
        laf.setColour(juce::Slider::rotarySliderFillColourId, c.accent);
        laf.setColour(juce::Slider::rotarySliderOutlineColourId, c.accentDim);
        laf.setColour(juce::Slider::textBoxTextColourId, c.text);
        laf.setColour(juce::Slider::textBoxBackgroundColourId, c.surfaceAlt);
        laf.setColour(juce::Slider::textBoxOutlineColourId, c.border);
        
        laf.setColour(juce::ToggleButton::tickColourId, c.accent);
        laf.setColour(juce::ToggleButton::tickDisabledColourId, c.textDim);
        
        laf.setColour(juce::Label::textColourId, c.text);
        
        laf.setColour(juce::ListBox::backgroundColourId, c.surface);
        laf.setColour(juce::ListBox::textColourId, c.text);
        
        laf.setColour(juce::PopupMenu::backgroundColourId, c.surface);
        laf.setColour(juce::PopupMenu::textColourId, c.text);
        laf.setColour(juce::PopupMenu::highlightedBackgroundColourId, c.accent);
        laf.setColour(juce::PopupMenu::highlightedTextColourId, isDark ? juce::Colours::white : juce::Colours::black);
        
        laf.setColour(juce::TabbedComponent::backgroundColourId, c.background);
        laf.setColour(juce::TabbedComponent::outlineColourId, c.border);
        
        laf.setColour(juce::ScrollBar::thumbColourId, c.accentDim.withAlpha(0.8f));
        laf.setColour(juce::ScrollBar::trackColourId, c.surfaceAlt);
    }
    
private:
    ThemeManager()
    {
        // Initialize available themes
        availableThemes.push_back(Themes::RolandDark());
        availableThemes.push_back(Themes::ModernSlate());
        availableThemes.push_back(Themes::ModernEmber());
        availableThemes.push_back(Themes::Light());
        availableThemes.push_back(Themes::ModernBreeze());
        availableThemes.push_back(Themes::HighContrast());
        availableThemes.push_back(Themes::Vintage());
        availableThemes.push_back(Themes::Cyberpunk());
        
        // Set default theme
        currentTheme = availableThemes[0];
    }
    
    void notifyListeners()
    {
        for (auto* listener : listeners)
            listener->themeChanged(currentTheme);
    }
    
    ThemeDefinition currentTheme;
    std::vector<ThemeDefinition> availableThemes;
    std::vector<Listener*> listeners;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeManager)
};

//==============================================================================
/**
 * Theme Selector Component
 */
class ThemeSelector : public juce::Component
{
public:
    ThemeSelector()
    {
        auto& manager = ThemeManager::getInstance();
        
        themeCombo.addItemList(manager.getThemeNames(), 1);
        themeCombo.setSelectedId(manager.getCurrentThemeIndex() + 1, juce::dontSendNotification);
        
        themeCombo.onChange = [this, &manager]
        {
            manager.setThemeByIndex(themeCombo.getSelectedId() - 1);
        };
        
        addAndMakeVisible(themeCombo);
        
        label.setText("Theme:", juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, manager.getColors().textMuted);
        addAndMakeVisible(label);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromLeft(50));
        themeCombo.setBounds(bounds);
    }
    
private:
    juce::ComboBox themeCombo;
    juce::Label label;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeSelector)
};

//==============================================================================
/**
 * Theme Preview Component
 * Shows a mini preview of theme colors
 */
class ThemePreview : public juce::Component
{
public:
    ThemePreview(const ThemeDefinition& theme) : themeToPreview(theme)
    {
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        const auto& c = themeToPreview.colors;
        
        // Background
        g.setColour(c.background);
        g.fillRoundedRectangle(bounds, 4);
        
        // Surface panel
        auto surfaceBounds = bounds.reduced(4);
        g.setColour(c.surface);
        g.fillRoundedRectangle(surfaceBounds.removeFromTop(surfaceBounds.getHeight() * 0.6f), 3);
        
        // Color swatches
        auto swatchArea = bounds.reduced(4).removeFromBottom(bounds.getHeight() * 0.3f);
        float swatchWidth = swatchArea.getWidth() / 5;
        
        g.setColour(c.accent);
        g.fillRect(swatchArea.removeFromLeft(swatchWidth).reduced(1));
        
        g.setColour(c.dco);
        g.fillRect(swatchArea.removeFromLeft(swatchWidth).reduced(1));
        
        g.setColour(c.vcf);
        g.fillRect(swatchArea.removeFromLeft(swatchWidth).reduced(1));
        
        g.setColour(c.env);
        g.fillRect(swatchArea.removeFromLeft(swatchWidth).reduced(1));
        
        g.setColour(c.lfo);
        g.fillRect(swatchArea.reduced(1));
        
        // Border
        g.setColour(c.border);
        g.drawRoundedRectangle(bounds, 4, 1);
    }
    
private:
    const ThemeDefinition& themeToPreview;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemePreview)
};

} // namespace Theme
