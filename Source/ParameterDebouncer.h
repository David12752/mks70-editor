/*
  ==============================================================================

    ParameterDebouncer.h
    Debouncing/throttling system for parameter changes
    
    Prevents MIDI/SysEx flooding by batching rapid parameter changes
    and sending them at controlled intervals

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Parameter Debouncer - batches and throttles parameter changes
*/
class ParameterDebouncer : public juce::Timer
{
public:
    ParameterDebouncer(int debounceMs = 50)
        : debounceInterval(debounceMs)
    {
    }
    
    ~ParameterDebouncer() override
    {
        stopTimer();
    }
    
    // Queue a parameter change
    void queueChange(const juce::String& parameterId, int value, std::function<void(int)> sendFunc)
    {
        {
            const juce::SpinLock::ScopedLockType lock(changeLock);
            
            // Store or update the pending change
            PendingChange change;
            change.parameterId = parameterId;
            change.value = value;
            change.sendFunction = sendFunc;
            change.timestamp = juce::Time::getMillisecondCounter();
            
            // Replace existing pending change for this parameter
            bool found = false;
            for (auto& pending : pendingChanges)
            {
                if (pending.parameterId == parameterId)
                {
                    pending = change;
                    found = true;
                    break;
                }
            }
            
            if (!found)
                pendingChanges.push_back(change);
        }
        
        // Start or restart the debounce timer
        startTimer(debounceInterval);
    }
    
    // Force immediate send of all pending changes
    void flush()
    {
        stopTimer();
        timerCallback();
    }
    
    void timerCallback() override
    {
        std::vector<PendingChange> changesToSend;
        
        {
            const juce::SpinLock::ScopedLockType lock(changeLock);
            changesToSend = pendingChanges;
            pendingChanges.clear();
        }
        
        // Send all batched changes
        for (const auto& change : changesToSend)
        {
            if (change.sendFunction)
                change.sendFunction(change.value);
        }
        
        stopTimer();
    }
    
    void setDebounceInterval(int ms) { debounceInterval = ms; }
    int getDebounceInterval() const { return debounceInterval; }
    
    bool hasPendingChanges() const
    {
        const juce::SpinLock::ScopedLockType lock(changeLock);
        return !pendingChanges.empty();
    }
    
private:
    struct PendingChange
    {
        juce::String parameterId;
        int value;
        std::function<void(int)> sendFunction;
        juce::uint32 timestamp;
    };
    
    std::vector<PendingChange> pendingChanges;
    mutable juce::SpinLock changeLock;
    int debounceInterval;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterDebouncer)
};

//==============================================================================
/**
    Global Parameter Debouncer Manager
*/
class DebouncerManager
{
public:
    static DebouncerManager& getInstance()
    {
        static DebouncerManager instance;
        return instance;
    }
    
    ParameterDebouncer& getDebouncer() { return debouncer; }
    
    // Queue a change with the global debouncer
    void queueParameterChange(const juce::String& parameterId, int value, 
                             std::function<void(int)> sendFunc)
    {
        debouncer.queueChange(parameterId, value, sendFunc);
    }
    
    void flush() { debouncer.flush(); }
    void setDebounceInterval(int ms) { debouncer.setDebounceInterval(ms); }
    
private:
    DebouncerManager() : debouncer(50) {}
    ParameterDebouncer debouncer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebouncerManager)
};
