/*
 * $Id$
 *
 * (C) 2007-2012 see Authors.txt
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

#include "../../../DSUtil/SharedInclude.h"
#include "../../../../include/stdafx_common.h"
#include "../../../../include/stdafx_common_afx.h"
#include "../../../../include/stdafx_common_dshow.h"

#include <d3dx9.h>
#include <evr.h>
#include <mfapi.h>
#include <Mferror.h>
#include <atlcoll.h>
#include <vector>

#if defined(_DEBUG) && defined(DXVA_LOGFILE_B)
void LOG(LPCTSTR fmt, ...);
#else
inline void LOG(LPCTSTR fmt, ...) {}
#endif
