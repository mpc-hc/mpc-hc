/*
 * $Id$
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

#include "stdafx.h"
#include <initguid.h>
#include "AllocatorCommon7.h"
#include "../DSUtil/DSUtil.h"

#include "DX7AllocatorPresenter.h"
#include "VMR7AllocatorPresenter.h"
#include "RM7AllocatorPresenter.h"
#include "QT7AllocatorPresenter.h"


bool IsVMR7InGraph(IFilterGraph* pFG)
{
	BeginEnumFilters(pFG, pEF, pBF)
	if(CComQIPtr<IVMRWindowlessControl>(pBF)) {
		return(true);
	}
	EndEnumFilters
	return(false);
}

using namespace DSObjects;

//

HRESULT CreateAP7(const CLSID& clsid, HWND hWnd, ISubPicAllocatorPresenter** ppAP)
{
	CheckPointer(ppAP, E_POINTER);

	*ppAP = NULL;

	HRESULT hr = S_OK;

	if ( IsEqualCLSID(clsid, CLSID_VMR7AllocatorPresenter) ) {
		*ppAP = DNew CVMR7AllocatorPresenter(hWnd, hr);
	}
	else if ( IsEqualCLSID(clsid, CLSID_RM7AllocatorPresenter) ) {
		*ppAP = DNew CRM7AllocatorPresenter(hWnd, hr);
	}
	else if ( IsEqualCLSID(clsid, CLSID_QT7AllocatorPresenter) ) {
		*ppAP = DNew CQT7AllocatorPresenter(hWnd, hr);
	}
	else {
		return E_FAIL;
	}

	if ( *ppAP == NULL ) {
		return E_OUTOFMEMORY;
	}

	(*ppAP)->AddRef();

	if(FAILED(hr)) {
		(*ppAP)->Release();
		*ppAP = NULL;
	}

	return hr;
}
