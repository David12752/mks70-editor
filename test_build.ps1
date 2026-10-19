# MKS-70 Editor Build Test Script
# Checks project structure and attempts build

Write-Host "=== MKS-70 Editor Build Test ===" -ForegroundColor Cyan
Write-Host ""

# MSBuild can fail on Windows if the process environment contains both Path and
# PATH. The Env: provider itself can throw in that state, so avoid querying it.
$processPath = [Environment]::GetEnvironmentVariable('Path', 'Process')
Remove-Item Env:PATH -ErrorAction SilentlyContinue
if (-not [string]::IsNullOrEmpty($processPath)) {
    $env:Path = $processPath
}

# Check if all source files exist
Write-Host "Checking source files..." -ForegroundColor Yellow
$sourceFiles = @(
    "Source\PluginProcessor.h",
    "Source\PluginProcessor.cpp",
    "Source\PluginEditor.h",
    "Source\PluginEditor.cpp",
    "Source\ToneParameters.h",
    "Source\PatchParameters.h",
    "Source\ArpParameters.h",
    "Source\MidiHandler.h",
    "Source\MidiHandler.cpp",
    "Source\ToneUpperTab.h",
    "Source\ToneUpperTab.cpp",
    "Source\ToneLowerTab.h",
    "Source\ToneLowerTab.cpp",
    "Source\PatchTab.h",
    "Source\PatchTab.cpp",
    "Source\ArpTab.h",
    "Source\ArpTab.cpp",
    "Source\AutomationRecorder.h",
    "Source\AutomationRecorder.cpp",
    "Source\AutomationLaneComponent.h",
    "Source\AutomationLaneComponent.cpp",
    "Source\AutomationPanel.h",
    "Source\AutomationPanel.cpp",
    "Source\LibrarianTab.h",
    "Source\LibrarianTab.cpp",
    "Source\SettingsTab.h",
    "Source\SettingsTab.cpp"
)

$allFilesExist = $true
foreach ($file in $sourceFiles) {
    if (Test-Path $file) {
        Write-Host "  [OK] $file" -ForegroundColor Green
    } else {
        Write-Host "  [MISSING] $file" -ForegroundColor Red
        $allFilesExist = $false
    }
}

Write-Host ""

# Check project files
Write-Host "Checking project files..." -ForegroundColor Yellow
$projectFiles = @(
    "Builds\VisualStudio2022\MKS70 Editor.sln",
    "Builds\VisualStudio2022\MKS70 Editor_SharedCode.vcxproj"
)

foreach ($file in $projectFiles) {
    if (Test-Path $file) {
        Write-Host "  [OK] $file" -ForegroundColor Green
    } else {
        Write-Host "  [MISSING] $file" -ForegroundColor Red
        $allFilesExist = $false
    }
}

Write-Host ""

if (-not $allFilesExist) {
    Write-Host "ERROR: Some required files are missing!" -ForegroundColor Red
    exit 1
}

Write-Host "All required files found!" -ForegroundColor Green
Write-Host ""

# Validate mapping and preset data before compiling. This catches broken editor
# resources even when the C++ project itself still builds.
Write-Host "Checking mapping and preset data..." -ForegroundColor Yellow

if (Test-Path "mks70_mapping.json") {
    try {
        $mappingJson = Get-Content "mks70_mapping.json" -Raw | ConvertFrom-Json
        if (-not $mappingJson.device -or -not $mappingJson.ToneParams) {
            Write-Host "  [INVALID] mks70_mapping.json is missing required fields" -ForegroundColor Red
            exit 1
        }

        Write-Host "  [OK] mks70_mapping.json parses successfully" -ForegroundColor Green
    } catch {
        Write-Host "  [INVALID] mks70_mapping.json could not be parsed" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "  [MISSING] mks70_mapping.json" -ForegroundColor Red
    exit 1
}

if (Test-Path "Presets") {
    $syxFiles = Get-ChildItem -Path "Presets" -Recurse -File -Filter "*.syx" -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -notmatch '\\__MACOSX\\' -and -not $_.Name.StartsWith("._") }
    $badSyx = @()

    foreach ($syx in $syxFiles) {
        $bytes = [System.IO.File]::ReadAllBytes($syx.FullName)
        $hasStart = $false
        $hasEnd = $false

        foreach ($b in $bytes) {
            if ($b -eq 0xF0) { $hasStart = $true }
            if ($b -eq 0xF7) { $hasEnd = $true }
            if ($hasStart -and $hasEnd) { break }
        }

        if ($bytes.Length -eq 0 -or -not $hasStart -or -not $hasEnd) {
            $badSyx += $syx.FullName
        }
    }

    if ($badSyx.Count -gt 0) {
        Write-Host "  [INVALID] Broken SysEx preset files:" -ForegroundColor Red
        $badSyx | Select-Object -First 20 | ForEach-Object { Write-Host "    $_" -ForegroundColor Red }
        if ($badSyx.Count -gt 20) {
            Write-Host "    ... and $($badSyx.Count - 20) more" -ForegroundColor Red
        }
        exit 1
    }

    Write-Host "  [OK] $($syxFiles.Count) SysEx preset files contain F0/F7 framing" -ForegroundColor Green
} else {
    Write-Host "  [WARNING] Presets folder not found; preset scan skipped" -ForegroundColor Yellow
}

Write-Host ""

# Check for MSBuild
Write-Host "Looking for MSBuild..." -ForegroundColor Yellow
$msbuildPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe -prerelease | Select-Object -First 1

if ($msbuildPath -and (Test-Path $msbuildPath)) {
    $amd64MSBuildPath = Join-Path (Split-Path $msbuildPath -Parent) "amd64\MSBuild.exe"
    if (Test-Path $amd64MSBuildPath) {
        $msbuildPath = $amd64MSBuildPath
    }

    Write-Host "  [OK] Found MSBuild at: $msbuildPath" -ForegroundColor Green
} else {
    Write-Host "  [WARNING] MSBuild not found. Build will be skipped." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Structure check PASSED! Ready to build in Visual Studio." -ForegroundColor Green
    exit 0
}

Write-Host ""

# Attempt to build
Write-Host "Attempting to build project..." -ForegroundColor Yellow
Write-Host "Building Debug|x64 configuration..." -ForegroundColor Cyan
Write-Host ""

$solutionPath = "Builds\VisualStudio2022\MKS70 Editor.sln"

try {
    & $msbuildPath $solutionPath /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "=== BUILD SUCCESSFUL ===" -ForegroundColor Green
        Write-Host ""
        Write-Host "The MKS-70 Editor project structure is correct and builds successfully!" -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "=== BUILD FAILED ===" -ForegroundColor Red
        Write-Host "Exit code: $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
} catch {
    Write-Host ""
    Write-Host "=== BUILD ERROR ===" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
