@SubWCRev .\ include\SubWCRev.conf include\version.h
@SubWCRev .\ src\apps\mplayerc\res\mpc-hc.exe.manifest.conf src\apps\mplayerc\res\mpc-hc.exe.manifest
@if %ERRORLEVEL% NEQ 0 goto :NoSubWCRev
@goto :eof

:NoSubWCRev
@echo NoSubWCRev
echo #pragma once > include\version.h
echo. >> include\Version.h
echo #define DO_MAKE_STR(x) #x >> include\Version.h
echo #define MAKE_STR(x) DO_MAKE_STR(x) >> include\Version.h
echo. >> include\Version.h
@echo #define VERSION_MAJOR 1 >> include\version.h
@echo #define VERSION_MINOR 3 >> include\version.h
@echo #define VERSION_REV 0 >> include\version.h
@echo #define VERSION_PATCH 0 >> include\version.h

@copy src\apps\mplayerc\res\mpc-hc.exe.manifest.template src\apps\mplayerc\res\mpc-hc.exe.manifest /Y