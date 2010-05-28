@ECHO OFF
SETLOCAL
REM The batch file accepts one argument so you can do a Build|Rebuild|Clean
REM e.g.: build.bat Rebuild

REM Check if the needed environment variables are present
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
ECHO: "* MinGW 32 bit build environment with MSYS pointed to in MINGW32 env var"
ECHO: "* MinGW 64 bit build environment with MSYS pointed to in MINGW64 env var"
PAUSE
GOTO :EndGood

:GoodPaths
SET BUILDTYPE=/%1
IF "%1"=="" SET BUILDTYPE=/Build

SET ORIGPATH="%CD%"
CALL "%VS90COMNTOOLS%vsvars32.bat"
CD %ORIGPATH%

devenv mpc-hc.sln %BUILDTYPE% "Release|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad

devenv mpciconlib.sln %BUILDTYPE% "Release|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad

FOR %%A IN ("Belarusian" "Catalan" "Chinese simplified" "Chinese traditional" 
"Czech" "Dutch" "French" "German" "Hungarian" "Italian" "Korean" "Polish" 
"Portuguese" "Russian" "Slovak" "Spanish" "Swedish" "Turkish" "Ukrainian"
) DO (
CALL :SubMPCRES %%A Win32
)

IF "%1"=="clean" GOTO x64
XCOPY src\apps\mplayerc\AUTHORS "bin\mpc-hc_x86\" /Y
XCOPY src\apps\mplayerc\ChangeLog "bin\mpc-hc_x86\" /Y
XCOPY COPYING "bin\mpc-hc_x86\" /Y
DEL/f/a "bin\mpc-hc_x86\mpciconlib.exp" "bin\mpc-hc_x86\mpciconlib.lib" >NUL 2>&1

IF DEFINED InnoSetupPath ("%InnoSetupPath%\iscc.exe" /Q^
 "distrib\mpc-hc_setup.iss") ELSE (GOTO :x64)

:x64

REM Uncomment the following line if you want to skip the x64 build
REM GOTO :Nox64
devenv mpc-hc.sln %BUILDTYPE% "Release|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpciconlib.sln %BUILDTYPE% "Release|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad

FOR %%A IN ("Belarusian" "Catalan" "Chinese simplified" "Chinese traditional" 
"Czech" "Dutch" "French" "German" "Hungarian" "Italian" "Korean" "Polish" 
"Portuguese" "Russian" "Slovak" "Spanish" "Swedish" "Turkish" "Ukrainian"
) DO (
CALL :SubMPCRES %%A x64
)

IF "%1"=="clean" GOTO :Nox64
XCOPY src\apps\mplayerc\AUTHORS "bin\mpc-hc_x64\" /Y
XCOPY src\apps\mplayerc\ChangeLog "bin\mpc-hc_x64\" /Y
XCOPY COPYING "bin\mpc-hc_x64\" /Y
DEL/f/a "bin\mpc-hc_x64\mpciconlib.exp" "bin\mpc-hc_x64\mpciconlib.lib" >NUL 2>&1

IF DEFINED InnoSetupPath ("%InnoSetupPath%\iscc.exe" /Q^
 "distrib\mpc-hc_setup.iss" /DBuildx64=True) ELSE (GOTO :Nox64)

:Nox64
GOTO :EndGood

:EndBad
ECHO: " "
ECHO: ERROR: Build failed and aborted
PAUSE
ENDLOCAL
EXIT

:EndGood
ENDLOCAL
GOTO :EOF

:SubMPCRES
devenv mpcresources.sln %BUILDTYPE% "Release %~1|%2"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
GOTO :EOF

:SubIS
SET InnoSetupPath=%*
GOTO :EOF