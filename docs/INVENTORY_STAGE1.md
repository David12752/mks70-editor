# Stage 1 — UI Inventory & Integrity Audit

Date: 2025-11-26
Scope: Source/ UI components — Upper/Lower Tone tabs, Patch, Arpeggiator, Librarian, Automation, Settings, Overlays (Compare, Morph, Random), visual components (Waveform/Envelope/Mixer).

## Tabs & Panels
- Tone Upper (Tone A)
- Tone Lower (Tone B)
- Patch
- Arpeggiator
- Librarian
- Automation
- Settings
- Overlays: ComparePanel, MorphPanel, RandomPanel

## Tone Tabs — Controls vs Params

- DCO-1
  - Range: Combo (1..4 → 16',8',4',2') → `ToneParams::dco1Range (0..3)` — bound
  - Waveform: Combo (Pulse,Saw,Square,Triangle) → `dco1Waveform (0..3)` — bound
  - Tune: Slider 0..127 → `dco1Tune` — bound
  - LFO Depth: Slider 0..127 → `dco1LFOMod` — bound
  - ENV Depth: Slider 0..127 → `dco1ENVMod` — bound
  - WaveformDisplay: title/grid/selection (updates combos) — not a param

- DCO-2
  - Range: Combo (1..4) → `dco2Range (0..3)` — bound
  - Waveform: Combo → `dco2Waveform (0..3)` — bound
  - Cross Mod: Combo (Off,Sync,Metal,Both) → `dco2CrossMod (0..3)` — bound
  - Tune: Slider 0..127 → `dco2Tune` — bound
  - Fine Tune: Slider 0..127 → `dco2FineTune` — bound
  - LFO Depth: Slider 0..127 → `dco2LFOMod` — bound
  - ENV Depth: Slider 0..127 → `dco2ENVMod` — bound
  - WaveformDisplay: params mirror — not a param

- Mixer
  - DCO1 Level: Slider 0..127 → `mixerDCO1Level` — bound
  - DCO2 Level: Slider 0..127 → `mixerDCO2Level` — bound
  - ENV Depth: Slider 0..127 → `mixerENVMod` — bound
  - MixerWaveformDisplay — visual only
  - Missing (per ToneParams): `mixerDynamics (0..3)`, `mixerENVMode (0..3)`

- HPF
  - HPF Cutoff: Combo 0..3 → `vcfHPF` — bound

- VCF
  - Cutoff: Slider 0..127 → `vcfCutoff` — bound
  - Resonance: Slider 0..127 → `vcfResonance` — bound
  - LFO Depth: Slider 0..127 → `vcfLFOMod` — bound
  - ENV Depth: Slider 0..127 → `vcfENVMod` — bound
  - Key Follow: Slider 0..127 → `vcfKeyFollow` — bound
  - Missing (per ToneParams): `vcfDynamics (0..3)`, `vcfENVMode (0..3)`

- VCA
  - Level: Slider 0..127 → `vcaLevel` — bound
  - Missing (per ToneParams): `vcaMode (Gate/Env 0..1)`, `vcaDynamics (0..3)`

- Envelopes (ENV-1 / ENV-2)
  - Attack (Time1): Slider 0..127 → `envXAttack` — bound
  - Decay (Time2): Slider 0..127 → `envXDecay` — bound
  - Sustain (Level1): Slider 0..127 → `envXSustain` — bound
  - Release (Time4): Slider 0..127 → `envXRelease` — bound
  - InteractiveEnvelopeEditor mirrors values — OK
  - Missing (per ToneParams): `env1KeyFollow (0..3)`, `env2KeyFollow (0..3)`

- LFO
  - Waveform: Combo (Random,Square,Saw,Sine) → maps to `lfoWaveform` — MISMATCH with `ToneParams` (expects 0..2 Triangle,Saw,Square). Needs reconciliation.
  - Delay: Slider 0..127 → `lfoDelay` — bound
  - Rate: Slider 0..127 → `lfoRate` — bound

- Chorus
  - Mode: Combo (Off,I,II) → `chorusMode (0..2)` — bound

- Buttons
  - Init Tone / Randomize / Copy (Upper↔Lower) — UI only, functional callbacks exist where applicable.

## Patch Tab — Controls vs Params
- Performance
  - Tone Select: Combo → `PatchParameters::toneSel (0..2)` — UI only (no processor binding yet)
  - Key Mode: Combo → `keyMode (0..2)` — UI only
  - Split Point: Slider 0..127 + note label — UI only
  - Balance: Slider 0..127 — UI only

