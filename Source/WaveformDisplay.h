#pragma once

#include <JuceHeader.h>
#include <functional>

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay()
    {
        setOpaque(true);
    }

    void setOscillatorParams(int newWaveform, int newPulseWidth, int newCrossMod, int newDetune)
    {
        waveform = juce::jlimit(0, 3, newWaveform);
        pulseWidth = juce::jlimit(0, 127, newPulseWidth);
        crossMod = juce::jlimit(0, 3, newCrossMod);
        detune = juce::jlimit(0, 127, newDetune);
        repaint();
    }

    void setMixerParams(int level1, int level2)
    {
        dco1Level = juce::jlimit(0, 127, level1);
        dco2Level = juce::jlimit(0, 127, level2);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        const int w = getWidth();
        const int h = getHeight();
        if (w <= 2 || h <= 2) return;

        // Grid and zero line
        if (showGrid)
        {
            // Horizontal grid
            g.setColour(juce::Colours::darkgrey.withAlpha(0.35f));
            for (float frac : { 0.25f, 0.75f })
            {
                const int y = (int)std::round(frac * (float)h);
                g.drawLine(0.0f, (float)y, (float)w, (float)y, 1.0f);
            }
            // Zero line (horizontal center)
            g.setColour(juce::Colours::darkgrey.withAlpha(0.6f));
            const int yMid = h / 2;
            g.drawLine(0.0f, (float)yMid, (float)w, (float)yMid, 1.0f);

            // Vertical grid
            g.setColour(juce::Colours::darkgrey.withAlpha(0.35f));
            for (float frac : { 0.25f, 0.75f })
            {
                const int x = (int)std::round(frac * (float)w);
                g.drawLine((float)x, 0.0f, (float)x, (float)h, 1.0f);
            }
            // Midline (vertical center)
            g.setColour(juce::Colours::darkgrey.withAlpha(0.6f));
            const int xMid = w / 2;
            g.drawLine((float)xMid, 0.0f, (float)xMid, (float)h, 1.0f);
        }

        juce::Path wavePath;

        for (int i = 0; i < w; ++i)
        {
            const float phase = (float)i / (float)w * (float)cycles * juce::MathConstants<float>::twoPi;
            const float sample = getSample(phase);
            const float x = (float)i;
            const float y = (1.0f - juce::jlimit(-1.0f, 1.0f, sample)) * 0.5f * (float)h;

            if (i == 0) wavePath.startNewSubPath(x, y);
            else        wavePath.lineTo(x, y);
        }

        g.setColour(juce::Colours::green.withAlpha(0.20f));
        g.strokePath(wavePath, juce::PathStrokeType(4.0f));
        g.setColour(juce::Colours::green);
        g.strokePath(wavePath, juce::PathStrokeType(1.5f));

        // Border
        if (showBorder)
        {
            g.setColour(juce::Colours::dimgrey);
            g.drawRect(getLocalBounds());
        }

        // Title
        if (title.isNotEmpty())
        {
            g.setColour(juce::Colours::lightgrey.withAlpha(0.9f));
            g.setFont(juce::FontOptions(11.0f));
            g.drawText(title, 4, 2, w - 8, 14, juce::Justification::topLeft, false);
        }

        // Wave label (top-right)
        if (showWaveLabel)
        {
            g.setColour(juce::Colours::lightgrey.withAlpha(0.85f));
            g.setFont(juce::FontOptions(11.0f));
            auto name = getWaveName(waveform);
            g.drawText(name, 0, 2, w - 4, 14, juce::Justification::topRight, false);
        }

        // Crosshair + value under cursor
        if (lastMouseX >= 0)
        {
            const float x = (float)lastMouseX;
            const float phase = (float)lastMouseX / (float)w * (float)cycles * juce::MathConstants<float>::twoPi;
            const float sample = juce::jlimit(-1.0f, 1.0f, getSample(phase));
            const float y = (1.0f - sample) * 0.5f * (float)h;
            g.setColour(juce::Colours::white.withAlpha(0.35f));
            g.drawLine(x, 0.0f, x, (float)h, 1.0f);
            g.drawLine(0.0f, y, (float)w, y, 1.0f);
            g.setColour(juce::Colours::white.withAlpha(0.9f));
            juce::String val = juce::String(sample, 2);
            g.setFont(juce::FontOptions(11.0f));
            g.drawText(val, 4, h - 16, 40, 14, juce::Justification::left, false);
        }
    }

    void mouseMove(const juce::MouseEvent& e) override
    {
        lastMouseX = e.getPosition().x;
        repaint();
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        lastMouseX = -1;
        repaint();
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (! e.mods.isRightButtonDown()) return;
        juce::PopupMenu menu;
        // Wave selection if allowed
        if (allowWaveSelection)
        {
            menu.addSectionHeader("Waveform");
            menu.addItem(1, "Noise", true, waveform == 0);
            menu.addItem(2, "Saw",   true, waveform == 1);
            menu.addItem(3, "Pulse", true, waveform == 2);
            menu.addItem(4, "Square",true, waveform == 3);
            menu.addSeparator();
        }
        // View options
        menu.addSectionHeader("View");
        menu.addItem(10, showGrid ? "Hide Grid" : "Show Grid");
        menu.addItem(11, showBorder ? "Hide Border" : "Show Border");
        // Cycles submenu
        juce::PopupMenu cyclesMenu;
        cyclesMenu.addItem(20, "1 cycle", true, cycles == 1);
        cyclesMenu.addItem(21, "2 cycles", true, cycles == 2);
        cyclesMenu.addItem(22, "4 cycles", true, cycles == 4);
        menu.addSubMenu("Cycles", cyclesMenu);

        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result)
        {
            if (result >= 1 && result <= 4)
            {
                const int wf = result - 1; // map 1..4 -> 0..3
                if (onRequestWaveformChange)
                    onRequestWaveformChange(wf);
                return;
            }
            if (result == 10) { showGrid = !showGrid; repaint(); return; }
            if (result == 11) { showBorder = !showBorder; repaint(); return; }
            if (result == 20) { setCycles(1); return; }
            if (result == 21) { setCycles(2); return; }
            if (result == 22) { setCycles(4); return; }
        });
    }

