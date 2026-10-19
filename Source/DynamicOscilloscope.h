#pragma once
#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * DynamicOscilloscope - Advanced multi-view visualization component
 * 
 * Features:
 * - Multiple view modes: Envelope, Spectrogram, Filter, Oscillator, Audio Out
 * - Single or split view layouts (like Cubase)
 * - Auto-switches view based on parameter being edited
 * - Real-time animation
 */
class DynamicOscilloscope : public juce::Component, public juce::Timer
{
public:
    // Available visualization modes
    enum class ViewMode {
        Oscillator,   // DCO waveform display
        Filter,       // VCF frequency response curve
        Envelope1,    // ENV1 (VCF) ADSR curve
        Envelope2,    // ENV2 (VCA) ADSR curve
        Spectrogram,  // Frequency spectrum analyzer
        AudioOut      // Final mixed output waveform
    };
    
    // Layout modes
    enum class LayoutMode {
        Single,       // One view fills entire area
        Split2H,      // Two views horizontal
        Split2V,      // Two views vertical
        Split4        // Four views grid
    };
    
    DynamicOscilloscope()
    {
        // Initialize spectrum data
        for (int i = 0; i < numBands; i++) {
            spectrumBands[i] = 0.0f;
            spectrumPeaks[i] = 0.0f;
        }
        
        // Setup view selector buttons
        setupViewButtons();
        
        // Setup zoom control
        setupZoomControl();
        
        startTimerHz(60); // Smooth 60fps animation
    }
    
    ~DynamicOscilloscope() override { stopTimer(); }
    
    //==========================================================================
    // View Control
    //==========================================================================
    
    void setViewMode(ViewMode mode, int slot = 0) {
        if (slot >= 0 && slot < 4) {
            activeViews[slot] = mode;
            repaint();
        }
    }
    
    ViewMode getViewMode(int slot = 0) const {
        return (slot >= 0 && slot < 4) ? activeViews[slot] : ViewMode::Oscillator;
    }
    
    void setLayoutMode(LayoutMode mode) {
        layoutMode = mode;
        resized();
        repaint();
    }
    
    LayoutMode getLayoutMode() const { return layoutMode; }
    
    // Auto-switch based on parameter category
    void focusOnCategory(const juce::String& category) {
        if (category.containsIgnoreCase("DCO") || category.containsIgnoreCase("OSC")) {
            setViewMode(ViewMode::Oscillator, 0);
        }
        else if (category.containsIgnoreCase("VCF") || category.containsIgnoreCase("FILTER")) {
            setViewMode(ViewMode::Filter, 0);
        }
        else if (category.containsIgnoreCase("ENV1")) {
            setViewMode(ViewMode::Envelope1, 0);
        }
        else if (category.containsIgnoreCase("ENV2") || category.containsIgnoreCase("VCA")) {
            setViewMode(ViewMode::Envelope2, 0);
        }
        else if (category.containsIgnoreCase("LFO") || category.containsIgnoreCase("MIX")) {
            setViewMode(ViewMode::AudioOut, 0);
        }
    }
    
    //==========================================================================
    // Parameter Updates
    //==========================================================================
    
    // DCO Parameters
    void setDCO1Wave(int wave) { dco1Wave = wave; }
    void setDCO2Wave(int wave) { dco2Wave = wave; }
    void setDCO1Level(float v) { dco1Level = v / 127.0f; }
    void setDCO2Level(float v) { dco2Level = v / 127.0f; }
    void setDCO1Tune(float v) { dco1Tune = v; }
    void setDCO2Tune(float v) { dco2Tune = v; }
    void setDCO1Range(int r) { dco1Range = r; }
    void setDCO2Range(int r) { dco2Range = r; }
    
    // VCF Parameters
    void setCutoff(float v) { vcfCutoff = v / 127.0f; }
    void setResonance(float v) { vcfReso = v / 127.0f; }
    void setVCFEnvMod(float v) { vcfEnvMod = v / 127.0f; }
    void setVCFKeyFollow(float v) { vcfKeyFollow = v / 127.0f; }
    void setHighPassCutoff(float v) { hpfCutoff = v / 127.0f; }
    
    // LFO Parameters
    void setLFORate(float v) { lfoRate = v / 127.0f; }
    void setLFOWave(int w) { lfoWave = w; }
    void setLFODelay(float v) { lfoDelay = v / 127.0f; }
    
    // ENV1 (VCF Envelope)
    void setEnv1Attack(float v) { env1A = v / 127.0f; }
    void setEnv1Decay(float v) { env1D = v / 127.0f; }
    void setEnv1Sustain(float v) { env1S = v / 127.0f; }
    void setEnv1Release(float v) { env1R = v / 127.0f; }
    void setEnv1KeyFollow(float v) { env1KeyFollow = v / 127.0f; }
    
    // ENV2 (VCA Envelope)
    void setEnv2Attack(float v) { env2A = v / 127.0f; }
    void setEnv2Decay(float v) { env2D = v / 127.0f; }
    void setEnv2Sustain(float v) { env2S = v / 127.0f; }
    void setEnv2Release(float v) { env2R = v / 127.0f; }
    
    // Mixer
    void setMixerLevel(float v) { mixerLevel = v / 127.0f; }
    
    //==========================================================================
    // Component overrides
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Main background - clean dark
        g.setColour(juce::Colour(0xFF0A0A0C));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Subtle border
        g.setColour(juce::Colour(0xFF1A1A20));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
        
        // Small toolbar area (22px height) - minimal
        auto toolbar = bounds.removeFromTop(22);
        
        // Draw views based on layout
        auto viewArea = bounds.reduced(1);
        
