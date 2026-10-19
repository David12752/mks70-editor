#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * ParameterLocking.h
 * Lock parameters from changes
 * 
 * Part of Improvement #19: Parameter Locking
 */

namespace Locking
{

//==============================================================================
/**
 * Lockable parameter info
 */
struct LockableParam
{
    juce::String id;
    juce::String name;
    juce::String section;
    bool locked = false;
};

//==============================================================================
/**
 * Parameter Lock Manager - manages locked parameters
 */
class ParameterLockManager
{
public:
    ParameterLockManager()
    {
        // Initialize with standard MKS-70 parameters by section
        addSection("DCO1", { "dco1_range", "dco1_wave", "dco1_tune", "dco1_lfo", "dco1_env" });
        addSection("DCO2", { "dco2_range", "dco2_wave", "dco2_tune", "dco2_lfo", "dco2_env", "dco2_cross", "dco2_fine" });
        addSection("VCF", { "vcf_freq", "vcf_res", "vcf_lfo", "vcf_env", "vcf_kybd" });
        addSection("VCA", { "vca_level", "vca_env", "vca_velocity" });
        addSection("LFO", { "lfo_rate", "lfo_delay", "lfo_wave" });
        addSection("ENV1", { "env1_attack", "env1_decay", "env1_sustain", "env1_release", "env1_kybd" });
        addSection("ENV2", { "env2_attack", "env2_decay", "env2_sustain", "env2_release", "env2_kybd" });
        addSection("CHORUS", { "chorus_rate", "chorus_depth", "chorus_mode" });
    }
    
    void addSection(const juce::String& section, std::initializer_list<juce::String> params)
    {
        for (const auto& param : params)
        {
            LockableParam lp;
            lp.id = param;
            lp.name = param.replaceCharacter('_', ' ').toUpperCase();
            lp.section = section;
            lp.locked = false;
            parameters[param] = lp;
            sectionParams[section].push_back(param);
        }
    }
    
    bool isLocked(const juce::String& paramId) const
    {
        auto it = parameters.find(paramId);
        return it != parameters.end() && it->second.locked;
    }
    
    void setLocked(const juce::String& paramId, bool locked)
    {
        auto it = parameters.find(paramId);
        if (it != parameters.end())
        {
            it->second.locked = locked;
            notifyListeners();
        }
    }
    
    void toggleLocked(const juce::String& paramId)
    {
        auto it = parameters.find(paramId);
        if (it != parameters.end())
        {
            it->second.locked = !it->second.locked;
            notifyListeners();
        }
    }
    
    void lockSection(const juce::String& section)
    {
        auto it = sectionParams.find(section);
        if (it != sectionParams.end())
        {
            for (const auto& paramId : it->second)
                parameters[paramId].locked = true;
        }
        notifyListeners();
    }
    
    void unlockSection(const juce::String& section)
    {
        auto it = sectionParams.find(section);
        if (it != sectionParams.end())
        {
            for (const auto& paramId : it->second)
                parameters[paramId].locked = false;
        }
        notifyListeners();
    }
    
    void lockAll()
    {
        for (auto& p : parameters)
            p.second.locked = true;
        notifyListeners();
    }
    
    void unlockAll()
    {
        for (auto& p : parameters)
            p.second.locked = false;
        notifyListeners();
    }
    
    bool isSectionFullyLocked(const juce::String& section) const
    {
        auto it = sectionParams.find(section);
        if (it == sectionParams.end())
            return false;
        
        for (const auto& paramId : it->second)
        {
            auto pit = parameters.find(paramId);
            if (pit == parameters.end() || !pit->second.locked)
                return false;
        }
        return true;
    }
    
    bool isSectionPartiallyLocked(const juce::String& section) const
    {
        auto it = sectionParams.find(section);
        if (it == sectionParams.end())
            return false;
        
        bool hasLocked = false;
        bool hasUnlocked = false;
        
        for (const auto& paramId : it->second)
        {
            auto pit = parameters.find(paramId);
            if (pit != parameters.end())
            {
                if (pit->second.locked)
                    hasLocked = true;
                else
                    hasUnlocked = true;
            }
        }
        
        return hasLocked && hasUnlocked;
    }
    
    std::vector<juce::String> getSections() const
    {
        std::vector<juce::String> result;
        for (const auto& sp : sectionParams)
            result.push_back(sp.first);
        return result;
    }
    
    std::vector<LockableParam> getParamsForSection(const juce::String& section) const
    {
        std::vector<LockableParam> result;
        auto it = sectionParams.find(section);
        if (it != sectionParams.end())
        {
            for (const auto& paramId : it->second)
            {
                auto pit = parameters.find(paramId);
                if (pit != parameters.end())
                    result.push_back(pit->second);
            }
        }
        return result;
    }
    
    int getLockedCount() const
    {
        int count = 0;
        for (const auto& p : parameters)
            if (p.second.locked) count++;
        return count;
    }
    
    int getTotalCount() const { return static_cast<int>(parameters.size()); }
    
