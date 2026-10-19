/*
  ==============================================================================
    HebrewTooltips.h - Hebrew tooltips for all MKS-70 parameters
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace HebrewTooltips
{
    // DCO-1 Section
    inline juce::String dco1Range() { return juce::CharPointer_UTF8(
        "טווח תדר של אוסילטור 1\n"
        "16' = אוקטבה נמוכה\n"
        "8' = טווח רגיל\n"
        "4' = אוקטבה גבוהה\n"
        "2' = שתי אוקטבות גבוהות"
    ); }
    
    inline juce::String dco1Waveform() { return juce::CharPointer_UTF8(
        "צורת גל של אוסילטור 1\n"
        "Pulse = דופק - צליל חלול\n"
        "Saw = משור - צליל עשיר\n"
        "Square = מרובע - צליל נקי\n"
        "Triangle = משולש - צליל רך"
    ); }
    
    inline juce::String dco1Tune() { return juce::CharPointer_UTF8(
        "כיוון גובה צליל של אוסילטור 1\n"
        "64 = כיוון מרכזי\n"
        "משפיע על התדר הבסיסי"
    ); }
    
    inline juce::String dco1LFOMod() { return juce::CharPointer_UTF8(
        "עומק מודולציה של LFO על אוסילטור 1\n"
        "יוצר ויברטו או אפקטים מודולריים\n"
        "ערך גבוה = אפקט חזק יותר"
    ); }
    
    inline juce::String dco1ENVMod() { return juce::CharPointer_UTF8(
        "עומק מודולציה של מעטפת על אוסילטור 1\n"
        "משנה את גובה הצליל לאורך זמן\n"
        "שימושי לצלילי בס ולידים"
    ); }

    // DCO-2 Section
    inline juce::String dco2Range() { return juce::CharPointer_UTF8(
        "טווח תדר של אוסילטור 2\n"
        "16' = אוקטבה נמוכה\n"
        "8' = טווח רגיל\n"
        "4' = אוקטבה גבוהה\n"
        "2' = שתי אוקטבות גבוהות"
    ); }
    
    inline juce::String dco2Waveform() { return juce::CharPointer_UTF8(
        "צורת גל של אוסילטור 2\n"
        "Pulse = דופק - צליל חלול\n"
        "Saw = משור - צליל עשיר\n"
        "Square = מרובע - צליל נקי\n"
        "Triangle = משולש - צליל רך"
    ); }
    
    inline juce::String dco2CrossMod() { return juce::CharPointer_UTF8(
        "מודולציה צולבת מ-DCO1 ל-DCO2\n"
        "יוצר צלילים מורכבים ואגרסיביים\n"
        "מצוין לצלילי סינת' קלאסיים\n"
        "ערך גבוה = צליל דיגיטלי יותר"
    ); }
    
    inline juce::String dco2Tune() { return juce::CharPointer_UTF8(
        "כיוון גובה צליל של אוסילטור 2\n"
        "64 = כיוון מרכזי\n"
        "עיוות קל יוצר עובי וחום"
    ); }
    
    inline juce::String dco2FineTune() { return juce::CharPointer_UTF8(
        "כיוון עדין של אוסילטור 2\n"
        "64 = כיוון מדויק\n"
        "עיוות קל יוצר chorus טבעי\n"
        "מצוין לפדים עשירים"
    ); }

    inline juce::String dco2LFOMod() { return juce::CharPointer_UTF8(
        "עומק מודולציה של LFO על אוסילטור 2\n"
        "יוצר ויברטו או אפקטים מודולריים\n"
        "ערך גבוה = אפקט חזק יותר"
    ); }

    inline juce::String dco2ENVMod() { return juce::CharPointer_UTF8(
        "עומק מודולציה של מעטפת על אוסילטור 2\n"
        "משנה את גובה הצליל לאורך זמן\n"
        "שימושי לצלילי בס ולידים"
    ); }

    // DCO Common
    inline juce::String dcoDynamics() { return juce::CharPointer_UTF8(
        "תגובה דינמית של האוסילטורים למהירות נגינה\n"
        "ערך גבוה = שינוי גדול יותר בצליל\n"
        "מוסיף ביטוי למגע"
    ); }

    inline juce::String dcoENVMode() { return juce::CharPointer_UTF8(
        "בחירת מעטפת לשליטה באוסילטורים\n"
        "Normal = מעטפת רגילה\n"
        "Inverted = מעטפת הפוכה\n"
        "קובע את כיוון המודולציה"
    ); }

    // Mixer Section
    inline juce::String mixerDCO1Level() { return juce::CharPointer_UTF8(
        "עוצמת אוסילטור 1 במיקסר\n"
        "127 = עוצמה מלאה\n"
        "0 = השתקה\n"
        "משפיע על איזון הטמבר"
    ); }

    inline juce::String mixerDCO2Level() { return juce::CharPointer_UTF8(
        "עוצמת אוסילטור 2 במיקסר\n"
        "127 = עוצמה מלאה\n"
        "0 = השתקה\n"
        "משפיע על איזון הטמבר"
    ); }

    inline juce::String mixerENVMod() { return juce::CharPointer_UTF8(
        "מודולציה של מעטפת על המיקסר\n"
        "משנה את האיזון בין האוסילטורים לאורך זמן\n"
        "יוצר אפקטים דינמיים מעניינים"
    ); }

    inline juce::String mixerDynamics() { return juce::CharPointer_UTF8(
        "תגובה דינמית של המיקסר למהירות נגינה\n"
        "ערך גבוה = שינוי גדול יותר באיזון\n"
        "מוסיף ביטוי למגע"
    ); }

    inline juce::String mixerENVMode() { return juce::CharPointer_UTF8(
        "בחירת מעטפת לשליטה במיקסר\n"
        "Normal = מעטפת רגילה\n"
        "Inverted = מעטפת הפוכה"
    ); }

    // VCF Section
    inline juce::String vcfHPF() { return juce::CharPointer_UTF8(
        "פילטר מעביר גבוהים (High-Pass Filter)\n"
        "מסיר תדרים נמוכים\n"
        "ערך גבוה = צליל דק יותר\n"
        "מצוין לפדים אוויריים"
    ); }

    inline juce::String vcfCutoff() { return juce::CharPointer_UTF8(
        "תדר חיתוך של הפילטר\n"
        "הפרמטר החשוב ביותר לעיצוב צליל!\n"
        "נמוך = צליל כהה\n"
        "גבוה = צליל בהיר\n"
        "מעצב את האופי הכללי"
    ); }

    inline juce::String vcfResonance() { return juce::CharPointer_UTF8(
        "רזוננס (הדגשה) של הפילטר\n"
        "מדגיש את תדר החיתוך\n"
        "ערך גבוה = צליל מוזר וצווחני\n"
        "קלאסי לצלילי אסיד ובס"
    ); }

    inline juce::String vcfLFOMod() { return juce::CharPointer_UTF8(
        "מודולציה של LFO על הפילטר\n"
        "יוצר אפקט wah-wah\n"
        "מצוין לפדים מתפתחים\n"
        "ערך גבוה = תנועה דרמטית"
    ); }

    inline juce::String vcfENVMod() { return juce::CharPointer_UTF8(
        "מודולציה של מעטפת על הפילטר\n"
        "משנה את הבהירות לאורך זמן\n"
        "64 = נקודת אמצע\n"
        "חיובי = בהיר יותר בהתקפה\n"
        "שלילי = כהה יותר בהתקפה"
    ); }

    inline juce::String vcfKeyFollow() { return juce::CharPointer_UTF8(
        "מעקב אחר מקש - Keyboard Tracking\n"
        "הפילטר עוקב אחר גובה הצליל\n"
        "64 = מעקב מלא\n"
        "0 = ללא מעקב\n"
        "חשוב לטמבר עקבי בכל הרגיסטר"
    ); }

    inline juce::String vcfDynamics() { return juce::CharPointer_UTF8(
        "תגובה דינמית של הפילטר למהירות נגינה\n"
        "נגינה חזקה = פילטר פתוח יותר\n"
        "מוסיף ביטוי ותגובה למגע"
    ); }

    inline juce::String vcfENVMode() { return juce::CharPointer_UTF8(
        "בחירת מעטפת לשליטה בפילטר\n"
        "ENV3 = מעטפת ייעודית לפילטר\n"
        "או שימוש במעטפות אחרות"
    ); }

    // VCA Section
    inline juce::String vcaMode() { return juce::CharPointer_UTF8(
        "מצב פעולה של מגבר העוצמה\n"
        "Gate = צליל קבוע כל עוד המקש לחוץ\n"
        "Env = עוצמה משתנה לפי מעטפת\n"
        "Env נדרש לרוב הצלילים"
    ); }

    inline juce::String vcaLevel() { return juce::CharPointer_UTF8(
        "עוצמת מגבר הצליל\n"
        "127 = עוצמה מלאה\n"
        "0 = שקט\n"
        "שליטה בעוצמה הכללית"
    ); }

    inline juce::String vcaDynamics() { return juce::CharPointer_UTF8(
        "תגובה דינמית של העוצמה למהירות נגינה\n"
        "נגינה חזקה = צליל חזק יותר\n"
        "מוסיף ביטוי ותגובה למגע"
    ); }

    // ENV Sections
    inline juce::String envAttack() { return juce::CharPointer_UTF8(
        "זמן התקפה - Attack Time\n"
        "כמה מהר הצליל מגיע למקסימום\n"
        "0 = מיידי (מתאים לפלאק, בס)\n"
        "גבוה = עלייה איטית (מתאים לפדים)"
    ); }

    inline juce::String envDecay() { return juce::CharPointer_UTF8(
        "זמן דעיכה - Decay Time\n"
        "כמה מהר הצליל יורד לרמת Sustain\n"
        "משפיע על אופי ההתקפה הראשונית"
    ); }

    inline juce::String envSustain() { return juce::CharPointer_UTF8(
        "רמת החזקה - Sustain Level\n"
        "עוצמת הצליל כל עוד המקש לחוץ\n"
        "127 = עוצמה מלאה\n"
        "0 = דומם לאחר ההתקפה"
    ); }

    inline juce::String envRelease() { return juce::CharPointer_UTF8(
        "זמן שחרור - Release Time\n"
        "כמה מהר הצליל נעלם אחרי שחרור המקש\n"
        "0 = מיידי\n"
        "גבוה = נפילה איטית (טבעי יותר)"
    ); }

    inline juce::String envTime1() { return juce::CharPointer_UTF8(
        "זמן שלב 1 (Attack)\n"
        "שלב ראשון במעטפת מורכבת\n"
        "מגיע מ-0 ל-Level1"
    ); }

    inline juce::String envLevel1() { return juce::CharPointer_UTF8(
        "רמת שלב 1\n"
        "גובה שיא ההתקפה הראשונית\n"
        "בדרך כלל 100 למקסימום"
    ); }

    inline juce::String envTime2() { return juce::CharPointer_UTF8(
        "זמן שלב 2 (Decay)\n"
        "שלב שני במעטפת מורכבת\n"
        "מ-Level1 ל-Level2"
    ); }

    inline juce::String envLevel2() { return juce::CharPointer_UTF8(
        "רמת שלב 2\n"
        "רמה אחרי הדעיכה הראשונית\n"
        "משפיע על האופי הכולל"
    ); }

    inline juce::String envTime3() { return juce::CharPointer_UTF8(
        "זמן שלב 3\n"
        "שלב שלישי במעטפת מורכבת\n"
        "מ-Level2 ל-Level3 (Sustain)"
    ); }

    inline juce::String envLevel3() { return juce::CharPointer_UTF8(
        "רמת שלב 3 (Sustain)\n"
        "רמת החזקה כל עוד המקש לחוץ\n"
        "השלב הממושך ביותר"
    ); }

    inline juce::String envTime4() { return juce::CharPointer_UTF8(
        "זמן שלב 4 (Release)\n"
        "זמן שחרור אחרי שחרור המקש\n"
        "הצליל נעלם ברגע זה"
    ); }

    inline juce::String envKeyFollow() { return juce::CharPointer_UTF8(
        "מעקב אחר מקש במעטפת\n"
        "תווים גבוהים = מעטפת מהירה יותר\n"
        "תווים נמוכים = מעטפת איטית יותר\n"
        "מדמה התנהגות אקוסטית"
    ); }

    // LFO Section
    inline juce::String lfoWaveform() { return juce::CharPointer_UTF8(
        "צורת גל של LFO\n"
        "Random = רעש אקראי (S&H)\n"
        "Square = מרובע - קפיצות\n"
        "Saw = משור - עלייה לינארית\n"
        "Sine = סינוס - חלק ועגול"
    ); }

    inline juce::String lfoDelay() { return juce::CharPointer_UTF8(
        "עיכוב הפעלת LFO\n"
        "זמן המתנה לפני תחילת המודולציה\n"
        "מצוין לויברטו מאוחר\n"
        "טבעי לכלי נשיפה וקול"
    ); }

    inline juce::String lfoRate() { return juce::CharPointer_UTF8(
        "מהירות LFO\n"
        "נמוך = מודולציה איטית\n"
        "גבוה = מודולציה מהירה\n"
        "קובע את מהירות האפקט"
    ); }

    // Chorus
    inline juce::String chorusMode() { return juce::CharPointer_UTF8(
        "מצב Chorus\n"
        "Off = ללא אפקט\n"
        "I = chorus עדין\n"
        "II = chorus עמוק\n"
        "מוסיף עומק ועושר לצליל"
    ); }

    // Patch Parameters
    inline juce::String patchKeyMode() { return juce::CharPointer_UTF8(
        "מצב פוליפוני/מונופוני\n"
        "Poly = מספר תווים במקביל\n"
        "Unison = כל התווים על תו אחד\n"
        "Solo = מונופוני עם עדיפות אחרונה"
    ); }

    inline juce::String patchPortamentoMode() { return juce::CharPointer_UTF8(
        "מצב Portamento\n"
        "Off = קפיצות מיידיות בין תווים\n"
        "On = גלישה חלקה בין תווים\n"
        "יפה לצלילי ליד ובס"
    ); }

    inline juce::String patchPortamentoTime() { return juce::CharPointer_UTF8(
        "מהירות Portamento\n"
        "0 = גלישה מהירה\n"
        "127 = גלישה איטית\n"
        "קובע זמן המעבר בין תווים"
    ); }

    inline juce::String patchHold() { return juce::CharPointer_UTF8(
        "החזקת תווים\n"
        "On = התווים נשארים אחרי שחרור\n"
        "Off = התווים נעצרים בשחרור\n"
        "שימושי לשכבות רקע"
    ); }

    inline juce::String patchBenderRange() { return juce::CharPointer_UTF8(
        "טווח גלגל Pitch Bend\n"
        "בטונים (semitones)\n"
        "12 = אוקטבה מלאה\n"
        "2 = טון (נפוץ)"
    ); }

    inline juce::String patchModulationDepth() { return juce::CharPointer_UTF8(
        "עומק גלגל Modulation\n"
        "קובע כמה ה-Mod Wheel משפיע\n"
        "ערך גבוה = אפקט דרמטי"
    ); }

    inline juce::String patchAftertouchAmount() { return juce::CharPointer_UTF8(
        "עומק Aftertouch\n"
        "לחיצה על מקש אחרי נגינה\n"
        "יוצר ביטוי נוסף\n"
        "מצוין לויברטו ומודולציה"
    ); }

    // Arpeggiator (Vecoven)
    inline juce::String arpMode() { return juce::CharPointer_UTF8(
        "מצב Arpeggiator\n"
        "Up = עולה\n"
        "Down = יורד\n"
        "Up-Down = עולה ויורד\n"
        "Random = אקראי\n"
        "מנגן את האקורדים בדפוס"
    ); }

    inline juce::String arpRange() { return juce::CharPointer_UTF8(
        "טווח Arpeggiator\n"
        "1 Oct = אוקטבה אחת\n"
        "2 Oct = שתי אוקטבות\n"
        "3 Oct = שלוש אוקטבות\n"
        "מרחיב את הדפוס למעלה"
    ); }

    inline juce::String arpRate() { return juce::CharPointer_UTF8(
        "מהירות Arpeggiator\n"
        "BPM או חלוקת קצב\n"
        "נמוך = איטי\n"
        "גבוה = מהיר\n"
        "ניתן לסנכרן ל-MIDI clock"
    ); }

    inline juce::String arpGate() { return juce::CharPointer_UTF8(
        "אורך תווים ב-Arpeggiator\n"
        "קצר = staccato\n"
        "ארוך = legato\n"
        "משפיע על תחושת הקצב"
    ); }

    inline juce::String arpSwing() { return juce::CharPointer_UTF8(
        "Swing - קצב לא ישר\n"
        "50% = ישר\n"
        "גבוה = קצב מתנדנד\n"
        "מוסיף תחושת groove"
    ); }
}
