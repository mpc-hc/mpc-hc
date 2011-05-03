@ECHO OFF

rem Check for the help switches
IF /I "%~1"=="help"   GOTO SHOWHELP
IF /I "%~1"=="/help"  GOTO SHOWHELP
IF /I "%~1"=="-help"  GOTO SHOWHELP
IF /I "%~1"=="--help" GOTO SHOWHELP
IF /I "%~1"=="/?"     GOTO SHOWHELP

IF DEFINED MINGW32 GOTO VarOk
ECHO ERROR: Please define MINGW32 (and/or MSYS) environment variable(s)
EXIT /B

:VarOk
SET PATH=%MSYS%\bin;%MINGW32%\bin;%PATH%

IF "%~1" == "" (
  SET "BUILDTYPE=build"
) ELSE (
  IF /I "%~1" == "Build"     SET "BUILDTYPE=build"   & GOTO SubMake
  IF /I "%~1" == "/Build"    SET "BUILDTYPE=build"   & GOTO SubMake
  IF /I "%~1" == "-Build"    SET "BUILDTYPE=build"   & GOTO SubMake
  IF /I "%~1" == "--Build"   SET "BUILDTYPE=build"   & GOTO SubMake
  IF /I "%~1" == "Clean"     SET "BUILDTYPE=clean"   & GOTO SubMake
  IF /I "%~1" == "/Clean"    SET "BUILDTYPE=clean"   & GOTO SubMake
  IF /I "%~1" == "-Clean"    SET "BUILDTYPE=clean"   & GOTO SubMake
  IF /I "%~1" == "--Clean"   SET "BUILDTYPE=clean"   & GOTO SubMake
  IF /I "%~1" == "Rebuild"   SET "BUILDTYPE=rebuild" & GOTO SubMake
  IF /I "%~1" == "/Rebuild"  SET "BUILDTYPE=rebuild" & GOTO SubMake
  IF /I "%~1" == "-Rebuild"  SET "BUILDTYPE=rebuild" & GOTO SubMake
  IF /I "%~1" == "--Rebuild" SET "BUILDTYPE=rebuild" & GOTO SubMake

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  EXIT /B
)


:SubMake
TITLE "make.exe VS2010=yes -j4 %BUILDTYPE%"
ECHO make.exe VS2010=yes -j4 %BUILDTYPE%
make.exe VS2010=yes -j4 %BUILDTYPE%
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
