#include "AutomationRecorder.h"

using namespace juce;

static double nowSeconds() { return Time::getMillisecondCounterHiRes() * 0.001; }

AutomationRecorder::AutomationRecorder() {}

void AutomationRecorder::startRecording()
{
    const ScopedLock sl(lock);
    recording = true;
    recordingStartTimeSeconds = nowSeconds();
}

void AutomationRecorder::stopRecording()
{
    const ScopedLock sl(lock);
    recording = false;
}

void AutomationRecorder::recordParameterChange(const String& paramId, int value, double timestamp)
{
    const ScopedLock sl(lock);
    if (!recording)
        return;

    auto* lane = findOrCreateLane(paramId);
    if (!lane)
        return;

    // If any lane is armed, only record to armed lanes
    bool anyArmed = false;
    for (auto& l : lanes) if (l.armed) { anyArmed = true; break; }
    if (anyArmed && !lane->armed)
        return;

    AutomationPoint pt;
    pt.timestamp = timestamp;
    pt.paramId = paramId;
    pt.value = clampMidiValue(value);

    // Keep points ordered by time
    auto it = std::lower_bound(lane->points.begin(), lane->points.end(), pt.timestamp,
        [](const AutomationPoint& a, double t){ return a.timestamp < t; });
    lane->points.insert(it, pt);
}

void AutomationRecorder::startPlayback()
{
    const ScopedLock sl(lock);
    if (playing) return;
    playing = true;
    playbackStartWallClockSeconds = nowSeconds() - playbackPosSeconds;
}

void AutomationRecorder::stopPlayback()
{
    const ScopedLock sl(lock);
    playing = false;
}

void AutomationRecorder::updatePlaybackPosition()
{
    const ScopedLock sl(lock);
    if (!playing)
        return;

    playbackPosSeconds = jmax(0.0, nowSeconds() - playbackStartWallClockSeconds);

    if (loopEnabled && loopEndSeconds > loopStartSeconds && playbackPosSeconds >= loopEndSeconds)
    {
        const double loopLength = loopEndSeconds - loopStartSeconds;
        playbackPosSeconds = loopStartSeconds + std::fmod(playbackPosSeconds - loopStartSeconds, loopLength);
        playbackStartWallClockSeconds = nowSeconds() - playbackPosSeconds;
    }
}

void AutomationRecorder::setPlaybackPosition(double timeSeconds)
{
    const ScopedLock sl(lock);
    playbackPosSeconds = jmax(0.0, timeSeconds);
    if (playing)
        playbackStartWallClockSeconds = nowSeconds() - playbackPosSeconds;
}

void AutomationRecorder::setLoop(double startTime, double endTime)
{
    const ScopedLock sl(lock);
    loopStartSeconds = jmax(0.0, jmin(startTime, endTime));
    loopEndSeconds   = jmax(0.0, jmax(startTime, endTime));
    loopEnabled = (loopEndSeconds > loopStartSeconds);
}

void AutomationRecorder::setTempo(double bpm)
{
    const ScopedLock sl(lock);
    tempoBPM = jlimit(20.0, 300.0, bpm);
}

void AutomationRecorder::setSyncMode(SyncMode mode)
{
    const ScopedLock sl(lock);
    syncMode = mode;
}

std::vector<std::pair<String, int>> AutomationRecorder::getValuesAtTime(double timeSeconds) const
{
    const ScopedLock sl(lock);
    std::vector<std::pair<String, int>> values;

    for (const auto& lane : lanes)
    {
        if (!lane.enabled || lane.points.empty() || lane.paramId.isEmpty())
            continue;

        if (timeSeconds <= lane.points.front().timestamp)
        {
            values.push_back({ lane.paramId, clampMidiValue(lane.points.front().value) });
            continue;
        }

        if (timeSeconds >= lane.points.back().timestamp)
        {
            values.push_back({ lane.paramId, clampMidiValue(lane.points.back().value) });
            continue;
        }

        auto upper = std::lower_bound(lane.points.begin(), lane.points.end(), timeSeconds,
            [](const AutomationPoint& point, double t) { return point.timestamp < t; });

        if (upper == lane.points.begin())
        {
            values.push_back({ lane.paramId, clampMidiValue(upper->value) });
            continue;
        }

        const auto& next = *upper;
        const auto& prev = *(upper - 1);
        const double span = jmax(0.001, next.timestamp - prev.timestamp);
        const double alpha = jlimit(0.0, 1.0, (timeSeconds - prev.timestamp) / span);
        const int interpolated = roundToInt(prev.value + (next.value - prev.value) * alpha);
        values.push_back({ lane.paramId, clampMidiValue(interpolated) });
    }

    return values;
}

