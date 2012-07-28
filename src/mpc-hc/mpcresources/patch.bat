@ECHO OFF
SETLOCAL

rem A simple script which demonstrates how to apply translated text to locale rc file.
rem This is only an example.

CALL "common.bat" perl
IF %ERRORLEVEL% NEQ 0 GOTO END

FOR %%i IN (*.rc) DO (
  ECHO Patching file %%i
  perl.exe patch.pl -i text\%%i.txt %%i
  ECHO ----------------------
)


:END
PAUSE
ENDLOCAL
EXIT /B
