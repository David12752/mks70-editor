/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MKS70EditorAudioProcessor::MKS70EditorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Initialize default parameters
    toneUpper = ToneParams();
    toneLower = ToneParams();
    patchParameters = PatchParameters();
    arpParameters = ArpParameters();
    
    // Load parameter mapping
    juce::File mappingFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                                        .getParentDirectory()
                                        .getChildFile("mks70_mapping.json");
    
    if (mappingFile.existsAsFile())
    {
        if (mappingLoader.loadFromFile(mappingFile))
        {
            DBG("Parameter mapping loaded successfully: " + mappingLoader.getDeviceName());
        }
        else
        {
            DBG("Failed to load mapping: " + mappingLoader.getLastError());
        }
    }
    else
    {
        DBG("Mapping file not found: " + mappingFile.getFullPathName());
    }
    
    // Setup MidiHandler callbacks
    midiHandler.onToneReceived = [this](ToneParams& tone, int toneNumber) 
    {
        handleToneReceived(tone, toneNumber);
    };

    midiHandler.onToneParameterReceived = [this](int toneNumber, int paramIndex, int value)
    {
        handleToneParameterReceived(toneNumber, paramIndex, value);
    };
    
    midiHandler.onPatchReceived = [this](PatchParameters& patch)
    {
        handlePatchReceived(patch);
    };

    midiHandler.onPatchParameterReceived = [this](int paramIndex, int value)
    {
        handlePatchParameterReceived(paramIndex, value);
    };
    
    midiHandler.onBulkDumpReceived = [this](const PatchBank& bank)
    {
        handleBulkDumpReceived(bank);
    };
    
    midiHandler.onError = [this](const juce::String& error)
    {
        handleMidiError(error);
    };
}

MKS70EditorAudioProcessor::~MKS70EditorAudioProcessor()
{
}

//==============================================================================
const juce::String MKS70EditorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MKS70EditorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MKS70EditorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MKS70EditorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MKS70EditorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MKS70EditorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MKS70EditorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MKS70EditorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MKS70EditorAudioProcessor::getProgramName (int index)
{
    return {};
}

void MKS70EditorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MKS70EditorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MKS70EditorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MKS70EditorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MKS70EditorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool MKS70EditorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MKS70EditorAudioProcessor::createEditor()
{
    return new MKS70EditorAudioProcessorEditor (*this);
}

//==============================================================================
// TOTAL RECALL - Save/Restore complete synth state with DAW project
void MKS70EditorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Create XML structure to store all synth state
    juce::XmlElement state("MKS70EditorState");
    state.setAttribute("version", "1.0");
    state.setAttribute("savedAt", juce::Time::getCurrentTime().toISO8601(true));
    
    // Store MIDI settings
    auto* midiSettings = state.createNewChildElement("MidiSettings");
    midiSettings->setAttribute("midiChannel", midiHandler.getMidiChannel());
    midiSettings->setAttribute("deviceId", midiHandler.getDeviceId());
    
    // Store Upper Tone parameters
    auto* upperTone = state.createNewChildElement("ToneUpper");
    storeToneToXml(*upperTone, toneUpper);
    
    // Store Lower Tone parameters  
    auto* lowerTone = state.createNewChildElement("ToneLower");
    storeToneToXml(*lowerTone, toneLower);
    
    // Store Patch parameters
    auto* patch = state.createNewChildElement("Patch");
    storePatchToXml(*patch, patchParameters);
    
    // Store Arpeggiator parameters
    auto* arp = state.createNewChildElement("Arpeggiator");
    storeArpToXml(*arp, arpParameters);
    
    // Store Compare slots (A, B, Original)
    auto* compareSlots = state.createNewChildElement("CompareSlots");
    if (compareManager.hasSlot(CompareManager::SLOT_A))
    {
        auto* slotA = compareSlots->createNewChildElement("SlotA");
        storeFullPatchToXml(*slotA, compareManager.getSlot(CompareManager::SLOT_A));
    }
    if (compareManager.hasSlot(CompareManager::SLOT_B))
    {
        auto* slotB = compareSlots->createNewChildElement("SlotB");
        storeFullPatchToXml(*slotB, compareManager.getSlot(CompareManager::SLOT_B));
    }
    
    // Store Total Recall settings
    auto* totalRecall = state.createNewChildElement("TotalRecall");
    totalRecall->setAttribute("enabled", totalRecallEnabled);
    totalRecall->setAttribute("sendOnLoad", sendSysExOnLoad);
    totalRecall->setAttribute("requestOnLoad", requestSysExOnLoad);
    totalRecall->setAttribute("sendOnEdit", sendOnEditEnabled);
    
    // Copy XML to memory block
    copyXmlToBinary(state, destData);
    
    DBG("State saved: " + juce::String(destData.getSize()) + " bytes");
}

