; Requirements:
; Inno Setup QuickStart Pack 5.3.8 Unicode
;   http://www.jrsoftware.org/isdl.php#qsp


;If you want to compile the 64bit version, change the "is64bit" to "True"
#define is64bit = False
#define include_license = True
#define localize = True


;workaround since ISPP doesn't work with relative paths
#include "Installer/../../include/Version.h"

#define app_name "Media Player Classic - Home Cinema"
#define app_version str(VERSION_MAJOR) + "." + str(VERSION_MINOR) + "." + str(VERSION_REV) + "." + str(VERSION_PATCH)
#define app_url "http://mpc-hc.sourceforge.net/"

;workaround in order to be able to build the 64bit installer through cmd; we define Buildx64=True for that.
#ifdef Buildx64
#define is64bit = True
#endif
#if is64bit
#define mpchc_exe = 'mpc-hc64.exe'
#define mpchc_ini = 'mpc-hc64.ini'
#else
#define mpchc_exe = 'mpc-hc.exe'
#define mpchc_ini = 'mpc-hc.ini'
#endif


[Setup]
#if is64bit
AppId={{2ACBF1FA-F5C3-4B19-A774-B22A31F231B9}
DefaultGroupName=Media Player Classic - Home Cinema x64
UninstallDisplayName={#app_name} v{#app_version} x64
OutputBaseFilename=MPC-HomeCinema.{#app_version}.x64
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#else
AppId={{2624B969-7135-4EB1-B0F6-2D8C397B45F7}
DefaultGroupName=Media Player Classic - Home Cinema
UninstallDisplayName={#app_name} v{#app_version}
OutputBaseFilename=MPC-HomeCinema.{#app_version}.x86
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
VersionInfoCopyright=Copyright © 2002-2010, see AUTHORS file
VersionInfoDescription={#app_name} {#app_version} Setup
VersionInfoTextVersion={#app_version}
VersionInfoVersion={#app_version}
VersionInfoProductName={#app_name}
VersionInfoProductVersion={#app_version}
VersionInfoProductTextVersion={#app_version}
UninstallDisplayIcon={app}\{#mpchc_exe}
DefaultDirName={code:GetInstallFolder}

#if include_license
LicenseFile=..\COPYING
#endif

OutputDir=Installer
SetupIconFile=..\src\apps\mplayerc\res\icon.ico
WizardImageFile=Images\WizardImageLarge.bmp
WizardSmallImageFile=Images\WizardImageSmall.bmp
Compression=lzma/ultra
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
Name: cz; MessagesFile: compiler:Languages\Czech.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl
Name: fr; MessagesFile: compiler:Languages\French.isl
Name: hu; MessagesFile: compiler:Languages\Hungarian.isl
Name: it; MessagesFile: compiler:Languages\Italian.isl
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
#include "custom_messages.iss"
#endif


[CustomMessages]
;tsk=Task, msg=Message
en.langid=00000000
en.tsk_AllUsers=For all users
en.tsk_CurrentUser=For the current user only
en.tsk_Other=Other tasks:
en.tsk_ResetSettings=Reset settings
en.msg_DeleteSettings=Do you also want to delete MPC-HC settings? %nIf you plan on installing MPC-HC again then you do not have to delete them.


[Messages]
BeveledLabel={#app_name} v{#app_version}


[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}
Name: desktopicon\user; Description: {cm:tsk_CurrentUser}; GroupDescription: {cm:AdditionalIcons}; Flags: exclusive
Name: desktopicon\common; Description: {cm:tsk_AllUsers}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked exclusive
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; OnlyBelowVersion: 0,6.01; Flags: unchecked
Name: reset_settings; Description: {cm:tsk_ResetSettings}; GroupDescription: {cm:tsk_Other}; Check: SettingsExistCheck(); Flags: checkedonce unchecked


[Files]
#if is64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpc-hc64.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpciconlib.dll; DestDir: {app}; Flags: ignoreversion
#if localize
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.??.dll; DestDir: {app}; Flags: ignoreversion
#endif
#else
Source: ..\src\apps\mplayerc\Release Unicode\mpc-hc.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpciconlib.dll; DestDir: {app}; Flags: ignoreversion
#if localize
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.??.dll; DestDir: {app}; Flags: ignoreversion
#endif
#endif

Source: ..\src\apps\mplayerc\AUTHORS; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\ChangeLog; DestDir: {app}; Flags: ignoreversion
Source: ..\COPYING; DestDir: {app}; Flags: ignoreversion


[Registry]
Root: HKCU; Subkey: Software\Gabest\Media Player Classic; Tasks: reset_settings; Flags: deletekey uninsdeletekeyifempty


[Run]
Filename: {app}\{#mpchc_exe}; Description: {cm:LaunchProgram,{#app_name}}; Flags: nowait postinstall skipifsilent unchecked


[Icons]
#if is64bit
Name: {group}\{#app_name} x64; Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}
Name: {commondesktop}\{#app_name} x64; Filename: {app}\{#mpchc_exe}; Tasks: desktopicon\common; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {userdesktop}\{#app_name} x64; Filename: {app}\{#mpchc_exe}; Tasks: desktopicon\user; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#app_name} x64; Filename: {app}\{#mpchc_exe}; Tasks: quicklaunchicon; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}
#else
Name: {group}\{#app_name}; Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {commondesktop}\{#app_name}; Filename: {app}\{#mpchc_exe}; Tasks: desktopicon\common; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {userdesktop}\{#app_name}; Filename: {app}\{#mpchc_exe}; Tasks: desktopicon\user; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#app_name}; Filename: {app}\{#mpchc_exe}; Tasks: quicklaunchicon; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
#endif
Name: {group}\{cm:ProgramOnTheWeb,{#app_name}}; Filename: {#app_url}
Name: {group}\{cm:UninstallProgram,{#app_name}}; Filename: {uninstallexe}; Comment: {cm:UninstallProgram,{#app_name}}; WorkingDir: {app}


[InstallDelete]
Type: files; Name: {userdesktop}\{#app_name}.lnk; Check: NOT IsTaskSelected('desktopicon\user')
Type: files; Name: {commondesktop}\{#app_name}.lnk; Check: NOT IsTaskSelected('desktopicon\common')
Type: files; Name: {app}\{#mpchc_ini}; Tasks: reset_settings
Type: files; Name: {app}\*.bak; Tasks: reset_settings


[Code]
// Check if MPC-HC's settings exist
function SettingsExistCheck(): Boolean;
begin
  Result := False;
  if RegKeyExists(HKEY_CURRENT_USER, 'Software\Gabest\Media Player Classic') OR FileExists(ExpandConstant('{app}\{#mpchc_ini}')) then
  Result := True;
end;

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


procedure CurStepChanged(CurStep: TSetupStep);
Var
  lang : Integer;
begin
  if CurStep = ssDone then begin
    lang := StrToInt(ExpandConstant('{cm:langid}'));
    if FileExists(ExpandConstant('{app}\{#mpchc_ini}')) then
      SetIniInt('Settings', 'InterfaceLanguage', lang, ExpandConstant('{app}\{#mpchc_ini}'))
    else
      RegWriteDWordValue(HKCU, 'Software\Gabest\Media Player Classic\Settings', 'InterfaceLanguage', lang);
  end;
end;


procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  // When uninstalling, ask the user to delete MPC-HC settings
  if CurUninstallStep = usUninstall then begin
    if SettingsExistCheck() then begin
      if MsgBox(ExpandConstant('{cm:msg_DeleteSettings}'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then begin
        DeleteFile(ExpandConstant('{userappdata}\Media Player Classic\default.mpcpl'));
        RemoveDir(ExpandConstant('{userappdata}\Media Player Classic'));
        RegDeleteKeyIncludingSubkeys(HKCU, 'Software\Gabest\Media Player Classic');
        RegDeleteKeyIncludingSubkeys(HKLM, 'SOFTWARE\Gabest\Media Player Classic');
        RegDeleteKeyIfEmpty(HKCU, 'SOFTWARE\Gabest');
        RegDeleteKeyIfEmpty(HKLM, 'SOFTWARE\Gabest');
      end;
    end;
  end;
end;
