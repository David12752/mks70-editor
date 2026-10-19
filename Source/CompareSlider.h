/*
  ==============================================================================

    CompareSlider.h
    Enhanced Slider with Compare Mode highlighting

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class MKS70EditorAudioProcessorEditor;

/**
 * Slider that highlights when parameter differs between Compare slots A and B
 */
class CompareSlider : public juce::Slider
{
public:
    CompareSlider(const juce::String& paramName, MKS70EditorAudioProcessorEditor* ed)
        : parameterName(paramName), editor(ed)
    {
    }
    ~CompareSlider() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    
    void paintOverChildren(juce::Graphics& g) override;
    
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
        
        return juce::Slider::getTooltip();
    }
    
private:
    bool shouldHighlight() const;
    std::pair<int, int> getABValues() const;
    
    juce::String parameterName;
    MKS70EditorAudioProcessorEditor* editor;
    bool midiSinkRegistered = false;
    
    void showLearnMenu();
    void showMappingEditor();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompareSlider)
};
