#pragma once

#include <JuceHeader.h>

namespace Theme
{
    // Color tokens
    inline juce::Colour bgBase()          { return juce::Colour::fromRGB(0x12,0x14,0x17); }
    inline juce::Colour bgPanel()         { return juce::Colour::fromRGB(0x1A,0x1D,0x21); }
    inline juce::Colour border()          { return juce::Colour::fromRGB(0x2A,0x2F,0x36); }
    inline juce::Colour textPrimary()     { return juce::Colour::fromRGB(0xE6,0xEA,0xF0); }
    inline juce::Colour textSecondary()   { return juce::Colour::fromRGB(0xA6,0xB0,0xBF); }
    inline juce::Colour accentPrimary()   { return juce::Colour::fromRGB(0x4D,0xD0,0xE1); }
    inline juce::Colour accentOk()        { return juce::Colour::fromRGB(0x78,0xC2,0x57); }
    inline juce::Colour accentWarn()      { return juce::Colour::fromRGB(0xFF,0xB7,0x4D); }
    inline juce::Colour accentError()     { return juce::Colour::fromRGB(0xEF,0x53,0x50); }
    inline juce::Colour gridLine(float a=.45f){ return juce::Colour::fromRGB(0x3A,0x40,0x48).withAlpha(a); }

    // Font tokens
    inline juce::FontOptions heading1()   { return juce::FontOptions(18.0f, juce::Font::bold); }
    inline juce::FontOptions heading2()   { return juce::FontOptions(14.0f, juce::Font::bold); }
    inline juce::FontOptions label()      { return juce::FontOptions(12.5f); }
    inline juce::FontOptions value()      { return juce::FontOptions(11.0f); }

    // Spacing tokens (in px)
    namespace Space { enum { xs=4, s=8, m=12, l=16, xl=20, xxl=24 }; }

    // Apply a minimal dark palette to global LookAndFeel ids
    inline void applyToLookAndFeel(juce::LookAndFeel& laf)
    {
        auto& cm = laf; // alias
        cm.setColour(juce::ResizableWindow::backgroundColourId, bgBase());
        cm.setColour(juce::DocumentWindow::backgroundColourId, bgBase());
        cm.setColour(juce::Label::textColourId, textPrimary());
        cm.setColour(juce::GroupComponent::outlineColourId, border());
        cm.setColour(juce::GroupComponent::textColourId, textSecondary());
        cm.setColour(juce::Slider::thumbColourId, accentPrimary());
        cm.setColour(juce::Slider::trackColourId, accentPrimary().withAlpha(0.5f));
        cm.setColour(juce::Slider::textBoxTextColourId, textPrimary());
        cm.setColour(juce::Slider::textBoxBackgroundColourId, bgPanel());
        cm.setColour(juce::Slider::textBoxOutlineColourId, border());
        cm.setColour(juce::ComboBox::backgroundColourId, bgPanel());
        cm.setColour(juce::ComboBox::outlineColourId, border());
        cm.setColour(juce::ComboBox::textColourId, textPrimary());
        cm.setColour(juce::TextButton::buttonColourId, bgPanel());
        cm.setColour(juce::TextButton::buttonOnColourId, accentPrimary().withAlpha(0.25f));
        cm.setColour(juce::TextButton::textColourOnId, textPrimary());
        cm.setColour(juce::TextButton::textColourOffId, textPrimary());
    }
}
