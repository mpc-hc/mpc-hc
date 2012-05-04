@ECHO OFF
REM $Id$
REM
REM (C) 2009-2012 see Authors.txt
REM
REM This file is part of MPC-HC.
REM
REM MPC-HC is free software; you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation; either version 3 of the License, or
REM (at your option) any later version.
REM
REM MPC-HC is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM
REM You should have received a copy of the GNU General Public License
REM along with this program.  If not, see <http://www.gnu.org/licenses/>.


SETLOCAL
CD /D %~dp0
REM Check if the %LOG_DIR% folder exists otherwise MSBuild will fail
SET "LOG_DIR=bin\logs"
IF NOT EXIST "%LOG_DIR%" MD "%LOG_DIR%"

SET "MSBUILD=%WINDIR%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe"
SET "MSBUILD_SWITCHES=/nologo /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true"


REM pre-build checks
IF "%VS100COMNTOOLS%" == "" GOTO MissingVar
IF "%MINGW32%" == ""        GOTO MissingVar
IF "%MINGW64%" == ""        GOTO MissingVar


SET ARG=%*
SET ARG=%ARG:/=%
SET ARG=%ARG:-=%
SET ARGB=0
SET ARGBC=0
SET ARGC=0
SET ARGP=0
SET INPUT=0

IF /I "%ARG%" == "?"        GOTO ShowHelp

FOR %%A IN (%ARG%) DO (
  IF /I "%%A" == "help"     GOTO ShowHelp
  IF /I "%%A" == "Build"    SET "BUILDTYPE=Build"   & Set /A ARGB+=1
  IF /I "%%A" == "Clean"    SET "BUILDTYPE=Clean"   & Set /A ARGB+=1
  IF /I "%%A" == "Rebuild"  SET "BUILDTYPE=Rebuild" & Set /A ARGB+=1
  IF /I "%%A" == "Both"     SET "PLATFORM=Both"     & Set /A ARGP+=1
  IF /I "%%A" == "Win32"    SET "PLATFORM=Win32"    & Set /A ARGP+=1
  IF /I "%%A" == "x86"      SET "PLATFORM=Win32"    & Set /A ARGP+=1
  IF /I "%%A" == "x64"      SET "PLATFORM=x64"      & Set /A ARGP+=1
  IF /I "%%A" == "All"      SET "CONFIG=All"        & Set /A ARGC+=1
  IF /I "%%A" == "Main"     SET "CONFIG=Main"       & Set /A ARGC+=1
  IF /I "%%A" == "Filters"  SET "CONFIG=Filters"    & Set /A ARGC+=1
  IF /I "%%A" == "MPCHC"    SET "CONFIG=MPCHC"      & Set /A ARGC+=1
  IF /I "%%A" == "Resources" SET "CONFIG=Resources" & Set /A ARGC+=1
  IF /I "%%A" == "Debug"    SET "BUILDCFG=Debug"    & Set /A ARGBC+=1
  IF /I "%%A" == "Release"  SET "BUILDCFG=Release"  & Set /A ARGBC+=1
)

FOR %%X IN (%*) DO SET /A INPUT+=1
SET /A VALID=%ARGB%+%ARGP%+%ARGC%+%ARGBC%

IF %VALID% NEQ %INPUT% GOTO UnsupportedSwitch

IF %ARGB%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGB% == 0  (SET "BUILDTYPE=Build")
IF %ARGP%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGP% == 0  (SET "PLATFORM=Both")
IF %ARGC%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGC% == 0  (SET "CONFIG=MPCHC")
IF %ARGBC% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGBC% == 0 (SET "BUILDCFG=Release")

GOTO Start


:UnsupportedSwitch
ECHO.
ECHO Unsupported commandline switch!
ECHO Run "%~nx0 help" for details about the commandline switches.
CALL :SubMsg "ERROR" "Compilation failed!"


:Start
SET START_TIME=%TIME%
SET START_DATE=%DATE%
IF "%PLATFORM%" == "Win32" GOTO Win32
IF "%PLATFORM%" == "x64"   GOTO x64


:Win32
CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86

IF "%CONFIG%" == "Filters"  CALL :SubFilters Win32 && GOTO x64
IF "%CONFIG%" == "Resources" CALL :SubResources Win32 && GOTO x64
CALL :SubMPCHC Win32
IF "%CONFIG%" == "Main" GOTO x64