AutomationRecorder::AutomationLane* AutomationRecorder::findOrCreateLane(const String& paramId)
{
    for (auto& l : lanes)
        if (l.paramId == paramId)
            return &l;
    lanes.push_back({ paramId, {}, true, false });
    return &lanes.back();
}

int AutomationRecorder::paramIdToController(const String& paramId)
{
    // Simple stable hash to 1..119 (avoid 120-127 for common uses)
    uint32 hash = 5381u;
    for (auto ch : paramId)
        hash = ((hash << 5) + hash) + (uint32) (uint16) ch;
    int cc = (int) (hash % 119u) + 1;
    return cc;
}

bool AutomationRecorder::exportToMidiFile(const File& file)
{
    const ScopedLock sl(lock);
    MidiFile mf;
    const int ticksPerQuarter = 960;
    mf.setTicksPerQuarterNote(ticksPerQuarter);

    // Single track
    MidiMessageSequence seq;

    const double secondsToTicks = (ticksPerQuarter * (tempoBPM / 60.0));

    // Add tempo meta at start
    seq.addEvent(MidiMessage::tempoMetaEvent(60.0 / tempoBPM));

    for (const auto& lane : lanes)
    {
        if (!lane.enabled) continue;
        const int cc = paramIdToController(lane.paramId);
        for (const auto& pt : lane.points)
        {
            const double t = jmax(0.0, pt.timestamp);
            const double ticks = t * secondsToTicks;
            MidiMessage msg = MidiMessage::controllerEvent(1, cc, clampMidiValue(pt.value));
            msg.setTimeStamp(ticks);
            seq.addEvent(msg);
        }
    }

    seq.updateMatchedPairs();
    mf.addTrack(seq);

    FileOutputStream out(file);
    if (!out.openedOk())
        return false;
    mf.writeTo(out);
    return true;
}

bool AutomationRecorder::exportToJSON(const File& file)
{
    const ScopedLock sl(lock);
    DynamicObject::Ptr rootObj = new DynamicObject();
    rootObj->setProperty("tempoBPM", tempoBPM);
    rootObj->setProperty("loopEnabled", loopEnabled);
    rootObj->setProperty("loopStart", loopStartSeconds);
    rootObj->setProperty("loopEnd", loopEndSeconds);

    Array<var> lanesArr;
    for (const auto& lane : lanes)
    {
        DynamicObject::Ptr l = new DynamicObject();
        l->setProperty("paramId", lane.paramId);
        l->setProperty("enabled", lane.enabled);
        l->setProperty("armed", lane.armed);

        Array<var> pts;
        for (const auto& pt : lane.points)
        {
            DynamicObject::Ptr p = new DynamicObject();
            p->setProperty("t", pt.timestamp);
            p->setProperty("v", pt.value);
            pts.add(var(p.get()));
        }
        l->setProperty("points", pts);
        lanesArr.add(var(l.get()));
    }
    rootObj->setProperty("lanes", lanesArr);

    const String jsonText = JSON::toString(var(rootObj.get()), true);
    return file.replaceWithText(jsonText);
}

bool AutomationRecorder::importFromJSON(const File& file)
{
    const ScopedLock sl(lock);
    const String text = file.loadFileAsString();
    var parsed = JSON::parse(text);
    if (parsed.isVoid() || !parsed.isObject())
        return false;

    lanes.clear();

    auto* obj = parsed.getDynamicObject();
    tempoBPM = (double) obj->getProperty("tempoBPM");
    loopEnabled = (bool) obj->getProperty("loopEnabled");
    loopStartSeconds = (double) obj->getProperty("loopStart");
    loopEndSeconds = (double) obj->getProperty("loopEnd");

    auto lanesVar = obj->getProperty("lanes");
    if (lanesVar.isArray())
    {
        auto* arr = lanesVar.getArray();
        for (auto& lv : *arr)
        {
            if (!lv.isObject()) continue;
            auto* lo = lv.getDynamicObject();
            AutomationLane lane;
            lane.paramId = lo->getProperty("paramId").toString();
            lane.enabled = (bool) lo->getProperty("enabled");
            lane.armed = (bool) lo->getProperty("armed");

            auto pointsVar = lo->getProperty("points");
            if (pointsVar.isArray())
            {
                auto* pts = pointsVar.getArray();
                for (auto& pv : *pts)
                {
                    if (!pv.isObject()) continue;
                    auto* po = pv.getDynamicObject();
                    AutomationPoint p;
                    p.timestamp = (double) po->getProperty("t");
                    p.value = clampMidiValue((int) po->getProperty("v"));
                    p.paramId = lane.paramId;
                    lane.points.push_back(p);
                }
            }
            std::sort(lane.points.begin(), lane.points.end(), [](const AutomationPoint& a, const AutomationPoint& b){ return a.timestamp < b.timestamp; });
            lanes.push_back(std::move(lane));
        }
    }
    return true;
}
