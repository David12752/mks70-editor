#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * ModulationMatrix.h
 * Visual modulation matrix and routing display
 * 
 * Part of Improvement #15: Modulation Matrix
 */

namespace ModMatrix
{

//==============================================================================
/**
 * Modulation source types
 */
enum class ModSource
{
    None,
    LFO1,
    LFO2,
    ENV1,
    ENV2,
    Velocity,
    KeyFollow,
    AfterTouch,
    ModWheel,
    PitchBend,
    Breath
};

/**
 * Modulation destination types
 */
enum class ModDest
{
    None,
    DCO1Pitch,
    DCO1PW,
    DCO2Pitch,
    DCO2PW,
    MixLevel,
    VCFCutoff,
    VCFResonance,
    VCALevel,
    PAN
};

//==============================================================================
/**
 * Modulation routing entry
 */
struct ModRoute
{
    ModSource source = ModSource::None;
    ModDest destination = ModDest::None;
    float amount = 0.0f;  // -1 to +1
    bool bipolar = true;
    
    bool isActive() const
    {
        return source != ModSource::None && destination != ModDest::None && amount != 0.0f;
    }
};

//==============================================================================
/**
 * Modulation Matrix Manager
 */
class MatrixManager
{
public:
    static constexpr int MAX_ROUTES = 8;
    
    MatrixManager()
    {
        routes.resize(MAX_ROUTES);
    }
    
    void setRoute(int index, const ModRoute& route)
    {
        if (index >= 0 && index < MAX_ROUTES)
        {
            routes[index] = route;
            if (onRouteChanged)
                onRouteChanged(index);
        }
    }
    
    ModRoute& getRoute(int index)
    {
        return routes[index];
    }
    
    const ModRoute& getRoute(int index) const
    {
        return routes[index];
    }
    
    void clearRoute(int index)
    {
        if (index >= 0 && index < MAX_ROUTES)
        {
            routes[index] = ModRoute();
            if (onRouteChanged)
                onRouteChanged(index);
        }
    }
    
    void clearAll()
    {
        for (auto& route : routes)
            route = ModRoute();
    }
    
    int getActiveRouteCount() const
    {
        int count = 0;
        for (const auto& route : routes)
            if (route.isActive())
                count++;
        return count;
    }
    
    std::function<void(int)> onRouteChanged;
    
    static juce::String getSourceName(ModSource source)
    {
        switch (source)
        {
            case ModSource::None:       return "None";
            case ModSource::LFO1:       return "LFO 1";
            case ModSource::LFO2:       return "LFO 2";
            case ModSource::ENV1:       return "ENV 1";
            case ModSource::ENV2:       return "ENV 2";
            case ModSource::Velocity:   return "Velocity";
            case ModSource::KeyFollow:  return "Key Follow";
            case ModSource::AfterTouch: return "Aftertouch";
            case ModSource::ModWheel:   return "Mod Wheel";
            case ModSource::PitchBend:  return "Pitch Bend";
            case ModSource::Breath:     return "Breath";
            default: return "?";
        }
    }
    
    static juce::String getDestName(ModDest dest)
    {
        switch (dest)
        {
            case ModDest::None:         return "None";
            case ModDest::DCO1Pitch:    return "DCO1 Pitch";
            case ModDest::DCO1PW:       return "DCO1 PW";
            case ModDest::DCO2Pitch:    return "DCO2 Pitch";
            case ModDest::DCO2PW:       return "DCO2 PW";
            case ModDest::MixLevel:     return "Mix Level";
            case ModDest::VCFCutoff:    return "VCF Cutoff";
            case ModDest::VCFResonance: return "VCF Reso";
            case ModDest::VCALevel:     return "VCA Level";
            case ModDest::PAN:          return "PAN";
            default: return "?";
        }
    }
    
