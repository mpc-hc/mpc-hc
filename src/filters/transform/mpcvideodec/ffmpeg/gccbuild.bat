@echo off
if NOT "x%MINGW32%" == "x" goto Var1Ok
echo "ERROR : please define MINGW32 (and/or MSYS) environment variable(s)"
exit 1005

:Var1Ok
if NOT "x%CC%" == "x" goto Var2Ok
echo "ERROR : please define CC environment variable"
exit 1005

:Var2Ok
set PATH=%MINGW32%;%YASM%;%PATH%

IF "%1%"=="rebuild" goto DoClean
IF "%1%"=="clean" goto OnlyClean
goto NoClean

:OnlyClean
make.exe clean
IF EXIST 64bit.build rm 64bit.build
goto End

:DoClean
make.exe clean
IF EXIST 64bit.build rm 64bit.build

:NoClean
IF NOT EXIST 64bit.build GOTO NoArchClean
make.exe clean
rm 64bit.build

:NoArchClean
touch 32bit.build
make.exe -j8

:End
