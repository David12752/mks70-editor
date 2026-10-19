# MKS-70 Editor - Build and Create Installer
# This script builds the Release version and creates the installer

param(
    [switch]$SkipBuild = $false,
    [switch]$OpenFolder = $true
)

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "MKS-70 Editor - Installer Builder" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

$ErrorActionPreference = "Stop"
$StartTime = Get-Date

# Paths
$RootDir = Split-Path -Parent $PSScriptRoot
$SolutionFile = Join-Path $RootDir "Builds\VisualStudio2022\MKS70 Editor.sln"
$InstallerScript = Join-Path $PSScriptRoot "setup.iss"
$OutputDir = Join-Path $PSScriptRoot "output"
$ReleaseExe = Join-Path $RootDir "Builds\VisualStudio2022\x64\Release\Standalone Plugin\MKS70 Editor.exe"

# Find MSBuild
Write-Host "[1/4] Locating MSBuild..." -ForegroundColor Yellow
$MSBuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"
if (-not (Test-Path $MSBuild)) {
    Write-Host "ERROR: MSBuild not found at: $MSBuild" -ForegroundColor Red
    exit 1
}
Write-Host "      ✓ Found: $MSBuild" -ForegroundColor Green

# Build Release version (unless skipped)
if (-not $SkipBuild) {
    Write-Host ""
    Write-Host "[2/4] Building Release version..." -ForegroundColor Yellow
    Write-Host "      This may take 1-2 minutes..." -ForegroundColor Gray
    
    & $MSBuild $SolutionFile /p:Configuration=Release /p:Platform=x64 /m /v:minimal
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
        exit 1
    }
    
    # Verify EXE exists
    if (-not (Test-Path $ReleaseExe)) {
        Write-Host "ERROR: Release EXE not found at: $ReleaseExe" -ForegroundColor Red
        exit 1
    }
    
    $ExeSize = (Get-Item $ReleaseExe).Length / 1MB
    Write-Host "      ✓ Build successful! EXE size: $([math]::Round($ExeSize, 2)) MB" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "[2/4] Skipping build (using existing Release)..." -ForegroundColor Yellow
    
    if (-not (Test-Path $ReleaseExe)) {
        Write-Host "ERROR: Release EXE not found. Run without -SkipBuild first." -ForegroundColor Red
        exit 1
    }
}

# Find Inno Setup
Write-Host ""
Write-Host "[3/4] Locating Inno Setup Compiler..." -ForegroundColor Yellow

$InnoSetupPaths = @(
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe",
    "C:\Program Files\Inno Setup 6\ISCC.exe",
    "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
    "$env:ProgramFiles\Inno Setup 6\ISCC.exe"
)

$ISCC = $null
foreach ($path in $InnoSetupPaths) {
    if (Test-Path $path) {
        $ISCC = $path
        break
    }
}

if (-not $ISCC) {
    Write-Host "ERROR: Inno Setup not found!" -ForegroundColor Red
    Write-Host "Please download and install from: https://jrsoftware.org/isdl.php" -ForegroundColor Yellow
    Write-Host "Looking for ISCC.exe in these locations:" -ForegroundColor Gray
    $InnoSetupPaths | ForEach-Object { Write-Host "  - $_" -ForegroundColor Gray }
    exit 1
}

Write-Host "      ✓ Found: $ISCC" -ForegroundColor Green

# Create installer
Write-Host ""
Write-Host "[4/4] Creating installer..." -ForegroundColor Yellow

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

# Count presets
$PresetCount = (Get-ChildItem -Path (Join-Path $RootDir "Presets") -Filter "*.syx" -Recurse).Count

Write-Host "      • Packaging $PresetCount presets" -ForegroundColor Gray
Write-Host "      • Compiling installer script..." -ForegroundColor Gray

& $ISCC $InstallerScript /Q

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Installer creation failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit 1
}

# Find the created installer
$InstallerFile = Get-ChildItem -Path $OutputDir -Filter "MKS70_Editor_Setup_*.exe" | Sort-Object LastWriteTime -Descending | Select-Object -First 1

if (-not $InstallerFile) {
    Write-Host "ERROR: Installer file not found in output directory" -ForegroundColor Red
    exit 1
}

$InstallerSize = $InstallerFile.Length / 1MB
$Duration = (Get-Date) - $StartTime

Write-Host ""
Write-Host "=====================================" -ForegroundColor Green
Write-Host "✓ SUCCESS!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green
Write-Host ""
Write-Host "Installer created:" -ForegroundColor White
Write-Host "  File: $($InstallerFile.Name)" -ForegroundColor Cyan
Write-Host "  Size: $([math]::Round($InstallerSize, 2)) MB" -ForegroundColor Cyan
Write-Host "  Path: $($InstallerFile.FullName)" -ForegroundColor Cyan
Write-Host ""
Write-Host "Build time: $([math]::Round($Duration.TotalSeconds, 1)) seconds" -ForegroundColor Gray
Write-Host ""
Write-Host "You can now distribute this installer to your friends!" -ForegroundColor Yellow
Write-Host ""

# Open output folder
if ($OpenFolder) {
    Start-Process explorer.exe -ArgumentList "/select,`"$($InstallerFile.FullName)`""
}
