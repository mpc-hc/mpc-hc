@ECHO OFF
CLS
SETLOCAL
SET "COMPILER=MSVC 2010"

IF /I "%~1"=="help"   GOTO SHOWHELP
IF /I "%~1"=="/help"  GOTO SHOWHELP
IF /I "%~1"=="-help"  GOTO SHOWHELP
IF /I "%~1"=="--help" GOTO SHOWHELP
IF /I "%~1"=="/?"     GOTO SHOWHELP


REM pre-build checks
IF "%VS100COMNTOOLS%"=="" GOTO MissingVar
IF "%MINGW32%"==""        GOTO MissingVar
IF "%MINGW64%"==""        GOTO MissingVar
CALL :SubDetectInnoSetup


REM set up variables
SET START_TIME=%DATE%-%TIME%

IF "%1"=="" (SET BUILDTYPE=Build) ELSE (SET BUILDTYPE=%1)

SET build_type=x86
IF /I "%2"=="x64" GOTO build_x64
GOTO call_vcvarsall

:build_x64
IF DEFINED PROGRAMFILES(x86) (SET build_type=amd64) ELSE (SET build_type=x86_amd64)

:call_vcvarsall
CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" %build_type%
CD /D %~dp0

REM Debug build only applies to Main (mpc-hc_2010.sln)
IF /I "%4"=="Debug" (SET BUILDCONFIG=Debug) ELSE (SET BUILDCONFIG=Release)

REM Do we want to build x86, x64 or both?
IF /I "%2"=="x64" GOTO skip32
SET OUTDIR=bin10\mpc-hc_x86
SET PLATFORM=Win32
CALL :Sub_build_internal %*


:skip32
IF /I "%2"=="x86" GOTO END
SET OUTDIR=bin10\mpc-hc_x64
SET PLATFORM=x64
CALL :Sub_build_internal %*
GOTO END


:EndWithError
TITLE Compiling MPC-HC [ERROR]
ECHO. & ECHO.
ECHO  **ERROR: Build failed and aborted!**
PAUSE
ENDLOCAL
EXIT


:END
TITLE Compiling MPC-HC with %COMPILER% [FINISHED]
ECHO. & ECHO.
ECHO MPC-HC's compilation started on %START_TIME%
ECHO and completed on %DATE%-%TIME%
ECHO.
ENDLOCAL
EXIT /B


:Sub_build_internal
IF /I "%3"=="Resource" GOTO skipMain

TITLE Compiling MPC-HC with %COMPILER% - %BUILDCONFIG%^|%PLATFORM%...
devenv /nologo mpc-hc_2010.sln /%BUILDTYPE% "%BUILDCONFIG%|%PLATFORM%"
IF %ERRORLEVEL% NEQ 0 GOTO EndWithError


:skipMain
IF /I "%3"=="Main" GOTO skipResource

TITLE Compiling mpciconlib with %COMPILER% - Release^|%PLATFORM%...
devenv /nologo mpciconlib_2010.sln /%BUILDTYPE% "Release|%PLATFORM%"
IF %ERRORLEVEL% NEQ 0 GOTO EndWithError

FOR %%A IN ("Armenian" "Belarusian" "Catalan" "Chinese simplified" "Chinese traditional"
 "Czech" "Dutch" "French" "German" "Hebrew" "Hungarian" "Italian" "Japanese" "Korean"
 "Polish" "Portuguese" "Russian" "Slovak" "Spanish" "Swedish" "Turkish" "Ukrainian"
) DO (
 CALL :SubMPCRES %%A
)


:skipResource
IF /I "%1"=="Clean"    EXIT /B
IF /I "%3"=="Resource" EXIT /B
IF /I "%3"=="Main"     EXIT /B
IF /I "%4"=="Debug"    EXIT /B

