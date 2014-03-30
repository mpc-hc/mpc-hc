@ECHO OFF
REM (C) 2009-2014 see Authors.txt
REM
REM This file is part of MPC-HC.
REM
REM MPC-HC is free software; you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation; either version 3 of the License, or
REM (at your option) any later version.
REM
REM MPC-HC is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM
REM You should have received a copy of the GNU General Public License
REM along with this program.  If not, see <http://www.gnu.org/licenses/>.


SETLOCAL
CD /D %~dp0

REM pre-build checks
IF EXIST "build.user.bat" (
  CALL "build.user.bat"
) ELSE (
  IF DEFINED MINGW32 (SET MPCHC_MINGW32=%MINGW32%) ELSE (GOTO MissingVar)
  IF DEFINED MINGW64 (SET MPCHC_MINGW64=%MINGW64%) ELSE (GOTO MissingVar)
  IF DEFINED MSYS    (SET MPCHC_MSYS=%MSYS%)       ELSE (GOTO MissingVar)
)

IF NOT EXIST "%MPCHC_MINGW32%" GOTO MissingVar
IF NOT EXIST "%MPCHC_MINGW64%" GOTO MissingVar
IF NOT EXIST "%MPCHC_MSYS%"    GOTO MissingVar

SET ARG=/%*
SET ARG=%ARG:/=%
SET ARG=%ARG:-=%
SET ARGB=0
SET ARGBC=0
SET ARGC=0
SET ARGCOMP=0
SET ARGPL=0
SET INPUT=0
SET VALID=0

IF /I "%ARG%" == "?"            GOTO ShowHelp

FOR %%G IN (%ARG%) DO (
  IF /I "%%G" == "help"         GOTO ShowHelp
  IF /I "%%G" == "GetVersion"   ENDLOCAL & SET "FORCE_VER_UPDATE=True" & CALL :SubGetVersion & EXIT /B
  IF /I "%%G" == "CopyDXDll"    ENDLOCAL & CALL :SubCopyDXDll x86 & CALL :SubCopyDXDll x64 & EXIT /B
  IF /I "%%G" == "CopyDX"       ENDLOCAL & CALL :SubCopyDXDll x86 & CALL :SubCopyDXDll x64 & EXIT /B
  IF /I "%%G" == "Build"        SET "BUILDTYPE=Build"    & SET /A ARGB+=1
  IF /I "%%G" == "Clean"        SET "BUILDTYPE=Clean"    & SET /A ARGB+=1  & SET "NO_INST=True" & SET /A "NO_ZIP=True" & SET "NO_LAV=True"
  IF /I "%%G" == "Rebuild"      SET "BUILDTYPE=Rebuild"  & SET /A ARGB+=1  & SET "NO_LAV=True"
  IF /I "%%G" == "Both"         SET "PPLATFORM=Both"     & SET /A ARGPL+=1
  IF /I "%%G" == "Win32"        SET "PPLATFORM=Win32"    & SET /A ARGPL+=1
  IF /I "%%G" == "x86"          SET "PPLATFORM=Win32"    & SET /A ARGPL+=1
  IF /I "%%G" == "x64"          SET "PPLATFORM=x64"      & SET /A ARGPL+=1
  IF /I "%%G" == "All"          SET "CONFIG=All"         & SET /A ARGC+=1
  IF /I "%%G" == "Main"         SET "CONFIG=Main"        & SET /A ARGC+=1  & SET "NO_INST=True" & SET "NO_ZIP=True" 
  IF /I "%%G" == "Filters"      SET "CONFIG=Filters"     & SET /A ARGC+=1  & SET "NO_INST=True" & SET "NO_LITE=True"
  IF /I "%%G" == "Filter"       SET "CONFIG=Filters"     & SET /A ARGC+=1  & SET "NO_INST=True" & SET "NO_LITE=True"
  IF /I "%%G" == "API"          SET "CONFIG=API"         & SET /A ARGC+=1  & SET "NO_INST=True" & SET "NO_ZIP=True" & SET "NO_LITE=True"
  IF /I "%%G" == "MPCHC"        SET "CONFIG=MPCHC"       & SET /A ARGC+=1
  IF /I "%%G" == "MPC-HC"       SET "CONFIG=MPCHC"       & SET /A ARGC+=1
  IF /I "%%G" == "Resources"    SET "CONFIG=Resources"   & SET /A ARGC+=1  & SET "NO_INST=True" & SET "NO_ZIP=True" & SET "NO_LITE=True"
  IF /I "%%G" == "MPCIconLib"   SET "CONFIG=IconLib"     & SET /A ARGC+=1  & SET "NO_INST=True" & SET "NO_ZIP=True" & SET "NO_LITE=True"
  IF /I "%%G" == "IconLib"      SET "CONFIG=IconLib"     & SET /A ARGC+=1  & SET "NO_INST=True" & SET "NO_ZIP=True" & SET "NO_LITE=True"
  IF /I "%%G" == "Translations" SET "CONFIG=Translation" & SET /A ARGC+=1  & SET "NO_INST=True" & SET "NO_ZIP=True" & SET "NO_LITE=True"
  IF /I "%%G" == "Debug"        SET "BUILDCFG=Debug"     & SET /A ARGBC+=1 & SET "NO_INST=True"
  IF /I "%%G" == "Release"      SET "BUILDCFG=Release"   & SET /A ARGBC+=1
  IF /I "%%G" == "VS2013"       SET "COMPILER=VS2013"    & SET /A ARGCOMP+=1
  IF /I "%%G" == "Packages"     SET "PACKAGES=True"      & SET /A VALID+=1
  IF /I "%%G" == "Installer"    SET "INSTALLER=True"     & SET /A VALID+=1
  IF /I "%%G" == "7z"           SET "ZIP=True"           & SET /A VALID+=1
  IF /I "%%G" == "Lite"         SET "MPCHC_LITE=True"    & SET /A VALID+=1
  IF /I "%%G" == "LAVFilters"   SET "CLEAN=LAVFilters"   & SET /A VALID+=1
  IF /I "%%G" == "Silent"       SET "SILENT=True"        & SET /A VALID+=1
  IF /I "%%G" == "Nocolors"     SET "NOCOLORS=True"      & SET /A VALID+=1
  IF /I "%%G" == "Analyze"      SET "ANALYZE=True"       & SET /A VALID+=1
)

