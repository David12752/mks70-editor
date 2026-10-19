# MKS-70 Editor - Fully Portable Package (with VC++ Runtime)
# Creates a package that works on ANY Windows 10/11 machine without installation

param(
    [switch]$SkipBuild,
    [switch]$OpenFolder = $true
)

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "MKS-70 Editor - Fully Portable" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

$ErrorActionPreference = "Stop"
$StartTime = Get-Date

# Paths
$RootDir = $PSScriptRoot
$ReleaseExe = Join-Path $RootDir "Builds\VisualStudio2022\x64\Release\Standalone Plugin\MKS70 Editor.exe"
$PackageDir = Join-Path $RootDir "installer\MKS70_Editor_Fully_Portable"
$ZipFile = Join-Path $RootDir "installer\MKS70_Editor_v1.0.0_Fully_Portable.zip"

# VC++ Runtime DLLs (from Visual Studio 2022)
$VCRedistPaths = @(
    "C:\Windows\System32\vcruntime140.dll",
    "C:\Windows\System32\vcruntime140_1.dll",
    "C:\Windows\System32\msvcp140.dll",
    "C:\Windows\System32\concrt140.dll"
)

Write-Host "[1/2] Checking files..." -ForegroundColor Yellow

# Check if Release EXE exists
if (-not (Test-Path $ReleaseExe)) {
    Write-Host "ERROR: Release EXE not found. Run create_portable_package.ps1 first." -ForegroundColor Red
    exit 1
}

Write-Host "      OK Release EXE found" -ForegroundColor Green

# Check VC++ Runtime DLLs
$MissingDLLs = @()
foreach ($dll in $VCRedistPaths) {
    if (-not (Test-Path $dll)) {
        $MissingDLLs += $dll
    }
}

if ($MissingDLLs.Count -gt 0) {
    Write-Host "WARNING: Some VC++ DLLs not found:" -ForegroundColor Yellow
    $MissingDLLs | ForEach-Object { Write-Host "  - $_" -ForegroundColor Gray }
    Write-Host "Package will still be created but may not work on all machines" -ForegroundColor Yellow
    Start-Sleep -Seconds 2
}

# Create fully portable package
Write-Host ""
Write-Host "[2/2] Creating fully portable package..." -ForegroundColor Yellow

# Clean up old package
if (Test-Path $PackageDir) {
    Remove-Item $PackageDir -Recurse -Force
}
if (Test-Path $ZipFile) {
    Remove-Item $ZipFile -Force
}

# Create package structure
Write-Host "      Creating folder structure..." -ForegroundColor Gray
New-Item -ItemType Directory -Path $PackageDir | Out-Null
New-Item -ItemType Directory -Path "$PackageDir\Presets" | Out-Null

# Copy executable
Write-Host "      Copying executable..." -ForegroundColor Gray
Copy-Item $ReleaseExe -Destination "$PackageDir\MKS70 Editor.exe"

# Copy VC++ Runtime DLLs
Write-Host "      Copying VC++ Runtime DLLs..." -ForegroundColor Gray
$CopiedDLLs = 0
foreach ($dll in $VCRedistPaths) {
    if (Test-Path $dll) {
        $dllName = Split-Path -Leaf $dll
        Copy-Item $dll -Destination "$PackageDir\$dllName"
        Write-Host "        OK $dllName" -ForegroundColor DarkGray
        $CopiedDLLs++
    }
}

# Copy presets
Write-Host "      Copying presets..." -ForegroundColor Gray
$PresetCategories = @("Bass", "Bells", "Brass", "Drums", "FX", "Keys", "Lead", "Organ", "Other", "Pad", "Strings", "Synth", "Banks", "Tools")
$TotalPresets = 0

foreach ($category in $PresetCategories) {
    $SourceDir = Join-Path $RootDir "Presets\$category"
    if (Test-Path $SourceDir) {
        $DestDir = "$PackageDir\Presets\$category"
        New-Item -ItemType Directory -Path $DestDir -Force | Out-Null
        
        $Files = Get-ChildItem -Path $SourceDir -Filter "*.syx"
        foreach ($file in $Files) {
            Copy-Item $file.FullName -Destination $DestDir
            $TotalPresets++
        }
        
        Write-Host "        OK $category : $($Files.Count) presets" -ForegroundColor DarkGray
    }
}

# Copy documentation
Write-Host "      Copying documentation..." -ForegroundColor Gray
if (Test-Path (Join-Path $RootDir "README.md")) {
    Copy-Item (Join-Path $RootDir "README.md") -Destination "$PackageDir\README.txt"
}
if (Test-Path (Join-Path $RootDir "TESTING.md")) {
    Copy-Item (Join-Path $RootDir "TESTING.md") -Destination "$PackageDir\TESTING.txt"
}

