/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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
#include "../../../../include/stdafx_common.h"
#undef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include "../../../../include/stdafx_common_afx2.h"
#include "../../../../include/stdafx_common_dshow.h"

#include <afxdlgs.h>

#include <Shlwapi.h>
#include <atlcoll.h>
#include <atlpath.h>

#define HITTEST_RET LRESULT

#include "../../thirdparty/zlib/zlib.h"
#include "../../CmdUI/CmdUI.h"
#include "../../thirdparty/ui/ResizableLib/ResizableDialog.h"
#include "../../thirdparty/ui/ResizableLib/ResizablePage.h"
#include "../../thirdparty/ui/ResizableLib/ResizableSheet.h"
#include "../../thirdparty/ui/sizecbar/sizecbar.h"
#include "../../thirdparty/ui/sizecbar/scbarcf.h"
#include "../../thirdparty/ui/sizecbar/scbarg.h"
#include "../../thirdparty/ui/TreePropSheet/TreePropSheet.h"
#include "../../DSUtil/DSUtil.h"

#ifndef _WIN64
#include <qt/qt.h>
#endif

#include <gdiplus.h>
#include <mpconfig.h>

#define ResStr(id) CString(MAKEINTRESOURCE(id))

template <class T = CString, class S = CString>
class CAtlStringMap : public CAtlMap<S, T, CStringElementTraits<S> > {};

#define CheckAndLog(x, msg)		hr = ##x; if (FAILED (hr)) { TRACE(msg" : 0x%08x\n", hr); return hr; }
#define CheckNoLog(x)			hr = ##x; if (FAILED (hr)) { return hr; }
