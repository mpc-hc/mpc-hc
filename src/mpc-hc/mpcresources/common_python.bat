@ECHO OFF
REM (C) 2012-2013, 2015, 2017 see Authors.txt
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

IF EXIST "%ROOT_DIR%\build.user.bat" CALL "%ROOT_DIR%\build.user.bat"

IF NOT DEFINED MPCHC_PYTHON IF DEFINED PYTHON (SET MPCHC_PYTHON=%PYTHON%)

REM If the define wasn't set, we try to detect Python 2.7 from the registry
IF NOT DEFINED MPCHC_PYTHON (
  FOR /F "delims=" %%G IN (
    'REG QUERY "HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.7\InstallPath" /ve 2^>NUL ^| FIND "REG_SZ" ^|^|
     REG QUERY "HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Python\PythonCore\2.7\InstallPath" /ve 2^>NUL ^| FIND "REG_SZ"') DO (
    SET "PYTHONPATH_REG=%%G" & CALL :SubPythonPath %%PYTHONPATH_REG:*REG_SZ=%%
  )
)

SET "PATH=%MPCHC_PYTHON%;%PATH%"
FOR %%G IN (python.exe) DO (SET PYTHON_PATH=%%~$PATH:G)
IF NOT DEFINED PYTHON_PATH GOTO MissingVar

ECHO Backing up current translation files...
IF NOT EXIST backup MD backup
XCOPY /I /D /Y /Q ..\mpc-hc.rc backup
IF NOT EXIST backup\PO MD backup\PO
XCOPY /I /D /Y /Q PO backup\PO
ECHO ----------------------

EXIT /B


:SubPythonPath
SET MPCHC_PYTHON=%*
EXIT /B


:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "%ROOT_DIR%\docs\Compilation.md" for more information.
EXIT /B 1
