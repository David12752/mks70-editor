# Project UI Redesign Checklist

Scope: All tabs + overlays. Ensure no missing data, no clipping, modern minimal aesthetic with full functionality.

Requirements Coverage (Stage 1 status)
- Envelopes visible and complete (A/D/S/R) — OK in Upper/Lower
- All DCO/VCF/VCA/LFO/Mixer/Chorus controls present — OK
- Waveform displays integrated in DCO1/2 + Mixer — OK
- Randomize updates UI and displays — OK
- Upper/Lower parity across controls and visuals — OK
- Build after changes — OK (Debug x64 Standalone + VST3)

Design/UX Goals
- Modern, minimal, Arturia-like dark theme
- Subtle animations (hover/focus/fade), high contrast labels
- Responsive layout (Grid/Flex), no hidden/overlapping controls
- Tooltips/Help for discoverability (centralized)

Open Actions for Upcoming Stages
- Stage 2: Wireframe layout and design tokens (colors, fonts, spacing)
- Stage 3: Theme class + global styles; replace hard-coded colours
- Stage 4: Validate all envelope sliders + visual editors in final layout
- Stage 5: Add animations and polish interactions uniformly
- Stage 6: DPI scaling, keyboard navigation, accessibility
- Stage 7: Harmonize visuals for waveform/envelope components with theme
- Stage 8: Regression tests: sizing, clipping, build/run checks
- Stage 9: Preferences for toggles (grid/border/labels/animations/theme)
