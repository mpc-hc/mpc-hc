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


SETLOCAL

PUSHD %~dp0

SET "AStyleVerMin=2.03"
astyle --version 2>NUL || (ECHO. & ECHO ERROR: AStyle not found & GOTO End)
CALL :SubCheckVer || GOTO End


:Start
TITLE Running astyle using %~dp0astyle.ini

IF "%~1" == "" (
  astyle -r --options=astyle.ini ..\*.h ..\*.cpp
) ELSE (
  FOR %%G IN (%*) DO astyle --options=astyle.ini %%G
)

IF %ERRORLEVEL% NEQ 0 ECHO. & ECHO ERROR: Something went wrong!


:End
POPD
ECHO. & ECHO Press any key to close this window...
PAUSE >NUL
ENDLOCAL
EXIT /B


:SubCheckVer
TITLE Checking astyle version
FOR /F "tokens=4 delims= " %%A IN ('astyle --version 2^>^&1 NUL') DO (
  SET "AStyleVer=%%A"
)

IF %AStyleVer% LSS %AStyleVerMin% (
  ECHO. & ECHO ERROR: AStyle v%AStyleVer% is too old, please update AStyle to v%AStyleVerMin% or newer.
  EXIT /B 1
)
EXIT /B
