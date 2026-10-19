# Device Settings Tab Specification (Vecoven 3.x)

This document captures the complete prompt + implementation guidance for building the **Device Settings** tab inside the MKS-70 Editor/Librarian (Vecoven firmware 3.x and newer). It consolidates the SysEx/NRPN protocol notes, UI blueprint, and UX requirements so the tab can be generated automatically or implemented directly.

> **Tech Target**: React + TypeScript (Electron/Next.js friendly). The supplied implementation notes assume Web MIDI access, responsive layout helpers, and a dark/light theme hook (`useTheme()`).

---

## 1. Roland MKS-70 SysEx Primer

| Message | Bytes | Notes |
| --- | --- | --- |
| Individual Parameter (IPR) | `F0 41 36 [CH] 24 [LEVEL] [GROUP] [PARAM] [VALUE] F7` | `LEVEL` = `0x30` patch / `0x20` tone, `GROUP` = `0x01` patch or tone A/B. |
| All Parameters (APR)       | `F0 41 35 [CH] 24 [LEVEL] [GROUP] [DATA...] F7` | `DATA` contains sequential parameter payload. |
| Program Change (PGR)       | `F0 41 34 [CH] 24 [LEVEL] [GROUP] [PROG] 00 F7` | `PROG` 0–63 patch, 0–99 tone. |

Vecoven bulk ops: WST (Want To Send) requests, APR dumps, and NRPN CC control for tone layers.

### NRPN (Vecoven 3.x)

```
CC 99 (NRPN MSB)  : Target selection (0=Upper, 1=Lower, 2=Both)
CC 98 (NRPN LSB)  : Parameter CC index (see tone table)
CC 6  (Data MSB)  : Value (0–127)
CC 38 (Data LSB)  : Fine resolution (optional)
```

---

## 2. Parameter Tables

### 2.1 MIDI Menu Parameters

(Use `paramNumber` for SysEx IPR calls. Values enumerated exactly as defined; `jx10Only` items should be display-only hints.)

```ts
interface MidiMenuParameters {
  CTRL_CHANNEL : range[1,16];  // Control Channel
  UPPER_CHANNEL: range[1,16];  // Tone A channel
  LOWER_CHANNEL: range[1,16];  // Tone B channel
  SYSEX        : ["OFF","RECV","SEND","ON"];
  SYSEX_IPR    : ["OFF","RECV","SEND","ON"];
  SYSEX_APR    : ["OFF","RECV","SEND","ON"];
  PATCH_PROG_CHG: ["OFF","ON"];
  UPPER_PROG_CHG: ["OFF","ON"];
  LOWER_PROG_CHG: ["OFF","ON"];
  MIDI_CC      : ["OFF","RECV","SEND","ON"];
  NOTE_MSG     : ["ALL","ODD","EVEN"];
  LOCAL_UPPER  : ["OFF","ON"] (JX-10 only);
  LOCAL_LOWER  : ["OFF","ON"] (JX-10 only);
  PG800        : ["OFF","ON"];
  EDIT_DISPLAY : ["OFF","SYSEX","PG800","CC","ALL"];
  COMPAT_MODE  : ["OFF","ON"];
  GU280_DISPLAY: ["OFF","ON"];
}
```

Include `description`, `default`, and `note` text from the prompt for tooltips.

### 2.2 Arpeggiator Patches (ARP-1 … ARP-8)

Parameters (per patch): BPM, Mode, Octave Span, Insert, Rate, Duration (`16` = LEGATO), Velocity Mode, MIDI sync.

### 2.3 Tone NRPN Table

Full CC table for DCO1/2, Mixer, VCF, VCA, Chorus, LFO, ENV1/2 per Vecoven spec. (Copy directly from prompt.)

### 2.4 Patch SysEx Mapping

Includes APR byte offsets for patch-level data (name, balance, split, portamento, assignment, chase, key mode, bend range). Provide helper `computeKeyMode()` and `computeBendRange()` functions.

### 2.5 Test Modes

