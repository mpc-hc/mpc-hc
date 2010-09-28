@echo off

rem A simple script demos how to extract all strings from rc files.

echo Generating string files...
perl rcstrings.pl -a
pause