    static juce::Colour getSourceColor(ModSource source)
    {
        switch (source)
        {
            case ModSource::LFO1:
            case ModSource::LFO2:       return juce::Colour(0xFF8b5cf6);
            case ModSource::ENV1:
            case ModSource::ENV2:       return juce::Colour(0xFFf59e0b);
            case ModSource::Velocity:
            case ModSource::KeyFollow:  return juce::Colour(0xFF10b981);
            case ModSource::ModWheel:
            case ModSource::PitchBend:
            case ModSource::AfterTouch:
            case ModSource::Breath:     return juce::Colour(0xFF3b82f6);
            default:                    return DS::Color::textMuted;
        }
    }
    
private:
    std::vector<ModRoute> routes;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixManager)
};

//==============================================================================
/**
 * Modulation Amount Slider
 */
class ModAmountSlider : public juce::Component
{
public:
    ModAmountSlider()
    {
        setRepaintsOnMouseActivity(true);
    }
    
    void setAmount(float a) { amount = juce::jlimit(-1.0f, 1.0f, a); repaint(); }
    float getAmount() const { return amount; }
    void setBipolar(bool b) { bipolar = b; repaint(); }
    
    std::function<void(float)> onAmountChanged;
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background track
        g.setColour(DS::Color::surfaceAlt);
        g.fillRoundedRectangle(bounds, 4);
        
        // Value bar
        float centerX = bounds.getCentreX();
        float width = bounds.getWidth();
        
        juce::Rectangle<float> valueBounds;
        
        if (bipolar)
        {
            if (amount >= 0)
                valueBounds = bounds.withLeft(centerX).withWidth(amount * width / 2);
            else
                valueBounds = bounds.withRight(centerX).withLeft(centerX + amount * width / 2);
        }
        else
        {
            valueBounds = bounds.withWidth(amount * width);
        }
        
        juce::Colour valueColor = amount >= 0 ? DS::Color::accent : juce::Colour(0xFFef4444);
        g.setColour(valueColor);
        g.fillRoundedRectangle(valueBounds, 4);
        
        // Center line for bipolar
        if (bipolar)
        {
            g.setColour(DS::Color::border);
            g.drawVerticalLine(static_cast<int>(centerX), bounds.getY(), bounds.getBottom());
        }
        
        // Border
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, 4, 1.0f);
        
        // Value text
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::label);
        g.drawText(juce::String(static_cast<int>(amount * 100)) + "%", bounds, juce::Justification::centred);
    }
    
    void mouseDown(const juce::MouseEvent& e) override { updateFromMouse(e); }
    void mouseDrag(const juce::MouseEvent& e) override { updateFromMouse(e); }
    
    void mouseDoubleClick(const juce::MouseEvent&) override
    {
        setAmount(0.0f);
        if (onAmountChanged)
            onAmountChanged(amount);
    }
    
private:
    void updateFromMouse(const juce::MouseEvent& e)
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        float normalized = (e.position.x - bounds.getX()) / bounds.getWidth();
        
        if (bipolar)
            amount = juce::jlimit(-1.0f, 1.0f, (normalized - 0.5f) * 2.0f);
        else
            amount = juce::jlimit(0.0f, 1.0f, normalized);
        
        repaint();
        
        if (onAmountChanged)
            onAmountChanged(amount);
    }
    
    float amount = 0.0f;
    bool bipolar = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModAmountSlider)
};

//==============================================================================
/**
 * Single modulation route row
 */
class ModRouteRow : public juce::Component
{
public:
    ModRouteRow(int index) : routeIndex(index)
    {
        sourceCombo.addItem("None", 1);
        sourceCombo.addItem("LFO 1", 2);
        sourceCombo.addItem("LFO 2", 3);
        sourceCombo.addItem("ENV 1", 4);
        sourceCombo.addItem("ENV 2", 5);
        sourceCombo.addItem("Velocity", 6);
        sourceCombo.addItem("Key Follow", 7);
        sourceCombo.addItem("Mod Wheel", 8);
        sourceCombo.onChange = [this] { if (onChanged) onChanged(); };
        addAndMakeVisible(sourceCombo);
        
        destCombo.addItem("None", 1);
        destCombo.addItem("DCO1 Pitch", 2);
        destCombo.addItem("DCO1 PW", 3);
        destCombo.addItem("DCO2 Pitch", 4);
        destCombo.addItem("VCF Cutoff", 5);
        destCombo.addItem("VCF Reso", 6);
        destCombo.addItem("VCA Level", 7);
        destCombo.onChange = [this] { if (onChanged) onChanged(); };
        addAndMakeVisible(destCombo);
        
        amountSlider.onAmountChanged = [this](float) { if (onChanged) onChanged(); };
        addAndMakeVisible(amountSlider);
    }
    
