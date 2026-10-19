#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

class MKS70EditorAudioProcessorEditor;

/**
 * RealisticKnob - Minimoog-style rotary knob with graphics around it
 * 
 * Can display:
 * - Waveform icons for DCO wave selection
 * - Octave numbers (2', 4', 8', 16', 32')
 * - Generic numbered values
 */
class RealisticKnob : public juce::Component, public juce::SettableTooltipClient
{
public:
    enum class KnobType {
        Generic,        // Regular knob with numbers
        Waveform,       // Shows waveform shapes around knob
        Octave,         // Shows octave numbers (2', 4', 8', etc)
        OnOff,          // Simple on/off
        Range           // Range selector (like LFO range)
    };
    
    RealisticKnob(const juce::String& name = "", KnobType type = KnobType::Generic)
        : paramName(name), knobType(type)
    {
        setSize(110, 130);  // Large realistic size for professional look
    }
    
    void setRange(double min, double max, double interval = 1.0)
    {
        minValue = min;
        maxValue = max;
        this->interval = interval;
        numSteps = (int)((max - min) / interval) + 1;
        repaint();
    }
    
    void setValue(double newValue, juce::NotificationType notification = juce::sendNotification)
    {
        newValue = juce::jlimit(minValue, maxValue, newValue);
        if (newValue != currentValue) {
            currentValue = newValue;
            if (notification != juce::dontSendNotification && onChange)
                onChange();
            repaint();
        }
    }
    
    double getValue() const { return currentValue; }
    
    void setItems(const juce::StringArray& items)
    {
        this->items = items;
        if (items.size() > 0) {
            minValue = 0;
            maxValue = items.size() - 1;
            numSteps = items.size();
        }
    }
    
    void setTooltip(const juce::String& tip) { juce::SettableTooltipClient::setTooltip(tip); }
    
    void setParameterName(const juce::String& name) { midiParameterName = name; }
    juce::String getParameterName() const { return midiParameterName; }
    
    void setEditor(class MKS70EditorAudioProcessorEditor* ed) { editor = ed; }
    
    std::function<void()> onChange;
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Draw label at top - compact
        auto labelArea = bounds.removeFromTop(11);
        g.setColour(DS::Color::textSecondary());
        g.setFont(DS::Font::tiny());
        g.drawText(paramName, labelArea, juce::Justification::centred);
        
        bounds.removeFromTop(1);
        
        // Main knob area - Minimoog style compact
        float knobSize = juce::jmin(bounds.getWidth(), bounds.getHeight() - 12) * 0.78f;
        auto knobBounds = bounds.withSizeKeepingCentre(knobSize, knobSize);
        knobBounds.setY(bounds.getY() + 4);
        
        // Draw surrounding indicators/graphics
        drawSurroundingGraphics(g, knobBounds.expanded(8));
        
        // Draw knob body (Minimoog style)
        drawMinimoogKnob(g, knobBounds);
        