void MKS70EditorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Parse XML from memory block
    std::unique_ptr<juce::XmlElement> state(getXmlFromBinary(data, sizeInBytes));
    
    if (state == nullptr || !state->hasTagName("MKS70EditorState"))
    {
        DBG("Invalid state data - cannot restore");
        return;
    }
    
    DBG("Restoring state from version: " + state->getStringAttribute("version"));
    
    // Restore MIDI settings
    if (auto* midiSettings = state->getChildByName("MidiSettings"))
    {
        int channel = midiSettings->getIntAttribute("midiChannel", 1);
        int deviceId = midiSettings->getIntAttribute("deviceId", 0x10);
        midiHandler.setMidiChannel(channel);
        midiHandler.setDeviceId(deviceId);
    }
    
    // Restore Upper Tone
    if (auto* upperTone = state->getChildByName("ToneUpper"))
    {
        loadToneFromXml(*upperTone, toneUpper);
    }
    
    // Restore Lower Tone
    if (auto* lowerTone = state->getChildByName("ToneLower"))
    {
        loadToneFromXml(*lowerTone, toneLower);
    }
    
    // Restore Patch
    if (auto* patch = state->getChildByName("Patch"))
    {
        loadPatchFromXml(*patch, patchParameters);
    }
    
    // Restore Arpeggiator
    if (auto* arp = state->getChildByName("Arpeggiator"))
    {
        loadArpFromXml(*arp, arpParameters);
    }
    
    // Restore Compare slots
    if (auto* compareSlots = state->getChildByName("CompareSlots"))
    {
        if (auto* slotA = compareSlots->getChildByName("SlotA"))
        {
            FullPatch patch;
            loadFullPatchFromXml(*slotA, patch);
            compareManager.storeToSlot(CompareManager::SLOT_A, patch);
        }
        if (auto* slotB = compareSlots->getChildByName("SlotB"))
        {
            FullPatch patch;
            loadFullPatchFromXml(*slotB, patch);
            compareManager.storeToSlot(CompareManager::SLOT_B, patch);
        }
    }
    
    // Restore Total Recall settings
    if (auto* totalRecall = state->getChildByName("TotalRecall"))
    {
        totalRecallEnabled = totalRecall->getBoolAttribute("enabled", true);
        sendSysExOnLoad = totalRecall->getBoolAttribute("sendOnLoad", true);
        requestSysExOnLoad = totalRecall->getBoolAttribute("requestOnLoad", false);
        sendOnEditEnabled = totalRecall->getBoolAttribute("sendOnEdit", true);
    }
    
    // Send to synth if Total Recall is enabled
    if (totalRecallEnabled && sendSysExOnLoad)
    {
        // Delay sending to allow MIDI to initialize
        juce::Timer::callAfterDelay(500, [this]()
        {
            DBG("Total Recall: Sending complete state to MKS-70...");
            sendPatchToSynth();
            sendToneToSynth(true);
            sendToneToSynth(false);
        });
    }
    
    // Notify editor to refresh
    if (auto* editor = dynamic_cast<MKS70EditorAudioProcessorEditor*>(getActiveEditor()))
    {
        juce::MessageManager::callAsync([editor]() {
            editor->refreshAllPanels();
        });
    }
    
    DBG("State restored successfully");
}

//==============================================================================
// XML Serialization helpers for Total Recall
void MKS70EditorAudioProcessor::storeToneToXml(juce::XmlElement& xml, const ToneParams& tone)
{
    // DCO1
    xml.setAttribute("dco1Range", tone.dco1Range);
    xml.setAttribute("dco1Waveform", tone.dco1Waveform);
    xml.setAttribute("dco1Tune", tone.dco1Tune);
    xml.setAttribute("dco1LFOMod", tone.dco1LFOMod);
    xml.setAttribute("dco1ENVMod", tone.dco1ENVMod);
    
    // DCO2
    xml.setAttribute("dco2Range", tone.dco2Range);
    xml.setAttribute("dco2Waveform", tone.dco2Waveform);
    xml.setAttribute("dco2CrossMod", tone.dco2CrossMod);
    xml.setAttribute("dco2Tune", tone.dco2Tune);
    xml.setAttribute("dco2FineTune", tone.dco2FineTune);
    xml.setAttribute("dco2LFOMod", tone.dco2LFOMod);
    xml.setAttribute("dco2ENVMod", tone.dco2ENVMod);
    
    // DCO Common
    xml.setAttribute("dcoDynamics", tone.dcoDynamics);
    xml.setAttribute("dcoENVMode", tone.dcoENVMode);
    
    // Mixer
    xml.setAttribute("mixerDCO1Level", tone.mixerDCO1Level);
    xml.setAttribute("mixerDCO2Level", tone.mixerDCO2Level);
    xml.setAttribute("mixerENVMod", tone.mixerENVMod);
    xml.setAttribute("mixerDynamics", tone.mixerDynamics);
    xml.setAttribute("mixerENVMode", tone.mixerENVMode);
    
    // VCF
    xml.setAttribute("vcfHPF", tone.vcfHPF);
    xml.setAttribute("vcfCutoff", tone.vcfCutoff);
    xml.setAttribute("vcfResonance", tone.vcfResonance);
    xml.setAttribute("vcfLFOMod", tone.vcfLFOMod);
    xml.setAttribute("vcfENVMod", tone.vcfENVMod);
    xml.setAttribute("vcfKeyFollow", tone.vcfKeyFollow);
    xml.setAttribute("vcfDynamics", tone.vcfDynamics);
    xml.setAttribute("vcfENVMode", tone.vcfENVMode);
    
    // VCA
    xml.setAttribute("vcaMode", tone.vcaMode);
    xml.setAttribute("vcaLevel", tone.vcaLevel);
    xml.setAttribute("vcaDynamics", tone.vcaDynamics);
    
    // ENV1 (7-Stage)
    xml.setAttribute("env1Time1", tone.env1Time1);
    xml.setAttribute("env1Level1", tone.env1Level1);
    xml.setAttribute("env1Time2", tone.env1Time2);
    xml.setAttribute("env1Level2", tone.env1Level2);
    xml.setAttribute("env1Time3", tone.env1Time3);
    xml.setAttribute("env1Level3", tone.env1Level3);
    xml.setAttribute("env1Time4", tone.env1Time4);
    xml.setAttribute("env1KeyFollow", tone.env1KeyFollow);
    
    // ENV2 (7-Stage)
    xml.setAttribute("env2Time1", tone.env2Time1);
    xml.setAttribute("env2Level1", tone.env2Level1);
    xml.setAttribute("env2Time2", tone.env2Time2);
    xml.setAttribute("env2Level2", tone.env2Level2);
    xml.setAttribute("env2Time3", tone.env2Time3);
    xml.setAttribute("env2Level3", tone.env2Level3);
    xml.setAttribute("env2Time4", tone.env2Time4);
    xml.setAttribute("env2KeyFollow", tone.env2KeyFollow);
    
    // ENV3 (ADSR for VCF)
    xml.setAttribute("env3Attack", tone.env3Attack);
    xml.setAttribute("env3Decay", tone.env3Decay);
    xml.setAttribute("env3Sustain", tone.env3Sustain);
    xml.setAttribute("env3Release", tone.env3Release);
    
    // ENV4 (ADSR for VCA)
    xml.setAttribute("env4Attack", tone.env4Attack);
    xml.setAttribute("env4Decay", tone.env4Decay);
    xml.setAttribute("env4Sustain", tone.env4Sustain);
    xml.setAttribute("env4Release", tone.env4Release);
    
    // LFO1
    xml.setAttribute("lfo1Waveform", tone.lfo1Waveform);
    xml.setAttribute("lfo1Delay", tone.lfo1Delay);
    xml.setAttribute("lfo1Rate", tone.lfo1Rate);
    
    // LFO2
    xml.setAttribute("lfo2Waveform", tone.lfo2Waveform);
    xml.setAttribute("lfo2Delay", tone.lfo2Delay);
    xml.setAttribute("lfo2Rate", tone.lfo2Rate);
    
    // Chorus
    xml.setAttribute("chorusMode", tone.chorusMode);
}

