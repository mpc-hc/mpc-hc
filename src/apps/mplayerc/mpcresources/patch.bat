@echo off

rem A simple script which demonstrates how to apply translated text to locale rc file.
rem This is only an example.

for %%i in (*.rc) do (
  echo Patching file %%i
  perl patch.pl -i text\%%i.txt %%i
  echo ----------------------
)
echo ----------------------

pause
