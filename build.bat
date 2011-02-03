@ECHO OFF
SETLOCAL

IF /I "%1"=="help" GOTO showhelp
IF /I "%1"=="/help" GOTO showhelp
IF /I "%1"=="-help" GOTO showhelp
IF /I "%1"=="--help" GOTO showhelp
IF /I "%1"=="/?" GOTO showhelp
GOTO start


:showhelp
TITLE build.bat %1
ECHO.
ECHO Usage:
ECHO build.bat [clean^|build^|rebuild] [null^|x86^|x64] [null^|Main^|Resource] [Debug]
ECHO.
ECHO Executing "build.bat" will use the defaults: "build.bat build null null"
ECHO.
ECHO Examples:
ECHO build.bat build x86 Resource     -Will build the x86 resources only
ECHO build.bat build null Resource    -Will build both x86 and x64 resources only
ECHO build.bat build x86              -Will build x86 Main exe and the resources
ECHO build.bat build x86 null Debug   -Will build x86 Main Debug exe and resources
ECHO.
ECHO "null" can be replaced with anything, e.g. "all": build.bat build x86 all Debug
ECHO.
ECHO NOTE: Debug only applies to Main project [mpc-hc.sln]
ECHO.
ENDLOCAL
EXIT /B


:start
REM pre-build checks
IF "%VS90COMNTOOLS%" == "" GOTO MissingVar
IF "%MINGW32%" == "" GOTO MissingVar
IF "%MINGW64%" == "" GOTO MissingVar

REM Detect if we are running on 64bit WIN and use Wow6432Node
IF "%PROGRAMFILES(x86)%zzz"=="zzz" (
  SET "U_=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"
) ELSE (
  SET "U_=HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
)

SET "I_=Inno Setup"
SET "A_=%I_% 5"
FOR /f "delims=" %%a IN (
  'REG QUERY "%U_%\%A_%_is1" /v "%I_%: App Path"2^>Nul^|FIND "REG_"') DO (
  SET "InnoSetupPath=%%a"&CALL :SubIS %%InnoSetupPath:*Z=%%)

GOTO NoVarMissing


:MissingVar
COLOR 0C
TITLE Compiling MPC-HC [ERROR]
ECHO Not all build dependencies found. To build MPC-HC you need:
ECHO * Visual Studio 2008 (SP1) installed
ECHO * MinGW 32 bit build environment with MSYS pointed to in MINGW32 env var
ECHO * MinGW 64 bit build environment with MSYS pointed to in MINGW64 env var
ECHO. & ECHO.
ECHO Press any key to exit...
PAUSE >NUL
ENDLOCAL
EXIT /B


:NoVarMissing
REM set up variables
SET start_time=%date%-%time%

IF "%1" == "" (SET BUILDTYPE=Build) ELSE (SET BUILDTYPE=%1)

SET build_type=x86
IF /I "%2" == "x64" GOTO build_x64
GOTO call_vcvarsall

:build_x64
IF "%PROGRAMFILES(x86)%zzz"=="zzz" (
  SET build_type=x86_amd64
) ELSE (
  SET build_type=amd64
)

:call_vcvarsall
CALL "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" %build_type%
CD /D %~dp0

REM Debug build only applies to Main (mpc-hc.sln)
IF /I "%4" == "Debug" (SET BUILDCONFIG=Debug) ELSE (SET BUILDCONFIG=Release)

REM Do we want to build x86, x64 or both?
IF /I "%2" == "x64" GOTO skip32
SET COPY_TO_DIR=bin\mpc-hc_x86
SET Platform=Win32
CALL :Sub_build_internal %*


:skip32
IF /I "%2" == "x86" GOTO END
SET COPY_TO_DIR=bin\mpc-hc_x64
SET Platform=x64
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
TITLE Compiling MPC-HC with MSVC 2008 [FINISHED]
ECHO. & ECHO.
ECHO MPC-HC's compilation started on %start_time%
ECHO and completed on %date%-%time%
ECHO.
ENDLOCAL
EXIT /B


:Sub_build_internal
TITLE Compiling MPC-HC with MSVC 2008 - %BUILDCONFIG%^|%Platform%...

IF /I "%3"=="Resource" GOTO skipMain

devenv /nologo mpc-hc.sln /%BUILDTYPE% "%BUILDCONFIG%|%Platform%"
IF %ERRORLEVEL% NEQ 0 GOTO EndWithError


:skipMain
IF /I "%3"=="Main" GOTO skipResource

TITLE Compiling mpciconlib with MSVC 2008 - Release^|%Platform%...
devenv /nologo mpciconlib.sln /%BUILDTYPE% "Release|%Platform%"
IF %ERRORLEVEL% NEQ 0 GOTO EndWithError

DEL/f/a "%COPY_TO_DIR%\mpciconlib.exp" "%COPY_TO_DIR%\mpciconlib.lib" >NUL 2>&1

FOR %%A IN ("Armenian" "Belarusian" "Catalan" "Chinese simplified" "Chinese traditional" 
"Czech" "Dutch" "French" "German" "Hungarian" "Italian" "Japanese" "Korean" 
"Polish" "Portuguese" "Russian" "Slovak" "Spanish" "Swedish" "Turkish" "Ukrainian"
) DO (
CALL :SubMPCRES %%A
)


:skipResource
IF /I "%1" == "Clean" EXIT /B
IF /I "%3" == "Resource" EXIT /B
IF /I "%3" == "Main" EXIT /B
IF /I "%4" == "Debug" EXIT /B

XCOPY "src\apps\mplayerc\Authors.txt" ".\%COPY_TO_DIR%\" /Y /V
XCOPY "src\apps\mplayerc\Changelog.txt" ".\%COPY_TO_DIR%\" /Y /V
XCOPY "COPYING.txt" ".\%COPY_TO_DIR%\" /Y /V

IF /I "%Platform%" == "x64" GOTO skipx86installer
IF DEFINED InnoSetupPath (
  TITLE Compiling x86 installer MSVC 2008...
  "%InnoSetupPath%\iscc.exe" /Q /O"bin" "distrib\mpc-hc_setup.iss"
  IF %ERRORLEVEL% NEQ 0 GOTO EndWithError
) ELSE (
  GOTO END
)
EXIT /B


:skipx86installer
IF /I "%Platform%" == "Win32" GOTO END
IF DEFINED InnoSetupPath (
  TITLE Compiling x64 installer MSVC 2008...
  "%InnoSetupPath%\iscc.exe" /Q /O"bin" "distrib\mpc-hc_setup.iss" /Dx64Build
  IF %ERRORLEVEL% NEQ 0 GOTO EndWithError
) ELSE (
  GOTO END
)
EXIT /B


:SubMPCRES
TITLE Compiling mpcresources with MSVC 2008 - %~1^|%Platform%...
devenv /nologo mpcresources.sln /%BUILDTYPE% "Release %~1|%Platform%"
IF %ERRORLEVEL% NEQ 0 GOTO EndWithError
EXIT /B


:SubIS
SET InnoSetupPath=%*
EXIT /B
