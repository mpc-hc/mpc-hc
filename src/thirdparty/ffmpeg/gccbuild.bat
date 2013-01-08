@ECHO OFF
REM (C) 2009-2013 see Authors.txt
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

IF /I "%ARG%" == "?"        GOTO ShowHelp

FOR %%G IN (%ARG%) DO (
  IF /I "%%G" == "help"     GOTO ShowHelp
  IF /I "%%G" == "Build"    SET "BUILDTYPE=Build"   & SET /A ARGB+=1
  IF /I "%%G" == "Clean"    SET "BUILDTYPE=Clean"   & SET /A ARGB+=1
  IF /I "%%G" == "Rebuild"  SET "BUILDTYPE=Rebuild" & SET /A ARGB+=1
  IF /I "%%G" == "Both"     SET "ARCH=Both"         & SET /A ARGPL+=1
  IF /I "%%G" == "Win32"    SET "ARCH=x86"          & SET /A ARGPL+=1
  IF /I "%%G" == "x86"      SET "ARCH=x86"          & SET /A ARGPL+=1
  IF /I "%%G" == "x64"      SET "ARCH=x64"          & SET /A ARGPL+=1
  IF /I "%%G" == "Debug"    SET "DEBUG=DEBUG=yes"   & SET /A ARGBC+=1
  IF /I "%%G" == "Release"  SET "DEBUG= "           & SET /A ARGBC+=1
  IF /I "%%G" == "VS2010"   SET "COMPILER=VS2010"   & SET /A ARGCOMP+=1
  IF /I "%%G" == "VS2012"   SET "COMPILER=VS2012"   & SET /A ARGCOMP+=1
)

FOR %%X IN (%*) DO SET /A INPUT+=1
SET /A VALID=%ARGB%+%ARGPL%+%ARGBC%+%ARGCOMP%

IF %VALID% NEQ %INPUT% GOTO UnsupportedSwitch

IF %ARGB%    GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGB% == 0    (SET "BUILDTYPE=Build")
IF %ARGPL%   GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGPL% == 0   (SET "ARCH=Both")
IF %ARGBC%   GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGBC% == 0   (SET "DEBUG= ")
IF %ARGCOMP% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGCOMP% == 0 (SET "COMPILER=VS2010")

IF /I "%ARCH%" == "Both" (
  SET "ARCH=x86" & CALL :Main
  SET "ARCH=x64" & CALL :Main
) ELSE (
  CALL :Main
)
GOTO End


:Main
IF %ERRORLEVEL% NEQ 0 EXIT /B
IF /I "%ARCH%" == "x64" (SET "x64=64BIT=yes") ELSE (SET "x64= ")

SET START_TIME=%TIME%
SET START_DATE=%DATE%

IF /I "%BUILDTYPE%" == "Rebuild" (
  SET "BUILDTYPE=Clean" & CALL :SubMake %x64% %DEBUG% %COMPILER%=yes clean
  CALL :SubCopyLibs
  SET "BUILDTYPE=Build" & CALL :SubMake %x64% %DEBUG% %COMPILER%=yes
  SET "BUILDTYPE=Rebuild"
  EXIT /B
)

IF /I "%BUILDTYPE%" == "Clean" (CALL :SubMake %x64% %DEBUG% %COMPILER%=yes clean & EXIT /B)

CALL :SubCopyLibs
CALL :SubMake %x64% %DEBUG% %COMPILER%=yes
EXIT /B


:End
TITLE Compiling FFmpeg [FINISHED]

SET END_TIME=%TIME%
CALL :SubGetDuration
ECHO. & ECHO FFmpeg compilation took %DURATION%

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


:SubCopyLibs
REM Set the GCC version
FOR /F "tokens=1,2 delims= " %%G IN ('gcc -dumpversion') DO (SET "gccver=%%G")

REM Copy the needed libraries
COPY /Y /V "%MPCHC_MINGW32%\lib\gcc\i686-w64-mingw32\%gccver%\libgcc.a"   "%ROOT_DIR%\lib\" >NUL
COPY /Y /V "%MPCHC_MINGW32%\i686-w64-mingw32\lib\libmingwex.a"            "%ROOT_DIR%\lib\" >NUL
COPY /Y /V "%MPCHC_MINGW64%\lib\gcc\x86_64-w64-mingw32\%gccver%\libgcc.a" "%ROOT_DIR%\lib64\" >NUL
COPY /Y /V "%MPCHC_MINGW64%\x86_64-w64-mingw32\lib\libmingwex.a"          "%ROOT_DIR%\lib64\" >NUL
EXIT /B


:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "%ROOT_DIR%\docs\Compilation.txt" for more information.
ENDLOCAL
EXIT /B 1


:UnsupportedSwitch
ECHO.
ECHO Unsupported commandline switch!
ECHO.
ECHO "%~nx0 %*"
ECHO.
ECHO Run "%~nx0 help" for details about the commandline switches.
ENDLOCAL
EXIT /B 1


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
