/*
  ==============================================================================

    ModulationRoutingDisplay.h
    Visual display of modulation routing connections for MKS-70

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
    ModulationRoute - Represents a single modulation connection
*/
struct ModulationRoute
{
    juce::String source;         // e.g., "LFO-1", "ENV-1"
    juce::String destination;    // e.g., "DCO-1 Pitch", "VCF Cutoff"
    float amount;                // 0-127 normalized to 0-1
    bool isActive;               // Whether the modulation is non-zero
    juce::Colour colour;         // Route colour
    
    ModulationRoute(const juce::String& src, const juce::String& dest, 
                   float amt = 0, juce::Colour col = DS::Colors::accent)
        : source(src), destination(dest), amount(amt), isActive(amt > 0), colour(col) {}
};

//==============================================================================
/**
    ModulationRoutingDisplay - Visual representation of modulation paths
*/
class ModulationRoutingDisplay : public juce::Component, private juce::Timer
{
public:
    ModulationRoutingDisplay()
    {
        setSize(400, 250);
        startTimerHz(30);
    }
    
    ~ModulationRoutingDisplay() override
    {
        stopTimer();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // Title
        g.setColour(DS::Colors::text);
        g.setFont(14.0f);
        g.drawText("Modulation Routing", bounds.removeFromTop(25), juce::Justification::centred);
        
        // Draw modules
        drawModules(g);
        
        // Draw routing connections
        drawRoutes(g);
    }
    
    void setRoutes(const std::vector<ModulationRoute>& newRoutes)
    {
        routes = newRoutes;
        repaint();
    }
    
    void setRouteAmount(int routeIndex, float amount)
    {
        if (routeIndex >= 0 && routeIndex < (int)routes.size())
        {
            routes[routeIndex].amount = amount;
            routes[routeIndex].isActive = amount > 0.01f;
            repaint();
        }
    }
    
    // Update from actual synth parameters
    void updateFromParams(int dco1LfoMod, int dco1EnvMod,
                         int dco2LfoMod, int dco2EnvMod,
                         int vcfLfoMod, int vcfEnvMod,
                         int mixerEnvMod)
    {
        routes.clear();
        
        // LFO -> DCO-1
        if (dco1LfoMod > 0)
            routes.push_back(ModulationRoute("LFO-1", "DCO-1 Pitch", dco1LfoMod / 127.0f, DS::Colors::sectionLFO));
        
        // ENV -> DCO-1
        if (dco1EnvMod > 0)
            routes.push_back(ModulationRoute("ENV-1", "DCO-1 Pitch", dco1EnvMod / 127.0f, DS::Colors::sectionENV));
        
        // LFO -> DCO-2
        if (dco2LfoMod > 0)
            routes.push_back(ModulationRoute("LFO-1", "DCO-2 Pitch", dco2LfoMod / 127.0f, DS::Colors::sectionLFO));
        
        // ENV -> DCO-2
        if (dco2EnvMod > 0)
            routes.push_back(ModulationRoute("ENV-1", "DCO-2 Pitch", dco2EnvMod / 127.0f, DS::Colors::sectionENV));
        
        // LFO -> VCF
        if (vcfLfoMod > 0)
            routes.push_back(ModulationRoute("LFO-1", "VCF Cutoff", vcfLfoMod / 127.0f, DS::Colors::sectionLFO));
        
        // ENV -> VCF
        if (vcfEnvMod > 0)
            routes.push_back(ModulationRoute("ENV-2", "VCF Cutoff", vcfEnvMod / 127.0f, DS::Colors::sectionENV));
        
        // ENV -> Mixer
        if (mixerEnvMod > 0)
            routes.push_back(ModulationRoute("ENV-1", "DCO Mix", mixerEnvMod / 127.0f, DS::Colors::sectionENV));
        
        repaint();
    }
    
private:
    std::vector<ModulationRoute> routes;
    float animationPhase = 0.0f;
    
    // Module positions
    struct ModulePos
    {
        juce::String name;
        juce::Rectangle<float> bounds;
        juce::Colour colour;
        bool isSource;
    };
    
    std::vector<ModulePos> modulePositions;
    
    void timerCallback() override
    {
        animationPhase += 0.1f;
        if (animationPhase > juce::MathConstants<float>::twoPi)
            animationPhase -= juce::MathConstants<float>::twoPi;
        repaint();
    }
    
