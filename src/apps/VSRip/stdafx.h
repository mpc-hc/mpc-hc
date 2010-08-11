/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of vsrip.
 *
 * Vsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN			// Exclude rarely-used stuff from Windows headers
#endif

#ifndef WINVER
#define WINVER 0x0600
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>				// MFC core and standard components
#include <afxext.h>				// MFC extensions
#include <afxdisp.h>			// MFC Automation classes
#include <afxdtctl.h>			// MFC support for Internet Explorer 4 Common Controls
#include <afxdlgs.h>

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>				// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <streams.h>
