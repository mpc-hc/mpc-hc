@ECHO OFF
REM (C) 2013-2015 see Authors.txt
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
PUSHD %~dp0

SET ROOT_DIR=..\..\..
SET "COMMON=%~dp0%ROOT_DIR%\common.bat"

CALL %COMMON% :SubSetPath
IF %ERRORLEVEL% NEQ 0 EXIT /B
CALL %COMMON% :SubDoesExist gcc.exe
IF %ERRORLEVEL% NEQ 0 EXIT /B

SET ARG=/%*
SET ARG=%ARG:/=%
SET ARG=%ARG:-=%
SET ARGB=0
SET ARGBC=0
SET ARGCOMP=0
SET ARGPL=0
SET INPUT=0
SET VALID=0

IF /I "%ARG%" == "?"        GOTO ShowHelp

FOR %%G IN (%ARG%) DO (
  IF /I "%%G" == "help"     GOTO ShowHelp
  IF /I "%%G" == "Build"    SET "BUILDTYPE=Build"     & SET /A ARGB+=1
  IF /I "%%G" == "Clean"    SET "BUILDTYPE=Clean"     & SET /A ARGB+=1
  IF /I "%%G" == "Rebuild"  SET "BUILDTYPE=Rebuild"   & SET /A ARGB+=1
  IF /I "%%G" == "Both"     SET "ARCH=Both"           & SET /A ARGPL+=1
  IF /I "%%G" == "Win32"    SET "ARCH=x86"            & SET /A ARGPL+=1
  IF /I "%%G" == "x86"      SET "ARCH=x86"            & SET /A ARGPL+=1
  IF /I "%%G" == "x64"      SET "ARCH=x64"            & SET /A ARGPL+=1
  IF /I "%%G" == "Debug"    SET "RELEASETYPE=Debug"   & SET /A ARGBC+=1
  IF /I "%%G" == "Release"  SET "RELEASETYPE=Release" & SET /A ARGBC+=1
  IF /I "%%G" == "VS2013"   SET "COMPILER=VS2013"     & SET /A ARGCOMP+=1
  IF /I "%%G" == "Silent"   SET "SILENT=True"         & SET /A VALID+=1
  IF /I "%%G" == "Nocolors" SET "NOCOLORS=True"       & SET /A VALID+=1
)

FOR %%X IN (%*) DO SET /A INPUT+=1
SET /A VALID+=%ARGB%+%ARGPL%+%ARGBC%+%ARGCOMP%

IF %VALID% NEQ %INPUT% GOTO UnsupportedSwitch

IF %ARGB%    GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGB% == 0    (SET "BUILDTYPE=Build")
IF %ARGPL%   GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGPL% == 0   (SET "ARCH=Both")
IF %ARGBC%   GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGBC% == 0   (SET "RELEASETYPE=Release")
IF %ARGCOMP% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGCOMP% == 0 (SET "COMPILER=VS2013")

IF NOT DEFINED VS120COMNTOOLS GOTO MissingVar
SET "TOOLSET=%VS120COMNTOOLS%..\..\VC\vcvarsall.bat"
SET "BIN_DIR=%ROOT_DIR%\bin"

IF /I "%ARCH%" == "Both" (
  SET "ARCH=x86" & CALL :Main
  SET "ARCH=x64" & CALL :Main
) ELSE (
  CALL :Main
)
GOTO End


:Main
IF %ERRORLEVEL% NEQ 0 EXIT /B

REM Always use x86_amd64 compiler, even on 64bit windows, because this is what VS is doing
IF /I "%ARCH%" == "x86" (SET TOOLSETARCH=x86) ELSE (SET TOOLSETARCH=x86_amd64)
CALL "%TOOLSET%" %TOOLSETARCH%

SET START_TIME=%TIME%
SET START_DATE=%DATE%

IF /I "%BUILDTYPE%" == "Rebuild" (
  SET "BUILDTYPE=Clean" & CALL :SubMake
  SET "BUILDTYPE=Build" & CALL :SubMake
  SET "BUILDTYPE=Rebuild"
  EXIT /B
)

IF /I "%BUILDTYPE%" == "Clean" (CALL :SubMake & EXIT /B)

CALL :SubMake

EXIT /B


:End
IF %ERRORLEVEL% NEQ 0 EXIT /B
TITLE Compiling LAV Filters %COMPILER% [FINISHED]
SET END_TIME=%TIME%
CALL %COMMON% :SubGetDuration
CALL %COMMON% :SubMsg "INFO" "LAV Filters compilation started on %START_DATE%-%START_TIME% and completed on %DATE%-%END_TIME% [%DURATION%]"
POPD
ENDLOCAL
EXIT /B


