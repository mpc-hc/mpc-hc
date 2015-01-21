@ECHO OFF
REM (C) 2013-2015 see Authors.txt
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

PUSHD %~dp0

IF EXIST "..\build.user.bat" CALL "..\build.user.bat"

IF NOT EXIST "%COV_PATH%" (CALL :SubMsg "ERROR" "Coverity not found in '%COV_PATH%'" & EXIT /B)


:Cleanup
IF EXIST "cov-int" RD /q /s "cov-int"
IF EXIST "MPC-HC.tar.xz"  DEL "MPC-HC.tar.xz"
IF EXIST "MPC-HC.tar"     DEL "MPC-HC.tar"
IF EXIST "MPC-HC.tgz"     DEL "MPC-HC.tgz"
IF EXIST "cov_upload.log" DEL "cov_upload.log"


:Main
CALL "..\build.bat" clean Lite Both Main Release silent
CALL "..\build.bat" clean Filters Both Release silent
CALL "..\build.bat" clean IconLib Both Release silent
CALL "..\build.bat" clean Api Both Release silent
"%COV_PATH%\bin\cov-build.exe" --dir cov-int "..\build.bat" Build Lite Both Main Release silent
"%COV_PATH%\bin\cov-build.exe" --dir cov-int "..\build.bat" Build Filters Both Release silent
"%COV_PATH%\bin\cov-build.exe" --dir cov-int "..\build.bat" Build IconLib Both Release silent
"%COV_PATH%\bin\cov-build.exe" --dir cov-int "..\build.bat" Build Api Both Release silent
IF %ERRORLEVEL% NEQ 0 (CALL :SubMsg "ERROR" "Build failed." & EXIT /B)


:tar
CALL :SubDetectTar
IF NOT EXIST "%TAR%" (CALL :SubMsg "WARNING" "tar not found. Trying 7-zip..." & GOTO SevenZip)
SET "XZ_OPT=-9e"
REM You can add -T{N} where {N} stands for count of threads to use. 0 will use all available threads.
REM Pay attention to memory usage, -9eT4 uses over 5GB of RAM, but -9eT1 uses 700MB in my test.
REM Lowering compression preset will also decrease memory usage significantly.

%TAR% cJf MPC-HC.tar.xz cov-int
IF %ERRORLEVEL% NEQ 0 (
  REM Fallback for 32-bit version of xz.
  SET "XZ_OPT=-7e"
  %TAR% cJf MPC-HC.tar.xz cov-int
)

IF %ERRORLEVEL% NEQ 0 (CALL :SubMsg "WARNING" "tar failed. Trying 7-zip..." & GOTO SevenZip)
GOTO Upload


:SevenZip
CALL :SubDetectSevenzipPath
IF NOT EXIST "%SEVENZIP%" (CALL :SubMsg "ERROR" "7-zip not found." & EXIT /B)
REM 7-Zip doesn't support tarball compliant LZMA2 archives, just use tar/gzip.
"%SEVENZIP%" a -ttar "MPC-HC.tar" "cov-int"
IF %ERRORLEVEL% NEQ 0 (CALL :SubMsg "ERROR" "7-zip failed." & EXIT /B)
"%SEVENZIP%" a -tgzip "MPC-HC.tgz" "MPC-HC.tar"
IF %ERRORLEVEL% NEQ 0 (CALL :SubMsg "ERROR" "7-zip failed." & EXIT /B)
IF EXIST "MPC-HC.tar" DEL "MPC-HC.tar"
GOTO Upload


:Upload
CALL "..\build.bat" GetVersion
CALL :SubDetectCurl
IF NOT EXIST "%CURL%" (CALL :SubMsg "WARNING" "curl not found. Upload aborted." & GOTO End)
IF NOT DEFINED COV_TOKEN (CALL :SubMsg "WARNING" "COV_TOKEN not defined. Upload aborted." & GOTO End)
IF NOT DEFINED COV_EMAIL (CALL :SubMsg "WARNING" "COV_EMAIL not defined. Upload aborted." & GOTO End)
%CURL% --form token=%COV_TOKEN% --form email=%COV_EMAIL% --form file=@MPC-HC.tar.xz --form version=%MPCHC_HASH% https://scan.coverity.com/builds?project=MPC-HC -o cov_upload.log
GOTO End


:End
POPD
CALL :SubMsg "INFO" "Done. Press any key to exit..."
PAUSE >NUL
ENDLOCAL
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


:SubDetectCurl
IF EXIST curl.exe (SET "CURL=curl.exe" & EXIT /B)
IF EXIST "%CURL_PATH%\curl.exe" (SET "CURL=%CURL_PATH%\curl.exe" & EXIT /B)
FOR %%G IN (curl.exe) DO (SET "CURL_PATH=%%~$PATH:G")
IF EXIST "%CURL_PATH%" (SET "CURL=%CURL_PATH%" & EXIT /B)
EXIT /B


:SubDetectTar
IF EXIST tar.exe (SET "TAR=tar.exe" & EXIT /B)
IF EXIST "%TAR_PATH%\tar.exe" (SET "TAR=%TAR_PATH%\tar.exe" & EXIT /B)
FOR %%G IN (tar.exe) DO (SET "TAR_PATH=%%~$PATH:G")
IF EXIST "%TAR_PATH%" (SET "TAR=%TAR_PATH%" & EXIT /B)
EXIT /B


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
  ECHO Press any key to exit...
  PAUSE >NUL
  POPD
  ENDLOCAL
  EXIT /B 1
) ELSE (
  EXIT /B
)


:SubColorText
FOR /F "tokens=1,2 delims=#" %%G IN (
  '"PROMPT #$H#$E# & ECHO ON & FOR %%H IN (1) DO REM"') DO (
  SET "DEL=%%G")
<NUL SET /p ".=%DEL%" > "%~2"
FINDSTR /v /a:%1 /R ".18" "%~2" NUL
DEL "%~2" > NUL 2>&1
REM The space in the following ECHO is intentional
ECHO  %~3
EXIT /B