void MKS70EditorAudioProcessor::loadToneFromXml(const juce::XmlElement& xml, ToneParams& tone)
{
    // DCO1
    tone.dco1Range = xml.getIntAttribute("dco1Range", 1);
    tone.dco1Waveform = xml.getIntAttribute("dco1Waveform", 1);
    tone.dco1Tune = xml.getIntAttribute("dco1Tune", 64);
    tone.dco1LFOMod = xml.getIntAttribute("dco1LFOMod", 0);
    tone.dco1ENVMod = xml.getIntAttribute("dco1ENVMod", 0);
    
    // DCO2
    tone.dco2Range = xml.getIntAttribute("dco2Range", 1);
    tone.dco2Waveform = xml.getIntAttribute("dco2Waveform", 1);
    tone.dco2CrossMod = xml.getIntAttribute("dco2CrossMod", 0);
    tone.dco2Tune = xml.getIntAttribute("dco2Tune", 64);
    tone.dco2FineTune = xml.getIntAttribute("dco2FineTune", 64);
    tone.dco2LFOMod = xml.getIntAttribute("dco2LFOMod", 0);
    tone.dco2ENVMod = xml.getIntAttribute("dco2ENVMod", 0);
    
    // DCO Common
    tone.dcoDynamics = xml.getIntAttribute("dcoDynamics", 0);
    tone.dcoENVMode = xml.getIntAttribute("dcoENVMode", 0);
    
    // Mixer
    tone.mixerDCO1Level = xml.getIntAttribute("mixerDCO1Level", 100);
    tone.mixerDCO2Level = xml.getIntAttribute("mixerDCO2Level", 100);
    tone.mixerENVMod = xml.getIntAttribute("mixerENVMod", 0);
    tone.mixerDynamics = xml.getIntAttribute("mixerDynamics", 0);
    tone.mixerENVMode = xml.getIntAttribute("mixerENVMode", 0);
    
    // VCF
    tone.vcfHPF = xml.getIntAttribute("vcfHPF", 0);
    tone.vcfCutoff = xml.getIntAttribute("vcfCutoff", 127);
    tone.vcfResonance = xml.getIntAttribute("vcfResonance", 0);
    tone.vcfLFOMod = xml.getIntAttribute("vcfLFOMod", 0);
    tone.vcfENVMod = xml.getIntAttribute("vcfENVMod", 64);
    tone.vcfKeyFollow = xml.getIntAttribute("vcfKeyFollow", 64);
    tone.vcfDynamics = xml.getIntAttribute("vcfDynamics", 0);
    tone.vcfENVMode = xml.getIntAttribute("vcfENVMode", 0);
    
    // VCA
    tone.vcaMode = xml.getIntAttribute("vcaMode", 1);
    tone.vcaLevel = xml.getIntAttribute("vcaLevel", 127);
    tone.vcaDynamics = xml.getIntAttribute("vcaDynamics", 0);
    
    // ENV1 (7-Stage)
    tone.env1Time1 = xml.getIntAttribute("env1Time1", 0);
    tone.env1Level1 = xml.getIntAttribute("env1Level1", 100);
    tone.env1Time2 = xml.getIntAttribute("env1Time2", 64);
    tone.env1Level2 = xml.getIntAttribute("env1Level2", 80);
    tone.env1Time3 = xml.getIntAttribute("env1Time3", 64);
    tone.env1Level3 = xml.getIntAttribute("env1Level3", 100);
    tone.env1Time4 = xml.getIntAttribute("env1Time4", 64);
    tone.env1KeyFollow = xml.getIntAttribute("env1KeyFollow", 0);
    
    // ENV2 (7-Stage)
    tone.env2Time1 = xml.getIntAttribute("env2Time1", 0);
    tone.env2Level1 = xml.getIntAttribute("env2Level1", 100);
    tone.env2Time2 = xml.getIntAttribute("env2Time2", 64);
    tone.env2Level2 = xml.getIntAttribute("env2Level2", 80);
    tone.env2Time3 = xml.getIntAttribute("env2Time3", 64);
    tone.env2Level3 = xml.getIntAttribute("env2Level3", 100);
    tone.env2Time4 = xml.getIntAttribute("env2Time4", 64);
    tone.env2KeyFollow = xml.getIntAttribute("env2KeyFollow", 0);
    
    // ENV3 (ADSR for VCF)
    tone.env3Attack = xml.getIntAttribute("env3Attack", 0);
    tone.env3Decay = xml.getIntAttribute("env3Decay", 64);
    tone.env3Sustain = xml.getIntAttribute("env3Sustain", 100);
    tone.env3Release = xml.getIntAttribute("env3Release", 64);
    
    // ENV4 (ADSR for VCA)
    tone.env4Attack = xml.getIntAttribute("env4Attack", 0);
    tone.env4Decay = xml.getIntAttribute("env4Decay", 64);
    tone.env4Sustain = xml.getIntAttribute("env4Sustain", 100);
    tone.env4Release = xml.getIntAttribute("env4Release", 64);
    
    // LFO1
    tone.lfo1Waveform = xml.getIntAttribute("lfo1Waveform", 3);
    tone.lfo1Delay = xml.getIntAttribute("lfo1Delay", 0);
    tone.lfo1Rate = xml.getIntAttribute("lfo1Rate", 64);
    
    // LFO2
    tone.lfo2Waveform = xml.getIntAttribute("lfo2Waveform", 3);
    tone.lfo2Delay = xml.getIntAttribute("lfo2Delay", 0);
    tone.lfo2Rate = xml.getIntAttribute("lfo2Rate", 64);
    
    // Chorus
    tone.chorusMode = xml.getIntAttribute("chorusMode", 0);
}