FOR %%G IN (%*) DO SET /A INPUT+=1
SET /A VALID+=%ARGB%+%ARGPL%+%ARGC%+%ARGBC%+%ARGCOMP%
IF %VALID% NEQ %INPUT% GOTO UnsupportedSwitch

IF %ARGB%    GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGB% == 0    (SET "BUILDTYPE=Build")
IF %ARGPL%   GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGPL% == 0   (SET "PPLATFORM=Both")
IF %ARGC%    GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGC% == 0    (SET "CONFIG=MPCHC")
IF %ARGBC%   GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGBC% == 0   (SET "BUILDCFG=Release")
IF %ARGCOMP% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGCOMP% == 0 (SET "COMPILER=VS2013")

IF /I "%PACKAGES%" == "True" SET "INSTALLER=True" & SET "ZIP=True"

IF /I "%INSTALLER%" == "True"   IF "%NO_INST%" == "True" GOTO UnsupportedSwitch
IF /I "%ZIP%" == "True"         IF "%NO_ZIP%" == "True"  GOTO UnsupportedSwitch
IF /I "%MPCHC_LITE%" == "True"  IF "%NO_LITE%" == "True" GOTO UnsupportedSwitch
IF /I "%CLEAN%" == "LAVFilters" IF "%NO_LAV%" == "True"  GOTO UnsupportedSwitch

IF NOT DEFINED VS120COMNTOOLS GOTO MissingVar
SET "TOOLSET=%VS120COMNTOOLS%..\..\VC\vcvarsall.bat"
SET "BIN_DIR=bin"

IF EXIST "%~dp0signinfo.txt" (
  IF /I "%INSTALLER%" == "True" SET "SIGN=True"
  IF /I "%ZIP%" == "True"       SET "SIGN=True"
)


:Start
REM Check if the %LOG_DIR% folder exists otherwise MSBuild will fail
SET "LOG_DIR=%BIN_DIR%\logs"
IF NOT EXIST "%LOG_DIR%" MD "%LOG_DIR%"