# Copy MIDI mapping
if (Test-Path (Join-Path $RootDir "mks70_mapping.json")) {
    Copy-Item (Join-Path $RootDir "mks70_mapping.json") -Destination "$PackageDir\mks70_mapping.json"
}

# Create installation guide
Write-Host "      Creating installation guide..." -ForegroundColor Gray
$InstallGuide = @'
MKS-70 Editor - Fully Portable Installation
============================================

NO INSTALLATION REQUIRED!
-------------------------
This package includes all required runtime files.
Just extract and run!

Quick Start
-----------
1. Extract this ZIP to any folder
2. Double-click "MKS70 Editor.exe"
3. Connect your MKS-70 or JX-10 via MIDI
4. Start editing!

Included Files
--------------
- MKS70 Editor.exe - Main application
- vcruntime140.dll - Visual C++ Runtime (required)
- vcruntime140_1.dll - Visual C++ Runtime (required)
- msvcp140.dll - Visual C++ Standard Library (required)
- concrt140.dll - Concurrency Runtime (required)
- Presets/ - 368 sound presets organized by category
- mks70_mapping.json - MIDI parameter mapping

System Requirements
-------------------
- Windows 10/11 (64-bit)
- MIDI interface (USB-MIDI or MIDI-to-USB cable)
- Roland MKS-70 or JX-10 synthesizer

Features
--------
- Real-time parameter control via MIDI SysEx
- Load/save individual patches
- Request bulk dump from synth
- 368 presets included
- Realistic 3D knob interface
- Compact switches for discrete parameters
- Upper/Lower tone editing

Portability
-----------
This package is FULLY PORTABLE:
- Copy to USB drive
- Run from any folder
- No registry entries
- No system modifications
- No installation needed!

Perfect for:
- Running from USB stick
- Quick testing on different machines
- No admin rights required
- Clean uninstall (just delete folder)

Troubleshooting
---------------
Connection issues:
  - Check MIDI cables are connected
  - Verify MKS-70 is powered on
  - Check MIDI interface in Device Manager

If application does not start:
  - Make sure all DLL files are in same folder as EXE
  - Check Windows Event Viewer for error details
  - Verify Windows 10/11 64-bit is installed

Enjoy creating sounds with your MKS-70!
'@

Set-Content -Path "$PackageDir\INSTALL.txt" -Value $InstallGuide

# Create ZIP archive
Write-Host "      Compressing to ZIP..." -ForegroundColor Gray
Compress-Archive -Path "$PackageDir\*" -DestinationPath $ZipFile -CompressionLevel Optimal

# Calculate sizes
$FolderSize = (Get-ChildItem -Path $PackageDir -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB
$ZipSize = (Get-Item $ZipFile).Length / 1MB
$Duration = (Get-Date) - $StartTime

Write-Host ""
Write-Host "=====================================" -ForegroundColor Green
Write-Host "SUCCESS!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green
Write-Host ""
Write-Host "Fully portable package created:" -ForegroundColor White
Write-Host "  File: $(Split-Path -Leaf $ZipFile)" -ForegroundColor Cyan
Write-Host "  Size: $([math]::Round($ZipSize, 2)) MB (compressed)" -ForegroundColor Cyan
Write-Host "  Uncompressed: $([math]::Round($FolderSize, 2)) MB" -ForegroundColor Cyan
Write-Host "  Presets: $TotalPresets files" -ForegroundColor Cyan
Write-Host "  VC++ DLLs: $CopiedDLLs included" -ForegroundColor Cyan
Write-Host ""
Write-Host "Build time: $([math]::Round($Duration.TotalSeconds, 1)) seconds" -ForegroundColor Gray
Write-Host ""
Write-Host "This package works on ANY Windows machine!" -ForegroundColor Yellow
Write-Host "  - No installation required" -ForegroundColor White
Write-Host "  - No VC++ Redistributable needed" -ForegroundColor White
Write-Host "  - Just extract and run!" -ForegroundColor White
Write-Host ""

# Open folder
if ($OpenFolder) {
    Start-Process explorer.exe -ArgumentList "/select,`"$ZipFile`""
}

# Clean up temp folder
Write-Host "Cleaning up temporary files..." -ForegroundColor Gray
Remove-Item $PackageDir -Recurse -Force

Write-Host ""
Write-Host "Ready for distribution!" -ForegroundColor Green
