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

REM You can set here the Inno Setup path if for example you have Inno Setup Unicode
REM installed and you want to use the ANSI Inno Setup which is in another location
IF NOT DEFINED InnoSetupPath SET "InnoSetupPath=H:\progs\thirdparty\isetup"

SET ROOT_DIR=..\..\..\..\..
SET "BIN_DIR=%ROOT_DIR%\bin12"
CALL :SubDetectInnoSetup
IF EXIST "%~dp0%ROOT_DIR%\signinfo.txt" (
  CALL :SubSign VSFilter.dll x86
  CALL :SubSign VSFilter.dll x64
)
CALL :SubInno
CALL :SubInno x64Build


:END
ECHO. & ECHO.
ENDLOCAL
EXIT /B


:SubSign
IF %ERRORLEVEL% NEQ 0 GOTO EndWithError
REM %1 is name of the file to sign
REM %2 is the platform

PUSHD "%BIN_DIR%\Filters_%~2\"
CALL "%~dp0%ROOT_DIR%\contrib\sign.bat" "%1" || (ECHO Problem signing %1 & GOTO Break)
ECHO %1 signed successfully.

:Break
POPD
EXIT /B


:SubInno
IF %ERRORLEVEL% NEQ 0 GOTO EndWithError
ECHO.
TITLE Building VSFilter installer...
"%InnoSetupPath%\ISCC.exe" /SMySignTool="cmd /c "%~dp0%ROOT_DIR%\contrib\sign.bat" $f" /Q^
 "vsfilter_setup.iss" /D%~1
IF %ERRORLEVEL% NEQ 0 GOTO EndWithError
IF /I "%~1%" == "x64Build" (
  ECHO Installer x64 compiled successfully!
) ELSE (
  ECHO Installer x86 compiled successfully!
)
EXIT /B


:SubDetectInnoSetup
REM Detect if we are running on 64bit Windows and use Wow6432Node since Inno Setup is
REM a 32-bit application, and set the registry key of Inno Setup accordingly
IF DEFINED PROGRAMFILES(x86) (
  SET "U_=HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
) ELSE (
  SET "U_=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"
)

IF DEFINED InnoSetupPath IF NOT EXIST "%InnoSetupPath%" (
  ECHO "%InnoSetupPath%" wasn't found on this machine! I will try to detect Inno Setup's path from the registry...
)

IF NOT EXIST "%InnoSetupPath%" (
  FOR /F "delims=" %%a IN (
    'REG QUERY "%U_%\Inno Setup 5_is1" /v "Inno Setup: App Path"2^>Nul^|FIND "REG_SZ"') DO (
    SET "InnoSetupPath=%%a" & CALL :SubInnoSetupPath %%InnoSetupPath:*Z=%%)
)

IF NOT EXIST "%InnoSetupPath%" ECHO Inno Setup wasn't found! & GOTO EndWithError
EXIT /B


:SubInnoSetupPath
SET "InnoSetupPath=%*"
EXIT /B


:EndWithError
Title Building VSFilter installer [ERROR]
COLOR 0C
ECHO. & ECHO.
ECHO ERROR: Build failed!
ECHO Press any key to close this window...
PAUSE >NUL
ENDLOCAL
EXIT
