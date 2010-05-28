@if /i "%3"=="Resource" goto skipMain
%BUILD_APP% ../../../mpc-hc.sln %BUILDTYPE% "%BUILDCONFIG%|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError


:skipMain
@if /i "%3"=="Main" goto skipResource
%BUILD_APP% mpciconlib.sln %BUILDTYPE% "Release|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError

FOR %%A IN ("Belarusian" "Catalan" "Chinese simplified" "Chinese traditional" 
"Czech" "Dutch" "French" "German" "Hungarian" "Italian" "Korean" "Polish" 
"Portuguese" "Russian" "Slovak" "Spanish" "Swedish" "Turkish" "Ukrainian"
) DO (
CALL :SubMPCRES %%A
)


:skipResource
@if /i "%1" == "clean" goto skipCopy

@if not exist %COPY_TO_DIR% mkdir %COPY_TO_DIR%

@if /i "%3" == "Resource" goto skipMainCopy

:skipMainCopy
@if /i "%3" == "Main" goto skipResourceCopy

:skipResourceCopy
rem xcopy AUTHORS ".\%COPY_TO_DIR%\" /y
rem xcopy ChangeLog ".\%COPY_TO_DIR%\" /y
rem xcopy ..\..\..\COPYING ".\%COPY_TO_DIR%\" /y

:skipCopy
:EndWithError

:SubMPCRES
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release %~1|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
GOTO :EOF