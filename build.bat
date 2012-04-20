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
  IF /I "%%A" == "Build"    SET "BUILDTYPE=Build"     & Set /A ARGB+=1
  IF /I "%%A" == "Clean"    SET "BUILDTYPE=Clean"     & Set /A ARGB+=1
  IF /I "%%A" == "Rebuild"  SET "BUILDTYPE=Rebuild"   & Set /A ARGB+=1
  IF /I "%%A" == "Both"     SET "PLATFORM=Both"       & Set /A ARGP+=1
  IF /I "%%A" == "x86"      SET "PLATFORM=Win32"      & Set /A ARGP+=1
  IF /I "%%A" == "x64"      SET "PLATFORM=x64"        & Set /A ARGP+=1
  IF /I "%%A" == "All"      SET "CONFIG=All"          & Set /A ARGC+=1
  IF /I "%%A" == "Main"     SET "CONFIG=Main"         & Set /A ARGC+=1
  IF /I "%%A" == "Filters"  SET "CONFIG=Filters"      & Set /A ARGC+=1
  IF /I "%%A" == "MPCHC"    SET "CONFIG=MPCHC"        & Set /A ARGC+=1
  IF /I "%%A" == "Resource" SET "CONFIG=Resource"     & Set /A ARGC+=1
  IF /I "%%A" == "Debug"    SET "BUILDCONFIG=Debug"   & Set /A ARGBC+=1
  IF /I "%%A" == "Release"  SET "BUILDCONFIG=Release" & Set /A ARGBC+=1
)

FOR %%X IN (%*) DO SET /A INPUT+=1
SET /A VALID=%ARGB%+%ARGP%+%ARGC%+%ARGBC%

IF %VALID% NEQ %INPUT% GOTO UnsupportedSwitch

IF %ARGB%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGB% == 0  (SET "BUILDTYPE=Build")
IF %ARGP%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGP% == 0  (SET "PLATFORM=Both")
IF %ARGC%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGC% == 0  (SET "CONFIG=MPCHC")
IF %ARGBC% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGBC% == 0 (SET "BUILDCONFIG=Release")

GOTO Start


:UnsupportedSwitch
ECHO.
ECHO Unsupported commandline switch!
ECHO Run "%~nx0 help" for details about the commandline switches.
CALL :SubMsg "ERROR" "Compilation failed!"


:Start
SET START_TIME=%DATE%-%TIME%
IF "%PLATFORM%" == "Win32" GOTO Win32
IF "%PLATFORM%" == "x64"   GOTO x64


:Win32
CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86

IF "%CONFIG%" == "Filters"  CALL :SubFilters Win32 && GOTO x64
IF "%CONFIG%" == "Resource" CALL :SubResources Win32 && GOTO x64
CALL :SubMPCHC Win32
IF "%CONFIG%" == "Main" GOTO x64

CALL :SubResources Win32
CALL :SubCreatePackages Win32
IF "%CONFIG%" == "All"  CALL :SubFilters Win32


:x64
IF "%PLATFORM%" == "Win32" GOTO End

IF DEFINED PROGRAMFILES(x86) (SET x64_type=amd64) ELSE (SET x64_type=x86_amd64)
CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" %x64_type%

IF "%CONFIG%" == "Filters"  CALL :SubFilters x64 && GOTO END
IF "%CONFIG%" == "Resource" CALL :SubResources x64 && GOTO END
CALL :SubMPCHC x64
IF "%CONFIG%" == "Main" GOTO End

CALL :SubResources x64
CALL :SubCreatePackages x64
IF "%CONFIG%" == "All"  CALL :SubFilters Win32


:End
TITLE Compiling MPC-HC [FINISHED]
CALL :SubMsg "INFO" "Compilation started on %START_TIME% and completed on %DATE%-%TIME%"
ENDLOCAL
EXIT /B