IF DEFINED MPCHC_LITE SET "BUILDCFG=%BUILDCFG% Lite"

SET "MSBUILD_SWITCHES=/nologo /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true"

SET START_TIME=%TIME%
SET START_DATE=%DATE%

IF /I "%PPLATFORM%" == "Both" (
  SET "PPLATFORM=Win32" & CALL :Main
  SET "PPLATFORM=x64"   & CALL :Main
) ELSE (
  CALL :Main
)
GOTO End


:Main
IF %ERRORLEVEL% NEQ 0 EXIT /B

IF /I "%PPLATFORM%" == "x64" (
  SET "LAVFILTERSDIR=LAVFilters64"
) ELSE (
  SET "LAVFILTERSDIR=LAVFilters"
)

IF /I "%CLEAN%" == "LAVFilters" CALL "src\thirdparty\LAVFilters\build_lavfilters.bat" Clean %PPLATFORM% %BUILDCFG% %COMPILER%
IF %ERRORLEVEL% NEQ 0 ENDLOCAL & EXIT /B

REM Always use x86_amd64 compiler, even on 64bit windows, because this is what VS is doing
IF /I "%PPLATFORM%" == "Win32" (SET ARCH=x86) ELSE (SET ARCH=x86_amd64)
CALL "%TOOLSET%" %ARCH%
IF %ERRORLEVEL% NEQ 0 GOTO MissingVar

IF /I "%CONFIG%" == "Filters" (
  CALL :SubFilters %PPLATFORM%
  IF /I "%ZIP%" == "True" CALL :SubCreatePackages Filters %PPLATFORM%
  EXIT /B
)

IF /I "%CONFIG%" == "IconLib" (
  CALL :SubMPCIconLib %PPLATFORM%
  EXIT /B
)

IF /I "%CONFIG%" == "Translation" (
  CALL :SubMPCRresources %PPLATFORM%
  EXIT /B
)

IF /I "%CONFIG%" == "API" (
  CALL :SubMPCTestAPI %PPLATFORM%
  EXIT /B
)

IF /I "%CONFIG%" NEQ "Resources" CALL :SubMPCHC %PPLATFORM%
IF /I "%CONFIG%" NEQ "Main"      CALL :SubResources %PPLATFORM%

IF /I "%INSTALLER%" == "True" CALL :SubCreateInstaller %PPLATFORM%
IF /I "%ZIP%" == "True"       CALL :SubCreatePackages MPC-HC %PPLATFORM%

IF /I "%CONFIG%" == "All" (
  CALL :SubFilters %PPLATFORM%
  IF /I "%ZIP%" == "True" CALL :SubCreatePackages Filters %PPLATFORM%
)
EXIT /B


:End
IF %ERRORLEVEL% NEQ 0 EXIT /B
TITLE Compiling MPC-HC %COMPILER% [FINISHED]
SET END_TIME=%TIME%
CALL :SubGetDuration
CALL :SubMsg "INFO" "Compilation started on %START_DATE%-%START_TIME% and completed on %DATE%-%END_TIME% [%DURATION%]"
ENDLOCAL
EXIT /B


:SubFilters
IF %ERRORLEVEL% NEQ 0 EXIT /B

TITLE Compiling MPC-HC Filters %COMPILER% - %BUILDCFG% Filter^|%1...
REM Call update_version.bat before building the filters
CALL "update_version.bat"

MSBuild.exe mpc-hc.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration="%BUILDCFG% Filter";Platform=%1^
 /flp1:LogFile=%LOG_DIR%\filters_errors_%BUILDCFG%_%1.log;errorsonly;Verbosity=diagnostic^
 /flp2:LogFile=%LOG_DIR%\filters_warnings_%BUILDCFG%_%1.log;warningsonly;Verbosity=diagnostic
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpc-hc.sln %BUILDCFG% Filter %1 - Compilation failed!"
  EXIT /B
) ELSE (
  CALL :SubMsg "INFO" "mpc-hc.sln %BUILDCFG% Filter %1 compiled successfully"
)
IF /I "%SIGN%" == "True" CALL :SubSign Filters *.ax
IF /I "%SIGN%" == "True" CALL :SubSign Filters VSFilter.dll
EXIT /B


