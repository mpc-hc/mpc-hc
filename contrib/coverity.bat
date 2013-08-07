@ECHO OFF
REM (C) 2013 see Authors.txt
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

SET COVDIR=H:\progs\thirdparty\cov-analysis-win64-6.5.1

CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86 || CALL "%VS110COMNTOOLS%..\..\VC\vcvarsall.bat" x86

SET MSBUILD_SWITCHES=/nologo /consoleloggerparameters:Verbosity=minimal /maxcpucount^
 /nodeReuse:true /target:Rebuild /property:Configuration="Release";Platform=Win32

"%COVDIR%\bin\cov-build.exe" --dir cov-int MSBuild "..\mpc-hc.sln" %MSBUILD_SWITCHES%

IF EXIST "MPC-HC.tar" DEL "MPC-HC.tar"
IF EXIST "MPC-HC.tgz" DEL "MPC-HC.tgz"


:tar
tar --version 1>&2 2>NUL || (ECHO. & ECHO ERROR: tar not found & GOTO SevenZip)
tar czvf MPC-HC.tgz cov-int
GOTO End


:SevenZip
IF NOT EXIST "%PROGRAMFILES%\7za.exe" (ECHO. & ECHO ERROR: "%PROGRAMFILES%\7za.exe" not found & GOTO End)
"%PROGRAMFILES%\7za.exe" a -ttar MPC-HC.tar cov-int
"%PROGRAMFILES%\7za.exe" a -tgzip MPC-HC.tgz MPC-HC.tar
IF EXIST "MPC-HC.tar" DEL "MPC-HC.tar"


:End
POPD
ECHO. & ECHO Press any key to close this window...
PAUSE >NUL
ENDLOCAL
EXIT /B