    // Save/load lock state
    juce::ValueTree saveState() const
    {
        juce::ValueTree state("ParameterLocks");
        
        for (const auto& p : parameters)
        {
            if (p.second.locked)
            {
                juce::ValueTree lock("Lock");
                lock.setProperty("param", p.first, nullptr);
                state.appendChild(lock, nullptr);
            }
        }
        
        return state;
    }
    
    void loadState(const juce::ValueTree& state)
    {
        unlockAll();
        
        for (int i = 0; i < state.getNumChildren(); i++)
        {
            auto lock = state.getChild(i);
            if (lock.hasType("Lock"))
            {
                juce::String paramId = lock.getProperty("param");
                setLocked(paramId, true);
            }
        }
    }
    
    // Listener support
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void lockStateChanged() = 0;
    };
    
    void addListener(Listener* l) { listeners.push_back(l); }
    void removeListener(Listener* l) 
    {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), l), listeners.end());
    }
    
private:
    void notifyListeners()
    {
        for (auto* l : listeners)
            l->lockStateChanged();
    }
    
    std::map<juce::String, LockableParam> parameters;
    std::map<juce::String, std::vector<juce::String>> sectionParams;
    std::vector<Listener*> listeners;
};

//==============================================================================
/**
 * Lock Indicator - shows lock status on a parameter
 */
class LockIndicator : public juce::Component
{
public:
    LockIndicator(ParameterLockManager& mgr, const juce::String& paramId)
        : manager(mgr), parameterId(paramId)
    {
        setSize(16, 16);
    }
    
    void paint(juce::Graphics& g) override
    {
        bool locked = manager.isLocked(parameterId);
        
        auto bounds = getLocalBounds().toFloat().reduced(2);
        
        if (locked)
        {
            // Locked icon
            g.setColour(juce::Colour(0xFFef4444));
            
            // Lock body
            auto body = bounds.reduced(2).withTrimmedTop(bounds.getHeight() * 0.3f);
            g.fillRoundedRectangle(body, 2);
            
            // Lock shackle
            auto shackle = bounds.withTrimmedBottom(bounds.getHeight() * 0.5f).reduced(3, 0);
            g.drawArc(shackle.getX(), shackle.getY(), shackle.getWidth(), shackle.getHeight() * 2,
                      juce::MathConstants<float>::pi, juce::MathConstants<float>::pi * 2, 2.0f);
        }
        else
        {
            // Unlocked - just show faint border
            g.setColour(DS::Color::border.withAlpha(0.3f));
            g.drawRoundedRectangle(bounds, 2, 1.0f);
        }
    }
    
    void mouseDown(const juce::MouseEvent&) override
    {
        manager.toggleLocked(parameterId);
        repaint();
    }
    
private:
    ParameterLockManager& manager;
    juce::String parameterId;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockIndicator)
};

//==============================================================================
/**
 * Section Lock Button - lock entire section
 */
class SectionLockButton : public juce::Component
{
public:
    SectionLockButton(ParameterLockManager& mgr, const juce::String& sec)
        : manager(mgr), section(sec)
    {
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        bool fullyLocked = manager.isSectionFullyLocked(section);
        bool partiallyLocked = manager.isSectionPartiallyLocked(section);
        
        // Background
        if (fullyLocked)
            g.setColour(juce::Colour(0xFFef4444).withAlpha(0.2f));
        else if (partiallyLocked)
            g.setColour(juce::Colour(0xFFf59e0b).withAlpha(0.2f));
        else
            g.setColour(DS::Color::surfaceAlt);
        
        g.fillRoundedRectangle(bounds, DS::Radius::small);
        
        // Border
        if (fullyLocked)
            g.setColour(juce::Colour(0xFFef4444));
        else if (partiallyLocked)
            g.setColour(juce::Colour(0xFFf59e0b));
        else
            g.setColour(DS::Color::border);
        
        g.drawRoundedRectangle(bounds, DS::Radius::small, 1.0f);
        
        // Lock icon
        auto iconBounds = bounds.reduced(4);
        auto iconSize = juce::jmin(iconBounds.getWidth(), iconBounds.getHeight());
        auto iconRect = juce::Rectangle<float>(iconSize, iconSize).withCentre(iconBounds.getCentre());
        
        if (fullyLocked || partiallyLocked)
        {
            g.setColour(fullyLocked ? juce::Colour(0xFFef4444) : juce::Colour(0xFFf59e0b));
            
            // Lock body
            auto body = iconRect.reduced(2).withTrimmedTop(iconRect.getHeight() * 0.3f);
            g.fillRoundedRectangle(body, 2);
            
            // Lock shackle
            auto shackle = iconRect.withTrimmedBottom(iconRect.getHeight() * 0.5f).reduced(3, 0);
            if (fullyLocked)
            {
                // Closed shackle
                g.drawArc(shackle.getX(), shackle.getY(), shackle.getWidth(), shackle.getHeight() * 2,
                          juce::MathConstants<float>::pi, juce::MathConstants<float>::pi * 2, 2.0f);
            }
            else
            {
                // Open shackle (partially locked)
                g.drawArc(shackle.getX() + 4, shackle.getY(), shackle.getWidth(), shackle.getHeight() * 2,
                          juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 2.0f);
            }
        }
        else
        {
            // Unlocked icon
            g.setColour(DS::Color::textMuted);
            g.drawRoundedRectangle(iconRect, 3, 1.0f);
        }
    }
    