void MKS70EditorAudioProcessor::storePatchToXml(juce::XmlElement& xml, const PatchParameters& patch)
{
    xml.setAttribute("toneName", patch.getToneName());
    xml.setAttribute("patchName", patch.getPatchName());
    xml.setAttribute("bankMSB", patch.bankMSB);
    xml.setAttribute("bankLSB", patch.bankLSB);
    xml.setAttribute("patchNumber", patch.patchNumber);
    xml.setAttribute("upperLowerBalance", patch.upperLowerBalance);
    xml.setAttribute("upperSplit", patch.upperSplit);
    xml.setAttribute("lowerSplit", patch.lowerSplit);
    xml.setAttribute("totalVolume", patch.totalVolume);
    xml.setAttribute("toneSel", patch.toneSel);
    xml.setAttribute("keyMode", patch.keyMode);
    xml.setAttribute("splitPoint", patch.splitPoint);
    xml.setAttribute("balance", patch.balance);
    xml.setAttribute("portamentoSwitch", patch.portamentoSwitch);
    xml.setAttribute("portamentoTime", patch.portamentoTime);
    xml.setAttribute("bendRange", patch.bendRange);
    xml.setAttribute("assignMode", patch.assignMode);
    xml.setAttribute("glide", patch.glide);
    xml.setAttribute("unisonDetune", patch.unisonDetune);
    xml.setAttribute("chasePlay", patch.chasePlay);
    xml.setAttribute("chaseLevel", patch.chaseLevel);
    xml.setAttribute("chaseTime", patch.chaseTime);
}

void MKS70EditorAudioProcessor::loadPatchFromXml(const juce::XmlElement& xml, PatchParameters& patch)
{
    patch.setToneName(xml.getStringAttribute("toneName", "Init Tone"));
    patch.setPatchName(xml.getStringAttribute("patchName", "Init Patch"));
    patch.bankMSB = xml.getIntAttribute("bankMSB", 0);
    patch.bankLSB = xml.getIntAttribute("bankLSB", 0);
    patch.patchNumber = xml.getIntAttribute("patchNumber", 0);
    patch.upperLowerBalance = xml.getIntAttribute("upperLowerBalance", 64);
    patch.upperSplit = xml.getIntAttribute("upperSplit", 60);
    patch.lowerSplit = xml.getIntAttribute("lowerSplit", 60);
    patch.totalVolume = xml.getIntAttribute("totalVolume", 100);
    patch.toneSel = xml.getIntAttribute("toneSel", 0);
    patch.keyMode = xml.getIntAttribute("keyMode", 0);
    patch.splitPoint = xml.getIntAttribute("splitPoint", 60);
    patch.balance = xml.getIntAttribute("balance", 64);
    patch.portamentoSwitch = xml.getIntAttribute("portamentoSwitch", 0);
    patch.portamentoTime = xml.getIntAttribute("portamentoTime", 0);
    patch.bendRange = xml.getIntAttribute("bendRange", 2);
    patch.assignMode = xml.getIntAttribute("assignMode", 0);
    patch.glide = xml.getIntAttribute("glide", 0);
    patch.unisonDetune = xml.getIntAttribute("unisonDetune", 0);
    patch.chasePlay = xml.getIntAttribute("chasePlay", 0);
    patch.chaseLevel = xml.getIntAttribute("chaseLevel", 0);
    patch.chaseTime = xml.getIntAttribute("chaseTime", 0);
}

void MKS70EditorAudioProcessor::storeArpToXml(juce::XmlElement& xml, const ArpParameters& arp)
{
    xml.setAttribute("arpOn", arp.arpOn);
    xml.setAttribute("tempo", arp.tempo);
    xml.setAttribute("mode", arp.mode);
    xml.setAttribute("octaveSpan", arp.octaveSpan);
    xml.setAttribute("insert", arp.insert);
    xml.setAttribute("playRate", arp.playRate);
    xml.setAttribute("duration", arp.duration);
    xml.setAttribute("legato", arp.legato);
    xml.setAttribute("velocityMode", arp.velocityMode);
    xml.setAttribute("fixedVelocity", arp.fixedVelocity);
    xml.setAttribute("midiOut", arp.midiOut);
    xml.setAttribute("sync", arp.sync);
    xml.setAttribute("latch", arp.latch);
}

void MKS70EditorAudioProcessor::loadArpFromXml(const juce::XmlElement& xml, ArpParameters& arp)
{
    arp.arpOn = xml.getIntAttribute("arpOn", 0);
    arp.tempo = xml.getIntAttribute("tempo", 120);
    arp.mode = xml.getIntAttribute("mode", 0);
    arp.octaveSpan = xml.getIntAttribute("octaveSpan", 1);
    arp.insert = xml.getIntAttribute("insert", 0);
    arp.playRate = xml.getIntAttribute("playRate", 2);
    arp.duration = xml.getIntAttribute("duration", 8);
    arp.legato = xml.getIntAttribute("legato", 0);
    arp.velocityMode = xml.getIntAttribute("velocityMode", 1);
    arp.fixedVelocity = xml.getIntAttribute("fixedVelocity", 100);
    arp.midiOut = xml.getIntAttribute("midiOut", 1);
    arp.sync = xml.getIntAttribute("sync", 0);
    arp.latch = xml.getIntAttribute("latch", 0);
}

void MKS70EditorAudioProcessor::storeFullPatchToXml(juce::XmlElement& xml, const FullPatch& fullPatch)
{
    xml.setAttribute("name", fullPatch.name);
    
    auto* patch = xml.createNewChildElement("Patch");
    storePatchToXml(*patch, fullPatch.patch);
    
    auto* upper = xml.createNewChildElement("ToneUpper");
    storeToneToXml(*upper, fullPatch.toneUpper);
    
    auto* lower = xml.createNewChildElement("ToneLower");
    storeToneToXml(*lower, fullPatch.toneLower);
    
    auto* arp = xml.createNewChildElement("Arp");
    storeArpToXml(*arp, fullPatch.arp);
}

