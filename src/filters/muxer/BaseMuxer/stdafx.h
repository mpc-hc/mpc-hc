/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2016 see Authors.txt
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

#define WIN32_LEAN_AND_MEAN                 // Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit
#define VC_EXTRALEAN                        // Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxwin.h>                         // MFC core and standard components

#include <atlbase.h>
#include <atlcoll.h>

#include "BaseClasses/streams.h"
#include "../../../DSUtil/DSUtil.h"
