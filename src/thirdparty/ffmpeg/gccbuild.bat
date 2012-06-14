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
PUSHD %~dp0

SET ROOT_DIR=..\..\..

IF EXIST "%ROOT_DIR%\build.user.bat" (
  CALL "%ROOT_DIR%\build.user.bat"
) ELSE (
  IF DEFINED MINGW32 (SET MPCHC_MINGW32=%MINGW32%) ELSE (GOTO MissingVar)
  IF DEFINED MINGW64 (SET MPCHC_MINGW64=%MINGW64%) ELSE (GOTO MissingVar)
  IF DEFINED MSYS    (SET MPCHC_MSYS=%MSYS%)       ELSE (GOTO MissingVar)
)

SET PATH=%MPCHC_MSYS%\bin;%MPCHC_MINGW32%\bin;%PATH%
FOR %%X IN (gcc.exe) DO (SET FOUND=%%~$PATH:X)
IF NOT DEFINED FOUND GOTO MissingVar

SET ARG=%*
SET ARG=%ARG:/=%
SET ARG=%ARG:-=%
SET ARGB=0
SET ARGBC=0
SET ARGLI=0
SET ARGPL=0
SET ARG86=0
SET INPUT=0

IF /I "%ARG%" == "?"          GOTO ShowHelp

FOR %%A IN (%ARG%) DO (
  IF /I "%%A" == "help"       GOTO ShowHelp
  IF /I "%%A" == "Build"      SET "BUILDTYPE=Build"   & SET /A ARGB+=1
  IF /I "%%A" == "Clean"      SET "BUILDTYPE=Clean"   & SET /A ARGB+=1
  IF /I "%%A" == "Rebuild"    SET "BUILDTYPE=Rebuild" & SET /A ARGB+=1
  IF /I "%%A" == "Both"       SET "ARCH=Both"         & SET /A ARGPL+=1
  IF /I "%%A" == "Win32"      SET "ARCH=x86"          & SET /A ARGPL+=1 & SET /A ARG86+=1
  IF /I "%%A" == "x86"        SET "ARCH=x86"          & SET /A ARGPL+=1 & SET /A ARG86+=1
  IF /I "%%A" == "x64"        SET "ARCH=x64"          & SET /A ARGPL+=1
  IF /I "%%A" == "Debug"      SET "DEBUG=DEBUG=yes"   & SET /A ARGBC+=1
  IF /I "%%A" == "Release"    SET "DEBUG="            & SET /A ARGBC+=1
  IF /I "%%A" == "libmingwex" SET "LIBMINGWEX=true"   & SET /A ARGLI+=1 & SET /A ARG86+=1
  IF /I "%%A" == "mingw"      SET "LIBMINGWEX=true"   & SET /A ARGLI+=1 & SET /A ARG86+=1
  IF /I "%%A" == "mingw64"    SET "LIBMINGWEX=true"   & SET /A ARGLI+=1 & SET /A ARG86+=1
)

FOR %%X IN (%*) DO SET /A INPUT+=1
SET /A VALID=%ARGB%+%ARGPL%+%ARGBC%+%ARGLI%

IF %VALID% NEQ %INPUT% GOTO UnsupportedSwitch

IF %ARGB%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGB% == 0  (SET "BUILDTYPE=Build")
IF %ARGPL% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGPL% == 0 (SET "ARCH=Both")
IF %ARGBC% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGBC% == 0 (SET "DEBUG=")
IF %ARGLI% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGLI% == 0 (SET "LIBMINGWEX=false")
IF %ARG86% GTR 1 (GOTO UnsupportedSwitch)


IF /I "%ARCH%" == "Both" (
  SET "ARCH=x86" & CALL :Main
  SET "ARCH=x64" & CALL :Main
  GOTO End
)


:Main
IF /I "%ARCH%" == "x64" (SET "x64=64BIT=yes") ELSE (SET "x64=")

CALL :SubCopyLibs

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


:SubCopyLibs
REM Set the GCC version
FOR /F "tokens=1,2 delims= " %%A IN ('gcc -dumpversion') DO (SET "gccver=%%A")

REM Copy the needed libraries
COPY /V /Y "%MPCHC_MINGW32%\lib\gcc\i686-pc-mingw32\%gccver%\libgcc.a"    "%ROOT_DIR%\lib\" >NUL
COPY /V /Y "%MPCHC_MINGW32%\i686-pc-mingw32\lib\libmingwex.a"             "%ROOT_DIR%\lib\" >NUL
COPY /V /Y "%MPCHC_MINGW64%\lib\gcc\x86_64-w64-mingw32\%gccver%\libgcc.a" "%ROOT_DIR%\lib64\" >NUL
REM libmingwex.a needs to be compiled separately for x64
IF /I "%ARCH%" == "x64" IF /I "%LIBMINGWEX%" == "true" CALL :SubLibmingwex
EXIT /B


