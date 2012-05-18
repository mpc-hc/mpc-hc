@echo off

rem This is a simple script to check out the head revision of mplayerc.rc,
rem then rename it to old file for rcfile.pl to process it

echo Getting the latest mplayerc.rc from repository...
svn cat -r head ../mplayerc.rc > $$TEMP$$.old
if %ERRORLEVEL% neq 0 goto NOSVNCLI

echo Generating new rc files and string files...
perl rcfile.pl -b $$TEMP$$.old
goto END

:NOSVNCLI
echo You'll need svn command line tool to use this script.
echo Or you can just checkout the head revision of mplayerc.rc file by yourself,
echo put it somewhere and then use the -b option to point to it.

:END
del $$TEMP$$.old
pause
