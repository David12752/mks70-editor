# MKS-70 Editor - Testing Guide

## שלב 10: בדיקות אינטגרציה

### מטרת הבדיקות
וידוא חיבור תקין בין UI, MidiHandler, ומערכת המיפוי.

---

## ✅ בדיקות שהושלמו

### 1. אינטגרציה בסיסית ✓
- [x] PluginProcessor מחובר ל-MidiHandler
- [x] PluginProcessor מחובר ל-MappingLoader
- [x] כל הטאבים מחוברים ל-Processor
- [x] טעינת קובץ מיפוי JSON בהפעלה
- [x] Callbacks מ-MidiHandler ל-Processor
- [x] Refresh UI מעדכן נתונים מ-Processor

### 2. בנייה והידור ✓
- [x] הפרויקט בנוי בהצלחה
- [x] אין שגיאות קומפילציה
- [x] רק אזהרות קלות (פרמטרים לא בשימוש)

---

## 📋 בדיקות לביצוע ידני

### בדיקה 1: חיבור MIDI בסיסי
**מטרה:** לוודא שה-Editor מזהה התקני MIDI

**צעדים:**
1. פתח את ה-Editor (Standalone או כ-VST3)
2. עבור לטאב "Settings"
3. בדוק ב-ComboBox "MIDI Input" - האם רשומים התקנים?
4. בדוק ב-ComboBox "MIDI Output" - האם רשומים התקנים?
5. בחר Input/Output מתאימים

**תוצאה צפויה:**
- רשימת כל התקני MIDI במערכת
- יכולת לבחור Input ו-Output

---

### בדיקה 2: שליחת פרמטר בודד
**מטרה:** לוודא שמירה של פרמטר בודד ל-MKS-70

**צעדים:**
1. חבר MKS-70 או MIDI Monitor
2. פתח טאב "Tone Upper"
3. שנה פרמטר (למשל: DCO-1 Waveform)
4. עקוב אחרי פלט MIDI

**תוצאה צפויה:**
```
SysEx: F0 41 <DevID> 00 24 12 <Address> <Value> <Checksum> F7
או
NRPN: B0 63 <MSB> B0 62 <LSB> B0 06 <Value>
```

---

### בדיקה 3: קבלת Tone Dump
**מטרה:** לקבל נתוני Tone מה-MKS-70

**צעדים:**
1. הכנס למצב Edit ב-MKS-70
2. ערוך כמה פרמטרים
3. ב-Editor: לחץ כפתור "Request Tone Upper"
4. בדוק ש-UI מתעדכן

**תוצאה צפויה:**
- מסרי SysEx נכנסים (DT1)
- UI מציג ערכים נכונים
- הודעה ב-Debug Log: "Tone received for Upper"

---

### בדיקה 4: קבלת Patch Dump
**מטרה:** לקבל Patch Parameters

**צעדים:**
1. שנה Split Point / Key Mode ב-MKS-70
2. ב-Editor: לחץ "Request Patch"
3. בדוק טאב "Patch"

**תוצאה צפויה:**
- Patch parameters מתעדכנים
- Split Point, Key Mode, Balance נכונים

---

### בדיקה 5: Bulk Dump שלם
**מטרה:** שמירה/טעינה של בנק שלם (64 patches)

**צעדים:**
1. ב-Editor: לחץ "Request Bulk Dump" (טאב Settings או Librarian)
2. בחר Bank (0-3)
3. המתן לקבלת כל 64 ה-Patches
4. שמור לקובץ .syx

**תוצאה צפויה:**
- הודעה: "Bulk dump received - Bank X with 64 patches"
- קובץ נשמר בהצלחה
- אפשר לטעון חזרה

---

### בדיקה 6: שמירה/טעינה של קבצים
**מטרה:** File I/O

**צעדים:**
1. צור Tone (עריכת פרמטרים)
2. לחץ "Save Tone" → בחר שם קובץ
3. אתחל פרמטרים (Init)
4. לחץ "Load Tone" → טען אותו קובץ

**תוצאה צפויה:**
- קובץ .tone נשמר
- טעינה מחזירה ערכים מקוריים

**פורמטים נתמכים:**
- `.tone` - Single tone
- `.patch` - Single patch
- `.syx` - SysEx bulk dump
- `.mks70bank` - פורמט מותאם אישית

---

### בדיקה 7: Copy/Paste Tones
**מטרה:** העתקה בין Upper ו-Lower

**צעדים:**
1. עריכת Tone Upper
2. לחץ "Copy to Lower"
3. עבור לטאב "Tone Lower"

**תוצאה צפויה:**
- כל הפרמטרים הועתקו
- Lower Tone זהה ל-Upper

---

### בדיקה 8: Arpeggiator Functions
**מטרה:** שליחת NRPN ל-Vecoven

