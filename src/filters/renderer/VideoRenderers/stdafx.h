/*
 * $Id: stdafx.h 1785 2010-04-09 14:12:59Z xhmikosr $
 * stdafx.h : include file for standard system include files,
 * or project specific include files that are used frequently, but
 * are changed infrequently
 *
 * (C) 2003-2006 Gabest
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

#if !defined(AFX_STDAFX_H__C76533D6_6242_4BEB_8FD3_C6BE58F07225__INCLUDED_)
#define AFX_STDAFX_H__C76533D6_6242_4BEB_8FD3_C6BE58F07225__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../../../DSUtil/SharedInclude.h"

//#define _WIN32_IE		0x0600
#define WINVER			0x0600

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define ResStr(id) CString(MAKEINTRESOURCE(id))

#include <afxdisp.h>
#include <afxole.h>

#include <streams.h>
#include <dvdmedia.h>
#include <mpconfig.h>
#ifndef _WIN64
#include <qt/qt.h>
#endif

#include "../../../DSUtil/DSUtil.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C76533D6_6242_4BEB_8FD3_C6BE58F07225__INCLUDED_)
