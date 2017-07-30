@ECHO OFF
REM (C) 2010-2013, 2015, 2017 see Authors.txt
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
SET "FILE_DIR=%~dp0"
PUSHD "%FILE_DIR%"

SET "COMMON=%FILE_DIR%\common.bat"

IF EXIST "build.user.bat" CALL "build.user.bat"

IF NOT DEFINED MPCHC_GIT IF DEFINED GIT (SET MPCHC_GIT=%GIT%)
SET "PATH=%MPCHC_GIT%\cmd;%PATH%"

CALL "%COMMON%" :SubSetPath
IF %ERRORLEVEL% NEQ 0 GOTO MissingVar
CALL "%COMMON%" :SubDoesExist bash.exe
IF %ERRORLEVEL% NEQ 0 GOTO MissingVar

bash.exe ./version.sh %*


:END
POPD
ENDLOCAL
EXIT /B


:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "docs\Compilation.md" for more information.
ENDLOCAL
EXIT /B 1