:SubLibmingwex
IF /I "%BUILDTYPE%" == "Rebuild" (
  IF EXIST "%ROOT_DIR%\lib64\libmingwex.a" DEL "%ROOT_DIR%\lib64\libmingwex.a"
) ELSE IF EXIST "%ROOT_DIR%\lib64\libmingwex.a" (
  ECHO "%ROOT_DIR%\lib64\libmingwex.a" is present.
  EXIT /B
  ) ELSE (
  ECHO "%ROOT_DIR%\lib64\libmingwex.a" is not present.
)

SET "SEARCHLIB=patches\root-i686-pc-mingw32\x86_64-w64-mingw32\lib\libmingwex.a"
IF EXIST %SEARCHLIB% (
  COPY /V /Y "%SEARCHLIB%" "%ROOT_DIR%\lib64\" >NUL
  EXIT /B
)

SET "CC=x86_64-w64-mingw32-gcc"
SET "HST=i686-pc-mingw32"
SET "TGT=x86_64-w64-mingw32"
SET "RT=root-%HST%"

IF NOT EXIST "patches\build\mingw\build-%HST%" MD "patches\build\mingw\build-%HST%"
IF NOT EXIST "patches\%RT%\%TGT%"              MD "patches\%RT%\%TGT%"

SET "PF=%~dp0patches\%RT%"
SET "BD=%~dp0patches\build"

PUSHD "%BD%\mingw"

rem Removing patched files...
IF EXIST "mingw-w64-crt/misc/delayimp.c" DEL "mingw-w64-crt\misc\delayimp.c"

ECHO Downloading MinGW64 crt and headers...
svn -q co "https://mingw-w64.svn.sourceforge.net/svnroot/mingw-w64/stable/v2.x" .
IF %ERRORLEVEL% NEQ 0 ECHO Downloading MinGW64 crt and headers failed! & EXIT /B

ECHO. & ECHO Applying Mingw64 compatibility patch...
patch -p0 -i ../../mpchc_Mingw64.patch
IF %ERRORLEVEL% NEQ 0 ECHO patching failed! & EXIT /B

ECHO. & ECHO Copying includes...
ECHO \.svn\> exclude.txt
XCOPY "mingw-w64-headers\include\*" "%PF%\%TGT%\include\" /C /E /H /I /Q /Y /EXCLUDE:exclude.txt
IF EXIST "exclude.txt" DEL "exclude.txt"

ECHO. & ECHO Compiling MinGW64 crt...
PUSHD "%BD%/mingw/build-%HST%"

sh ../mingw-w64-crt/configure --prefix="%PF%" --with-sysroot="%PF%" --host="%TGT%" --disable-lib32
IF %ERRORLEVEL% NEQ 0 ECHO Compiling MinGW64 crt failed! (in configure) & EXIT /B

CALL :SubMake CFLAGS="-O2 -fno-leading-underscore -pipe" -s
IF %ERRORLEVEL% NEQ 0 ECHO Compiling MinGW64 crt failed! & EXIT /B

make install
POPD

POPD

IF EXIST "%PF%\%TGT%\lib\libmingwex.a" COPY /Y /V "%PF%\%TGT%\lib\libmingwex.a" "%ROOT_DIR%\lib64\" >NUL
rem IF EXIST "patches\build" RD /Q /S "patches\build"
rem IF EXIST "patches\%RT%"  RD /Q /S "patches\%RT%"
EXIT /B


:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "%ROOT_DIR%\docs\Compilation.txt" for more information.
ENDLOCAL
EXIT /B


:UnsupportedSwitch
ECHO.
ECHO Unsupported commandline switch!
ECHO.
ECHO "%~nx0 %*"
ECHO.
ECHO Run "%~nx0 help" for details about the commandline switches.
ENDLOCAL
EXIT /B


:ShowHelp
TITLE %~nx0 Help
ECHO.
ECHO Usage:
ECHO %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|Both] [Debug^|Release] [libmingwex]
ECHO.
ECHO libmingwex: If you already have libmingwex.a in %ROOT_DIR%\lib64 it will be skipped
ECHO             unless /rebuild is used too.
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