void MKS70EditorAudioProcessor::loadFullPatchFromXml(const juce::XmlElement& xml, FullPatch& fullPatch)
{
    fullPatch.name = xml.getStringAttribute("name", "Unnamed");
    
    if (auto* patch = xml.getChildByName("Patch"))
        loadPatchFromXml(*patch, fullPatch.patch);
    
    if (auto* upper = xml.getChildByName("ToneUpper"))
        loadToneFromXml(*upper, fullPatch.toneUpper);
    
    if (auto* lower = xml.getChildByName("ToneLower"))
        loadToneFromXml(*lower, fullPatch.toneLower);
    
    if (auto* arp = xml.getChildByName("Arp"))
        loadArpFromXml(*arp, fullPatch.arp);
}

//==============================================================================
// Parameter setters with MIDI send and UNDO support
int MKS70EditorAudioProcessor::getToneParameterValue(const ToneParams& tone, int paramIndex) const
{
    switch (paramIndex)
    {
        case MKS70Mapping::DCO1_RANGE_SYSEX: return tone.dco1Range;
        case MKS70Mapping::DCO1_WAVEFORM_SYSEX: return tone.dco1Waveform;
        case MKS70Mapping::DCO1_TUNE_SYSEX: return tone.dco1Tune;
        case MKS70Mapping::DCO1_LFO_DEPTH_SYSEX: return tone.dco1LFOMod;
        case MKS70Mapping::DCO1_ENV_DEPTH_SYSEX: return tone.dco1ENVMod;
        case MKS70Mapping::DCO1_DYNAMICS_SYSEX: return tone.dcoDynamics;
        case MKS70Mapping::DCO1_ENV_SOURCE_SYSEX: return tone.dcoENVMode;

        case MKS70Mapping::DCO2_RANGE_SYSEX: return tone.dco2Range;
        case MKS70Mapping::DCO2_WAVEFORM_SYSEX: return tone.dco2Waveform;
        case MKS70Mapping::DCO2_TUNE_SYSEX: return tone.dco2Tune;
        case MKS70Mapping::DCO2_FINETUNE_SYSEX: return tone.dco2FineTune;
        case MKS70Mapping::DCO2_LFO_DEPTH_SYSEX: return tone.dco2LFOMod;
        case MKS70Mapping::DCO2_ENV_DEPTH_SYSEX: return tone.dco2ENVMod;
        case MKS70Mapping::DCO_CROSSMOD_SYSEX: return tone.dco2CrossMod;

        case MKS70Mapping::MIXER_DCO1_LEVEL_SYSEX: return tone.mixerDCO1Level;
        case MKS70Mapping::MIXER_DCO2_LEVEL_SYSEX: return tone.mixerDCO2Level;
        case MKS70Mapping::MIXER_ENV_DEPTH_SYSEX: return tone.mixerENVMod;
        case MKS70Mapping::MIXER_DYNAMICS_SYSEX: return tone.mixerDynamics;
        case MKS70Mapping::MIXER_ENV_SOURCE_SYSEX: return tone.mixerENVMode;

        case MKS70Mapping::HPF_CUTOFF_SYSEX: return tone.vcfHPF;
        case MKS70Mapping::VCF_CUTOFF_SYSEX: return tone.vcfCutoff;
        case MKS70Mapping::VCF_RESONANCE_SYSEX: return tone.vcfResonance;
        case MKS70Mapping::VCF_LFO1_DEPTH_SYSEX: return tone.vcfLFOMod;
        case MKS70Mapping::VCF_ENV_DEPTH_SYSEX: return tone.vcfENVMod;
        case MKS70Mapping::VCF_KEY_FOLLOW_SYSEX: return tone.vcfKeyFollow;
        case MKS70Mapping::VCF_DYNAMICS_SYSEX: return tone.vcfDynamics;
        case MKS70Mapping::VCF_ENV_SOURCE_SYSEX: return tone.vcfENVMode;

        case MKS70Mapping::VCA_LEVEL_SYSEX: return tone.vcaLevel;
        case MKS70Mapping::VCA_ENV_SOURCE_SYSEX: return tone.vcaMode;
        case MKS70Mapping::VCA_DYNAMICS_SYSEX: return tone.vcaDynamics;

        case MKS70Mapping::LFO1_WAVEFORM_SYSEX: return tone.lfo1Waveform;
        case MKS70Mapping::LFO1_DELAY_SYSEX: return tone.lfo1Delay;
        case MKS70Mapping::LFO1_RATE_SYSEX: return tone.lfo1Rate;

        case MKS70Mapping::ENV1_TIME1_SYSEX: return tone.env1Time1;
        case MKS70Mapping::ENV1_TIME2_SYSEX: return tone.env1Time2;
        case MKS70Mapping::ENV1_LEVEL3_SYSEX: return tone.env1Level3;
        case MKS70Mapping::ENV1_TIME4_SYSEX: return tone.env1Time4;
        case MKS70Mapping::ENV1_KEYFOLLOW_LOOP_SYSEX: return tone.env1KeyFollow;

        case MKS70Mapping::ENV2_TIME1_SYSEX: return tone.env2Time1;
        case MKS70Mapping::ENV2_TIME2_SYSEX: return tone.env2Time2;
        case MKS70Mapping::ENV2_LEVEL3_SYSEX: return tone.env2Level3;
        case MKS70Mapping::ENV2_TIME4_SYSEX: return tone.env2Time4;
        case MKS70Mapping::ENV2_KEYFOLLOW_LOOP_SYSEX: return tone.env2KeyFollow;

        case MKS70Mapping::CHORUS_SYSEX: return tone.chorusMode;
        default: return 0;
    }
}

