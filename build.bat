@ECHO OFF
REM (C) 2009-2012 see Authors.txt
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
IF NOT DEFINED VS100COMNTOOLS GOTO MissingVar

SET ARG=/%*
SET ARG=%ARG:/=%
SET ARG=%ARG:-=%
SET ARGB=0
SET ARGBC=0
SET ARGC=0
SET ARGCL=0
SET ARGD=0
SET ARGF=0
SET ARGL=0
SET ARGM=0
SET ARGPL=0
SET ARGRE=0
SET INPUT=0
SET VALID=0

IF /I "%ARG%" == "?"          GOTO ShowHelp

FOR %%G IN (%ARG%) DO (
  IF /I "%%G" == "help"       GOTO ShowHelp
  IF /I "%%G" == "GetVersion" ENDLOCAL & CALL :SubGetVersion & EXIT /B
  IF /I "%%G" == "Build"      SET "BUILDTYPE=Build"   & SET /A ARGB+=1
  IF /I "%%G" == "Clean"      SET "BUILDTYPE=Clean"   & SET /A ARGB+=1  & SET /A ARGCL+=1
  IF /I "%%G" == "Rebuild"    SET "BUILDTYPE=Rebuild" & SET /A ARGB+=1  & SET /A ARGRE+=1
  IF /I "%%G" == "Both"       SET "PPLATFORM=Both"    & SET /A ARGPL+=1
  IF /I "%%G" == "Win32"      SET "PPLATFORM=Win32"   & SET /A ARGPL+=1
  IF /I "%%G" == "x86"        SET "PPLATFORM=Win32"   & SET /A ARGPL+=1
  IF /I "%%G" == "x64"        SET "PPLATFORM=x64"     & SET /A ARGPL+=1
  IF /I "%%G" == "All"        SET "CONFIG=All"        & SET /A ARGC+=1
  IF /I "%%G" == "Main"       SET "CONFIG=Main"       & SET /A ARGC+=1  & SET /A ARGM+=1
  IF /I "%%G" == "Filters"    SET "CONFIG=Filters"    & SET /A ARGC+=1  & SET /A ARGF+=1 & SET /A ARGL+=1
  IF /I "%%G" == "Filter"     SET "CONFIG=Filters"    & SET /A ARGC+=1  & SET /A ARGF+=1 & SET /A ARGL+=1
  IF /I "%%G" == "MPCHC"      SET "CONFIG=MPCHC"      & SET /A ARGC+=1
  IF /I "%%G" == "MPC-HC"     SET "CONFIG=MPCHC"      & SET /A ARGC+=1
  IF /I "%%G" == "Resource"   SET "CONFIG=Resources"  & SET /A ARGC+=1  & SET /A ARGD+=1
  IF /I "%%G" == "Resources"  SET "CONFIG=Resources"  & SET /A ARGC+=1  & SET /A ARGD+=1
  IF /I "%%G" == "Debug"      SET "BUILDCFG=Debug"    & SET /A ARGBC+=1 & SET /A ARGD+=1
  IF /I "%%G" == "Release"    SET "BUILDCFG=Release"  & SET /A ARGBC+=1
  IF /I "%%G" == "Packages"   SET "PACKAGES=True"     & SET /A VALID+=1 & SET /A ARGCL+=1 & SET /A ARGD+=1 & SET /A ARGF+=1 & SET /A ARGM+=1
  IF /I "%%G" == "Installer"  SET "INSTALLER=True"    & SET /A VALID+=1 & SET /A ARGCL+=1 & SET /A ARGD+=1 & SET /A ARGF+=1 & SET /A ARGM+=1
  IF /I "%%G" == "Zip"        SET "ZIP=True"          & SET /A VALID+=1 & SET /A ARGCL+=1 & SET /A ARGM+=1
  IF /I "%%G" == "7z"         SET "ZIP=True"          & SET /A VALID+=1 & SET /A ARGCL+=1 & SET /A ARGM+=1
  IF /I "%%G" == "Lite"       SET "MPCHC_LITE=True"   & SET /A VALID+=1 & SET /A ARGL+=1
  IF /I "%%G" == "FFmpeg"     SET "Rebuild=FFmpeg"    & SET /A VALID+=1 & SET /A ARGCL+=1 & SET /A ARGRE+=1
)

