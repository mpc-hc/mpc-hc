/*****************************************************************
|
|    AP4 - Shared Types
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

#ifndef _AP4_TYPES_H_
#define _AP4_TYPES_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Config.h"

/*----------------------------------------------------------------------
|   types
+---------------------------------------------------------------------*/
typedef int            AP4_Result;
typedef unsigned int   AP4_Flags;
typedef unsigned int   AP4_Mask;
typedef unsigned int   AP4_Cardinal;
typedef unsigned int   AP4_Ordinal;
typedef unsigned int   AP4_UI32;
typedef signed   int   AP4_SI32;
typedef unsigned short AP4_UI16;
typedef signed   short AP4_SI16;
typedef unsigned char  AP4_UI08;
typedef AP4_UI08       AP4_Byte;
typedef AP4_UI32       AP4_Size;

// the rest depends on whether the platform supports 64-bit integers
#if defined(AP4_CONFIG_HAVE_INT64)
    // we have 64-bit integers
    typedef AP4_CONFIG_INT64_TYPE          AP4_SI64;
    typedef unsigned AP4_CONFIG_INT64_TYPE AP4_UI64;
    typedef unsigned AP4_CONFIG_INT64_TYPE AP4_LargeSize;
    typedef AP4_CONFIG_INT64_TYPE          AP4_Offset;
    typedef unsigned AP4_CONFIG_INT64_TYPE AP4_Position;
#else
    // use only 32-bit integers
    typedef struct {
        AP4_UI32 hi;
        AP4_UI32 lo;
    } AP4_UI64, AP4_SI64;
    typedef unsigned long  AP4_LargeSize;
    typedef long           AP4_Offset;
    typedef unsigned long  AP4_Position;
#endif

#ifndef NULL
#define NULL 0
#endif

#endif // _AP4_TYPES_H_
