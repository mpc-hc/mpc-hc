@echo off

IF "%VS90COMNTOOLS%"=="" goto BadPaths
IF "%MINGW32%"=="" goto BadPaths
IF "%MINGW64%"=="" goto BadPaths
IF "%MINGW32_GCCLIB%"=="" goto BadPaths
IF "%MINGW64_GCCLIB%"=="" goto BadPaths
goto GoodPaths

:BadPaths
echo "Not all build dependencies found. To build you need:"
echo "* Visual Studio 2008 installed"
echo "* MinGW 32 bit build environment with coreMSYS pointed to in MINGW32 env var"
echo "* MinGW 64 bit build environment with coreMSYS pointed to in MINGW64 env var"
echo "* MinGW 32 bit gcc library directory pointed to in MINGW32_GCCLIB env var"
echo "* MinGW 64 bit gcc library directory pointed to in MINGW64_GCCLIB env var"
pause
goto EndGood

:GoodPaths
SET BUILDTYPE=/%1
IF "%1"=="" set BUILDTYPE=/build

SET ORIGPATH="%CD%"
call "%VS90COMNTOOLS%vsvars32.bat"
cd %ORIGPATH%

devenv ../../../mpc-hc.sln %BUILDTYPE% "Release Unicode|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad

devenv mpciconlib.sln %BUILDTYPE% "Release Unicode|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Chinese simplified|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Chinese traditional|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Czech|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode French|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode German|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Hungarian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Italian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Korean|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Polish|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Russian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Slovak|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Spanish|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Turkish|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Ukrainian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Belarusian|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Swedish|Win32"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad

mkdir Build_x86
xcopy "Release Unicode\*.dll" ".\Build_x86\" /y 
xcopy "Release Unicode\*.exe" ".\Build_x86\" /y
xcopy AUTHORS ".\Build_x86\" /y
xcopy ChangeLog ".\Build_x86\" /y
xcopy Homepage.url ".\Build_x86\" /y
xcopy ..\..\..\COPYING ".\Build_x86\" /y

rem goto Nox64
devenv ..\..\..\mpc-hc.sln %BUILDTYPE% "Release Unicode|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpciconlib.sln %BUILDTYPE% "Release Unicode|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Chinese simplified|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Chinese traditional|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Czech|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode French|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode German|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Hungarian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Italian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Korean|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Polish|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Russian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Slovak|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Spanish|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Turkish|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Ukrainian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Unicode Belarusian|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad
devenv mpcresources.sln %BUILDTYPE% "Release Swedish|x64"
IF %ERRORLEVEL% NEQ 0 GOTO EndBad

mkdir Build_x64
xcopy "x64\Release Unicode\*.dll" ".\Build_x64\" /y 
xcopy "x64\Release Unicode\*.exe" ".\Build_x64\" /y
xcopy AUTHORS ".\Build_x64\" /y
xcopy ChangeLog ".\Build_x64\" /y
xcopy Homepage.url ".\Build_x64\" /y
xcopy ..\..\..\COPYING ".\Build_x64\" /y

:Nox64

goto EndGood

:EndBad
echo " "
echo ERROR: Build failed and aborted
pause

:EndGood
