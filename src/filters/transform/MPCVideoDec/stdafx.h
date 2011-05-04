/*
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

#pragma once

#include "../../../DSUtil/SharedInclude.h"
#include "../../../../include/stdafx_common.h"
#include "../../../../include/stdafx_common_afx.h"
#include "../../../../include/stdafx_common_dshow.h"

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
