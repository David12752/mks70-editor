#include "LabeledSlider.h"
#include "PluginEditor.h"  // Full definition required
#include "MidiLearnManager.h"
#include "MidiMappingEditor.h"

// Static members for copy/paste
double LabeledSlider::clipboardValue = 0.0;
bool LabeledSlider::hasClipboardValue = false;

LabeledSlider::~LabeledSlider()
{
    if (editor != nullptr && !midiParamName.isEmpty())
        editor->getMidiLearnManager().unregisterParamSink(midiParamName);
}

void LabeledSlider::paint(juce::Graphics& g)
{
    // Draw MIDI Learn indicator
    if (editor != nullptr && !midiParamName.isEmpty())
    {
        auto& learn = editor->getMidiLearnManager();
        if (learn.isLearning() && learn.getLearningTarget() == midiParamName)
        {
            g.setColour(juce::Colours::red.withAlpha(0.3f));
            g.fillRect(getLocalBounds());
            g.setColour(juce::Colours::red);
            g.drawRect(getLocalBounds().reduced(1), 2);
        }
        else if (learn.getMappingForParam(midiParamName) != nullptr)
        {
            // Green dot indicator for mapped parameters
            g.setColour(juce::Colours::green);
            g.fillEllipse((float)getWidth() - 10.0f, 4.0f, 6.0f, 6.0f);
        }
    }
}

void LabeledSlider::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        showContextMenu();
        return;
    }
    Component::mouseDown(e);
}

