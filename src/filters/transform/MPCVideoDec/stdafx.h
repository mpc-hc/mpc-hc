/*
 * $Id$
 *
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#pragma once
#include "../../../DSUtil/SharedInclude.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>			// MFC core and standard components


#include <streams.h>
#include <dvdmedia.h>
#include <d3dx9.h>
#include <evr.h>
#include <mfapi.h>
#include <mferror.h>
#include <atlcoll.h>
#include <vector>


#ifdef _DEBUG
void LOG(LPCTSTR fmt, ...);
#else
inline void LOG(LPCTSTR fmt, ...) {}
#endif

#define CHECK_HR(x)			hr = ##x; if (FAILED (hr)) { TRACE("Error : 0x%08x\n", hr); ASSERT (hr==VFW_E_NOT_COMMITTED); return hr; }
