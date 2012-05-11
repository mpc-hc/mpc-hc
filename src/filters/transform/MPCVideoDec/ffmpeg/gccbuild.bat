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

IF EXIST "..\..\..\..\..\build.user.bat" (
  CALL "..\..\..\..\..\build.user.bat"
) ELSE (
  IF DEFINED MINGW32 (SET MPCHC_MINGW32=%MINGW32%) ELSE (GOTO MissingVar)
  IF DEFINED MINGW64 (SET MPCHC_MINGW64=%MINGW64%) ELSE (GOTO MissingVar)
  IF DEFINED MSYS    (SET MPCHC_MSYS=%MSYS%)       ELSE (GOTO MissingVar)
)

SET PATH=%MPCHC_MSYS%\bin;%MPCHC_MINGW32%\bin;%PATH%
FOR %%X IN (gcc.exe) DO (SET FOUND=%%~$PATH:X)
IF NOT DEFINED FOUND GOTO MissingVar

PUSHD %~dp0

SET ARG=%*
SET ARG=%ARG:/=%
SET ARG=%ARG:-=%
SET ARGB=0
SET ARGBC=0
SET ARGPL=0
SET INPUT=0

IF /I "%ARG%" == "?"          GOTO ShowHelp

FOR %%A IN (%ARG%) DO (
  IF /I "%%A" == "help"       GOTO ShowHelp
  IF /I "%%A" == "Build"      SET "BUILDTYPE=Build"   & SET /A ARGB+=1
  IF /I "%%A" == "Clean"      SET "BUILDTYPE=Clean"   & SET /A ARGB+=1
  IF /I "%%A" == "Rebuild"    SET "BUILDTYPE=Rebuild" & SET /A ARGB+=1
  IF /I "%%A" == "Both"       SET "ARCH=Both"         & SET /A ARGPL+=1
  IF /I "%%A" == "Win32"      SET "ARCH=x86"          & SET /A ARGPL+=1
  IF /I "%%A" == "x86"        SET "ARCH=x86"          & SET /A ARGPL+=1
  IF /I "%%A" == "x64"        SET "ARCH=x64"          & SET /A ARGPL+=1
  IF /I "%%A" == "Debug"      SET "DEBUG=DEBUG=yes"   & SET /A ARGBC+=1
  IF /I "%%A" == "Release"    SET "DEBUG="            & SET /A ARGBC+=1
)

FOR %%X IN (%*) DO SET /A INPUT+=1
SET /A VALID=%ARGB%+%ARGPL%+%ARGBC%

IF %VALID% NEQ %INPUT% GOTO UnsupportedSwitch

IF %ARGB%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGB% == 0  (SET "BUILDTYPE=Build")
IF %ARGPL% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGPL% == 0 (SET "ARCH=Both")
IF %ARGBC% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGBC% == 0 (SET "DEBUG=")


IF /I "%ARCH%" == "Both" (
  SET "ARCH=x86" & CALL :Main
  SET "ARCH=x64" & CALL :Main
  GOTO End
)


:Main
IF /I "%ARCH%" == "x64" (SET "x64=64BIT=yes") ELSE (SET "x64=")

IF /I "%BUILDTYPE%" == "Rebuild" (
  SET "BUILDTYPE=Clean" & CALL :SubMake %x64% %DEBUG% clean
  SET "BUILDTYPE=Build" & CALL :SubMake %x64% %DEBUG%
  SET "BUILDTYPE=Rebuild"
  EXIT /B
)

IF /I "%BUILDTYPE%" == "Clean" (CALL :SubMake %x64% %DEBUG% clean & EXIT /B)

CALL :SubMake %x64% %DEBUG%
EXIT /B


:End
TITLE Compiling FFmpeg [FINISHED]
POPD
ENDLOCAL
EXIT /B


:SubMake
IF DEFINED NUMBER_OF_PROCESSORS (SET JOBS=%NUMBER_OF_PROCESSORS%) ELSE (SET JOBS=4)
IF /I "%BUILDTYPE%" == "Clean"  (SET JOBS=1)

TITLE make -j%JOBS% %*
ECHO make -j%JOBS% %*
make.exe -j%JOBS% %*
EXIT /B


:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "docs\Compilation.txt" for more information.
ENDLOCAL
EXIT /B


:UnsupportedSwitch
ECHO.
ECHO Unsupported commandline switch!
ECHO Run "%~nx0 help" for details about the commandline switches.
ENDLOCAL
EXIT /B


:ShowHelp
TITLE %~nx0 Help
ECHO.
ECHO Usage:
ECHO %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|Both] [Debug^|Release]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        The arguments are not case sensitive and can be ommitted.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments will use the default ones:
ECHO "%~nx0 Build Both Release"
ECHO.
POPD
ENDLOCAL
EXIT /B
