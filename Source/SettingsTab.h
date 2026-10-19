/*
  ==============================================================================
    SettingsTab.h - Comprehensive Settings with Device/Software tabs
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DeviceSettingsPanel.h"
#include "MidiTestPanel.h"
#include "DesignSystem.h"

class MKS70EditorAudioProcessor;

//==============================================================================
class SettingsTab : public juce::Component, public juce::Button::Listener
{
public:
    SettingsTab();
    ~SettingsTab() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    
    void setProcessor(MKS70EditorAudioProcessor* proc);
    
    // Callback for theme changes
    std::function<void(int)> onThemeChanged;
    
    // Callback for UI scale changes
    std::function<void(float)> onUIScaleChanged;
    
    // Callback for animation speed changes
    std::function<void(float)> onAnimationSpeedChanged;
    
    // Callback for firmware version changes (Original=1, V3=2, V4=3)
    std::function<void(int)> onFirmwareVersionChanged;
    
    // Get current firmware selection
    int getFirmwareVersion() const { return fwVersionCombo.getSelectedId(); }
    
    // Save/Load settings
    void saveSettings();
    void loadSettings();
    void applySettingsToProcessor();
    
    // Refresh from processor (for Total Recall)
    void refreshFromProcessor() {
        if (processor) {
            // Refresh MIDI Test panel
            if (midiTestPanel) midiTestPanel->repaint();
            // Refresh device settings
            deviceSettingsPanel.repaint();
        }
    }

private:
    MKS70EditorAudioProcessor* processor = nullptr;
    
    // Tab buttons
    juce::TextButton deviceTabBtn { "Device" };
    juce::TextButton softwareTabBtn { "Software" };
    juce::TextButton midiTestTabBtn { "MIDI Test" };
    int currentTab = 0;
    
    // Device panel (direct, no viewport)
    DeviceSettingsPanel deviceSettingsPanel;
    
    // MIDI Test panel with viewport for scrolling
    juce::Viewport midiTestViewport;
    std::unique_ptr<MidiTestPanel> midiTestPanel;
    
    // Software settings container with scrollable viewport
    juce::Viewport softwareViewport;
    juce::Component softwareContainer;
    
    //==========================================================================
    // SECTION 1: Appearance
    //==========================================================================
    juce::Label appearanceHeader{{}, "APPEARANCE"};
    
    juce::Label themeLabel{{}, "Theme:"};
    juce::ComboBox themeCombo;
    
    juce::Label uiScaleLabel{{}, "UI Scale:"};
    juce::ComboBox uiScaleCombo;
    
    juce::Label animSpeedLabel{{}, "Animation:"};
    juce::ComboBox animSpeedCombo;
    
    juce::ToggleButton showTooltipsToggle{"Show Tooltips"};
    juce::ToggleButton showParameterValuesToggle{"Show Parameter Values"};
    
    //==========================================================================
    // SECTION 2: Workflow & Sync
    //==========================================================================
    juce::Label workflowHeader{{}, "WORKFLOW"};
    
    juce::ToggleButton sendOnEditToggle{"Send Changes to Synth"};
    juce::ToggleButton requestOnLoadToggle{"Request Patch on Load"};
    juce::ToggleButton autoBackupToggle{"Auto Backup Presets"};
    juce::ToggleButton confirmActionsToggle{"Confirm Destructive Actions"};
    juce::ToggleButton rememberWindowToggle{"Remember Window Position"};
    
    //==========================================================================
    // SECTION 3: File Paths
    //==========================================================================
    juce::Label pathsHeader{{}, "FILE PATHS"};
    
    juce::Label libraryLabel{{}, "Preset Library:"};
    juce::TextEditor libraryPath;
    juce::TextButton browseLibraryBtn{"..."};
    
    juce::Label backupLabel{{}, "Backup Folder:"};
    juce::TextEditor backupPathEdit;
    juce::TextButton browseBackupBtn{"..."};
    
    juce::Label backupIntLabel{{}, "Backup Interval:"};
    juce::Slider backupIntSlider;
    juce::Label backupIntValueLabel{{}, "5 min"};
    
    //==========================================================================
    // SECTION 4: Vecoven Firmware
    //==========================================================================
    juce::Label vecovenHeader{{}, "VECOVEN FIRMWARE"};
    
    juce::Label fwVersionLabel{{}, "Firmware Version:"};
    juce::ComboBox fwVersionCombo;
    
    juce::ToggleButton pwmToggle{"PWM Mode"};
    juce::ToggleButton aprToggle{"APR Mode"};
    
    juce::Label sysexRedirLabel{{}, "SysEx Redirect:"};
    juce::ComboBox sysexRedirCombo;
    
    //==========================================================================
    // SECTION 5: Advanced/Timing
    //==========================================================================
    juce::Label advancedHeader{{}, "ADVANCED"};
    
    juce::Label sysexDelayLabel{{}, "SysEx Delay (ms):"};
    juce::Slider sysexDelaySlider;
    
    juce::Label bulkDelayLabel{{}, "Bulk Delay (ms):"};
    juce::Slider bulkDelaySlider;
    
    juce::ToggleButton midiThruToggle{"MIDI Thru"};
    juce::ToggleButton debugModeToggle{"Debug Mode"};
    juce::ToggleButton logToFileToggle{"Log to File"};
    
    //==========================================================================
    // Action Buttons
    //==========================================================================
    juce::TextButton testMidiBtn{"Test MIDI"};
    juce::TextButton resetBtn{"Reset All"};
    juce::TextButton importBtn{"Import"};
    juce::TextButton exportBtn{"Export"};
    juce::TextButton aboutBtn{"About"};
    
    void setupSoftwareSettings();
    void layoutSoftwareSettings();
    void showDeviceTab();
    void showSoftwareTab();
    void showMidiTestTab();
    void testMidiConnection();
    void resetToDefaults();
    void importSettings();
    void exportSettings();
    void showAbout();
    void browseForLibraryPath();
    void browseForBackupPath();
    
    void styleCombo(juce::ComboBox& c, juce::Colour accent = DS::Color::Accent::primary());
    void styleLabel(juce::Label& l);
    void styleSectionHeader(juce::Label& l);
    void styleSlider(juce::Slider& s, juce::Colour accent = DS::Color::Accent::primary());
    void styleToggle(juce::ToggleButton& t, juce::Colour accent = DS::Color::Accent::primary());
    void styleButton(juce::TextButton& b, juce::Colour accent = DS::Color::Accent::primary());
    void styleTextEditor(juce::TextEditor& e);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsTab)
};
