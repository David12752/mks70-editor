# MKS-70 Editor - Simple Portable Package Builder
# Creates a ZIP package that can be distributed without installer

param(
    [switch]$SkipBuild,
    [switch]$OpenFolder = $true
)

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "MKS-70 Editor - Portable Package" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

$ErrorActionPreference = "Stop"
$StartTime = Get-Date

# Paths
$RootDir = $PSScriptRoot
$SolutionFile = Join-Path $RootDir "Builds\VisualStudio2022\MKS70 Editor.sln"
$ReleaseExe = Join-Path $RootDir "Builds\VisualStudio2022\x64\Release\Standalone Plugin\MKS70 Editor.exe"
$PackageDir = Join-Path $RootDir "installer\MKS70_Editor_Portable"
$ZipFile = Join-Path $RootDir "installer\MKS70_Editor_v1.0.0_Portable.zip"

# Find MSBuild
Write-Host "[1/3] Locating MSBuild..." -ForegroundColor Yellow
$MSBuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"
if (-not (Test-Path $MSBuild)) {
    Write-Host "ERROR: MSBuild not found" -ForegroundColor Red
    exit 1
}
Write-Host "      OK Found MSBuild" -ForegroundColor Green

# Build Release version (unless skipped)
if (-not $SkipBuild) {
    Write-Host ""
    Write-Host "[2/3] Building Release version..." -ForegroundColor Yellow
    Write-Host "      This may take 1-2 minutes..." -ForegroundColor Gray
    
    & $MSBuild $SolutionFile /p:Configuration=Release /p:Platform=x64 /m /v:minimal
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Build failed" -ForegroundColor Red
        exit 1
    }
    
    if (-not (Test-Path $ReleaseExe)) {
        Write-Host "ERROR: Release EXE not found" -ForegroundColor Red
        exit 1
    }
    
    $ExeSize = (Get-Item $ReleaseExe).Length / 1MB
    Write-Host "      OK Build successful! Size: $([math]::Round($ExeSize, 2)) MB" -ForegroundColor Green
}
else {
    Write-Host ""
    Write-Host "[2/3] Skipping build (using existing)..." -ForegroundColor Yellow
    
    if (-not (Test-Path $ReleaseExe)) {
        Write-Host "ERROR: Release EXE not found" -ForegroundColor Red
        exit 1
    }
}

# Create portable package
Write-Host ""
Write-Host "[3/3] Creating portable package..." -ForegroundColor Yellow

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

# Copy presets with structure
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

# Create installation instructions
Write-Host "      Creating installation guide..." -ForegroundColor Gray
$InstallGuide = @'
MKS-70 Editor - Installation Instructions
==========================================

Quick Start
-----------
1. Run the application - Double-click MKS70 Editor.exe
2. Connect your synth - MKS-70 or JX-10 via MIDI interface
3. Test connection - Go to Settings tab and click Test Connection
4. Load presets - Browse the Presets folder (160+ sounds)

Folder Contents
---------------
- MKS70 Editor.exe - Main application
- Presets - 160+ sound presets organized by category
- mks70_mapping.json - MIDI parameter mapping
- README.txt - Full documentation
- TESTING.txt - Testing guide

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
- 160+ presets included
- Realistic 3D knob interface
- Upper/Lower tone editing

Troubleshooting
---------------
Connection issues:
  - Check MIDI cables
  - Verify MKS-70 is powered on
  - Check MIDI interface in Device Manager

Application issues:
  - Install Visual C++ Redistributable 2022 x64
  - Download: https://aka.ms/vs/17/release/vc_redist.x64.exe

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
Write-Host "Portable package created:" -ForegroundColor White
Write-Host "  File: $(Split-Path -Leaf $ZipFile)" -ForegroundColor Cyan
Write-Host "  Size: $([math]::Round($ZipSize, 2)) MB (compressed)" -ForegroundColor Cyan
Write-Host "  Uncompressed: $([math]::Round($FolderSize, 2)) MB" -ForegroundColor Cyan
Write-Host "  Presets: $TotalPresets files" -ForegroundColor Cyan
Write-Host ""
Write-Host "Build time: $([math]::Round($Duration.TotalSeconds, 1)) seconds" -ForegroundColor Gray
Write-Host ""
Write-Host "Distribution:" -ForegroundColor Yellow
Write-Host "  1. Share the ZIP file" -ForegroundColor White
Write-Host "  2. Users extract and run MKS70 Editor.exe" -ForegroundColor White
Write-Host "  3. No installation required - fully portable!" -ForegroundColor White
Write-Host ""

# Open folder
if ($OpenFolder) {
    Start-Process explorer.exe -ArgumentList "/select,`"$ZipFile`""
}

# Clean up temp folder
Write-Host "Cleaning up temporary files..." -ForegroundColor Gray
Remove-Item $PackageDir -Recurse -Force

Write-Host ""
Write-Host "Ready to distribute!" -ForegroundColor Green
