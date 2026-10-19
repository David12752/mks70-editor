#pragma once
#include <JuceHeader.h>
#include "ToneParams.h"

//==============================================================================
// Advanced AI Knowledge System for MKS-70 Sound Design
// Provides step-by-step guidance with specific parameter values
//==============================================================================
namespace AdvancedMKS70Knowledge
{
    struct SoundDesignStep
    {
        int stepNumber;
        juce::String instruction;        // What to do
        juce::String explanation;        // Why we're doing it
        juce::String parameterName;      // Which parameter (from ToneParams)
        int suggestedValue;             // Recommended value
        juce::String tabName;           // Which tab: "upper", "lower", "patch", "arp"
    };
    
    struct SoundRecipe
    {
        juce::String soundName;
        juce::String description;
        juce::Array<SoundDesignStep> steps;
    };
    
    //==============================================================================
    // WARM PAD RECIPE
    //==============================================================================
    inline SoundRecipe getWarmPadRecipe()
    {
        SoundRecipe recipe;
        recipe.soundName = "Warm Pad";
        recipe.description = juce::CharPointer_UTF8(
            "\xd7\xa4\xd7\x90\xd7\x93 \xd7\x97\xd7\x9d \xd7\x95\xd7\xa2\xd7\xa9\xd7\x99\xd7\xa8 - \xd7\x9e\xd7\x95\xd7\xa9\xd7\x9c\xd7\x9d \xd7\x9c\xd7\xa8\xd7\xa7\xd7\xa2\xd7\x99\xd7\x9d \xd7\x95\xd7\x90\xd7\x9e\xd7\x91\xd7\x99\xd7\x99\xd7\xa0\xd7\x98");
            // "פאד חם ועשיר - מושלם לרקעים ואמביינט"
        
        recipe.steps.add({1, 
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 DCO-1 \xd7\x9c\xd7\xa6\xd7\x95\xd7\xa8\xd7\xaa Saw"),
            juce::CharPointer_UTF8("\xd7\x92\xd7\x9c \xd7\xa9\xd7\x95\xd7\xa8 \xd7\xa0\xd7\x95\xd7\xaa\xd7\x9f \xd7\xa6\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa2\xd7\xa9\xd7\x99\xd7\xa8 \xd7\x95\xd7\x97\xd7\x9d"),
            // "הגדר DCO-1 לצורת Saw" | "גל שור נותן צליל עשיר וחם"
            "DCO1_WAVEFORM", 1, "upper"});
        
        recipe.steps.add({2,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 DCO-2 \xd7\x9c\xd7\xa6\xd7\x95\xd7\xa8\xd7\xaa Pulse"),
            juce::CharPointer_UTF8("\xd7\x9e\xd7\x95\xd7\xa1\xd7\x99\xd7\xa3 \xd7\xa2\xd7\x95\xd7\x91\xd7\x99 \xd7\x95\xd7\x97\xd7\x9e\xd7\x99\xd7\x9e\xd7\x95\xd7\xaa"),
            // "הגדר DCO-2 לצורת Pulse" | "מוסיף עובי וחמימות"
            "DCO2_WAVEFORM", 2, "upper"});
            
        recipe.steps.add({3,
            juce::CharPointer_UTF8("\xd7\xa9\xd7\xa0\xd7\x94 DCO-2 Fine Tune \xd7\x9c-7"),
            juce::CharPointer_UTF8("\xd7\x93\xd7\x99\xd7\x98\xd7\x95\xd7\x9f \xd7\xa7\xd7\x9c \xd7\x99\xd7\x95\xd7\xa6\xd7\xa8 \xd7\xaa\xd7\xa0\xd7\x95\xd7\xd7\x94 \xd7\xa2\xd7\xa9\xd7\x99\xd7\xa8\xd7\x94"),
            // "שנה DCO-2 Fine Tune ל-7" | "דיטון קל יוצר תנודה עשירה"
            "DCO2_FINE_TUNE", 7, "upper"});
            
        recipe.steps.add({4,
            juce::CharPointer_UTF8("\xd7\x94\xd7\xa2\xd7\x9c\xd7\x94 Mixer DCO1 Level \xd7\x9c-90"),
            juce::CharPointer_UTF8("\xd7\xa2\xd7\x95\xd7\xa6\xd7\x9e\xd7\xaa DCO1 \xd7\xaa\xd7\x94\xd7\x99\xd7\x94 \xd7\x94\xd7\x91\xd7\xa1\xd7\x99\xd7\xa1"),
            // "העלה Mixer DCO1 Level ל-90" | "עוצמת DCO1 תהיה הבסיס"
            "MIXER_DCO1_LEVEL", 90, "upper"});
            
        recipe.steps.add({5,
            juce::CharPointer_UTF8("\xd7\x94\xd7\xa2\xd7\x9c\xd7\x94 Mixer DCO2 Level \xd7\x9c-70"),
            juce::CharPointer_UTF8("\xd7\x9e\xd7\x95\xd7\xa1\xd7\x99\xd7\xa3 \xd7\xa2\xd7\x95\xd7\x91\xd7\x99 \xd7\x9e\xd7\x91\xd7\x9c\xd7\x99 \xd7\x9c\xd7\x94\xd7\x93\xd7\x91\xd7\x99\xd7\xa7"),
            // "העלה Mixer DCO2 Level ל-70" | "מוסיף עובי מבלי להדביק"
            "MIXER_DCO2_LEVEL", 70, "upper"});
            
        recipe.steps.add({6,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 VCF HPF \xd7\x9c-2"),
            juce::CharPointer_UTF8("\xd7\x9e\xd7\xa1\xd7\xa0\xd7\x9f \xd7\xaa\xd7\x97\xd7\xaa\xd7\x95\xd7\xa0\xd7\x99\xd7\x9d \xd7\xa2\xd7\x99\xd7\xa7\xd7\xa9\xd7\x99\xd7\x9d"),
            // "הגדר VCF HPF ל-2" | "מסנן תחתונים עיקשים"
            "VCF_HPF", 2, "upper"});
            
        recipe.steps.add({7,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 VCF Cutoff \xd7\x9c-60"),
            juce::CharPointer_UTF8("\xd7\xa6\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa8\xd7\x9a \xd7\x95\xd7\x97\xd7\x9d, \xd7\x9c\xd7\x90 \xd7\x97\xd7\x93"),
            // "הגדר VCF Cutoff ל-60" | "צליל רך וחם, לא חד"
            "VCF_CUTOFF", 60, "upper"});
            
        recipe.steps.add({8,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 VCF Resonance \xd7\x9c-25"),
            juce::CharPointer_UTF8("\xd7\x9e\xd7\x95\xd7\xa1\xd7\x99\xd7\xa3 \xd7\x90\xd7\x95\xd7\xa4\xd7\x99 \xd7\x95\xd7\xa2\xd7\x95\xd7\x9e\xd7\xa7"),
            // "הגדר VCF Resonance ל-25" | "מוסיף אופי ועומק"
            "VCF_RESONANCE", 25, "upper"});
            
        recipe.steps.add({9,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 ENV1 Attack \xd7\x9c-50"),
            juce::CharPointer_UTF8("\xd7\x94\xd7\xaa\xd7\x97\xd7\x9c\xd7\x94 \xd7\x90\xd7\x99\xd7\x98\xd7\x99\xd7\xaa \xd7\xa9\xd7\x9c \xd7\xa4\xd7\x90\xd7\x93"),
            // "הגדר ENV1 Attack ל-50" | "התחלה איטית של פאד"
            "ENV1_ATTACK", 50, "upper"});
            
        recipe.steps.add({10,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 ENV1 Release \xd7\x9c-60"),
            juce::CharPointer_UTF8("\xd7\x94\xd7\xa2\xd7\x9c\xd7\x9e\xd7\x95\xd7\xaa \xd7\x90\xd7\x99\xd7\x98\xd7\x99\xd7\xaa \xd7\x9c\xd7\x90\xd7\x97\xd7\xa8 \xd7\xa9\xd7\x97\xd7\xa8\xd7\x95\xd7\xa8"),
            // "הגדר ENV1 Release ל-60" | "העלמות איטית לאחר שחרור"
            "ENV1_RELEASE", 60, "upper"});
            
        recipe.steps.add({11,
            juce::CharPointer_UTF8("\xd7\x94\xd7\xa4\xd7\xa2\xd7\x9c Chorus \xd7\x9c\xd7\x9e\xd7\xa6\xd7\x91 II"),
            juce::CharPointer_UTF8("\xd7\x9e\xd7\x95\xd7\xa1\xd7\x99\xd7\xa3 \xd7\xa8\xd7\x95\xd7\x97\xd7\x91 \xd7\xa1\xd7\x98\xd7\xa8\xd7\x99\xd7\x90\xd7\x95 \xd7\x95\xd7\xa2\xd7\x95\xd7\xa9\xd7\xa8"),
            // "הפעל Chorus למצב II" | "מוסיף רוחב סטריאו ועושר"
            "CHORUS", 2, "upper"});
        
        return recipe;
    }
    
