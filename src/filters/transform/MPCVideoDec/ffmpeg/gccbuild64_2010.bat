@ECHO OFF

rem Check for the help switches
IF /I "%~1"=="help"   GOTO SHOWHELP
IF /I "%~1"=="/help"  GOTO SHOWHELP
IF /I "%~1"=="-help"  GOTO SHOWHELP
IF /I "%~1"=="--help" GOTO SHOWHELP
IF /I "%~1"=="/?"     GOTO SHOWHELP

IF DEFINED MINGW64 GOTO VarOk
ECHO ERROR: Please define MINGW64 (and/or MSYS) environment variable(s)
EXIT /B

:VarOk
SET PATH=%MSYS%\bin;%MINGW64%\bin;%PATH%

IF "%~1" == "" (
  SET "BUILDTYPE=build"
) ELSE (
  IF /I "%~1" == "Build"     SET "BUILDTYPE=build"   & CALL :SubMake & EXIT /B
  IF /I "%~1" == "/Build"    SET "BUILDTYPE=build"   & CALL :SubMake & EXIT /B
  IF /I "%~1" == "-Build"    SET "BUILDTYPE=build"   & CALL :SubMake & EXIT /B
  IF /I "%~1" == "--Build"   SET "BUILDTYPE=build"   & CALL :SubMake & EXIT /B
  IF /I "%~1" == "Clean"     SET "BUILDTYPE=clean"   & CALL :SubMake clean & EXIT /B
  IF /I "%~1" == "/Clean"    SET "BUILDTYPE=clean"   & CALL :SubMake clean & EXIT /B
  IF /I "%~1" == "-Clean"    SET "BUILDTYPE=clean"   & CALL :SubMake clean & EXIT /B
  IF /I "%~1" == "--Clean"   SET "BUILDTYPE=clean"   & CALL :SubMake clean & EXIT /B
  IF /I "%~1" == "Rebuild"   SET "BUILDTYPE=rebuild" & CALL :SubMake clean & CALL :SubMake & EXIT /B
  IF /I "%~1" == "/Rebuild"  SET "BUILDTYPE=rebuild" & CALL :SubMake clean & CALL :SubMake & EXIT /B
  IF /I "%~1" == "-Rebuild"  SET "BUILDTYPE=rebuild" & CALL :SubMake clean & CALL :SubMake & EXIT /B
  IF /I "%~1" == "--Rebuild" SET "BUILDTYPE=rebuild" & CALL :SubMake clean & CALL :SubMake & EXIT /B

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  EXIT /B
)


:SubMake
SET "make_args=-j4"
IF /I "%BUILDTYPE%"=="clean" SET "make_args="

TITLE "make.exe VS2010=yes 64BIT=yes %make_args% %*"
ECHO make.exe VS2010=yes 64BIT=yes %make_args% %*
make.exe VS2010=yes 64BIT=yes %make_args% %*
EXIT /B


:SHOWHELP
TITLE "%~nx0 %1"
ECHO. & ECHO.
ECHO Usage:   %~nx0 [Clean^|Build^|Rebuild]
ECHO.
ECHO Notes:   You can also prefix the commands with "-", "--" or "/".
ECHO          The arguments are case insesitive.
ECHO. & ECHO.
ECHO Executing "%~nx0" will use the defaults: "%~nx0 build"
ECHO.
ENDLOCAL
EXIT /B
