/*
  ==============================================================================
    ParameterLink.h
    Link parameters together for synchronized editing
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
 * LinkMode - How linked parameters follow each other
 */
enum class LinkMode
{
    Absolute,   // Same value
    Offset,     // Fixed offset difference
    Ratio,      // Percentage ratio
    Inverse     // Inverted (127 - value)
};

//==============================================================================
/**
 * ParameterLink - Defines a link between two parameters
 */
struct ParameterLink
{
    juce::String sourceParamId;
    juce::String targetParamId;
    LinkMode mode = LinkMode::Absolute;
    int offset = 0;         // For Offset mode
    float ratio = 1.0f;     // For Ratio mode
    bool enabled = true;
    
    int calculateTargetValue(int sourceValue, int sourceMin, int sourceMax) const
    {
        if (!enabled) return -1;
        
        switch (mode)
        {
            case LinkMode::Absolute:
                return sourceValue;
                
            case LinkMode::Offset:
                return juce::jlimit(sourceMin, sourceMax, sourceValue + offset);
                
            case LinkMode::Ratio:
                return juce::jlimit(sourceMin, sourceMax, 
                    (int)(sourceValue * ratio));
                
            case LinkMode::Inverse:
                return sourceMax - sourceValue;
                
            default:
                return sourceValue;
        }
    }
};

//==============================================================================
/**
 * ParameterLinkManager - Manages all parameter links
 */
class ParameterLinkManager
{
public:
    ParameterLinkManager() {}
    
    //==========================================================================
    // Link Management
    //==========================================================================
    
    void addLink(const ParameterLink& link)
    {
        links.push_back(link);
        if (onLinksChanged)
            onLinksChanged();
    }
    
    void removeLink(const juce::String& sourceId, const juce::String& targetId)
    {
        links.erase(
            std::remove_if(links.begin(), links.end(),
                [&](const ParameterLink& l) {
                    return l.sourceParamId == sourceId && l.targetParamId == targetId;
                }),
            links.end()
        );
        if (onLinksChanged)
            onLinksChanged();
    }
    
    void removeAllLinksFor(const juce::String& paramId)
    {
        links.erase(
            std::remove_if(links.begin(), links.end(),
                [&](const ParameterLink& l) {
                    return l.sourceParamId == paramId || l.targetParamId == paramId;
                }),
            links.end()
        );
        if (onLinksChanged)
            onLinksChanged();
    }
    
    void clearAllLinks()
    {
        links.clear();
        if (onLinksChanged)
            onLinksChanged();
    }
    
    //==========================================================================
    // Link Queries
    //==========================================================================
    
    std::vector<ParameterLink> getLinksForSource(const juce::String& sourceId) const
    {
        std::vector<ParameterLink> result;
        for (const auto& link : links)
        {
            if (link.sourceParamId == sourceId && link.enabled)
                result.push_back(link);
        }
        return result;
    }
    
    bool hasLinks(const juce::String& paramId) const
    {
        for (const auto& link : links)
        {
            if ((link.sourceParamId == paramId || link.targetParamId == paramId) 
                && link.enabled)
                return true;
        }
        return false;
    }
    
    const std::vector<ParameterLink>& getAllLinks() const { return links; }
    
    //==========================================================================
    // Quick Link Presets
    //==========================================================================
    
    void linkEnvelopes()
    {
        // Link ENV1 and ENV2 parameters
        addLink({"env1_attack", "env2_attack", LinkMode::Absolute, 0, 1.0f, true});
        addLink({"env1_decay", "env2_decay", LinkMode::Absolute, 0, 1.0f, true});
        addLink({"env1_sustain", "env2_sustain", LinkMode::Absolute, 0, 1.0f, true});
        addLink({"env1_release", "env2_release", LinkMode::Absolute, 0, 1.0f, true});
    }
    
    void linkDCOs()
    {
        // Link DCO1 and DCO2 ranges
        addLink({"dco1_range", "dco2_range", LinkMode::Absolute, 0, 1.0f, true});
    }
    
