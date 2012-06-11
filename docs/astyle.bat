@ECHO OFF
SETLOCAL

PUSHD %~dp0

astyle.exe --version 1>&2 2>NUL

IF %ERRORLEVEL% NEQ 0 (
  ECHO ERROR: Astyle wasn't found!
  ECHO Visit http://astyle.sourceforge.net/ for download and details.
  GOTO END
)

rem current command
astyle.exe^
 --indent=force-tab --indent-switches --indent-namespaces --indent-col1-comments^
 --pad-header --lineend=windows --preserve-date^
 --keep-one-line-blocks --keep-one-line-statements^
 --suffix=none --ignore-exclude-errors --ignore-exclude-errors-x --recursive^
 --exclude=FileVersionInfo.cpp --exclude=FileVersionInfo.h --exclude=LineNumberEdit.cpp^
 --exclude=LineNumberEdit.h --exclude=Struct.h --exclude=atl --exclude=avisynth^
 --exclude=detours --exclude=dx --exclude=lglcd --exclude=ogg --exclude=qt^
 --exclude=realmedia --exclude=vd2 --exclude=winddk --exclude=inttypes.h^
 --exclude=MediaInfoDLL.h --exclude=unrar.h --exclude=Version.h --exclude=Version_rev.h^
 --exclude=thirdparty --exclude=resource.h --exclude=simd.h --exclude=simd_common.h^
 --exclude=libmpeg2.h --exclude=libmpeg2.cpp --exclude=attributes.h --exclude=MPEG2Dec.h^
 --exclude=MPEG2Dec.cpp --exclude=idctref.cpp --exclude=idctfpu.cpp --exclude=vd_asm.cpp^
 --exclude=vd.cpp --exclude=vd.h --exclude=deinterlace.cpp --exclude=mmintrin64.c^
 --exclude=simd_instructions.h --exclude=DeCSS --exclude=ffImgfmt.cpp --exclude=ffImgfmt.h^
 --exclude=H264RandomAccess.cpp --exclude=H264RandomAccess.h --exclude=array_allocator.h^
 --exclude=char_t.h ..\*.h ..\*.cpp

rem TODO: use --indent=spaces=4 --style=kr --add-brackets --pad-header --pad-oper --convert-tabs
rem maybe TODO:  --align-pointer=type --align-reference=type: there is a problem when the type isn't known like *dump
rem              --unpad-paren: HANDLE ()-> HANDLE() while the space should be kept

IF %ERRORLEVEL% NEQ 0 (
  ECHO.
  ECHO ERROR: Something went wrong!
)

:END
POPD
PAUSE
ENDLOCAL
EXIT /B