:SubMPCHC
IF %ERRORLEVEL% NEQ 0 EXIT /B

TITLE Compiling MPC-HC %COMPILER% - %BUILDCFG%^|%1...
MSBuild.exe mpc-hc.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration="%BUILDCFG%";Platform=%1^
 /flp1:LogFile="%LOG_DIR%\mpc-hc_errors_%BUILDCFG%_%1.log";errorsonly;Verbosity=diagnostic^
 /flp2:LogFile="%LOG_DIR%\mpc-hc_warnings_%BUILDCFG%_%1.log";warningsonly;Verbosity=diagnostic
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpc-hc.sln %BUILDCFG% %1 - Compilation failed!"
  EXIT /B
) ELSE (
  CALL :SubMsg "INFO" "mpc-hc.sln %BUILDCFG% %1 compiled successfully"
)
IF /I "%SIGN%" == "True" CALL :SubSign MPC-HC mpc-hc*.exe
IF /I "%SIGN%" == "True" CALL :SubSign MPC-HC *.dll %LAVFILTERSDIR%
IF /I "%SIGN%" == "True" CALL :SubSign MPC-HC *.ax %LAVFILTERSDIR%

EXIT /B


:SubResources
IF %ERRORLEVEL% NEQ 0 EXIT /B

IF /I "%BUILDCFG%" == "Debug" (
  CALL :SubMsg "WARNING" "/debug was used, resources will not be built"
  EXIT /B
)

CALL :SubMPCIconLib %1

IF DEFINED MPCHC_LITE (
  CALL :SubMsg "WARNING" "/lite was used, translations will not be built"
  EXIT /B
)

CALL :SubMPCRresources %1
EXIT /B


:SubMPCIconLib
IF %ERRORLEVEL% NEQ 0 EXIT /B

TITLE Compiling mpciconlib %COMPILER% - Release^|%1...
MSBuild.exe mpciconlib.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration=Release;Platform=%1
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpciconlib.sln %1 - Compilation failed!"
  EXIT /B
) ELSE (
  CALL :SubMsg "INFO" "mpciconlib.sln %1 compiled successfully"
)
IF /I "%SIGN%" == "True" CALL :SubSign MPC-HC mpciconlib.dll
EXIT /B


:SubMPCRresources
IF %ERRORLEVEL% NEQ 0 EXIT /B

FOR %%G IN ("Armenian" "Basque" "Belarusian" "Bengali" "Catalan" "Chinese Simplified"
 "Chinese Traditional" "Croatian" "Czech" "Dutch" "English (British)" "French"
 "Galician" "German" "Greek" "Hebrew" "Hungarian" "Italian" "Japanese" "Korean"
 "Malay" "Polish" "Portuguese (Brazil)" "Romanian" "Russian" "Slovak" "Slovenian"
 "Spanish" "Swedish" "Tatar" "Turkish" "Ukrainian" "Vietnamese"
) DO (
 TITLE Compiling mpcresources %COMPILER% - %%~G^|%1...
 MSBuild.exe mpcresources.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration="Release %%~G";Platform=%1
 IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!" & EXIT /B
)
IF /I "%SIGN%" == "True" CALL :SubSign MPC-HC mpcresources.??.dll Lang
IF /I "%SIGN%" == "True" CALL :SubSign MPC-HC mpcresources.??_??.dll Lang
EXIT /B


:SubMPCTestAPI
IF %ERRORLEVEL% NEQ 0 EXIT /B

PUSHD "src\MPCTestAPI"
TITLE Compiling MPCTestAPI %COMPILER% - %BUILDCFG%^|%1...
MSBuild.exe MPCTestAPI.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration=%BUILDCFG%;Platform=%1
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "MPCTestAPI.sln %1 - Compilation failed!"
  EXIT /B
) ELSE (
  CALL :SubMsg "INFO" "MPCTestAPI.sln %1 compiled successfully"
)
POPD
EXIT /B