    void drawModules(juce::Graphics& g)
    {
        modulePositions.clear();
        
        auto bounds = getLocalBounds().reduced(15);
        bounds.removeFromTop(30);
        
        // Source modules (left side)
        auto sourceArea = bounds.removeFromLeft(80);
        drawModule(g, "LFO-1", sourceArea.removeFromTop(45).toFloat(), DS::Colors::sectionLFO, true);
        sourceArea.removeFromTop(10);
        drawModule(g, "LFO-2", sourceArea.removeFromTop(45).toFloat(), DS::Colors::sectionLFO, true);
        sourceArea.removeFromTop(10);
        drawModule(g, "ENV-1", sourceArea.removeFromTop(45).toFloat(), DS::Colors::sectionENV, true);
        sourceArea.removeFromTop(10);
        drawModule(g, "ENV-2", sourceArea.removeFromTop(45).toFloat(), DS::Colors::sectionENV, true);
        
        // Destination modules (right side)
        bounds.removeFromLeft(20);
        auto destArea = bounds.removeFromRight(80);
        drawModule(g, "DCO-1 Pitch", destArea.removeFromTop(35).toFloat(), DS::Colors::sectionDCO, false);
        destArea.removeFromTop(8);
        drawModule(g, "DCO-2 Pitch", destArea.removeFromTop(35).toFloat(), DS::Colors::sectionDCO, false);
        destArea.removeFromTop(8);
        drawModule(g, "DCO Mix", destArea.removeFromTop(35).toFloat(), DS::Colors::sectionMixer, false);
        destArea.removeFromTop(8);
        drawModule(g, "VCF Cutoff", destArea.removeFromTop(35).toFloat(), DS::Colors::sectionVCF, false);
        destArea.removeFromTop(8);
        drawModule(g, "VCF Reso", destArea.removeFromTop(35).toFloat(), DS::Colors::sectionVCF, false);
        destArea.removeFromTop(8);
        drawModule(g, "VCA Level", destArea.removeFromTop(35).toFloat(), DS::Colors::sectionVCA, false);
    }
    
    void drawModule(juce::Graphics& g, const juce::String& name, juce::Rectangle<float> area,
                   juce::Colour colour, bool isSource)
    {
        // Store position
        modulePositions.push_back({name, area, colour, isSource});
        
        // Draw module box
        g.setColour(colour.withAlpha(0.15f));
        g.fillRoundedRectangle(area, 4.0f);
        
        g.setColour(colour.withAlpha(0.5f));
        g.drawRoundedRectangle(area, 4.0f, 1.0f);
        
        // Connection dot
        float dotX = isSource ? area.getRight() - 3 : area.getX() + 3;
        float dotY = area.getCentreY();
        
        g.setColour(colour);
        g.fillEllipse(dotX - 4, dotY - 4, 8, 8);
        
        // Name
        g.setColour(colour.interpolatedWith(DS::Colors::text, 0.5f));
        g.setFont(10.0f);
        g.drawText(name, area.reduced(5, 0), juce::Justification::centred);
    }
    
    void drawRoutes(juce::Graphics& g)
    {
        for (const auto& route : routes)
        {
            if (!route.isActive) continue;
            
            // Find source and destination positions
            juce::Point<float> sourcePoint, destPoint;
            juce::Colour routeColour = route.colour;
            
            for (const auto& mod : modulePositions)
            {
                if (mod.name.containsIgnoreCase(route.source.substring(0, 3)))
                {
                    if (mod.isSource)
                    {
                        sourcePoint = juce::Point<float>(mod.bounds.getRight(), mod.bounds.getCentreY());
                        routeColour = mod.colour;
                    }
                }
                if (mod.name.containsIgnoreCase(route.destination.substring(0, 3)))
                {
                    if (!mod.isSource)
                    {
                        destPoint = juce::Point<float>(mod.bounds.getX(), mod.bounds.getCentreY());
                    }
                }
            }
            
            // Draw bezier curve connection
            if (sourcePoint.x > 0 && destPoint.x > 0)
            {
                drawConnectionCurve(g, sourcePoint, destPoint, routeColour, route.amount);
            }
        }
    }
    
