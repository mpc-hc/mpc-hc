@ECHO OFF
SubWCRev .\ include\SubWCRev.h include\Version_rev.h
SubWCRev .\ src\apps\mplayerc\res\mpc-hc.exe.manifest.conf src\apps\mplayerc\res\mpc-hc.exe.manifest
IF %ERRORLEVEL% NEQ 0 GOTO :NoSubWCRev
GOTO :EOF

:NoSubWCRev
ECHO:NoSubWCRev
ECHO:#define VERSION_REV 0 > include\Version_rev.h
COPY /Y src\apps\mplayerc\res\mpc-hc.exe.manifest.template src\apps\mplayerc\res\mpc-hc.exe.manifest
