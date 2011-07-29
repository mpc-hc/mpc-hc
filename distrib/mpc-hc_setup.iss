; $Id$
;
; (C) 2009-2011 see AUTHORS
;
; This file is part of MPC-HC.
;
; MPC-HC is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 3 of the License, or
; (at your option) any later version.
;
; MPC-HC is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.



; Requirements:
; Inno Setup Unicode: http://www.jrsoftware.org/isdl.php


;If you want to compile the 64bit version define "x64build" (uncomment the define below or use build_2010.bat)
#define localize
;#define x64Build

;Don't forget to update the DirectX SDK number in include\Version.h (not updated so often)


;From now on you shouldn't need to change anything

#if VER < 0x05040200
  #error Update your Inno Setup version
#endif

#define ISPP_IS_BUGGY
#include "..\include\Version.h"

#define copyright_year "2002-2011"
#define app_name       "Media Player Classic - Home Cinema"
#define app_version    str(MPC_VERSION_MAJOR) + "." + str(MPC_VERSION_MINOR) + "." + str(MPC_VERSION_PATCH) + "." + str(MPC_VERSION_REV)


#ifdef x64Build
  #define mpchc_exe     = "mpc-hc64.exe"
  #define mpchc_ini     = "mpc-hc64.ini"
#else
  #define mpchc_exe     = "mpc-hc.exe"
  #define mpchc_ini     = "mpc-hc.ini"
#endif

#define bindir        = "..\bin10"
#define sse_required


