@ECHO OFF
SETLOCAL

rem A simple script which demonstrates how to extract all strings from rc files.

CALL "common.bat" perl
IF %ERRORLEVEL% NEQ 0 GOTO END

ECHO Generating string files...
perl.exe rcstrings.pl -a


:END
PAUSE
ENDLOCAL
EXIT /B
