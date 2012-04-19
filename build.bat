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


CLS
SETLOCAL
CD /D %~dp0
REM Check if the bin folder exists otherwise MSBuild will fail
IF NOT EXIST "bin" MD "bin"

IF /I "%~1" == "help"   GOTO ShowHelp
IF /I "%~1" == "/help"  GOTO ShowHelp
IF /I "%~1" == "-help"  GOTO ShowHelp
IF /I "%~1" == "--help" GOTO ShowHelp
IF /I "%~1" == "/?"     GOTO ShowHelp


REM pre-build checks
IF "%VS100COMNTOOLS%" == "" GOTO MissingVar
IF "%MINGW32%" == ""        GOTO MissingVar
IF "%MINGW64%" == ""        GOTO MissingVar


REM Check for the first switch
IF "%~1" == "" (
  SET "BUILDTYPE=Build"
) ELSE (
  IF /I "%~1" == "Build"     SET "BUILDTYPE=Build"   & GOTO CheckSecondArg
  IF /I "%~1" == "/Build"    SET "BUILDTYPE=Build"   & GOTO CheckSecondArg
  IF /I "%~1" == "-Build"    SET "BUILDTYPE=Build"   & GOTO CheckSecondArg
  IF /I "%~1" == "--Build"   SET "BUILDTYPE=Build"   & GOTO CheckSecondArg
  IF /I "%~1" == "Clean"     SET "BUILDTYPE=Clean"   & GOTO CheckSecondArg
  IF /I "%~1" == "/Clean"    SET "BUILDTYPE=Clean"   & GOTO CheckSecondArg
  IF /I "%~1" == "-Clean"    SET "BUILDTYPE=Clean"   & GOTO CheckSecondArg
  IF /I "%~1" == "--Clean"   SET "BUILDTYPE=Clean"   & GOTO CheckSecondArg
  IF /I "%~1" == "Rebuild"   SET "BUILDTYPE=Rebuild" & GOTO CheckSecondArg
  IF /I "%~1" == "/Rebuild"  SET "BUILDTYPE=Rebuild" & GOTO CheckSecondArg
  IF /I "%~1" == "-Rebuild"  SET "BUILDTYPE=Rebuild" & GOTO CheckSecondArg
  IF /I "%~1" == "--Rebuild" SET "BUILDTYPE=Rebuild" & GOTO CheckSecondArg

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SubMsg "ERROR" "Compilation failed!"
)


:CheckSecondArg
REM Check for the second switch
IF "%~2" == "" (
  SET "PLATFORM=all"
) ELSE (
  IF /I "%~2" == "x86"   SET "PLATFORM=Win32" & GOTO CheckThirdArg
  IF /I "%~2" == "/x86"  SET "PLATFORM=Win32" & GOTO CheckThirdArg
  IF /I "%~2" == "-x86"  SET "PLATFORM=Win32" & GOTO CheckThirdArg
  IF /I "%~2" == "--x86" SET "PLATFORM=Win32" & GOTO CheckThirdArg
  IF /I "%~2" == "x64"   SET "PLATFORM=x64"   & GOTO CheckThirdArg
  IF /I "%~2" == "/x64"  SET "PLATFORM=x64"   & GOTO CheckThirdArg
  IF /I "%~2" == "-x64"  SET "PLATFORM=x64"   & GOTO CheckThirdArg
  IF /I "%~2" == "--x64" SET "PLATFORM=x64"   & GOTO CheckThirdArg
  IF /I "%~2" == "all"   SET "PLATFORM=all"   & GOTO CheckThirdArg
  IF /I "%~2" == "/all"  SET "PLATFORM=all"   & GOTO CheckThirdArg
  IF /I "%~2" == "-all"  SET "PLATFORM=all"   & GOTO CheckThirdArg
  IF /I "%~2" == "--all" SET "PLATFORM=all"   & GOTO CheckThirdArg

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SubMsg "ERROR" "Compilation failed!"
)


:CheckThirdArg
REM Check for the third switch
IF "%~3" == "" (
  SET "CONFIG=all"
) ELSE (
  IF /I "%~3" == "Main"       SET "CONFIG=Main"     & GOTO CheckFourthArg
  IF /I "%~3" == "/Main"      SET "CONFIG=Main"     & GOTO CheckFourthArg
  IF /I "%~3" == "-Main"      SET "CONFIG=Main"     & GOTO CheckFourthArg
  IF /I "%~3" == "--Main"     SET "CONFIG=Main"     & GOTO CheckFourthArg
  IF /I "%~3" == "Resource"   SET "CONFIG=Resource" & GOTO CheckFourthArg
  IF /I "%~3" == "/Resource"  SET "CONFIG=Resource" & GOTO CheckFourthArg
  IF /I "%~3" == "-Resource"  SET "CONFIG=Resource" & GOTO CheckFourthArg
  IF /I "%~3" == "--Resource" SET "CONFIG=Resource" & GOTO CheckFourthArg
  IF /I "%~3" == "all"        SET "CONFIG=all"      & GOTO CheckFourthArg
  IF /I "%~3" == "/all"       SET "CONFIG=all"      & GOTO CheckFourthArg
  IF /I "%~3" == "-all"       SET "CONFIG=all"      & GOTO CheckFourthArg
  IF /I "%~3" == "--all"      SET "CONFIG=all"      & GOTO CheckFourthArg

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SubMsg "ERROR" "Compilation failed!"
)