FOR %%G IN (%*) DO SET /A INPUT+=1
SET /A VALID+=%ARGB%+%ARGPL%+%ARGC%+%ARGBC%

IF %VALID% NEQ %INPUT% GOTO UnsupportedSwitch

IF %ARGB%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGB% == 0  (SET "BUILDTYPE=Build")
IF %ARGPL% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGPL% == 0 (SET "PPLATFORM=Both")
IF %ARGC%  GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGC% == 0  (SET "CONFIG=MPCHC")
IF %ARGBC% GTR 1 (GOTO UnsupportedSwitch) ELSE IF %ARGBC% == 0 (SET "BUILDCFG=Release")
IF %ARGCL% GTR 1 (GOTO UnsupportedSwitch)
IF %ARGD%  GTR 1 (GOTO UnsupportedSwitch)
IF %ARGF%  GTR 1 (GOTO UnsupportedSwitch)
IF %ARGL%  GTR 1 (GOTO UnsupportedSwitch)
IF %ARGM%  GTR 1 (GOTO UnsupportedSwitch)
IF %ARGRE% GTR 1 (GOTO UnsupportedSwitch)


:Start
REM Check if the %LOG_DIR% folder exists otherwise MSBuild will fail
SET "LOG_DIR=bin\logs"
IF NOT EXIST "%LOG_DIR%" MD "%LOG_DIR%"

IF /I "%PACKAGES%" == "True" SET "INSTALLER=True" & SET "ZIP=True"
IF DEFINED MPCHC_LITE SET "BUILDCFG=%BUILDCFG% Lite"

CALL :SubDetectWinArch

SET "MSBUILD=%WINDIR%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe"
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

IF /I "%Rebuild%" == "FFmpeg" CALL "src\thirdparty\ffmpeg\gccbuild.bat" Rebuild %PPLATFORM% %BUILDCFG%
IF %ERRORLEVEL% NEQ 0 EXIT /B

IF /I "%PPLATFORM%" == "Win32" (SET ARCH=x86) ELSE (SET ARCH=%x64_type%)
CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" %ARCH%

IF /I "%CONFIG%" == "Filters" (
  CALL :SubFilters %PPLATFORM%
  IF /I "%ZIP%" == "True" CALL :SubCreatePackages Filters %PPLATFORM%
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

TITLE Compiling MPC-HC [FINISHED]
SET END_TIME=%TIME%
CALL :SubGetDuration
CALL :SubMsg "INFO" "Compilation started on %START_DATE%-%START_TIME% and completed on %DATE%-%END_TIME% [%DURATION%]"
ENDLOCAL
EXIT /B


:SubFilters
IF %ERRORLEVEL% NEQ 0 EXIT /B

TITLE Compiling MPC-HC Filters - %BUILDCFG% Filter^|%1...
REM Call update_version.bat before building the filters
CALL "update_version.bat"

"%MSBUILD%" mpc-hc.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration="%BUILDCFG% Filter";PPLATFORM=%1^
 /flp1:LogFile=%LOG_DIR%\filters_errors_%BUILDCFG%_%1.log;errorsonly;Verbosity=diagnostic^
 /flp2:LogFile=%LOG_DIR%\filters_warnings_%BUILDCFG%_%1.log;warningsonly;Verbosity=diagnostic
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpc-hc.sln %BUILDCFG% Filter %1 - Compilation failed!"
  EXIT /B
) ELSE (
  CALL :SubMsg "INFO" "mpc-hc.sln %BUILDCFG% Filter %1 compiled successfully"
)
EXIT /B


:SubMPCHC
IF %ERRORLEVEL% NEQ 0 EXIT /B

