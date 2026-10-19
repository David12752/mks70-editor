/*
  ==============================================================================

    CompareSlider.cpp
    Enhanced Slider with Compare Mode highlighting

  ==============================================================================
*/

#include "CompareSlider.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "MidiLearnManager.h"
#include "MidiMappingEditor.h"

bool CompareSlider::shouldHighlight() const
{
    if (editor == nullptr)
        return false;
    
    if (!editor->isCompareModeActive())
        return false;
    
    return editor->getProcessor().compareManager.isDifferent(parameterName);
}

std::pair<int, int> CompareSlider::getABValues() const
{
    if (editor == nullptr)
        return {-1, -1};
    
    return editor->getProcessor().compareManager.getParameterValues(parameterName);
}

void CompareSlider::paintOverChildren(juce::Graphics& g)
{
    if (shouldHighlight())
    {
        g.setColour(juce::Colours::orange.withAlpha(0.3f));
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colours::orange.withAlpha(0.6f));
        g.drawRect(getLocalBounds(), 2);
    }

    if (editor != nullptr)
    {
        auto& learn = editor->getMidiLearnManager();
        if (learn.isLearning() && learn.getLearningTarget() == parameterName)
        {
            g.setColour(juce::Colours::red);
            g.drawRect(getLocalBounds().reduced(1), 2);
        }
        else if (learn.getMappingForParam(parameterName) != nullptr)
        {
            g.setColour(juce::Colours::green);
            g.fillEllipse((float)getWidth() - 10.0f, 4.0f, 6.0f, 6.0f);
        }
    }
}

CompareSlider::~CompareSlider()
{
    if (editor != nullptr)
        editor->getMidiLearnManager().unregisterParamSink(parameterName);
}

void CompareSlider::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        showLearnMenu();
        return;
    }
    juce::Slider::mouseDown(e);
}

void CompareSlider::showLearnMenu()
{
    if (editor == nullptr) return;
    // Lazy-register sink on first interaction
    if (!midiSinkRegistered)
    {
        editor->getMidiLearnManager().registerParamSink(parameterName, [this](int v){
            juce::MessageManager::callAsync([this, v](){ this->setValue(v, juce::sendNotificationSync); });
        });
        midiSinkRegistered = true;
    }
    auto& learn = editor->getMidiLearnManager();
    juce::PopupMenu menu;
    menu.addItem(1, "MIDI Learn");
    menu.addItem(2, "Clear MIDI Assignment");
    menu.addSeparator();
    if (auto* mapping = learn.getMappingForParam(parameterName))
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
    menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result)
    {
        if (editor == nullptr) return;
        auto& learn = editor->getMidiLearnManager();
        if (result == 1) learn.startLearn(parameterName);
        if (result == 2) learn.removeMapping(parameterName);
        if (result == 3) showMappingEditor();
        repaint();
    });
}

void CompareSlider::showMappingEditor()
{
    if (editor == nullptr) return;
    auto& learn = editor->getMidiLearnManager();
    auto* mm = new MidiMappingEditor(parameterName, learn);
    mm->onClose = [mm]() { delete mm; };
    mm->addToDesktop(juce::ComponentPeer::windowIsTemporary | juce::ComponentPeer::windowHasTitleBar);
    mm->centreWithSize(520, 320);
    mm->setVisible(true);
}
