@ECHO OFF

IF EXIST "..\..\..\build.user.bat" (
  CALL "..\..\..\build.user.bat"
) ELSE (
  IF /I NOT "%1"=="perl" (
    IF DEFINED GIT (SET MPCHC_GIT=%GIT%)
  )
  IF DEFINED PERL (SET MPCHC_PERL=%PERL%) ELSE (GOTO MissingVar)
)

SET PATH=%MPCHC_PERL%\bin;%MPCHC_GIT%\cmd;%PATH%
FOR %%X IN (git.exe)  DO (SET FOUND=%%~$PATH:X)
FOR %%X IN (perl.exe) DO (SET FOUND=%%~$PATH:X)
IF NOT DEFINED FOUND GOTO MissingVar
EXIT /B

:MissingVar
ECHO Not all build dependencies were found.
ECHO.
ECHO See "docs\Compilation.txt" for more information.
ENDLOCAL
EXIT /B 1
