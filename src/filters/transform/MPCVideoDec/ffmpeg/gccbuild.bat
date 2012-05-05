@ECHO OFF
SETLOCAL

rem Check for the help switches
IF /I "%~1"=="help"   GOTO SHOWHELP
IF /I "%~1"=="/help"  GOTO SHOWHELP
IF /I "%~1"=="-help"  GOTO SHOWHELP
IF /I "%~1"=="--help" GOTO SHOWHELP
IF /I "%~1"=="/?"     GOTO SHOWHELP

IF EXIST "..\..\..\..\..\build.user.bat" (
  CALL "..\..\..\..\..\build.user.bat"
) ELSE (
  IF DEFINED MINGW32 (SET MPCHC_MINGW32=%MINGW32%) ELSE (GOTO MissingVar)
  IF DEFINED MSYS    (SET MPCHC_MSYS=%MSYS%)       ELSE (GOTO MissingVar)
)

SET 'PATH="%MPCHC_MSYS%\bin;%MPCHC_MINGW32%\bin;%PATH%"'
FOR %%X IN (gcc.exe) DO (SET FOUND=%%~$PATH:X)
IF NOT DEFINED FOUND GOTO MissingVar


:VarOk
IF /I "%~2" == "Debug" SET "DEBUG=DEBUG=yes"

IF "%~1" == "" (
  SET "BUILDTYPE=build"
  CALL :SubMake
  EXIT /B
) ELSE (
  IF /I "%~1" == "Build" (
    SET "BUILDTYPE=build"
    CALL :SubMake
    EXIT /B
  )

  IF /I "%~1" == "Clean" (
    SET "BUILDTYPE=clean"
    CALL :SubMake clean
    EXIT /B
  )

  IF /I "%~1" == "Rebuild" (
    SET "BUILDTYPE=clean"
    CALL :SubMake clean
    SET "BUILDTYPE=build"
    CALL :SubMake
    EXIT /B
  )

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  ENDLOCAL
  EXIT /B
)


:SubMake
IF "%BUILDTYPE%" == "clean" (
  SET JOBS=1
) ELSE (
  IF DEFINED NUMBER_OF_PROCESSORS (
    SET JOBS=%NUMBER_OF_PROCESSORS%
  ) ELSE (
    REM Default number of jobs
    SET JOBS=4
  )
)

SET "JOBS=-j%JOBS%"

TITLE "make %JOBS% %DEBUG% %*"
ECHO make %JOBS% %DEBUG% %*
make.exe %JOBS% %DEBUG% %*

ENDLOCAL
EXIT /B


:MissingVar
ECHO ERROR: Please define MINGW32 (and/or MSYS) environment variable(s)
ENDLOCAL
EXIT /B


:SHOWHELP
TITLE "%~nx0 %1"
ECHO. & ECHO.
ECHO Usage:   %~nx0 [Clean^|Build^|Rebuild] [Debug]
ECHO.
ECHO Notes:   The arguments are not case sensitive.
ECHO. & ECHO.
ECHO Executing "%~nx0" will use the defaults: "%~nx0 build"
ECHO.
ENDLOCAL
EXIT /B
