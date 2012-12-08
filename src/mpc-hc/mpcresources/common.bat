@ECHO OFF

SET ROOT_DIR=..\..\..

IF EXIST "%ROOT_DIR%\build.user.bat" (
  CALL "%ROOT_DIR%\build.user.bat"
) ELSE (
  IF /I NOT "%1"=="perl" (
    IF DEFINED GIT (SET MPCHC_GIT=%GIT%)
  )
  IF DEFINED PERL (SET MPCHC_PERL=%PERL%)
)

SET PATH=%MPCHC_PERL%\bin;%MPCHC_GIT%\cmd;%PATH%
FOR %%G IN (git.exe)  DO (SET GIT_PATH=%%~$PATH:G)
FOR %%G IN (perl.exe) DO (SET PERL_PATH=%%~$PATH:G)
IF NOT DEFINED GIT_PATH  GOTO MissingVar
IF NOT DEFINED PERL_PATH GOTO MissingVar

ECHO Backing up current translation files...
IF NOT EXIST backup MD backup
COPY /Y /V ..\mplayerc.rc backup
COPY /Y /V *.rc backup
IF NOT EXIST backup\text MD backup\text
COPY /Y /V text backup\text
ECHO ----------------------

EXIT /B


:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "%ROOT_DIR%\docs\Compilation.txt" for more information.
ENDLOCAL
EXIT /B 1
