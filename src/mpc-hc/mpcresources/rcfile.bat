@echo off
setlocal

rem This is a simple script to check out the head revision of mplayerc.rc,
rem then rename it to old file for rcfile.pl to process it

call "common.bat"
if %errorlevel% neq 0 goto end

echo Getting the latest mplayerc.rc from repository...
git.exe show HEAD:../mplayerc.rc > $$TEMP$$.old

echo Generating new rc files and string files...
perl.exe rcfile.pl -b $$TEMP$$.old

del $$TEMP$$.old

:end
pause
endlocal
exit /b
