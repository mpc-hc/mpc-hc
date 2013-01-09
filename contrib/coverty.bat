@ECHO OFF

SET COVDIR=H:\progs\thirdparty\cov-analysis-win64-6.0.5

CALL "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86

"%COVDIR%\bin\cov-build.exe" --dir cov-int mpc-hc-covbuild.bat
"%PROGRAMFILES%\7za.exe" a -ttar MPC-HC.tar cov-int
"%PROGRAMFILES%\7za.exe" a -tgzip MPC-HC.tgz MPC-HC.tar
IF EXIST "MPC-HC.tar" DEL "MPC-HC.tar"
