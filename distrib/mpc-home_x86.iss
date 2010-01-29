#include "../include/Version.h"

#define MyAppName "Media Player Classic - Home Cinema"
#define MyAppVerName "Media Player Classic - Home Cinema v."
#define MyAppURL "http://mpc-hc.sourceforge.net/"
#define MyAppExeName "mpc-hc.exe"
#define MyAppININame "\mpc-hc.ini"
#define MyDateTimeString GetDateTimeString('yyyymmddhhnnss', '', '');

[Setup]
AppId={{2624B969-7135-4EB1-B0F6-2D8C397B45F7}
AppName={#MyAppName}
AppVersion={#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
AppVerName={#MyAppVerName} {#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
AppPublisher=MPC-HC Team
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
AppContact={#MyAppURL}
AppCopyright=Copyright � 2002-2010, see AUTHORS file
VersionInfoCompany=MPC-HC Team
VersionInfoCopyright=Copyright � 2002-2010, see AUTHORS file
VersionInfoDescription={#MyAppName} {#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH} Setup
VersionInfoTextVersion={#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
VersionInfoVersion={#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
VersionInfoProductName=Process Hacker
VersionInfoProductVersion={#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
VersionInfoProductTextVersion={#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
DefaultDirName={pf}\MPC HomeCinema
DefaultGroupName={#MyAppName}
LicenseFile=..\COPYING
OutputDir=Installer
OutputBaseFilename=MPC-HomeCinema.{#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}.(x86)
SetupIconFile=..\src\apps\mplayerc\res\icon.ico
Compression=lzma/ultra64
SolidCompression=yes
AllowNoIcons=yes
ShowUndisplayableLanguages=true
DisableDirPage=auto
DisableProgramGroupPage=auto

[Files]
Source: ..\src\apps\mplayerc\Release Unicode\mpc-hc.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.??.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpciconlib.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\AUTHORS; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\ChangeLog; DestDir: {app}; Flags: ignoreversion
Source: ..\COPYING; DestDir: {app}; Flags: ignoreversion

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent unchecked

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExeName}
Name: {group}\{cm:ProgramOnTheWeb,{#MyAppName}}; Filename: {#MyAppURL}
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {commondesktop}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: desktopicon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: quicklaunchicon

[Languages]
Name: en; MessagesFile: compiler:Default.isl
Name: br; MessagesFile: Languages\BrazilianPortuguese.isl
Name: by; MessagesFile: Languages\Belarus.isl
Name: cz; MessagesFile: compiler:Languages\Czech.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl
Name: fi; MessagesFile: compiler:Languages\Finnish.isl
Name: fr; MessagesFile: compiler:Languages\French.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: hu; MessagesFile: compiler:Languages\Hungarian.isl
Name: it; MessagesFile: compiler:Languages\Italian.isl
Name: kr; MessagesFile: Languages\Korean.isl
Name: no; MessagesFile: compiler:Languages\Norwegian.isl
Name: pl; MessagesFile: compiler:Languages\Polish.isl
Name: pt; MessagesFile: compiler:Languages\Portuguese.isl
Name: ru; MessagesFile: compiler:Languages\Russian.isl
Name: sc; MessagesFile: Languages\ChineseSimp.isl
Name: se; MessagesFile: Languages\Swedish.isl
Name: sk; MessagesFile: compiler:Languages\Slovak.isl
Name: tc; MessagesFile: Languages\ChineseTrad.isl
Name: tr; MessagesFile: Languages\Turkish.isl
Name: ua; MessagesFile: Languages\Ukrainian.isl

[CustomMessages]
en.Save_set_mpc=Keep previous settings
br.Save_set_mpc=Manter ajustes anteriores
by.Save_set_mpc=Захаваць папярэднiя налады
cz.Save_set_mpc=Zachovat původní nastavení
de.Save_set_mpc=Die vorherigen Einstellungen behalten
es.Save_set_mpc=Keep previous settings
fr.Save_set_mpc=Conserver les réglages précédents
hu.Save_set_mpc=Keep previous settings
it.Save_set_mpc=Mantieni le impostazioni precedenti
kr.Save_set_mpc=이전 설정 유지
pl.Save_set_mpc=Zachowaj bieżące ustawienia programu
pt.Save_set_mpc=Keep previous settings
ru.Save_set_mpc=Не удалять предыдущие настройки
sc.Save_set_mpc=保持当前配置
se.Save_set_mpc=Behåll tidigare inställningar
sk.Save_set_mpc=Zachovať predchádzajúce nastavenia
tc.Save_set_mpc=保留先前的設定
tr.Save_set_mpc=Önceki ayarları kullan
ua.Save_set_mpc=Keep previous settings

en.langid=00000000
br.langid=00000017
by.langid=00000015
cz.langid=00000005
de.langid=00000002
es.langid=00000006
fr.langid=00000001
hu.langid=00000007
it.langid=00000011
kr.langid=00000008
pl.langid=00000009
pt.langid=00000017
ru.langid=00000003
sc.langid=00000013
se.langid=00000016
sk.langid=00000012
tc.langid=00000014
tr.langid=00000004
ua.langid=00000010


[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: checkedonce
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: checkedonce
Name: uninstallable_set; Description: {cm:Save_set_mpc}

[Code]
procedure BackupRegistry();
Var
    res: integer;
    RegEdit, RegFile : String;
begin
    RegEdit := ExpandConstant('{win}\regedit.exe');
    RegFile := ExpandConstant('{app}\Backup_1_' + '{#MyDateTimeString}' + '.reg');
	  Exec(RegEdit,' /ea ' + AddQuotes(RegFile)+' HKEY_CURRENT_USER\Software\Gabest',ExpandConstant('{win}'),SW_HIDE,ewWaitUntilTerminated,res);
    RegFile := ExpandConstant('{app}\Backup_2_' + '{#MyDateTimeString}' + '.reg');
    Exec(RegEdit,' /ea ' + AddQuotes(RegFile)+' HKEY_LOCAL_MACHINE\Software\Gabest',ExpandConstant('{win}'),SW_HIDE,ewWaitUntilTerminated,res);
end;

procedure CurStepChanged(CurStep: TSetupStep);
Var
    lang : Integer;
begin
    if CurStep = ssDone then
    begin
        if IsTaskSelected('uninstallable_set') = False then
        begin

            BackupRegistry();
            renamefile (ExpandConstant('{app}\' + '{#MyAppININame}'), ExpandConstant('{app}\' + '{#MyAppININame}' + '{#MyDateTimeString}' + '.bak'));
            RegDeleteKeyIncludingSubkeys(HKCU, 'Software\Gabest\');
            RegDeleteKeyIncludingSubkeys(HKLM, 'Software\Gabest\');
        end;
	end;
		lang := StrToInt(ExpandConstant('{cm:langid}'));
		if FileExists(ExpandConstant('{app}\' + '{#MyAppININame}')) then
		SetIniInt('Settings', 'InterfaceLanguage', lang, ExpandConstant('{app}\' + '{#MyAppININame}'))
		else
		RegWriteDWordValue(HKCU, 'Software\Gabest\Media Player Classic\Settings', 'InterfaceLanguage', lang);
    // rename binary from previous installer
    renamefile (ExpandConstant('{app}\' + 'mplayerc.exe'), ExpandConstant('{app}\' + 'mplayerc.exe.bak'));
end;