CALL :SubResources Win32
CALL :SubCreatePackages Win32
IF "%CONFIG%" == "All"  CALL :SubFilters Win32


:x64
IF "%PLATFORM%" == "Win32" GOTO End

IF DEFINED PROGRAMFILES(x86) (SET x64_type=amd64) ELSE (SET x64_type=x86_amd64)
CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" %x64_type%

IF "%CONFIG%" == "Filters"   CALL :SubFilters x64 && GOTO END
IF "%CONFIG%" == "Resources" CALL :SubResources x64 && GOTO END
CALL :SubMPCHC x64
IF "%CONFIG%" == "Main" GOTO End

CALL :SubResources x64
CALL :SubCreatePackages x64
IF "%CONFIG%" == "All"  CALL :SubFilters x64


:End
TITLE Compiling MPC-HC [FINISHED]
SET END_TIME=%TIME%
CALL :SubGetDuration
CALL :SubMsg "INFO" "Compilation started on %START_DATE%-%START_TIME% and completed on %DATE%-%END_TIME% [%DURATION%]"
ENDLOCAL
EXIT /B


:SubFilters
TITLE Compiling MPC-HC Filters - %BUILDCFG% Filter^|%1...
REM Call update_version.bat before building the filters
CALL "update_version.bat"
"%MSBUILD%" mpc-hc.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration="%BUILDCFG% Filter";Platform=%1^
 /flp1:LogFile=%LOG_DIR%\filters_errors_%BUILDCFG%_%1.log;errorsonly;Verbosity=diagnostic^
 /flp2:LogFile=%LOG_DIR%\filters_warnings_%BUILDCFG%_%1.log;warningsonly;Verbosity=diagnostic
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpc-hc.sln %BUILDCFG% Filter %1 - Compilation failed!"
) ELSE (
  CALL :SubMsg "INFO" "mpc-hc.sln %BUILDCFG% Filter %1 compiled successfully"
)
EXIT /B


:SubMPCHC
TITLE Compiling MPC-HC - %BUILDCFG%^|%1...
"%MSBUILD%" mpc-hc.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration=%BUILDCFG%;Platform=%1^
 /flp1:LogFile=%LOG_DIR%\mpc-hc_errors_%BUILDCFG%_%1.log;errorsonly;Verbosity=diagnostic^
 /flp2:LogFile=%LOG_DIR%\mpc-hc_warnings_%BUILDCFG%_%1.log;warningsonly;Verbosity=diagnostic
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpc-hc.sln %BUILDCFG% %1 - Compilation failed!"
) ELSE (
  CALL :SubMsg "INFO" "mpc-hc.sln %BUILDCFG% %1 compiled successfully"
)
EXIT /B


:SubResources
TITLE Compiling mpciconlib - Release^|%1...
"%MSBUILD%" mpciconlib.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration=Release;Platform=%1
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpciconlib.sln %1 - Compilation failed!"
) ELSE (
  CALL :SubMsg "INFO" "mpciconlib.sln %1 compiled successfully"
)

FOR %%A IN ("Armenian" "Basque" "Belarusian" "Catalan" "Chinese Simplified" "Chinese Traditional"
 "Czech" "Dutch" "French" "German" "Hebrew" "Hungarian" "Italian" "Japanese" "Korean"
 "Polish" "Portuguese" "Russian" "Slovak" "Spanish" "Swedish" "Turkish" "Ukrainian"
) DO (
 TITLE Compiling mpcresources - %%~A^|%1...
 "%MSBUILD%" mpcresources.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration="Release %%~A";Platform=%1
 IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!"
)
EXIT /B


:SubCreatePackages
IF "%BUILDTYPE%" == "Clean" EXIT /B
IF "%BUILDCFG%" == "Debug"  EXIT /B
IF "%CONFIG%" == "Filters"  EXIT /B

IF "%~1" == "x64" SET ISDefs=/Dx64Build

CALL :SubDetectInnoSetup

IF DEFINED InnoSetupPath (
  TITLE Compiling %1 installer...
  "%InnoSetupPath%\iscc.exe" /Q /O"bin" "distrib\mpc-hc_setup.iss" %ISDefs%
  IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!"
  CALL :SubMsg "INFO" "%1 installer successfully built"
) ELSE (
  CALL :SubMsg "WARNING" "Inno Setup wasn't found, the %1 installer wasn't built"
)
EXIT /B