void LabeledSlider::showContextMenu()
{
    juce::PopupMenu menu;
    
    // === VALUE OPERATIONS ===
    menu.addSectionHeader("Value");
    menu.addItem(10, "Copy Value");
    menu.addItem(11, "Paste Value", hasClipboardValue);
    menu.addSeparator();
    menu.addItem(12, "Reset to Default");
    menu.addItem(13, "Set to Minimum");
    menu.addItem(14, "Set to Maximum");
    menu.addItem(15, "Set to Center");
    menu.addSeparator();
    
    // Quick presets submenu
    juce::PopupMenu presetsMenu;
    presetsMenu.addItem(20, "0%");
    presetsMenu.addItem(21, "25%");
    presetsMenu.addItem(22, "50%");
    presetsMenu.addItem(23, "75%");
    presetsMenu.addItem(24, "100%");
    menu.addSubMenu("Quick Values", presetsMenu);
    
    // Random submenu
    juce::PopupMenu randomMenu;
    randomMenu.addItem(30, "Full Random");
    randomMenu.addItem(31, "Random +/- 10%");
    randomMenu.addItem(32, "Random +/- 25%");
    menu.addSubMenu("Randomize", randomMenu);
    
    menu.addSeparator();
    
    // === MIDI LEARN ===
    if (editor != nullptr && !midiParamName.isEmpty())
    {
        menu.addSectionHeader("MIDI Control");
        menu.addItem(1, "MIDI Learn");
        menu.addItem(2, "Clear MIDI Assignment");
        
        auto& learn = editor->getMidiLearnManager();
        if (auto* mapping = learn.getMappingForParam(midiParamName))
        {
            menu.addItem(3, "Edit Mapping...");
            juce::String info;
            switch (mapping->type)
            {
                case MidiLearnManager::MidiMapping::CC: info = "CC "; break;
                case MidiLearnManager::MidiMapping::NRPN: info = "NRPN "; break;
                case MidiLearnManager::MidiMapping::NOTE: info = "NOTE "; break;
                case MidiLearnManager::MidiMapping::AFTERTOUCH: info = "AFTER "; break;
                case MidiLearnManager::MidiMapping::PITCHBEND: info = "PB "; break;
            }
            info << mapping->controller << "  Ch " << (mapping->channel >= 0 ? juce::String(mapping->channel + 1) : juce::String("Omni"));
            menu.addItem(-1, info, false);
        }
    }
    
    // === DISPLAY INFO ===
    menu.addSeparator();
    menu.addSectionHeader("Info");
    juce::String valueInfo = "Current: " + juce::String(slider.getValue(), 1);
    menu.addItem(-1, valueInfo, false);
    juce::String rangeInfo = "Range: " + juce::String(slider.getMinimum(), 0) + " - " + juce::String(slider.getMaximum(), 0);
    menu.addItem(-1, rangeInfo, false);
    if (!midiParamName.isEmpty())
    {
        juce::String paramInfo = "Param: " + midiParamName;
        menu.addItem(-1, paramInfo, false);
    }
    
    menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result)
    {
        double minVal = slider.getMinimum();
        double maxVal = slider.getMaximum();
        double range = maxVal - minVal;
        
        switch (result)
        {
            // Value operations
            case 10: // Copy
                clipboardValue = slider.getValue();
                hasClipboardValue = true;
                break;
            case 11: // Paste
                if (hasClipboardValue)
                    slider.setValue(juce::jlimit(minVal, maxVal, clipboardValue), juce::sendNotification);
                break;
            case 12: // Reset to default
                slider.setValue(defaultValue, juce::sendNotification);
                break;
            case 13: // Set to min
                slider.setValue(minVal, juce::sendNotification);
                break;
            case 14: // Set to max
                slider.setValue(maxVal, juce::sendNotification);
                break;
            case 15: // Set to center
                slider.setValue(minVal + range * 0.5, juce::sendNotification);
                break;
                
            // Quick presets
            case 20: slider.setValue(minVal, juce::sendNotification); break;
            case 21: slider.setValue(minVal + range * 0.25, juce::sendNotification); break;
            case 22: slider.setValue(minVal + range * 0.5, juce::sendNotification); break;
            case 23: slider.setValue(minVal + range * 0.75, juce::sendNotification); break;
            case 24: slider.setValue(maxVal, juce::sendNotification); break;
            
            // Random
            case 30: // Full random
                slider.setValue(minVal + juce::Random::getSystemRandom().nextDouble() * range, juce::sendNotification);
                break;
            case 31: // Random +/- 10%
                {
                    double current = slider.getValue();
                    double offset = (juce::Random::getSystemRandom().nextDouble() - 0.5) * range * 0.2;
                    slider.setValue(juce::jlimit(minVal, maxVal, current + offset), juce::sendNotification);
                }
                break;
            case 32: // Random +/- 25%
                {
                    double current = slider.getValue();
                    double offset = (juce::Random::getSystemRandom().nextDouble() - 0.5) * range * 0.5;
                    slider.setValue(juce::jlimit(minVal, maxVal, current + offset), juce::sendNotification);
                }
                break;
                
            // MIDI Learn
            case 1:
                if (editor != nullptr)
                {
                    // Lazy-register sink on first interaction
                    if (!midiSinkRegistered)
                    {
                        editor->getMidiLearnManager().registerParamSink(midiParamName, [this](int v){
                            juce::MessageManager::callAsync([this, v](){ 
                                this->slider.setValue(v, juce::sendNotificationSync); 
                            });
                        });
                        midiSinkRegistered = true;
                    }
                    editor->getMidiLearnManager().startLearn(midiParamName);
                }
                break;
            case 2:
                if (editor != nullptr)
                    editor->getMidiLearnManager().removeMapping(midiParamName);
                break;
            case 3:
                showMappingEditor();
                break;
        }
        repaint();
    });
}

void LabeledSlider::showLearnMenu()
{
    // Backwards compatibility - redirect to full context menu
    showContextMenu();
}

void LabeledSlider::showMappingEditor()
{
    if (editor == nullptr || midiParamName.isEmpty()) return;
    auto& learn = editor->getMidiLearnManager();
    auto* mm = new MidiMappingEditor(midiParamName, learn);
    mm->onClose = [mm]() { delete mm; };
    mm->addToDesktop(juce::ComponentPeer::windowIsTemporary | juce::ComponentPeer::windowHasTitleBar);
    mm->centreWithSize(520, 320);
    mm->setVisible(true);
}
