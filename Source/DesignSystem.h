#pragma once
#include <JuceHeader.h>
#include "ThemeManager.h"

//==============================================================================
// DS - Modern Design System for MKS-70 Editor
// נועז, זוהר, מודרני - Now with THEME SUPPORT
//==============================================================================
namespace DS {

    //==========================================================================
    // 🎨 COLOR PALETTE - Dynamic colors from ThemeManager
    //==========================================================================
    namespace Color {
        
        // Get current theme colors
        inline const Theme::ColorPalette& theme() {
            return Theme::ThemeManager::getInstance().getColors();
        }
        
        // === BACKGROUNDS - Dynamic from theme ===
        inline juce::Colour bg()           { return theme().background; }
        inline juce::Colour bgCard()       { return theme().surface; }
        inline juce::Colour bgElevated()   { return theme().surfaceAlt; }
        inline juce::Colour bgHover()      { return theme().surfaceHover; }
        inline juce::Colour bgInput()      { return theme().surface.darker(0.1f); }
        
        // === ACCENT COLORS - Dynamic from theme ===
        namespace Accent {
            // Primary - from theme accent
            inline juce::Colour primary()      { return theme().accent; }
            inline juce::Colour primaryGlow()  { return theme().accentHover; }
            inline juce::Colour primaryDark()  { return theme().accentDim; }
            
            // Secondary - cyan (section color)
            inline juce::Colour cyan()         { return theme().patch; }
            inline juce::Colour cyanGlow()     { return theme().patch.brighter(0.3f); }
            
            // Tertiary - magenta/pink
            inline juce::Colour magenta()      { return theme().vcf; }
            inline juce::Colour magentaGlow()  { return theme().vcf.brighter(0.3f); }
            
            // Success/Warning/Error from theme
            inline juce::Colour success()      { return theme().success; }
            inline juce::Colour warning()      { return theme().warning; }
            inline juce::Colour error()        { return theme().error; }
        }
        
        // === SECTION COLORS - Dynamic from theme ===
        namespace Section {
            inline juce::Colour dco1()    { return theme().dco; }
            inline juce::Colour dco2()    { return theme().dco.withRotatedHue(0.05f); }
            inline juce::Colour mixer()   { return theme().lfo.withRotatedHue(0.1f); }
            inline juce::Colour vcf()     { return theme().vcf; }
            inline juce::Colour vca()     { return theme().vca; }
            inline juce::Colour env1()    { return theme().env; }
            inline juce::Colour env2()    { return theme().arp; }
            inline juce::Colour lfo()     { return theme().lfo; }
            inline juce::Colour chorus()  { return theme().patch; }
        }
        
        // === TEXT - Dynamic from theme ===
        inline juce::Colour textPrimary()   { return theme().text; }
        inline juce::Colour textSecondary() { return theme().textMuted; }
        inline juce::Colour textMuted()     { return theme().textMuted; }
        inline juce::Colour textDisabled()  { return theme().textDim; }
        
        // === BORDERS & DIVIDERS - Dynamic from theme ===
        inline juce::Colour border()        { return theme().border; }
        inline juce::Colour borderLight()   { return theme().border.brighter(0.2f); }
        inline juce::Colour borderFocus()   { return theme().borderFocus; }
        
        // === GRADIENTS - Helper for gradient colors ===
        inline juce::ColourGradient gradientPrimary(juce::Rectangle<float> bounds) {
            return juce::ColourGradient(
                Accent::primary(), bounds.getTopLeft(),
                Accent::cyan(), bounds.getBottomRight(), false
            );
        }
        
        inline juce::ColourGradient gradientWarm(juce::Rectangle<float> bounds) {
            return juce::ColourGradient(
                Accent::magenta(), bounds.getTopLeft(),
                juce::Colour(0xFFF59E0B), bounds.getBottomRight(), false
            );
        }
        
