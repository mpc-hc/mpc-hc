@ECHO OFF
IF DEFINED MINGW64 GOTO VarOk
ECHO: ERROR: Please define MINGW64(and/or MSYS) environment variable(s)
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
GOTO NoArchClean

:OnlyClean
make.exe -f makefile VS2010=yes 64BIT=yes clean
GOTO End

:DoClean
make.exe -f makefile VS2010=yes 64BIT=yes clean

:NoArchClean
make.exe -f makefile VS2010=yes 64BIT=yes -j4

:End
