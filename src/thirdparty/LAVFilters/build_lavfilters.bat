@ECHO OFF
REM (C) 2013 see Authors.txt
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

IF EXIST "%ROOT_DIR%\build.user.bat" (
  CALL "%ROOT_DIR%\build.user.bat"
) ELSE (
  IF DEFINED MINGW32 (SET MPCHC_MINGW32=%MINGW32%) ELSE (GOTO MissingVar)
  IF DEFINED MINGW64 (SET MPCHC_MINGW64=%MINGW64%) ELSE (GOTO MissingVar)
  IF DEFINED MSYS    (SET MPCHC_MSYS=%MSYS%)       ELSE (GOTO MissingVar)
)

IF NOT EXIST %MPCHC_MINGW32% GOTO MissingVar
IF NOT EXIST %MPCHC_MINGW64% GOTO MissingVar
IF NOT EXIST %MPCHC_MSYS%    GOTO MissingVar

SET PATH=%MPCHC_MSYS%\bin;%MPCHC_MINGW32%\bin;%PATH%
FOR %%G IN (gcc.exe) DO (SET FOUND=%%~$PATH:G)
IF NOT DEFINED FOUND GOTO MissingVar

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
  IF /I "%%G" == "VS2010"   SET "COMPILER=VS2010"     & SET /A ARGCOMP+=1
  IF /I "%%G" == "VS2012"   SET "COMPILER=VS2012"     & SET /A ARGCOMP+=1
  IF /I "%%G" == "Silent"   SET "SILENT=True"         & SET /A VALID+=1
  IF /I "%%G" == "Nocolors" SET "NOCOLORS=True"       & SET /A VALID+=1
)

FOR %%X IN (%*) DO SET /A INPUT+=1
SET /A VALID+=%ARGB%+%ARGPL%+%ARGBC%+%ARGCOMP%

IF %VALID% NEQ %INPUT% GOTO UnsupportedSwitch

IF %ARGB%    GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGB% == 0    (SET "BUILDTYPE=Build")
IF %ARGPL%   GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGPL% == 0   (SET "ARCH=Both")
IF %ARGBC%   GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGBC% == 0   (SET "RELEASETYPE=Release")
IF %ARGCOMP% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGCOMP% == 0 (SET "COMPILER=VS2010")

IF /I "%COMPILER%" == "VS2012" (
  IF NOT DEFINED VS110COMNTOOLS GOTO MissingVar
  SET "TOOLSET=%VS110COMNTOOLS%..\..\VC\vcvarsall.bat"
  SET "BIN_DIR=%ROOT_DIR%\bin12"
  SET "SLN_SUFFIX=2012"
) ELSE (
  IF NOT DEFINED VS100COMNTOOLS GOTO MissingVar
  SET "TOOLSET=%VS100COMNTOOLS%..\..\VC\vcvarsall.bat"
  SET "BIN_DIR=%ROOT_DIR%\bin"
  SET "SLN_SUFFIX="
)

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
IF %ERRORLEVEL% NEQ 0 EXIT /B 1
TITLE Compiling LAV Filters %COMPILER% [FINISHED]
SET END_TIME=%TIME%
CALL :SubGetDuration
CALL :SubMsg "INFO" "LAV Filters compilation started on %START_DATE%-%START_TIME% and completed on %DATE%-%END_TIME% [%DURATION%]"
POPD
ENDLOCAL
EXIT /B


:SubMake
IF %ERRORLEVEL% NEQ 0 EXIT /B

IF /I "%ARCH%" == "x86" (SET "ARCHVS=Win32") ELSE (SET "ARCHVS=x64")

:: Build FFmpeg
sh build_ffmpeg.sh %ARCH% %BUILDTYPE%
IF %ERRORLEVEL% NEQ 0 (
    CALL :SubMsg "ERROR" "'sh build_ffmpeg.sh %ARCH% %BUILDTYPE%' failed!"
    EXIT /B
)

PUSHD src

:: Build LAVFilters
IF /I "%ARCH%" == "x86" (SET "ARCHVS=Win32") ELSE (SET "ARCHVS=x64")