    //==============================================================================
    // BASS RECIPE
    //==============================================================================
    inline SoundRecipe getBassRecipe()
    {
        SoundRecipe recipe;
        recipe.soundName = "Deep Bass";
        recipe.description = juce::CharPointer_UTF8(
            "\xd7\x91\xd7\x90\xd7\xa1 \xd7\xa2\xd7\x9e\xd7\x95\xd7\xa7 \xd7\x95\xd7\xa2\xd7\x95\xd7\xa6\xd7\x9e\xd7\xaa\xd7\x99 - \xd7\x9e\xd7\x95\xd7\xa9\xd7\x9c\xd7\x9d \xd7\x9c\xd7\xa7\xd7\xa6\xd7\x91\xd7\x99\xd7\x9d");
            // "באס עמוק ועוצמתי - מושלם לקצבים"
        
        recipe.steps.add({1,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 DCO-1 Range \xd7\x9c-16'"),
            juce::CharPointer_UTF8("\xd7\x90\xd7\x95\xd7\xa7\xd7\x98\xd7\x91\xd7\x94 \xd7\xa0\xd7\x9e\xd7\x95\xd7\x9b\xd7\x94 \xd7\x9c\xd7\x91\xd7\x90\xd7\xa1"),
            // "הגדר DCO-1 Range ל-16'" | "אוקטבה נמוכה לבאס"
            "DCO1_RANGE", 1, "upper"});
            
        recipe.steps.add({2,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 DCO-1 \xd7\x9c\xd7\xa6\xd7\x95\xd7\xa8\xd7\xaa Pulse"),
            juce::CharPointer_UTF8("\xd7\xa0\xd7\x95\xd7\xaa\xd7\x9f \xd7\xa6\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa2\xd7\x91\xd7\x94 \xd7\x95\xd7\xa2\xd7\x9e\xd7\x95\xd7\xa7"),
            // "הגדר DCO-1 לצורת Pulse" | "נותן צליל עבה ועמוק"
            "DCO1_WAVEFORM", 2, "upper"});
            
        recipe.steps.add({3,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 Mixer DCO1 Level \xd7\x9c-100"),
            juce::CharPointer_UTF8("\xd7\x9e\xd7\xa7\xd7\xa1\xd7\x99\xd7\x9e\xd7\x95\xd7\x9d \xd7\xa2\xd7\x95\xd7\xa6\xd7\x9e\xd7\x94"),
            // "הגדר Mixer DCO1 Level ל-100" | "מקסימום עוצמה"
            "MIXER_DCO1_LEVEL", 100, "upper"});
            
        recipe.steps.add({4,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 VCF Cutoff \xd7\x9c-40"),
            juce::CharPointer_UTF8("\xd7\xa1\xd7\x95\xd7\xa0\xd7\x9f \xd7\xaa\xd7\x97\xd7\xaa\xd7\x95\xd7\xa0\xd7\x99\xd7\x9d \xd7\xa2\xd7\x9e\xd7\x95\xd7\xa7\xd7\x99\xd7\x9d"),
            // "הגדר VCF Cutoff ל-40" | "סונן תחתונים עמוקים"
            "VCF_CUTOFF", 40, "upper"});
            
        recipe.steps.add({5,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 VCF Resonance \xd7\x9c-35"),
            juce::CharPointer_UTF8("\xd7\x9e\xd7\x95\xd7\xa1\xd7\x99\xd7\xa3 punch \xd7\x95\xd7\x90\xd7\x92\xd7\xa8\xd7\xa1\xd7\x99\xd7\x91\xd7\x99\xd7\x95\xd7\xaa"),
            // "הגדר VCF Resonance ל-35" | "מוסיף punch ואגרסיביות"
            "VCF_RESONANCE", 35, "upper"});
            
        recipe.steps.add({6,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 ENV1 Attack \xd7\x9c-0"),
            juce::CharPointer_UTF8("\xd7\x94\xd7\xaa\xd7\x97\xd7\x9c\xd7\x94 \xd7\x9e\xd7\x99\xd7\x93\xd7\x99\xd7\xaa"),
            // "הגדר ENV1 Attack ל-0" | "התחלה מיידית"
            "ENV1_ATTACK", 0, "upper"});
            
        recipe.steps.add({7,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 ENV1 Decay \xd7\x9c-40"),
            juce::CharPointer_UTF8("\xd7\x94\xd7\x99\xd7\xa1\xd7\x9c\xd7\xa7\xd7\x95\xd7\xaa \xd7\xa7\xd7\xa6\xd7\xa8\xd7\x94 \xd7\x9c\xd7\x91\xd7\x90\xd7\xa1 \xd7\xa4\xd7\xa0\xd7\xa6'\xd7\x99"),
            // "הגדר ENV1 Decay ל-40" | "היסלקות קצרה לבאס פנצ'י"
            "ENV1_DECAY", 40, "upper"});
            
        recipe.steps.add({8,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 ENV1 Sustain \xd7\x9c-30"),
            juce::CharPointer_UTF8("\xd7\xa8\xd7\x9e\xd7\xaa sustain \xd7\xa0\xd7\x9e\xd7\x95\xd7\x9b\xd7\x94"),
            // "הגדר ENV1 Sustain ל-30" | "רמת sustain נמוכה"
            "ENV1_SUSTAIN", 30, "upper"});
        
        return recipe;
    }
    
