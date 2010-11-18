@ECHO OFF
SubWCRev .\ include\SubWCRev.h include\Version_rev.h
SubWCRev .\ src\apps\mplayerc\res\mpc-hc.exe.manifest.conf src\apps\mplayerc\res\mpc-hc.exe.manifest
IF %ERRORLEVEL% NEQ 0 GOTO :NoSubWCRev
GOTO :EOF

:NoSubWCRev
ECHO:NoSubWCRev, will use VERSION_REV=0
ECHO:#define VERSION_REV 0 >include\Version_rev.h
COPY /Y src\apps\mplayerc\res\mpc-hc.exe.manifest.template src\apps\mplayerc\res\mpc-hc.exe.manifest
