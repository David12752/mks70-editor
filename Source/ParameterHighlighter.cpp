#include "ParameterHighlighter.h"

// Singleton implementation
#include "ParameterHighlighter.h"

// Singleton implementation
ParameterHighlighter* ParameterHighlighter::_getInstance()
{
    static ParameterHighlighter instance;
    return &instance;
}

void ParameterHighlighter::_deleteInstance()
{
    // Static instance, nothing to delete
}
