@echo off
setlocal

rem A simple script which demonstrates how to extract all strings from rc files.

call "common.bat" perl
if %errorlevel% neq 0 goto end

echo Generating string files...
perl.exe rcstrings.pl -a

:end
pause
endlocal
exit /b