        inline juce::ColourGradient gradientCool(juce::Rectangle<float> bounds) {
            return juce::ColourGradient(
                Accent::cyan(), bounds.getTopLeft(),
                Accent::primary(), bounds.getBottomRight(), false
            );
        }
    }

    //==========================================================================
    // 📐 SPACING - מרווחים
    //==========================================================================
    namespace Space {
        constexpr int XXS = 2;
        constexpr int XS  = 4;
        constexpr int S   = 8;
        constexpr int M   = 12;
        constexpr int L   = 16;
        constexpr int XL  = 24;
        constexpr int XXL = 32;
        constexpr int XXXL = 48;
    }

    //==========================================================================
    // 📏 SIZES - גדלים
    //==========================================================================
    namespace Size {
        // Sliders
        constexpr int SliderWidth = 55;
        constexpr int SliderHeight = 130;
        constexpr int SliderHeightCompact = 90;
        
        // Knobs - VCA-style size (85px = ~2.2cm diameter)
        constexpr int KnobTiny = 55;
        constexpr int KnobSmall = 75;   // Increased from 65 for better usability
        constexpr int KnobMedium = 85;
        constexpr int KnobLarge = 100;
        constexpr int KnobSize = 85; // Default knob size (VCA style)
        
        // Controls - MINIMAL combos
        constexpr int ComboHeight = 28;  // Increased from 20 for better usability
        constexpr int ComboHeightTiny = 20;  // Increased from 16
        constexpr int ComboWidth = 70;
        constexpr int ButtonHeight = 36;
        constexpr int ButtonHeightSmall = 28;
        constexpr int ToggleSize = 24;
        constexpr int TextFieldHeight = 36;
        
        // Layout
        constexpr int SectionHeaderHeight = 20;
        constexpr int TabHeight = 44;
        constexpr int HeaderHeight = 38;  // Compact header
        constexpr int SidebarWidth = 280;
        
        // Cards
        constexpr int CardPadding = 16;
        constexpr int CardRadius = 12;
    }

    //==========================================================================
    // 🔤 FONTS - פונטים
    //==========================================================================
    namespace Font {
        // Base fonts using FontOptions
        inline juce::Font base(float size = 13.0f) { 
            return juce::Font(juce::FontOptions(size)); 
        }
        inline juce::Font bold(float size = 13.0f) { 
            return juce::Font(juce::FontOptions(size)).boldened(); 
        }
        inline juce::Font mono(float size = 12.0f) { 
            return juce::Font(juce::FontOptions(size)).withTypefaceStyle("Regular"); 
        }
        
        // Semantic fonts
        inline juce::Font tiny()        { return base(10.0f); }
        inline juce::Font small()       { return base(11.0f); }
        inline juce::Font normal()      { return base(13.0f); }
        inline juce::Font medium()      { return bold(14.0f); }
        inline juce::Font large()       { return bold(16.0f); }
        inline juce::Font xlarge()      { return bold(20.0f); }
        inline juce::Font display()     { return bold(28.0f); }
        
        // Component-specific
        inline juce::Font label()       { return base(11.0f); }
        inline juce::Font value()       { return mono(12.0f); }
        inline juce::Font sectionTitle(){ return bold(10.0f); }
        inline juce::Font tabLabel()    { return bold(12.0f); }
        inline juce::Font button()      { return bold(12.0f); }
    }

    //==========================================================================
    // ⭕ RADIUS - עיגול פינות
    //==========================================================================
    namespace Radius {
        constexpr float None = 0.0f;
        constexpr float XS = 4.0f;
        constexpr float S = 6.0f;
        constexpr float M = 8.0f;
        constexpr float L = 12.0f;
        constexpr float XL = 16.0f;
        constexpr float XXL = 24.0f;
        constexpr float Full = 999.0f;
    }