    void setRoute(const ModRoute& route)
    {
        sourceCombo.setSelectedId(static_cast<int>(route.source) + 1, juce::dontSendNotification);
        destCombo.setSelectedId(static_cast<int>(route.destination) + 1, juce::dontSendNotification);
        amountSlider.setAmount(route.amount);
    }
    
    ModRoute getRoute() const
    {
        ModRoute route;
        route.source = static_cast<ModSource>(sourceCombo.getSelectedId() - 1);
        route.destination = static_cast<ModDest>(destCombo.getSelectedId() - 1);
        route.amount = amountSlider.getAmount();
        return route;
    }
    
    std::function<void()> onChanged;
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(2);
        sourceCombo.setBounds(bounds.removeFromLeft(100));
        bounds.removeFromLeft(8);
        destCombo.setBounds(bounds.removeFromLeft(100));
        bounds.removeFromLeft(8);
        amountSlider.setBounds(bounds);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto route = getRoute();
        
        if (route.isActive())
        {
            g.setColour(MatrixManager::getSourceColor(route.source).withAlpha(0.1f));
            g.fillRoundedRectangle(getLocalBounds().toFloat(), 4);
        }
        
        // Route index
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
    }
    
private:
    int routeIndex;
    juce::ComboBox sourceCombo;
    juce::ComboBox destCombo;
    ModAmountSlider amountSlider;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModRouteRow)
};

//==============================================================================
/**
 * Modulation Matrix Panel
 */
class MatrixPanel : public juce::Component
{
public:
    MatrixPanel(MatrixManager& manager) : matrixManager(manager)
    {
        for (int i = 0; i < MatrixManager::MAX_ROUTES; i++)
        {
            auto row = std::make_unique<ModRouteRow>(i);
            row->setRoute(manager.getRoute(i));
            row->onChanged = [this, i]
            {
                matrixManager.setRoute(i, routeRows[i]->getRoute());
            };
            addAndMakeVisible(*row);
            routeRows.push_back(std::move(row));
        }
    }
    
    void refreshFromManager()
    {
        for (int i = 0; i < MatrixManager::MAX_ROUTES; i++)
        {
            routeRows[i]->setRoute(matrixManager.getRoute(i));
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        
        // Title
        bounds.removeFromTop(30);
        bounds.removeFromTop(DS::Space::sm);
        
        // Header
        bounds.removeFromTop(20);
        bounds.removeFromTop(4);
        
        // Routes
        for (auto& row : routeRows)
        {
            row->setBounds(bounds.removeFromTop(32));
            bounds.removeFromTop(4);
        }
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        auto contentBounds = bounds.reduced(DS::Space::md);
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading);
        g.drawText("MODULATION MATRIX", contentBounds.removeFromTop(24), juce::Justification::centredLeft);
        
        // Active routes indicator
        int activeCount = matrixManager.getActiveRouteCount();
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText(juce::String(activeCount) + "/" + juce::String(MatrixManager::MAX_ROUTES) + " active",
                   bounds.reduced(DS::Space::md).removeFromTop(24), juce::Justification::centredRight);
        
        contentBounds.removeFromTop(DS::Space::sm);
        
        // Header
        auto headerBounds = contentBounds.removeFromTop(20);
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText("SOURCE", headerBounds.removeFromLeft(100), juce::Justification::centredLeft);
        headerBounds.removeFromLeft(8);
        g.drawText("DESTINATION", headerBounds.removeFromLeft(100), juce::Justification::centredLeft);
        headerBounds.removeFromLeft(8);
        g.drawText("AMOUNT", headerBounds, juce::Justification::centredLeft);
    }
    
private:
    MatrixManager& matrixManager;
    std::vector<std::unique_ptr<ModRouteRow>> routeRows;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixPanel)
};

} // namespace ModMatrix