:SubSign
IF %ERRORLEVEL% NEQ 0 EXIT /B
REM %1 is Filters or MPC-HC
REM %2 is name of the file to sign
REM %3 is the subfolder

IF /I "%PPLATFORM%" == "Win32" PUSHD "%BIN_DIR%\%~1_x86\%3"
IF /I "%PPLATFORM%" == "x64"   PUSHD "%BIN_DIR%\%~1_x64\%3"

FOR /F "delims=" %%A IN ('DIR "%2" /b') DO (
  CALL "%~dp0contrib\sign.bat" "%%A" || (CALL :SubMsg "ERROR" "Problem signing %%A" & GOTO Break)
)
CALL :SubMsg "INFO" "%2 signed successfully."

:Break
POPD
EXIT /B


:SubCopyDXDll
IF /I "%BUILDCFG%" == "Debug" EXIT /B
PUSHD "%BIN_DIR%"
EXPAND "%DXSDK_DIR%\Redist\Jun2010_D3DCompiler_43_%~1.cab" -F:D3DCompiler_43.dll mpc-hc_%~1
IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Problem when extracting %DXSDK_DIR%\Redist\Jun2010_D3DCompiler_43_%~1.cab" & EXIT /B
EXPAND "%DXSDK_DIR%\Redist\Jun2010_d3dx9_43_%~1.cab" -F:d3dx9_43.dll mpc-hc_%~1
IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Problem when extracting Jun2010_d3dx9_43_%~1.cab" & EXIT /B
POPD
EXIT /B


:SubCreateInstaller
IF %ERRORLEVEL% NEQ 0 EXIT /B

IF DEFINED MPCHC_LITE SET MPCHC_INNO_DEF=%MPCHC_INNO_DEF% /DMPCHC_LITE
IF /I "%~1" == "x64" (
  SET MPCHC_INNO_DEF=%MPCHC_INNO_DEF% /Dx64Build
  CALL :SubCopyDXDll x64
) ELSE CALL :SubCopyDXDll x86

CALL :SubDetectInnoSetup

IF NOT DEFINED InnoSetupPath (
  CALL :SubMsg "WARNING" "Inno Setup wasn't found, the %1 installer wasn't built"
  EXIT /B
)

TITLE Compiling %1 %COMPILER% installer...
"%InnoSetupPath%" /SMySignTool="cmd /c "%~dp0contrib\sign.bat" $f" /Q /O"%BIN_DIR%"^
 "distrib\mpc-hc_setup.iss" %MPCHC_INNO_DEF%
IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!" & EXIT /B
CALL :SubMsg "INFO" "%1 installer successfully built"

EXIT /B


:SubCreatePackages
IF %ERRORLEVEL% NEQ 0 EXIT /B

CALL :SubDetectSevenzipPath
CALL :SubGetVersion

IF NOT DEFINED SEVENZIP (
  CALL :SubMsg "WARNING" "7-Zip wasn't found, the %1 %2 package wasn't built"
  EXIT /B
)

IF /I "%~1" == "Filters" (SET "NAME=MPC-HC_standalone_filters") ELSE (SET "NAME=MPC-HC")
IF /I "%~2" == "Win32" (
  SET ARCH=x86
  CALL :SubCopyDXDll x86
) ELSE (
  SET ARCH=x64
  CALL :SubCopyDXDll x64
)

PUSHD "%BIN_DIR%"

SET "VS_OUT_DIR=%~1_%ARCH%"
SET "PCKG_NAME=%NAME%.%MPCHC_VER%.%ARCH%"
IF DEFINED MPCHC_LITE (SET "PCKG_NAME=%PCKG_NAME%.Lite")
IF /I "%BUILDCFG%" == "Debug" (
  SET "PCKG_NAME=%PCKG_NAME%.dbg"
  SET "VS_OUT_DIR=%VS_OUT_DIR%_Debug"
)

IF EXIST "%PCKG_NAME%.7z"     DEL "%PCKG_NAME%.7z"
IF EXIST "%PCKG_NAME%.pdb.7z" DEL "%PCKG_NAME%.pdb.7z"
IF EXIST "%PCKG_NAME%"        RD /Q /S "%PCKG_NAME%"

