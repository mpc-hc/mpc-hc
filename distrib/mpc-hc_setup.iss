; $Id$
;
; (C) 2009-2012 see Authors.txt
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


#if VER < EncodeVer(5,5,0)
  #error Update your Inno Setup version (5.5.0 or newer)
#endif

#ifndef UNICODE
  #error Use the Unicode Inno Setup
#endif

; Include translations by default. You can bypass this by defining localize=whatever or false etc in build.bat or here
#if !defined(localize)
  #if defined(MPCHC_LITE)
    #define localize = "false"
  #else
    #define localize = "true"
  #endif
#endif
#define sse_required
; If you want to compile the 64-bit version define "x64build" (uncomment the define below or use build.bat)
;#define x64Build
;#define MPCHC_LITE


; From now on you shouldn't need to change anything

#include "..\include\Version.h"

#define copyright_year "2002-2012"
#define app_name       "MPC-HC"
#define app_version    str(MPC_VERSION_MAJOR) + "." + str(MPC_VERSION_MINOR) + "." + str(MPC_VERSION_PATCH) + "." + str(MPC_VERSION_REV)
#define quick_launch   "{userappdata}\Microsoft\Internet Explorer\Quick Launch"


#ifdef x64Build
  #define bindir       = "..\bin\mpc-hc_x64"
  #define mpchc_exe    = "mpc-hc64.exe"
  #define mpchc_ini    = "mpc-hc64.ini"
  #define OutFilename  = app_name + "." + app_version + ".x64"
#else
  #define bindir       = "..\bin\mpc-hc_x86"
  #define mpchc_exe    = "mpc-hc.exe"
  #define mpchc_ini    = "mpc-hc.ini"
  #define OutFilename  = app_name + "." + app_version + ".x86"
#endif

#if localize != "true"
  #define OutFilename  = OutFilename + ".en"
#endif

#ifnexist bindir + "\" + mpchc_exe
  #error Compile MPC-HC first
#endif


