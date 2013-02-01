@ECHO OFF
REM (C) 2010-2013 see Authors.txt
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

rem This is a simple script to check out a revision of mplayerc.rc,
rem then rename it to old file for rcfile.pl to process it

CALL "common.bat"
IF %ERRORLEVEL% NEQ 0 GOTO END

SET REF=HEAD
IF NOT "%1"=="" SET REF=%1

ECHO Get mplayerc.rc from %REF% first...
git.exe show %REF%:../mplayerc.rc > $$TEMP$$.old

ECHO Generating new rc files and string files...
perl.exe rcfile.pl -b $$TEMP$$.old

IF EXIST $$TEMP$$.old DEL $$TEMP$$.old


:END
PAUSE
ENDLOCAL
EXIT /B
