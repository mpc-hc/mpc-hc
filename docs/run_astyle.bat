@ECHO OFF
SETLOCAL

PUSHD %~dp0
TITLE Running astyle using astyle.ini
astyle --options=astyle.ini ..\*.h ..\*.cpp
IF %ERRORLEVEL% NEQ 0 ECHO. & ECHO ERROR: Something went wrong!
POPD

:END
ECHO Press any key to close this window...
PAUSE >NUL
ENDLOCAL
EXIT /B
