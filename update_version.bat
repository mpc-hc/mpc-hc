@if not exist "%programfiles%\TortoiseSVN\bin\SubWCRev.exe" goto :x64

@"%programfiles%\TortoiseSVN\bin\SubWCRev.exe" .\ include\SubWCRev.conf include\version.h
@if %ERRORLEVEL%==6 goto :NoSubWCRev
@if %ERRORLEVEL%==10 goto :NoSubWCRev
@goto :eof

:x64
@if not exist "%ProgramW6432%\TortoiseSVN\bin\SubWCRev.exe" goto :NoSubWCRev

@"%ProgramW6432%\TortoiseSVN\bin\SubWCRev.exe" .\ include\SubWCRev.conf include\version.h
@if %ERRORLEVEL%==6 goto :NoSubWCRev
@if %ERRORLEVEL%==10 goto :NoSubWCRev
@goto :eof

:NoSubWCRev
@echo NoSubWCRev
@if exist include\version.h del include\version.h
echo #pragma once >> include\version.h
echo. >> include\Version.h
echo #define DO_MAKE_STR(x) #x >> include\Version.h
echo #define MAKE_STR(x) DO_MAKE_STR(x) >> include\Version.h
echo. >> include\Version.h
@echo #define VERSION_MAJOR 1 >> include\version.h
@echo #define VERSION_MINOR 3 >> include\version.h
@echo #define VERSION_REV 0 >> include\version.h
@echo #define VERSION_PATCH 0 >> include\version.h
