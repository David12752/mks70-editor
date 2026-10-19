# MKS-70 Editor/Librarian

> **Editor מקצועי עבור Roland MKS-70 Super JX Synthesizer**  
> תומך בשדרוג Vecoven V4.x + Flash Drive

---

## 🎹 תכונות

### עריכת פרמטרים
- ✅ **Tone Upper/Lower** - עריכת 93 פרמטרי צליל
  - DCO1/2 (Oscillators)
  - VCF (Filter)
  - VCA (Amplifier)
  - ENV1/2/3/4 (Envelopes)
  - LFO1/2 (Modulation)
  - Mixer, HPF, Chorus

- ✅ **Patch Parameters** - הגדרות גלובליות
  - Key Mode (Poly1/2, Unison, Solo)
  - Split Point
  - Portamento
  - Pitch Bend Range
  - Chase Play

- ✅ **Arpeggiator** (Vecoven 3.x+)
  - 5 Modes (Up, Down, Up-Down, Played, Random)
  - Octave Span (1-4)
  - Tempo (30-300 BPM)
  - Latch, Legato, Velocity modes

### ניהול קבצים
- ✅ **Librarian** - ארגון עד 256 patches (4 banks × 64)
- ✅ **Import/Export** - SysEx, .tone, .patch, .mks70bank
- ✅ **Bulk Dump** - גיבוי מלא של הסינת'
- ✅ **Copy/Paste** - העתקה בין Upper/Lower

### תקשורת MIDI
- ✅ **Roland SysEx** - פרוטוקול רשמי
- ✅ **NRPN** - תמיכה בשדרוג Vecoven
- ✅ **Two-Way Sync** - UI ↔ Synth בזמן אמת
- ✅ **APR Support** - Auto Parameter Response
- ✅ **Device ID** - תמיכה בריבוי יחידות (0-31)

### מערכת מיפוי
- ✅ **JSON-Based** - הגדרת פרמטרים חיצונית
- ✅ **Firmware Versions** - Vecoven 3.x, 4.x, Stock
- ✅ **Hot Reload** - החלפת מיפוי ללא קומפילציה

---

## 📦 התקנה

### דרישות מערכת
- **Windows:** 10/11 (64-bit)
- **JUCE:** 8.0.10+
- **Visual Studio:** 2022
- **MIDI Interface:** כרטיס MIDI או USB-MIDI

### הרצת Standalone
```bash
cd "Builds\VisualStudio2022\x64\Debug\Standalone Plugin"
"MKS70 Editor.exe"
```

### טעינת VST3
```
קובץ: Builds\VisualStudio2022\x64\Debug\VST3\MKS70 Editor.vst3
העתק ל: C:\Program Files\Common Files\VST3\
```

---

## 🔧 הגדרות ראשוניות

### 1. חיבור MIDI
```
Settings Tab:
┌─────────────────────────┐
│ MIDI Input:  [MKS-70]   │
│ MIDI Output: [MKS-70]   │
│ SysEx Device ID: 17     │ ← Default
│ Comm Mode: [SysEx+NRPN] │
└─────────────────────────┘
```

### 2. טעינת מיפוי
```
Settings → Mapping File:
📄 mks70_mapping.json
[Load Mapping]
```

### 3. הפעלת APR
```
MKS-70 Menu:
System → APR Send = ON
```

---

## 🎛️ מדריך שימוש

### עריכת Tone
1. טאב **Tone Upper**
2. שנה פרמטרים (Sliders/ComboBoxes)
3. שמור: [Save Tone] → `mytone.tone`

### ניהול בנקים
1. טאב **Librarian**
2. [Request Bulk Dump] → Bank 0-3
3. Drag & Drop patches
4. [Send Bank] → העלאה ל-MKS-70

### העתקה בין Tones
```
Tone Upper Tab:
[Copy to Lower] ──────┐
                       ↓
Tone Lower Tab מתעדכן
```

---

## 🗂️ מבנה פרויקט

