#include "MidiLearnManager.h"

using Type = MidiLearnManager::MidiMapping::Type;

static juce::String typeToString(Type t)
{
    switch (t)
    {
        case Type::CC: return "CC"; 
        case Type::NRPN: return "NRPN"; 
        case Type::NOTE: return "NOTE"; 
        case Type::AFTERTOUCH: return "AFTERTOUCH"; 
        case Type::PITCHBEND: return "PITCHBEND"; 
        default: return "CC";
    }
}

static Type stringToType(const juce::String& s)
{
    auto up = s.toUpperCase();
    if (up == "CC") return Type::CC;
    if (up == "NRPN") return Type::NRPN;
    if (up == "NOTE") return Type::NOTE;
    if (up == "AFTERTOUCH") return Type::AFTERTOUCH;
    if (up == "PITCHBEND") return Type::PITCHBEND;
    return Type::CC;
}

int MidiLearnManager::scaleInt(int inVal, int inMin, int inMax, int outMin, int outMax, bool inverted)
{
    if (inMax == inMin) return outMin;
    inVal = clampInt(inVal, inMin, inMax);
    if (inverted)
        inVal = inMin + (inMax - (inVal - inMin));
    auto norm = (inVal - inMin) / double(inMax - inMin);
    auto out = outMin + norm * double(outMax - outMin);
    return clampInt(int(std::round(out)), outMin, outMax);
}

void MidiLearnManager::startLearn(const juce::String& targetParamId)
{
    currentLearningParam = targetParamId;
    learning = true;
}

void MidiLearnManager::stopLearn()
{
    learning = false;
    currentLearningParam = {};
}

void MidiLearnManager::addMapping(const MidiMapping& m)
{
    mappings[m.paramId] = m;
    // refresh cache
    allMappingsCache.clear();
    allMappingsCache.reserve(mappings.size());
    for (auto& kv : mappings) allMappingsCache.push_back(kv.second);
}

void MidiLearnManager::removeMapping(const juce::String& paramId)
{
    mappings.erase(paramId);
    lastOutputForParam.erase(paramId);
    lastInputForParam.erase(paramId);
    paramSinks.erase(paramId);
    // refresh cache
    allMappingsCache.clear();
    allMappingsCache.reserve(mappings.size());
    for (auto& kv : mappings) allMappingsCache.push_back(kv.second);
}

MidiLearnManager::MidiMapping* MidiLearnManager::getMappingForParam(const juce::String& paramId)
{
    auto it = mappings.find(paramId);
    if (it == mappings.end()) return nullptr;
    return &it->second;
}

std::vector<MidiLearnManager::MidiMapping>& MidiLearnManager::getAllMappings()
{
    // ensure cache is up to date
    allMappingsCache.clear();
    allMappingsCache.reserve(mappings.size());
    for (auto& kv : mappings) allMappingsCache.push_back(kv.second);
    return allMappingsCache;
}

void MidiLearnManager::saveMappingPreset(const juce::File& file)
{
    juce::DynamicObject::Ptr rootObj = new juce::DynamicObject();
    juce::var arr(juce::Array<juce::var>{});
    for (auto& kv : mappings)
    {
        const auto& m = kv.second;
        juce::DynamicObject::Ptr o = new juce::DynamicObject();
        o->setProperty("paramId", m.paramId);
        o->setProperty("channel", m.channel);
        o->setProperty("type", typeToString(m.type));
        o->setProperty("controller", m.controller);
        o->setProperty("minIn", m.minIn);
        o->setProperty("maxIn", m.maxIn);
        o->setProperty("minOut", m.minOut);
        o->setProperty("maxOut", m.maxOut);
        o->setProperty("inverted", m.inverted);
        o->setProperty("relative", m.relative);
        o->setProperty("smoothing", m.smoothing);
        arr.append(juce::var(o.get()));
    }
    rootObj->setProperty("mappings", arr);
    juce::var root(rootObj.get());
    file.replaceWithText(juce::JSON::toString(root, true));
}

void MidiLearnManager::loadMappingPreset(const juce::File& file)
{
    if (!file.existsAsFile()) return;
    juce::var root = juce::JSON::parse(file);
    if (!root.isObject()) return;
    clearAllMappings();
    auto* obj = root.getDynamicObject();
    auto arr = obj->getProperty("mappings");
    if (arr.isArray())
    {
        for (auto& v : *arr.getArray())
        {
            if (!v.isObject()) continue;
            auto* o = v.getDynamicObject();
            MidiMapping m;
            m.paramId = o->getProperty("paramId").toString();
            m.channel = int(o->getProperty("channel"));
            m.type = stringToType(o->getProperty("type").toString());
            m.controller = int(o->getProperty("controller"));
            m.minIn = int(o->getProperty("minIn"));
            m.maxIn = int(o->getProperty("maxIn"));
            m.minOut = int(o->getProperty("minOut"));
            m.maxOut = int(o->getProperty("maxOut"));
            m.inverted = bool(o->getProperty("inverted"));
            m.relative = bool(o->getProperty("relative"));
            m.smoothing = float(double(o->getProperty("smoothing")));
            addMapping(m);
        }
    }
}