    //==============================================================================
    // LEAD RECIPE
    //==============================================================================
    inline SoundRecipe getLeadRecipe()
    {
        SoundRecipe recipe;
        recipe.soundName = "Powerful Lead";
        recipe.description = juce::CharPointer_UTF8(
            "\xd7\x9c\xd7\x99\xd7\x93 \xd7\xa2\xd7\x95\xd7\xa6\xd7\x9e\xd7\xaa\xd7\x99 - \xd7\xa0\xd7\x95\xd7\xa7\xd7\x91 \xd7\x95\xd7\xa2\xd7\xa9\xd7\x99\xd7\xa8");
            // "ליד עוצמתי - נוקב ועשיר"
        
        recipe.steps.add({1,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 DCO-1 \xd7\x9c\xd7\xa6\xd7\x95\xd7\xa8\xd7\xaa Saw"),
            juce::CharPointer_UTF8("\xd7\x92\xd7\x9c \xd7\xa9\xd7\x95\xd7\xa8 = \xd7\xa6\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa2\xd7\xa9\xd7\x99\xd7\xa8"),
            // "הגדר DCO-1 לצורת Saw" | "גל שור = צליל עשיר"
            "DCO1_WAVEFORM", 1, "upper"});
            
        recipe.steps.add({2,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 DCO-2 \xd7\x9c\xd7\xa6\xd7\x95\xd7\xa8\xd7\xaa Saw"),
            juce::CharPointer_UTF8("\xd7\xa9\xd7\xa0\xd7\x99 \xd7\x92\xd7\x9c\xd7\x99 \xd7\xa9\xd7\x95\xd7\xa8 = \xd7\xa2\xd7\x95\xd7\xa6\xd7\x9e\xd7\x94"),
            // "הגדר DCO-2 לצורת Saw" | "שני גלי שור = עוצמה"
            "DCO2_WAVEFORM", 1, "upper"});
            
        recipe.steps.add({3,
            juce::CharPointer_UTF8("\xd7\xa9\xd7\xa0\xd7\x94 DCO-2 Tune \xd7\x9c-+12"),
            juce::CharPointer_UTF8("\xd7\x90\xd7\x95\xd7\xa7\xd7\x98\xd7\x91\xd7\x94 \xd7\x9c\xd7\x9e\xd7\xa2\xd7\x9c\xd7\x94 \xd7\x9c\xd7\xa2\xd7\x95\xd7\x91\xd7\x99"),
            // "שנה DCO-2 Tune ל-+12" | "אוקטבה למעלה לעובי"
            "DCO2_TUNE", 12, "upper"});
            
        recipe.steps.add({4,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 Mixer DCO1 \xd7\x9c-100"),
            juce::CharPointer_UTF8("\xd7\x91\xd7\xa1\xd7\x99\xd7\xa1 \xd7\x97\xd7\x96\xd7\xa7"),
            // "הגדר Mixer DCO1 ל-100" | "בסיס חזק"
            "MIXER_DCO1_LEVEL", 100, "upper"});
            
        recipe.steps.add({5,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 Mixer DCO2 \xd7\x9c-60"),
            juce::CharPointer_UTF8("\xd7\x90\xd7\x95\xd7\xa7\xd7\x98\xd7\x91\xd7\x94 \xd7\xa2\xd7\x9c\xd7\x99\xd7\x95\xd7\xa0\xd7\x94 \xd7\xa2\xd7\x93\xd7\x99\xd7\xa0\xd7\x94"),
            // "הגדר Mixer DCO2 ל-60" | "אוקטבה עליונה עדינה"
            "MIXER_DCO2_LEVEL", 60, "upper"});
            
        recipe.steps.add({6,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 VCF Cutoff \xd7\x9c-75"),
            juce::CharPointer_UTF8("\xd7\xa6\xd7\x9c\xd7\x99\xd7\x9c \xd7\x91\xd7\x94\xd7\x99\xd7\xa8 \xd7\x95\xd7\xa0\xd7\x95\xd7\xa7\xd7\x91"),
            // "הגדר VCF Cutoff ל-75" | "צליל בהיר ונוקב"
            "VCF_CUTOFF", 75, "upper"});
            
        recipe.steps.add({7,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 VCF Resonance \xd7\x9c-40"),
            juce::CharPointer_UTF8("\xd7\x90\xd7\x95\xd7\xa4\xd7\x99 \xd7\x90\xd7\x92\xd7\xa8\xd7\xa1\xd7\x99\xd7\x91\xd7\x99"),
            // "הגדר VCF Resonance ל-40" | "אופי אגרסיבי"
            "VCF_RESONANCE", 40, "upper"});
            
        recipe.steps.add({8,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 VCF ENV Mod \xd7\x9c-50"),
            juce::CharPointer_UTF8("\xd7\xaa\xd7\xa0\xd7\x95\xd7\xa2\xd7\x94 \xd7\x91\xd7\xa4\xd7\x99\xd7\x9c\xd7\x98\xd7\xa8"),
            // "הגדר VCF ENV Mod ל-50" | "תנועה בפילטר"
            "VCF_ENV_MOD", 50, "upper"});
            
        recipe.steps.add({9,
            juce::CharPointer_UTF8("\xd7\x94\xd7\x92\xd7\x93\xd7\xa8 Portamento Time \xd7\x9c-20"),
            juce::CharPointer_UTF8("\xd7\x92\xd7\x9c\xd7\x99\xd7\xa9\xd7\x94 \xd7\xa2\xd7\x93\xd7\x99\xd7\xa0\xd7\x94 \xd7\x91\xd7\x99\xd7\x9f \xd7\xa6\xd7\x9c\xd7\x99\xd7\x9c\xd7\x99\xd7\x9d"),
            // "הגדר Portamento Time ל-20" | "גלישה עדינה בין צלילים"
            "PATCH_PORTAMENTO_TIME", 20, "patch"});
        
        return recipe;
    }
    
    //==============================================================================
    // Get recipe by sound type
    //==============================================================================
    inline SoundRecipe getRecipeForSound(const juce::String& soundType)
    {
        juce::String type = soundType.toLowerCase();
        
        if (type.contains("pad") || type.contains("\xd7\xa4\xd7\x90\xd7\x93"))
            return getWarmPadRecipe();
        else if (type.contains("bass") || type.contains("\xd7\x91\xd7\x90\xd7\xa1"))
            return getBassRecipe();
        else if (type.contains("lead") || type.contains("\xd7\x9c\xd7\x99\xd7\x93"))
            return getLeadRecipe();
        
        // Default to pad
        return getWarmPadRecipe();
    }
}
