@echo off
setlocal

rem A simple script which demonstrates how to apply translated text to locale rc file.
rem This is only an example.

call "common.bat" perl
if %errorlevel% neq 0 goto end

for %%i in (*.rc) do (
  echo Patching file %%i
  perl.exe patch.pl -i text\%%i.txt %%i
  echo ----------------------
)

:end
pause
endlocal
exit /b
