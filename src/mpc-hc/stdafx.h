/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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

#define VC_EXTRALEAN                        // Exclude rarely-used stuff from Windows headers

#ifndef STRICT_TYPED_ITEMIDS
#define STRICT_TYPED_ITEMIDS
#endif

#include <afxwin.h>                         // MFC core and standard components
#include <afxext.h>                         // MFC extensions
#include <afxdisp.h>                        // MFC Automation classes
#include <afxdtctl.h>                       // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                         // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxdlgs.h>


#include "SharedInclude.h"
#include "mpc-hc_config.h"
#include "DSUtil.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <set>
#include <vector>

// Workaround compilation errors when including GDI+ with NOMINMAX defined
namespace Gdiplus
{
    using std::min;
    using std::max;
};

#include <afxole.h>
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
#include "../Subtitles/RLECodedSubtitle.h"

template <class T = CString, class S = CString>
class CAtlStringMap : public CAtlMap<S, T, CStringElementTraits<S>> {};

#define CheckAndLog(x, msg)  hr = ##x; if (FAILED(hr)) { TRACE(msg _T(": 0x%08x\n"), hr); return hr; }
#define CheckNoLog(x)        hr = ##x; if (FAILED(hr)) { return hr; }
#define CheckNoLogBool(x)    if (FAILED(x)) { return false; }

#include "resource.h"
#include "FakeFilterMapper2.h"
#include "AppSettings.h"