**צעדים:**
1. טאב "Arpeggiator"
2. הפעל Arp (On/Off)
3. שנה Tempo, Mode, Octave Span

**תוצאה צפויה:**
```
NRPN messages:
B0 63 02   (MSB = 2 for Arp)
B0 62 00   (LSB = parameter)
B0 06 <Value>
```

---

### בדיקה 9: Two-Way Sync
**מטרה:** סנכרון דו-כיווני UI ↔ Synth

**תצורה:** הפעל "Send on Edit" + "APR Send" (Settings)

**מהלך:**
1. שנה פרמטר ב-UI → עקוב בסינת'
2. שנה פרמטר בסינת' → עקוב ב-UI

**תוצאה צפויה:**
- שינויים נשלחים מיידית ל-MKS-70
- APR messages חוזרים ומעדכנים UI
- אין lag או conflicts

---

### בדיקה 10: טיפול ב-APR (Auto Parameter Response)
**מטרה:** קבלת APR messages מה-MKS-70

**הגדרה ב-MKS-70:**
```
System → APR Send = ON
```

**צעדים:**
1. שנה פרמטר מפאנל הסינת'
2. עקוב אחרי Debug Log ב-Editor

**תוצאה צפויה:**
```
Received APR: F0 41 <DevID> 00 24 13 <Address> <Value> <Checksum> F7
Parameter updated from APR
UI reflects change
```

---

## 🔧 כלי עזר לבדיקות

### 1. MIDI Monitor
**ממליץ:** MIDI-OX (Windows) או MIDI Monitor (Mac)

**שימוש:**
- מציג כל מסר MIDI שנשלח/נתקבל
- עוזר לאמת SysEx messages
- בדיקת Checksum

### 2. Debug Log
**הפעלה:** Settings → Debug Mode = ON

**מיקום:** 
```
Windows: %APPDATA%\MKS70 Editor\debug.log
Mac: ~/Library/Application Support/MKS70 Editor/debug.log
```

### 3. Virtual MIDI
**אם אין MKS-70 פיזי:**
- Windows: loopMIDI
- Mac: IAC Driver (built-in)

**שימוש:**
- צור Virtual MIDI port
- חבר Editor ל-Monitor
- בדוק הודעות

---

## 🐛 תרחישי שגיאה

### שגיאה: "MIDI Device Not Found"
**פתרון:**
1. בדוק חיבול פיזי
2. רענן רשימת התקנים (Settings)
3. בדוק Device ID (0-31)

### שגיאה: "Checksum Error"
**פתרון:**
1. בדוק interference ב-MIDI cable
2. הפחת Sysex Delay (Settings → Advanced)
3. בדוק firmware version

### שגיאה: "No Response from Synth"
**פתרון:**
1. ודא ש-MKS-70 במצב Play (לא Edit)
2. בדוק MIDI Channel
3. אפשר "APR Send" בסינת'

---

## 📊 תוצאות צפויות

### Build Status
```
✅ Compilation: Success
✅ Warnings: Minor (unreferenced parameters)
✅ Errors: None
```

### Files Created
```
✅ mks70_mapping.json - 93 parameters
✅ ParameterInfo.h - Metadata structure
✅ MappingLoader.h/.cpp - JSON loader
✅ MidiHandler.h/.cpp - MIDI communication
✅ All tabs integrated
```

### Functionality
```
✅ UI ↔ Processor connection
✅ Processor ↔ MidiHandler communication
✅ JSON mapping loaded
✅ Callbacks functional
✅ Parameter sync framework ready
```

---

## 🚀 שלבים הבאים (אחרי בדיקות)

1. **Listener Implementation**
   - הוסף onChange listeners לכל פקד UI
   - שלח פרמטרים בזמן אמת

2. **APR Parser**
   - מימוש מלא של parseDT1 (APR handler)
   - עדכון UI אוטומטי

3. **Librarian Integration**
   - חיבור Bulk Dump ל-Librarian Tab
   - שמירת בנקים

4. **File Format**
   - פורמט קובץ סטנדרטי
   - Import/Export

5. **Polish**
   - Tooltips
   - Error messages משופרים
   - Progress bars

---

## ✅ Checklist סופי

בדיקות חובה לפני release:

- [ ] כל התקני MIDI מזוהים
- [ ] פרמטר בודד נשלח בהצלחה
- [ ] Tone Dump מתקבל ומתפרס
- [ ] Patch Dump עובד
- [ ] Bulk Dump 64 patches
- [ ] Save/Load קבצים
- [ ] Copy/Paste בין tones
- [ ] Arpeggiator שולח NRPN
- [ ] Two-way sync עובד
- [ ] APR messages מתקבלים

---

**מצב נוכחי:** כל האינטגרציה מוכנה, מוכן לבדיקות ידניות! 🎉
