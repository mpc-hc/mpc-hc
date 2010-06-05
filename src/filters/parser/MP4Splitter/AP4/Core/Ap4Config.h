/*****************************************************************
|
|    AP4 - Target Platform and Compiler Configuration
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/
/**
 * @file
 * @brief Platform Configuration
 */
#ifndef _AP4_CONFIG_H_
#define _AP4_CONFIG_H_

/*----------------------------------------------------------------------
|   defaults
+---------------------------------------------------------------------*/
#define AP4_CONFIG_HAVE_STDIO_H
#define AP4_CONFIG_HAVE_ASSERT_H
#define AP4_CONFIG_HAVE_STRING_H
#define AP4_CONFIG_HAVE_SNPRINTF
#define AP4_CONFIG_HAVE_VSNPRINTF
#define AP4_CONFIG_HAVE_INT64

/*----------------------------------------------------------------------
|   byte order
+---------------------------------------------------------------------*/
// define AP4_PLATFORM_BYTE_ORDER to one of these two choices
#define AP4_PLATFORM_BYTE_ORDER_BIG_ENDIAN    0
#define AP4_PLATFORM_BYTE_ORDER_LITTLE_ENDIAN 1

#if !defined(AP4_PLATFORM_BYTE_ORDER)
#if defined(__ppc__)
#define AP4_PLATFORM_BYTE_ORDER AP4_PLATFORM_BYTE_ORDER_BIG_ENDIAN
#elif defined(__i386__) || defined(__x86_64__) || defined(__arm__)
#define AP4_PLATFORM_BYTE_ORDER AP4_PLATFORM_BYTE_ORDER_LITTLE_ENDIAN
#else /* MPC custom code*/
#define AP4_PLATFORM_BYTE_ORDER AP4_PLATFORM_BYTE_ORDER_LITTLE_ENDIAN
#endif
#endif

/*----------------------------------------------------------------------
|   standard C++ runtime
+---------------------------------------------------------------------*/
#define APT_CONFIG_HAVE_NEW_H

/*----------------------------------------------------------------------
|   platform specifics
+---------------------------------------------------------------------*/

/* Microsoft Platforms */
#if defined(_MSC_VER)
#define AP4_CONFIG_INT64_TYPE __int64
#if (_MSC_VER >= 1400) && !defined(_WIN32_WCE)
#define AP4_CONFIG_HAVE_FOPEN_S
#define AP4_snprintf(s,c,f,...) _snprintf_s(s,c,_TRUNCATE,f,__VA_ARGS__)
#define AP4_vsnprintf(s,c,f,a)  _vsnprintf_s(s,c,_TRUNCATE,f,a)
#define fileno _fileno
#define AP4_fseek _fseeki64
#define AP4_ftell _ftelli64
#else
#define AP4_snprintf   _snprintf
#define AP4_vsnprintf  _vsnprintf
#endif
#if defined(_WIN32_WCE)
#define AP4_fseek fseek
#define AP4_ftell ftell
#endif
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#endif
#endif

/* Cygwin */
#if defined(__CYGWIN__)
#define AP4_fseek fseek
#define AP4_ftell ftell
#endif

/* Symbian */
#if defined(__SYMBIAN32__)
#undef APT_CONFIG_HAVE_NEW_H
#include "e32std.h"
/**
 * Define the Platform byte order here
 * for Symbian.
 */
#define AP4_PLATFORM_BYTE_ORDER AP4_PLATFORM_BYTE_ORDER_LITTLE_ENDIAN
#define AP4_fseek fseek
#define AP4_ftell ftell
#define explicit
#endif

/* Android */
#if defined(ANDROID)
#define AP4_CONFIG_NO_RTTI
#define AP4_CONFIG_NO_EXCEPTIONS
#endif

/*----------------------------------------------------------------------
|    defaults
+---------------------------------------------------------------------*/
#if !defined(AP4_CONFIG_INT64_TYPE)
#define AP4_CONFIG_INT64_TYPE long long
#endif

#if !defined(AP4_fseek)
#define AP4_fseek fseeko
#endif
#if !defined(AP4_ftell)
#define AP4_ftell ftello
#endif

/* some compilers (ex: MSVC 8) deprecate those, so we rename them */
#if !defined(AP4_snprintf)
#define AP4_snprintf snprintf
#endif
#if !defined(AP4_vsnprintf)
#define AP4_vsnprintf vsnprintf
#endif

#endif // _AP4_CONFIG_H_
