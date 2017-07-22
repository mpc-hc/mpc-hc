; (C) 2012-2017 see Authors.txt
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
; Inno Setup: http://www.jrsoftware.org/isdl.php


#if VER < EncodeVer(5,5,9)
  #error Update your Inno Setup version (5.5.9 or newer)
#endif

; If you want to compile the 64-bit version define "x64Build" (uncomment the define below or use build.bat)
#define sse2_required
;#define VS2015
;#define x64Build


#define VerMajor  "2"
#define VerMinor  "41"
#define top_dir   "..\..\..\..\.."

#include AddBackslash(top_dir) + "include\mpc-hc_config.h"
#include AddBackslash(top_dir) + "include\version.h"

#define copyright_str   "2001-2017"
#define app_name        "VSFilter"

#define app_version     str(VerMajor) + "." + str(VerMinor) + "." + str(MPC_VERSION_REV)
#define app_vername     = app_name + " " + app_version
#define quick_launch    "{userappdata}\Microsoft\Internet Explorer\Quick Launch"

#if defined(VS2015)
  #define base_bindir   = AddBackslash(top_dir) + "bin15"
#else
  #define base_bindir   = AddBackslash(top_dir) + "bin"
#endif

#ifdef x64Build
  #define bindir        = AddBackslash(base_bindir) + "Filters_x64"
  #define OutFilename   = app_name + "_" + app_version + "_x64"
#else
  #define bindir        = AddBackslash(base_bindir) + "Filters_x86"
  #define OutFilename   = app_name + "_" + app_version + "_x86"
#endif

#ifnexist AddBackslash(bindir) + "VSFilter.dll"
  #error Compile VSFilter first
#endif

#if MPC_NIGHTLY_RELEASE
  #define FullAppNameVer = app_vername + " " + "(" + str(MPCHC_HASH) + ")"
#else
  #define FullAppNameVer = app_vername
#endif

#if MPC_NIGHTLY_RELEASE
  #define FullAppNameVer = FullAppNameVer + " " + str(MPC_VERSION_NIGHTLY)
#endif

#if defined(VS2015)
  #define FullAppNameVer = FullAppNameVer + " VS2015"
#endif

#ifdef x64Build
  #define FullAppNameVer = FullAppNameVer + " " + "(64-bit)"
#endif


