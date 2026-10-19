/*
  ==============================================================================

    AppConfig.h
    Application configuration - included before all JUCE headers

  ==============================================================================
*/

#pragma once

// ASIO driver support (Windows only)
#if JUCE_WINDOWS
 #ifndef JUCE_ASIO
  #define JUCE_ASIO 1
 #endif
#endif

// Standalone window resizing configuration
#define JucePlugin_StandaloneFilterWindow_MinWidth    800
#define JucePlugin_StandaloneFilterWindow_MinHeight   500
#define JucePlugin_StandaloneFilterWindow_MaxWidth    3840
#define JucePlugin_StandaloneFilterWindow_MaxHeight   2160
