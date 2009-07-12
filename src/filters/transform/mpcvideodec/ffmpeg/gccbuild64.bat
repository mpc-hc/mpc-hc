@echo off
if NOT "x%CC%" == "x" goto VarOk
echo "ERROR : please define CC environment variable"
exit 1005

:VarOk

IF "%1%"=="rebuild" goto DoClean
IF "%1%"=="clean" goto OnlyClean
goto NoClean

:OnlyClean
make.exe 64BIT=yes clean
IF EXIST 32bit.build rm 32bit.build
goto End

:DoClean
make.exe 64BIT=yes clean
IF EXIST 32bit.build rm 32bit.build

:NoClean
IF NOT EXIST 32bit.build GOTO NoArchClean
make.exe 64BIT=yes clean
rm 32bit.build

:NoArchClean
touch 64bit.build
make.exe 64BIT=yes -j8

:End
