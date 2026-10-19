/*
  ==============================================================================

    CompareComboBox.h
    Enhanced ComboBox with Compare Mode highlighting

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class MKS70EditorAudioProcessorEditor;

/**
 * ComboBox that highlights when parameter differs between Compare slots A and B
 */
class CompareComboBox : public juce::ComboBox
{
public:
    CompareComboBox(const juce::String& paramName, MKS70EditorAudioProcessorEditor* ed)
        : parameterName(paramName), editor(ed)
    {
    }
    
    void paintOverChildren(juce::Graphics& g) override
    {
        if (shouldHighlight())
        {
            // Draw semi-transparent orange overlay
            g.setColour(juce::Colours::orange.withAlpha(0.3f));
            g.fillRect(getLocalBounds());
            
            // Draw orange border for extra visibility
            g.setColour(juce::Colours::orange.withAlpha(0.6f));
            g.drawRect(getLocalBounds(), 2);
        }
    }
    
    juce::String getTooltip() override
    {
        if (shouldHighlight())
        {
            auto values = getABValues();
            if (values.first != -1 && values.second != -1)
            {
                return juce::String("A: ") + juce::String(values.first) + 
                       juce::String(", B: ") + juce::String(values.second);
            }
        }
        
        return juce::ComboBox::getTooltip();
    }
    
private:
    bool shouldHighlight() const;
    std::pair<int, int> getABValues() const;
    
    juce::String parameterName;
    MKS70EditorAudioProcessorEditor* editor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompareComboBox)
};
