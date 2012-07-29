@ECHO OFF
SETLOCAL

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
