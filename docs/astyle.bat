@ECHO OFF
SETLOCAL
TITLE Applying astyle using %~dp0astyle.ini

PUSHD %~dp0

astyle.exe --version 1>&2 2>NUL

IF %ERRORLEVEL% NEQ 0 (
  ECHO ERROR: Astyle wasn't found!
  CHOICE /C yn /M "Do you want to visit its webpage now"
  IF ERRORLEVEL 2 ECHO. & GOTO END
  START "" http://astyle.sourceforge.net/
  GOTO END
)

astyle.exe --options=astyle.ini ..\*.h ..\*.cpp
IF %ERRORLEVEL% NEQ 0 ECHO. & ECHO ERROR: Something went wrong!

:END
POPD
ECHO Press any key to close this window...
PAUSE >NUL
ENDLOCAL
EXIT /B
