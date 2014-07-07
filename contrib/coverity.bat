@ECHO OFF
REM (C) 2013-2014 see Authors.txt
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

IF NOT DEFINED COVDIR SET "COVDIR=H:\progs\thirdparty\cov-analysis-win64-7.5.0"
IF DEFINED COVDIR IF NOT EXIST "%COVDIR%" (
  ECHO.
  ECHO ERROR: Coverity not found in "%COVDIR%"
  GOTO End
)


CALL "%VS120COMNTOOLS%\vsvars32.bat"
IF %ERRORLEVEL% NEQ 0 (
  ECHO vsvars32.bat call failed.
  GOTO End
)


:Cleanup
IF EXIST "cov-int" RD /q /s "cov-int"
IF EXIST "MPC-HC.lzma" DEL "MPC-HC.lzma"
IF EXIST "MPC-HC.tar"  DEL "MPC-HC.tar"
IF EXIST "MPC-HC.tgz"  DEL "MPC-HC.tgz"


:Main
CALL "..\build.bat" clean Lite Both Main Release silent
CALL "..\build.bat" clean Filters Both Release silent
CALL "..\build.bat" clean IconLib Both Release silent
CALL "..\build.bat" clean Api Both Release silent
"%COVDIR%\bin\cov-build.exe" --dir cov-int "..\build.bat" Build Lite Both Main Release silent
"%COVDIR%\bin\cov-build.exe" --dir cov-int "..\build.bat" Build Filters Both Release silent
"%COVDIR%\bin\cov-build.exe" --dir cov-int "..\build.bat" Build IconLib Both Release silent
"%COVDIR%\bin\cov-build.exe" --dir cov-int "..\build.bat" Build Api Both Release silent


:tar
tar --version 1>&2 2>NUL || (ECHO. & ECHO ERROR: tar not found & GOTO SevenZip)
tar caf "MPC-HC.lzma" "cov-int"
GOTO End


:SevenZip
CALL :SubDetectSevenzipPath

rem Coverity is totally bogus with lzma...
rem And since I cannot replicate the arguments with 7-Zip, just use tar/gzip.
IF EXIST "%SEVENZIP%" (
  "%SEVENZIP%" a -ttar "MPC-HC.tar" "cov-int"
  "%SEVENZIP%" a -tgzip "MPC-HC.tgz" "MPC-HC.tar"
  IF EXIST "MPC-HC.tar" DEL "MPC-HC.tar"
  GOTO End
)


:SubDetectSevenzipPath
FOR %%G IN (7z.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

FOR %%G IN (7za.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

FOR /F "tokens=2*" %%A IN (
  'REG QUERY "HKLM\SOFTWARE\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ" ^|^|
   REG QUERY "HKLM\SOFTWARE\Wow6432Node\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ"') DO SET "SEVENZIP=%%B\7z.exe"
EXIT /B


:End
POPD
ECHO. & ECHO Press any key to close this window...
PAUSE >NUL
ENDLOCAL
EXIT /B
