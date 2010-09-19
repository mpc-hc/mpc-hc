@echo off

rem This is a simple script to checkout head revision mplayerc.rc file,
rem then rename it to old file for rcfile.pl to process

echo Get the latest mplayerc.rc from repository...
svn cat -r head ../mplayerc.rc > $$TEMP$$.old
if %ERRORLEVEL% neq 0 GOTO :NOSVNCLI

echo Generating new rc files and string files...
perl rcfile.pl -b $$TEMP$$.old
goto :END

:NOSVNCLI
echo You'll need svn command line tools to use this script.
echo Or you can just checkout the head revision of mplayerc.rc file by yourself,
echo put it somewhere and then use the -b option to point to it.

:END
del $$TEMP$$.old
pause