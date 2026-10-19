# Stage 2: Wireframe & Design System Proposal

Objectives
- Establish a modern, minimal dark theme (Arturia-inspired) with full functionality.
- Define design tokens (colors, typography, spacing, radii) and layout guidelines.
- Propose responsive grid for all tabs without hiding controls.

Design Tokens (initial)
- Colors:
  - bg/base: #121417
  - bg/panel: #1A1D21
  - border: #2A2F36
  - text/primary: #E6EAF0
  - text/secondary: #A6B0BF
  - accent/primary: #4DD0E1 (cyan) or #FF8A65 (warm) — TBD per section
  - accent/ok: #78C257
  - accent/warn: #FFB74D
  - accent/error: #EF5350
  - grid lines: #3A4048 @ 35–60% alpha
- Typography:
  - font family: default JUCE sans (platform), sizes:
  - h1: 18–20 bold, h2: 14 bold, label: 12–13 med, value: 11–12
- Spacing scale:
  - xs=4, s=8, m=12, l=16, xl=20, 2xl=24 px
- Radii:
  - small: 4px (panels, buttons)
- Effects:
  - focus ring: 1–2px accent/primary; hover glow subtle alpha
  - animation durations: 150–220ms ease-in-out

Responsive Grid (per tab)
- Overall: vertical stack of sections; each section uses a 3-column grid where applicable.
- Row 1 — DCOs + Mixer (3 columns):
  - Col 1: DCO-1 (title, waveform 72–96px, controls)
  - Col 2: DCO-2 (title, waveform 72–96px, controls, x-mod)
  - Col 3: Mixer (title, combined waveform 72–96px, levels + ENV depth)
- Row 2 — Filters (HPF + VCF):
  - HPF in narrow left column; VCF in remaining width; wrap to two lines if needed.
- Row 3 — Amp & Envelopes:
  - Col 1: VCA; Col 2: ENV-1 (A/D/S/R sliders + editor); Col 3: ENV-2 (A/D/S/R + editor)
  - Editors share remaining height to avoid clipping; sliders at top of each env column.
- Row 4 — LFO:
  - Waveform, Delay, Rate laid horizontally; wrap under small widths.
- Row 5 — Chorus & Actions:
  - Chorus selector + Init/Randomize/Copy buttons aligned right.

Behavior & States
- Sliders/combos:
  - hover: slight accent tint; active: 1px ring.
  - values always visible; long labels ellipsize with tooltip.
- Waveform/Envelope displays:
  - titles top-left; waveform label top-right; zero line & grid toggles; crosshair on hover.
- Panels & borders: 1px border, subtle panel background.

Accessibility & Sizing
- Maintain min size ~800x500; scale font + spacing with DPI.
- Keyboard focus order left-to-right, top-to-bottom in each section.

Approval Notes
- Colors can be tuned in Stage 3 once theme class exists; this wireframe ensures no hidden controls and maintains balance across sections.
