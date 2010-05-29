@setlocal

@rem Notes
@rem Visual Studio 2008 Devenv Command Line Switches - http://msdn.microsoft.com/en-us/library/xee0c8y7.aspx

@rem Usage
@rem build_specific.bat [clean|build|rebuild] [null|x86|x64] [null|Main|Resource] [Debug]
@rem Executing "build_specific.bat" will cause it to use defaults "build_specific.bat build null null
@rem Example
@rem 'null' can be replaced with anything example 'all': "build_specific.bat build x86 all Debug"
@rem "build_specific.bat build x86 Resource" - Will build the x86 resources only
@rem "build_specific.bat build null Resource" - Will build both x86 and x64 resources only
@rem "build_specific.bat build x86" - Will build x86 Main exe and resources (*.dll)
@rem "build_specific.bat build x86 null Debug" - Will build x86 Main Debug exe and resources (*.dll)
@rem NOTE: Debug only applies to Main(mpc-hc.sln) project

@rem pre-build checks

@if "%VS90COMNTOOLS%" == "" goto :MissingEnv
@if "%MINGW32%" == "" goto :MissingEnv
@if "%MINGW64%" == "" goto :MissingEnv
@goto :NoEnvMissing

:MissingEnv
@echo "Not all build dependencies found. To build MPC-HC you need:"
@echo "* Visual Studio 2008 (SP1) installed"
@echo "* MinGW 32 bit build environment with coreMSYS pointed to in MINGW32 env var"
@echo "* MinGW 64 bit build environment with coreMSYS pointed to in MINGW64 env var"
@pause
@goto :End

:NoEnvMissing
@rem setup variables

@if "%1" == "" (set BUILDTYPE=/Build) else (set BUILDTYPE=/%1)

@set ORIGPATH="%CD%"
@rem FIXME: Does this work for x64 builds ??
@rem we do have a good alternative vcvarsall.bat x86 | x64
@rem Default location: "C:\Program Files\Microsoft Visual Studio 9\VC\Vcvarsall.bat"
@call "%VS90COMNTOOLS%vsvars32.bat"
@cd %ORIGPATH%

@rem no @ before devenv, we need to see the project name in case something fails
@set BUILD_APP=devenv /nologo
@rem For debugging batch script:
@rem set BUILD_APP=@echo devenv /nologo

@rem Debug build only applies to Main(mpc-hc.sln), Resource will always build a Release
@if /i "%4" == "Debug" (set BUILDCONFIG=Debug) else (set BUILDCONFIG=Release)

@rem Do we want to build x86, x64 or both ?
@if /i "%2" == "x64" goto :skip32

@set COPY_TO_DIR=..\..\..\bin\mpc-hc_x86
@set Platform=Win32
@call :Sub_build_internal %*
@if %ERRORLEVEL% NEQ 0 goto :EndWithError

:skip32
@if /i "%2" == "x86" goto :skip64

@set COPY_TO_DIR=..\..\..\bin\mpc-hc_x64
@set Platform=x64
@call :Sub_build_internal %*
@if %ERRORLEVEL% NEQ 0 goto :EndWithError

:skip64

@goto :End

:EndWithError
@echo
@echo ERROR: Build failed and aborted
@pause
@exit

:End
@goto :eof


:Sub_build_internal
@if /i "%3"=="Resource" goto :skipMain
%BUILD_APP% ..\..\..\mpc-hc.sln %BUILDTYPE% "%BUILDCONFIG%|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto :EndWithError

:skipMain
@if /i "%3"=="Main" goto :skipResource
%BUILD_APP% ..\..\..\mpciconlib.sln %BUILDTYPE% "Release|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto :EndWithError

@DEL/f/a "%COPY_TO_DIR%\mpciconlib.exp" "%COPY_TO_DIR%\mpciconlib.lib"

@FOR %%A IN ("Belarusian" "Catalan" "Chinese simplified" "Chinese traditional" 
"Czech" "Dutch" "French" "German" "Hungarian" "Italian" "Korean" "Polish" 
"Portuguese" "Russian" "Slovak" "Spanish" "Swedish" "Turkish" "Ukrainian"
) DO (
CALL :SubMPCRES %%A
)

:skipResource
@if /i "%1" == "clean" goto :skipCopy

@if not exist %COPY_TO_DIR% mkdir %COPY_TO_DIR%

@if /i "%3" == "Resource" goto :skipMainCopy

:skipMainCopy
@if /i "%3" == "Main" goto :skipResourceCopy

:skipResourceCopy
@if /i "%3" == "Resource" goto :skipCopy
xcopy AUTHORS ".\%COPY_TO_DIR%\" /y /v
xcopy ChangeLog ".\%COPY_TO_DIR%\" /y /v
xcopy ..\..\..\COPYING ".\%COPY_TO_DIR%\" /y /v

:skipCopy
:EndWithError
@goto :eof


:SubMPCRES
%BUILD_APP% ..\..\..\mpcresources.sln %BUILDTYPE% "Release %~1|%Platform%"
@if %ERRORLEVEL% NEQ 0 goto :EndWithError
@goto :eof
