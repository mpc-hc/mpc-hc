@ECHO OFF
IF DEFINED MINGW64 GOTO VarOk
ECHO ERROR: Please define MINGW64(and/or MSYS) environment variable(s)
EXIT /B

:VarOk
SET CC=gcc.exe
SET PATH=%MSYS%\bin;%MINGW64%\bin;%YASM%;%PATH%

IF /I "%1%"=="rebuild" GOTO DoClean
IF /I "%1%"=="/rebuild" GOTO DoClean
IF /I "%1%"=="-rebuild" GOTO DoClean
IF /I "%1%"=="--rebuild" GOTO DoClean
IF /I "%1%"=="clean" GOTO OnlyClean
IF /I "%1%"=="/clean" GOTO OnlyClean
IF /I "%1%"=="-clean" GOTO OnlyClean
IF /I "%1%"=="--clean" GOTO OnlyClean
GOTO Build

:OnlyClean
make.exe VS2010=yes 64BIT=yes clean
EXIT /B

:DoClean
make.exe VS2010=yes 64BIT=yes clean

:Build
make.exe VS2010=yes 64BIT=yes -j4
EXIT /B