```
MKS70 Editor/
├── Source/
│   ├── PluginProcessor.cpp/h     # Core processor
│   ├── PluginEditor.cpp/h        # Main UI
│   ├── MidiHandler.cpp/h         # MIDI comm
│   ├── MappingLoader.cpp/h       # JSON loader
│   ├── ParameterInfo.h           # Parameter metadata
│   ├── ToneParameters.h          # Tone data structure
│   ├── PatchParameters.h         # Patch data
│   ├── ArpParameters.h           # Arpeggiator data
│   ├── ToneUpperTab.cpp/h        # UI - Tone Upper
│   ├── ToneLowerTab.cpp/h        # UI - Tone Lower
│   ├── PatchTab.cpp/h            # UI - Patch
│   ├── ArpTab.cpp/h              # UI - Arpeggiator
│   ├── LibrarianTab.cpp/h        # UI - Librarian
│   └── SettingsTab.cpp/h         # UI - Settings
├── mks70_mapping.json            # Parameter mapping
├── TESTING.md                    # Testing guide
└── README.md                     # This file
```

---

## 🧪 בדיקות

ראה **[TESTING.md](TESTING.md)** למדריך בדיקות מפורט.

**בדיקות מהירות:**
```
1. Settings → Test MIDI Connection
2. Tone Upper → שנה DCO-1 Waveform
3. Settings → Request Tone Dump
4. Tone Upper → [Copy to Lower]
5. Librarian → Request Bulk Dump
```

---

## 📚 פורמטים נתמכים

### SysEx (Roland)
```
F0 41 <DevID> 00 24 <Cmd> <Address> <Data> <Checksum> F7

Commands:
0x11 - RQ1 (Request)
0x12 - DT1 (Data Set)
0x13 - APR (Auto Parameter Response)
```

### NRPN (Vecoven)
```
B0 63 <MSB>    # NRPN MSB
B0 62 <LSB>    # NRPN LSB
B0 06 <Value>  # Data Entry MSB
```

### JSON Mapping
```json
{
  "device": "Roland MKS-70",
  "firmware": "Vecoven 4.x",
  "toneParameters": [
    {
      "name": "DCO1_Range",
      "displayName": "DCO-1 Range",
      "category": "DCO1",
      "sysexOffset": 11,
      "nrpnMSB": 0,
      "nrpnLSB": 0,
      "min": 0,
      "max": 3,
      "values": ["16'", "8'", "4'", "2'"],
      "default": 1
    }
  ]
}
```

---

## ⚙️ הגדרות מתקדמות

### Timing
```
Settings → Advanced:
┌────────────────────────────┐
│ SysEx Delay: [10ms]        │ ← בין הודעות
│ Bulk Dump Delay: [50ms]    │ ← בין patches
└────────────────────────────┘
```

### Debug
```
Settings → Advanced:
┌────────────────────────────┐
│ ☑ Debug Mode               │
│ ☑ Log to File              │
│ Location: %APPDATA%\...    │
└────────────────────────────┘
```

---

## 🛠️ פיתוח

### Build
```powershell
cd "Builds\VisualStudio2022"
MSBuild "MKS70 Editor.sln" /p:Configuration=Debug /p:Platform=x64
```

### Dependencies
- **JUCE Framework** 8.0.10
- **juce_audio_processors** - Plugin hosting
- **juce_audio_devices** - MIDI I/O
- **juce_gui_basics** - UI components

### הוספת פרמטר חדש
1. ערוך `mks70_mapping.json`
2. הוסף entry עם offset/NRPN
3. Reload mapping בסטינגס
4. אין צורך בקומפילציה!

---

## 🐛 Troubleshooting

### "No MIDI Devices Found"
```
✓ בדוק חיבול USB/MIDI
✓ התקן drivers
✓ רענן רשימה (Settings)
```

### "Checksum Error"
```
✓ בדוק כבל MIDI איכותי
✓ הפחת SysEx Delay
✓ בדוק Device ID
```

### "Parameters Not Updating"
```
✓ APR Send = ON ב-MKS-70
✓ Send on Edit = ON ב-Editor
✓ בדוק MIDI Channel
```

---

## 📜 רישיון

**GNU AGPL v3** (ראו `LICENSE`), כי העורך בנוי על JUCE 8, שהרישיון הפתוח שלו הוא AGPLv3.

Copyright (C) 2025-2026 David12752

---

## 🙏 תודות

- **Roland** - MKS-70 Super JX
- **Vecoven** - V4.x firmware upgrade
- **JUCE** - Cross-platform framework
- **Community** - Testing & feedback

---

## 📞 קישורים

- **Vecoven Firmware:** [vecoven.com](https://vecoven.com)
- **JUCE:** [juce.com](https://juce.com)
- **MKS-70 Manual:** [Roland official docs]

---

**Built with ❤️ using JUCE**  
Version 1.0.0 | November 2025