XCOPY "src\apps\mplayerc\Authors.txt"   "%OUTDIR%\" /Y /V>NUL
XCOPY "src\apps\mplayerc\Changelog.txt" "%OUTDIR%\" /Y /V>NUL
XCOPY "COPYING.txt"                     "%OUTDIR%\" /Y /V>NUL

IF /I "%PLATFORM%"=="x64" GOTO skipx86installer

IF DEFINED InnoSetupPath (
  TITLE Compiling x86 installer %COMPILER%...
  "%InnoSetupPath%\iscc.exe" /Q /O"bin10" "distrib\mpc-hc_setup.iss"
  IF %ERRORLEVEL% NEQ 0 GOTO EndWithError
  ECHO. & ECHO x86 installer successfully built
) ELSE (
  ECHO. & ECHO Inno Setup wasn't found, the installer wasn't built
  GOTO END
)
EXIT /B


:skipx86installer
IF /I "%PLATFORM%"=="Win32" GOTO END

IF DEFINED InnoSetupPath (
  TITLE Compiling x64 installer %COMPILER%...
  "%InnoSetupPath%\iscc.exe" /Q /O"bin10" "distrib\mpc-hc_setup.iss" /Dx64Build
  IF %ERRORLEVEL% NEQ 0 GOTO EndWithError
  ECHO. & ECHO x64 installer successfully built
) ELSE (
  ECHO. & ECHO Inno Setup wasn't found, the installer wasn't built
  GOTO END
)
EXIT /B


:SubMPCRES
TITLE Compiling mpcresources with %COMPILER% - %~1^|%PLATFORM%...
devenv /nologo mpcresources_2010.sln /%BUILDTYPE% "Release %~1|%PLATFORM%"
IF %ERRORLEVEL% NEQ 0 GOTO EndWithError
EXIT /B


:SHOWHELP
TITLE "%~nx0 %1"
ECHO.
ECHO Usage:
ECHO %~nx0 [clean^|build^|rebuild] [null^|x86^|x64] [null^|Main^|Resource] [Debug]
ECHO.
ECHO Executing "%~nx0" without any arguments will use the default ones:
ECHO "%~nx0 build null null"
ECHO.
ECHO Examples:
ECHO %~nx0 build x86 Resource      -Builds the x86 resources only
ECHO %~nx0 build null Resource     -Builds both x86 and x64 resources only
ECHO %~nx0 build x86               -Builds x86 Main exe and the resources
ECHO %~nx0 build x86 null Debug    -Builds x86 Main Debug exe and resources
ECHO.
ECHO "null" can be replaced with anything, e.g. "all":
ECHO "%~nx0 build x86 all Debug"
ECHO.
ECHO NOTE: Debug only applies to Main project [mpc-hc.sln]
ECHO.
ENDLOCAL
EXIT /B


:MissingVar
COLOR 0C
TITLE Compiling MPC-HC [ERROR]
ECHO Not all build dependencies were found. To build MPC-HC you need:
ECHO * Visual Studio 2010 SP1 installed
ECHO * MinGW 32bit with MSYS pointed to in MINGW32 environment variable
ECHO * MinGW 64bit with MSYS pointed to in MINGW64 environment variable
ECHO.
ECHO See "Compilation.txt" for more information.
ECHO. & ECHO.
ECHO Press any key to exit...
PAUSE >NUL
ENDLOCAL
EXIT /B


:SubDetectInnoSetup
REM Detect if we are running on 64bit WIN and use Wow6432Node, and set the path
REM of Inno Setup accordingly
IF DEFINED PROGRAMFILES(x86) (
  SET "U_=HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
) ELSE (
  SET "U_=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"
)

FOR /F "delims=" %%a IN (
  'REG QUERY "%U_%\Inno Setup 5_is1" /v "Inno Setup: App Path"2^>Nul^|FIND "REG_"') DO (
  SET "InnoSetupPath=%%a" & CALL :SubInnoSetupPath %%InnoSetupPath:*Z=%%)
EXIT /B


:SubInnoSetupPath
SET InnoSetupPath=%*
EXIT /B
