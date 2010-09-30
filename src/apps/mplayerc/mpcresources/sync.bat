@echo off

rem An all in one script which demonstrates how to sync all locale rc files to the latest mplayerc.rc.
rem It will try to patch existing local rc files first, then sync them to mplayerc.rc.
rem Then it will overwrite rc files with new rc ones, and after that it will generate the text files.
rem This is only an example.

for %%i in (*.rc) do perl patch.pl -i text\%%i.txt %%i & echo Patching file %%i & echo ----------------------
echo ----------------------

echo Get the latest mplayerc.rc from repository...
svn cat -r head ../mplayerc.rc > $$TEMP$$.old
if %ERRORLEVEL% neq 0 GOTO :NOSVNCLI
echo ----------------------

echo Generating new rc files and string files...
perl rcfile.pl -b $$TEMP$$.old
goto :CONTINUE

:NOSVNCLI
echo You'll need svn command line tool to use this script.
echo Or you can just checkout the head revision of mplayerc.rc file by yourself,
echo put it somewhere and then use the -b option to point to it.

:CONTINUE
del $$TEMP$$.old
echo ----------------------

copy newrc\*.rc .
echo ----------------------

echo Generating string files...
perl rcstrings.pl -a
echo ----------------------

:END
pause