TITLE Compiling MPC-HC - %BUILDCFG%^|%1...
"%MSBUILD%" mpc-hc.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration="%BUILDCFG%";PPLATFORM=%1^
 /flp1:LogFile="%LOG_DIR%\mpc-hc_errors_%BUILDCFG%_%1.log";errorsonly;Verbosity=diagnostic^
 /flp2:LogFile="%LOG_DIR%\mpc-hc_warnings_%BUILDCFG%_%1.log";warningsonly;Verbosity=diagnostic
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpc-hc.sln %BUILDCFG% %1 - Compilation failed!"
  EXIT /B
) ELSE (
  CALL :SubMsg "INFO" "mpc-hc.sln %BUILDCFG% %1 compiled successfully"
)
EXIT /B


:SubResources
IF %ERRORLEVEL% NEQ 0 EXIT /B

IF /I "%BUILDCFG%" == "Debug" (
  CALL :SubMsg "WARNING" "/debug was used, resources will not be built"
  EXIT /B
)

TITLE Compiling mpciconlib - Release^|%1...
"%MSBUILD%" mpciconlib.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration=Release;PPLATFORM=%1
IF %ERRORLEVEL% NEQ 0 (
  CALL :SubMsg "ERROR" "mpciconlib.sln %1 - Compilation failed!"
  EXIT /B
) ELSE (
  CALL :SubMsg "INFO" "mpciconlib.sln %1 compiled successfully"
)

IF DEFINED MPCHC_LITE (
  CALL :SubMsg "WARNING" "/lite was used, translations will not be built"
  EXIT /B
)

FOR %%G IN ("Armenian" "Basque" "Belarusian" "Catalan" "Chinese Simplified"
 "Chinese Traditional" "Czech" "Dutch" "French" "German" "Hebrew" "Hungarian"
 "Italian" "Japanese" "Korean" "Polish" "Portuguese" "Russian" "Slovak" "Spanish"
 "Swedish" "Turkish" "Ukrainian"
) DO (
 TITLE Compiling mpcresources - %%~G^|%1...
 "%MSBUILD%" mpcresources.sln %MSBUILD_SWITCHES%^
 /target:%BUILDTYPE% /property:Configuration="Release %%~G";PPLATFORM=%1
 IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!" & EXIT /B
)
EXIT /B


:SubCopyDXDll
PUSHD "bin"
EXPAND "%DXSDK_DIR%\Redist\Jun2010_D3DCompiler_43_%~1.cab" -F:D3DCompiler_43.dll mpc-hc_%~1 >NUL
EXPAND "%DXSDK_DIR%\Redist\Jun2010_d3dx9_43_%~1.cab" -F:d3dx9_43.dll mpc-hc_%~1 >NUL
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

TITLE Compiling %1 installer...
"%InnoSetupPath%" /Q /O"bin" "distrib\mpc-hc_setup.iss" %MPCHC_INNO_DEF%
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

PUSHD "bin"

IF DEFINED MPCHC_LITE (
  SET "PCKG_NAME=%NAME%.%MPCHC_VER%.%ARCH%.Lite"
) ELSE (
  SET "PCKG_NAME=%NAME%.%MPCHC_VER%.%ARCH%"
)

IF EXIST "%PCKG_NAME%.7z"     DEL "%PCKG_NAME%.7z"
IF EXIST "%PCKG_NAME%.pdb.7z" DEL "%PCKG_NAME%.pdb.7z"
IF EXIST "%PCKG_NAME%"        RD /Q /S "%PCKG_NAME%"

REM Compress the pdb file for mpc-hc only
IF /I "%NAME%" == "MPC-HC" (
  PUSHD "%~1_%ARCH%"
  TITLE Creating archive %PCKG_NAME%.pdb.7z...
  START "7z" /B /WAIT "%SEVENZIP%" a -t7z "%PCKG_NAME%.pdb.7z" "*.pdb" -m0=LZMA -mx9 -ms=on
  IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Unable to create %PCKG_NAME%.pdb.7z!" & EXIT /B
  CALL :SubMsg "INFO" "%PCKG_NAME%.pdb.7z successfully created"
  IF EXIST "%PCKG_NAME%.pdb.7z" MOVE /Y "%PCKG_NAME%.pdb.7z" ".." >NUL
  POPD
)

TITLE Copying %PCKG_NAME%...
IF NOT EXIST "%PCKG_NAME%" MD "%PCKG_NAME%"