:SubFilters
TITLE Compiling MPC-HC Filters - %BUILDCONFIG% Filter^|%1...
REM Call update_version.bat before building the filters
CALL "update_version.bat"
"%WINDIR%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe" /nologo filters.sln^
 /target:%BUILDTYPE% /property:Configuration="%BUILDCONFIG% Filter";Platform=%1^
 /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true^
 /flp1:LogFile=%LOG_DIR%\filters_errors_%BUILDCONFIG%_%1.txt;errorsonly;Verbosity=diagnostic^
 /flp2:LogFile=%LOG_DIR%\filters_warnings_%BUILDCONFIG%_%1.txt;warningsonly;Verbosity=diagnostic
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "filters.sln - Compilation failed!"
) ELSE (
  CALL :SubMsg "INFO" "filters.sln %BUILDCONFIG% %1 compiled successfully"
)
EXIT /B


:SubMPCHC
TITLE Compiling MPC-HC - %BUILDCONFIG%^|%1...
"%WINDIR%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe" /nologo mpc-hc.sln^
 /target:%BUILDTYPE% /property:Configuration=%BUILDCONFIG%;Platform=%1^
 /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true^
 /flp1:LogFile=%LOG_DIR%\mpc_errors_%BUILDCONFIG%_%1.txt;errorsonly;Verbosity=diagnostic^
 /flp2:LogFile=%LOG_DIR%\mpc_warnings_%BUILDCONFIG%_%1.txt;warningsonly;Verbosity=diagnostic
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpc-hc.sln - Compilation failed!"
) ELSE (
  CALL :SubMsg "INFO" "mpc-hc.sln %BUILDCONFIG% %1 compiled successfully"
)
EXIT /B


:SubResources
TITLE Compiling mpciconlib - Release^|%1...
"%WINDIR%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe" /nologo mpciconlib.sln^
 /target:%BUILDTYPE% /property:Configuration=Release;Platform=%1^
 /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpciconlib.sln - Compilation failed!"
) ELSE (
  CALL :SubMsg "INFO" "mpciconlib.sln %1 compiled successfully"
)

FOR %%A IN ("Armenian" "Belarusian" "Catalan" "Chinese Simplified" "Chinese Traditional"
 "Czech" "Dutch" "French" "German" "Hebrew" "Hungarian" "Italian" "Japanese" "Korean"
 "Polish" "Portuguese" "Russian" "Slovak" "Spanish" "Swedish" "Turkish" "Ukrainian"
) DO (
 TITLE Compiling mpcresources - %%~A^|%1...
 "%WINDIR%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe" /nologo mpcresources.sln^
  /target:%BUILDTYPE% /property:Configuration="Release %%~A";Platform=%1^
  /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true
 IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!"
)
EXIT /B


:SubCreatePackages
IF "%BUILDTYPE%" == "Clean"   EXIT /B
IF "%BUILDCONFIG%" == "Debug" EXIT /B
IF "%CONFIG%" == "Filters"    EXIT /B

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
ECHO %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|Both] [Main^|Resource^|MPCHC^|Filters^|All] [Debug^|Release]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        The arguments are not case sensitive.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments will use the default ones:
ECHO "%~nx0 Build Both MPCHC Release"
ECHO.
ECHO Examples:
ECHO %~nx0 x86 Resource   -Builds the x86 resources
ECHO %~nx0 Resource       -Builds both x86 and x64 resources
ECHO %~nx0 x86            -Builds x86 Main exe and the x86 resources
ECHO %~nx0 x86 Debug      -Builds x86 Main Debug exe and x86 resources
ECHO %~nx0 x86 Filters    -Builds x86 Filters
ECHO %~nx0 x86 All        -Builds x86 Main exe, x86 Filters and the x86 resources
ECHO.
ECHO NOTES:
ECHO Debug only applies to mpc-hc.sln and filters.sln
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
  CALL :ColorText "0C" "[%~1]" & ECHO  %~2
) ELSE IF /I "%~1" == "INFO" (
  CALL :ColorText "0A" "[%~1]" & ECHO  %~2
) ELSE IF /I "%~1" == "WARNING" (
  CALL :ColorText "0E" "[%~1]" & ECHO  %~2
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


:ColorText
FOR /F "tokens=1,2 delims=#" %%A IN (
  '"PROMPT #$H#$E# & ECHO ON & FOR %%B IN (1) DO REM"') DO (
  SET "DEL=%%A")
<NUL SET /p ".=%DEL%" > "%~2"
FINDSTR /v /a:%1 /R ".18" "%~2" NUL
DEL "%~2" > NUL 2>&1
