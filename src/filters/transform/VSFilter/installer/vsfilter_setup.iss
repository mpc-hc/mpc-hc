; $Id$
;
; (C) 2012 see Authors.txt
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


; If you want to compile the 64-bit version define "x64build" (uncomment the define below or use build_installer.bat)

#define VerMajor  "2"
#define VerMinor  "41"
#define copyright "2001-2012"
#define top_dir   "..\..\..\..\.."
;#define x64Build

#ifdef x64_build
#define bindir    top_dir + "\bin\Filters_x64"
#else
#define bindir    top_dir + "\bin\Filters_x86"
#endif

#ifnexist bindir + "\VSFilter.dll"
  #error Compile VSFilter first
#endif

#if VER < EncodeVer(5,4,3)
  #error Update your Inno Setup version (5.4.3 or newer)
#endif

#define ISPP_IS_BUGGY
#include top_dir + "\include\Version.h"
#define app_version str(VerMajor) + "." + str(VerMinor) + "." + str(MPC_VERSION_REV)


[Setup]
AppName=DirectVobSub
AppVerName=DirectVobSub {#app_version}
AppVersion={#app_version}
AppPublisher=MPC-HC Team
AppPublisherURL=http://mpc-hc.sourceforge.net/
AppSupportURL=http://mpc-hc.sourceforge.net/
AppUpdatesURL=http://mpc-hc.sourceforge.net/
AppContact=http://mpc-hc.sourceforge.net/
AppCopyright=Copyright © {#copyright}, see Authors.txt file
VersionInfoCompany=MPC-HC Team
VersionInfoCopyright=Copyright © {#copyright}, MPC-HC Team
VersionInfoDescription=DirectVobSub {#app_version} Setup
VersionInfoTextVersion={#app_version}
VersionInfoVersion={#app_version}
VersionInfoProductName=DirectVobSub
VersionInfoProductVersion={#app_version}
VersionInfoProductTextVersion={#app_version}
UninstallDisplayIcon={app}\VSFilter.dll
DefaultDirName={pf}\DirectVobSub
DefaultGroupName=DirectVobSub
LicenseFile={#top_dir}\COPYING.txt
OutputDir=.
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
MinVersion=0,5.01.2600sp3
#ifdef x64_build
AppID=vsfilter64
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
UninstallDisplayName=DirectVobSub {#app_version} (x64)
OutputBaseFilename=DirectVobSub_{#app_version}_x64
#else
AppID=vsfilter
UninstallDisplayName=DirectVobSub {#app_version}
OutputBaseFilename=DirectVobSub_{#app_version}_x86
#endif


[Languages]
Name: en; MessagesFile: compiler:Default.isl


[Messages]
#ifdef x64Build
BeveledLabel=DirectVobSub {#app_version} x64
#else
BeveledLabel=DirectVobSub {#app_version}
#endif
SetupAppTitle=Setup - DirectVobSub
SetupWindowTitle=Setup - DirectVobSub


[CustomMessages]
en.msg_DeleteSettings =Do you also want to delete DirectVobSub's settings?%n%nIf you plan on installing DirectVobSub again then you do not have to delete them.
en.tsk_ResetSettings  =Reset DirectVobSub's settings


[Tasks]
Name: reset_settings; Description: {cm:tsk_ResetSettings}; Flags: checkedonce unchecked; Check: SettingsExist()


[Files]
Source: {#bindir}\VSFilter.dll;        DestDir: {app}; Flags: restartreplace regserver uninsrestartdelete ignoreversion
#ifdef x64_build
Source: unrar64.dll;                   DestDir: {sys}; Flags: onlyifdoesntexist sharedfile uninsnosharedfileprompt
#else
Source: unrar.dll;                     DestDir: {sys}; Flags: onlyifdoesntexist sharedfile uninsnosharedfileprompt
#endif
Source: {#top_dir}\COPYING.txt;        DestDir: {app}; Flags: ignoreversion
Source: {#top_dir}\docs\Authors.txt;   DestDir: {app}; Flags: ignoreversion
Source: {#top_dir}\docs\Changelog.txt; DestDir: {app}; Flags: ignoreversion
Source: {#top_dir}\docs\Readme.txt;    DestDir: {app}; Flags: ignoreversion
Source: unrar_license.txt;             DestDir: {app}; Flags: ignoreversion


[Icons]
#ifdef x64_build
Name: {group}\Configuration (x64); Filename: {sys}\rundll32.exe; Parameters: VSFilter.dll,DirectVobSub; WorkingDir: {app}; IconFilename: {app}\VSFilter.dll
Name: {group}\Uninstall (x64);     Filename: {uninstallexe}
#else
Name: {group}\Configuration;       Filename: {sys}\rundll32.exe; Parameters: VSFilter.dll,DirectVobSub; WorkingDir: {app}; IconFilename: {app}\VSFilter.dll
Name: {group}\Uninstall;           Filename: {uninstallexe}
#endif


[Run]
Filename: {sys}\rundll32.exe; Parameters: VSFilter.dll,DirectVobSub; Description: Configure DirectVobSub; WorkingDir: {app}; Flags: nowait postinstall skipifsilent unchecked


[Code]
// Check if VSFilter's settings exist
function SettingsExist(): Boolean;
begin
  if RegKeyExists(HKCU, 'Software\Gabest\VSFilter') then
    Result := True
  else
    Result := False;
end;


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
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\Gabest\VSFilter');
  RegDeleteKeyIfEmpty(HKCU, 'Software\Gabest');
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