void MKS70EditorAudioProcessor::updateToneParameterValue(ToneParams& tone, int paramIndex, int value)
{
    const int clampedValue = juce::jlimit(0, 127, value);

    switch (paramIndex)
    {
        case MKS70Mapping::DCO1_RANGE_SYSEX: tone.dco1Range = juce::jlimit(0, 3, value); break;
        case MKS70Mapping::DCO1_WAVEFORM_SYSEX: tone.dco1Waveform = juce::jlimit(0, 3, value); break;
        case MKS70Mapping::DCO1_TUNE_SYSEX: tone.dco1Tune = clampedValue; break;
        case MKS70Mapping::DCO1_LFO_DEPTH_SYSEX: tone.dco1LFOMod = clampedValue; break;
        case MKS70Mapping::DCO1_ENV_DEPTH_SYSEX: tone.dco1ENVMod = clampedValue; break;
        case MKS70Mapping::DCO1_DYNAMICS_SYSEX: tone.dcoDynamics = juce::jlimit(0, 3, value); break;
        case MKS70Mapping::DCO1_ENV_SOURCE_SYSEX: tone.dcoENVMode = juce::jlimit(0, 3, value); break;

        case MKS70Mapping::DCO2_RANGE_SYSEX: tone.dco2Range = juce::jlimit(0, 3, value); break;
        case MKS70Mapping::DCO2_WAVEFORM_SYSEX: tone.dco2Waveform = juce::jlimit(0, 3, value); break;
        case MKS70Mapping::DCO2_TUNE_SYSEX: tone.dco2Tune = clampedValue; break;
        case MKS70Mapping::DCO2_FINETUNE_SYSEX: tone.dco2FineTune = clampedValue; break;
        case MKS70Mapping::DCO2_LFO_DEPTH_SYSEX: tone.dco2LFOMod = clampedValue; break;
        case MKS70Mapping::DCO2_ENV_DEPTH_SYSEX: tone.dco2ENVMod = clampedValue; break;
        case MKS70Mapping::DCO_CROSSMOD_SYSEX: tone.dco2CrossMod = juce::jlimit(0, 3, value); break;

        case MKS70Mapping::MIXER_DCO1_LEVEL_SYSEX: tone.mixerDCO1Level = clampedValue; break;
        case MKS70Mapping::MIXER_DCO2_LEVEL_SYSEX: tone.mixerDCO2Level = clampedValue; break;
        case MKS70Mapping::MIXER_ENV_DEPTH_SYSEX: tone.mixerENVMod = clampedValue; break;
        case MKS70Mapping::MIXER_DYNAMICS_SYSEX: tone.mixerDynamics = juce::jlimit(0, 3, value); break;
        case MKS70Mapping::MIXER_ENV_SOURCE_SYSEX: tone.mixerENVMode = juce::jlimit(0, 3, value); break;

        case MKS70Mapping::HPF_CUTOFF_SYSEX: tone.vcfHPF = juce::jlimit(0, 3, value); break;
        case MKS70Mapping::VCF_CUTOFF_SYSEX: tone.vcfCutoff = clampedValue; break;
        case MKS70Mapping::VCF_RESONANCE_SYSEX: tone.vcfResonance = clampedValue; break;
        case MKS70Mapping::VCF_LFO1_DEPTH_SYSEX: tone.vcfLFOMod = clampedValue; break;
        case MKS70Mapping::VCF_ENV_DEPTH_SYSEX: tone.vcfENVMod = clampedValue; break;
        case MKS70Mapping::VCF_KEY_FOLLOW_SYSEX: tone.vcfKeyFollow = clampedValue; break;
        case MKS70Mapping::VCF_DYNAMICS_SYSEX: tone.vcfDynamics = juce::jlimit(0, 3, value); break;
        case MKS70Mapping::VCF_ENV_SOURCE_SYSEX: tone.vcfENVMode = juce::jlimit(0, 3, value); break;

        case MKS70Mapping::VCA_LEVEL_SYSEX: tone.vcaLevel = clampedValue; break;
        case MKS70Mapping::VCA_ENV_SOURCE_SYSEX: tone.vcaMode = juce::jlimit(0, 1, value); break;
        case MKS70Mapping::VCA_DYNAMICS_SYSEX: tone.vcaDynamics = juce::jlimit(0, 3, value); break;

        case MKS70Mapping::LFO1_WAVEFORM_SYSEX: tone.lfo1Waveform = juce::jlimit(0, 3, value); break;
        case MKS70Mapping::LFO1_DELAY_SYSEX: tone.lfo1Delay = clampedValue; break;
        case MKS70Mapping::LFO1_RATE_SYSEX: tone.lfo1Rate = clampedValue; break;

        case MKS70Mapping::ENV1_TIME1_SYSEX: tone.env1Time1 = clampedValue; break;
        case MKS70Mapping::ENV1_TIME2_SYSEX: tone.env1Time2 = clampedValue; break;
        case MKS70Mapping::ENV1_LEVEL3_SYSEX: tone.env1Level3 = clampedValue; break;
        case MKS70Mapping::ENV1_TIME4_SYSEX: tone.env1Time4 = clampedValue; break;
        case MKS70Mapping::ENV1_KEYFOLLOW_LOOP_SYSEX: tone.env1KeyFollow = juce::jlimit(0, 3, value); break;

        case MKS70Mapping::ENV2_TIME1_SYSEX: tone.env2Time1 = clampedValue; break;
        case MKS70Mapping::ENV2_TIME2_SYSEX: tone.env2Time2 = clampedValue; break;
        case MKS70Mapping::ENV2_LEVEL3_SYSEX: tone.env2Level3 = clampedValue; break;
        case MKS70Mapping::ENV2_TIME4_SYSEX: tone.env2Time4 = clampedValue; break;
        case MKS70Mapping::ENV2_KEYFOLLOW_LOOP_SYSEX: tone.env2KeyFollow = juce::jlimit(0, 3, value); break;

        case MKS70Mapping::CHORUS_SYSEX: tone.chorusMode = juce::jlimit(0, 2, value); break;
        default: break;
    }
}

