; MKS-70 Editor - Inno Setup Installation Script
; Roland MKS-70 / JX-10 Synthesizer Editor
; Created: November 2025

#define MyAppName "MKS-70 Editor"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "MKS-70 Editor Development"
#define MyAppURL "https://github.com/yourusername/mks70-editor"
#define MyAppExeName "MKS70 Editor.exe"

[Setup]
; Basic Information
AppId={{A8F3C2B1-9D4E-4A5C-B6E7-8F9A0B1C2D3E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

; Installation Directories
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes

; Output Configuration
OutputDir=..\installer\output
OutputBaseFilename=MKS70_Editor_Setup_v{#MyAppVersion}
; SetupIconFile=..\artifacts\icon.ico
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern

; System Requirements
MinVersion=10.0
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

; Privileges
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog

; Uninstall
UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName={#MyAppName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Main Application (Standalone)
Source: "..\Builds\VisualStudio2022\x64\Release\Standalone Plugin\MKS70 Editor.exe"; DestDir: "{app}"; Flags: ignoreversion

; VST3 Plugin (if exists)
Source: "..\Builds\VisualStudio2022\x64\Release\VST3\MKS70 Editor.vst3\*"; DestDir: "{commoncf64}\VST3\MKS70 Editor.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: VST3Exists

; Documentation
Source: "..\README.md"; DestDir: "{app}"; DestName: "README.txt"; Flags: ignoreversion isreadme
Source: "..\TESTING.md"; DestDir: "{app}\docs"; DestName: "TESTING.txt"; Flags: ignoreversion

; Presets - All Categories (300+ presets)
Source: "..\Presets\Bass\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Bass"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Bells\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Bells"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Brass\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Brass"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Drums\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Drums"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\FX\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\FX"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Keys\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Keys"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Lead\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Lead"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Organ\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Organ"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Other\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Other"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Pad\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Pad"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Strings\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Strings"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Synth\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Synth"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Presets\Banks\*.syx"; DestDir: "{userappdata}\{#MyAppName}\Presets\Banks"; Flags: ignoreversion recursesubdirs createallsubdirs

; MIDI Mapping Configuration
Source: "..\mks70_mapping.json"; DestDir: "{userappdata}\{#MyAppName}"; Flags: ignoreversion

[Icons]
; Start Menu
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Presets Folder"; Filename: "{userappdata}\{#MyAppName}\Presets"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

; Desktop Icon (optional)
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

; Quick Launch (optional)
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
; Launch application after installation
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
// Check if VST3 build exists
function VST3Exists: Boolean;
begin
  Result := DirExists(ExpandConstant('..\Builds\VisualStudio2022\x64\Release\VST3'));
end;

// Custom welcome message
procedure InitializeWizard;
var
  WelcomeLabel: TNewStaticText;
begin
  WelcomeLabel := TNewStaticText.Create(WizardForm);
  WelcomeLabel.Parent := WizardForm.WelcomePage;
  WelcomeLabel.Caption := 
    'This will install MKS-70 Editor - a professional editor for Roland MKS-70 and JX-10 synthesizers.' + #13#10 + #13#10 +
    'The installer includes:' + #13#10 +
    '• Standalone Application' + #13#10 +
    '• VST3 Plugin' + #13#10 +
    '• 700+ Factory and Custom Presets' + #13#10 +
    '• Complete MIDI SysEx Support' + #13#10 +
    '• Real-time Parameter Control';
  WelcomeLabel.AutoSize := False;
  WelcomeLabel.Left := ScaleX(176);
  WelcomeLabel.Top := ScaleY(64);
  WelcomeLabel.Width := ScaleX(200);
  WelcomeLabel.Height := ScaleY(150);
  WelcomeLabel.WordWrap := True;
end;

// Post-installation message
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // Create a marker file indicating successful installation
    SaveStringToFile(ExpandConstant('{app}\installed.txt'), 
      'MKS-70 Editor v' + '{#MyAppVersion}' + ' installed successfully!' + #13#10 +
      'Installation Date: ' + GetDateTimeString('yyyy-mm-dd hh:nn:ss', '-', ':'), 
      False);
  end;
end;

[UninstallDelete]
; Clean up user data on uninstall (optional - commented out to preserve user presets)
; Type: filesandordirs; Name: "{userappdata}\{#MyAppName}"

[Messages]
WelcomeLabel1=Welcome to MKS-70 Editor Setup
WelcomeLabel2=This will install [name/ver] on your computer.%n%nConnect your Roland MKS-70 or JX-10 synthesizer via MIDI to start editing sounds in real-time!

[CustomMessages]
english.LaunchProgram=Launch MKS-70 Editor after installation
hebrew.LaunchProgram=הפעל את MKS-70 Editor לאחר ההתקנה