- Portamento
  - Switch: Toggle → `portamentoSwitch (0/1)` — UI only
  - Time: Slider 0..127 — UI only

- Pitch
  - Bend Range: Slider 0..12 — UI only
  - Assign Mode: Combo 0..5 — UI only
  - Glide: Toggle 0/1 — UI only
  - Unison Detune: Slider 0..127 — UI only

- Chase Play
  - Switch/Level/Time: UI only

- Names
  - Patch/Tone name editors (max 10 chars) — UI only

Binding gap: PatchTab lacks onChange wiring to update `PluginProcessor::patchParameters` and send to synth.

## Arpeggiator Tab — Controls vs Params
- Main: Arp On/Off Toggle — UI only
- Range & Tempo: BPM slider 20..300 (display only)
- Mode & Pattern: Mode/Octave Span/Insert — UI only
- Timing: Play Rate, Duration (1..15), Legato toggle — UI only
- Velocity: Mode (Fixed/Played), Fixed Velocity (1..127) — UI only
- MIDI & Sync: Output (Off/Internal/All+Out), Sync (Internal/Ext) — UI only

Binding gap: ArpTab lacks onChange wiring to update `PluginProcessor::arpParameters`.

## Overlays & Panels
- ComparePanel: interacts with `CompareManager`, toggled via editor toolbar or PatchTab button.
- MorphPanel: UI present; details omitted in this audit.
- RandomPanel: generates/apply patches — applies to Processor (Upper/Lower/Patch/Arp) and invokes `onPatchApplied()` to refresh UI; History and Favorites maintained locally.
- AutomationPanel: independent recorder and lanes; not bound to synth parameters.

## Theme & Visual
- Theme tokens present in `Theme.h` and applied in `PluginEditor.cpp`.
- ToneUpper/ToneLower: section borders use `Theme::border()`. Title font uses `Theme::heading1()`.
- Tab backgrounds use `Theme::bgPanel()`.
- Toolbar buttons mapped to tokens.
- Many labels/sliders still using default fonts; spacing hard-coded.

## Mismatches & Missing UI (Gaps)
- ToneParams exposed but not surfaced in UI:
  - `dcoDynamics`, `dcoENVMode`, `mixerDynamics`, `mixerENVMode`, `vcfDynamics`, `vcfENVMode`, `vcaMode`, `vcaDynamics`, `env1KeyFollow`, `env2KeyFollow`.
- LFO Waveform mapping mismatch:
  - UI offers 4 options (Random, Square, Saw, Sine) vs ToneParams (Triangle,Saw,Square, 0..2). Requires harmonization and migration logic.
- PatchTab and ArpTab lack processor bindings for write-back (UI → model) and no send-to-synth on change.
- Tooltips: minimal; many controls lack consistent tooltip/ellipsis behavior.

## Clipping/Overlap Risks (by layout)
- Minimum window set to 1000x840, default 1200x850. At smaller sizes prior to Stage 4, DCO and Envelope sections could clip. Current bounds appear safe at min size; further responsive spacing (Theme::Space) recommended.
- ArpTab and PatchTab use tight rows; labels can collide in localized languages or DPI > 150%. Ellipsis + tooltip recommended.

## Screenshots — How to Capture (Small/Medium/Large)
A helper script is provided: `tools/capture_screens.ps1`.
It will launch the Standalone app, resize the window to three sizes, and store PNGs under `artifacts/screens/`.

Sizes:
- Small: 1024x840
- Medium: 1280x900
- Large: 1600x1000

Run:
```powershell
cd "c:\Users\User\Desktop\synths editors\MKS70 Editor"
powershell -ExecutionPolicy Bypass -File .\tools\capture_screens.ps1
```

Note: Automatic clipping detection is not implemented. Review screenshots and annotate issues in `docs/CLIPPING_LOG.md` (template included when you first run the script).

## Build Verification
- Debug x64: Standalone and VST3 — succeeded.
- Release x64: solution-level build — succeeded (per terminal log).
- Known toolchain warning for VST3 about 32-bit helper; non-blocking.

## Summary of Gaps (Action Items)
1. Add missing ToneParams UI (ENV key follow, dynamics/env modes, VCA mode/dynamics, mixer/VCF dynamics/env modes).
2. Harmonize LFO waveform enum between UI and `ToneParams` (+ migration path for existing states).
3. Bind PatchTab and ArpTab controls to Processor (onChange → set params → send to synth) + refresh hooks.
4. Add Theme typography/spacing tokens to all labels/controls; add tooltips and ellipsis for long texts.
5. Add responsive spacing using `Theme::Space` to reduce collision risks at HiDPI.
