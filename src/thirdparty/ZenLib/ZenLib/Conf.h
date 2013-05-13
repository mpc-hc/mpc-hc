/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef ZenConfH
#define ZenConfH
//---------------------------------------------------------------------------

//***************************************************************************
// Platforms
//***************************************************************************

//---------------------------------------------------------------------------
//Win32
#if defined(__NT__) || defined(_WIN32) || defined(WIN32)
    #ifndef WIN32
        #define WIN32
    #endif
    #ifndef _WIN32
        #define _WIN32
    #endif
    #ifndef __WIN32__
        #define __WIN32__ 1
    #endif
#endif

//---------------------------------------------------------------------------
//Win64
#if defined(_WIN64) || defined(WIN64)
    #ifndef WIN64
        #define WIN64
    #endif
    #ifndef _WIN64
        #define _WIN64
    #endif
    #ifndef __WIN64__
        #define __WIN64__ 1
    #endif
#endif

//---------------------------------------------------------------------------
//Windows
#if defined(WIN32) || defined(WIN64)
    #ifndef WINDOWS
        #define WINDOWS
    #endif
    #ifndef _WINDOWS
        #define _WINDOWS
    #endif
    #ifndef __WINDOWS__
        #define __WINDOWS__ 1
    #endif
#endif
#if !defined(ZENLIB_NO_WIN9X_SUPPORT) && (defined(_M_X64) || defined(_M_IA64))
    #define ZENLIB_NO_WIN9X_SUPPORT
#endif

//---------------------------------------------------------------------------
//Unix (Linux, HP, Sun, BeOS...)
#if defined(UNIX) || defined(_UNIX) || defined(__UNIX__) \
    || defined(__unix) || defined(__unix__) \
    || defined(____SVR4____) || defined(__LINUX__) || defined(__sgi) \
    || defined(__hpux) || defined(sun) || defined(__SUN__) || defined(_AIX) \
    || defined(__EMX__) || defined(__VMS) || defined(__BEOS__)
    #ifndef UNIX
        #define UNIX
    #endif
    #ifndef _UNIX
        #define _UNIX
    #endif
    #ifndef __UNIX__
        #define __UNIX__ 1
    #endif
#endif

//---------------------------------------------------------------------------
//MacOS Classic
#if defined(macintosh)
    #ifndef MACOS
        #define MACOS
    #endif
    #ifndef _MACOS
        #define _MACOS
    #endif
    #ifndef __MACOS__
        #define __MACOS__ 1
    #endif
#endif

//---------------------------------------------------------------------------
//MacOS X
#if defined(__APPLE__) && defined(__MACH__)
    #ifndef MACOSX
        #define MACOSX
    #endif
    #ifndef _MACOSX
        #define _MACOSX
    #endif
    #ifndef __MACOSX__
        #define __MACOSX__ 1
    #endif
#endif

//Test of targets
#if defined(WINDOWS) && defined(UNIX) && defined(MACOS) && defined(MACOSX)
    #pragma message Multiple platforms???
#endif

#if !defined(WIN32) && !defined(UNIX) && !defined(MACOS) && !defined(MACOSX)
    #pragma message No known platforms, assume default
#endif

//***************************************************************************
// Internationnal
//***************************************************************************

//---------------------------------------------------------------------------
//Unicode
#if defined(_UNICODE) || defined(UNICODE) || defined(__UNICODE__)
    #ifndef _UNICODE
        #define _UNICODE
    #endif
    #ifndef UNICODE
        #define UNICODE
    #endif
    #ifndef __UNICODE__
        #define __UNICODE__ 1
    #endif
#endif

//---------------------------------------------------------------------------
//wchar_t stuff
#if defined(MACOS) || defined(MACOSX)
    #include <wchar.h>
#endif

//***************************************************************************
// Compiler bugs/unuseful warning
//***************************************************************************

//MSVC6 : for(int t=0; t<10; ++t) { do something }; for(int t=0; t<10; ++t) { do something }
#if defined(_MSC_VER) && _MSC_VER <= 1200
    #define for if(true)for
    #pragma warning(disable:4786) // MSVC6 doesn't like typenames longer than 255 chars (which generates an enormous amount of warnings).
#endif

//MSVC2005 : "deprecated" warning (replacement functions are not in MinGW32 or Borland!)
#if defined(_MSC_VER) && _MSC_VER >= 1400
    #pragma warning(disable : 4996)
#endif

//***************************************************************************
// (Without Namespace)
//***************************************************************************

//---------------------------------------------------------------------------
#include <limits.h>

//---------------------------------------------------------------------------
#if defined(ZENLIB_DEBUG) && (defined(DEBUG) || defined(_DEBUG))
    #include "ZenLib/MemoryDebug.h"
#endif // defined(ZENLIB_DEBUG) && (defined(DEBUG) || defined(_DEBUG))

//***************************************************************************
// Compiler helpers
//***************************************************************************

//---------------------------------------------------------------------------
//Macro to cut down on compiler warnings
#ifndef UNUSED
    #define UNUSED(Identifier)