    //==========================================================================
    // 🌟 EFFECTS - אפקטים
    //==========================================================================
    namespace Effect {
        // Glow intensity - REDUCED for subtlety
        constexpr float GlowSmall = 3.0f;
        constexpr float GlowMedium = 5.0f;
        constexpr float GlowLarge = 10.0f;
        
        // Shadow - REDUCED
        inline juce::DropShadow cardShadow() {
            return juce::DropShadow(juce::Colours::black.withAlpha(0.2f), 6, {0, 2});
        }
        
        inline juce::DropShadow buttonShadow() {
            return juce::DropShadow(juce::Colours::black.withAlpha(0.15f), 3, {0, 1});
        }
        
        // Draw glow effect
        inline void drawGlow(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour color, float radius) {
            for (float i = radius; i > 0; i -= 2.0f) {
                float alpha = (1.0f - i / radius) * 0.15f;
                g.setColour(color.withAlpha(alpha));
                g.fillRoundedRectangle(bounds.expanded(i), Radius::M + i);
            }
        }
        
        // Draw gradient border
        inline void drawGradientBorder(juce::Graphics& g, juce::Rectangle<float> bounds, 
                                        juce::Colour c1, juce::Colour c2, float thickness = 1.5f) {
            juce::Path path;
            path.addRoundedRectangle(bounds, Radius::M);
            
            juce::ColourGradient gradient(c1, bounds.getTopLeft(), c2, bounds.getBottomRight(), false);
            g.setGradientFill(gradient);
            g.strokePath(path, juce::PathStrokeType(thickness));
        }
    }

    //==========================================================================
    // 🎬 ANIMATION - אנימציות (זמנים במילישניות)
    //==========================================================================
    namespace Anim {
        constexpr int Fast = 100;
        constexpr int Normal = 200;
        constexpr int Slow = 400;
        constexpr int VerySlow = 600;
    }
}

//==============================================================================
// LEGACY NAMESPACE - תאימות לאחור עם הקוד הקיים
// נשמר כדי שהקוד הישן ימשיך לעבוד
//==============================================================================
namespace DesignSystem {

    namespace Spacing {
        constexpr int XXS = DS::Space::XXS;
        constexpr int XS  = DS::Space::XS;
        constexpr int S   = DS::Space::S;
        constexpr int M   = DS::Space::M;
        constexpr int L   = DS::Space::L;
        constexpr int XL  = DS::Space::XL;
        constexpr int XXL = DS::Space::XXL;
    }

    namespace Size {
        constexpr int KnobSize = DS::Size::KnobMedium;
        constexpr int KnobSizeSmall = DS::Size::KnobSmall;
        constexpr int KnobSizeLarge = DS::Size::KnobLarge;
        constexpr int ComboHeightTiny = DS::Size::ComboHeightTiny;
        
        constexpr int SliderWidth = DS::Size::SliderWidth;
        constexpr int SliderHeight = DS::Size::SliderHeight;
        constexpr int SliderHeightCompact = DS::Size::SliderHeightCompact;
        constexpr int SliderWidthHorizontal = 80;
        constexpr int SliderHeightHorizontal = 20;
        
        constexpr int ComboBoxWidth = DS::Size::ComboWidth;
        constexpr int ComboBoxWidthWide = 90;
        constexpr int ComboBoxHeight = DS::Size::ComboHeight;
        
        constexpr int ButtonHeight = DS::Size::ButtonHeight;
        constexpr int ButtonHeightSmall = DS::Size::ButtonHeightSmall;
        constexpr int ButtonWidthMin = 60;
        constexpr int ToggleSize = DS::Size::ToggleSize;
        
        constexpr int TextFieldHeight = DS::Size::TextFieldHeight;
        constexpr int LabelHeight = 12;
        
        constexpr int SectionHeaderHeight = DS::Size::SectionHeaderHeight;
        constexpr int SectionMinWidth = 120;
        constexpr int SectionPadding = DS::Size::CardPadding;
        
