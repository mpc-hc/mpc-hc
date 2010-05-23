@if /i "%3"=="Resource" goto skipMain
%BUILD_APP% ../../../mpc-hc.sln %BUILDTYPE% "%BUILDCONFIG% Unicode|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError


:skipMain
@if /i "%3"=="Main" goto skipResource
%BUILD_APP% mpciconlib.sln %BUILDTYPE% "Release Unicode|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError

%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Belarusian|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Catalan|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Chinese simplified|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Chinese traditional|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Czech|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Dutch|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release French|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release German|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Hungarian|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Italian|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Korean|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Polish|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Portuguese|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Russian|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Slovak|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Spanish|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Swedish|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Turkish|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError
%BUILD_APP% mpcresources.sln %BUILDTYPE% "Release Ukrainian|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto EndWithError


:skipResource
@if /i "%1" == "clean" goto skipCopy

@if not exist %COPY_TO_DIR% mkdir %COPY_TO_DIR%

@if /i "%3" == "Resource" goto skipMainCopy
xcopy "%BUILDCONFIG% Unicode\*.exe" ".\%COPY_TO_DIR%\" /y
@echo vc90.pdb > ExcludeList.txt
xcopy "%BUILDCONFIG% Unicode\*.pdb" ".\%COPY_TO_DIR%\" /y /exclude:ExcludeList.txt
@del ExcludeList.txt /q

:skipMainCopy
@if /i "%3" == "Main" goto skipResourceCopy

xcopy "Release Unicode\*.dll" ".\%COPY_TO_DIR%\" /y

:skipResourceCopy
xcopy AUTHORS ".\%COPY_TO_DIR%\" /y
xcopy ChangeLog ".\%COPY_TO_DIR%\" /y
xcopy ..\..\..\COPYING ".\%COPY_TO_DIR%\" /y

:skipCopy
:EndWithError
