/*
 * $Id$
 *
 * (C) 2011-2012 see Authors.txt
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

#include <Windows.h>

struct IDirect3D9;

BOOL IsWinXPOrLater();
BOOL IsWinVistaOrLater();
BOOL IsWin7OrLater();
//BOOL IsWin8();

bool SetPrivilege(LPCTSTR privilege, bool bEnable=true);

bool ExportRegistryKey(CStdioFile& file, HKEY hKeyRoot, CString keyName=_T(""));

UINT GetAdapter(IDirect3D9* pD3D, HWND hWnd);

bool IsFontInstalled(LPCTSTR lpszFont);

bool ExploreToFile(CString path);