void MKS70EditorAudioProcessor::setToneParameter(bool isUpper, int paramIndex, int value)
{
    if (paramIndex == MKS70Mapping::PARAM_NOT_AVAILABLE || paramIndex == 0xFF)
        return;

    ToneParams& tone = isUpper ? toneUpper : toneLower;
    const int oldValue = getToneParameterValue(tone, paramIndex);

    auto applyValue = [this, isUpper, paramIndex](int newValue)
    {
        ToneParams& targetTone = isUpper ? toneUpper : toneLower;
        updateToneParameterValue(targetTone, paramIndex, newValue);
        if (sendOnEditEnabled)
            midiHandler.sendToneParameter(isUpper ? MKS70Mapping::TONE_UPPER : MKS70Mapping::TONE_LOWER,
                                          paramIndex,
                                          MKS70Mapping::mapToneValueForSysex(paramIndex, newValue));
    };

    if (oldValue == value)
    {
        applyValue(value);
        return;
    }

    const juce::String paramName = "Param " + juce::String(paramIndex);
    const juce::String section = isUpper ? "Upper Tone" : "Lower Tone";
    undoManager.performParameterChange(paramName, section, oldValue, value, applyValue);
}

void MKS70EditorAudioProcessor::setToneParameters(bool isUpper, const ToneParams& params, bool sendToSynth)
{
    ToneParams& tone = isUpper ? toneUpper : toneLower;
    tone = params;

    if (sendToSynth)
        sendToneToSynth(isUpper);
}

void MKS70EditorAudioProcessor::setPatchParameter(int paramIndex, int value)
{
    updatePatchParameterValue(patchParameters, paramIndex, value);
    if (sendOnEditEnabled)
        midiHandler.sendPatchParameter(paramIndex, value);
}

void MKS70EditorAudioProcessor::setPatchParameters(const PatchParameters& params, bool sendToSynth)
{
    patchParameters = params;

    if (sendToSynth)
        sendPatchToSynth();
}

void MKS70EditorAudioProcessor::setArpParameter(int paramIndex, int value)
{
    updateArpParameterValue(arpParameters, paramIndex, value);
    if (sendOnEditEnabled)
        midiHandler.sendNRPN(1, 2, paramIndex, value);  // NRPN MSB=2 for arp
}

void MKS70EditorAudioProcessor::setArpParameters(const ArpParameters& params, bool sendToSynth)
{
    arpParameters = params;

    if (!sendToSynth)
        return;

    midiHandler.sendNRPN(1, 2, 15, arpParameters.arpOn);
    midiHandler.sendNRPN(1, 2, 16, arpParameters.tempo);
    midiHandler.sendNRPN(1, 2, 17, arpParameters.mode);
    midiHandler.sendNRPN(1, 2, 18, arpParameters.octaveSpan);
    midiHandler.sendNRPN(1, 2, 19, arpParameters.playRate);
    midiHandler.sendNRPN(1, 2, 20, arpParameters.latch);
}

void MKS70EditorAudioProcessor::updatePatchParameterValue(PatchParameters& patch, int paramIndex, int value)
{
    const int clampedValue = juce::jlimit(0, 127, value);

    switch (paramIndex)
    {
        case 0x10: patch.toneSel = juce::jlimit(0, 2, value); break;
        case 0x11: patch.keyMode = juce::jlimit(0, 3, value); break;
        case 0x12:
            patch.balance = clampedValue;
            patch.upperLowerBalance = clampedValue;
            break;
        case 0x13: patch.assignMode = juce::jlimit(0, 5, value); break;
        case 0x14: patch.portamentoSwitch = value != 0 ? 1 : 0; break;
        case 0x15: patch.portamentoTime = clampedValue; break;
        case 0x16: patch.glide = value != 0 ? 1 : 0; break;
        case 0x18: patch.bendRange = juce::jlimit(0, 12, value); break;
        case 0x19:
            patch.splitPoint = clampedValue;
            patch.upperSplit = clampedValue;
            break;
        case 0x1A: patch.lowerSplit = clampedValue; break;
        case 0x1C: patch.totalVolume = clampedValue; break;
        case 0x1D: patch.unisonDetune = clampedValue; break;
        case 0x20: patch.chasePlay = value != 0 ? 1 : 0; break;
        case 0x21: patch.chaseLevel = clampedValue; break;
        case 0x22: patch.chaseTime = clampedValue; break;
        default: break;
    }
}

void MKS70EditorAudioProcessor::updateArpParameterValue(ArpParameters& arp, int paramIndex, int value)
{
    switch (paramIndex)
    {
        case 15: arp.arpOn = value != 0 ? 1 : 0; break;
        case 16: arp.tempo = juce::jlimit(20, 300, value); break;
        case 17: arp.mode = juce::jlimit(0, 4, value); break;
        case 18: arp.octaveSpan = juce::jlimit(1, 4, value); break;
        case 19: arp.playRate = juce::jlimit(0, 7, value); break;
        case 20: arp.latch = value != 0 ? 1 : 0; break;
        default: break;
    }
}

//==============================================================================
// Request functions
void MKS70EditorAudioProcessor::requestToneDump(bool isUpper)
{
    midiHandler.requestToneDump(isUpper ? 0 : 1);
}

void MKS70EditorAudioProcessor::requestPatchDump()
{
    midiHandler.requestPatchDump();
}

void MKS70EditorAudioProcessor::requestBulkDump(int bank)
{
    midiHandler.requestBulkDump(bank);
}

//==============================================================================
// Send complete data
void MKS70EditorAudioProcessor::sendToneToSynth(bool isUpper)
{
    ToneParams& tone = isUpper ? toneUpper : toneLower;
    // MKS-70 tone IDs: 1 = Upper (Tone A), 2 = Lower (Tone B)
    midiHandler.sendCompleteTone(isUpper ? 1 : 2, tone);
}

void MKS70EditorAudioProcessor::sendPatchToSynth()
{
    midiHandler.sendCompletePatch(patchParameters);
}

//==============================================================================
// Compare mode operations
void MKS70EditorAudioProcessor::storeCurrentToSlot(CompareManager::Slot slot)
{
    FullPatch currentPatch(patchParameters, toneUpper, toneLower, arpParameters, "Current Edit");
    compareManager.storeToSlot(slot, currentPatch);
    DBG("Stored current state to slot: " + compareManager.getSlotName(slot));
}