:CheckFourthArg
REM Check for the fourth switch
IF "%~4" == "" (
  SET "BUILDCONFIG=Release"
) ELSE (
  IF /I "%~4" == "Debug"   SET "BUILDCONFIG=Debug" & GOTO Start
  IF /I "%~4" == "/Debug"  SET "BUILDCONFIG=Debug" & GOTO Start
  IF /I "%~4" == "-Debug"  SET "BUILDCONFIG=Debug" & GOTO Start
  IF /I "%~4" == "--Debug" SET "BUILDCONFIG=Debug" & GOTO Start

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SubMsg "ERROR" "Compilation failed!"
)


:Start
SET START_TIME=%DATE%-%TIME%
IF "%PLATFORM%" == "Win32" GOTO Win32
IF "%PLATFORM%" == "x64"   GOTO x64


:Win32
CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86

IF "%CONFIG%" == "Resource" CALL :SubResources Win32 && GOTO x64
CALL :SubMPCHC Win32
IF "%CONFIG%" == "Main" GOTO x64

CALL :SubResources Win32
CALL :SubCreatePackages Win32


:x64
IF "%PLATFORM%" == "Win32" GOTO End

IF DEFINED PROGRAMFILES(x86) (SET build_type=amd64) ELSE (SET build_type=x86_amd64)
CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" %build_type%

IF "%CONFIG%" == "Resource" CALL :SubResources x64 && GOTO END
CALL :SubMPCHC x64
IF "%CONFIG%" == "Main" GOTO End

CALL :SubResources x64
CALL :SubCreatePackages x64


:End
TITLE Compiling MPC-HC [FINISHED]
CALL :SubMsg "INFO" "MPC-HC's compilation started on %START_TIME% and completed on %DATE%-%TIME%"
ENDLOCAL
EXIT /B


:SubMPCHC
TITLE Compiling MPC-HC - %BUILDCONFIG%^|%1...
"%WINDIR%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe" /nologo mpc-hc.sln^
 /target:%BUILDTYPE% /property:Configuration=%BUILDCONFIG%;Platform=%1^
 /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true^
 /flp1:LogFile=bin\errors_%1.txt;errorsonly;Verbosity=diagnostic^
 /flp2:LogFile=bin\warnings_%1.txt;warningsonly;Verbosity=diagnostic
IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!"
EXIT /B


:SubResources
TITLE Compiling mpciconlib - Release^|%1...
"%WINDIR%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe" /nologo mpciconlib.sln^
 /target:%BUILDTYPE% /property:Configuration=Release;Platform=%1^
 /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true
IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!"

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

IF "%~1" == "Win32" SET OUTDIR=bin\mpc-hc_x86
IF "%~1" == "x64"   SET OUTDIR=bin\mpc-hc_x64 & SET ISDefs=/Dx64Build

XCOPY "COPYING.txt"        "%OUTDIR%\" /Y /V >NUL
XCOPY "docs\Authors.txt"   "%OUTDIR%\" /Y /V >NUL
XCOPY "docs\Changelog.txt" "%OUTDIR%\" /Y /V >NUL
XCOPY "docs\Readme.txt"    "%OUTDIR%\" /Y /V >NUL

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
ECHO %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|all] [Main^|Resource^|all] [Debug]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        The arguments are not case sensitive.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments will use the default ones:
ECHO "%~nx0 build all all"
ECHO.
ECHO Examples:
ECHO %~nx0 build x86 Resource     -Builds the x86 resources only
ECHO %~nx0 build all Resource     -Builds both x86 and x64 resources only
ECHO %~nx0 build x86              -Builds x86 Main exe and the resources
ECHO %~nx0 build x86 all Debug    -Builds x86 Main Debug exe and resources
ECHO.
ECHO NOTES:
ECHO Debug only applies to Main project [mpc-hc.sln]
ECHO "%~nx0 x86" or "%~nx0 debug" won't work.
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

FOR /F "delims=" %%a IN (
  'REG QUERY "%U_%\Inno Setup 5_is1" /v "Inno Setup: App Path"2^>Nul^|FIND "REG_"') DO (
  SET "InnoSetupPath=%%a" & CALL :SubInnoSetupPath %%InnoSetupPath:*Z=%%)
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
  PAUSE
  ENDLOCAL
  EXIT
) ELSE (
  EXIT /B
)


:ColorText
FOR /F "tokens=1,2 delims=#" %%a IN (
  '"PROMPT #$H#$E# & ECHO ON & FOR %%b IN (1) DO REM"') DO (
  SET "DEL=%%a")
<NUL SET /p ".=%DEL%" > "%~2"
FINDSTR /v /a:%1 /R ".18" "%~2" NUL
DEL "%~2" > NUL 2>&1