IF /I "%NAME%" == "MPC-HC" (
  IF NOT DEFINED MPCHC_LITE (
    IF NOT EXIST "%PCKG_NAME%\Lang" MD "%PCKG_NAME%\Lang"
  )
  IF /I "%ARCH%" == "x64" (
    COPY /Y /V "%~1_%ARCH%\mpc-hc64.exe" "%PCKG_NAME%\mpc-hc64.exe" >NUL
  ) ELSE (
    COPY /Y /V "%~1_%ARCH%\mpc-hc.exe"   "%PCKG_NAME%\mpc-hc.exe" >NUL
  )
  COPY /Y /V "%~1_%ARCH%\mpciconlib.dll"             "%PCKG_NAME%\*.dll" >NUL
  IF NOT DEFINED MPCHC_LITE (
    COPY /Y /V "%~1_%ARCH%\Lang\mpcresources.??.dll" "%PCKG_NAME%\Lang\mpcresources.??.dll" >NUL
  )
  COPY /Y /V "%~1_%ARCH%\D3DCompiler_43.dll"         "%PCKG_NAME%\D3DCompiler_43.dll" >NUL
  COPY /Y /V "%~1_%ARCH%\d3dx9_43.dll"               "%PCKG_NAME%\d3dx9_43.dll" >NUL
) ELSE (
  COPY /Y /V "%~1_%ARCH%\*.ax"           "%PCKG_NAME%\*.ax" >NUL
  COPY /Y /V "%~1_%ARCH%\VSFilter.dll"   "%PCKG_NAME%\VSFilter.dll" >NUL
)

COPY /Y /V "..\COPYING.txt"         "%PCKG_NAME%" >NUL
COPY /Y /V "..\docs\Authors.txt"    "%PCKG_NAME%" >NUL
COPY /Y /V "..\docs\Changelog.txt"  "%PCKG_NAME%" >NUL
COPY /Y /V "..\docs\Readme.txt"      %PCKG_NAME%" >NUL

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
FOR /F "tokens=3,4 delims= " %%G IN (
  'FINDSTR /I /L /C:"define MPC_VERSION_MAJOR" "include\Version.h"') DO (SET "VerMajor=%%G")
FOR /F "tokens=3,4 delims= " %%G IN (
  'FINDSTR /I /L /C:"define MPC_VERSION_MINOR" "include\Version.h"') DO (SET "VerMinor=%%G")
FOR /F "tokens=3,4 delims= " %%G IN (
  'FINDSTR /I /L /C:"define MPC_VERSION_PATCH" "include\Version.h"') DO (SET "VerPatch=%%G")
FOR /F "tokens=3,4 delims= " %%G IN (
  'FINDSTR /I /L /C:"define MPC_VERSION_REV" "include\Version_rev.h"') DO (SET "VerRev=%%G")

SET MPCHC_VER=%VerMajor%.%VerMinor%.%VerPatch%.%VerRev%
EXIT /B


:SubDetectWinArch
IF DEFINED PROGRAMFILES(x86) (SET x64_type=amd64) ELSE (SET x64_type=x86_amd64)
EXIT /B


:SubDetectInnoSetup
REM Detect if we are running on 64bit WIN and use Wow6432Node, and set the path
REM of Inno Setup accordingly since Inno Setup is a 32-bit application
IF /I "%x64_type%" == "amd64" (
  SET "U_=HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
) ELSE (
  SET "U_=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"
)

FOR /F "delims=" %%G IN (
  'REG QUERY "%U_%\Inno Setup 5_is1" /v "Inno Setup: App Path" 2^>Nul ^|FIND "REG_SZ"') DO (
  SET "InnoSetupPath=%%G" & CALL :SubInnoSetupPath %%InnoSetupPath:*Z=%%)
EXIT /B


:SubDetectSevenzipPath
IF EXIST "%PROGRAMFILES%\7za.exe" (SET "SEVENZIP=%PROGRAMFILES%\7za.exe" & EXIT /B)
IF EXIST "7za.exe"                (SET "SEVENZIP=7za.exe" & EXIT /B)