        constexpr int TabHeight = DS::Size::TabHeight;
        constexpr int TabMinWidth = 80;
    }

    namespace Font {
        inline juce::Font Tiny()     { return DS::Font::tiny(); }
        inline juce::Font Small()    { return DS::Font::small(); }
        inline juce::Font Normal()   { return DS::Font::normal(); }
        inline juce::Font Medium()   { return DS::Font::medium(); }
        inline juce::Font Large()    { return DS::Font::large(); }
        inline juce::Font XLarge()   { return DS::Font::xlarge(); }
        inline juce::Font Header()   { return DS::Font::display(); }
        
        inline juce::Font SliderLabel()    { return DS::Font::label(); }
        inline juce::Font SliderValue()    { return DS::Font::value(); }
        inline juce::Font SectionTitle()   { return DS::Font::sectionTitle(); }
        inline juce::Font TabLabel()       { return DS::Font::tabLabel(); }
        inline juce::Font ParameterName()  { return DS::Font::label(); }
        inline juce::Font ParameterValue() { return DS::Font::medium(); }
    }

    namespace Color {
        // Background layers - mapped to new DS
        inline juce::Colour Background()      { return DS::Color::bg(); }
        inline juce::Colour BackgroundLight() { return DS::Color::bgCard(); }
        inline juce::Colour BackgroundDark()  { return juce::Colour(0xFF08080A); }
        
        // Sections
        inline juce::Colour SectionBg()       { return DS::Color::bgCard(); }
        inline juce::Colour SectionBorder()   { return DS::Color::border(); }
        inline juce::Colour SectionHeader()   { return DS::Color::bgElevated(); }
        
        // Text
        inline juce::Colour TextPrimary()     { return DS::Color::textPrimary(); }
        inline juce::Colour TextSecondary()   { return DS::Color::textSecondary(); }
        inline juce::Colour TextDisabled()    { return DS::Color::textDisabled(); }
        inline juce::Colour TextHighlight()   { return juce::Colour(0xFFFFFFFF); }
        
        // Accent colors - mapped to new vibrant section colors
        inline juce::Colour AccentDCO()       { return DS::Color::Section::dco1(); }
        inline juce::Colour AccentVCF()       { return DS::Color::Section::vcf(); }
        inline juce::Colour AccentVCA()       { return DS::Color::Section::vca(); }
        inline juce::Colour AccentENV()       { return DS::Color::Section::env1(); }
        inline juce::Colour AccentLFO()       { return DS::Color::Section::lfo(); }
        inline juce::Colour AccentMixer()     { return DS::Color::Section::mixer(); }
        
        // Interactive
        inline juce::Colour SliderTrack()     { return DS::Color::bgInput(); }
        inline juce::Colour SliderFill()      { return DS::Color::Accent::primary(); }
        inline juce::Colour SliderThumb()     { return DS::Color::textPrimary(); }
        inline juce::Colour ButtonBg()        { return DS::Color::bgElevated(); }
        inline juce::Colour ButtonHover()     { return DS::Color::bgHover(); }
        inline juce::Colour ButtonActive()    { return DS::Color::Accent::primary(); }
        
        // Status
        inline juce::Colour Success()         { return DS::Color::Accent::success(); }
        inline juce::Colour Warning()         { return DS::Color::Accent::warning(); }
        inline juce::Colour Error()           { return DS::Color::Accent::error(); }
        inline juce::Colour Info()            { return DS::Color::Accent::cyan(); }
    }

    namespace Radius {
        constexpr float None = DS::Radius::None;
        constexpr float Small = DS::Radius::S;
        constexpr float Medium = DS::Radius::M;
        constexpr float Large = DS::Radius::L;
        constexpr float Round = DS::Radius::Full;
    }

    namespace Border {
        constexpr float None = 0.0f;
        constexpr float Thin = 1.0f;
        constexpr float Normal = 1.5f;
        constexpr float Thick = 2.0f;
    }
}