#endif
//---------------------------------------------------------------------------
//If we need size_t specific integer conversion
#if !defined(SIZE_T_IS_LONG) && (defined(__LP64__) || defined(MACOSX))
    #define SIZE_T_IS_LONG
#endif

//---------------------------------------------------------------------------
//(-1) is known to be the MAX of an unsigned int but GCC complains about it
#include <new>
#include <cstring> //size_t
namespace ZenLib
{
    const std::size_t Error=((std::size_t)(-1));
    const std::size_t All=((std::size_t)(-1));
    const std::size_t Unlimited=((std::size_t)(-1));
}

//***************************************************************************
// (With namespace)
//***************************************************************************

namespace ZenLib
{

//***************************************************************************
// International
//***************************************************************************

//---------------------------------------------------------------------------
//Char types
#if defined(__UNICODE__)
    #if defined (_MSC_VER) && !defined (_NATIVE_WCHAR_T_DEFINED)
        #pragma message Native wchar_t is not defined, not tested, you should put /Zc:wchar_t in compiler options
    #endif
    typedef wchar_t Char;
    #undef  __T
    #define __T(__x) L ## __x
#else // defined(__UNICODE__)
    typedef char Char;
    #undef  __T
    #define __T(__x) __x
#endif // defined(__UNICODE__)
#ifdef wchar_t
    typedef wchar_t wchar;
#endif // wchar_t

//***************************************************************************
// Platform differences
//***************************************************************************

//End of line
extern const Char* EOL;
extern const Char  PathSeparator;

//***************************************************************************
// Types
//***************************************************************************

//---------------------------------------------------------------------------
//int
typedef signed   int            ints;
typedef unsigned int            intu;

//---------------------------------------------------------------------------
//8-bit int
#if UCHAR_MAX==0xff
    #undef  MAXTYPE_INT
    #define MAXTYPE_INT 8
    typedef signed   char       int8s;
    typedef unsigned char       int8u;
#else
    #pragma message This machine has no 8-bit integertype?
#endif

//---------------------------------------------------------------------------
//16-bit int
#if UINT_MAX == 0xffff
    #undef  MAXTYPE_INT
    #define MAXTYPE_INT 16
    typedef signed   int        int16s;
    typedef unsigned int        int16u;
#elif USHRT_MAX == 0xffff
    #undef  MAXTYPE_INT
    #define MAXTYPE_INT 16
    typedef signed   short      int16s;
    typedef unsigned short      int16u;
#else
    #pragma message This machine has no 16-bit integertype?
#endif

//---------------------------------------------------------------------------
//32-bit int
#if UINT_MAX == 0xfffffffful
    #undef  MAXTYPE_INT
    #define MAXTYPE_INT 32
    typedef signed   int        int32s;
    typedef unsigned int        int32u;
#elif ULONG_MAX == 0xfffffffful
    #undef  MAXTYPE_INT
    #define MAXTYPE_INT 32
    typedef signed   long       int32s;
    typedef unsigned long       int32u;
#elif USHRT_MAX == 0xfffffffful
    #undef  MAXTYPE_INT
    #define MAXTYPE_INT 32
    typedef signed   short      int32s;
    typedef unsigned short      int32u;
#else
    #pragma message This machine has no 32-bit integer type?
#endif

//---------------------------------------------------------------------------
//64-bit int
#if defined(__MINGW32__) || defined(__CYGWIN32__) || defined(__UNIX__) || defined(__MACOSX__)
    #undef  MAXTYPE_INT
    #define MAXTYPE_INT 64
    typedef signed   long long  int64s;
    typedef unsigned long long  int64u;
#elif defined(__WIN32__)
    #undef  MAXTYPE_INT
    #define MAXTYPE_INT 64
    typedef signed   __int64    int64s;
    typedef unsigned __int64    int64u;
#else
    #pragma message This machine has no 64-bit integer type?
#endif

//---------------------------------------------------------------------------
//32-bit float
#if defined(WINDOWS) || defined(UNIX) || defined(MACOSX)
    #undef  MAXTYPE_FLOAT
    #define MAXTYPE_FLOAT 32
    typedef float                float32;
#else
    #pragma message This machine has no 32-bit float type?
#endif

//---------------------------------------------------------------------------
//64-bit float
#if defined(WINDOWS) || defined(UNIX) || defined(MACOSX)
    #undef  MAXTYPE_FLOAT
    #define MAXTYPE_FLOAT 64
    typedef double                float64;
#else
    #pragma message This machine has no 64-bit float type?
#endif

//---------------------------------------------------------------------------
//80-bit float
#if defined(WINDOWS) || defined(UNIX) || defined(MACOSX)
    #undef  MAXTYPE_FLOAT
    #define MAXTYPE_FLOAT 80
    typedef long double           float80;
#else
    #pragma message This machine has no 80-bit float type?
#endif

//***************************************************************************
// Nested functions
//***************************************************************************

//Unices
#if defined (UNIX)
    #define snwprintf swprintf
#endif

//Windows - MSVC
#if defined (_MSC_VER)
    #define snprintf _snprintf
    #define snwprintf _snwprintf
#endif

} //namespace
#endif
