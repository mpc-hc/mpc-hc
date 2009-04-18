@echo off
set PATH=%YASM%;%MINGW64%\bin;%MINGW32%\bin;%PATH%
set INCLUDE=%MINGW64%\x86_64-pc-mingw32\include;%INCLUDE%
IF EXIST "%MINGW32%\bin\mingw32-make.exe" copy /y "%MINGW32%\bin\mingw32-make.exe" "%MINGW32%\bin\make.exe"
IF EXIST "%MINGW64%\x86_64-pc-mingw32\bin\ar.exe" copy /y "%MINGW64%\x86_64-pc-mingw32\bin\ar.exe" "%MINGW64%\bin\x86_64-pc-mingw32-ar.exe"
IF EXIST "%MINGW64%\bin\x86_64-pc-mingw32-make.exe" copy /y "%MINGW64%\bin\x86_64-pc-mingw32-make.exe" "%MINGW64%\bin\make.exe"
IF EXIST "%MINGW64%\bin\x86_64-pc-mingw32-gcc.exe" copy /y "%MINGW64%\bin\x86_64-pc-mingw32-gcc.exe" "%MINGW64%\bin\cc.exe"
IF EXIST "%MINGW64%\bin\x86_64-pc-mingw32-gcc.exe" copy /y "%MINGW64%\bin\x86_64-pc-mingw32-gcc.exe" "%MINGW64%\bin\x86_64-pc-mingw32-cc.exe"

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
