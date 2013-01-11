@ECHO OFF
SETLOCAL

PUSHD %~dp0

SET COVDIR=H:\progs\thirdparty\cov-analysis-win64-6.5.1

CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86

"%COVDIR%\bin\cov-build.exe" --dir cov-int devenv "..\mpc-hc_vs2012.sln" /Rebuild "Release|Win32"
"%PROGRAMFILES%\7za.exe" a -ttar MPC-HC.tar cov-int
"%PROGRAMFILES%\7za.exe" a -tgzip MPC-HC.tgz MPC-HC.tar
IF EXIST "MPC-HC.tar" DEL "MPC-HC.tar"

ENDLOCAL
PAUSE