devenv LAVFilters%SLN_SUFFIX%.sln /%BUILDTYPE% "%RELEASETYPE%|%ARCHVS%"
IF %ERRORLEVEL% NEQ 0 (
    CALL :SubMsg "ERROR" "'devenv LAVFilters%SLN_SUFFIX%.sln /%BUILDTYPE% "%RELEASETYPE%-%ARCHVS%" failed!"
    EXIT /B
)

POPD

SET "SRCFOLDER=src\bin_%ARCHVS%"
IF /I "%RELEASETYPE%" == "Debug" (
    SET "SRCFOLDER=%SRCFOLDER%d"
)
SET "DESTFOLDER=%BIN_DIR%\mpc-hc_%ARCH%"
IF /I "%RELEASETYPE%" == "Debug" (
    SET "DESTFOLDER=%DESTFOLDER%_Debug"
)
SET "DESTFOLDER=%DESTFOLDER%\LAVFilters"

IF /I "%BUILDTYPE%" == "Build" (
    :: Move LAVFilters files to MPC-HC output directory
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
    :: Remove LAVFilters files in MPC-HC output directory
    IF EXIST %DESTFOLDER% RMDIR /S /Q %DESTFOLDER%
)

EXIT /B


:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "%ROOT_DIR%\docs\Compilation.txt" for more information.
CALL :SubMsg "ERROR" "LAV Filters compilation failed!" & EXIT /B 1


:UnsupportedSwitch
ECHO.
ECHO Unsupported commandline switch!
ECHO.
ECHO "%~nx0 %*"
ECHO.
ECHO Run "%~nx0 help" for details about the commandline switches.
CALL :SubMsg "ERROR" "LAV Filters compilation failed!" & EXIT /B 1


:ShowHelp
TITLE %~nx0 Help
ECHO.
ECHO Usage:
ECHO %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|Both] [Debug^|Release] [VS2010^|VS2012]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        The arguments are not case sensitive and can be ommitted.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments will use the default ones:
ECHO "%~nx0 Build Both Release VS2010"
ECHO.
POPD
ENDLOCAL
EXIT /B


:SubMsg
ECHO. & ECHO ------------------------------
IF /I "%~1" == "ERROR" (
  CALL :SubColorText "0C" "[%~1]" "%~2"
) ELSE IF /I "%~1" == "INFO" (
  CALL :SubColorText "0A" "[%~1]" "%~2"
) ELSE IF /I "%~1" == "WARNING" (
  CALL :SubColorText "0E" "[%~1]" "%~2"
)
ECHO ------------------------------ & ECHO.
IF /I "%~1" == "ERROR" (
  IF NOT DEFINED SILENT (
    ECHO Press any key to exit...
    PAUSE >NUL
  )
  POPD
  ENDLOCAL
  EXIT /B 1
) ELSE (
  EXIT /B
)


:SubColorText
IF DEFINED NOCOLORS ECHO %~2 %~3 & EXIT /B
FOR /F "tokens=1,2 delims=#" %%G IN (
  '"PROMPT #$H#$E# & ECHO ON & FOR %%H IN (1) DO REM"') DO (
  SET "DEL=%%G")
<NUL SET /p ".=%DEL%" > "%~2"
FINDSTR /v /a:%1 /R ".18" "%~2" NUL
DEL "%~2" > NUL 2>&1
ECHO  %~3
EXIT /B


:SubGetDuration
SET START_TIME=%START_TIME: =%
SET END_TIME=%END_TIME: =%

FOR /F "tokens=1-4 delims=:.," %%G IN ("%START_TIME%") DO (
  SET /A "STARTTIME=(100%%G %% 100) * 360000 + (100%%H %% 100) * 6000 + (100%%I %% 100) * 100 + (100%%J %% 100)"
)

FOR /F "tokens=1-4 delims=:.," %%G IN ("%END_TIME%") DO (
  SET /A "ENDTIME=(100%%G %% 100) * 360000 + (100%%H %% 100) * 6000 + (100%%I %% 100) * 100 + (100%%J %% 100)"
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