    void linkLFODepths()
    {
        // Link all LFO depths
        addLink({"dco1_lfo", "dco2_lfo", LinkMode::Absolute, 0, 1.0f, true});
    }
    
    //==========================================================================
    // Serialization
    //==========================================================================
    
    juce::var toVar() const
    {
        juce::Array<juce::var> linksArray;
        for (const auto& link : links)
        {
            auto obj = new juce::DynamicObject();
            obj->setProperty("source", link.sourceParamId);
            obj->setProperty("target", link.targetParamId);
            obj->setProperty("mode", (int)link.mode);
            obj->setProperty("offset", link.offset);
            obj->setProperty("ratio", link.ratio);
            obj->setProperty("enabled", link.enabled);
            linksArray.add(juce::var(obj));
        }
        return linksArray;
    }
    
    void fromVar(const juce::var& v)
    {
        links.clear();
        if (auto* arr = v.getArray())
        {
            for (const auto& item : *arr)
            {
                if (auto* obj = item.getDynamicObject())
                {
                    ParameterLink link;
                    link.sourceParamId = obj->getProperty("source").toString();
                    link.targetParamId = obj->getProperty("target").toString();
                    link.mode = (LinkMode)(int)obj->getProperty("mode");
                    link.offset = obj->getProperty("offset");
                    link.ratio = obj->getProperty("ratio");
                    link.enabled = obj->getProperty("enabled");
                    links.push_back(link);
                }
            }
        }
    }
    
    // Callback when links change
    std::function<void()> onLinksChanged;
    
private:
    std::vector<ParameterLink> links;
};

//==============================================================================
/**
 * LinkIndicator - Visual indicator showing a parameter is linked
 */
class LinkIndicator : public juce::Component
{
public:
    LinkIndicator()
    {
        setSize(12, 12);
    }
    
    void paint(juce::Graphics& g) override
    {
        if (!isLinked) return;
        
        auto bounds = getLocalBounds().toFloat().reduced(1);
        
        // Draw chain link icon
        g.setColour(DS::Color::Accent::warning());
        
        // Two interlocking circles
        g.drawEllipse(bounds.getX(), bounds.getCentreY() - 3, 6, 6, 1.5f);
        g.drawEllipse(bounds.getRight() - 6, bounds.getCentreY() - 3, 6, 6, 1.5f);
        
        // Connecting line
        g.drawLine(bounds.getCentreX() - 1, bounds.getCentreY(),
                   bounds.getCentreX() + 1, bounds.getCentreY(), 1.5f);
    }
    
    void setLinked(bool linked)
    {
        if (isLinked != linked)
        {
            isLinked = linked;
            repaint();
        }
    }
    
    bool getIsLinked() const { return isLinked; }
    
private:
    bool isLinked = false;
};

//==============================================================================
/**
 * LinkSetupDialog - Dialog for setting up parameter links
 */
