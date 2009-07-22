;the revision number - 2009-07-21 11:41
#define MyAppExeNumber "1.2.1175.0"

#define MyAppName "Media Player Classic - Home Cinema x64"
#define MyAppVerName "Media Player Classic - Home Cinema v."
#define MyAppURL "http://mpc-hc.sourceforge.net/"
#define MyAppExeName "mplayerc64.exe"
#define MyAppININame "\mplayerc64.ini"
#define MyDateTimeString GetDateTimeString('yyyymmddhhnnss', '', '');

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2ACBF1FA-F5C3-4B19-A774-B22A31F231B9}
AppName={#MyAppName}
AppVerName={#MyAppVerName}{#MyAppExeNumber}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf64}\MPC HomeCinema (x64)
DefaultGroupName={#MyAppName}
LicenseFile=..\COPYING
OutputDir=Installer
OutputBaseFilename=MPC-HomeCinema.{#MyAppExeNumber}.(x64)
VersionInfoVersion={#MyAppExeNumber}
SetupIconFile=..\src\apps\mplayerc\res\icon.ico
Compression=lzma/ultra64
SolidCompression=yes
AllowNoIcons=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
ShowUndisplayableLanguages=true

[Files]
Source: ..\src\apps\mplayerc\Build_x64\mplayerc64.exe; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.br.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.by.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.cz.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.de.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.es.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.fr.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.hu.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.kr.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.pl.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.ru.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.sk.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.tr.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.ua.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.it.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.sc.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.sv.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpcresources.tc.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\mpciconlib.dll; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\AUTHORS; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\ChangeLog; DestDir: {app}; Flags: ignoreversion 64bit
Source: ..\src\apps\mplayerc\Build_x64\COPYING; DestDir: {app}; Flags: ignoreversion 64bit
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