:SubMake
IF %ERRORLEVEL% NEQ 0 EXIT /B

IF /I "%ARCH%" == "x86" (SET "ARCHVS=Win32") ELSE (SET "ARCHVS=x64")

REM Build FFmpeg
sh build_ffmpeg.sh %ARCH% %BUILDTYPE%
IF %ERRORLEVEL% NEQ 0 (
  CALL %COMMON% :SubMsg "ERROR" "'sh build_ffmpeg.sh %ARCH% %BUILDTYPE%' failed!"
  EXIT /B
)

PUSHD src

REM Build LAVFilters
IF /I "%ARCH%" == "x86" (SET "ARCHVS=Win32") ELSE (SET "ARCHVS=x64")

MSBuild.exe LAVFilters.sln /nologo /consoleloggerparameters:Verbosity=minimal /nodeReuse:true /m /t:%BUILDTYPE% /property:Configuration=%RELEASETYPE%;Platform=%ARCHVS%
IF %ERRORLEVEL% NEQ 0 (
  CALL %COMMON% :SubMsg "ERROR" "'MSBuild.exe LAVFilters.sln /nologo /consoleloggerparameters:Verbosity=minimal /nodeReuse:true /m /t:%BUILDTYPE% /property:Configuration=%RELEASETYPE%;Platform=%ARCHVS%' failed!"
  EXIT /B
)

POPD

IF /I "%RELEASETYPE%" == "Debug" (
  SET "SRCFOLDER=src\bin_%ARCHVS%d"
) ELSE (
  SET "SRCFOLDER=src\bin_%ARCHVS%"
)

IF /I "%RELEASETYPE%" == "Debug" (
  SET "DESTFOLDER=%BIN_DIR%\mpc-hc_%ARCH%_Debug"
) ELSE (
  SET "DESTFOLDER=%BIN_DIR%\mpc-hc_%ARCH%"
)

IF /I "%ARCH%" == "x64" (
  SET "DESTFOLDER=%DESTFOLDER%\LAVFilters64"
) ELSE (
  SET "DESTFOLDER=%DESTFOLDER%\LAVFilters"
)

IF /I "%BUILDTYPE%" == "Build" (
  REM Copy LAVFilters files to MPC-HC output directory
  IF NOT EXIST %DESTFOLDER% MD %DESTFOLDER%

  COPY /Y /V %SRCFOLDER%\*.dll %DESTFOLDER%
  COPY /Y /V %SRCFOLDER%\*.ax %DESTFOLDER%
  COPY /Y /V %SRCFOLDER%\*.manifest %DESTFOLDER%
  IF /I "%RELEASETYPE%" == "Release" (
    COPY /Y /V %SRCFOLDER%\IntelQuickSyncDecoder\IntelQuickSyncDecoder.pdb %DESTFOLDER%
    COPY /Y /V %SRCFOLDER%\LAVAudio\LAVAudio.pdb %DESTFOLDER%
    COPY /Y /V %SRCFOLDER%\LAVSplitter\LAVSplitter.pdb %DESTFOLDER%
    COPY /Y /V %SRCFOLDER%\LAVVideo\LAVVideo.pdb %DESTFOLDER%
    COPY /Y /V %SRCFOLDER%\libbluray\libbluray.pdb %DESTFOLDER%
  ) ELSE (
    COPY /Y /V %SRCFOLDER%\*.pdb %DESTFOLDER%
  )
) ELSE IF /I "%BUILDTYPE%" == "Clean" (
  REM Remove LAVFilters files in MPC-HC output directory
  IF EXIST %DESTFOLDER% RD /Q /S %DESTFOLDER%
)

EXIT /B


:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "%ROOT_DIR%\docs\Compilation.txt" for more information.
CALL %COMMON% :SubMsg "ERROR" "LAV Filters compilation failed!" & EXIT /B 1


:UnsupportedSwitch
ECHO.
ECHO Unsupported commandline switch!
ECHO.
ECHO "%~nx0 %*"
ECHO.
ECHO Run "%~nx0 help" for details about the commandline switches.
CALL %COMMON% :SubMsg "ERROR" "LAV Filters compilation failed!" & EXIT /B 1


:ShowHelp
TITLE %~nx0 Help
ECHO.
ECHO Usage:
ECHO %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|Both] [Debug^|Release] [VS2013]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        The arguments are not case sensitive and can be ommitted.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments will use the default ones:
ECHO "%~nx0 Build Both Release VS2013"
ECHO.
POPD
ENDLOCAL
EXIT /B
