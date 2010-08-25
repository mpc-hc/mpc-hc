// ZenLib::Conf_Internal - To be independant of platform & compiler
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenConf_InternalH
#define ZenConf_InternalH
//---------------------------------------------------------------------------

#include "ZenLib/Conf.h"

//***************************************************************************
// Choice of method
//***************************************************************************

#ifndef ZENLIB_USEWX
    #ifdef WINDOWS
    #else
        #define ZENLIB_STANDARD //We select the C/C++ standard as much as possible
    #endif
#endif //ZENLIB_USEWX

//***************************************************************************
// Default values
//***************************************************************************

#ifndef _LARGE_FILES
    #define _LARGE_FILES
#endif //_LARGE_FILES

//***************************************************************************
// Includes
//***************************************************************************

//---------------------------------------------------------------------------
//Useful for precompiled headers
#ifdef ZENLIB_USEWX
    #ifndef __BORLANDC__ //Borland C++ does NOT support large files
        #ifndef _FILE_OFFSET_BITS
            #define _FILE_OFFSET_BITS 64
        #endif //_FILE_OFFSET_BITS
        #ifndef _LARGE_FILES
            #define _LARGE_FILES
        #endif //_LARGE_FILES
        #ifndef _LARGEFILE_SOURCE
            #define _LARGEFILE_SOURCE 1
        #endif //_LARGEFILE_SOURCE
    #endif //__BORLANDC__
    #ifdef __BORLANDC__
        #include <mem.h> //memcpy
    #endif //__BORLANDC__
    #include <wx/wxprec.h>
#else //ZENLIB_USEWX
    #if defined(__VISUALC__) || defined(__BORLANDC__)
        #if defined WINDOWS && !defined ZENLIB_STANDARD
            #undef __TEXT
            #include <windows.h>
        #endif //WINDOWS
        #include <algorithm>
        #include <cmath>
        #include <complex>
        #include <cstdio>
        #include <cstdlib>
        #include <cstring>
        #include <ctime>
        #include <fstream>
        #include <functional>
        #include <iomanip>
        #include <map>
        #include <memory>
        #include <sstream>
        #include <stack>
        #include <string>
    #endif //defined(__VISUALC__) || defined(__BORLANDC__)
#endif //ZENLIB_USEWX

#endif