    void drawConnectionCurve(juce::Graphics& g, juce::Point<float> start, juce::Point<float> end,
                            juce::Colour colour, float intensity)
    {
        // Bezier control points
        float midX = (start.x + end.x) / 2;
        juce::Point<float> ctrl1(midX, start.y);
        juce::Point<float> ctrl2(midX, end.y);
        
        // Create path
        juce::Path curvePath;
        curvePath.startNewSubPath(start);
        curvePath.cubicTo(ctrl1, ctrl2, end);
        
        // Draw glow
        float alpha = 0.15f + intensity * 0.35f;
        g.setColour(colour.withAlpha(alpha));
        g.strokePath(curvePath, juce::PathStrokeType(6.0f + intensity * 4.0f));
        
        // Draw main line
        float lineWidth = 1.5f + intensity * 2.0f;
        g.setColour(colour.withAlpha(0.5f + intensity * 0.5f));
        g.strokePath(curvePath, juce::PathStrokeType(lineWidth));
        
        // Animated flow dots
        float pathLength = curvePath.getLength();
        int numDots = juce::jmax(2, (int)(intensity * 5));
        
        for (int i = 0; i < numDots; i++)
        {
            float t = std::fmod(animationPhase / juce::MathConstants<float>::twoPi + 
                               (float)i / numDots, 1.0f);
            auto dotPos = curvePath.getPointAlongPath(t * pathLength);
            
            g.setColour(colour);
            float dotSize = 4.0f + intensity * 3.0f;
            g.fillEllipse(dotPos.x - dotSize/2, dotPos.y - dotSize/2, dotSize, dotSize);
        }
        
        // Draw arrowhead at destination
        drawArrowHead(g, ctrl2, end, colour.withAlpha(0.8f));
    }
    
    void drawArrowHead(juce::Graphics& g, juce::Point<float> from, juce::Point<float> to,
                      juce::Colour colour)
    {
        float angle = std::atan2(to.y - from.y, to.x - from.x);
        float arrowSize = 8.0f;
        
        juce::Path arrow;
        arrow.startNewSubPath(to);
        arrow.lineTo(to.x - arrowSize * std::cos(angle - 0.4f),
                    to.y - arrowSize * std::sin(angle - 0.4f));
        arrow.lineTo(to.x - arrowSize * std::cos(angle + 0.4f),
                    to.y - arrowSize * std::sin(angle + 0.4f));
        arrow.closeSubPath();
        
        g.setColour(colour);
        g.fillPath(arrow);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationRoutingDisplay)
};

//==============================================================================
/**
    MiniModRoutingIndicator - Compact routing indicator badge
*/
class MiniModRoutingIndicator : public juce::Component
{
public:
    MiniModRoutingIndicator()
    {
        setSize(100, 30);
    }
    
    void setActiveRoutes(int count)
    {
        activeRouteCount = count;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        // Background
        g.setColour(juce::Colour(0xff1e1e1e));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Icon
        auto iconArea = bounds.removeFromLeft(24).reduced(4);
        g.setColour(activeRouteCount > 0 ? DS::Colors::accent : DS::Colors::textMuted);
        
        // Draw mini routing icon
        juce::Path icon;
        icon.addEllipse(iconArea.getX(), iconArea.getCentreY() - 3, 6, 6);
        icon.addEllipse(iconArea.getRight() - 6, iconArea.getY() + 2, 6, 6);
        icon.addEllipse(iconArea.getRight() - 6, iconArea.getBottom() - 8, 6, 6);
        
        g.fillPath(icon);
        
        // Lines
        g.drawLine(iconArea.getX() + 6, iconArea.getCentreY(),
                  iconArea.getRight() - 6, iconArea.getY() + 5, 1.5f);
        g.drawLine(iconArea.getX() + 6, iconArea.getCentreY(),
                  iconArea.getRight() - 6, iconArea.getBottom() - 5, 1.5f);
        
        // Text
        g.setFont(11.0f);
        juce::String text = juce::String(activeRouteCount) + " route" + (activeRouteCount != 1 ? "s" : "");
        g.drawText(text, bounds.reduced(4, 0), juce::Justification::centredLeft);
    }
    
private:
    int activeRouteCount = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MiniModRoutingIndicator)
};
