@echo off
if NOT "x%CC%" == "x" goto VarOk
echo "ERROR : please define CC environment variable"
exit 1005

:VarOk

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
