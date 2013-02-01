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

rem An all in one script which demonstrates how to sync all locale rc files to a revision of mplayerc.rc.
rem It will try to patch existing local rc files first, then sync them to mplayerc.rc.
rem Then it will overwrite rc files with new rc ones, and after that it will generate the text files.
rem This is only an example.

CALL "common.bat"
IF %ERRORLEVEL% NEQ 0 GOTO END

SET REF=HEAD
IF NOT "%1"=="" SET REF=%1

ECHO Get mplayerc.rc from %REF% first...
git.exe show %REF%:../mplayerc.rc > $$TEMP$$.old
ECHO ----------------------

FOR %%i IN (*.rc) DO (
  ECHO Patching file %%i
  perl.exe patch.pl -i text\%%i.txt %%i
  ECHO ----------------------
)
ECHO ----------------------

ECHO Generating new rc files...
perl.exe rcfile.pl -b $$TEMP$$.old
IF EXIST $$TEMP$$.old DEL $$TEMP$$.old
ECHO ----------------------

COPY /Y /V newrc\*.rc .
ECHO ----------------------

ECHO Generating new string files...
COPY /Y /V ..\mplayerc.rc .
perl.exe rcstrings.pl -a
IF EXIST mplayerc.rc DEL mplayerc.rc
ECHO ----------------------


:END
PAUSE
ENDLOCAL
EXIT /B
