@ECHO OFF
SubWCRev .\ include\SubWCRev.conf include\Version.h
SubWCRev .\ src\apps\mplayerc\res\mpc-hc.exe.manifest.conf src\apps\mplayerc\res\mpc-hc.exe.manifest
IF %ERRORLEVEL% NEQ 0 GOTO :NoSubWCRev
GOTO :EOF

:NoSubWCRev
ECHO:NoSubWCRev
ECHO:#pragma once > include\Version.h
ECHO.>> include\Version.h
ECHO:#define DO_MAKE_STR(x) #x >> include\Version.h
ECHO:#define MAKE_STR(x) DO_MAKE_STR(x) >> include\Version.h
ECHO.>> include\Version.h
ECHO:#define VERSION_MAJOR 1 >> include\Version.h
ECHO:#define VERSION_MINOR 3 >> include\Version.h
ECHO:#define VERSION_REV 0 >> include\Version.h
ECHO:#define VERSION_PATCH 0 >> include\Version.h

COPY src\apps\mplayerc\res\mpc-hc.exe.manifest.template src\apps\mplayerc\res\mpc-hc.exe.manifest /Y