        switch (layoutMode) {
            case LayoutMode::Single:
                drawView(g, viewArea, activeViews[0]);
                break;
                
            case LayoutMode::Split2H: {
                auto left = viewArea.removeFromLeft(viewArea.getWidth() * 0.5f - 1);
                viewArea.removeFromLeft(2);
                drawView(g, left, activeViews[0]);
                drawView(g, viewArea, activeViews[1]);
                break;
            }
            
            case LayoutMode::Split2V: {
                auto top = viewArea.removeFromTop(viewArea.getHeight() * 0.5f - 1);
                viewArea.removeFromTop(2);
                drawView(g, top, activeViews[0]);
                drawView(g, viewArea, activeViews[1]);
                break;
            }
            
            case LayoutMode::Split4: {
                float hw = viewArea.getWidth() * 0.5f - 1;
                float hh = viewArea.getHeight() * 0.5f - 1;
                drawView(g, juce::Rectangle<float>(viewArea.getX(), viewArea.getY(), hw, hh), activeViews[0]);
                drawView(g, juce::Rectangle<float>(viewArea.getX() + hw + 2, viewArea.getY(), hw, hh), activeViews[1]);
                drawView(g, juce::Rectangle<float>(viewArea.getX(), viewArea.getY() + hh + 2, hw, hh), activeViews[2]);
                drawView(g, juce::Rectangle<float>(viewArea.getX() + hw + 2, viewArea.getY() + hh + 2, hw, hh), activeViews[3]);
                break;
            }
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        
        // === VERTICAL ZOOM SLIDER on RIGHT edge - minimum height 50px ===
        auto zoomArea = bounds.removeFromRight(20);  // Slightly wider for easier grabbing
        zoomInBtn.setBounds(zoomArea.removeFromTop(20).reduced(1));
        zoomOutBtn.setBounds(zoomArea.removeFromBottom(20).reduced(1));
        // Slider takes the remaining space
        if (zoomArea.getHeight() > 10) {
            zoomSlider.setBounds(zoomArea.reduced(3, 2));
            zoomSlider.setVisible(true);
        }
        
        // Toolbar at top (minimal)
        auto toolbar = bounds.removeFromTop(20).reduced(3, 2);
        
        // Left side: Menu button only
        menuButton.setBounds(toolbar.removeFromLeft(20).reduced(1));
        
        // Hide layout and view buttons (accessed via menu)
        for (int i = 0; i < 4; i++)
            layoutButtons[i].setVisible(false);
        for (int i = 0; i < 6; i++)
            if (viewButtons[i])
                viewButtons[i]->setVisible(false);
        zoomFitBtn.setVisible(false);
    }
    
    void timerCallback() override
    {
        // Update animation phase
        phase += 0.08f;
        if (phase > juce::MathConstants<float>::twoPi * 10)
            phase -= juce::MathConstants<float>::twoPi * 10;
        
        // Update spectrum data
        updateSpectrumData();
        
        repaint();
    }
    
private:
    //==========================================================================
    // Graphical View Button with icon drawing
    //==========================================================================
    class ViewIconButton : public juce::Component
    {
    public:
        enum IconType { WaveIcon, FilterIcon, Env1Icon, Env2Icon, SpectrumIcon, OutputIcon };
        
        ViewIconButton(IconType type, const juce::String& tip, juce::Colour col)
            : iconType(type), color(col)
        {
            setTooltip(tip);
        }
        
        void setTooltip(const juce::String& tip) { tooltip = tip; }
        void setActive(bool a) { active = a; repaint(); }
        bool isActive() const { return active; }
        std::function<void()> onClick;
        
        void paint(juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced(1);
            
            // Background
            if (active)
                g.setColour(color.withAlpha(0.25f));
            else if (hovering)
                g.setColour(juce::Colour(0xFF252530));
            else
                g.setColour(juce::Colour(0xFF1E1E28));
            
            g.fillRoundedRectangle(b, 3.0f);
            
            // Border
            if (active)
            {
                g.setColour(color.withAlpha(0.6f));
                g.drawRoundedRectangle(b, 3.0f, 1.0f);
            }
            
            // Draw icon
            g.setColour(active ? color : color.withAlpha(0.5f));
            auto iconArea = b.reduced(4);
            
            switch (iconType)
            {
                case WaveIcon:
                    drawWaveIcon(g, iconArea);
                    break;
                case FilterIcon:
                    drawFilterIcon(g, iconArea);
                    break;
                case Env1Icon:
                    drawEnvIcon(g, iconArea, juce::Colour(0xFF10B981));
                    break;
                case Env2Icon:
                    drawEnvIcon(g, iconArea, juce::Colour(0xFFA855F7));
                    break;
                case SpectrumIcon:
                    drawSpectrumIcon(g, iconArea);
                    break;
                case OutputIcon:
                    drawOutputIcon(g, iconArea);
                    break;
            }
        }
        
        void mouseEnter(const juce::MouseEvent&) override { hovering = true; repaint(); }
        void mouseExit(const juce::MouseEvent&) override { hovering = false; repaint(); }
        void mouseUp(const juce::MouseEvent& e) override
        {
            if (getLocalBounds().contains(e.getPosition()) && onClick)
                onClick();
        }
        
    private:
        IconType iconType;
        juce::Colour color;
        juce::String tooltip;
        bool active = false;
        bool hovering = false;
        
        void drawWaveIcon(juce::Graphics& g, juce::Rectangle<float> b)
        {
            // Sine wave icon
            juce::Path p;
            float cy = b.getCentreY();
            float amp = b.getHeight() * 0.35f;
            for (int i = 0; i <= (int)b.getWidth(); i++)
            {
                float x = b.getX() + i;
                float t = (float)i / b.getWidth() * juce::MathConstants<float>::twoPi * 2.0f;
                float y = cy - std::sin(t) * amp;
                if (i == 0) p.startNewSubPath(x, y);
                else p.lineTo(x, y);
            }
            g.strokePath(p, juce::PathStrokeType(1.5f));
        }
        
