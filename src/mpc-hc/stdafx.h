/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "SharedInclude.h"
#include "../../include/stdafx_common.h"
#undef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include "../../include/stdafx_common_afx2.h"
#include "../../include/stdafx_common_dshow.h"
#include "mpc-hc_config.h"

#include <Windows.h>
#include <algorithm>
#include <afxadv.h>
#include <afxcview.h>
#include <afxcmn.h>
#include <afxpriv.h>
#include <afxsock.h>
#include <afxwin.h>
#include <afxinet.h>
#include <atlbase.h>
#include <atlcoll.h>
#include <atlconv.h>
#include <atlimage.h>
#include <atlpath.h>
#include <atlsync.h>
#include <comdef.h>
#include <commdlg.h>

#include <dlgs.h>
#include <ks.h>
#include <ksmedia.h>
#include <mpconfig.h>
#include <psapi.h>
#include <shlobj.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <dxva2api.h>
#include <dvdevcod.h>
#include <dsound.h>
#include <evr.h>
#include <evr9.h>
#include <vmr9.h>

#include <Il21dec.h>

#include "sizecbar/scbarg.h"
#include "ResizableLib/ResizableDialog.h"

#include "../Subtitles/RTS.h"
#include "../Subtitles/STS.h"

#include "DSUtil.h"

template <class T = CString, class S = CString>
class CAtlStringMap : public CAtlMap<S, T, CStringElementTraits<S>> {};

#define CheckAndLog(x, msg)  hr = ##x; if (FAILED(hr)) { TRACE(msg _T(": 0x%08x\n"), hr); return hr; }
#define CheckNoLog(x)        hr = ##x; if (FAILED(hr)) { return hr; }

#include "resource.h"
#include "FakeFilterMapper2.h"
#include "AppSettings.h"
