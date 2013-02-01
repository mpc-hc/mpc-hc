@ECHO OFF
REM (C) 2012-2013 see Authors.txt
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


PUSHD %~dp0

SET ROOT_DIR=..\..\..

IF EXIST "%ROOT_DIR%\build.user.bat" (
  CALL "%ROOT_DIR%\build.user.bat"
) ELSE (
  IF /I NOT "%1"=="perl" (
    IF DEFINED GIT (SET MPCHC_GIT=%GIT%)
  )
  IF DEFINED PERL (SET MPCHC_PERL=%PERL%)
)

SET PATH=%MPCHC_PERL%\bin;%MPCHC_GIT%\cmd;%PATH%
FOR %%G IN (git.exe)  DO (SET GIT_PATH=%%~$PATH:G)
FOR %%G IN (perl.exe) DO (SET PERL_PATH=%%~$PATH:G)
IF NOT DEFINED GIT_PATH  GOTO MissingVar
IF NOT DEFINED PERL_PATH GOTO MissingVar

ECHO Backing up current translation files...
IF NOT EXIST backup MD backup
COPY /Y /V ..\mplayerc.rc backup
COPY /Y /V *.rc backup
IF NOT EXIST backup\text MD backup\text
COPY /Y /V text backup\text
ECHO ----------------------

EXIT /B


:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "%ROOT_DIR%\docs\Compilation.txt" for more information.
ENDLOCAL
EXIT /B 1
