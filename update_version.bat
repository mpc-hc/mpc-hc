@ECHO OFF
REM $Id$
REM
REM (C) 2010-2012 see Authors.txt
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

IF EXIST "SubWCRev.exe" SET "SUBWCREV=SubWCRev.exe"
FOR %%A IN (SubWCRev.exe) DO (SET SUBWCREV=%%~$PATH:A)
IF NOT DEFINED SUBWCREV GOTO SubNoSubWCRev

"%SUBWCREV%" . "include\Version_rev.h.in" "include\Version_rev.h" -f
IF %ERRORLEVEL% NEQ 0 GOTO SubError

"%SUBWCREV%" . "src\mpc-hc\res\mpc-hc.exe.manifest.conf" "src\mpc-hc\res\mpc-hc.exe.manifest" -f >NUL
IF %ERRORLEVEL% NEQ 0 GOTO SubError

:END
POPD
ENDLOCAL
EXIT /B


:SubNoSubWCRev
ECHO. & ECHO SubWCRev, which is part of TortoiseSVN, wasn't found!
ECHO You should (re)install TortoiseSVN.
GOTO SubCommon

:SubError
ECHO Something went wrong when generating the revision number.

:SubCommon
ECHO I'll use MPC_VERSION_REV=0 for now.

ECHO #define MPC_VERSION_REV 0 > "include\Version_rev.h"
TYPE "src\mpc-hc\res\mpc-hc.exe.manifest.template" > "src\mpc-hc\res\mpc-hc.exe.manifest"
GOTO END
