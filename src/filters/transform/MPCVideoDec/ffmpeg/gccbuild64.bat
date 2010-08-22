@ECHO OFF
IF NOT "x%MINGW64%" == "x" GOTO VarOk
ECHO "ERROR : Please define MINGW64 (and/or MSYS) environment variable(s)"
EXIT 1005

:VarOk
SET CC=gcc.exe
SET PATH=%MSYS%\bin;%MINGW64%\bin;%YASM%;%PATH%

IF "%1%"=="rebuild" GOTO DoClean
IF "%1%"=="clean" GOTO OnlyClean
GOTO NoArchClean

:OnlyClean
make.exe 64BIT=yes clean
GOTO End

:DoClean
make.exe 64BIT=yes clean

:NoArchClean
make.exe 64BIT=yes -j4

:End
