@ECHO OFF
REM (C) 2013, 2015 see Authors.txt
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

SET SILENT=%1

CALL "common_python.bat"
IF %ERRORLEVEL% NEQ 0 GOTO END

python.exe sync.py

:END
IF NOT DEFINED SILENT (
  PAUSE
)
IF %ERRORLEVEL% NEQ 0 (
  ENDLOCAL
  EXIT /B 1
) ELSE (
  ENDLOCAL
  EXIT /B
)
