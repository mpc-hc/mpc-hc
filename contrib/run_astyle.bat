@ECHO OFF
REM (C) 2012-2017 see Authors.txt
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

SET "AStyleVerReq=3.0.1"
astyle --ascii --version 2>NUL || (ECHO. & ECHO ERROR: AStyle not found & GOTO End)
CALL :SubCheckVer || GOTO End


:Start
TITLE Running astyle using %FILE_DIR%astyle.ini

IF "%~1" == "" (
  astyle --ascii -r --options=astyle.ini ..\*.cpp
  astyle --ascii -r --options=astyle.ini --keep-one-line-blocks ..\*.h
) ELSE (
  FOR %%G IN (%*) DO astyle --ascii --options=astyle.ini %%G
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
FOR /F "tokens=4 delims= " %%A IN ('astyle --ascii --version 2^>^&1 NUL') DO (
  SET "AStyleVer=%%A"
)

IF %AStyleVer% NEQ %AStyleVerReq% (
  ECHO. & ECHO ERROR: AStyle v%AStyleVerReq% is required.
  EXIT /B 1
)
EXIT /B