class LinkSetupDialog : public juce::Component
{
public:
    LinkSetupDialog(ParameterLinkManager& manager, 
                    const juce::String& sourceParam,
                    const juce::StringArray& availableTargets)
        : linkManager(manager), sourceParamId(sourceParam)
    {
        setSize(300, 200);
        
        // Title
        titleLabel.setText("Link: " + sourceParam, juce::dontSendNotification);
        titleLabel.setFont(DS::Font::heading());
        titleLabel.setColour(juce::Label::textColourId, DS::Color::textPrimary());
        addAndMakeVisible(titleLabel);
        
        // Target selector
        targetLabel.setText("Link to:", juce::dontSendNotification);
        addAndMakeVisible(targetLabel);
        
        for (int i = 0; i < availableTargets.size(); ++i)
            targetCombo.addItem(availableTargets[i], i + 1);
        targetCombo.setSelectedId(1);
        addAndMakeVisible(targetCombo);
        
        // Mode selector
        modeLabel.setText("Mode:", juce::dontSendNotification);
        addAndMakeVisible(modeLabel);
        
        modeCombo.addItem("Absolute (Same Value)", 1);
        modeCombo.addItem("Offset (+/-)", 2);
        modeCombo.addItem("Ratio (%)", 3);
        modeCombo.addItem("Inverse", 4);
        modeCombo.setSelectedId(1);
        modeCombo.onChange = [this]() { updateModeUI(); };
        addAndMakeVisible(modeCombo);
        
        // Offset/Ratio value
        valueLabel.setText("Offset:", juce::dontSendNotification);
        addAndMakeVisible(valueLabel);
        
        valueSlider.setRange(-127, 127, 1);
        valueSlider.setValue(0);
        addAndMakeVisible(valueSlider);
        valueSlider.setVisible(false);
        valueLabel.setVisible(false);
        
        // Buttons
        createBtn.setButtonText("Create Link");
        createBtn.onClick = [this]() { createLink(); };
        addAndMakeVisible(createBtn);
        
        cancelBtn.setButtonText("Cancel");
        cancelBtn.onClick = [this]() { if (onClose) onClose(); };
        addAndMakeVisible(cancelBtn);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bgPanel());
        g.setColour(DS::Color::border());
        g.drawRect(getLocalBounds());
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(10);
        
        titleLabel.setBounds(bounds.removeFromTop(30));
        bounds.removeFromTop(10);
        
        auto row = bounds.removeFromTop(24);
        targetLabel.setBounds(row.removeFromLeft(60));
        targetCombo.setBounds(row);
        
        bounds.removeFromTop(8);
        row = bounds.removeFromTop(24);
        modeLabel.setBounds(row.removeFromLeft(60));
        modeCombo.setBounds(row);
        
        bounds.removeFromTop(8);
        row = bounds.removeFromTop(24);
        valueLabel.setBounds(row.removeFromLeft(60));
        valueSlider.setBounds(row);
        
        bounds.removeFromTop(20);
        row = bounds.removeFromTop(30);
        cancelBtn.setBounds(row.removeFromLeft(100).reduced(2));
        row.removeFromLeft(10);
        createBtn.setBounds(row.removeFromLeft(100).reduced(2));
    }
    
    std::function<void()> onClose;
    std::function<void(const ParameterLink&)> onLinkCreated;
    
private:
    ParameterLinkManager& linkManager;
    juce::String sourceParamId;
    
    juce::Label titleLabel, targetLabel, modeLabel, valueLabel;
    juce::ComboBox targetCombo, modeCombo;
    juce::Slider valueSlider;
    juce::TextButton createBtn, cancelBtn;
    
    void updateModeUI()
    {
        int mode = modeCombo.getSelectedId();
        bool showValue = (mode == 2 || mode == 3);
        valueLabel.setVisible(showValue);
        valueSlider.setVisible(showValue);
        
        if (mode == 2)
        {
            valueLabel.setText("Offset:", juce::dontSendNotification);
            valueSlider.setRange(-127, 127, 1);
        }
        else if (mode == 3)
        {
            valueLabel.setText("Ratio:", juce::dontSendNotification);
            valueSlider.setRange(0.1, 2.0, 0.1);
            valueSlider.setValue(1.0);
        }
    }
    
    void createLink()
    {
        ParameterLink link;
        link.sourceParamId = sourceParamId;
        link.targetParamId = targetCombo.getText();
        link.mode = (LinkMode)(modeCombo.getSelectedId() - 1);
        
        if (link.mode == LinkMode::Offset)
            link.offset = (int)valueSlider.getValue();
        else if (link.mode == LinkMode::Ratio)
            link.ratio = (float)valueSlider.getValue();
            
        link.enabled = true;
        
        linkManager.addLink(link);
        
        if (onLinkCreated)
            onLinkCreated(link);
        if (onClose)
            onClose();
    }
};