void MidiLearnManager::clearAllMappings()
{
    mappings.clear();
    allMappingsCache.clear();
    lastOutputForParam.clear();
    lastInputForParam.clear();
    paramSinks.clear();
}

void MidiLearnManager::registerParamSink(const juce::String& paramId, std::function<void(int)> sink)
{
    paramSinks[paramId] = std::move(sink);
}

void MidiLearnManager::unregisterParamSink(const juce::String& paramId)
{
    paramSinks.erase(paramId);
}

void MidiLearnManager::dispatchValue(const juce::String& paramId, int value)
{
    if (auto it = paramSinks.find(paramId); it != paramSinks.end())
    {
        it->second(value);
    }
    if (onMappedParameterChange)
        onMappedParameterChange(paramId, value);
}

void MidiLearnManager::processMidiMessage(const juce::MidiMessage& msg)
{
    const int ch = msg.getChannel() - 1; // 0..15

    // Learning: pick first eligible message
    if (learning && currentLearningParam.isNotEmpty())
    {
        MidiMapping m;
        m.paramId = currentLearningParam;
        m.channel = ch;
        if (msg.isController())
        {
            m.type = Type::CC;
            m.controller = msg.getControllerNumber();
            m.minIn = 0; m.maxIn = 127;
        }
        else if (msg.isPitchWheel())
        {
            m.type = Type::PITCHBEND;
            m.controller = 0;
            m.minIn = 0; m.maxIn = 16383;
        }
        else if (msg.isAftertouch() || msg.isChannelPressure())
        {
            m.type = Type::AFTERTOUCH;
            m.controller = 0;
            m.minIn = 0; m.maxIn = 127;
        }
        else if (msg.isNoteOn())
        {
            m.type = Type::NOTE;
            m.controller = msg.getNoteNumber();
            m.minIn = 0; m.maxIn = 127;
        }
        else
        {
            // For NRPN learn we need to capture after CC 99/98 then CC6
            if (msg.isController() && (msg.getControllerNumber() == 99 || msg.getControllerNumber() == 98))
            {
                // Wait for data entry to complete
                return;
            }
            // Unknown for learning; ignore
            return;
        }

        addMapping(m);
        if (onLearnComplete) onLearnComplete(m);
        stopLearn();
        return;
    }

    // Not learning: apply mappings
    // Handle NRPN accumulation per channel
    if (msg.isController())
    {
        const int cc = msg.getControllerNumber();
        const int val = msg.getControllerValue();

        if (cc == 99) { nrpnStates[ch].nrpnMsb = val; }
        else if (cc == 98) { nrpnStates[ch].nrpnLsb = val; }
        else if (cc == 6) { nrpnStates[ch].dataMsb = val; }
        else if (cc == 38) { nrpnStates[ch].dataLsb = val; }
        else if (cc == 96) { // Data increment
            if (nrpnStates[ch].hasNumber()) nrpnStates[ch].dataLsb = juce::jlimit(0,127, (nrpnStates[ch].dataLsb<0?0:nrpnStates[ch].dataLsb)+1);
        }
        else if (cc == 97) { // Data decrement
            if (nrpnStates[ch].hasNumber()) nrpnStates[ch].dataLsb = juce::jlimit(0,127, (nrpnStates[ch].dataLsb<0?0:nrpnStates[ch].dataLsb)-1);
        }

        // CC mappings
        for (auto& kv : mappings)
        {
            auto& m = kv.second;
            if (m.type != Type::CC) continue;
            if (m.controller != cc) continue;
            if (!(m.channel < 0 || m.channel == ch)) continue;

            int inputVal = val;
            if (m.relative)
            {
                // two's complement style: 1..63 = -N, 65..127 = +N, 64=0
                int delta = 0;
                if (inputVal > 64) delta = inputVal - 64;
                else if (inputVal < 64) delta = inputVal - 64; // negative
                int base = lastOutputForParam.count(m.paramId) ? int(std::round(lastOutputForParam[m.paramId])) : m.minOut;
                int newVal = juce::jlimit(m.minOut, m.maxOut, base + delta);
                // smoothing on relative: simple blend
                double yPrev = lastOutputForParam.count(m.paramId) ? lastOutputForParam[m.paramId] : newVal;
                double alpha = juce::jlimit(0.0, 1.0, double(m.smoothing));
                double y = yPrev + alpha * (newVal - yPrev);
                lastOutputForParam[m.paramId] = y;
                dispatchValue(m.paramId, int(std::round(y)));
            }
            else
            {
                int mapped = scaleInt(inputVal, m.minIn, m.maxIn, m.minOut, m.maxOut, m.inverted);
                double yPrev = lastOutputForParam.count(m.paramId) ? lastOutputForParam[m.paramId] : mapped;
                double alpha = juce::jlimit(0.0, 1.0, double(m.smoothing));
                double y = yPrev + alpha * (mapped - yPrev);
                lastOutputForParam[m.paramId] = y;
                dispatchValue(m.paramId, int(std::round(y)));
            }
        }

        // NRPN delivery once we have number and data MSB present
        if (nrpnStates[ch].hasNumber() && nrpnStates[ch].dataMsb >= 0)
        {
            int nrpnNumber = nrpnStates[ch].number();
            int nrpnValue14 = nrpnStates[ch].value14();

            for (auto& kv : mappings)
            {
                auto& m = kv.second;
                if (m.type != Type::NRPN) continue;
                if (!(m.channel < 0 || m.channel == ch)) continue;
                if (m.controller != nrpnNumber) continue;

                int mapped = scaleInt(nrpnValue14, m.minIn, m.maxIn, m.minOut, m.maxOut, m.inverted);
                double yPrev = lastOutputForParam.count(m.paramId) ? lastOutputForParam[m.paramId] : mapped;
                double alpha = juce::jlimit(0.0, 1.0, double(m.smoothing));
                double y = yPrev + alpha * (mapped - yPrev);
                lastOutputForParam[m.paramId] = y;
                dispatchValue(m.paramId, int(std::round(y)));
            }
            // Keep state but allow updates as more data arrives
        }
        return;
    }

    if (msg.isPitchWheel())
    {
        int value14 = msg.getPitchWheelValue(); // 0..16383
        for (auto& kv : mappings)
        {
            auto& m = kv.second;
            if (m.type != Type::PITCHBEND) continue;
            if (!(m.channel < 0 || m.channel == ch)) continue;
            int mapped = scaleInt(value14, m.minIn, m.maxIn, m.minOut, m.maxOut, m.inverted);
            double yPrev = lastOutputForParam.count(m.paramId) ? lastOutputForParam[m.paramId] : mapped;
            double alpha = juce::jlimit(0.0, 1.0, double(m.smoothing));
            double y = yPrev + alpha * (mapped - yPrev);
            lastOutputForParam[m.paramId] = y;
            dispatchValue(m.paramId, int(std::round(y)));
        }
        return;
    }

    if (msg.isAftertouch() || msg.isChannelPressure())
    {
        int val = msg.isAftertouch() ? msg.getAfterTouchValue() : msg.getChannelPressureValue();
        for (auto& kv : mappings)
        {
            auto& m = kv.second;
            if (m.type != Type::AFTERTOUCH) continue;
            if (!(m.channel < 0 || m.channel == ch)) continue;
            int mapped = scaleInt(val, m.minIn, m.maxIn, m.minOut, m.maxOut, m.inverted);
            double yPrev = lastOutputForParam.count(m.paramId) ? lastOutputForParam[m.paramId] : mapped;
            double alpha = juce::jlimit(0.0, 1.0, double(m.smoothing));
            double y = yPrev + alpha * (mapped - yPrev);
            lastOutputForParam[m.paramId] = y;
            dispatchValue(m.paramId, int(std::round(y)));
        }
        return;
    }

    if (msg.isNoteOn())
    {
        int note = msg.getNoteNumber();
        int vel = int(std::round(msg.getVelocity() * 127.0f));
        for (auto& kv : mappings)
        {
            auto& m = kv.second;
            if (m.type != Type::NOTE) continue;
            if (!(m.channel < 0 || m.channel == ch)) continue;
            if (m.controller != note) continue;
            int mapped = scaleInt(vel, m.minIn, m.maxIn, m.minOut, m.maxOut, m.inverted);
            double yPrev = lastOutputForParam.count(m.paramId) ? lastOutputForParam[m.paramId] : mapped;
            double alpha = juce::jlimit(0.0, 1.0, double(m.smoothing));
            double y = yPrev + alpha * (mapped - yPrev);
            lastOutputForParam[m.paramId] = y;
            dispatchValue(m.paramId, int(std::round(y)));
        }
        return;
    }
}