SET "PDB_FILES=*.pdb"
IF NOT DEFINED MPCHC_LITE (SET "PDB_FILES=%PDB_FILES% %LAVFILTERSDIR%\*.pdb")

REM Compress the pdb file for mpc-hc only
IF /I "%NAME%" == "MPC-HC" (
  PUSHD "%VS_OUT_DIR%"
  TITLE Creating archive %PCKG_NAME%.pdb.7z...
  START "7z" /B /WAIT "%SEVENZIP%" a -t7z "%PCKG_NAME%.pdb.7z" %PDB_FILES% -m0=LZMA -mx9 -ms=on
  IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Unable to create %PCKG_NAME%.pdb.7z!" & EXIT /B
  CALL :SubMsg "INFO" "%PCKG_NAME%.pdb.7z successfully created"
  IF EXIST "%PCKG_NAME%.pdb.7z" MOVE /Y "%PCKG_NAME%.pdb.7z" ".." >NUL
  POPD
)

TITLE Copying %PCKG_NAME%...
IF NOT EXIST "%PCKG_NAME%" MD "%PCKG_NAME%"

IF /I "%NAME%" == "MPC-HC" (
  IF NOT DEFINED MPCHC_LITE (
    IF /I "%BUILDCFG%" NEQ "Debug" (
      IF NOT EXIST "%PCKG_NAME%\Lang"          MD "%PCKG_NAME%\Lang"
    )
    IF NOT EXIST "%PCKG_NAME%\%LAVFILTERSDIR%" MD "%PCKG_NAME%\%LAVFILTERSDIR%"
  )
  IF /I "%ARCH%" == "x64" (
    COPY /Y /V "%VS_OUT_DIR%\mpc-hc64.exe" "%PCKG_NAME%\mpc-hc64.exe" >NUL
  ) ELSE (
    COPY /Y /V "%VS_OUT_DIR%\mpc-hc.exe"   "%PCKG_NAME%\mpc-hc.exe" >NUL
  )
  COPY /Y /V "%VS_OUT_DIR%\mpciconlib.dll"                "%PCKG_NAME%\*.dll" >NUL
  IF NOT DEFINED MPCHC_LITE (
    COPY /Y /V "%VS_OUT_DIR%\Lang\mpcresources.??.dll"    "%PCKG_NAME%\Lang\" >NUL
    COPY /Y /V "%VS_OUT_DIR%\Lang\mpcresources.??_??.dll" "%PCKG_NAME%\Lang\" >NUL
    COPY /Y /V "%VS_OUT_DIR%\%LAVFILTERSDIR%\*.ax"        "%PCKG_NAME%\%LAVFILTERSDIR%" >NUL
    COPY /Y /V "%VS_OUT_DIR%\%LAVFILTERSDIR%\*.dll"       "%PCKG_NAME%\%LAVFILTERSDIR%" >NUL
    COPY /Y /V "%VS_OUT_DIR%\%LAVFILTERSDIR%\*.manifest"  "%PCKG_NAME%\%LAVFILTERSDIR%" >NUL
  )
  COPY /Y /V "%VS_OUT_DIR%\D3DCompiler_43.dll"            "%PCKG_NAME%\D3DCompiler_43.dll" >NUL
  COPY /Y /V "%VS_OUT_DIR%\d3dx9_43.dll"                  "%PCKG_NAME%\d3dx9_43.dll" >NUL
  IF NOT EXIST "%PCKG_NAME%\Shaders" MD "%PCKG_NAME%\Shaders"
  COPY /Y /V "..\src\mpc-hc\res\shaders\external\*.hlsl" "%PCKG_NAME%\Shaders" >NUL
) ELSE (
  COPY /Y /V "%VS_OUT_DIR%\*.ax"           "%PCKG_NAME%\*.ax" >NUL
  COPY /Y /V "%VS_OUT_DIR%\VSFilter.dll"   "%PCKG_NAME%\VSFilter.dll" >NUL
)

COPY /Y /V "..\COPYING.txt"         "%PCKG_NAME%" >NUL
COPY /Y /V "..\docs\Authors.txt"    "%PCKG_NAME%" >NUL
COPY /Y /V "..\docs\Changelog.txt"  "%PCKG_NAME%" >NUL
COPY /Y /V "..\docs\Readme.txt"     "%PCKG_NAME%" >NUL

