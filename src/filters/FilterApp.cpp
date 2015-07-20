/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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

#include "stdafx.h"
#include "FilterApp.h"

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

CFilterApp::CFilterApp()
{
}

BOOL CFilterApp::InitInstance()
{
    if (!__super::InitInstance()) {
        return FALSE;
    }

    SetRegistryKey(_T("Gabest"));

    DllEntryPoint(AfxGetInstanceHandle(), DLL_PROCESS_ATTACH, 0);

    return TRUE;
}

BOOL CFilterApp::ExitInstance()
{
    DllEntryPoint(AfxGetInstanceHandle(), DLL_PROCESS_DETACH, 0);

    return __super::ExitInstance();
}

BEGIN_MESSAGE_MAP(CFilterApp, CWinApp)
END_MESSAGE_MAP()
