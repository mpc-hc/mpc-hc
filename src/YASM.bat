@ECHO OFF

PUSHD %~dp0
IF EXIST "..\build.user.bat" (
  CALL "..\build.user.bat"
) ELSE (
  IF DEFINED MSYS SET MPCHC_MSYS=%MSYS%
)
POPD
SET PATH=%PATH%;%MPCHC_MSYS%\bin

yasm.exe %*
