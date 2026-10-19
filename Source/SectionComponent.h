#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

class SectionComponent : public juce::Component {
public:
    SectionComponent(const juce::String& title, juce::Colour accentColor = juce::Colour())
        : sectionTitle(title), accent(accentColor)
    {
        if (!accent.isOpaque())
            accent = DesignSystem::Color::SectionBorder();
    }
    
    void paint(juce::Graphics& g) override {
        using namespace DesignSystem;
        
        auto bounds = getLocalBounds().toFloat();
        
        // === OUTER SHADOW (minimal) ===
        g.setColour(juce::Colours::black.withAlpha(0.1f));
        g.fillRoundedRectangle(bounds.translated(0, 1.0f).expanded(0.5f), Radius::Medium + 1);
        
        // === MAIN BACKGROUND with gradient ===
        juce::ColourGradient bgGradient(
            Color::SectionBg().brighter(0.03f), bounds.getCentreX(), bounds.getY(),
            Color::SectionBg().darker(0.08f), bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill(bgGradient);
        g.fillRoundedRectangle(bounds, Radius::Medium);
        
        // === INNER SHADOW (inset effect) ===
        g.setColour(juce::Colours::black.withAlpha(0.15f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), Radius::Medium - 1, 1.5f);
        
        // === HIGHLIGHT RIM (top edge light) ===
        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.drawLine(bounds.getX() + Radius::Medium, bounds.getY() + 1,
                   bounds.getRight() - Radius::Medium, bounds.getY() + 1, 1.0f);
        
        // === OUTER BORDER ===
        g.setColour(Color::SectionBorder());
        g.drawRoundedRectangle(bounds.reduced(0.5f), Radius::Medium, Border::Thin);
        
        // === HEADER BAR with gradient ===
        auto headerBounds = bounds.removeFromTop((float) Size::SectionHeaderHeight);
        juce::ColourGradient headerGradient(
            Color::SectionHeader().brighter(0.1f), headerBounds.getCentreX(), headerBounds.getY(),
            Color::SectionHeader().darker(0.1f), headerBounds.getCentreX(), headerBounds.getBottom(), false);
        g.setGradientFill(headerGradient);
        g.fillRoundedRectangle(headerBounds.withTrimmedBottom(-Radius::Medium), 
                               Radius::Medium);
        
        // === ACCENT LINE (thin) ===
        float accentY = headerBounds.getBottom() - 1.0f;
        // No glow - just thin line
        g.setColour(accent);
        g.fillRect(headerBounds.getX() + 1.0f, accentY,
                   headerBounds.getWidth() - 2.0f, 1.0f);
        
        // === TITLE (no shadow) ===
        g.setFont(Font::SectionTitle());
        g.setColour(Color::TextHighlight());
        g.drawText(sectionTitle, headerBounds.reduced(6, 0), 
                   juce::Justification::centredLeft);
    }
    
    void resized() override {
        using namespace DesignSystem;
        auto bounds = getLocalBounds();
        bounds.removeFromTop(Size::SectionHeaderHeight);  // No gap
        bounds.reduce(2, 1);  // Ultra-minimal padding
        contentBounds = bounds;
    }
    
    juce::Rectangle<int> getContentBounds() const { return contentBounds; }
    
    // Helper to set child bounds
    void setContentComponent(juce::Component* child) {
        if (child) {
            addAndMakeVisible(child);
            child->setBounds(contentBounds);
        }
    }
    
private:
    juce::String sectionTitle;
    juce::Colour accent;
    juce::Rectangle<int> contentBounds;
};