    void mouseDown(const juce::MouseEvent&) override
    {
        if (manager.isSectionFullyLocked(section))
            manager.unlockSection(section);
        else
            manager.lockSection(section);
        
        repaint();
    }
    
private:
    ParameterLockManager& manager;
    juce::String section;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionLockButton)
};

//==============================================================================
/**
 * Lock Panel - full lock management UI
 */
class LockPanel : public juce::Component, public ParameterLockManager::Listener
{
public:
    LockPanel(ParameterLockManager& mgr)
        : manager(mgr)
    {
        manager.addListener(this);
        
        // Lock all button
        lockAllButton.setButtonText("Lock All");
        lockAllButton.onClick = [this]() { manager.lockAll(); };
        addAndMakeVisible(lockAllButton);
        
        // Unlock all button
        unlockAllButton.setButtonText("Unlock All");
        unlockAllButton.onClick = [this]() { manager.unlockAll(); };
        addAndMakeVisible(unlockAllButton);
        
        // Create section buttons
        for (const auto& section : manager.getSections())
        {
            auto* btn = new SectionLockButton(manager, section);
            sectionButtons.add(btn);
            addAndMakeVisible(btn);
        }
        
        updateStatusText();
    }
    
    ~LockPanel() override
    {
        manager.removeListener(this);
    }
    
    void lockStateChanged() override
    {
        updateStatusText();
        repaint();
        
        for (auto* btn : sectionButtons)
            btn->repaint();
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        
        // Title
        bounds.removeFromTop(24);
        bounds.removeFromTop(DS::Space::sm);
        
        // Status
        bounds.removeFromTop(20);
        bounds.removeFromTop(DS::Space::sm);
        
        // Buttons
        auto buttonRow = bounds.removeFromTop(28);
        lockAllButton.setBounds(buttonRow.removeFromLeft(100));
        buttonRow.removeFromLeft(DS::Space::sm);
        unlockAllButton.setBounds(buttonRow.removeFromLeft(100));
        
        bounds.removeFromTop(DS::Space::md);
        
        // Section buttons
        auto sections = manager.getSections();
        int cols = 4;
        int rows = (static_cast<int>(sections.size()) + cols - 1) / cols;
        int btnWidth = (bounds.getWidth() - (cols - 1) * DS::Space::sm) / cols;
        int btnHeight = 60;
        
        for (int i = 0; i < sectionButtons.size(); i++)
        {
            int row = i / cols;
            int col = i % cols;
            
            int x = col * (btnWidth + DS::Space::sm);
            int y = row * (btnHeight + DS::Space::sm);
            
            sectionButtons[i]->setBounds(x, bounds.getY() + y, btnWidth, btnHeight);
        }
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::large);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::large, 1.0f);
        
        auto contentBounds = bounds.reduced(DS::Space::md);
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading2);
        g.drawText("Parameter Locking", contentBounds.removeFromTop(24), juce::Justification::centredLeft);
        
        contentBounds.removeFromTop(DS::Space::sm);
        
        // Status
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::body);
        g.drawText(statusText, contentBounds.removeFromTop(20), juce::Justification::centredLeft);
        
        contentBounds.removeFromTop(DS::Space::sm + 28 + DS::Space::md);
        
        // Section labels
        auto sections = manager.getSections();
        int cols = 4;
        int btnWidth = (static_cast<int>(contentBounds.getWidth()) - (cols - 1) * DS::Space::sm) / cols;
        int btnHeight = 60;
        
        g.setFont(DS::Font::label);
        
        for (int i = 0; i < static_cast<int>(sections.size()); i++)
        {
            int row = i / cols;
            int col = i % cols;
            
            int x = static_cast<int>(contentBounds.getX()) + col * (btnWidth + DS::Space::sm);
            int y = static_cast<int>(contentBounds.getY()) + row * (btnHeight + DS::Space::sm);
            
            auto labelBounds = juce::Rectangle<int>(x, y + btnHeight - 18, btnWidth, 16);
            g.drawText(sections[i], labelBounds, juce::Justification::centred);
        }
    }
    
private:
    void updateStatusText()
    {
        int locked = manager.getLockedCount();
        int total = manager.getTotalCount();
        statusText = juce::String(locked) + " of " + juce::String(total) + " parameters locked";
    }
    
    ParameterLockManager& manager;
    juce::TextButton lockAllButton;
    juce::TextButton unlockAllButton;
    juce::OwnedArray<SectionLockButton> sectionButtons;
    juce::String statusText;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockPanel)
};

} // namespace Locking
