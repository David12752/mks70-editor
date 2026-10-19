; ============================================================================
; MKS-70 Editor - Professional Installer Script
; ============================================================================
; Roland MKS-70 / JX-10 Synthesizer Editor
; Version: 1.0.0
; Created: December 2025
;
; SECURITY MEASURES INCLUDED:
; - Full publisher information
; - Digital signature ready
; - Clean uninstall
; - Proper registry entries
; - Windows SmartScreen compatible
; ============================================================================

#define MyAppName "MKS-70 Editor"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "MKS-70 Editor Development Team"
#define MyAppURL "https://github.com/mks70editor/mks70-editor"
#define MyAppExeName "MKS70 Editor.exe"
#define MyAppDescription "Professional editor for Roland MKS-70 and JX-10 synthesizers"
#define MyAppCopyright "Copyright (c) 2025 MKS-70 Editor Development Team"

[Setup]
; ============================================================================
; Application Identity (Important for antivirus reputation)
; ============================================================================
AppId={{A8F3C2B1-9D4E-4A5C-B6E7-8F9A0B1C2D3E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases
AppContact=support@mks70editor.com
AppComments={#MyAppDescription}
AppCopyright={#MyAppCopyright}
VersionInfoVersion=1.0.0.0
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppDescription}
VersionInfoTextVersion={#MyAppVersion}
VersionInfoCopyright={#MyAppCopyright}
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}
VersionInfoProductTextVersion={#MyAppVersion}

; ============================================================================
; Installation Settings
; ============================================================================
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE.txt
; InfoBeforeFile and InfoAfterFile are optional
; InfoBeforeFile=..\README.md
; InfoAfterFile=..\docs\WELCOME.txt

; ============================================================================
; Output Configuration
; ============================================================================
OutputDir=output
OutputBaseFilename=MKS70_Editor_Setup_v{#MyAppVersion}
; SetupIconFile=..\artifacts\icon.ico  ; Uncomment if icon exists
UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName={#MyAppName}

; ============================================================================
; Compression (LZMA2 for best ratio)
; ============================================================================
Compression=lzma2/ultra64
SolidCompression=yes
LZMAUseSeparateProcess=yes
LZMADictionarySize=65536
LZMANumFastBytes=273

; ============================================================================
; Visual Style
; ============================================================================
WizardStyle=modern
WizardSizePercent=120
WizardResizable=yes
ShowLanguageDialog=yes

; ============================================================================
; System Requirements
; ============================================================================
MinVersion=10.0
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; ============================================================================
; Privileges (Admin for VST3 installation, but can fallback to user)
; ============================================================================
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog commandline

; ============================================================================
; Security Flags (SignTool configured externally via build script)
; ============================================================================
; SignTool is applied via post-build script for flexibility
CloseApplications=yes
RestartApplications=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"

[Types]
Name: "full"; Description: "Full installation (Standalone + VST3 + All Presets)"
Name: "standalone"; Description: "Standalone application only"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "main"; Description: "MKS-70 Editor Application"; Types: full standalone custom; Flags: fixed
Name: "vst3"; Description: "VST3 Plugin (for DAW integration)"; Types: full
Name: "presets"; Description: "Factory Presets (300+)"; Types: full standalone
Name: "presets\bass"; Description: "Bass Sounds"; Types: full standalone
Name: "presets\lead"; Description: "Lead Sounds"; Types: full standalone
Name: "presets\pad"; Description: "Pad Sounds"; Types: full standalone
Name: "presets\keys"; Description: "Keys & Piano Sounds"; Types: full standalone
Name: "presets\strings"; Description: "String Sounds"; Types: full standalone
Name: "presets\brass"; Description: "Brass Sounds"; Types: full standalone
Name: "presets\fx"; Description: "FX & Special Sounds"; Types: full standalone
Name: "presets\other"; Description: "Other Sounds & Banks"; Types: full standalone
Name: "docs"; Description: "Documentation"; Types: full

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "associatesyx"; Description: "Associate .syx files with MKS-70 Editor"; GroupDescription: "File Associations:"

[Files]
; ============================================================================
; Main Application
; ============================================================================
Source: "..\Builds\VisualStudio2022\x64\Release\Standalone Plugin\MKS70 Editor.exe"; DestDir: "{app}"; Components: main; Flags: ignoreversion

; ============================================================================
; VST3 Plugin
; ============================================================================
Source: "..\Builds\VisualStudio2022\x64\Release\VST3\MKS70 Editor.vst3\*"; DestDir: "{commoncf64}\VST3\MKS70 Editor.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs; Check: DirExists(ExpandConstant('{src}\..\Builds\VisualStudio2022\x64\Release\VST3\MKS70 Editor.vst3'))

; ============================================================================
; Configuration Files
; ============================================================================
Source: "..\mks70_mapping.json"; DestDir: "{app}"; Components: main; Flags: ignoreversion

; ============================================================================
; Documentation
; ============================================================================
Source: "..\README.md"; DestDir: "{app}"; DestName: "README.txt"; Components: docs; Flags: ignoreversion isreadme
Source: "..\TESTING.md"; DestDir: "{app}\docs"; DestName: "TESTING.txt"; Components: docs; Flags: ignoreversion
Source: "..\docs\*.md"; DestDir: "{app}\docs"; Components: docs; Flags: ignoreversion skipifsourcedoesntexist

; ============================================================================
; Presets - Organized by Category
; ============================================================================
Source: "..\Presets\Bass\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Bass"; Components: presets\bass; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Lead\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Lead"; Components: presets\lead; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Pad\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Pad"; Components: presets\pad; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Keys\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Keys"; Components: presets\keys; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Strings\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Strings"; Components: presets\strings; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Brass\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Brass"; Components: presets\brass; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\FX\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\FX"; Components: presets\fx; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Bells\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Bells"; Components: presets\other; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Drums\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Drums"; Components: presets\other; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Organ\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Organ"; Components: presets\other; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Synth\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Synth"; Components: presets\other; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Banks\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Banks"; Components: presets\other; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
Source: "..\Presets\Other\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Other"; Components: presets\other; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist

; ============================================================================
; Visual C++ Runtime (embedded if needed)
; ============================================================================
; Source: "redist\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; Check: NeedsVCRedist

[Icons]
; Start Menu
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Comment: "{#MyAppDescription}"
Name: "{group}\Presets Folder"; Filename: "{userappdata}\{#MyAppName}\Presets"
Name: "{group}\Documentation"; Filename: "{app}\README.txt"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

; Desktop Icon
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Comment: "{#MyAppDescription}"; Tasks: desktopicon

; Quick Launch
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Registry]
; Application registration
Root: HKLM; Subkey: "SOFTWARE\{#MyAppPublisher}\{#MyAppName}"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\{#MyAppPublisher}\{#MyAppName}"; ValueType: string; ValueName: "Version"; ValueData: "{#MyAppVersion}"
Root: HKLM; Subkey: "SOFTWARE\{#MyAppPublisher}\{#MyAppName}"; ValueType: string; ValueName: "PresetsPath"; ValueData: "{userappdata}\{#MyAppName}\Presets"

; .syx file association
Root: HKCR; Subkey: ".syx"; ValueType: string; ValueName: ""; ValueData: "MKS70Editor.SysEx"; Flags: uninsdeletevalue; Tasks: associatesyx
Root: HKCR; Subkey: "MKS70Editor.SysEx"; ValueType: string; ValueName: ""; ValueData: "MIDI System Exclusive File"; Flags: uninsdeletekey; Tasks: associatesyx
Root: HKCR; Subkey: "MKS70Editor.SysEx\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: associatesyx
Root: HKCR; Subkey: "MKS70Editor.SysEx\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: associatesyx

[Run]
; Install VC++ Runtime if needed
; Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing Visual C++ Runtime..."; Check: NeedsVCRedist

; Launch application after installation
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
; Clean up on uninstall (optional)
; Filename: "{app}\cleanup.exe"; Parameters: "/silent"; RunOnceId: "Cleanup"

[UninstallDelete]
; Remove config files (optional - keep presets)
Type: files; Name: "{userappdata}\{#MyAppName}\settings.json"
Type: dirifempty; Name: "{userappdata}\{#MyAppName}"

[Code]
// ============================================================================
// Custom Pascal Code
// ============================================================================

var
  DownloadPage: TDownloadWizardPage;

// Check if VC++ Runtime is installed
function NeedsVCRedist: Boolean;
var
  Key: String;
  Install: Cardinal;
begin
  Key := 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64';
  Result := not (RegQueryDWordValue(HKLM, Key, 'Installed', Install) and (Install = 1));
end;

// Initialize wizard with custom styling
procedure InitializeWizard;
begin
  // Make welcome page more informative
  WizardForm.WelcomeLabel2.Caption := 
    'This will install MKS-70 Editor version {#MyAppVersion} on your computer.' + #13#10 + #13#10 +
    'MKS-70 Editor is a professional editor for Roland MKS-70 and JX-10 synthesizers. ' +
    'It provides real-time control over all synthesizer parameters via MIDI.' + #13#10 + #13#10 +
    'Features:' + #13#10 +
    '• Real-time parameter editing' + #13#10 +
    '• 300+ factory presets' + #13#10 +
    '• Librarian with AI analysis' + #13#10 +
    '• VST3 plugin for DAW integration' + #13#10 + #13#10 +
    'Click Next to continue.';
end;

// Post-installation tasks
procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
begin
  if CurStep = ssPostInstall then
  begin
    // Create installation marker
    SaveStringToFile(
      ExpandConstant('{app}\version.txt'),
      'MKS-70 Editor v{#MyAppVersion}' + #13#10 +
      'Installed: ' + GetDateTimeString('yyyy-mm-dd hh:nn:ss', '-', ':') + #13#10 +
      'Path: ' + ExpandConstant('{app}'),
      False
    );
    
    // Create presets directory if it doesn't exist
    ForceDirectories(ExpandConstant('{userappdata}\{#MyAppName}\Presets'));
  end;
end;

// Confirm uninstall
function InitializeUninstall: Boolean;
begin
  Result := MsgBox(
    'Are you sure you want to uninstall MKS-70 Editor?' + #13#10 + #13#10 +
    'Your presets in the user folder will be preserved.',
    mbConfirmation, MB_YESNO
  ) = IDYES;
end;

[Messages]
WelcomeLabel1=Welcome to MKS-70 Editor
WelcomeLabel2=Professional editor for Roland MKS-70 and JX-10 synthesizers
FinishedHeadingLabel=Installation Complete!
FinishedLabel=MKS-70 Editor has been installed on your computer.%n%nConnect your Roland MKS-70 or JX-10 via MIDI to start editing!

[CustomMessages]
english.LaunchProgram=Launch MKS-70 Editor
hebrew.LaunchProgram=הפעל את MKS-70 Editor
english.CreateDesktopIcon=Create a &desktop icon
hebrew.CreateDesktopIcon=צור סמל על שולחן העבודה