TITLE Creating archive %PCKG_NAME%.7z...
START "7z" /B /WAIT "%SEVENZIP%" a -t7z "%PCKG_NAME%.7z" "%PCKG_NAME%"^
 -m0=LZMA -mx9 -ms=on
IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Unable to create %PCKG_NAME%.7z!" & EXIT /B
CALL :SubMsg "INFO" "%PCKG_NAME%.7z successfully created"

IF EXIST "%PCKG_NAME%" RD /Q /S "%PCKG_NAME%"

POPD
EXIT /B


:SubGetVersion
REM Get the version
IF NOT EXIST "include\version_rev.h" SET "FORCE_VER_UPDATE=True"
IF /I "%FORCE_VER_UPDATE%" == "True" CALL "update_version.bat" && SET "FORCE_VER_UPDATE=False"

FOR /F "tokens=2,3" %%A IN ('FINDSTR /R /C:"define MPC_VERSION_[M,P]" "include\version.h"') DO (
  SET "%%A=%%B"
)

FOR /F "tokens=2,3" %%A IN ('FINDSTR /R /C:"define MPC" "include\version_rev.h"') DO (
  SET "%%A=%%B"
)

FOR /F "tokens=3" %%A IN ('FINDSTR /R /C:"define MPC_NIGHTLY_RELEASE" "include\version.h"') DO (
  SET "MPCHC_NIGHTLY=%%A"
)

SET "MPCHC_HASH=%MPCHC_HASH:~4,-2%"
IF DEFINED MPCHC_BRANCH (
  SET "MPCHC_BRANCH=%MPCHC_BRANCH:~4,-2%"
)

IF "%MPCHC_NIGHTLY%" NEQ "0" (
  SET "MPCHC_VER=%MPC_VERSION_MAJOR%.%MPC_VERSION_MINOR%.%MPC_VERSION_PATCH%.%MPC_VERSION_REV%"
) ELSE (
  SET "MPCHC_VER=%MPC_VERSION_MAJOR%.%MPC_VERSION_MINOR%.%MPC_VERSION_PATCH%"
)
EXIT /B


