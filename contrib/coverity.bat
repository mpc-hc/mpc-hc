@ECHO OFF
SETLOCAL

PUSHD %~dp0

SET COVDIR=H:\progs\thirdparty\cov-analysis-win64-6.5.1

CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86

SET MSBUILD_SWITCHES=/nologo /consoleloggerparameters:Verbosity=minimal /maxcpucount^
 /nodeReuse:true /target:Rebuild /property:Configuration="Release";Platform=Win32

"%COVDIR%\bin\cov-build.exe" --dir cov-int MSBuild "..\mpc-hc.sln" %MSBUILD_SWITCHES%

IF EXIST "MPC-HC.tar" DEL "MPC-HC.tar"
IF EXIST "MPC-HC.tgz" DEL "MPC-HC.tgz"
"%PROGRAMFILES%\7za.exe" a -ttar MPC-HC.tar cov-int
"%PROGRAMFILES%\7za.exe" a -tgzip MPC-HC.tgz MPC-HC.tar
IF EXIST "MPC-HC.tar" DEL "MPC-HC.tar"

ENDLOCAL
PAUSE
