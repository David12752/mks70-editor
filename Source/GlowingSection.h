#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
// GlowingSection - כרטיס סקציה עם זוהר ועיצוב מודרני
// With collapse/expand functionality
//==============================================================================
class GlowingSection : public juce::Component,
                       private juce::Timer {
public:
    GlowingSection(const juce::String& title, juce::Colour accentColor)
        : sectionTitle(title), accent(accentColor) 
    {
        // Enable mouse click detection for collapse/expand
        setInterceptsMouseClicks(true, true);
    }
    
    ~GlowingSection() override
    {
        stopTimer();
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // === OUTER GLOW ===
        DS::Effect::drawGlow(g, bounds.reduced(4.0f), accent, DS::Effect::GlowSmall);
        
        // === CARD BACKGROUND with subtle gradient ===
        juce::ColourGradient bgGradient(
            DS::Color::bgCard().brighter(0.03f), bounds.getTopLeft(),
            DS::Color::bgCard().darker(0.02f), bounds.getBottomLeft(), false
        );
        g.setGradientFill(bgGradient);
        g.fillRoundedRectangle(bounds, DS::Radius::L);
        
        // === GRADIENT BORDER ===
        DS::Effect::drawGradientBorder(g, bounds.reduced(0.5f), 
                                       accent.withAlpha(0.6f), 
                                       accent.withAlpha(0.2f));
        
        // === HEADER AREA ===
        auto headerBounds = bounds.removeFromTop((float)DS::Size::SectionHeaderHeight);
        
        // Header gradient background
        juce::ColourGradient headerGradient(
            accent.withAlpha(0.15f), headerBounds.getTopLeft(),
            accent.withAlpha(0.05f), headerBounds.getBottomLeft(), false
        );
        g.setGradientFill(headerGradient);
        g.fillRoundedRectangle(headerBounds.withTrimmedBottom(-DS::Radius::L), DS::Radius::L);
        
        // Accent line at bottom of header (only when expanded)
        if (animatedExpandAmount > 0.1f)
        {
            g.setColour(accent.withAlpha(animatedExpandAmount));
            g.fillRect(headerBounds.getX() + (float)DS::Space::M, 
                       headerBounds.getBottom() - 2.0f,
                       headerBounds.getWidth() - (float)DS::Space::M * 2.0f, 2.0f);
        }
        
        // Icon dot
        g.setColour(accent);
        g.fillEllipse(headerBounds.getX() + (float)DS::Space::L, 
                      headerBounds.getCentreY() - 4.0f, 8.0f, 8.0f);
        
        // Section title
        g.setColour(DS::Color::textPrimary());
        g.setFont(DS::Font::sectionTitle());
        g.drawText(sectionTitle, 
                   headerBounds.withTrimmedLeft((float)DS::Space::L + 16.0f).toNearestInt(), 
                   juce::Justification::centredLeft);
        
        // === COLLAPSE/EXPAND CHEVRON ===
        if (collapsible)
        {
            auto chevronBounds = headerBounds.removeFromRight(40.0f).reduced(10.0f);
            
            // Draw animated chevron arrow
            g.setColour(DS::Color::textSecondary().withAlpha(isMouseOverHeader ? 1.0f : 0.6f));
            
            juce::Path chevron;
            float cx = chevronBounds.getCentreX();
            float cy = chevronBounds.getCentreY();
            float size = 6.0f;
            
            // Rotate based on expanded state
            float angle = animatedExpandAmount * juce::MathConstants<float>::pi;
            
            // Point down when collapsed (0), point up when expanded (pi)
            chevron.startNewSubPath(cx - size, cy - size * 0.5f + animatedExpandAmount * size);
            chevron.lineTo(cx, cy + size * 0.5f - animatedExpandAmount * size);
            chevron.lineTo(cx + size, cy - size * 0.5f + animatedExpandAmount * size);
            
            g.strokePath(chevron, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            
            // Hover highlight on chevron area
            if (isMouseOverHeader)
            {
                g.setColour(accent.withAlpha(0.1f));
                g.fillEllipse(chevronBounds.expanded(4.0f));
            }
        }
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        expandedHeight = bounds.getHeight();
        collapsedHeight = DS::Size::SectionHeaderHeight + DS::Space::S;
        
        bounds.removeFromTop(DS::Size::SectionHeaderHeight + DS::Space::S);
        bounds.reduce(DS::Size::CardPadding, DS::Space::M);
        contentBounds = bounds;
        
        // Update child visibility based on collapse state
        updateChildVisibility();
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (!collapsible) return;
        
        // Check if click is in header area
        auto headerBounds = getLocalBounds().removeFromTop(DS::Size::SectionHeaderHeight);
        if (headerBounds.contains(e.getPosition()))
        {
            setExpanded(!isExpanded);
        }
    }
    
    void mouseMove(const juce::MouseEvent& e) override
    {
        if (!collapsible) return;
        
        auto headerBounds = getLocalBounds().removeFromTop(DS::Size::SectionHeaderHeight);
        bool wasOverHeader = isMouseOverHeader;
        isMouseOverHeader = headerBounds.contains(e.getPosition());
        
        if (wasOverHeader != isMouseOverHeader)
            repaint();
    }
    
    void mouseExit(const juce::MouseEvent&) override
    {
        if (isMouseOverHeader)
        {
            isMouseOverHeader = false;
            repaint();
        }
    }
    
    // Returns content bounds in PARENT coordinates (for children that are siblings, not our children)
    juce::Rectangle<int> getContentBounds() const { 
        // Ensure we have valid bounds before returning
        if (getWidth() <= 0 || getHeight() <= 0)
            return {};
        
        // Recalculate contentBounds based on current size to ensure it's up-to-date
        auto bounds = getLocalBounds();
        bounds.removeFromTop(DS::Size::SectionHeaderHeight + DS::Space::S);
        bounds.reduce(DS::Size::CardPadding, DS::Space::M);
        
        // Return in parent coordinates
        return bounds.translated(getX(), getY()); 
    }
    
    // Returns content bounds in LOCAL coordinates (for children added to this section)
    juce::Rectangle<int> getLocalContentBounds() const { 
        if (getWidth() <= 0 || getHeight() <= 0)
            return {};
        return contentBounds; 
    }
    
    juce::Colour getAccentColor() const { return accent; }
    void setAccentColor(juce::Colour newColor) { accent = newColor; repaint(); }
    
    // Collapse/Expand API
    void setCollapsible(bool canCollapse) { collapsible = canCollapse; repaint(); }
    bool isCollapsible() const { return collapsible; }
    
    void setExpanded(bool shouldBeExpanded)
    {
        if (isExpanded == shouldBeExpanded) return;
        isExpanded = shouldBeExpanded;
        targetExpandAmount = isExpanded ? 1.0f : 0.0f;
        startTimerHz(60);  // Animate at 60fps
        
        if (onCollapseStateChanged)
            onCollapseStateChanged(isExpanded);
    }
    
    bool getExpanded() const { return isExpanded; }
    
    int getCollapsedHeight() const { return collapsedHeight; }
    int getExpandedHeight() const { return expandedHeight; }
    int getCurrentAnimatedHeight() const 
    { 
        return collapsedHeight + static_cast<int>((expandedHeight - collapsedHeight) * animatedExpandAmount);
    }
    
    // Callback for parent to update layout
    std::function<void(bool expanded)> onCollapseStateChanged;
    
private:
    void timerCallback() override
    {
        // Smooth animation
        float diff = targetExpandAmount - animatedExpandAmount;
        if (std::abs(diff) < 0.01f)
        {
            animatedExpandAmount = targetExpandAmount;
            stopTimer();
        }
        else
        {
            animatedExpandAmount += diff * 0.2f;  // Smooth easing
        }
        
        updateChildVisibility();
        repaint();
        
        // Notify parent to update layout
        if (auto* parent = getParentComponent())
            parent->resized();
    }
    
    void updateChildVisibility()
    {
        // Hide children when collapsed (below threshold)
        bool showChildren = animatedExpandAmount > 0.3f;
        for (auto* child : getChildren())
        {
            child->setVisible(showChildren);
            child->setAlpha(animatedExpandAmount);
        }
    }
    
    juce::String sectionTitle;
    juce::Colour accent;
    juce::Rectangle<int> contentBounds;
    
    // Collapse/Expand state
    bool collapsible = true;
    bool isExpanded = true;
    bool isMouseOverHeader = false;
    float animatedExpandAmount = 1.0f;  // 0.0 = collapsed, 1.0 = expanded
    float targetExpandAmount = 1.0f;
    int expandedHeight = 200;
    int collapsedHeight = 40;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlowingSection)
};
