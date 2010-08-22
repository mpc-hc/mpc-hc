@ECHO OFF
IF NOT "x%MINGW32%" == "x" GOTO :VarOk
ECHO: "ERROR : Please define MINGW32 (and/or MSYS) environment variable(s)"
EXIT 1005

:VarOk
SET CC=gcc.exe
SET PATH=%MSYS%\bin;%MINGW32%\bin;%YASM%;%PATH%

IF "%1%"=="rebuild" GOTO :DoClean
IF "%1%"=="clean" GOTO :OnlyClean
GOTO :NoArchClean

:OnlyClean
make.exe -f makefile_2010 clean
GOTO :End

:DoClean
make.exe -f makefile_2010 clean

:NoArchClean
make.exe -f makefile_2010 -j4

:End