[Setup]
#ifdef x64Build
AppId={{2ACBF1FA-F5C3-4B19-A774-B22A31F231B9}
DefaultGroupName={#app_name} x64
UninstallDisplayName={#app_name} {#app_version} (64-bit)
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#else
AppId={{2624B969-7135-4EB1-B0F6-2D8C397B45F7}
DefaultGroupName={#app_name}
UninstallDisplayName={#app_name} {#app_version}
#endif

AppName={#app_name}
AppVersion={#app_version}
AppVerName={#app_name} {#app_version}
AppPublisher=MPC-HC Team
AppPublisherURL=http://mpc-hc.sourceforge.net/
AppSupportURL=http://mpc-hc.sourceforge.net/
AppUpdatesURL=http://mpc-hc.sourceforge.net/
AppContact=http://mpc-hc.sourceforge.net/
AppCopyright=Copyright © {#copyright_year} all contributors, see Authors.txt
VersionInfoCompany=MPC-HC Team
VersionInfoCopyright=Copyright © {#copyright_year}, MPC-HC Team
VersionInfoDescription={#app_name} Setup
VersionInfoProductName={#app_name}
VersionInfoProductVersion={#app_version}
VersionInfoProductTextVersion={#app_version}
VersionInfoTextVersion={#app_version}
VersionInfoVersion={#app_version}
UninstallDisplayIcon={app}\{#mpchc_exe}
OutputBaseFilename={#OutFilename}
DefaultDirName={code:GetInstallFolder}
LicenseFile=..\COPYING.txt
OutputDir=.
SetupIconFile=..\src\mpc-hc\res\icon.ico
AppReadmeFile={app}\Readme.txt
WizardImageFile=WizardImageFile.bmp
WizardSmallImageFile=WizardSmallImageFile.bmp
Compression=lzma2/ultra
SolidCompression=yes
AllowNoIcons=yes
ShowTasksTreeLines=yes
DisableDirPage=auto
DisableProgramGroupPage=auto
MinVersion=5.01.2600sp3
AppMutex=MediaPlayerClassicW


[Languages]
Name: en; MessagesFile: compiler:Default.isl

#if localize == "true"
Name: br; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: by; MessagesFile: Languages\Belarusian.isl
Name: ca; MessagesFile: compiler:Languages\Catalan.isl
Name: cz; MessagesFile: compiler:Languages\Czech.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl
Name: eu; MessagesFile: compiler:Languages\Basque.isl
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
Name: sv; MessagesFile: Languages\Swedish.isl
Name: sk; MessagesFile: compiler:Languages\Slovak.isl
Name: tc; MessagesFile: Languages\ChineseTrad.isl
Name: tr; MessagesFile: Languages\Turkish.isl
Name: ua; MessagesFile: compiler:Languages\Ukrainian.isl
#endif

; Include installer's custom messages
#include "custom_messages.iss"


[Messages]
#ifdef x64Build
BeveledLabel={#app_name} {#app_version} (64-bit)
#else
BeveledLabel={#app_name} {#app_version}
#endif


[Types]
Name: default;            Description: {cm:types_DefaultInstallation}
Name: custom;             Description: {cm:types_CustomInstallation};                     Flags: iscustom


[Components]
Name: main;               Description: {#app_name} {#app_version}; Types: default custom; Flags: fixed
Name: mpciconlib;         Description: {cm:comp_mpciconlib};       Types: default custom
#if localize == "true"
Name: mpcresources;       Description: {cm:comp_mpcresources};     Types: default custom; Flags: disablenouninstallwarning
#endif


[Tasks]
Name: desktopicon;        Description: {cm:CreateDesktopIcon};     GroupDescription: {cm:AdditionalIcons}
Name: desktopicon\user;   Description: {cm:tsk_CurrentUser};       GroupDescription: {cm:AdditionalIcons}; Flags: exclusive
Name: desktopicon\common; Description: {cm:tsk_AllUsers};          GroupDescription: {cm:AdditionalIcons}; Flags: unchecked exclusive
Name: quicklaunchicon;    Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked;             OnlyBelowVersion: 6.01
Name: reset_settings;     Description: {cm:tsk_ResetSettings};     GroupDescription: {cm:tsk_Other};       Flags: checkedonce unchecked; Check: SettingsExistCheck()


[Files]
Source: {#bindir}\{#mpchc_exe};             DestDir: {app};      Components: main;         Flags: ignoreversion
Source: {#bindir}\mpciconlib.dll;           DestDir: {app};      Components: mpciconlib;   Flags: ignoreversion
Source: {#bindir}\D3DCompiler_{#MPC_DX_SDK_NUMBER}.dll; DestDir: {app}; Components: main;  Flags: ignoreversion
Source: {#bindir}\d3dx9_{#MPC_DX_SDK_NUMBER}.dll;       DestDir: {app}; Components: main;  Flags: ignoreversion
#if localize == "true"
Source: {#bindir}\Lang\mpcresources.br.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.by.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.ca.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.cz.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.de.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.es.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.eu.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.fr.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.he.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.hu.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.hy.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.it.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.ja.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.kr.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.nl.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.pl.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.ru.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.sc.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.sk.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.sv.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.tc.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.tr.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
Source: {#bindir}\Lang\mpcresources.ua.dll; DestDir: {app}\Lang; Components: mpcresources; Flags: ignoreversion
#endif
Source: ..\COPYING.txt;                     DestDir: {app};      Components: main;         Flags: ignoreversion
Source: ..\docs\Authors.txt;                DestDir: {app};      Components: main;         Flags: ignoreversion
Source: ..\docs\Changelog.txt;              DestDir: {app};      Components: main;         Flags: ignoreversion
Source: ..\docs\Readme.txt;                 DestDir: {app};      Components: main;         Flags: ignoreversion


[Icons]
#ifdef x64Build
Name: {group}\{#app_name} x64;                   Filename: {app}\{#mpchc_exe}; Comment: {#app_name} {#app_version} (64-bit); WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {commondesktop}\{#app_name} x64;           Filename: {app}\{#mpchc_exe}; Comment: {#app_name} {#app_version} (64-bit); WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: desktopicon\common
Name: {userdesktop}\{#app_name} x64;             Filename: {app}\{#mpchc_exe}; Comment: {#app_name} {#app_version} (64-bit); WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: desktopicon\user
Name: {#quick_launch}\{#app_name} x64;           Filename: {app}\{#mpchc_exe}; Comment: {#app_name} {#app_version} (64-bit); WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: quicklaunchicon
#else
Name: {group}\{#app_name};                       Filename: {app}\{#mpchc_exe}; Comment: {#app_name} {#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0
Name: {commondesktop}\{#app_name};               Filename: {app}\{#mpchc_exe}; Comment: {#app_name} {#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: desktopicon\common
Name: {userdesktop}\{#app_name};                 Filename: {app}\{#mpchc_exe}; Comment: {#app_name} {#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: desktopicon\user
Name: {#quick_launch}\{#app_name};               Filename: {app}\{#mpchc_exe}; Comment: {#app_name} {#app_version}; WorkingDir: {app}; IconFilename: {app}\{#mpchc_exe}; IconIndex: 0; Tasks: quicklaunchicon
#endif
Name: {group}\Changelog;                         Filename: {app}\Changelog.txt; Comment: {cm:ViewChangelog};                WorkingDir: {app}
Name: {group}\{cm:ProgramOnTheWeb,{#app_name}};  Filename: http://mpc-hc.sourceforge.net/
Name: {group}\{cm:UninstallProgram,{#app_name}}; Filename: {uninstallexe};      Comment: {cm:UninstallProgram,{#app_name}}; WorkingDir: {app}


[Run]
Filename: {app}\{#mpchc_exe};                    Description: {cm:LaunchProgram,{#app_name}}; WorkingDir: {app}; Flags: nowait postinstall skipifsilent unchecked
Filename: {app}\Changelog.txt;                   Description: {cm:ViewChangelog};             WorkingDir: {app}; Flags: nowait postinstall skipifsilent unchecked shellexec


[InstallDelete]
Type: files; Name: {userdesktop}\{#app_name}.lnk;   Check: not IsTaskSelected('desktopicon\user')   and IsUpgrade()
Type: files; Name: {commondesktop}\{#app_name}.lnk; Check: not IsTaskSelected('desktopicon\common') and IsUpgrade()
Type: files; Name: {#quick_launch}\{#app_name}.lnk; Check: not IsTaskSelected('quicklaunchicon')    and IsUpgrade(); OnlyBelowVersion: 6.01
Type: files; Name: {app}\AUTHORS
Type: files; Name: {app}\ChangeLog
Type: files; Name: {app}\COPYING

; old shortcuts
#ifdef x64Build
Type: files; Name: {group}\Media Player Classic - Home Cinema x64.lnk;               Check: IsUpgrade()
Type: files; Name: {commondesktop}\Media Player Classic - Home Cinema x64.lnk;       Check: IsUpgrade()
Type: files; Name: {userdesktop}\Media Player Classic - Home Cinema x64.lnk;         Check: IsUpgrade()
Type: files; Name: {#quick_launch}\Media Player Classic - Home Cinema x64.lnk;       Check: IsUpgrade()
#else
Type: files; Name: {group}\Media Player Classic - Home Cinema.lnk;                   Check: IsUpgrade()
Type: files; Name: {commondesktop}\Media Player Classic - Home Cinema.lnk;           Check: IsUpgrade()
Type: files; Name: {userdesktop}\Media Player Classic - Home Cinema.lnk;             Check: IsUpgrade()
Type: files; Name: {#quick_launch}\Media Player Classic - Home Cinema.lnk;           Check: IsUpgrade()
#endif
Type: files; Name: {group}\{cm:ProgramOnTheWeb,Media Player Classic - Home Cinema}.url;  Check: IsUpgrade()
Type: files; Name: {group}\{cm:UninstallProgram,Media Player Classic - Home Cinema}.lnk; Check: IsUpgrade()

Type: files; Name: {userdesktop}\Media Player Classic - Home Cinema.lnk;   Check: not IsTaskSelected('desktopicon\user')   and IsUpgrade()
Type: files; Name: {commondesktop}\Media Player Classic - Home Cinema.lnk; Check: not IsTaskSelected('desktopicon\common') and IsUpgrade()
Type: files; Name: {#quick_launch}\Media Player Classic - Home Cinema.lnk; Check: not IsTaskSelected('quicklaunchicon')    and IsUpgrade(); OnlyBelowVersion: 6.01


#if localize == "true"
; remove the old language dlls when upgrading
Type: files; Name: {app}\mpcresources.br.dll
Type: files; Name: {app}\mpcresources.by.dll
Type: files; Name: {app}\mpcresources.ca.dll
Type: files; Name: {app}\mpcresources.cz.dll
Type: files; Name: {app}\mpcresources.de.dll
Type: files; Name: {app}\mpcresources.es.dll
Type: files; Name: {app}\mpcresources.fr.dll
Type: files; Name: {app}\mpcresources.he.dll
Type: files; Name: {app}\mpcresources.hu.dll
Type: files; Name: {app}\mpcresources.hy.dll
Type: files; Name: {app}\mpcresources.it.dll
Type: files; Name: {app}\mpcresources.ja.dll
Type: files; Name: {app}\mpcresources.kr.dll
Type: files; Name: {app}\mpcresources.nl.dll
Type: files; Name: {app}\mpcresources.pl.dll
Type: files; Name: {app}\mpcresources.ru.dll
Type: files; Name: {app}\mpcresources.sc.dll
Type: files; Name: {app}\mpcresources.sk.dll
Type: files; Name: {app}\mpcresources.sv.dll
Type: files; Name: {app}\mpcresources.tc.dll
Type: files; Name: {app}\mpcresources.tr.dll
Type: files; Name: {app}\mpcresources.ua.dll
#endif


[Code]
#if defined(sse_required) || defined(sse2_required)
function IsProcessorFeaturePresent(Feature: Integer): Boolean;
external 'IsProcessorFeaturePresent@kernel32.dll stdcall';
#endif

const installer_mutex = 'mpchc_setup_mutex';


function GetInstallFolder(Default: String): String;
var
  sInstallPath: String;
begin
  if not RegQueryStringValue(HKLM, 'SOFTWARE\Gabest\Media Player Classic', 'ExePath', sInstallPath) then begin
    Result := ExpandConstant('{pf}\MPC-HC');
  end
  else begin
    RegQueryStringValue(HKLM, 'SOFTWARE\Gabest\Media Player Classic', 'ExePath', sInstallPath);
    Result := ExtractFileDir(sInstallPath);
    if (Result = '') or not DirExists(Result) then begin
      Result := ExpandConstant('{pf}\MPC-HC');
    end;
  end;
end;


#if defined(sse_required)
function Is_SSE_Supported(): Boolean;
begin
  // PF_XMMI_INSTRUCTIONS_AVAILABLE
  Result := IsProcessorFeaturePresent(6);
end;

#elif defined(sse2_required)

function Is_SSE2_Supported(): Boolean;
begin
  // PF_XMMI64_INSTRUCTIONS_AVAILABLE
  Result := IsProcessorFeaturePresent(10);
end;

#endif


function IsUpgrade(): Boolean;
var
  sPrevPath: String;
begin
  sPrevPath := WizardForm.PrevAppDir;
  Result := (sPrevPath <> '');
end;


// Check if MPC-HC's settings exist
function SettingsExistCheck(): Boolean;
begin
  if RegKeyExists(HKEY_CURRENT_USER, 'Software\Gabest\Media Player Classic') or
  FileExists(ExpandConstant('{app}\{#mpchc_ini}')) then
    Result := True
  else
    Result := False;
end;


function ShouldSkipPage(PageID: Integer): Boolean;
begin
  // Hide the License page
  if IsUpgrade() and (PageID = wpLicense) then
    Result := True;
end;


procedure CleanUpSettingsAndFiles();
begin
  //DeleteFile(ExpandConstant('{app}\bitstream.log'));
  //DeleteFile(ExpandConstant('{app}\dxva_ipinhook.log'));
  //DeleteFile(ExpandConstant('{app}\picture.log'));
  //DeleteFile(ExpandConstant('{app}\slicelong.log'));
  //DeleteFile(ExpandConstant('{app}\sliceshort.log'));
  //DelTree('{app}\BitStream*.bin', False, True, False);
  //DelTree('{app}\Matrix*.bin', False, True, False);
  DeleteFile(ExpandConstant('{app}\{#mpchc_ini}'));
  DeleteFile(ExpandConstant('{userappdata}\Media Player Classic\default.mpcpl'));
  RemoveDir(ExpandConstant('{userappdata}\Media Player Classic'));
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\Gabest\Filters');
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\Gabest\Media Player Classic');
  RegDeleteKeyIfEmpty(HKCU, 'Software\Gabest');
end;


procedure CurStepChanged(CurStep: TSetupStep);
var
  iLanguage: Integer;
begin
  if CurStep = ssPostInstall then begin
    if IsTaskSelected('reset_settings') then
      CleanUpSettingsAndFiles();

    iLanguage := StrToInt(ExpandConstant('{cm:langid}'));
    if IsComponentSelected('mpcresources') and FileExists(ExpandConstant('{app}\{#mpchc_ini}')) then
      SetIniInt('Settings', 'InterfaceLanguage', iLanguage, ExpandConstant('{app}\{#mpchc_ini}'))
    else
      RegWriteDWordValue(HKCU, 'Software\Gabest\Media Player Classic\Settings', 'InterfaceLanguage', iLanguage);
  end;

end;


procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  // When uninstalling, ask the user to delete MPC-HC settings
  if (CurUninstallStep = usUninstall) and SettingsExistCheck() then begin
    if SuppressibleMsgBox(CustomMessage('msg_DeleteSettings'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2, IDNO) = IDYES then
      CleanUpSettingsAndFiles();

    RegDeleteValue(HKLM, 'SOFTWARE\Gabest\Media Player Classic', 'ExePath')
    RegDeleteKeyIfEmpty(HKLM, 'SOFTWARE\Gabest\Media Player Classic');
    RegDeleteKeyIfEmpty(HKLM, 'SOFTWARE\Gabest');

  end;
end;


function InitializeSetup(): Boolean;
begin
  // Create a mutex for the installer and if it's already running display a message and stop installation
  if CheckForMutexes(installer_mutex) and not WizardSilent() then begin
    SuppressibleMsgBox(CustomMessage('msg_SetupIsRunningWarning'), mbError, MB_OK, MB_OK);
    Result := False;
  end
  else begin
    Result := True;
    CreateMutex(installer_mutex);

#if defined(sse2_required)
    if not Is_SSE2_Supported() then begin
      SuppressibleMsgBox(CustomMessage('msg_simd_sse2'), mbCriticalError, MB_OK, MB_OK);
      Result := False;
    end;
#elif defined(sse_required)
    if not Is_SSE_Supported() then begin
      SuppressibleMsgBox(CustomMessage('msg_simd_sse'), mbCriticalError, MB_OK, MB_OK);
      Result := False;
    end;
#endif

  end;
end;


function InitializeUninstall(): Boolean;
begin
  if CheckForMutexes(installer_mutex) then begin
    SuppressibleMsgBox(CustomMessage('msg_SetupIsRunningWarning'), mbError, MB_OK, MB_OK);
    Result := False;
  end
  else begin
    Result := True;
    CreateMutex(installer_mutex);
  end;
end;