[Setup]
#ifdef x64Build
AppId={{2ACBF1FA-F5C3-4B19-A774-B22A31F231B9}
DefaultGroupName={#app_name} x64
OutputBaseFilename=MPC-HomeCinema.{#app_version}.x64
UninstallDisplayName={#app_name} v{#app_version} x64
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#else
AppId={{2624B969-7135-4EB1-B0F6-2D8C397B45F7}
DefaultGroupName={#app_name}
OutputBaseFilename=MPC-HomeCinema.{#app_version}.x86
UninstallDisplayName={#app_name} v{#app_version}
#endif

AppName={#app_name}
AppVersion={#app_version}
AppVerName={#app_name} v{#app_version}
AppPublisher=MPC-HC Team
AppPublisherURL=http://mpc-hc.sourceforge.net/
AppSupportURL=http://mpc-hc.sourceforge.net/
AppUpdatesURL=http://mpc-hc.sourceforge.net/
AppContact=http://mpc-hc.sourceforge.net/
AppCopyright=Copyright © {#copyright_year} all contributors, see AUTHORS file
VersionInfoCompany=MPC-HC Team
VersionInfoCopyright=Copyright © {#copyright_year}, MPC-HC Team
VersionInfoDescription={#app_name} Setup
VersionInfoProductName={#app_name}
VersionInfoProductVersion={#app_version}
VersionInfoProductTextVersion={#app_version}
VersionInfoTextVersion={#app_version}
VersionInfoVersion={#app_version}
UninstallDisplayIcon={app}\{#mpchc_exe}
DefaultDirName={code:GetInstallFolder}
LicenseFile=..\COPYING.txt
OutputDir=.
SetupIconFile=..\src\apps\mplayerc\res\icon.ico
AppReadmeFile={app}\Readme.txt
WizardImageFile=WizardImageFile.bmp
WizardSmallImageFile=WizardSmallImageFile.bmp
Compression=lzma/ultra
SolidCompression=yes
EnableDirDoesntExistWarning=no
AllowNoIcons=yes
ShowTasksTreeLines=yes
AlwaysShowDirOnReadyPage=yes
AlwaysShowGroupOnReadyPage=yes
ShowLanguageDialog=yes
DisableDirPage=auto
DisableProgramGroupPage=auto
MinVersion=0,5.01.2600sp3
AppMutex=MediaPlayerClassicW


[Languages]
Name: en; MessagesFile: compiler:Default.isl

#ifdef localize
Name: br; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: by; MessagesFile: Languages\Belarusian.isl
Name: ca; MessagesFile: compiler:Languages\Catalan.isl
Name: cz; MessagesFile: compiler:Languages\Czech.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl
Name: fr; MessagesFile: compiler:Languages\French.isl
Name: he; MessagesFile: compiler:Languages\Hebrew.isl
Name: hu; MessagesFile: Languages\Hungarian.isl
Name: hy; MessagesFile: Languages\Armenian.islu
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
Name: default;            Description: {cm:types_DefaultInstallation}
Name: custom;             Description: {cm:types_CustomInstallation};                      Flags: iscustom


[Components]
Name: main;               Description: {#app_name} v{#app_version}; Types: default custom; Flags: fixed
Name: mpciconlib;         Description: {cm:comp_mpciconlib};        Types: default custom
#ifdef localize
Name: mpcresources;       Description: {cm:comp_mpcresources};      Types: default custom
#endif


[Tasks]
Name: desktopicon;        Description: {cm:CreateDesktopIcon};     GroupDescription: {cm:AdditionalIcons}
Name: desktopicon\user;   Description: {cm:tsk_CurrentUser};       GroupDescription: {cm:AdditionalIcons};                              Flags: exclusive
Name: desktopicon\common; Description: {cm:tsk_AllUsers};          GroupDescription: {cm:AdditionalIcons};                              Flags: unchecked exclusive
Name: quicklaunchicon;    Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; OnlyBelowVersion: 0,6.01;    Flags: unchecked
Name: reset_settings;     Description: {cm:tsk_ResetSettings};     GroupDescription: {cm:tsk_Other};       Check: SettingsExistCheck(); Flags: checkedonce unchecked


[Files]
; For CPU detection
#if defined(sse_required) || defined(sse2_required)
Source: WinCPUID.dll;                             DestDir: {tmp};                           Flags: dontcopy noencryption
#endif

#ifdef x64Build
Source: {#bindir}\mpc-hc_x64\mpc-hc64.exe;        DestDir: {app}; Components: main;         Flags: ignoreversion
Source: {#bindir}\mpc-hc_x64\mpciconlib.dll;      DestDir: {app}; Components: mpciconlib;   Flags: ignoreversion
#ifdef localize
Source: {#bindir}\mpc-hc_x64\mpcresources.??.dll; DestDir: {app}; Components: mpcresources; Flags: ignoreversion
#endif
#else
Source: {#bindir}\mpc-hc_x86\mpc-hc.exe;          DestDir: {app}; Components: main;         Flags: ignoreversion
Source: {#bindir}\mpc-hc_x86\mpciconlib.dll;      DestDir: {app}; Components: mpciconlib;   Flags: ignoreversion
#ifdef localize
Source: {#bindir}\mpc-hc_x86\mpcresources.??.dll; DestDir: {app}; Components: mpcresources; Flags: ignoreversion
#endif
#endif

Source: ..\docs\Authors.txt;                      DestDir: {app}; Components: main;         Flags: ignoreversion
Source: ..\docs\Changelog.txt;                    DestDir: {app}; Components: main;         Flags: ignoreversion
Source: ..\COPYING.txt;                           DestDir: {app}; Components: main;         Flags: ignoreversion
Source: ..\docs\Readme.txt;                       DestDir: {app}; Components: main;         Flags: ignoreversion


[Icons]
#ifdef x64Build
Name: {group}\{#app_name} x64;                   Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {commondesktop}\{#app_name} x64;           Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: desktopicon\common
Name: {userdesktop}\{#app_name} x64;             Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: desktopicon\user
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#app_name} x64;  Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version} x64; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: quicklaunchicon
#else
Name: {group}\{#app_name};                       Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {commondesktop}\{#app_name};               Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: desktopicon\common
Name: {userdesktop}\{#app_name};                 Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: desktopicon\user
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#app_name};      Filename: {app}\{#mpchc_exe}; Comment: {#app_name} v{#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: quicklaunchicon
#endif
Name: {group}\Changelog;                         Filename: {app}\Changelog.txt; Comment: {cm:ViewChangelog};                WorkingDir: {app}
Name: {group}\{cm:ProgramOnTheWeb,{#app_name}};  Filename: http://mpc-hc.sourceforge.net/
Name: {group}\{cm:UninstallProgram,{#app_name}}; Filename: {uninstallexe};      Comment: {cm:UninstallProgram,{#app_name}}; WorkingDir: {app}


[Run]
Filename: {app}\{#mpchc_exe};                    Description: {cm:LaunchProgram,{#app_name}}; WorkingDir: {app}; Flags: nowait postinstall skipifsilent unchecked
Filename: {app}\Changelog.txt;                   Description: {cm:ViewChangelog};             WorkingDir: {app}; Flags: shellexec nowait postinstall skipifsilent unchecked


[InstallDelete]
Type: files; Name: {userdesktop}\{#app_name}.lnk;   Check: NOT IsTaskSelected('desktopicon\user')   AND IsUpdate()
Type: files; Name: {commondesktop}\{#app_name}.lnk; Check: NOT IsTaskSelected('desktopicon\common') AND IsUpdate()
Type: files; Name: {app}\AUTHORS;                   Check: IsUpdate()
Type: files; Name: {app}\ChangeLog;                 Check: IsUpdate()
Type: files; Name: {app}\COPYING;                   Check: IsUpdate()


[Code]
// CPU detection functions
#if defined(sse_required) || defined(sse2_required)
#include "innosetup_cpu_detection.iss"
#endif

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
  if FileExists(ExpandConstant('{sys}\D3DX9_{#DIRECTX_SDK_NUMBER}.dll')) then begin
    Result := True;
  end else
    Result := False;
end;


function IsUpdate(): Boolean;
begin
  Result := is_update;
end;


// Check if MPC-HC's settings exist
function SettingsExistCheck(): Boolean;
begin
  if RegKeyExists(HKEY_CURRENT_USER, 'Software\Gabest\Media Player Classic') OR FileExists(ExpandConstant('{app}\{#mpchc_ini}')) then begin
    Result := True;
  end else
    Result := False;
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
      SuppressibleMsgBox(ExpandConstant('{cm:msg_NoD3DX9DLL_found}'), mbCriticalError, MB_OK, MB_OK);
    end;
  end;

end;


procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  // When uninstalling, ask the user to delete MPC-HC settings
  if CurUninstallStep = usUninstall then begin
    if SettingsExistCheck() then begin
      if SuppressibleMsgBox(ExpandConstant('{cm:msg_DeleteSettings}'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2, IDNO) = IDYES then begin
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
      SuppressibleMsgBox(ExpandConstant('{cm:msg_SetupIsRunningWarning}'), mbError, MB_OK, MB_OK);
      Result := False;
  end else begin
    CreateMutex(installer_mutex_name);

#if defined(sse_required) || defined(sse2_required)
  // Acquire CPU information
  CPUCheck;

#if defined(sse2_required)
  if Result AND NOT Is_SSE2_Supported() then begin
    Result := False;
    SuppressibleMsgBox(CustomMessage('msg_simd_sse2'), mbError, MB_OK, MB_OK);
  end;
#elif defined(sse_required)
  if Result AND NOT Is_SSE_Supported() then begin
    Result := False;
    SuppressibleMsgBox(CustomMessage('msg_simd_sse'), mbError, MB_OK, MB_OK);
  end;
  #endif

#endif

  #ifdef x64Build
    is_update := RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{2ACBF1FA-F5C3-4B19-A774-B22A31F231B9}_is1');
  #else
    is_update := RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{2624B969-7135-4EB1-B0F6-2D8C397B45F7}_is1');
  #endif

  end;
end;

