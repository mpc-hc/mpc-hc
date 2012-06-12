@ECHO OFF

PUSHD %~dp0

IF EXIST "..\build.user.bat" (
  CALL "..\build.user.bat"
) ELSE (
  IF DEFINED MSYS SET MPCHC_MSYS=%MSYS%
)
SET PATH=%PATH%;%MPCHC_MSYS%\bin

POPD
yasm.exe %*
