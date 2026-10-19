#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

class MKS70LookAndFeel : public juce::LookAndFeel_V4 {
public:
    MKS70LookAndFeel() {
        // Set base colors using new DS namespace
        setColour(juce::ResizableWindow::backgroundColourId, DS::Color::bg());
        setColour(juce::TextButton::buttonColourId, DS::Color::bgElevated());
        setColour(juce::TextButton::textColourOnId, DS::Color::textPrimary());
        setColour(juce::TextButton::textColourOffId, DS::Color::textPrimary());
        setColour(juce::ComboBox::backgroundColourId, DS::Color::bgInput());
        setColour(juce::ComboBox::textColourId, DS::Color::textPrimary());
        setColour(juce::ComboBox::outlineColourId, DS::Color::border());
        setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        setColour(juce::TextEditor::textColourId, DS::Color::textPrimary());
        setColour(juce::TextEditor::outlineColourId, DS::Color::border());
        setColour(juce::Label::textColourId, DS::Color::textPrimary());
        setColour(juce::Slider::textBoxTextColourId, DS::Color::textPrimary());
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::TabbedComponent::backgroundColourId, DS::Color::bg());
        setColour(juce::TabbedComponent::outlineColourId, DS::Color::border());
    }

    //==========================================================================
    // 🎚️ VERTICAL SLIDER - סליידר אנכי עם glow
    //==========================================================================
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override 
    {
        const bool isVertical = style == juce::Slider::LinearVertical ||
                                style == juce::Slider::LinearBarVertical;
        const bool isEnabled = slider.isEnabled();
        
        // Get accent color from component property or use default
        juce::Colour accent = slider.findColour(juce::Slider::thumbColourId);
        if (accent == juce::Colour()) accent = DS::Color::Accent::primary();
        
        if (isVertical) {
            drawVerticalSlider(g, x, y, width, height, sliderPos, accent, isEnabled);
        } else {
            drawHorizontalSlider(g, x, y, width, height, sliderPos, accent, isEnabled);
        }
    }
    
    void drawVerticalSlider(juce::Graphics& g, int x, int y, int width, int height,
                            float sliderPos, juce::Colour accent, bool isEnabled)
    {
        const float trackWidth = 6.0f;
        const float trackX = x + (width - trackWidth) / 2.0f;
        
        juce::Rectangle<float> track(trackX, (float)y + 8.0f, trackWidth, (float)height - 16.0f);
        juce::Rectangle<float> filled(trackX, sliderPos, trackWidth, ((float)y + (float)height - 8.0f) - sliderPos);
        
        // Track background with inner shadow effect
        g.setColour(DS::Color::bgInput());
        g.fillRoundedRectangle(track, DS::Radius::S);
        
        // Inner shadow
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawRoundedRectangle(track.reduced(0.5f), DS::Radius::S, 1.0f);
        
        // Filled portion with gradient
        if (isEnabled && filled.getHeight() > 0) {
            juce::ColourGradient fillGradient(
                accent.brighter(0.2f), filled.getTopLeft(),
                accent.darker(0.2f), filled.getBottomLeft(), false
            );
            g.setGradientFill(fillGradient);
            g.fillRoundedRectangle(filled, DS::Radius::S);
            
            // Glow effect
            g.setColour(accent.withAlpha(0.3f));
            g.fillRoundedRectangle(filled.expanded(2.0f, 0.0f), DS::Radius::S);
        }
        
        // Thumb
        const float thumbSize = 18.0f;
        juce::Rectangle<float> thumb(
            (float)x + ((float)width - thumbSize) / 2.0f, 
            sliderPos - thumbSize / 2.0f,
            thumbSize, thumbSize
        );
        
        // Thumb glow
        if (isEnabled) {
            DS::Effect::drawGlow(g, thumb, accent, DS::Effect::GlowSmall);
        }
        
        // Thumb body with gradient
        juce::ColourGradient thumbGradient(
            juce::Colours::white.withAlpha(0.95f), thumb.getTopLeft(),
            juce::Colours::white.withAlpha(0.8f), thumb.getBottomLeft(), false
        );
        g.setGradientFill(thumbGradient);
        g.fillRoundedRectangle(thumb, DS::Radius::Full);
        
        // Thumb border
        g.setColour(accent);
        g.drawRoundedRectangle(thumb.reduced(0.5f), DS::Radius::Full, 2.0f);
    }
    
    void drawHorizontalSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, juce::Colour accent, bool isEnabled)
    {
        const float trackHeight = 6.0f;
        const float trackY = (float)y + ((float)height - trackHeight) / 2.0f;
        
        juce::Rectangle<float> track((float)x + 8.0f, trackY, (float)width - 16.0f, trackHeight);
        juce::Rectangle<float> filled((float)x + 8.0f, trackY, sliderPos - ((float)x + 8.0f), trackHeight);
        
        // Track
        g.setColour(DS::Color::bgInput());
        g.fillRoundedRectangle(track, DS::Radius::S);
        
        // Filled with gradient
        if (isEnabled && filled.getWidth() > 0) {
            juce::ColourGradient fillGradient(
                accent.darker(0.2f), filled.getTopLeft(),
                accent.brighter(0.2f), filled.getTopRight(), false
            );
            g.setGradientFill(fillGradient);
            g.fillRoundedRectangle(filled, DS::Radius::S);
        }
        
        // Thumb
        const float thumbSize = 18.0f;
        juce::Rectangle<float> thumb(
            sliderPos - thumbSize / 2.0f,
            (float)y + ((float)height - thumbSize) / 2.0f,
            thumbSize, thumbSize
        );
        
        if (isEnabled) {
            DS::Effect::drawGlow(g, thumb, accent, DS::Effect::GlowSmall);
        }
        
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillEllipse(thumb);
        g.setColour(accent);
        g.drawEllipse(thumb.reduced(1.0f), 2.0f);
    }

    //==========================================================================
    // 🎛️ ROTARY KNOB - נוב עם glow ועומק
    //==========================================================================
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override 
    {
        juce::Colour accent = slider.findColour(juce::Slider::thumbColourId);
        if (accent == juce::Colour()) accent = DS::Color::Accent::primary();
        
        const float radius = (float)juce::jmin(width / 2, height / 2) - 6.0f;
        const float centreX = (float)x + (float)width * 0.5f;
        const float centreY = (float)y + (float)height * 0.5f;
        const float angle = rotaryStartAngle + sliderPosProportional * 
                           (rotaryEndAngle - rotaryStartAngle);
        
        // === OUTER SHADOW - Subtle drop shadow (REDUCED) ===
        for (int i = 3; i >= 1; --i) {
            float shadowRadius = radius + (float)i * 1.5f;
            float alpha = 0.08f * (float)(4 - i);
            g.setColour(juce::Colours::black.withAlpha(alpha));
            g.fillEllipse(centreX - shadowRadius, centreY - shadowRadius + 2.0f, 
                         shadowRadius * 2.0f, shadowRadius * 2.0f);
        }
        
        // === OUTER GLOW - Accent color glow based on value (REDUCED) ===
        if (slider.isEnabled() && sliderPosProportional > 0.01f) {
            for (float i = DS::Effect::GlowMedium; i > 0; i -= 1.5f) {
                float glowAlpha = (1.0f - i / DS::Effect::GlowMedium) * 0.08f * sliderPosProportional;
                g.setColour(accent.withAlpha(glowAlpha));
                g.fillEllipse(centreX - radius - i, centreY - radius - i,
                             (radius + i) * 2.0f, (radius + i) * 2.0f);
            }
        }
        
        // === KNOB BODY - Main circle with metallic gradient ===
        juce::ColourGradient bodyGradient(
            DS::Color::bgElevated().brighter(0.15f), centreX, centreY - radius,
            DS::Color::bgElevated().darker(0.25f), centreX, centreY + radius, false);
        bodyGradient.addColour(0.5, DS::Color::bgElevated());
        g.setGradientFill(bodyGradient);
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
        
        // === OUTER RING - Metallic edge ===
        juce::ColourGradient ringGradient(
            juce::Colour(0xFF555565), centreX, centreY - radius,
            juce::Colour(0xFF202030), centreX, centreY + radius, false);
        g.setGradientFill(ringGradient);
        g.drawEllipse(centreX - radius + 1.5f, centreY - radius + 1.5f, 
                      (radius - 1.5f) * 2.0f, (radius - 1.5f) * 2.0f, 2.5f);
        
        // === TOP HIGHLIGHT - 3D shine ===
        juce::Path topShine;
        topShine.addEllipse(centreX - radius * 0.6f, centreY - radius * 0.9f,
                            radius * 1.2f, radius * 0.4f);
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillPath(topShine);
        
        // === ARC TRACK (background) ===
        juce::Path arcBg;
        const float arcRadius = radius - 6.0f;
        arcBg.addCentredArc(centreX, centreY, arcRadius, arcRadius,
                            0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(DS::Color::bgInput());
        g.strokePath(arcBg, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, 
                                                  juce::PathStrokeType::rounded));
        
        // === ARC (filled portion) with gradient & glow ===
        if (slider.isEnabled() && sliderPosProportional > 0.001f) {
            juce::Path arc;
            arc.addCentredArc(centreX, centreY, arcRadius, arcRadius,
                              0.0f, rotaryStartAngle, angle, true);
            
            // Arc glow
            g.setColour(accent.withAlpha(0.4f));
            g.strokePath(arc, juce::PathStrokeType(9.0f, juce::PathStrokeType::curved, 
                                                    juce::PathStrokeType::rounded));
            
            // Main arc with gradient
            juce::ColourGradient arcGradient(
                accent.brighter(0.3f), centreX - radius, centreY,
                accent.darker(0.2f), centreX + radius, centreY, false
            );
            g.setGradientFill(arcGradient);
            g.strokePath(arc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, 
                                                    juce::PathStrokeType::rounded));
        }
        
        // === CENTER CAP - Raised inner circle ===
        const float capRadius = radius * 0.38f;
        
        // Cap shadow
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillEllipse(centreX - capRadius + 1.0f, centreY - capRadius + 2.0f, 
                      capRadius * 2.0f, capRadius * 2.0f);
        
        // Cap gradient
        juce::ColourGradient capGradient(
            juce::Colour(0xFF454555), centreX - capRadius * 0.4f, centreY - capRadius * 0.6f,
            juce::Colour(0xFF252535), centreX + capRadius * 0.4f, centreY + capRadius * 0.6f, true);
        g.setGradientFill(capGradient);
        g.fillEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);
        
        // Cap rim
        g.setColour(juce::Colour(0xFF606070));
        g.drawEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);
        
        // Cap highlight
        juce::Path capHighlight;
        capHighlight.addEllipse(centreX - capRadius * 0.45f, centreY - capRadius * 0.75f,
                                capRadius * 0.9f, capRadius * 0.35f);
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.fillPath(capHighlight);
        
        // === CENTER DOT ===
        g.setColour(accent);
        g.fillEllipse(centreX - 4.0f, centreY - 4.0f, 8.0f, 8.0f);
        
        // === POINTER LINE ===
        juce::Path pointer;
        const float pointerLength = radius * 0.45f;
        const float pointerThickness = 3.0f;
        
        pointer.addRoundedRectangle(-pointerThickness / 2.0f, -radius + 10.0f,
                                     pointerThickness, pointerLength, 1.5f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle)
                              .translated(centreX, centreY));
        
        // Pointer glow
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.strokePath(pointer, juce::PathStrokeType(5.0f));
        
        // Pointer body
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillPath(pointer);
    }

    //==========================================================================
    // 🔘 BUTTONS - כפתורים עם gradient
    //==========================================================================
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                              const juce::Colour& /*backgroundColour*/,
                              bool shouldDrawButtonAsHighlighted, 
                              bool shouldDrawButtonAsDown) override 
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        
        juce::Colour bg = DS::Color::bgElevated();
        juce::Colour border = DS::Color::border();
        
        if (button.getToggleState()) {
            bg = DS::Color::Accent::primary();
            border = DS::Color::Accent::primaryGlow();
            
            // Glow for active buttons
            DS::Effect::drawGlow(g, bounds, DS::Color::Accent::primary(), 
                                 DS::Effect::GlowSmall);
        } else if (shouldDrawButtonAsDown) {
            bg = DS::Color::Accent::primaryDark();
        } else if (shouldDrawButtonAsHighlighted) {
            bg = DS::Color::bgHover();
            border = DS::Color::borderLight();
        }
        
        // Background with subtle gradient
        juce::ColourGradient bgGradient(
            bg.brighter(0.05f), bounds.getTopLeft(),
            bg.darker(0.05f), bounds.getBottomLeft(), false
        );
        g.setGradientFill(bgGradient);
        g.fillRoundedRectangle(bounds, DS::Radius::M);
        
        // Border
        g.setColour(border);
        g.drawRoundedRectangle(bounds, DS::Radius::M, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, 
                        bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
    {
        auto bounds = button.getLocalBounds();
        
        g.setFont(DS::Font::button());
        g.setColour(button.getToggleState() ? juce::Colours::white : DS::Color::textPrimary());
        g.drawFittedText(button.getButtonText(), bounds, juce::Justification::centred, 1);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int /*buttonHeight*/) override {
        return DS::Font::button();
    }

    //==========================================================================
    // 📋 COMBOBOX - תפריט נפתח מודרני וברור
    //==========================================================================
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                      juce::ComboBox& box) override 
    {
        juce::Rectangle<float> bounds(0.0f, 0.0f, (float)width, (float)height);
        
        // Background with subtle gradient for depth
        juce::ColourGradient bgGrad(
            DS::Color::bgInput().brighter(0.05f), bounds.getTopLeft(),
            DS::Color::bgInput(), bounds.getBottomLeft(), false
        );
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, DS::Radius::S);
        
        // Border - accent color when focused, muted otherwise
        juce::Colour borderCol = box.hasKeyboardFocus(true) ? 
            DS::Color::Accent::primary() : 
            (isButtonDown ? DS::Color::Accent::primary().withAlpha(0.7f) : DS::Color::border());
        g.setColour(borderCol);
        g.drawRoundedRectangle(bounds.reduced(0.5f), DS::Radius::S, 1.0f);
        
        // Arrow area with subtle highlight
        juce::Rectangle<float> arrowZone((float)width - 22.0f, 2.0f, 20.0f, (float)height - 4.0f);
        if (isButtonDown) {
            g.setColour(DS::Color::Accent::primary().withAlpha(0.1f));
            g.fillRoundedRectangle(arrowZone, 3.0f);
        }
        
        // Modern arrow
        juce::Path arrow;
        float arrowX = arrowZone.getCentreX();
        float arrowY = arrowZone.getCentreY();
        float arrowSize = 4.0f;
        arrow.addTriangle(arrowX - arrowSize, arrowY - 2.0f,
                          arrowX + arrowSize, arrowY - 2.0f,
                          arrowX, arrowY + 3.0f);
        
        g.setColour(isButtonDown ? DS::Color::Accent::primary() : DS::Color::textSecondary());
        g.fillPath(arrow);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override {
        return DS::Font::small();
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& /*shortcutKeyText*/,
                           const juce::Drawable* /*icon*/, const juce::Colour* /*textColour*/) override
    {
        if (isSeparator) {
            g.setColour(DS::Color::border().withAlpha(0.5f));
            g.fillRect(area.reduced(8, 0).withHeight(1).withY(area.getCentreY()));
            return;
        }
        
        auto r = area.reduced(4, 1);
        
        if (isHighlighted && isActive) {
            // Gradient highlight for selected item
            juce::ColourGradient highlightGrad(
                DS::Color::Accent::primary().withAlpha(0.25f), r.toFloat().getTopLeft(),
                DS::Color::Accent::primary().withAlpha(0.15f), r.toFloat().getBottomLeft(), false
            );
            g.setGradientFill(highlightGrad);
            g.fillRoundedRectangle(r.toFloat(), DS::Radius::S);
            
            // Left accent bar
            g.setColour(DS::Color::Accent::primary());
            g.fillRoundedRectangle(r.toFloat().withWidth(3.0f), 1.5f);
        }
        
        // Text color
        if (!isActive)
            g.setColour(DS::Color::textDisabled());
        else if (isHighlighted)
            g.setColour(DS::Color::textPrimary());
        else
            g.setColour(DS::Color::textSecondary());
        
        g.setFont(DS::Font::small());
        
        auto textArea = r.reduced(10, 0);
        if (isTicked)
            textArea.removeFromRight(20);
        if (hasSubMenu)
            textArea.removeFromRight(16);
            
        g.drawFittedText(text, textArea, juce::Justification::centredLeft, 1);
        
        // Tick mark for selected item
        if (isTicked) {
            auto tickArea = r.removeFromRight(20).toFloat();
            g.setColour(DS::Color::Accent::success());
            
            juce::Path tick;
            float cx = tickArea.getCentreX();
            float cy = tickArea.getCentreY();
            tick.startNewSubPath(cx - 4, cy);
            tick.lineTo(cx - 1, cy + 3);
            tick.lineTo(cx + 5, cy - 4);
            g.strokePath(tick, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        }
        
        // Submenu arrow
        if (hasSubMenu) {
            auto arrowArea = r.removeFromRight(14).toFloat();
            float cx = arrowArea.getCentreX();
            float cy = arrowArea.getCentreY();
            juce::Path arrow;
            arrow.addTriangle(cx - 2, cy - 4, cx - 2, cy + 4, cx + 3, cy);
            g.setColour(DS::Color::textMuted());
            g.fillPath(arrow);
        }
    }
    
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
        
        // Solid dark background
        g.setColour(DS::Color::bgElevated());
        g.fillRoundedRectangle(bounds, DS::Radius::M);
        
        // Subtle border
        g.setColour(DS::Color::border());
        g.drawRoundedRectangle(bounds.reduced(0.5f), DS::Radius::M, 1.0f);
        
        // Top accent line
        g.setColour(DS::Color::Accent::primary().withAlpha(0.3f));
        g.fillRect(bounds.reduced(8, 0).withHeight(1.0f).withY(1.0f));
    }
    
    void drawPopupMenuSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& area, 
                                     const juce::String& sectionName) override
    {
        g.setColour(DS::Color::Accent::primary().withAlpha(0.7f));
        g.setFont(DS::Font::tiny().boldened());
        g.drawText(sectionName.toUpperCase(), area.reduced(12, 0), 
                   juce::Justification::centredLeft, true);
    }

    //==========================================================================
    // 📑 TABS - כרטיסיות מודרניות עם עומק והצללה
    //==========================================================================
    void drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, 
                       bool isMouseDown) override 
    {
        auto bounds = button.getActiveArea().toFloat();
        const bool isFront = button.isFrontTab();
        
        // Ensure minimum height for text visibility
        bounds = bounds.withHeight(juce::jmax(bounds.getHeight(), 32.0f));
        
        if (isFront) {
            // Active tab with gradient and glow
            juce::ColourGradient bgGradient(
                DS::Color::bgCard().brighter(0.15f), bounds.getTopLeft(),
                DS::Color::bgCard(), bounds.getBottomLeft(), false
            );
            g.setGradientFill(bgGradient);
            g.fillRoundedRectangle(bounds.withTrimmedBottom(2.0f), DS::Radius::M);
            
            // Subtle inner shadow at top
            g.setColour(DS::Color::Accent::primary().withAlpha(0.1f));
            g.fillRect(bounds.reduced(4, 0).withHeight(2.0f));
            
            // Accent line at bottom with glow
            auto accentBounds = juce::Rectangle<float>(
                bounds.getX() + 6.0f, bounds.getBottom() - 3.0f,
                bounds.getWidth() - 12.0f, 3.0f
            );
            
            // Glow under accent
            g.setColour(DS::Color::Accent::primary().withAlpha(0.3f));
            g.fillRoundedRectangle(accentBounds.expanded(2, 1), 2.0f);
            
            // Accent line
            g.setColour(DS::Color::Accent::primary());
            g.fillRoundedRectangle(accentBounds, 1.5f);
            
        } else if (isMouseOver || isMouseDown) {
            // Hover state with subtle highlight
            g.setColour(DS::Color::bgHover().withAlpha(isMouseDown ? 0.8f : 0.5f));
            g.fillRoundedRectangle(bounds.reduced(2.0f), DS::Radius::S);
        }
        
        // Text with shadow for depth - LARGER FONT for visibility
        juce::String text = button.getButtonText();
        auto textBounds = bounds.toNearestInt().reduced(4, 4);
        
        // Use larger font for tab labels
        auto tabFont = juce::Font(14.0f, juce::Font::bold);
        
        if (isFront) {
            // Strong shadow for active tab text
            g.setColour(juce::Colours::black.withAlpha(0.8f));
            g.setFont(tabFont);
            g.drawFittedText(text, textBounds.translated(1, 1), juce::Justification::centred, 1);
            
            // Main text - pure white with slight glow
            g.setColour(juce::Colours::white);
            g.drawFittedText(text, textBounds, juce::Justification::centred, 1);
        } else {
            // Inactive tabs - white text for visibility
            g.setFont(tabFont.withHeight(13.0f));
            g.setColour(isMouseOver ? juce::Colours::white : juce::Colours::white.withAlpha(0.75f));
            g.drawFittedText(text, textBounds, juce::Justification::centred, 1);
        }
    }

    int getTabButtonBestWidth(juce::TabBarButton& button, int /*tabDepth*/) override {
        return juce::jmax(90, DS::Font::tabLabel().getStringWidth(button.getButtonText()) + 32);
    }
    
    void drawTabAreaBehindFrontButton(juce::TabbedButtonBar& /*bar*/, juce::Graphics& g, 
                                       int w, int h) override
    {
        // Subtle gradient background for tab bar
        juce::ColourGradient bgGrad(
            DS::Color::bg().brighter(0.02f), 0, 0,
            DS::Color::bg(), 0, (float)h, false
        );
        g.setGradientFill(bgGrad);
        g.fillRect(0, 0, w, h);
        
        // Bottom border line
        g.setColour(DS::Color::border().withAlpha(0.5f));
        g.fillRect(0, h - 1, w, 1);
    }

    //==========================================================================
    // 🏷️ LABELS
    //==========================================================================
    void drawLabel(juce::Graphics& g, juce::Label& label) override {
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(DS::Font::normal());
        
        auto textArea = label.getLocalBounds();
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, (int)(textArea.getHeight() / DS::Font::normal().getHeight())),
                         label.getMinimumHorizontalScale());
    }

    //==========================================================================
    // ⌨️ TEXT EDITOR - שדות טקסט
    //==========================================================================
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);
        
        g.setColour(editor.hasKeyboardFocus(true) ? 
                    DS::Color::Accent::primary() : DS::Color::border());
        g.drawRoundedRectangle(bounds.reduced(0.5f), DS::Radius::M, 1.0f);
    }

    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& /*editor*/) override {
        g.setColour(DS::Color::bgInput());
        g.fillRoundedRectangle(juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height), DS::Radius::M);
    }

    //==========================================================================
    // 📜 SCROLLBAR - פס גלילה
    //==========================================================================
    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& /*scrollbar*/, int x, int y, int width, int height,
                       bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                       bool isMouseOver, bool isMouseDown) override
    {
        juce::Rectangle<int> thumbBounds;
        
        if (isScrollbarVertical)
            thumbBounds = { x + 2, thumbStartPosition, width - 4, thumbSize };
        else
            thumbBounds = { thumbStartPosition, y + 2, thumbSize, height - 4 };
        
        juce::Colour thumbCol = DS::Color::border();
        if (isMouseDown) thumbCol = DS::Color::Accent::primary();
        else if (isMouseOver) thumbCol = DS::Color::borderLight();
        
        g.setColour(thumbCol);
        g.fillRoundedRectangle(thumbBounds.toFloat(), DS::Radius::Full);
    }

    int getDefaultScrollbarWidth() override { return 10; }
};