Voice, Controller, Memory tests (Keyboard test flagged as JX-10 only). List access paths for tooltips.

---

## 3. UI Blueprint

Tab layout sketch:

```
┌─────────────────────────────────────────────────────────────┐
│ 🔧 Device Settings                           [Refresh][Save] │
├─────────────────────────────────────────────────────────────┤
│ Connection Status (ports + Test button + firmware badge)    │
│ MIDI Channels (Control/Upper/Lower combos)                  │
│ SysEx Settings (Vecoven 3.x)                                │
│ Program Change toggles                                      │
│ PG-800 & Display (PG-800 toggle, Edit Display combo, GU280) │
│ Compatibility (mode toggle + Note Message filter)           │
│ Arpeggiator (Patch selector + per-patch controls)          │
│ System Commands (Bulk, Reset, Write to Memory)             │
│ Quick Reference (recommended settings list)                │
└─────────────────────────────────────────────────────────────┘
```

**UX expectations**

1. Theme aware (dark/light). Use DS colors already defined elsewhere if reusing design system; otherwise CSS variables.
2. Tooltips on every enum explaining purpose, plus `jx10Only` badge. For MIDI menu settings that cannot be changed remotely, show a helper text (“Hardware only – adjust via front panel”).
3. Inline validation (range enforcement, discrete choices). Number inputs show display values (0–99, -50…50, note name) using `conversions` helpers.
4. Connection banner shows dot state (green/yellow/red). `Test Connection` sends MIDI identity request and parses response for “Vecoven 3.x”.
5. Buttons for `Refresh`, `Save`, `Request Bulk Dump`, `Send Bulk Dump`, `Factory Reset`, `Write Settings to Memory`. Save should remind user to press `MIDI → WRITE → ENTER` on hardware.

---

## 4. Required Functionality

1. **Port selection** (Web MIDI) + auto detect firmware (identity reply).
2. **Read/Write**: Request/Send all settings, per-param updates via SysEx or NRPN.
3. **Bulk ops**: request/send dumps, cartridge ↔ internal copy (if applicable).
4. **Factory reset instructions** (power on while holding `WRITE + LEFT ARROW`).
5. **Live logging** of SysEx/NRPN (optional) for debugging.

---

## 5. Code Scaffold (React + TS)

See `src/web/device-settings` for ready-to-use code:

- `deviceSettingsTypes.ts` – state, enums, parameter metadata interfaces.
- `midiMenuParameters.ts` – literal table with param numbers, ranges, text.
- `sysexBuilders.ts` – helpers for IPR/APR/WST/NRPN + value conversions.
- `midiService.ts` – Web MIDI wrapper (port listing, connection test, writes, bulk ops).
- `useDeviceSettings.ts` – reducer hook (connection state, midi menu, arp banks, logging).
- `DeviceSettingsTab.tsx` – UI composition with `SectionCard`, `ToggleGroup`, tooltips, responsive grid.

> **Note:** The provided code is framework-agnostic except for UI bindings. Replace the `SectionCard`/`Tooltip` wrappers with your design system equivalents if necessary.

---

## 6. Implementation Tips

- Use context to inject `MidiService` so the tab stays testable.
- Debounce value changes that would spam SysEx (e.g., sliders). Buttons can send immediately.
- Keep a max 100-entry SysEx log for performance.
- Provide fallbacks when Web MIDI is unavailable (disable controls + banner).
- Always wrap MIDI I/O in `try/catch` and display friendly error toasts.

---

## 7. Testing Checklist

1. **Identity Request** returns firmware string when hardware connected.
2. **SysEx toggles** send correct IPR messages (validate bytes in log).
3. **Arp patch editing** sends NRPN CC values per selected patch.
4. **Bulk dump** request receives APR payload, parsed without crashes.
5. **Theme** toggles (dark/light) keep contrast accessible.
6. **Hardware-only settings** show guidance text (no remote write attempt).

---

With this document and the accompanying TypeScript modules, the Device Settings tab can be generated end-to-end without manual copy/paste.