FOR %%G IN (7z.exe)  DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

FOR %%G IN (7za.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

IF /I "%x64_type%" == "amd64" (
  FOR /F "delims=" %%G IN (
    'REG QUERY "HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\7-Zip" /v "Path" 2^>Nul ^| FIND "REG_SZ"') DO (
    SET "SEVENZIP_REG=%%G" & CALL :SubSevenzipPath %%SEVENZIP_REG:*REG_SZ=%%
  )
)

FOR /F "delims=" %%G IN (
  'REG QUERY "HKEY_LOCAL_MACHINE\SOFTWARE\7-Zip" /v "Path" 2^>Nul ^| FIND "REG_SZ"') DO (
  SET "SEVENZIP_REG=%%G" & CALL :SubSevenzipPath %%SEVENZIP_REG:*REG_SZ=%%
)
EXIT /B


:ShowHelp
TITLE %~nx0 Help
ECHO.
ECHO Usage:
ECHO %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|Both] [Main^|Resources^|MPCHC^|Filters^|All] [Debug^|Release] [Lite] [Packages^|Installer^|Zip] [FFmpeg]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        Debug only applies to mpc-hc.sln.
ECHO        The arguments are not case sensitive and can be ommitted.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments will use the default ones:
ECHO "%~nx0 Build Both MPCHC Release"
ECHO. & ECHO.
ECHO Examples:
ECHO %~nx0 x86 Resources -Builds the x86 resources
ECHO %~nx0 Resources     -Builds both x86 and x64 resources
ECHO %~nx0 x86           -Builds x86 Main exe and the x86 resources
ECHO %~nx0 x86 Debug     -Builds x86 Main Debug exe and x86 resources
ECHO %~nx0 x86 Filters   -Builds x86 Filters
ECHO %~nx0 x86 All       -Builds x86 Main exe, x86 Filters and the x86 resources
ECHO %~nx0 x86 Packages  -Builds x86 Main exe, x86 resources and creates the installer and the .7z package
ECHO %~nx0 x64 7z        -Rebuilds FFmpeg, builds x64 Main exe, x64 resources and creates the .7z package
ECHO.
ENDLOCAL
EXIT /B


:MissingVar
COLOR 0C
TITLE Compiling MPC-HC [ERROR]
ECHO Not all build dependencies were found.
ECHO.
ECHO See "docs\Compilation.txt" for more information.
ECHO. & ECHO.
ECHO Press any key to exit...
PAUSE >NUL
ENDLOCAL
EXIT /B


:UnsupportedSwitch
ECHO.
ECHO Unsupported commandline switch!
ECHO.
ECHO "%~nx0 %*"
ECHO.
ECHO Run "%~nx0 help" for details about the commandline switches.
CALL :SubMsg "ERROR" "Compilation failed!" & EXIT /B


:SubInnoSetupPath
SET "InnoSetupPath=%*\ISCC.exe"
EXIT /B


:SubSevenzipPath
SET "SEVENZIP=%*\7z.exe"
EXIT /B


:SubMsg
ECHO. & ECHO ------------------------------
IF /I "%~1" == "ERROR" (
  CALL :SubColorText "0C" "[%~1]" & ECHO  %~2
) ELSE IF /I "%~1" == "INFO" (
  CALL :SubColorText "0A" "[%~1]" & ECHO  %~2
) ELSE IF /I "%~1" == "WARNING" (
  CALL :SubColorText "0E" "[%~1]" & ECHO  %~2
)
ECHO ------------------------------ & ECHO.
IF /I "%~1" == "ERROR" (
  ECHO Press any key to exit...
  PAUSE >NUL
  EXIT /B 1
) ELSE (
  EXIT /B 0
)


:SubColorText
FOR /F "tokens=1,2 delims=#" %%G IN (
  '"PROMPT #$H#$E# & ECHO ON & FOR %%H IN (1) DO REM"') DO (
  SET "DEL=%%G")
<NUL SET /p ".=%DEL%" > "%~2"
FINDSTR /v /a:%1 /R ".18" "%~2" NUL
DEL "%~2" > NUL 2>&1
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
