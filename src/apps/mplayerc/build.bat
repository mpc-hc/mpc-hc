@ECHO OFF

REM Check if the needed files are present
IF "%VS90COMNTOOLS%"=="" GOTO :BadPaths
IF "%MINGW32%"=="" GOTO :BadPaths
IF "%MINGW64%"=="" GOTO :BadPaths

REM Detect if we are running on 64bit WIN and use Wow6432Node, set the path
REM of Inno Setup accordingly
IF "%PROGRAMFILES(x86)%zzz"=="zzz" (SET "U_=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"
) ELSE (
SET "U_=HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
)

SET "I_=Inno Setup"
SET "A_=%I_% 5"
FOR /f "delims=" %%a IN (
	'REG QUERY "%U_%\%A_%_is1" /v "%I_%: App Path"2^>Nul^|FIND "REG_"') DO (
	SET "InnoSetupPath=%%a"&CALL :SubIS %%InnoSetupPath:*Z=%%)

GOTO :GoodPaths

:BadPaths
ECHO: "Not all build dependencies found. To build MPC-HC you need:"
ECHO: "* Visual Studio 2008 installed"
ECHO: "* MinGW 32 bit build environment with coreMSYS pointed to in MINGW32 env var"
ECHO: "* MinGW 64 bit build environment with coreMSYS pointed to in MINGW64 env var"
PAUSE
GOTO :EndGood

:GoodPaths
SET BUILDTYPE=/%1
IF "%1"=="" SET BUILDTYPE=/build

SET ORIGPATH="%CD%"
CALL "%VS90COMNTOOLS%vsvars32.bat"
CD %ORIGPATH%

devenv ..\..\..\mpc-hc.sln %BUILDTYPE% "Release|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad

devenv ..\..\..\mpciconlib.sln %BUILDTYPE% "Release|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Belarusian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Catalan|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Chinese simplified|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Chinese traditional|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Czech|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Dutch|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release French|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release German|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Hungarian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Italian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Korean|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Polish|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Portuguese|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Russian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Slovak|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Spanish|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Swedish|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Turkish|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Ukrainian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad

IF "%1"=="clean" GOTO x64

IF DEFINED InnoSetupPath ("%InnoSetupPath%\iscc.exe" /Q^
 "..\..\..\distrib\mpc-hc_setup.iss") ELSE (GOTO :x64)

:x64

REM GOTO :Nox64
devenv ..\..\..\mpc-hc.sln %BUILDTYPE% "Release|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpciconlib.sln %BUILDTYPE% "Release|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Belarusian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Catalan|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Chinese simplified|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Chinese traditional|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Czech|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Dutch|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release French|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release German|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Hungarian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Italian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Korean|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Polish|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Portuguese|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Russian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Slovak|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Spanish|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Swedish|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Turkish|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv ..\..\..\mpcresources.sln %BUILDTYPE% "Release Ukrainian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad

IF "%1"=="clean" GOTO :Nox64

IF DEFINED InnoSetupPath ("%InnoSetupPath%\iscc.exe" /Q^
 "..\..\..\distrib\mpc-hc_setup.iss" /DBuildx64=True) ELSE (GOTO :Nox64)

:Nox64
GOTO :EndGood

:EndBad
ECHO: " "
ECHO: ERROR: Build failed and aborted
PAUSE
GOTO :EOF

:EndGood
GOTO :EOF

:SubIS
SET InnoSetupPath=%*
GOTO :EOF