protected:
    virtual float getSample(float phase)
    {
        float s = 0.0f;
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float pw = juce::jmap((float)pulseWidth, 0.0f, 127.0f, 0.05f, juce::MathConstants<float>::pi);
        const float p = std::fmod(phase, twoPi);

        switch (waveform)
        {
            case 1: // Saw
                // from -1..1 ramp
                s = 1.0f - 2.0f * (p / twoPi);
                break;
            case 2: // Pulse
                s = (p < pw ? 1.0f : -1.0f);
                break;
            case 3: // Square
                s = (p < juce::MathConstants<float>::pi ? 1.0f : -1.0f);
                break;
            case 0: // Noise (simplified, deterministic per x via hash)
            default:
            {
                // Use a simple hash of phase to avoid flicker across repaints
                const uint32_t k = (uint32_t)std::floor(phase * 1000.0f) * 2654435761u;
                const float nf = ((k >> 9) & 0xFFFF) / 32767.5f; // 0..2.0
                s = juce::jlimit(-1.0f, 1.0f, nf - 1.0f);
                break;
            }
        }
        return s;
    }

    // State
    int waveform = 1;    // 0=Noise, 1=Saw, 2=Pulse, 3=Square
    int pulseWidth = 64; // 0..127
    int crossMod = 0;    // 0..3 (not used in base)
    int detune = 64;     // 0..127 (not used in base)
    int dco1Level = 127; // 0..127 (for mixer display)
    int dco2Level = 127; // 0..127 (for mixer display)

public:
    void setTitle(const juce::String& t) { title = t; repaint(); }
    void setShowGrid(bool b) { showGrid = b; repaint(); }
    void setShowBorder(bool b) { showBorder = b; repaint(); }
    void setShowWaveLabel(bool b) { showWaveLabel = b; repaint(); }
    void setCycles(int c) { cycles = juce::jlimit(1, 8, c); repaint(); }
    void setAllowWaveSelection(bool b) { allowWaveSelection = b; }
    std::function<void(int)> onRequestWaveformChange;

private:
    juce::String title;
    bool showGrid = true;
    bool showBorder = true;
    bool showWaveLabel = true;
    int cycles = 2; // number of cycles across width
    bool allowWaveSelection = false;
    int lastMouseX = -1;

    static juce::String getWaveName(int wf)
    {
        switch (wf)
        {
            case 0: return "Noise";
            case 1: return "Saw";
            case 2: return "Pulse";
            case 3: return "Square";
            default: return "?";
        }
    }
};
