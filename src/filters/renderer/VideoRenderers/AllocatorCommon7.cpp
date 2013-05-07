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

#include "stdafx.h"
#include <InitGuid.h>
#include "AllocatorCommon7.h"
#include "../DSUtil/DSUtil.h"

#include "DX7AllocatorPresenter.h"
#include "VMR7AllocatorPresenter.h"
#include "RM7AllocatorPresenter.h"
#include "QT7AllocatorPresenter.h"


bool IsVMR7InGraph(IFilterGraph* pFG)
{
    BeginEnumFilters(pFG, pEF, pBF);
    if (CComQIPtr<IVMRWindowlessControl>(pBF)) {
        return true;
    }
    EndEnumFilters;
    return false;
}

using namespace DSObjects;

//

HRESULT CreateAP7(const CLSID& clsid, HWND hWnd, ISubPicAllocatorPresenter** ppAP)
{
    CheckPointer(ppAP, E_POINTER);

    *ppAP = nullptr;

    HRESULT hr = S_OK;

    if (IsEqualCLSID(clsid, CLSID_VMR7AllocatorPresenter)) {
        *ppAP = DEBUG_NEW CVMR7AllocatorPresenter(hWnd, hr);
    } else if (IsEqualCLSID(clsid, CLSID_RM7AllocatorPresenter)) {
        *ppAP = DEBUG_NEW CRM7AllocatorPresenter(hWnd, hr);
    } else if (IsEqualCLSID(clsid, CLSID_QT7AllocatorPresenter)) {
        *ppAP = DEBUG_NEW CQT7AllocatorPresenter(hWnd, hr);
    } else {
        return E_FAIL;
    }

    if (*ppAP == nullptr) {
        return E_OUTOFMEMORY;
    }

    (*ppAP)->AddRef();

    if (FAILED(hr)) {
        (*ppAP)->Release();
        *ppAP = nullptr;
    }

    return hr;
}