        // Draw value below knob - compact
        auto valueArea = bounds.removeFromBottom(11);
        g.setColour(DS::Color::textPrimary());
        g.setFont(DS::Font::tiny());
        g.drawText(getValueText(), valueArea, juce::Justification::centred);
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown()) {
            showContextMenu();
            return;
        }
        dragStartY = e.position.y;
        dragStartValue = currentValue;
        
        // For discrete knobs, also allow click-to-position
        if (numSteps <= 10) {
            handleClickPosition(e);
        }
    }
    
    void mouseDrag(const juce::MouseEvent& e) override
    {
        // Calculate drag sensitivity based on knob type
        float baseSensitivity = (numSteps <= 10) ? 2.0f : 0.5f;
        float sensitivity = (e.mods.isShiftDown()) ? baseSensitivity * 0.2f : baseSensitivity;
        
        float delta = (dragStartY - e.position.y) * sensitivity;
        
        double range = maxValue - minValue;
        double newValue;
        
        if (numSteps <= 10) {
            // For discrete selectors - step-based dragging
            int stepDelta = (int)(delta / 15.0f); // 15 pixels per step
            newValue = dragStartValue + stepDelta * interval;
        } else {
            // For continuous knobs - smooth dragging
            newValue = dragStartValue + (delta / 100.0) * range;
            if (interval > 0) {
                newValue = std::round(newValue / interval) * interval;
            }
        }
        
        setValue(newValue);
    }
    
    void mouseDoubleClick(const juce::MouseEvent&) override
    {
        // Reset to center/default
        setValue((maxValue + minValue) / 2.0);
    }
    
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        if (numSteps <= 10) {
            // Discrete steps for selector knobs
            int step = (wheel.deltaY > 0) ? 1 : -1;
            setValue(currentValue + step * interval);
        } else {
            // Smooth for continuous knobs
            double delta = wheel.deltaY * (maxValue - minValue) * 0.05;
            if (interval > 0 && std::abs(delta) < interval)
                delta = (delta > 0 ? 1 : -1) * interval;
            setValue(currentValue + delta);
        }
    }
    
    void handleClickPosition(const juce::MouseEvent& e)
    {
        // Calculate angle from center
        auto bounds = getLocalBounds().toFloat();
        bounds.removeFromTop(16); // Label area
        float knobSize = juce::jmin(bounds.getWidth(), bounds.getHeight() - 16) * 0.75f;
        auto knobBounds = bounds.withSizeKeepingCentre(knobSize, knobSize);
        knobBounds.setY(bounds.getY() + 8);
        
        float cx = knobBounds.getCentreX();
        float cy = knobBounds.getCentreY();
        float dx = e.position.x - cx;
        float dy = e.position.y - cy;
        
        // Only respond to clicks on the knob area
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > knobSize * 0.8f) {
            // Click was outside knob - use angle to select value
            float angle = std::atan2(dx, -dy); // angle from top
            // Map angle (-135° to +135°) to value
            float startAngle = -2.356f;
            float endAngle = 2.356f;
            float normalized = (angle - startAngle) / (endAngle - startAngle);
            normalized = juce::jlimit(0.0f, 1.0f, normalized);
            double newValue = minValue + normalized * (maxValue - minValue);
            newValue = std::round(newValue / interval) * interval;
            setValue(newValue);
        }
    }
    
    std::function<void()> onRightClick;
    
    void showContextMenu()
    {
        if (onRightClick) {
            onRightClick();
            return;
        }
        
        juce::PopupMenu menu;
        menu.addItem(1, "Reset to Default");
        if (midiParameterName.isNotEmpty() && editor != nullptr) {
            menu.addSeparator();
            menu.addItem(10, "MIDI Learn...");
            menu.addItem(11, "Clear MIDI Learn");
        }
        
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) {
                // Reset to middle value
                setValue((maxValue + minValue) / 2.0);
            }
            else if (result == 10 && editor != nullptr) {
                // Start MIDI Learn mode
                // This will be handled by MidiLearnManager
                DBG("MIDI Learn requested for: " << midiParameterName);
            }
            else if (result == 11 && editor != nullptr) {
                // Clear MIDI mapping
                DBG("Clear MIDI Learn for: " << midiParameterName);
            }
        });
    }
    