        void drawFilterIcon(juce::Graphics& g, juce::Rectangle<float> b)
        {
            // Low-pass filter curve icon
            juce::Path p;
            p.startNewSubPath(b.getX(), b.getCentreY() - b.getHeight() * 0.3f);
            p.lineTo(b.getCentreX(), b.getCentreY() - b.getHeight() * 0.3f);
            p.quadraticTo(b.getRight() - 2, b.getCentreY(), b.getRight(), b.getBottom() - 2);
            g.strokePath(p, juce::PathStrokeType(1.5f));
        }
        
        void drawEnvIcon(juce::Graphics& g, juce::Rectangle<float> b, juce::Colour c)
        {
            g.setColour(active ? c : c.withAlpha(0.5f));
            // ADSR shape
            juce::Path p;
            float x = b.getX();
            float bottom = b.getBottom();
            float top = b.getY() + 2;
            float w = b.getWidth();
            
            p.startNewSubPath(x, bottom);  // Start
            p.lineTo(x + w * 0.15f, top);  // Attack
            p.lineTo(x + w * 0.35f, b.getCentreY());  // Decay to Sustain
            p.lineTo(x + w * 0.7f, b.getCentreY());  // Sustain hold
            p.lineTo(x + w, bottom);  // Release
            
            g.strokePath(p, juce::PathStrokeType(1.5f));
        }
        
        void drawSpectrumIcon(juce::Graphics& g, juce::Rectangle<float> b)
        {
            // Spectrum bars icon
            float barW = b.getWidth() / 7.0f;
            float heights[] = { 0.3f, 0.6f, 0.9f, 0.7f, 0.5f, 0.4f, 0.2f };
            for (int i = 0; i < 7; i++)
            {
                float h = b.getHeight() * heights[i];
                g.fillRect(b.getX() + i * barW + 1, b.getBottom() - h, barW - 2, h);
            }
        }
        
        void drawOutputIcon(juce::Graphics& g, juce::Rectangle<float> b)
        {
            // Complex waveform (mixed output)
            juce::Path p;
            float cy = b.getCentreY();
            float amp = b.getHeight() * 0.35f;
            for (int i = 0; i <= (int)b.getWidth(); i++)
            {
                float x = b.getX() + i;
                float t = (float)i / b.getWidth() * juce::MathConstants<float>::twoPi * 3.0f;
                float y = cy - (std::sin(t) + std::sin(t * 2.1f) * 0.5f) * amp * 0.5f;
                if (i == 0) p.startNewSubPath(x, y);
                else p.lineTo(x, y);
            }
            g.strokePath(p, juce::PathStrokeType(1.5f));
        }
    };
    
    //==========================================================================
    // Menu Button - Small icon that opens view/layout menu
    //==========================================================================
    class MenuIconButton : public juce::Component
    {
    public:
        MenuIconButton() {}
        std::function<void()> onClick;
        
        void paint(juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced(3);
            g.setColour(hovering ? juce::Colour(0xFF6366F1) : juce::Colour(0xFF606070));
            
            // Three horizontal lines (hamburger menu icon)
            float lineH = 2.0f;
            float spacing = (b.getHeight() - lineH * 3) / 2;
            for (int i = 0; i < 3; i++)
            {
                float y = b.getY() + i * (lineH + spacing);
                g.fillRoundedRectangle(b.getX(), y, b.getWidth(), lineH, 1.0f);
            }
        }
        
        void mouseEnter(const juce::MouseEvent&) override { hovering = true; repaint(); }
        void mouseExit(const juce::MouseEvent&) override { hovering = false; repaint(); }
        void mouseUp(const juce::MouseEvent& e) override
        {
            if (getLocalBounds().contains(e.getPosition()) && onClick)
                onClick();
        }
    private:
        bool hovering = false;
    };
    
    // Layout and view state
    LayoutMode layoutMode = LayoutMode::Single;
    ViewMode activeViews[4] = { ViewMode::AudioOut, ViewMode::Spectrogram, ViewMode::Envelope1, ViewMode::Filter };
    
    // Menu button (replaces many toolbar buttons)
    MenuIconButton menuButton;
    
    // Buttons (hidden, accessed via menu)
    juce::TextButton layoutButtons[4];
    std::unique_ptr<ViewIconButton> viewButtons[6];
    
    // Zoom controls (visible on right side)
    juce::TextButton zoomInBtn, zoomOutBtn, zoomFitBtn;
    juce::Slider zoomSlider;
    float zoomLevel = 1.0f;  // 0.25 to 4.0
    float verticalScale = 1.0f;  // For auto-fit
    
    // Animation
    float phase = 0.0f;
    
    // Spectrum analyzer
    static constexpr int numBands = 32;
    float spectrumBands[numBands];
    float spectrumPeaks[numBands];
    
    // Synth parameters
    int dco1Wave = 1, dco2Wave = 1, lfoWave = 3;
    int dco1Range = 2, dco2Range = 2;
    float dco1Level = 0.8f, dco2Level = 0.6f;
    float dco1Tune = 64.0f, dco2Tune = 64.0f;
    float vcfCutoff = 0.7f, vcfReso = 0.3f, vcfEnvMod = 0.5f;
    float vcfKeyFollow = 0.5f, hpfCutoff = 0.0f;
    float lfoRate = 0.3f, lfoDelay = 0.0f;
    float env1A = 0.1f, env1D = 0.3f, env1S = 0.7f, env1R = 0.4f, env1KeyFollow = 0.5f;
    float env2A = 0.05f, env2D = 0.2f, env2S = 0.8f, env2R = 0.3f;
    float mixerLevel = 0.8f;
    
    // Filter state
    float filterState1 = 0.0f, filterState2 = 0.0f;
    
