;the revision number - 2009-07-22 13:36

#include "../include/Version.h"

#define MyAppName "Media Player Classic - Home Cinema"
#define MyAppVerName "Media Player Classic - Home Cinema v."
#define MyAppURL "http://mpc-hc.sourceforge.net/"
#define MyAppExeName "mplayerc.exe"
#define MyAppININame "\mplayerc.ini"
#define MyDateTimeString GetDateTimeString('yyyymmddhhnnss', '', '');

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2624B969-7135-4EB1-B0F6-2D8C397B45F7}

AppName={#MyAppName}
AppVerName={#MyAppVerName} {#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\MPC HomeCinema
DefaultGroupName={#MyAppName}
LicenseFile=..\COPYING
OutputDir=Installer
OutputBaseFilename=MPC-HomeCinema.{#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}.(x86)
VersionInfoVersion={#VERSION_MAJOR}.{#VERSION_MINOR}.{#VERSION_REV}.{#VERSION_PATCH}
SetupIconFile=..\src\apps\mplayerc\res\icon.ico
AllowNoIcons=yes
Compression=lzma/ultra64
SolidCompression=yes
ShowUndisplayableLanguages=true

[Files]
Source: ..\src\apps\mplayerc\Release Unicode\mplayerc.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.br.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.by.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.cz.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.de.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.es.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.fr.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.hu.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.kr.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.pl.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.ru.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.sk.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.tr.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.ua.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.it.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.sc.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.sv.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpcresources.tc.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\Release Unicode\mpciconlib.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\AUTHORS; DestDir: {app}; Flags: ignoreversion
Source: ..\src\apps\mplayerc\ChangeLog; DestDir: {app}; Flags: ignoreversion
Source: ..\COPYING; DestDir: {app}; Flags: ignoreversion
;Source: ..\src\apps\mplayerc\Build_x86\d3dx9_41.dll;       DestDir: {app}; Flags: ignoreversion

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
Name: br; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
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
en.Save_set_mpc=To save the old program settings
cz.Save_set_mpc=Ulozit stare nastaveni programu
fi.Save_set_mpc=Tallenna vanhan ohjelman asetuksia
fr.Save_set_mpc=Sauver l'ancien programme de reglages
de.Save_set_mpc=Alte Abstimmungen des Programms zu behalten
hu.Save_set_mpc=To save the old program settings
it.Save_set_mpc=Salvare il vecchio impostazioni del programma
no.Save_set_mpc=Lagre den gamle programmet innstillinger
pl.Save_set_mpc=Zachowac stare nastrajania programu
pt.Save_set_mpc=Salve o programa antigo configuracoes
ru.Save_set_mpc=Сохранить старые настройки программы
es.Save_set_mpc=Salvar el viejo programa de configuracion

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
end;