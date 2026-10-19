# MKS-70 Editor - Installer Creation Guide

## Quick Start

### Prerequisites
1. **Inno Setup 6** - Download from: https://jrsoftware.org/isdl.php
2. **Visual Studio 2022** with C++ Desktop Development
3. **Release build** of MKS-70 Editor

### Create Installer (Automatic)

Simply run the PowerShell script:

```powershell
cd installer
.\build_installer.ps1
```

This will:
1. Build Release version (x64)
2. Package the EXE and 160+ presets
3. Create installer in `installer\output\`
4. Open the folder when done

### Create Installer (Manual)

If you prefer manual control:

1. **Build Release:**
   ```powershell
   cd "Builds\VisualStudio2022"
   MSBuild.exe "MKS70 Editor.sln" /p:Configuration=Release /p:Platform=x64
   ```

2. **Compile Installer:**
   ```powershell
   cd installer
   "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" setup.iss
   ```

3. **Find installer:**
   ```
   installer\output\MKS70_Editor_Setup_v1.0.0.exe
   ```

## What Gets Installed

The installer includes:

### Application Files
- **Standalone App:** `MKS70 Editor.exe` (main application)
- **VST3 Plugin:** `MKS70 Editor.vst3` (if available)
- **Documentation:** README and TESTING guides

### Presets (160+ files)
All presets are installed to user's AppData:
```
%APPDATA%\MKS-70 Editor\Presets\
├── Bass\       (Synth bass sounds)
├── Bells\      (Bell and mallet sounds)
├── Brass\      (Brass and wind sounds)
├── Drums\      (Percussion sounds)
├── FX\         (Sound effects)
├── Keys\       (Piano, clav, harpsichord)
├── Lead\       (Lead synth sounds)
├── Organ\      (Organ sounds)
├── Pad\        (Pad and atmosphere sounds)
├── Strings\    (String sounds)
├── Synth\      (General synth sounds)
├── Banks\      (Full factory banks)
└── Tools\      (Utility patches)
```

### MIDI Configuration
- `mks70_mapping.json` - Parameter mapping for SysEx

## Installation Options

During installation, users can choose:
- ✅ **Desktop shortcut** (optional)
- ✅ **Quick Launch icon** (optional)
- ✅ **Start Menu group** (default)
- ✅ **Launch after install** (default)

## System Requirements

- **OS:** Windows 10/11 (x64)
- **MIDI:** USB-MIDI interface or MIDI-to-USB cable
- **Synth:** Roland MKS-70 or JX-10
- **RAM:** 100 MB
- **Disk Space:** ~50 MB (includes all presets)

## Distribution

The created installer is a single `.exe` file that can be:
- Shared via email, USB, or cloud storage
- Distributed on forums (Gearslutz, VI-Control, etc.)
- Posted on GitHub Releases
- Uploaded to your website

**File size:** ~15-25 MB (compressed with LZMA2)

## Installer Features

✅ **Professional appearance** - Modern Windows installer UI  
✅ **Multi-language** - English and Hebrew support  
✅ **Clean uninstall** - Removes all files properly  
✅ **No registry mess** - Portable preset storage  
✅ **Admin rights** - Handles VST3 installation correctly  
✅ **Smart detection** - Only installs VST3 if available  

## Troubleshooting

### "Inno Setup not found"
Download and install Inno Setup 6 from:
https://jrsoftware.org/isdl.php

### "Release EXE not found"
Run without `-SkipBuild` or build Release manually:
```powershell
.\build_installer.ps1  # Will build first
```

### "Build failed"
Check that Visual Studio 2022 is installed with:
- Desktop development with C++
- Windows 10/11 SDK
- MSVC v143 compiler

## Advanced Options

### Skip build (use existing Release)
```powershell
.\build_installer.ps1 -SkipBuild
```

### Don't open folder after
```powershell
.\build_installer.ps1 -OpenFolder:$false
```

### Edit version number
Open `setup.iss` and change:
```pascal
#define MyAppVersion "1.0.0"
```

## Testing the Installer

Before distribution, test on a clean machine:

1. **Run installer** on fresh Windows VM
2. **Check Start Menu** - shortcut exists
3. **Launch app** - opens without errors
4. **Check presets** - all 160+ files present
5. **Test MIDI** - connects to MKS-70
6. **Uninstall** - removes cleanly

## File Locations After Install

```
Program Files:
  C:\Program Files\MKS-70 Editor\
  └── MKS70 Editor.exe

VST3 (if installed):
  C:\Program Files\Common Files\VST3\
  └── MKS70 Editor.vst3\

User Data:
  C:\Users\[Username]\AppData\Roaming\MKS-70 Editor\
  ├── Presets\       (160+ .syx files)
  └── mks70_mapping.json
```

## Support

For issues or questions:
- GitHub Issues: [your-repo-url]
- Email: [your-email]
- Forum: [forum-link]

---

**Happy distributing!** 🎹✨
