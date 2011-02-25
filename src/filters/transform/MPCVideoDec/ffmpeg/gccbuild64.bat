@ECHO OFF
IF DEFINED MINGW64 GOTO VarOk
ECHO ERROR: Please define MINGW64 (and/or MSYS) environment variable(s)
EXIT /B

:VarOk
SET CC=gcc.exe
SET PATH=%MSYS%\bin;%MINGW64%\bin;%YASM%;%PATH%

FOR /f "tokens=3,4,* delims=. " %%a IN ('gcc -v 2^>^&1 ^| findstr /b /c:"gcc version" ') DO SET "GCCVER=%%a.%%b."
IF "%GCCVER%"=="4.6." (
	SET "makefile=Makefile_gcc_4.6"
) ELSE (
	SET "makefile=Makefile"	
)

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
make.exe -f %makefile% 64BIT=yes clean
EXIT /B

:DoClean
make.exe -f %makefile% 64BIT=yes clean

:Build
make.exe -f %makefile% 64BIT=yes -j4
EXIT /B
