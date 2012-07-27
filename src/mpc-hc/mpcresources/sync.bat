@echo off
setlocal

rem An all in one script which demonstrates how to sync all locale rc files to the latest mplayerc.rc.
rem It will try to patch existing local rc files first, then sync them to mplayerc.rc.
rem Then it will overwrite rc files with new rc ones, and after that it will generate the text files.
rem This is only an example.

call "common.bat"
if %errorlevel% neq 0 goto end

echo Get the latest mplayerc.rc from repository first...
git.exe show HEAD:../mplayerc.rc > $$TEMP$$.old
echo ----------------------

for %%i in (*.rc) do (
  echo Patching file %%i
  perl.exe patch.pl -i text\%%i.txt %%i
  echo ----------------------
)
echo ----------------------

echo Generating new rc files...
perl.exe rcfile.pl -b $$TEMP$$.old
del $$TEMP$$.old
echo ----------------------

copy newrc\*.rc .
echo ----------------------

echo Generating new string files...
copy ..\mplayerc.rc .
perl.exe rcstrings.pl -a
del mplayerc.rc
echo ----------------------

:end
pause
endlocal
exit /b
