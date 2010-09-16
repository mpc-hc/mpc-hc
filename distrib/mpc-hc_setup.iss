; Requirements:
; Inno Setup QuickStart Pack 5.3.10(+) Unicode
;   http://www.jrsoftware.org/isdl.php#qsp
;
; $Id$


;If you want to compile the 64bit version, change the "is64bit" to "True"
#define is64bit = False
#define include_license = True
#define localize = True
;If you want to compile the MSVC2010 build installer, change the "VS2010" to "True"
#define VS2010 = False

;Don't forget to update the DirectX SDK number (not updated so often)
#define DIRECTX_SDK_NUMBER = 43


;workaround since ISPP doesn't work with relative paths
#include "Installer\..\..\include\Version.h"

#define app_name "Media Player Classic - Home Cinema"
#define app_version str(VERSION_MAJOR) + "." + str(VERSION_MINOR) + "." + str(VERSION_REV) + "." + str(VERSION_PATCH)
#define app_url "http://mpc-hc.sourceforge.net/"


;workaround in order to be able to build the 64bit installer through cmd; we define Buildx64=True for that.
#ifdef Buildx64
#define is64bit = True
#endif

;workaround in order to be able to build the MSVC2010 installer through cmd; we define VS2010build=True for that.
#ifdef VS2010build
#define VS2010 = True
#endif

#if is64bit
#define mpchc_exe = 'mpc-hc64.exe'
#define mpchc_ini = 'mpc-hc64.ini'
#else
#define mpchc_exe = 'mpc-hc.exe'
#define mpchc_ini = 'mpc-hc.ini'
#endif

#if VS2010
#define bindir = '..\bin10'
#define sse_required = False
#define sse2_required = True
#else
#define bindir = '..\bin'
#define sse_required = True
#define sse2_required = False
#endif