private:
    juce::String paramName;
    juce::String midiParameterName;
    MKS70EditorAudioProcessorEditor* editor = nullptr;
    KnobType knobType;
    double minValue = 0, maxValue = 127, interval = 1;
    double currentValue = 64;
    int numSteps = 128;
    juce::StringArray items;
    
    float dragStartY = 0;
    double dragStartValue = 0;
    
    juce::String getValueText() const
    {
        if (!items.isEmpty() && (int)currentValue < items.size())
            return items[(int)currentValue];
        
        switch (knobType) {
            case KnobType::Octave:
                return getOctaveText((int)currentValue);
            case KnobType::OnOff:
                return currentValue > 0.5 ? "ON" : "OFF";
            default:
                return juce::String((int)currentValue);
        }
    }
    
    juce::String getOctaveText(int val) const
    {
        const char* octaves[] = { "32'", "16'", "8'", "4'", "2'" };
        if (val >= 0 && val < 5) return octaves[val];
        return juce::String(val);
    }
    
    void drawMinimoogKnob(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = bounds.getWidth() / 2.0f;
        
        // === OUTER SHADOW (deep professional look) ===
        for (int i = 3; i >= 1; i--) {
            g.setColour(juce::Colour(0x00000000).withAlpha(0.15f * i));
            g.fillEllipse(bounds.translated(0.5f * i, 1.0f * i).expanded(1.5f * i));
        }
        
        // === BEZEL RING (thick chrome edge) ===
        auto bezelBounds = bounds.expanded(1.5f);
        juce::ColourGradient bezelGrad(juce::Colour(0xFF606068), cx, cy - radius,
                                        juce::Colour(0xFF202028), cx, cy + radius, false);
        bezelGrad.addColour(0.3, juce::Colour(0xFF808088));
        bezelGrad.addColour(0.5, juce::Colour(0xFF404048));
        bezelGrad.addColour(0.7, juce::Colour(0xFF303038));
        g.setGradientFill(bezelGrad);
        g.fillEllipse(bezelBounds);
        
        // === OUTER RING (dark brushed metal) ===
        juce::ColourGradient outerGrad(juce::Colour(0xFF454550), cx, cy - radius,
                                        juce::Colour(0xFF18181C), cx, cy + radius, false);
        outerGrad.addColour(0.4, juce::Colour(0xFF2A2A30));
        g.setGradientFill(outerGrad);
        g.fillEllipse(bounds);
        
        // Outer ring highlight
        g.setColour(juce::Colour(0x20FFFFFF));
        g.drawEllipse(bounds.reduced(1), 1.0f);
        
        // === INNER RING (machined groove) ===
        auto innerBounds = bounds.reduced(2.5f);
        g.setColour(juce::Colour(0xFF0A0A0C));
        g.fillEllipse(innerBounds);
        
        // === CAP (main knob surface - premium brushed aluminum) ===
        auto capBounds = bounds.reduced(4);
        float capR = capBounds.getWidth() / 2.0f;
        
        // Dark inset groove
        g.setColour(juce::Colour(0xFF080810));
        g.fillEllipse(capBounds.expanded(1));
        
        // Multi-stop gradient for realistic brushed metal - lighter, more contrast
        juce::ColourGradient capGrad(juce::Colour(0xFF707080), cx - capR * 0.5f, cy - capR * 0.5f,
                                      juce::Colour(0xFF252530), cx + capR * 0.5f, cy + capR * 0.5f, true);
        capGrad.addColour(0.2, juce::Colour(0xFF606070));
        capGrad.addColour(0.4, juce::Colour(0xFF505060));
        capGrad.addColour(0.6, juce::Colour(0xFF353540));
        capGrad.addColour(0.8, juce::Colour(0xFF2A2A35));
        g.setGradientFill(capGrad);
        g.fillEllipse(capBounds);
        
        // === BRUSHED METAL TEXTURE (concentric rings) ===
        g.saveState();
        g.reduceClipRegion(capBounds.toNearestInt());
        for (float r = capR * 0.3f; r < capR; r += 2.0f) {
            g.setColour(juce::Colour(0x08FFFFFF));
            g.drawEllipse(cx - r, cy - r, r * 2, r * 2, 0.5f);
        }
        g.restoreState();
        
        // === SPECULAR HIGHLIGHT (bright glossy reflection) ===
        auto highlightBounds = capBounds.reduced(capR * 0.2f);
        highlightBounds.setY(capBounds.getY() + 1);
        highlightBounds.setHeight(highlightBounds.getHeight() * 0.35f);
        juce::ColourGradient highlightGrad(juce::Colour(0x50FFFFFF), highlightBounds.getCentreX(), highlightBounds.getY(),
                                            juce::Colours::transparentWhite, highlightBounds.getCentreX(), highlightBounds.getBottom(), false);
        g.setGradientFill(highlightGrad);
        g.fillEllipse(highlightBounds);
        
        // === SECONDARY HIGHLIGHT (edge glint) ===
        g.setColour(juce::Colour(0x18FFFFFF));
        g.drawEllipse(capBounds.reduced(2), 1.0f);
        
        // === POINTER (professional indicator line) ===
        float angle = valueToAngle(currentValue);
        float pointerInner = capR * 0.15f;
        float pointerOuter = capR * 0.78f;
        float px1 = cx + std::sin(angle) * pointerInner;
        float py1 = cy - std::cos(angle) * pointerInner;
        float px2 = cx + std::sin(angle) * pointerOuter;
        float py2 = cy - std::cos(angle) * pointerOuter;
        
        // Recessed groove shadow
        g.setColour(juce::Colour(0x60000000));
        g.drawLine(px1 + 0.8f, py1 + 1.2f, px2 + 0.8f, py2 + 1.2f, 4.0f);
        
        // Pointer base (dark recessed groove)
        g.setColour(juce::Colour(0xFF101015));
        g.drawLine(px1, py1, px2, py2, 3.5f);
        
        // Pointer glow (soft outer glow)
        juce::Colour accentCol = getKnobAccentColor();
        g.setColour(accentCol.withAlpha(0.15f));
        g.drawLine(px1, py1, px2, py2, 7.0f);
        g.setColour(accentCol.withAlpha(0.35f));
        g.drawLine(px1, py1, px2, py2, 4.5f);
        
        // Main pointer line (bright)
        g.setColour(accentCol.brighter(0.3f));
        g.drawLine(px1, py1, px2, py2, 2.2f);
        
        // Pointer tip highlight
        g.setColour(accentCol.brighter(0.6f));
        g.fillEllipse(px2 - 2.5f, py2 - 2.5f, 5.0f, 5.0f);
        
        // === CENTER HUB (recessed screw hole) ===
        // Outer dark ring
        g.setColour(juce::Colour(0xFF050508));
        g.fillEllipse(cx - 5.0f, cy - 5.0f, 10.0f, 10.0f);
        // Inner raised area
        juce::ColourGradient centerGrad(juce::Colour(0xFF303038), cx - 3, cy - 3,
                                         juce::Colour(0xFF151518), cx + 3, cy + 3, true);
        g.setGradientFill(centerGrad);
        g.fillEllipse(cx - 3.5f, cy - 3.5f, 7.0f, 7.0f);
        // Center dimple highlight
        g.setColour(juce::Colour(0x20FFFFFF));
        g.fillEllipse(cx - 1.5f, cy - 2.0f, 3.0f, 2.0f);
    }
    
    juce::Colour getKnobAccentColor() const
    {
        switch (knobType) {
            case KnobType::Waveform: return DS::Color::Accent::cyan();
            case KnobType::Octave: return DS::Color::Accent::warning();
            case KnobType::OnOff: return DS::Color::Accent::success();
            default: return DS::Color::Accent::primary();
        }
    }
    
    float valueToAngle(double value) const
    {
        float normalized = (float)(value - minValue) / (float)(maxValue - minValue);
        // -135° to +135° range (270° total)
        float startAngle = -2.356f; // -135° in radians
        float endAngle = 2.356f;    // +135° in radians
        return startAngle + normalized * (endAngle - startAngle);
    }
    
    void drawSurroundingGraphics(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = bounds.getWidth() / 2.0f;
        
        switch (knobType) {
            case KnobType::Waveform:
                drawWaveformIndicators(g, cx, cy, radius);
                break;
            case KnobType::Octave:
                drawOctaveIndicators(g, cx, cy, radius);
                break;
            default:
                drawTickMarks(g, cx, cy, radius);
                break;
        }
    }
    
    void drawWaveformIndicators(juce::Graphics& g, float cx, float cy, float radius)
    {
        // Draw small waveform icons around the knob at specific angles
        // Only 4 positions for typical synth waveforms
        float angles[] = { -2.1f, -0.7f, 0.7f, 2.1f };
        float iconRadius = radius + 8;
        
        for (int i = 0; i < 4 && i < numSteps; i++) {
            float angle = angles[i];
            float iconX = cx + std::sin(angle) * iconRadius;
            float iconY = cy - std::cos(angle) * iconRadius;
            
            bool isSelected = ((int)currentValue == i);
            g.setColour(isSelected ? DS::Color::Accent::cyan() : DS::Color::textMuted().withAlpha(0.5f));
            
            // Draw mini waveform
            drawMiniWaveform(g, iconX, iconY, i, isSelected);
        }
    }
    
    void drawMiniWaveform(juce::Graphics& g, float x, float y, int waveType, bool selected)
    {
        float size = 10.0f;
        juce::Rectangle<float> bounds(x - size/2, y - size/2, size, size);
        
        juce::Path path;
        switch (waveType) {
            case 0: // Noise
                for (int i = 0; i < 6; i++) {
                    float px = bounds.getX() + (i / 5.0f) * bounds.getWidth();
                    float py = bounds.getCentreY() + (juce::Random::getSystemRandom().nextFloat() - 0.5f) * bounds.getHeight() * 0.8f;
                    if (i == 0) path.startNewSubPath(px, py);
                    else path.lineTo(px, py);
                }
                break;
            case 1: // Saw
                path.startNewSubPath(bounds.getX(), bounds.getBottom());
                path.lineTo(bounds.getRight(), bounds.getY());
                path.lineTo(bounds.getRight(), bounds.getBottom());
                break;
            case 2: // Pulse
                path.startNewSubPath(bounds.getX(), bounds.getBottom());
                path.lineTo(bounds.getX(), bounds.getY());
                path.lineTo(bounds.getCentreX() + 2, bounds.getY());
                path.lineTo(bounds.getCentreX() + 2, bounds.getBottom());
                path.lineTo(bounds.getRight(), bounds.getBottom());
                break;
            case 3: // Square
                path.startNewSubPath(bounds.getX(), bounds.getBottom());
                path.lineTo(bounds.getX(), bounds.getY());
                path.lineTo(bounds.getCentreX(), bounds.getY());
                path.lineTo(bounds.getCentreX(), bounds.getBottom());
                path.lineTo(bounds.getRight(), bounds.getBottom());
                break;
        }
        
        g.strokePath(path, juce::PathStrokeType(selected ? 2.0f : 1.0f));
    }
    
    void drawOctaveIndicators(juce::Graphics& g, float cx, float cy, float radius)
    {
        // Draw octave labels only if we have them in items, otherwise use defaults
        const char* defaultOctaves[] = { "32'", "16'", "8'", "4'", "2'" };
        int numItems = items.isEmpty() ? 5 : juce::jmin(5, items.size());
        
        // Spread angles more for better spacing - avoid overlapping
        float angleSpread = 4.2f; // Total angle spread
        float startAngle = -angleSpread / 2;
        
        for (int i = 0; i < numItems && i < numSteps; i++) {
            float t = (float)i / (numItems - 1);
            float angle = startAngle + t * angleSpread;
            
            // Position text further out to avoid overlap
            float textRadius = radius + 10;
            float textX = cx + std::sin(angle) * textRadius;
            float textY = cy - std::cos(angle) * textRadius;
            
            bool isSelected = ((int)currentValue == i);
            g.setColour(isSelected ? DS::Color::Accent::warning() : DS::Color::textMuted().withAlpha(0.6f));
            g.setFont(juce::Font(9.0f).boldened());
            
            juce::String label = items.isEmpty() ? defaultOctaves[i] : items[i];
            auto textBounds = juce::Rectangle<float>(textX - 14, textY - 5, 28, 10);
            g.drawText(label, textBounds, juce::Justification::centred);
            
            // Draw tick mark
            float tickInner = radius - 2;
            float tickOuter = radius + 3;
            float tx1 = cx + std::sin(angle) * tickInner;
            float ty1 = cy - std::cos(angle) * tickInner;
            float tx2 = cx + std::sin(angle) * tickOuter;
            float ty2 = cy - std::cos(angle) * tickOuter;
            g.drawLine(tx1, ty1, tx2, ty2, isSelected ? 2.0f : 1.0f);
        }
    }
    
    void drawTickMarks(juce::Graphics& g, float cx, float cy, float radius)
    {
        int numTicks = juce::jmin(11, numSteps);
        float startAngle = -2.356f;
        float endAngle = 2.356f;
        
        for (int i = 0; i < numTicks; i++) {
            float t = (float)i / (numTicks - 1);
            float angle = startAngle + t * (endAngle - startAngle);
            
            float innerR = radius - 4;
            float outerR = radius;
            
            float x1 = cx + std::sin(angle) * innerR;
            float y1 = cy - std::cos(angle) * innerR;
            float x2 = cx + std::sin(angle) * outerR;
            float y2 = cy - std::cos(angle) * outerR;
            
            g.setColour(DS::Color::textMuted().withAlpha(0.4f));
            g.drawLine(x1, y1, x2, y2, 1.0f);
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RealisticKnob)
};


/**
 * LabeledRealisticKnob - Wrapper with consistent sizing
 */
class LabeledRealisticKnob : public juce::Component
{
public:
    LabeledRealisticKnob(const juce::String& name, RealisticKnob::KnobType type = RealisticKnob::KnobType::Generic)
        : knob(name, type)
    {
        addAndMakeVisible(knob);
    }
    
    void resized() override
    {
        // All knobs same size: 50x62 (compact Minimoog style)
        auto bounds = getLocalBounds();
        int knobW = 50;
        int knobH = 62;
        knob.setBounds(bounds.withSizeKeepingCentre(knobW, knobH));
    }
    
    RealisticKnob& getKnob() { return knob; }
    
    void setRange(double min, double max, double interval = 1.0) { knob.setRange(min, max, interval); }
    void setValue(double v, juce::NotificationType n = juce::sendNotification) { knob.setValue(v, n); }
    double getValue() const { return knob.getValue(); }
    void setItems(const juce::StringArray& items) { knob.setItems(items); }
    
    std::function<void()> onChange;
    
private:
    RealisticKnob knob;
    
    void childrenChanged() override
    {
        knob.onChange = [this]() { if (onChange) onChange(); };
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabeledRealisticKnob)
};
