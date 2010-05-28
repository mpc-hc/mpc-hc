@echo off
if NOT "x%MINGW32%" == "x" goto Var1Ok
echo "ERROR : please define MINGW32 (and/or MSYS) environment variable(s)"
exit 1005

:Var1Ok
set CC=gcc.exe
set PATH=%MSYS%\bin;%MINGW32%\bin;%YASM%;%PATH%

IF "%1%"=="rebuild" goto DoClean
IF "%1%"=="clean" goto OnlyClean
goto NoArchClean

:OnlyClean
make.exe clean
goto End

:DoClean
make.exe clean

:NoArchClean
make.exe -j4

:End
