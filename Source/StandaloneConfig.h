/*
  ==============================================================================

    StandaloneConfig.h
    Custom configuration for Standalone application window

  ==============================================================================
*/

#pragma once

// Define this before including juce_audio_plugin_client
#define JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP 1

// Standalone window settings
namespace juce
{
    namespace StandalonePluginHolder
    {
        // Custom window constraints
        inline constexpr int defaultWindowWidth = 900;
        inline constexpr int defaultWindowHeight = 600;
        inline constexpr int minWindowWidth = 800;
        inline constexpr int minWindowHeight = 500;
        inline constexpr int maxWindowWidth = 3840;
        inline constexpr int maxWindowHeight = 2160;
    }
}
