@ECHO OFF
SETLOCAL

PUSHD %~dp0
TITLE Running astyle using astyle.ini

IF "%~1" == "" (
  astyle -r --options=astyle.ini ..\*.h ..\*.cpp
) ELSE (
  FOR %%G IN (%*) DO astyle --options=astyle.ini %%G
)

IF %ERRORLEVEL% NEQ 0 ECHO. & ECHO ERROR: Something went wrong!
POPD

:END
ECHO Press any key to close this window...
PAUSE >NUL
ENDLOCAL
EXIT /B
