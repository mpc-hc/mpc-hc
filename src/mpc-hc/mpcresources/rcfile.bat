@ECHO OFF
SETLOCAL

rem This is a simple script to check out the head revision of mplayerc.rc,
rem then rename it to old file for rcfile.pl to process it

CALL "common.bat"
IF %ERRORLEVEL% NEQ 0 GOTO END

ECHO Getting the latest mplayerc.rc from repository...
git.exe show origin/HEAD:../mplayerc.rc > $$TEMP$$.old

ECHO Generating new rc files and string files...
perl.exe rcfile.pl -b $$TEMP$$.old

IF EXIST $$TEMP$$.old DEL $$TEMP$$.old


:END
PAUSE
ENDLOCAL
EXIT /B
