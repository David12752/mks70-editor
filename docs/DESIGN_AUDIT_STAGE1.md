# Stage 1: UI Inventory & Integrity Audit

Date: 2025-11-26
Target: Full project (all tabs), JUCE UI integrity, no missing data, consistent relative sizes, no clipping/hidden info.

Summary
- Build: Debug x64 builds successfully (Standalone + VST3). Verified with MSBuild.
- Tabs detected: Tone Upper, Tone Lower, Patch, Arpeggiator, Librarian, Automation, Settings. Overlays: Compare, Morph, Random. Macro Strip + Toolbar.
- Envelopes: Both tabs include sliders and interactive editors for ENV-1/ENV-2 (ADSR). No missing controls in code. Layout enlarged to keep envelopes visible.
- Waveforms: DCO-1/DCO-2 waveform displays + Mixer waveform display present on both tabs; live updates wired.
- Risk points: Fixed minor build issues (Waveform tooltips/mouse-move). Potential for clipping on very small windows remains due to fixed per-row heights; DPI scaling to validate in Stage 6.

Tabs & Sections
- Tone Upper (`ToneUpperTab.[h|cpp]`)
  - DCO-1: Range, Waveform, Tune, LFO Depth, ENV Depth, WaveformDisplay
  - DCO-2: Range, Waveform, CrossMod, Tune, Fine, LFO Depth, ENV Depth, WaveformDisplay
  - Mixer: DCO1 Level, DCO2 Level, ENV Depth, MixerWaveformDisplay
  - HPF: Cutoff combo
  - VCF: Cutoff, Resonance, ENV Depth, LFO Depth, Key Follow, Dynamics combo
  - VCA: Level, Dynamics combo
  - ENV-1 (A/D/S/R sliders) + InteractiveEnvelopeEditor
  - ENV-2 (A/D/S/R sliders) + InteractiveEnvelopeEditor
  - LFO: Waveform, Delay, Rate
  - Chorus: Mode combo
  - Buttons: Init, Randomize, Copy to Lower
  - MIDI Learn: Right-click sliders (mapping indicators drawn in paintOverChildren)

- Tone Lower (`ToneLowerTab.[h|cpp]`)
  - Mirrors Tone Upper sections, including waveforms + envelopes
  - Buttons: Init, Randomize, Copy to Upper

- Patch (`PatchTab.[h|cpp]`) — present and wired to processor refresh
- Arpeggiator (`ArpTab.[h|cpp]`) — present and wired to processor refresh
- Librarian (`LibrarianTab.[h|cpp]`) — present
- Automation (`AutomationPanel.[h|cpp]`) — present
- Settings (`SettingsTab.[h|cpp]`) — present
- Overlays: Compare (`ComparePanel`), Morph (`MorphPanel`), Random (`RandomPanel`), Macro system

Key Controls Presence Check
- Envelopes:
  - ENV-1: Attack, Decay, Sustain, Release — found in both tabs + editors
  - ENV-2: Attack, Decay, Sustain, Release — found in both tabs + editors
- DCOs: Waveform, Range, Tune/Fine, CrossMod, LFO/ENV depths — present
- VCF: Cutoff, Resonance, LFO Depth, ENV Depth, Key Follow — present
- VCA: Level (+ Dynamics selector) — present
- Mixer: DCO levels + ENV depth — present
- LFO: Waveform, Delay, Rate — present
- Chorus: Mode — present

Potential Issues / Findings
- Clipping risk at narrow heights: Rows have fixed pixel heights; under minimal window sizes, bottom sections can compress. Resizing allowed; min size set to 800x500, but extreme DPI scaling may still cause elide.
- Tooltips for waveform displays: Removed for compatibility (JUCE Component tooltip availability differs by version). Context menus for right-click remain.
- Performance: Waveform drawing per repaint is lightweight; no timers running continuously. Envelope visualizers animate on demand.

Data Integrity
- All UI controls map to `ToneParams` fields and call `onParameterChanged()` which sends updated tone to synth via `processor->sendToneToSynth(isUpper)`.
- Randomize applies and UI refresh is wired in `PluginEditor::refreshUI()` via `RandomPanel` callback.

Next Steps
- Stage 2: Wireframe + design system proposal (tokens, grid, dark theme baseline).
- Stage 3+: Theming infrastructure, typography, hover/active states, minimal animations.
- Stage 6: DPI, accessibility, tooltips re-introduction via a centralized `TooltipWindow` if desired.
