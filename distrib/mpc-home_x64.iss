;the revision number - 2009-07-22 13:36

#include "../include/Version.h"

#define MyAppName "Media Player Classic - Home Cinema x64"
#define MyAppVerName "Media Player Classic - Home Cinema v."
#define MyAppURL "http://mpc-hc.sourceforge.net/"
#define MyAppExeName "mpc-hc64.exe"
#define MyAppININame "\mpc-hc64.ini"
#define MyDateTimeString GetDateTimeString('yyyymmddhhnnss', '', '');

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2ACBF1FA-F5C3-4B19-A774-B22A31F231B9}
AppName={#MyAppName}
AppVerName={#MyAppVerName} {#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf64}\MPC HomeCinema (x64)
DefaultGroupName={#MyAppName}
LicenseFile=..\COPYING
OutputDir=Installer
OutputBaseFilename=MPC-HomeCinema.{#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}.(x64)
VersionInfoVersion={#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
SetupIconFile=..\src\apps\mplayerc\res\icon.ico
Compression=lzma/ultra64
SolidCompression=yes
AllowNoIcons=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
ShowUndisplayableLanguages=true

[Files]
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpc-hc64.exe; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.br.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.by.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.cz.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.de.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.es.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.fr.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.hu.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.kr.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.pl.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.ru.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.sk.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.tr.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.ua.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.it.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.sc.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.sv.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpcresources.tc.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\x64\Release Unicode\mpciconlib.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\AUTHORS; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\ChangeLog; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\COPYING; DestDir: {app}; Flags: ignoreversion 64bit
;Source: ..\src\apps\mplayerc\Build_x64\d3dx9_41.dll;       DestDir: {app}; Flags: ignoreversion 64bit

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
by.Save_set_mpc=Keep previous settings
cz.Save_set_mpc=Zachovat původní nastavení
de.Save_set_mpc=Die vorherigen Einstellungen behalten
es.Save_set_mpc=Keep previous settings
fr.Save_set_mpc=Conserver les réglages précédents
hu.Save_set_mpc=Keep previous settings
it.Save_set_mpc=Mantieni le impostazioni precedenti
kr.Save_set_mpc=이전 설정 유지
pl.Save_set_mpc=Zachowaj poprzednie ustawienia
pt.Save_set_mpc=Keep previous settings
ru.Save_set_mpc=Keep previous settings
sc.Save_set_mpc=保持当前配置
se.Save_set_mpc=Keep previous settings
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
    renamefile (ExpandConstant('{app}\' + 'mplayerc64.exe'), ExpandConstant('{app}\' + 'mplayerc64.exe.bak'));
end;

