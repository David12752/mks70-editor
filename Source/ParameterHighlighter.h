#pragma once
#include <JuceHeader.h>

//==============================================================================
// System for highlighting specific parameters in the UI with glowing effect
// Used by AI Assistant to guide users step-by-step
//==============================================================================
class ParameterHighlighter : public juce::Component, 
                              public juce::Timer,
                              public juce::DeletedAtShutdown
{
public:
    struct HighlightedParameter
    {
        juce::Component* targetComponent = nullptr;
        juce::String parameterName;
        juce::String explanation;
        int stepNumber = 0;
        juce::Rectangle<int> bounds;
    };
    
    ParameterHighlighter()
    {
        setInterceptsMouseClicks(false, true);
        startTimer(30); // 30fps for smooth animation
    }
    
    ~ParameterHighlighter() override
    {
        // Cleanup if needed
    }
    
    // Highlight a parameter with glowing effect
    void highlightParameter(juce::Component* component, 
                           const juce::String& name, 
                           const juce::String& explanation,
                           int step = 0)
    {
        if (component == nullptr) return;
        
        HighlightedParameter param;
        param.targetComponent = component;
        param.parameterName = name;
        param.explanation = explanation;
        param.stepNumber = step;
        param.bounds = component->getBoundsInParent();
        
        // If this is a new step, clear previous highlights
        if (step != currentStep)
        {
            currentHighlights.clear();
            currentStep = step;
        }
        
        currentHighlights.add(param);
        
        // Make sure we're visible
        setVisible(true);
        
        // Position ourselves over the parent of the component
        if (component->getParentComponent() != nullptr)
        {
            auto* parent = component->getParentComponent();
            setBounds(parent->getBounds());
            parent->addAndMakeVisible(this);
        }
        
        repaint();
    }
    
    // Clear all highlights
    void clearHighlights()
    {
        currentHighlights.clear();
        currentStep = -1;
        setVisible(false);
        repaint();
    }
    
    // Get current step
    int getCurrentStep() const { return currentStep; }
    
    void paint(juce::Graphics& g) override
    {
        if (currentHighlights.isEmpty()) return;
        
        // Calculate breathing effect (0.0 to 1.0)
        float breath = 0.5f + 0.5f * std::sin(breathPhase);
        
        for (const auto& param : currentHighlights)
        {
            if (param.targetComponent == nullptr || !param.targetComponent->isVisible())
                continue;
                
            auto bounds = param.bounds.toFloat();
            
            // Convert to our coordinate space
            auto* parent = param.targetComponent->getParentComponent();
            if (parent != nullptr)
            {
                auto localBounds = parent->getLocalArea(param.targetComponent, 
                                                         param.targetComponent->getLocalBounds());
                bounds = localBounds.toFloat();
            }
            
            // Expand bounds for glow effect
            bounds = bounds.expanded(20.0f);
            
            // Draw multiple layers of glow (breathing effect)
            juce::Colour glowColor = juce::Colour(0xff4af6ff); // Cyan glow
            
            // Outer glow layers
            for (int i = 5; i >= 0; --i)
            {
                float expansion = i * 4.0f * (0.7f + breath * 0.3f);
                float alpha = (1.0f - i / 6.0f) * 0.15f * breath;
                g.setColour(glowColor.withAlpha(alpha));
                g.fillRoundedRectangle(bounds.expanded(expansion), 12.0f + expansion * 0.5f);
            }
            
            // Main highlight border (pulsing)
            g.setColour(glowColor.withAlpha(0.6f + breath * 0.4f));
            g.drawRoundedRectangle(bounds, 12.0f, 3.0f);
            
            // Inner glow
            g.setColour(glowColor.withAlpha(0.1f + breath * 0.1f));
            g.fillRoundedRectangle(bounds, 12.0f);
            
            // Draw step number badge
            if (param.stepNumber > 0)
            {
                float badgeSize = 40.0f;
                juce::Rectangle<float> badgeBounds(bounds.getX() - 10.0f, bounds.getY() - 10.0f, 
                                                    badgeSize, badgeSize);
                
                // Badge background with glow
                g.setColour(glowColor.withAlpha(0.9f));
                g.fillEllipse(badgeBounds);
                
                // Badge border
                g.setColour(juce::Colours::white.withAlpha(0.8f));
                g.drawEllipse(badgeBounds, 2.0f);
                
                // Step number
                g.setColour(juce::Colours::black);
                g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
                g.drawText(juce::String(param.stepNumber), 
                          badgeBounds.toNearestInt(), 
                          juce::Justification::centred);
            }
            
            // Draw explanation tooltip below
            if (param.explanation.isNotEmpty())
            {
                float tooltipHeight = 60.0f;
                juce::Rectangle<float> tooltipBounds(bounds.getX() + 10.0f, 
                                                     bounds.getBottom() + 10.0f,
                                                     bounds.getWidth() - 20.0f, 
                                                     tooltipHeight);
                
                // Tooltip background with glow
                g.setColour(juce::Colour(0xff1a1a2e).withAlpha(0.95f));
                g.fillRoundedRectangle(tooltipBounds, 8.0f);
                
                g.setColour(glowColor.withAlpha(0.5f));
                g.drawRoundedRectangle(tooltipBounds, 8.0f, 2.0f);
                
                // Tooltip text
                g.setColour(juce::Colours::white);
                g.setFont(juce::FontOptions(14.0f));
                g.drawFittedText(param.explanation, 
                                tooltipBounds.reduced(10.0f, 5.0f).toNearestInt(),
                                juce::Justification::centred, 
                                2);
            }
        }
    }
    
    void timerCallback() override
    {
        breathPhase += 0.05f;
        if (breathPhase > juce::MathConstants<float>::twoPi)
            breathPhase -= juce::MathConstants<float>::twoPi;
            
        if (isVisible() && !currentHighlights.isEmpty())
            repaint();
    }
    
    // Public singleton access methods
    static ParameterHighlighter* getInstance()
    {
        static ParameterHighlighter instance;
        return &instance;
    }
    
    static void deleteInstance()
    {
        // Static instance managed automatically
    }
    
private:
    juce::Array<HighlightedParameter> currentHighlights;
    int currentStep = -1;
    float breathPhase = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterHighlighter)
};
