@ECHO OFF
SETLOCAL

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
