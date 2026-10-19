/*
  ==============================================================================

    CompareComboBox.cpp
    Enhanced ComboBox with Compare Mode highlighting

  ==============================================================================
*/

#include "CompareComboBox.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"

bool CompareComboBox::shouldHighlight() const
{
    if (editor == nullptr)
        return false;
    
    if (!editor->isCompareModeActive())
        return false;
    
    return editor->getProcessor().compareManager.isDifferent(parameterName);
}

std::pair<int, int> CompareComboBox::getABValues() const
{
    if (editor == nullptr)
        return {-1, -1};
    
    return editor->getProcessor().compareManager.getParameterValues(parameterName);
}