    //==========================================================================
    // Setup
    //==========================================================================
    
    void setupViewButtons()
    {
        // Menu button - opens dropdown with all view/layout options
        menuButton.onClick = [this]() { showViewMenu(); };
        addAndMakeVisible(menuButton);
        
        // Layout buttons (hidden, accessed via menu)
        const char* layoutLabels[] = { "1", "2H", "2V", "4" };
        const char* layoutTips[] = { "Single View", "Split Horizontal", "Split Vertical", "Quad View" };
        for (int i = 0; i < 4; i++) {
            layoutButtons[i].setButtonText(layoutLabels[i]);
            layoutButtons[i].setTooltip(layoutTips[i]);
            layoutButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1E1E28));
            layoutButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF808090));
            layoutButtons[i].onClick = [this, i]() { 
                setLayoutMode(static_cast<LayoutMode>(i));
                updateButtonStates();
            };
            addChildComponent(layoutButtons[i]); // Hidden by default
        }
        
        // View buttons (hidden, accessed via menu)
        const char* viewTips[] = { 
            "Oscillator - DCO Waveforms", 
            "Filter - VCF Response", 
            "Envelope 1 - VCF ADSR", 
            "Envelope 2 - VCA ADSR", 
            "Spectrum - Frequency Analyzer", 
            "Output - Mixed Waveform" 
        };
        const juce::Colour viewColors[] = {
            juce::Colour(0xFF22D3EE),  // Cyan - Wave
            juce::Colour(0xFFF59E0B),  // Amber - Filter  
            juce::Colour(0xFF10B981),  // Green - ENV1
            juce::Colour(0xFFA855F7),  // Purple - ENV2
            juce::Colour(0xFF6366F1),  // Indigo - Spectrum
            juce::Colour(0xFFEC4899)   // Pink - Output
        };
        
        for (int i = 0; i < 6; i++) {
            viewButtons[i] = std::make_unique<ViewIconButton>(
                static_cast<ViewIconButton::IconType>(i),
                viewTips[i],
                viewColors[i]
            );
            viewButtons[i]->onClick = [this, i]() {
                setViewMode(static_cast<ViewMode>(i), 0);
                updateButtonStates();
            };
            addChildComponent(viewButtons[i].get()); // Hidden by default
        }
        
        updateButtonStates();
    }
    
    void setupZoomControl()
    {
        // Fit button - auto-scales to show entire signal
        zoomFitBtn.setButtonText("FIT");
        zoomFitBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1A1A20));
        zoomFitBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF10B981));
        zoomFitBtn.setTooltip("Auto-fit to show full signal");
        zoomFitBtn.onClick = [this]() { 
            zoomLevel = 1.0f;
            verticalScale = 0.5f;  // Reduce vertical to fit peaks
            zoomSlider.setValue(1.0, juce::dontSendNotification);
            repaint();
        };
        addAndMakeVisible(zoomFitBtn);
        
        // Zoom out
        zoomOutBtn.setButtonText("-");
        zoomOutBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1A1A20));
        zoomOutBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF808090));
        zoomOutBtn.onClick = [this]() { 
            zoomLevel = juce::jmax(0.25f, zoomLevel - 0.25f);
            verticalScale = 1.0f / zoomLevel;
            zoomSlider.setValue(zoomLevel, juce::dontSendNotification);
            repaint();
        };
        addAndMakeVisible(zoomOutBtn);
        
        // Zoom slider - VERTICAL on right edge
        zoomSlider.setRange(0.25, 2.0, 0.05);
        zoomSlider.setValue(1.0);
        zoomSlider.setSliderStyle(juce::Slider::LinearVertical);
        zoomSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        zoomSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xFF2D2D3A));
        zoomSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xFF6366F1));
        zoomSlider.onValueChange = [this]() {
            zoomLevel = (float)zoomSlider.getValue();
            verticalScale = 1.0f / zoomLevel;  // Lower zoom = more compressed vertically
            repaint();
        };
        addAndMakeVisible(zoomSlider);
        
        // Zoom in
        zoomInBtn.setButtonText("+");
        zoomInBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1A1A20));
        zoomInBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF808090));
        zoomInBtn.onClick = [this]() {
            zoomLevel = juce::jmin(2.0f, zoomLevel + 0.25f);
            verticalScale = 1.0f / zoomLevel;
            zoomSlider.setValue(zoomLevel, juce::dontSendNotification);
            repaint();
        };
        addAndMakeVisible(zoomInBtn);
    }
    
    void updateButtonStates()
    {
        // Update layout button highlighting
        for (int i = 0; i < 4; i++) {
            bool isActive = (static_cast<int>(layoutMode) == i);
            layoutButtons[i].setColour(juce::TextButton::buttonColourId, 
                isActive ? juce::Colour(0xFF6366F1).withAlpha(0.3f) : juce::Colour(0xFF1E1E28));
            layoutButtons[i].setColour(juce::TextButton::textColourOffId,
                isActive ? juce::Colour(0xFF6366F1) : juce::Colour(0xFF606070));
        }
        
        // Update view button highlighting (using graphical buttons)
        for (int i = 0; i < 6; i++) {
            if (viewButtons[i]) {
                bool isActive = (static_cast<int>(activeViews[0]) == i);
                viewButtons[i]->setActive(isActive);
            }
        }
        
        repaint();
    }
    
    void showViewMenu()
    {
        juce::PopupMenu menu;
        
        // View selection submenu
        juce::PopupMenu viewMenu;
        viewMenu.addItem(1, "Oscillator - DCO Waveforms", true, activeViews[0] == ViewMode::Oscillator);
        viewMenu.addItem(2, "Filter - VCF Response", true, activeViews[0] == ViewMode::Filter);
        viewMenu.addItem(3, "Envelope 1 - VCF ADSR", true, activeViews[0] == ViewMode::Envelope1);
        viewMenu.addItem(4, "Envelope 2 - VCA ADSR", true, activeViews[0] == ViewMode::Envelope2);
        viewMenu.addItem(5, "Spectrum Analyzer", true, activeViews[0] == ViewMode::Spectrogram);
        viewMenu.addItem(6, "Audio Output", true, activeViews[0] == ViewMode::AudioOut);
        menu.addSubMenu("View", viewMenu);
        
        // Layout submenu
        juce::PopupMenu layoutMenu;
        layoutMenu.addItem(10, "Single View", true, layoutMode == LayoutMode::Single);
        layoutMenu.addItem(11, "Split Horizontal", true, layoutMode == LayoutMode::Split2H);
        layoutMenu.addItem(12, "Split Vertical", true, layoutMode == LayoutMode::Split2V);
        layoutMenu.addItem(13, "Quad View", true, layoutMode == LayoutMode::Split4);
        menu.addSubMenu("Layout", layoutMenu);
        
        menu.addSeparator();
        menu.addItem(20, "Auto-Fit Zoom");
        
        menu.showMenuAsync(juce::PopupMenu::Options()
            .withTargetComponent(&menuButton)
            .withMinimumWidth(150),
            [this](int result) {
                if (result >= 1 && result <= 6) {
                    setViewMode(static_cast<ViewMode>(result - 1), 0);
                    updateButtonStates();
                }
                else if (result >= 10 && result <= 13) {
                    setLayoutMode(static_cast<LayoutMode>(result - 10));
                    updateButtonStates();
                }
                else if (result == 20) {
                    zoomLevel = 1.0f;
                    verticalScale = 0.5f;
                    zoomSlider.setValue(1.0, juce::dontSendNotification);
                    repaint();
                }
            });
    }
    
    //==========================================================================
    // View Drawing
    //==========================================================================
    
    void drawView(juce::Graphics& g, juce::Rectangle<float> bounds, ViewMode mode)
    {
        // View background
        g.setColour(juce::Colour(0xFF0C0C10));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Grid lines
        drawGrid(g, bounds);
        
        // Draw specific view
        switch (mode) {
            case ViewMode::Oscillator:
                drawOscillatorView(g, bounds);
                break;
            case ViewMode::Filter:
                drawFilterView(g, bounds);
                break;
            case ViewMode::Envelope1:
                drawEnvelopeView(g, bounds, env1A, env1D, env1S, env1R, DS::Color::Accent::success());
                break;
            case ViewMode::Envelope2:
                drawEnvelopeView(g, bounds, env2A, env2D, env2S, env2R, DS::Color::Accent::magenta());
                break;
            case ViewMode::Spectrogram:
                drawSpectrogramView(g, bounds);
                break;
            case ViewMode::AudioOut:
                drawAudioOutView(g, bounds);
                break;
        }
        
        // View label
        drawViewLabel(g, bounds, mode);
    }
    
    void drawGrid(juce::Graphics& g, juce::Rectangle<float> b)
    {
        g.setColour(juce::Colour(0xFF151518));
        
        // Vertical lines
        for (int i = 1; i < 8; i++) {
            float x = b.getX() + b.getWidth() * i / 8.0f;
            g.drawVerticalLine((int)x, b.getY(), b.getBottom());
        }
        
        // Horizontal lines
        for (int i = 1; i < 4; i++) {
            float y = b.getY() + b.getHeight() * i / 4.0f;
            g.drawHorizontalLine((int)y, b.getX(), b.getRight());
        }
    }
    
    void drawViewLabel(juce::Graphics& g, juce::Rectangle<float> bounds, ViewMode mode)
    {
        const char* labels[] = { "OSCILLATOR", "FILTER", "ENV 1 (VCF)", "ENV 2 (VCA)", "SPECTRUM", "OUTPUT" };
        const juce::Colour colors[] = {
            DS::Color::Accent::cyan(),
            DS::Color::Accent::warning(),
            DS::Color::Accent::success(),
            DS::Color::Accent::magenta(),
            DS::Color::Accent::primary(),
            juce::Colour(0xFF10B981)
        };
        
        int idx = static_cast<int>(mode);
        g.setColour(colors[idx].withAlpha(0.6f));
        g.setFont(DS::Font::tiny());
        g.drawText(labels[idx], bounds.reduced(6, 4), juce::Justification::topLeft);
    }
    
    //==========================================================================
    // Oscillator View
    //==========================================================================
    
    void drawOscillatorView(juce::Graphics& g, juce::Rectangle<float> b)
    {
        b = b.reduced(8, 20);
        if (b.getWidth() < 10) return;

        // Zoom affects amplitude and number of cycles
        float zoom = zoomLevel;

        // Draw DCO1 waveform
        juce::Path path1;
        int n = (int)b.getWidth();
        float centerY = b.getCentreY();
        float amp = b.getHeight() * (0.35f + 0.25f * (zoom - 1.0f)); // amplitude increases with zoom

        float cycles = 3.0f * zoom; // more cycles when zoomed in
        for (int i = 0; i < n; i++) {
            float t = (float)i / n * juce::MathConstants<float>::twoPi * cycles + phase * 0.3f;
            float sample = calcOsc(dco1Wave, t) * dco1Level;
            float y = centerY - amp * 0.5f - sample * amp * 0.4f;

            if (i == 0) path1.startNewSubPath(b.getX() + i, y);
            else path1.lineTo(b.getX() + i, y);
        }

        // DCO1 with glow
        g.setColour(DS::Color::Accent::cyan().withAlpha(0.1f));
        g.strokePath(path1, juce::PathStrokeType(5.0f));
        g.setColour(DS::Color::Accent::cyan().withAlpha(0.7f));
        g.strokePath(path1, juce::PathStrokeType(1.5f));

        // Draw DCO2 waveform
        juce::Path path2;
        float pitchRatio = std::pow(2.0f, (dco2Tune - 64.0f) / 48.0f);

        for (int i = 0; i < n; i++) {
            float t = (float)i / n * juce::MathConstants<float>::twoPi * cycles * pitchRatio + phase * 0.25f;
            float sample = calcOsc(dco2Wave, t) * dco2Level;
            float y = centerY + amp * 0.5f - sample * amp * 0.4f;

            if (i == 0) path2.startNewSubPath(b.getX() + i, y);
            else path2.lineTo(b.getX() + i, y);
        }

        // DCO2 with glow
        g.setColour(DS::Color::Accent::magenta().withAlpha(0.1f));
        g.strokePath(path2, juce::PathStrokeType(5.0f));
        g.setColour(DS::Color::Accent::magenta().withAlpha(0.7f));
        g.strokePath(path2, juce::PathStrokeType(1.5f));

        // Labels
        g.setFont(DS::Font::tiny());
        g.setColour(DS::Color::Accent::cyan().withAlpha(0.5f));
        g.drawText("DCO1: " + getWaveName(dco1Wave), b.removeFromTop(12), juce::Justification::topRight);
        g.setColour(DS::Color::Accent::magenta().withAlpha(0.5f));
        g.drawText("DCO2: " + getWaveName(dco2Wave), b.removeFromBottom(12), juce::Justification::bottomRight);
    }
    
    juce::String getWaveName(int wave) {
        switch (wave) {
            case 0: return "NOISE";
            case 1: return "SAW";
            case 2: return "PULSE";
            case 3: return "SQUARE";
            default: return "SINE";
        }
    }
    
    float calcOsc(int wave, float p) {
        float n = std::fmod(p, juce::MathConstants<float>::twoPi);
        if (n < 0) n += juce::MathConstants<float>::twoPi;
        float pi = juce::MathConstants<float>::pi;
        
        switch (wave) {
            case 0: return juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
            case 1: return (n / pi) - 1.0f;
            case 2: return n < pi * 1.2f ? 0.9f : -0.9f;
            case 3: return n < pi ? 0.9f : -0.9f;
            default: return std::sin(p);
        }
    }
    
    //==========================================================================
    // Filter View
    //==========================================================================
    
    void drawFilterView(juce::Graphics& g, juce::Rectangle<float> b)
    {
        b = b.reduced(8, 20);
        if (b.getWidth() < 10) return;
        
        juce::Path path;
        int n = (int)b.getWidth();
        
        // Apply zoom-based vertical scaling to prevent clipping
        float vScale = verticalScale * 0.65f;  // Additional reduction for resonance peaks
        
        // Draw frequency response curve
        for (int i = 0; i < n; i++) {
            float freq = (float)i / n;  // 0-1 frequency
            
            // Calculate filter response
            float response = 1.0f;
            
            // HPF effect
            if (hpfCutoff > 0.01f) {
                float hpfDist = freq - hpfCutoff * 0.3f;
                if (hpfDist < 0) {
                    response *= std::exp(hpfDist * 8.0f);
                }
            }
            
            // LPF effect - animate with LFO
            float animCutoff = vcfCutoff + std::sin(phase * 0.15f) * lfoRate * 0.1f;
            animCutoff = juce::jlimit(0.05f, 0.95f, animCutoff);
            
            float lpfDist = freq - animCutoff;
            if (lpfDist > 0) {
                response *= std::exp(-lpfDist * 6.0f);
            }
            
            // Resonance peak - scaled to prevent clipping
            float resoPeak = std::exp(-std::pow(freq - animCutoff, 2.0f) * 50.0f);
            response += resoPeak * vcfReso * vcfReso * 2.0f;
            
            // Apply vertical scale to keep within bounds
            response = juce::jlimit(0.0f, 1.0f / vScale, response) * vScale;
            
            float x = b.getX() + i;
            float y = b.getBottom() - response * b.getHeight() * 0.7f;
            
            if (i == 0) path.startNewSubPath(x, y);
            else path.lineTo(x, y);
        }
        
        // Close path for fill
        juce::Path fillPath = path;
        fillPath.lineTo(b.getRight(), b.getBottom());
        fillPath.lineTo(b.getX(), b.getBottom());
        fillPath.closeSubPath();
        
        // Fill with gradient
        juce::ColourGradient grad(DS::Color::Accent::warning().withAlpha(0.3f), b.getTopLeft(),
                                  DS::Color::Accent::warning().withAlpha(0.02f), b.getBottomLeft(), false);
        g.setGradientFill(grad);
        g.fillPath(fillPath);
        
        // Outer glow
        g.setColour(DS::Color::Accent::warning().withAlpha(0.1f));
        g.strokePath(path, juce::PathStrokeType(6.0f));
        
        // Main line
        g.setColour(DS::Color::Accent::warning().withAlpha(0.9f));
        g.strokePath(path, juce::PathStrokeType(2.0f));
        
        // Cutoff indicator line
        float cutoffX = b.getX() + vcfCutoff * b.getWidth();
        g.setColour(DS::Color::Accent::warning().withAlpha(0.4f));
        g.drawVerticalLine((int)cutoffX, b.getY(), b.getBottom());
        
        // Labels
        g.setFont(DS::Font::tiny());
        g.setColour(DS::Color::textMuted());
        g.drawText("20Hz", b.removeFromLeft(30), juce::Justification::bottomLeft);
        g.drawText("20kHz", b.removeFromRight(35), juce::Justification::bottomRight);
        
        // Cutoff value
        g.setColour(DS::Color::Accent::warning().withAlpha(0.6f));
        int cutoffHz = (int)(20.0f + vcfCutoff * vcfCutoff * 19980.0f);
        g.drawText("Cutoff: " + juce::String(cutoffHz) + "Hz  Reso: " + juce::String((int)(vcfReso * 100)) + "%",
                   getLocalBounds().reduced(8, 0).removeFromTop(40), juce::Justification::topRight);
    }
    
    //==========================================================================
    // Envelope View
    //==========================================================================
    
    void drawEnvelopeView(juce::Graphics& g, juce::Rectangle<float> b, 
                          float a, float d, float s, float r, juce::Colour color)
    {
        b = b.reduced(10, 25);
        if (b.getWidth() < 10) return;
        
        // Calculate envelope points
        float totalTime = a + d + 0.3f + r;  // Attack + Decay + Sustain hold + Release
        float w = b.getWidth();
        float h = b.getHeight();
        
        float attackX = (a / totalTime) * w;
        float decayX = ((a + d) / totalTime) * w;
        float sustainX = ((a + d + 0.3f) / totalTime) * w;
        float releaseX = w;
        
        // Build envelope path
        juce::Path path;
        path.startNewSubPath(b.getX(), b.getBottom());
        
        // Attack - curved rise
        float attackY = b.getY();
        path.quadraticTo(b.getX() + attackX * 0.3f, b.getBottom() - h * 0.2f,
                         b.getX() + attackX, attackY);
        
        // Decay - exponential fall to sustain
        float sustainY = b.getBottom() - s * h;
        path.quadraticTo(b.getX() + attackX + (decayX - attackX) * 0.5f, attackY + (sustainY - attackY) * 0.3f,
                         b.getX() + decayX, sustainY);
        
        // Sustain hold
        path.lineTo(b.getX() + sustainX, sustainY);
        
        // Release - exponential fall
        path.quadraticTo(b.getX() + sustainX + (releaseX - sustainX) * 0.4f, sustainY + (b.getBottom() - sustainY) * 0.5f,
                         b.getX() + releaseX, b.getBottom());
        
        // Fill
        juce::Path fillPath = path;
        fillPath.lineTo(b.getRight(), b.getBottom());
        fillPath.lineTo(b.getX(), b.getBottom());
        fillPath.closeSubPath();
        
        juce::ColourGradient grad(color.withAlpha(0.25f), b.getTopLeft(),
                                  color.withAlpha(0.02f), b.getBottomLeft(), false);
        g.setGradientFill(grad);
        g.fillPath(fillPath);
        
        // Glow
        g.setColour(color.withAlpha(0.1f));
        g.strokePath(path, juce::PathStrokeType(6.0f));
        
        // Main line
        g.setColour(color.withAlpha(0.9f));
        g.strokePath(path, juce::PathStrokeType(2.0f));
        
        // Animated playhead
        float playheadPos = std::fmod(phase * 0.15f, 1.0f);
        float playheadX = b.getX() + playheadPos * w;
        g.setColour(color.withAlpha(0.5f));
        g.drawVerticalLine((int)playheadX, b.getY(), b.getBottom());
        
        // ADSR Labels
        g.setFont(DS::Font::tiny());
        g.setColour(color.withAlpha(0.5f));
        g.drawText("A:" + juce::String((int)(a * 127)), b.removeFromLeft(w * 0.25f), juce::Justification::centredBottom);
        g.drawText("D:" + juce::String((int)(d * 127)), b.removeFromLeft(w * 0.25f), juce::Justification::centredBottom);
        g.drawText("S:" + juce::String((int)(s * 127)), b.removeFromLeft(w * 0.25f), juce::Justification::centredBottom);
        g.drawText("R:" + juce::String((int)(r * 127)), b, juce::Justification::centredBottom);
    }
    
    //==========================================================================
    // Spectrogram View
    //==========================================================================
    
    void drawSpectrogramView(juce::Graphics& g, juce::Rectangle<float> b)
    {
        b = b.reduced(4, 18);
        if (b.getWidth() < 10 || b.getHeight() < 10) return;
        
        float barWidth = b.getWidth() / numBands;
        float gap = 2.0f;
        
        for (int i = 0; i < numBands; i++) {
            float x = b.getX() + i * barWidth;
            float level = juce::jlimit(0.0f, 1.0f, spectrumBands[i]);
            float barHeight = level * b.getHeight() * 0.95f;
            
            // Color gradient from cyan to magenta based on frequency
            float hue = 0.5f - (float)i / numBands * 0.35f;
            juce::Colour barColor = juce::Colour::fromHSV(hue, 0.75f, 0.95f, 1.0f);
            
            juce::Rectangle<float> bar(x + gap * 0.5f, 
                                        b.getBottom() - barHeight, 
                                        barWidth - gap, 
                                        barHeight);
            
            // Gradient fill
            juce::ColourGradient grad(barColor.withAlpha(0.9f), bar.getBottomLeft(),
                                      barColor.withAlpha(0.3f), bar.getTopLeft(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bar, 2.0f);
            
            // Glow
            g.setColour(barColor.withAlpha(0.15f));
            g.fillRoundedRectangle(bar.expanded(2, 0), 3.0f);
            
            // Peak marker
            float peakY = b.getBottom() - spectrumPeaks[i] * b.getHeight() * 0.95f;
            if (peakY < b.getBottom() - 3) {
                g.setColour(barColor.brighter(0.5f).withAlpha(0.9f));
                g.fillRect(x + gap * 0.5f, peakY - 2, barWidth - gap, 2.0f);
            }
        }
        
        // Frequency labels
        g.setColour(DS::Color::textMuted().withAlpha(0.4f));
        g.setFont(DS::Font::tiny());
        auto labelArea = b;
        g.drawText("50", labelArea.removeFromLeft(25), juce::Justification::bottomLeft);
        g.drawText("500", labelArea.withWidth(30).translated(labelArea.getWidth() * 0.3f, 0), juce::Justification::centredBottom);
        g.drawText("2k", labelArea.withWidth(25).translated(labelArea.getWidth() * 0.6f, 0), juce::Justification::centredBottom);
        g.drawText("16k", labelArea.removeFromRight(30), juce::Justification::bottomRight);
    }
    
    void updateSpectrumData()
    {
        // Decay peaks slowly
        for (int i = 0; i < numBands; i++) {
            spectrumPeaks[i] *= 0.97f;
        }
        
        float baseLevel = (dco1Level + dco2Level) * 0.5f;
        
        for (int i = 0; i < numBands; i++) {
            float freq = (float)(i + 1) / numBands;
            float level = 0.0f;
            
            // Generate spectrum based on oscillator waves
            if (dco1Wave == 1)  // Saw - all harmonics
                level += dco1Level * (0.9f / (i + 1));
            else if (dco1Wave == 2 || dco1Wave == 3)  // Pulse/Square - odd harmonics
                if (i % 2 == 0) level += dco1Level * (0.8f / (i + 1));
            else if (dco1Wave == 0)  // Noise - flat
                level += dco1Level * 0.35f * (juce::Random::getSystemRandom().nextFloat() * 0.5f + 0.5f);
            
            if (dco2Wave == 1)
                level += dco2Level * (0.8f / (i + 1));
            else if (dco2Wave == 2 || dco2Wave == 3)
                if (i % 2 == 0) level += dco2Level * (0.7f / (i + 1));
            else if (dco2Wave == 0)
                level += dco2Level * 0.3f * (juce::Random::getSystemRandom().nextFloat() * 0.5f + 0.5f);
            
            // Apply VCF effect
            float cutoffBand = vcfCutoff * numBands;
            float cutoffDist = (float)i - cutoffBand;
            
            if (cutoffDist > 0)
                level *= std::exp(-cutoffDist * 0.35f);
            
            // Resonance peak
            float resoPeak = std::exp(-cutoffDist * cutoffDist / (5.0f + (1.0f - vcfReso) * 25.0f));
            level += vcfReso * vcfReso * 1.8f * resoPeak;
            
            // Animation
            level *= 0.75f + 0.25f * std::sin(phase * 0.08f + i * 0.15f);
            
            // Smooth
            spectrumBands[i] = spectrumBands[i] * 0.75f + level * 0.25f;
            
            if (spectrumBands[i] > spectrumPeaks[i])
                spectrumPeaks[i] = spectrumBands[i];
        }
    }
    
    //==========================================================================
    // Audio Output View
    //==========================================================================
    
    void drawAudioOutView(juce::Graphics& g, juce::Rectangle<float> b)
    {
        b = b.reduced(8, 18);
        if (b.getWidth() < 10) return;
        
        juce::Path path;
        int n = (int)b.getWidth();
        float centerY = b.getCentreY();
        float amp = b.getHeight() * 0.4f;
        
        // Reset filter state
        filterState1 = 0.0f;
        filterState2 = 0.0f;
        
        float pitchRatio2 = std::pow(2.0f, (dco2Tune - 64.0f) / 48.0f);
        float lfoVal = std::sin(phase * 0.18f) * lfoRate * 0.15f;
        float envVal = env1S * 0.85f + 0.15f;
        float vcaEnv = env2S * 0.75f + 0.25f;
        
        for (int i = 0; i < n; i++) {
            float t = (float)i / n * juce::MathConstants<float>::twoPi * 4.0f;
            
            // Mix oscillators
            float osc1 = calcOsc(dco1Wave, t + phase * 0.4f) * dco1Level;
            float osc2 = calcOsc(dco2Wave, (t + phase * 0.35f) * pitchRatio2) * dco2Level;
            float mix = (osc1 + osc2) * 0.55f;
            
            // Apply filter
            float dynCutoff = juce::jlimit(0.03f, 0.97f, vcfCutoff + envVal * vcfEnvMod * 0.4f + lfoVal);
            float filtered = applyFilter(mix, dynCutoff, vcfReso);
            
            // Apply VCA
            float sample = filtered * vcaEnv * mixerLevel;
            
            float x = b.getX() + i;
            float y = centerY - sample * amp;
            
            if (i == 0) path.startNewSubPath(x, y);
            else path.lineTo(x, y);
        }
        
        // Green/emerald color for output
        juce::Colour outColor(0xFF10B981);
        
        // Outer glow
        g.setColour(outColor.withAlpha(0.08f));
        g.strokePath(path, juce::PathStrokeType(8.0f));
        
        // Medium glow
        g.setColour(outColor.withAlpha(0.2f));
        g.strokePath(path, juce::PathStrokeType(3.5f));
        
        // Main line
        g.setColour(outColor.withAlpha(0.9f));
        g.strokePath(path, juce::PathStrokeType(1.8f));
        
        // Bright core
        g.setColour(outColor.brighter(0.4f));
        g.strokePath(path, juce::PathStrokeType(0.8f));
        
        // Level indicator
        g.setFont(DS::Font::tiny());
        g.setColour(outColor.withAlpha(0.5f));
        g.drawText("Level: " + juce::String((int)(mixerLevel * 100)) + "%", 
                   getLocalBounds().reduced(8).removeFromTop(40), juce::Justification::topRight);
    }
    
    float applyFilter(float input, float cutoff, float reso) {
        float fc = cutoff * cutoff * cutoff * 0.5f + 0.01f;
        float fb = reso * reso * 4.2f;
        
        filterState1 += fc * (input - filterState1 + fb * (filterState1 - filterState2));
        filterState2 += fc * (filterState1 - filterState2);
        
        float out = filterState2;
        if (out > 1.0f) out = 1.0f - 1.0f / (out + 1.0f);
        else if (out < -1.0f) out = -1.0f + 1.0f / (-out + 1.0f);
        
        return out;
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicOscilloscope)
};

