@ECHO OFF
REM (C) 2015 see Authors.txt
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

SET ROOT_DIR=..\..\..
SET "COMMON=%FILE_DIR%%ROOT_DIR%\common.bat"
CALL "%COMMON%" :SubPreBuild

IF "%1" == "" CALL "%COMMON%" :SubMsg "ERROR" "%~nx0, No argument was provided." & EXIT /B

SET "SRCFOLDER=%MPCHC_DOCTORDUMP%\bin"
IF /I "%~1" == "x64" (
  SET "SRCFOLDER=%SRCFOLDER%\x64"
)

SET "DESTFOLDER=%~2CrashReporter"

IF EXIST "%MPCHC_DOCTORDUMP%" (
  IF NOT EXIST "%DESTFOLDER%" MD "%DESTFOLDER%"
  COPY /Y /V "%SRCFOLDER%\crashrpt.dll" "%DESTFOLDER%"
  COPY /Y /V "%SRCFOLDER%\dbghelp.dll"  "%DESTFOLDER%"
  COPY /Y /V "%SRCFOLDER%\sendrpt.exe"  "%DESTFOLDER%"
) ELSE IF DEFINED MPCHC_DOCTORDUMP (
  CALL "%COMMON%" :SubMsg "WARNING" "Invalid path to Doctor Dump SDK, files were not copied"
)

:End
POPD
ENDLOCAL
EXIT /B