:ShowHelp
TITLE %~nx0 %1
ECHO. & ECHO.
ECHO Usage:
ECHO %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|Both] [Main^|Resources^|MPCHC^|Filters^|All] [Debug^|Release]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        The arguments are not case sensitive.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments will use the default ones:
ECHO "%~nx0 Build Both MPCHC Release"
ECHO.
ECHO Examples:
ECHO %~nx0 x86 Resources  -Builds the x86 resources
ECHO %~nx0 Resources      -Builds both x86 and x64 resources
ECHO %~nx0 x86            -Builds x86 Main exe and the x86 resources
ECHO %~nx0 x86 Debug      -Builds x86 Main Debug exe and x86 resources
ECHO %~nx0 x86 Filters    -Builds x86 Filters
ECHO %~nx0 x86 All        -Builds x86 Main exe, x86 Filters and the x86 resources
ECHO.
ECHO NOTES:
ECHO Debug only applies to mpc-hc.sln
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
ECHO See "docs\Compilation.txt" for more information.
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

FOR /F "delims=" %%A IN (
  'REG QUERY "%U_%\Inno Setup 5_is1" /v "Inno Setup: App Path"2^>Nul^|FIND "REG_"') DO (
  SET "InnoSetupPath=%%A" & CALL :SubInnoSetupPath %%InnoSetupPath:*Z=%%)
EXIT /B


:SubInnoSetupPath
SET InnoSetupPath=%*
EXIT /B


:SubMsg
ECHO. & ECHO ------------------------------
IF /I "%~1" == "ERROR" (
  CALL :SubColorText "0C" "[%~1]" & ECHO  %~2
) ELSE IF /I "%~1" == "INFO" (
  CALL :SubColorText "0A" "[%~1]" & ECHO  %~2
) ELSE IF /I "%~1" == "WARNING" (
  CALL :SubColorText "0E" "[%~1]" & ECHO  %~2
)
ECHO ------------------------------ & ECHO.
IF /I "%~1" == "ERROR" (
  ECHO Press any key to exit...
  PAUSE >NUL
  ENDLOCAL
  EXIT
) ELSE (
  EXIT /B
)


:SubColorText
FOR /F "tokens=1,2 delims=#" %%A IN (
  '"PROMPT #$H#$E# & ECHO ON & FOR %%B IN (1) DO REM"') DO (
  SET "DEL=%%A")
<NUL SET /p ".=%DEL%" > "%~2"
FINDSTR /v /a:%1 /R ".18" "%~2" NUL
DEL "%~2" > NUL 2>&1
EXIT /B


:SubGetDuration
SET START_TIME=%START_TIME: =%
SET END_TIME=%END_TIME: =%

FOR /F "tokens=1-4 delims=:.," %%A IN ("%START_TIME%") DO (
  SET /A "STARTTIME=(100%%A %% 100) * 360000 + (100%%B %% 100) * 6000 + (100%%C %% 100) * 100 + (100%%D %% 100)"
)

FOR /F "tokens=1-4 delims=:.," %%A IN ("%END_TIME%") DO (
  SET /A "ENDTIME=(100%%A %% 100) * 360000 + (100%%B %% 100) * 6000 + (100%%C %% 100) * 100 + (100%%D %% 100)"
)

SET /A DURATION=%ENDTIME%-%STARTTIME%
IF %ENDTIME% LSS %STARTTIME% SET /A "DURATION+=24 * 360000"

SET /A DURATIONH=%DURATION% / 360000
SET /A DURATIONM=(%DURATION% - %DURATIONH%*360000) / 6000
SET /A DURATIONS=(%DURATION% - %DURATIONH%*360000 - %DURATIONM%*6000) / 100
SET /A DURATIONHS=(%DURATION% - %DURATIONH%*360000 - %DURATIONM%*6000 - %DURATIONS%*100)*10

IF %DURATIONH%  EQU 0 (SET DURATIONH=)  ELSE (SET DURATIONH=%DURATIONH%h )
IF %DURATIONM%  EQU 0 (SET DURATIONM=)  ELSE (SET DURATIONM=%DURATIONM%m )
IF %DURATIONS%  EQU 0 (SET DURATIONS=)  ELSE (SET DURATIONS=%DURATIONS%s )
IF %DURATIONHS% EQU 0 (SET DURATIONHS=) ELSE (SET DURATIONHS=%DURATIONHS%ms)

SET "DURATION=%DURATIONH%%DURATIONM%%DURATIONS%%DURATIONHS%"
EXIT /B
