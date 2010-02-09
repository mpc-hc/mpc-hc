/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include "../../../DSUtil/SharedInclude.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components

// TODO: reference additional headers your program requires here

#include <dshow.h>
#include <streams.h>
#include <dvdmedia.h>

#include <atlbase.h>
#include <atlcoll.h>

#include "../../../DSUtil/DSUtil.h"
#include "../../../DSUtil/MediaTypes.h"
#include "../../../DSUtil/vd.h"

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

#include <libdirac_common/common.h>
#include <libdirac_common/dirac_assertions.h>
#include <libdirac_decoder/dirac_parser.h>

using namespace dirac;


// === Imported from Dirac library
#define isFrameStartCode(c) ((c) == 0x00 || (c) == 0x0C || (c) == 0x08 || (c) == 0x0D || (c) == 0x0E || (c) == 0x09 || (c) == 0x0A || (c) == 0x10)

    const unsigned int START_CODE_PREFIX = 0x42424344; //BBCD
    const unsigned int START_CODE_PREFIX_BYTE0 = 
                                        (START_CODE_PREFIX >> 24) & 0xFF;
    const unsigned int START_CODE_PREFIX_BYTE1 = 
                                        (START_CODE_PREFIX >> 16) & 0xFF;
    const unsigned int START_CODE_PREFIX_BYTE2 = 
                                        (START_CODE_PREFIX >>  8) & 0xFF;
    const unsigned int START_CODE_PREFIX_BYTE3 = 
                                        START_CODE_PREFIX & 0xFF;

    //! Access unit start code
    const unsigned char AU_START_CODE = 0x00;
    //! Sequence end code
    const unsigned char SEQ_END_CODE = 0x10;

