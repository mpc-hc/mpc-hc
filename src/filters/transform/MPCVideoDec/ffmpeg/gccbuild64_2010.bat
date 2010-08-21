@echo off
if NOT "x%MINGW64%" == "x" goto Var1Ok
echo "ERROR : please define MINGW64 (and/or MSYS) environment variable(s)"
exit 1005

:Var1Ok
set CC=gcc.exe
set PATH=%MSYS%\bin;%MINGW64%\bin;%YASM%;%PATH%

IF "%1%"=="rebuild" goto DoClean
IF "%1%"=="clean" goto OnlyClean
goto NoArchClean

:OnlyClean
make.exe -f makefile_2010 64BIT=yes clean
goto End

:DoClean
make.exe -f makefile_2010 64BIT=yes clean

:NoArchClean
make.exe -f makefile_2010 64BIT=yes -j4

:End
