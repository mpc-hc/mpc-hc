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
SET "FILE_DIR=%~dp0"
PUSHD "%FILE_DIR%"

SET "COMMON=%FILE_DIR%..\common.bat"

IF EXIST "..\build.user.bat" CALL "..\build.user.bat"

IF NOT EXIST "%COV_PATH%" (CALL "%COMMON%" :SubMsg "ERROR" "Coverity not found in '%COV_PATH%'" & EXIT /B)


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
IF %ERRORLEVEL% NEQ 0 (CALL "%COMMON%" :SubMsg "ERROR" "Build failed." & EXIT /B)


:tar
CALL "%COMMON%" :SubDetectTar
IF NOT EXIST "%TAR%" (CALL "%COMMON%" :SubMsg "WARNING" "tar not found. Trying 7-zip..." & GOTO SevenZip)

SET "FILE_NAME=MPC-HC.tar.xz"
SET "XZ_OPT=-9e"
REM You can add -T{N} where {N} stands for count of threads to use. 0 will use all available threads.
REM Pay attention to memory usage, -9eT4 uses over 5GB of RAM, but -9eT1 uses 700MB in my test.
REM Lowering compression preset will also decrease memory usage significantly.

%TAR% cJf %FILE_NAME% cov-int
IF %ERRORLEVEL% NEQ 0 (
  REM Fallback for 32-bit version of xz.
  CALL "%COMMON%" :SubMsg "WARNING" "Fallback for 32-bit xz. Trying again with 'XZ_OPT=-7e'..."
  SET "XZ_OPT=-7e"
  %TAR% cJf %FILE_NAME% cov-int
)

IF %ERRORLEVEL% NEQ 0 (CALL "%COMMON%" :SubMsg "WARNING" "tar failed. Trying 7-zip..." & GOTO SevenZip)
GOTO Upload


:SevenZip
CALL "%COMMON%" :SubDetectSevenzipPath
IF NOT EXIST "%SEVENZIP%" (CALL "%COMMON%" :SubMsg "ERROR" "7-zip not found." & EXIT /B)

SET "FILE_NAME=MPC-HC.tgz"
REM 7-Zip doesn't support tarball compliant LZMA2 archives, just use tar/gzip.
"%SEVENZIP%" a -ttar "MPC-HC.tar" "cov-int"
IF %ERRORLEVEL% NEQ 0 (CALL "%COMMON%" :SubMsg "ERROR" "7-zip failed." & EXIT /B)
"%SEVENZIP%" a -tgzip "%FILE_NAME%" "MPC-HC.tar"
IF %ERRORLEVEL% NEQ 0 (CALL "%COMMON%" :SubMsg "ERROR" "7-zip failed." & EXIT /B)
IF EXIST "MPC-HC.tar" DEL "MPC-HC.tar"
GOTO Upload


:Upload
CALL "..\build.bat" GetVersion
CALL "%COMMON%" :SubDetectCurl
IF NOT EXIST "%CURL%" (CALL "%COMMON%" :SubMsg "WARNING" "curl not found. Upload aborted." & GOTO End)
IF NOT DEFINED COV_TOKEN (CALL "%COMMON%" :SubMsg "WARNING" "COV_TOKEN not defined. Upload aborted." & GOTO End)
IF NOT DEFINED COV_EMAIL (CALL "%COMMON%" :SubMsg "WARNING" "COV_EMAIL not defined. Upload aborted." & GOTO End)
%CURL% --form token=%COV_TOKEN% --form email=%COV_EMAIL% --form file=@%FILE_NAME% --form version=%MPCHC_HASH% https://scan.coverity.com/builds?project=MPC-HC -o cov_upload.log
GOTO End


:End
POPD
CALL "%COMMON%" :SubMsg "INFO" "Done. Press any key to exit..."
PAUSE >NUL
ENDLOCAL
EXIT /B