[Setup]
AppName={#app_name}
AppVerName={#app_vername}
AppVersion={#app_version}
AppPublisher=MPC-HC Team
AppPublisherURL={#WEBSITE_URL}
AppSupportURL={#TRAC_URL}
AppUpdatesURL={#WEBSITE_URL}
AppContact={#WEBSITE_URL}contact-us/
AppCopyright=Copyright © {#copyright_str}, see Authors.txt file
VersionInfoCompany=MPC-HC Team
VersionInfoCopyright=Copyright © {#copyright_str}, MPC-HC Team
VersionInfoDescription={#app_name} {#app_version} Setup
VersionInfoTextVersion={#app_version}
VersionInfoVersion={#app_version}
VersionInfoProductName={#app_name}
VersionInfoProductVersion={#app_version}
VersionInfoProductTextVersion={#app_version}
UninstallDisplayIcon={app}\VSFilter.dll
UninstallDisplayName={#FullAppNameVer}
DefaultDirName={pf}\{#app_name}
DefaultGroupName={#app_name}
LicenseFile={#top_dir}\COPYING.txt
OutputDir=.
OutputBaseFilename={#OutFilename}
AllowNoIcons=yes
Compression=lzma2/ultra
SolidCompression=yes
EnableDirDoesntExistWarning=no
ShowTasksTreeLines=yes
AlwaysShowDirOnReadyPage=yes
AlwaysShowGroupOnReadyPage=yes
DisableDirPage=auto
DisableProgramGroupPage=auto
InfoBeforeFile=InfoBefore.rtf
MinVersion=5.01.2600sp3
#ifdef x64Build
AppID=vsfilter64
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#else
AppID=vsfilter
#endif
#ifexist top_dir + "\signinfo.txt"
SignTool=MySignTool
#endif


[Languages]
Name: en; MessagesFile: compiler:Default.isl


[Messages]
BeveledLabel={#FullAppNameVer}
SetupAppTitle=Setup - {#app_name}
SetupWindowTitle=Setup - {#app_name}


[CustomMessages]
en.msg_DeleteSettings=Do you also want to delete {#app_name}'s settings?%n%nIf you plan on installing {#app_name} again then you do not have to delete them.
en.msg_SetupIsRunningWarning={#app_name} setup is already running!
#if defined(sse_required)
en.msg_simd_sse=This build of {#app_name} requires a CPU with SSE extension support.%n%nYour CPU does not have those capabilities.
#elif defined(sse2_required)
en.msg_simd_sse2=This build of {#app_name} requires a CPU with SSE2 extension support.%n%nYour CPU does not have those capabilities.
#endif
en.tsk_ResetSettings=Reset {#app_name}'s settings


[Tasks]
Name: reset_settings; Description: {cm:tsk_ResetSettings}; Flags: checkedonce unchecked; Check: SettingsExist()


[Files]
Source: {#bindir}\VSFilter.dll;        DestDir: {app}; Flags: restartreplace regserver uninsrestartdelete ignoreversion
Source: {#top_dir}\COPYING.txt;        DestDir: {app}; Flags: ignoreversion
Source: {#top_dir}\docs\Authors.txt;   DestDir: {app}; Flags: ignoreversion
Source: {#top_dir}\docs\Changelog.txt; DestDir: {app}; Flags: ignoreversion
Source: {#top_dir}\docs\Readme.txt;    DestDir: {app}; Flags: ignoreversion


[Icons]
#ifdef x64Build
Name: {group}\Configuration (x64); Filename: {sys}\rundll32.exe; Parameters: VSFilter.dll,DirectVobSub; WorkingDir: {app}; IconFilename: {app}\VSFilter.dll
Name: {group}\Uninstall (x64);     Filename: {uninstallexe}
#else
Name: {group}\Configuration;       Filename: {sys}\rundll32.exe; Parameters: VSFilter.dll,DirectVobSub; WorkingDir: {app}; IconFilename: {app}\VSFilter.dll
Name: {group}\Uninstall;           Filename: {uninstallexe}
#endif


[Run]
Filename: {sys}\rundll32.exe; Parameters: VSFilter.dll,DirectVobSub; Description: Configure VSFilter; WorkingDir: {app}; Flags: nowait postinstall skipifsilent unchecked


[Code]
#if defined(sse_required) || defined(sse2_required)
function IsProcessorFeaturePresent(Feature: Integer): Boolean;
external 'IsProcessorFeaturePresent@kernel32.dll stdcall';
#endif

const installer_mutex = 'vsfilter_setup_mutex';

// Check if VSFilter's settings exist
function SettingsExist(): Boolean;
begin
  if RegKeyExists(HKCU, 'Software\MPC-HC\VSFilter') then
    Result := True
  else
    Result := False;
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


function ShouldSkipPage(PageID: Integer): Boolean;
begin
  // Hide the InfoBefore, License and Ready page when upgrading
  if IsUpgrade() and ((PageID = wpInfoBefore) or (PageID = wpReady) or (PageID = wpLicense)) then
    Result := True;
end;


procedure CleanUpSettings();
begin
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\MPC-HC\VSFilter');
  RegDeleteKeyIfEmpty(HKCU, 'Software\MPC-HC');
end;


procedure CurPageChanged(CurPageID: Integer);
begin
  if IsUpgrade() then
    if not SettingsExist() and (CurPageID = wpWelcome) then
      WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall)
    else if (CurPageID = wpSelectTasks) then
      WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall);
end;


procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) and IsTaskSelected('reset_settings') then
    CleanUpSettings();
end;


procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  // When uninstalling, ask the user to delete VSFilter's settings
  if (CurUninstallStep = usUninstall) and SettingsExist() then
    if SuppressibleMsgBox(CustomMessage('msg_DeleteSettings'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2, IDNO) = IDYES then
      CleanUpSettings();
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
