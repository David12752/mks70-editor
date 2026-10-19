COMMON FIXES — תיקונים נפוצים
===============================

1. טקסט נחתך (Clipped text)
- הגדל את ה-bounds של הרכיב
- השתמש ב-`drawFittedText` במקום `drawText`
- בדוק שה-`Font` לא גדול מדי עבור ה-bounds

2. סליידרים בגדלים שונים (Inconsistent slider sizes)
- ודא שכולם משתמשים ב-`DesignSystem::Size`
- בדוק שה-parent bounds מספיק גדול לכל הסליידרים

3. אלמנטים לא מיושרים (Misaligned elements)
- השתמש ב-`FlexBox` או `Grid`
- חשב `startX` ל-centering כשצריך

4. צבעים לא עקביים (Inconsistent colors)
- החלף hardcoded colors ב-`DesignSystem::Color`
- ודא שה-`LookAndFeel` המותאם מופעל

5. מרווחים לא אחידים (Inconsistent spacing)
- השתמש רק בערכים מ-`DesignSystem::Spacing`
- בדוק שימוש ב-`bounds.reduce()` ו-`removeFromX()`/`removeFromY()`

6. Scroll לא עובד (Scrolling issues)
- ודא שה-content גבוה מה-viewport
- בדוק `setScrollBarsShown()`
- ודא שה-`contentContainer` בגודל נכון (width=viewport width, height=content height)

7. חלון קטן מדי (Window too small)
- הגדר `setResizeLimits()` ב-window/editor הראשי
- או הוסף Scroll סביב התוכן