[Setup]
#if is64bit
AppId={{2ACBF1FA-F5C3-4B19-A774-B22A31F231B9}
DefaultGroupName={#app_name} x64
UninstallDisplayName={#app_name} v{#app_version} x64
#if VS2010
OutputBaseFilename=MPC-HomeCinema.{#app_version}.x64.MSVC2010
#else
OutputBaseFilename=MPC-HomeCinema.{#app_version}.x64
#endif
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#else
AppId={{2624B969-7135-4EB1-B0F6-2D8C397B45F7}
DefaultGroupName={#app_name}
UninstallDisplayName={#app_name} v{#app_version}
#if VS2010
OutputBaseFilename=MPC-HomeCinema.{#app_version}.x86.MSVC2010
#else
OutputBaseFilename=MPC-HomeCinema.{#app_version}.x86
#endif
#endif

AppName={#app_name}
AppVersion={#app_version}
AppVerName={#app_name} v{#app_version}
AppPublisher=MPC-HC Team
AppPublisherURL={#app_url}
AppSupportURL={#app_url}
AppUpdatesURL={#app_url}
AppContact={#app_url}
AppCopyright=Copyright © 2002-2010, see AUTHORS file
VersionInfoCompany=MPC-HC Team
VersionInfoCopyright=Copyright © 2002-2010, MPC-HC Team
VersionInfoDescription={#app_name} {#app_version} Setup
VersionInfoTextVersion={#app_version}
VersionInfoVersion={#app_version}
VersionInfoProductName={#app_name}
VersionInfoProductVersion={#app_version}
VersionInfoProductTextVersion={#app_version}
UninstallDisplayIcon={app}\{#mpchc_exe}
DefaultDirName={code:GetInstallFolder}

#if include_license
LicenseFile=..\COPYING.txt
#endif

OutputDir=.
SetupIconFile=..\src\apps\mplayerc\res\icon.ico
WizardImageFile=WizardImageFile.bmp
WizardSmallImageFile=WizardSmallImageFile.bmp
Compression=lzma/ultra
InternalCompressLevel=ultra
SolidCompression=yes
DirExistsWarning=no
EnableDirDoesntExistWarning=no
AllowNoIcons=yes
ShowTasksTreeLines=yes
AlwaysShowDirOnReadyPage=yes
AlwaysShowGroupOnReadyPage=yes
ShowLanguageDialog=yes
DisableDirPage=auto
DisableProgramGroupPage=auto
MinVersion=0,5.01.2600
AppMutex=MediaPlayerClassicW


[Languages]
Name: en; MessagesFile: compiler:Default.isl

#if localize
Name: br; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: by; MessagesFile: Languages\Belarus.isl
Name: ca; MessagesFile: compiler:Languages\Catalan.isl
Name: cz; MessagesFile: compiler:Languages\Czech.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl
Name: fr; MessagesFile: compiler:Languages\French.isl
Name: hu; MessagesFile: Languages\Hungarian.isl
Name: it; MessagesFile: compiler:Languages\Italian.isl
Name: ja; MessagesFile: compiler:Languages\Japanese.isl
Name: kr; MessagesFile: Languages\Korean.isl
Name: nl; MessagesFile: compiler:Languages\Dutch.isl
Name: pl; MessagesFile: compiler:Languages\Polish.isl
Name: ru; MessagesFile: compiler:Languages\Russian.isl
Name: sc; MessagesFile: Languages\ChineseSimp.isl
Name: se; MessagesFile: Languages\Swedish.isl
Name: sk; MessagesFile: compiler:Languages\Slovak.isl
Name: tc; MessagesFile: Languages\ChineseTrad.isl
Name: tr; MessagesFile: Languages\Turkish.isl
Name: ua; MessagesFile: Languages\Ukrainian.isl
#endif

; Include installer's custom messages
#include "custom_messages.iss"


[Messages]
BeveledLabel={#app_name} v{#app_version}


[Types]
Name: default; Description: {cm:types_DefaultInstallation}
Name: custom; Description: {cm:types_CustomInstallation}; Flags: iscustom


[Components]
Name: main; Description: {#app_name} v{#app_version}; Types: default custom; Flags: fixed
Name: mpciconlib; Description: {cm:comp_mpciconlib}; Types: default custom
#if localize
Name: mpcresources; Description: {cm:comp_mpcresources}; Types: default custom
#endif


[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}
Name: desktopicon\user; Description: {cm:tsk_CurrentUser}; GroupDescription: {cm:AdditionalIcons}; Flags: exclusive
Name: desktopicon\common; Description: {cm:tsk_AllUsers}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked exclusive
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; OnlyBelowVersion: 0,6.01; Flags: unchecked
Name: reset_settings; Description: {cm:tsk_ResetSettings}; GroupDescription: {cm:tsk_Other}; Check: SettingsExistCheck(); Flags: checkedonce unchecked


[Files]
#if is64bit
Source: {#bindir}\mpc-hc_x64\mpc-hc64.exe; DestDir: {app}; Components: main; Flags: ignoreversion
Source: {#bindir}\mpc-hc_x64\mpciconlib.dll; DestDir: {app}; Components: mpciconlib; Flags: ignoreversion
#if localize
Source: {#bindir}\mpc-hc_x64\mpcresources.??.dll; DestDir: {app}; Components: mpcresources; Flags: ignoreversion
#endif
#else
Source: {#bindir}\mpc-hc_x86\mpc-hc.exe; DestDir: {app}; Components: main; Flags: ignoreversion
Source: {#bindir}\mpc-hc_x86\mpciconlib.dll; DestDir: {app}; Components: mpciconlib; Flags: ignoreversion
#if localize
Source: {#bindir}\mpc-hc_x86\mpcresources.??.dll; DestDir: {app}; Components: mpcresources; Flags: ignoreversion
#endif
#endif

Source: ..\src\apps\mplayerc\Authors.txt; DestDir: {app}; Components: main; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Changelog.txt; DestDir: {app}; Components: main; Flags: ignoreversion
Source: ..\COPYING.txt; DestDir: {app}; Components: main; Flags: ignoreversion


[Run]
Filename: {app}\{#mpchc_exe}; Description: {cm:LaunchProgram,{#app_name}}; WorkingDir: {app}; Flags: nowait postinstall skipifsilent unchecked
Filename: {app}\Changelog.txt; Description: {cm:ViewChangelog}; WorkingDir: {app}; Flags: shellexec nowait postinstall skipifsilent unchecked


[Icons]
#if is64bit
Name: {group}\{#app_name} x64; Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {commondesktop}\{#app_name} x64; Filename: {app}\{#mpchc_exe}; Tasks: desktopicon\common; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {userdesktop}\{#app_name} x64; Filename: {app}\{#mpchc_exe}; Tasks: desktopicon\user; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#app_name} x64; Filename: {app}\{#mpchc_exe}; Tasks: quicklaunchicon; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
#else
Name: {group}\{#app_name}; Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {commondesktop}\{#app_name}; Filename: {app}\{#mpchc_exe}; Tasks: desktopicon\common; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {userdesktop}\{#app_name}; Filename: {app}\{#mpchc_exe}; Tasks: desktopicon\user; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#app_name}; Filename: {app}\{#mpchc_exe}; Tasks: quicklaunchicon; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
#endif
Name: {group}\Changelog; Filename: {app}\Changelog.txt; Comment: {cm:ViewChangelog}; WorkingDir: {app}
Name: {group}\{cm:ProgramOnTheWeb,{#app_name}}; Filename: {#app_url}
Name: {group}\{cm:UninstallProgram,{#app_name}}; Filename: {uninstallexe}; Comment: {cm:UninstallProgram,{#app_name}}; WorkingDir: {app}


[InstallDelete]
Type: files; Name: {userdesktop}\{#app_name}.lnk; Check: NOT IsTaskSelected('desktopicon\user') AND IsUpdate()
Type: files; Name: {commondesktop}\{#app_name}.lnk; Check: NOT IsTaskSelected('desktopicon\common') AND IsUpdate()
Type: files; Name: {app}\AUTHORS; Check: IsUpdate()
Type: files; Name: {app}\ChangeLog; Check: IsUpdate()
Type: files; Name: {app}\COPYING; Check: IsUpdate()


[Code]
// CPU detection functions
#include "innosetup_cpu_detection.iss"

// Global variables and constants
const installer_mutex_name = 'mpchc_setup_mutex';
var
  is_update: Boolean;


function GetInstallFolder(Default: String): String;
var
  InstallPath: String;
begin
  if NOT RegQueryStringValue(HKLM, 'SOFTWARE\Gabest\Media Player Classic', 'ExePath', InstallPath) then begin
    Result := ExpandConstant('{pf}\Media Player Classic - Home Cinema');
  end else begin
  RegQueryStringValue(HKLM, 'SOFTWARE\Gabest\Media Player Classic', 'ExePath', InstallPath)
    Result := ExtractFileDir(InstallPath);
    if (Result = '') OR NOT DirExists(Result) then begin
      Result := ExpandConstant('{pf}\Media Player Classic - Home Cinema');
    end;
  end;
end;


function D3DX9DLLExists(): Boolean;
begin
  Result := False;
  if FileExists(ExpandConstant('{sys}\D3DX9_{#DIRECTX_SDK_NUMBER}.dll')) then
  Result := True;
end;


function IsUpdate(): Boolean;
begin
  Result := is_update;
end;


// Check if MPC-HC's settings exist
function SettingsExistCheck(): Boolean;
begin
  Result := False;
  if RegKeyExists(HKEY_CURRENT_USER, 'Software\Gabest\Media Player Classic') OR FileExists(ExpandConstant('{app}\{#mpchc_ini}')) then
  Result := True;
end;


function ShouldSkipPage(PageID: Integer): Boolean;
begin
  if IsUpdate then begin
    Case PageID of
      // Hide the license page
      wpLicense: Result := True;
    else
      Result := False;
    end;
  end;
end;


procedure CleanUpSettingsAndFiles();
begin
  DeleteFile(ExpandConstant('{app}\*.bak'));
  DeleteFile(ExpandConstant('{app}\{#mpchc_ini}'));
  DeleteFile(ExpandConstant('{userappdata}\Media Player Classic\default.mpcpl'));
  RemoveDir(ExpandConstant('{userappdata}\Media Player Classic'));
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\Gabest\Filters');
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\Gabest\Media Player Classic');
  RegDeleteKeyIncludingSubkeys(HKLM, 'SOFTWARE\Gabest\Media Player Classic');
  RegDeleteKeyIfEmpty(HKCU, 'Software\Gabest');
  RegDeleteKeyIfEmpty(HKLM, 'SOFTWARE\Gabest');
end;


procedure CurStepChanged(CurStep: TSetupStep);
var
  lang : Integer;
begin
  if CurStep = ssPostInstall then begin
    if IsTaskSelected('reset_settings') then begin
      CleanUpSettingsAndFiles;
    end;
    lang := StrToInt(ExpandConstant('{cm:langid}'));
    RegWriteStringValue(HKLM, 'SOFTWARE\Gabest\Media Player Classic', 'ExePath', ExpandConstant('{app}\{#mpchc_exe}'));

    if IsComponentSelected ('mpcresources') then begin
      if FileExists(ExpandConstant('{app}\{#mpchc_ini}')) then
        SetIniInt('Settings', 'InterfaceLanguage', lang, ExpandConstant('{app}\{#mpchc_ini}'))
      else
        RegWriteDWordValue(HKCU, 'Software\Gabest\Media Player Classic\Settings', 'InterfaceLanguage', lang);
    end;

  end;

  if CurStep = ssDone then begin
    if NOT WizardSilent() AND NOT D3DX9DLLExists() then begin
      MsgBox(ExpandConstant('{cm:msg_NoD3DX9DLL_found}'), mbCriticalError, MB_OK)
    end;
  end;

end;


procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  // When uninstalling, ask the user to delete MPC-HC settings
  if CurUninstallStep = usUninstall then begin
    if SettingsExistCheck() then begin
      if MsgBox(ExpandConstant('{cm:msg_DeleteSettings}'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then begin
        CleanUpSettingsAndFiles;
      end;
    end;
  end;
end;


function InitializeSetup(): Boolean;
begin
  Result := True;
  // Create a mutex for the installer and if it's already running display a message and stop installation
  if CheckForMutexes(installer_mutex_name) then begin
    if NOT WizardSilent() then
      MsgBox(ExpandConstant('{cm:msg_SetupIsRunningWarning}'), mbError, MB_OK);
      Result := False;
  end else begin
    CreateMutex(installer_mutex_name);


  // Acquire CPU information
  CPUCheck;

  if NOT HasSupportedCPU() then begin
    Result := False;
    MsgBox(CustomMessage('msg_unsupported_cpu'), mbError, MB_OK);
  end;

  #if sse2_required
  if Result AND NOT Is_SSE2_Supported() then begin
    Result := False;
    MsgBox(CustomMessage('msg_simd_sse2'), mbError, MB_OK);
  end;
  #elif sse_required
  if Result AND NOT Is_SSE_Supported() then begin
    Result := False;
    MsgBox(CustomMessage('msg_simd_sse'), mbError, MB_OK);
  end;
  #endif


    #if is64bit
    is_update := RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{2ACBF1FA-F5C3-4B19-A774-B22A31F231B9}_is1');
    #else
    is_update := RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{2624B969-7135-4EB1-B0F6-2D8C397B45F7}_is1');
    #endif

  end;
end;


