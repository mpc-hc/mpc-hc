@ECHO OFF
SETLOCAL
SubWCRev .\ include\Version_rev.h.in include\Version_rev.h -f
IF %ERRORLEVEL% NEQ 0 GOTO NoSubWCRev

SubWCRev .\ src\apps\mplayerc\res\mpc-hc.exe.manifest.conf src\apps\mplayerc\res\mpc-hc.exe.manifest -f
IF %ERRORLEVEL% NEQ 0 GOTO NoSubWCRev

EXIT /B

:NoSubWCRev
ECHO NoSubWCRev, will use MPC_VERSION_REV=0
ECHO #define MPC_VERSION_REV 0 >include\Version_rev.h

COPY /Y src\apps\mplayerc\res\mpc-hc.exe.manifest.template src\apps\mplayerc\res\mpc-hc.exe.manifest

EXIT /B