:SubDetectInnoSetup
FOR /F "tokens=5*" %%A IN (
  'REG QUERY "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 5_is1" /v "Inno Setup: App Path" 2^>NUL ^| FIND "REG_SZ" ^|^|
   REG QUERY "HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 5_is1" /v "Inno Setup: App Path" 2^>NUL ^| FIND "REG_SZ"') DO SET "InnoSetupPath=%%B\ISCC.exe"
EXIT /B


:SubDetectSevenzipPath
FOR %%G IN (7z.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

FOR %%G IN (7za.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

FOR /F "tokens=2*" %%A IN (
  'REG QUERY "HKLM\SOFTWARE\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ" ^|^|
   REG QUERY "HKLM\SOFTWARE\Wow6432Node\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ"') DO SET "SEVENZIP=%%B\7z.exe"
EXIT /B


:ShowHelp
TITLE %~nx0 Help
ECHO.
ECHO Usage:
ECHO %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|Both] [Main^|Resources^|MPCHC^|IconLib^|Translations^|Filters^|API^|All] [Debug^|Release] [Lite] [Packages^|Installer^|7z] [LAVFilters] [VS2013] [Analyze]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        Debug only applies to mpc-hc.sln.
ECHO        The arguments are not case sensitive and can be ommitted.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments will use the default ones:
ECHO "%~nx0 Build Both MPCHC Release VS2013"
ECHO. & ECHO.
ECHO Examples:
ECHO %~nx0 x86 Resources     -Builds the x86 resources
ECHO %~nx0 Resources         -Builds both x86 and x64 resources
ECHO %~nx0 x86               -Builds x86 Main exe and the x86 resources
ECHO %~nx0 x86 Debug         -Builds x86 Main Debug exe and x86 resources
ECHO %~nx0 x86 Filters       -Builds x86 Filters
ECHO %~nx0 x86 All           -Builds x86 Main exe, x86 Filters and the x86 resources
ECHO %~nx0 x86 Packages      -Builds x86 Main exe, x86 resources and creates the installer and the .7z package
ECHO %~nx0 x64 LAVFilters 7z -Rebuilds LAVFilters, builds x64 Main exe, x64 resources and creates the .7z package
ECHO.
ENDLOCAL
EXIT /B


:MissingVar
COLOR 0C
TITLE Compiling MPC-HC %COMPILER% [ERROR]
ECHO Not all build dependencies were found.
ECHO.
ECHO See "docs\Compilation.txt" for more information.
CALL :SubMsg "ERROR" "Compilation failed!" & EXIT /B


:UnsupportedSwitch
ECHO.
ECHO Unsupported commandline switch!
ECHO.
ECHO "%~nx0 %*"
ECHO.
ECHO Run "%~nx0 help" for details about the commandline switches.
CALL :SubMsg "ERROR" "Compilation failed!" & EXIT /B


:SubMsg
ECHO. & ECHO ------------------------------
IF /I "%~1" == "ERROR" (
  CALL :SubColorText "0C" "[%~1]" "%~2"
) ELSE IF /I "%~1" == "INFO" (
  CALL :SubColorText "0A" "[%~1]" "%~2"
) ELSE IF /I "%~1" == "WARNING" (
  CALL :SubColorText "0E" "[%~1]" "%~2"
)
ECHO ------------------------------ & ECHO.
IF /I "%~1" == "ERROR" (
  IF NOT DEFINED SILENT (
    ECHO Press any key to exit...
    PAUSE >NUL
  )
  ENDLOCAL
  EXIT /B 1
) ELSE (
  EXIT /B
)


:SubColorText
IF DEFINED NOCOLORS ECHO %~2 %~3 & EXIT /B
FOR /F "tokens=1,2 delims=#" %%G IN (
  '"PROMPT #$H#$E# & ECHO ON & FOR %%H IN (1) DO REM"') DO (
  SET "DEL=%%G")
<NUL SET /p ".=%DEL%" > "%~2"
FINDSTR /v /a:%1 /R ".18" "%~2" NUL
DEL "%~2" > NUL 2>&1
REM The space in the following ECHO is intentional
ECHO  %~3
EXIT /B


:SubGetDuration
SET START_TIME=%START_TIME: =%
SET END_TIME=%END_TIME: =%

FOR /F "tokens=1-4 delims=:.," %%G IN ("%START_TIME%") DO (
  SET /A "STARTTIME=(100%%G %% 100) * 360000 + (100%%H %% 100) * 6000 + (100%%I %% 100) * 100 + (100%%J %% 100)"
)

FOR /F "tokens=1-4 delims=:.," %%G IN ("%END_TIME%") DO (
  SET /A "ENDTIME=(100%%G %% 100) * 360000 + (100%%H %% 100) * 6000 + (100%%I %% 100) * 100 + (100%%J %% 100)"
)

SET /A DURATION=%ENDTIME%-%STARTTIME%
IF %ENDTIME% LSS %STARTTIME% SET /A "DURATION+=24 * 360000"

SET /A DURATIONH=%DURATION% / 360000
SET /A DURATIONM=(%DURATION% - %DURATIONH%*360000) / 6000
SET /A DURATIONS=(%DURATION% - %DURATIONH%*360000 - %DURATIONM%*6000) / 100
SET /A DURATIONHS=(%DURATION% - %DURATIONH%*360000 - %DURATIONM%*6000 - %DURATIONS%*100)*10

IF %DURATIONH%  EQU 0 (SET DURATIONH=)  ELSE (SET DURATIONH=%DURATIONH%h )
IF %DURATIONM%  EQU 0 (SET DURATIONM=)  ELSE (SET DURATIONM=%DURATIONM%m )
IF %DURATIONS%  EQU 0 (SET DURATIONS=)  ELSE (SET DURATIONS=%DURATIONS%s )
IF %DURATIONHS% EQU 0 (SET DURATIONHS=) ELSE (SET DURATIONHS=%DURATIONHS%ms)

SET "DURATION=%DURATIONH%%DURATIONM%%DURATIONS%%DURATIONHS%"
EXIT /B
