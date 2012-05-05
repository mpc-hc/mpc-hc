@ECHO OFF
SETLOCAL

PUSHD %~dp0

IF EXIST "SubWCRev.exe" SET "SUBWCREV=SubWCRev.exe"
FOR %%A IN (SubWCRev.exe) DO (SET SUBWCREV=%%~$PATH:A)
IF NOT DEFINED SUBWCREV GOTO SubNoSubWCRev

"%SUBWCREV%" . "include\Version_rev.h.in" "include\Version_rev.h" -f
IF %ERRORLEVEL% NEQ 0 GOTO SubError

"%SUBWCREV%" . "src\apps\mplayerc\res\mpc-hc.exe.manifest.conf" "src\apps\mplayerc\res\mpc-hc.exe.manifest" -f >NUL
IF %ERRORLEVEL% NEQ 0 GOTO SubError

:END
POPD
ENDLOCAL
EXIT /B


:SubNoSubWCRev
ECHO. & ECHO SubWCRev, which is part of TortoiseSVN, wasn't found!
ECHO You should (re)install TortoiseSVN.
GOTO SubCommon

:SubError
ECHO Something went wrong when generating the revision number.

:SubCommon
ECHO I'll use MPC_VERSION_REV=0 for now.

ECHO #define MPC_VERSION_REV 0 > "include\Version_rev.h"
TYPE "src\apps\mplayerc\res\mpc-hc.exe.manifest.template" > "src\apps\mplayerc\res\mpc-hc.exe.manifest"
EXIT /B