void MKS70EditorAudioProcessor::loadSlotToCurrent(CompareManager::Slot slot)
{
    FullPatch patch = compareManager.getSlot(slot);
    
    if (!patch.name.isEmpty())
    {
        patchParameters = patch.patch;
        toneUpper = patch.toneUpper;
        toneLower = patch.toneLower;
        arpParameters = patch.arp;
        
        // Send all to synth
        sendPatchToSynth();
        sendToneToSynth(true);
        sendToneToSynth(false);
        
        DBG("Loaded slot to current: " + compareManager.getSlotName(slot));
    }
}

std::vector<ParameterDiff> MKS70EditorAudioProcessor::compareAB()
{
    return compareManager.compareSlots(CompareManager::Slot::SLOT_A, CompareManager::Slot::SLOT_B);
}

//==============================================================================
// MidiHandler callbacks
void MKS70EditorAudioProcessor::handleToneReceived(ToneParams& tone, int toneNumber)
{
    if (toneNumber == 0 || toneNumber == MKS70Mapping::TONE_UPPER)
        toneUpper = tone;
    else if (toneNumber == MKS70Mapping::TONE_LOWER)
        toneLower = tone;
    
    DBG("Tone received for " + juce::String((toneNumber == MKS70Mapping::TONE_LOWER) ? "Lower" : "Upper"));
    
    // Notify editor to update UI
    if (auto* editor = dynamic_cast<MKS70EditorAudioProcessorEditor*>(getActiveEditor()))
    {
        // Editor will pull updated data
        juce::MessageManager::callAsync([editor]() {
            editor->refreshUI();
        });
    }
}

void MKS70EditorAudioProcessor::handleToneParameterReceived(int toneNumber, int paramIndex, int value)
{
    ToneParams& tone = (toneNumber == MKS70Mapping::TONE_LOWER) ? toneLower : toneUpper;
    updateToneParameterValue(tone, paramIndex, value);

    DBG("Tone parameter received: " + juce::String(toneNumber == MKS70Mapping::TONE_LOWER ? "Lower" : "Upper") +
        " param " + juce::String(paramIndex) + " = " + juce::String(value));

    if (auto* editor = dynamic_cast<MKS70EditorAudioProcessorEditor*>(getActiveEditor()))
    {
        juce::MessageManager::callAsync([editor]() {
            editor->refreshUI();
        });
    }
}

void MKS70EditorAudioProcessor::handlePatchReceived(PatchParameters& patch)
{
    patchParameters = patch;
    DBG("Patch received");
    
    // Notify editor
    if (auto* editor = dynamic_cast<MKS70EditorAudioProcessorEditor*>(getActiveEditor()))
    {
        juce::MessageManager::callAsync([editor]() {
            editor->refreshUI();
        });
    }
}

void MKS70EditorAudioProcessor::handlePatchParameterReceived(int paramIndex, int value)
{
    updatePatchParameterValue(patchParameters, paramIndex, value);
    DBG("Patch parameter received: " + juce::String(paramIndex) + " = " + juce::String(value));

    if (auto* editor = dynamic_cast<MKS70EditorAudioProcessorEditor*>(getActiveEditor()))
    {
        juce::MessageManager::callAsync([editor]() {
            editor->refreshUI();
        });
    }
}

void MKS70EditorAudioProcessor::handleBulkDumpReceived(const PatchBank& bank)
{
    DBG("Bulk dump received - Bank " + juce::String(bank.bankNumber) + 
        " with " + juce::String(bank.patches.size()) + " patches");
    
    // This would typically be saved to the librarian
    // For now just log it
}

void MKS70EditorAudioProcessor::handleMidiError(const juce::String& error)
{
    DBG("MIDI Error: " + error);
    
    // Don't show blocking AlertWindow - just log the error
    // The UI will show the error in the status bar or console
    // This prevents the UI from freezing when MIDI device is not connected
}

//==============================================================================
// Compare mode operations - TODO: Implement when CompareManager is added

/*
void MKS70EditorAudioProcessor::storeCurrentToSlot(CompareManager::Slot slot)
{
    FullPatch current(patchParameters, toneUpper, toneLower, arpParameters, 
                      "Current Patch");
    compareManager.storeToSlot(slot, current);
    
    DBG("Stored current patch to " + CompareManager::getSlotName(slot));
}

void MKS70EditorAudioProcessor::loadSlotToCurrent(CompareManager::Slot slot)
{
    if (!compareManager.hasSlot(slot))
    {
        DBG("Cannot load - slot is empty");
        return;
    }
    
    const FullPatch& patch = compareManager.getSlot(slot);
    
    // Load all parameters
    patchParameters = patch.patch;
    toneUpper = patch.toneUpper;
    toneLower = patch.toneLower;
    arpParameters = patch.arp;
    
    // Send to synth
    sendPatchToSynth();
    sendToneToSynth(true);  // Upper
    sendToneToSynth(false); // Lower
    
    // Notify editor
    auto* editor = dynamic_cast<MKS70EditorAudioProcessorEditor*>(getActiveEditor());
    if (editor)
    {
        juce::MessageManager::callAsync([editor]() {
            editor->refreshUI();
        });
    }
    
    DBG("Loaded patch from " + CompareManager::getSlotName(slot));
}

void MKS70EditorAudioProcessor::compareAB()
{
    auto diffs = compareManager.compareSlots(CompareManager::SLOT_A, CompareManager::SLOT_B);
    
    DBG("Compare A/B: Found " + juce::String(diffs.size()) + " differences");
    
    // Log first 10 differences for debugging
    for (int i = 0; i < juce::jmin(10, static_cast<int>(diffs.size())); ++i)
    {
        const auto& diff = diffs[i];
        DBG("  " + diff.section + " - " + diff.parameterName + 
            ": " + diff.valueADisplay + " -> " + diff.valueBDisplay);
    }
}
*/

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MKS70EditorAudioProcessor();
}
