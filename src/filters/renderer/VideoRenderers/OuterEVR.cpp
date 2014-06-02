/*
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

#include "stdafx.h"
#include "AllocatorCommon.h"
#include "EVRAllocatorPresenter.h"
#include "OuterEVR.h"

using namespace DSObjects;

STDMETHODIMP COuterEVR::EnumPins(__out  IEnumPins** ppEnum)
{
    if (m_pEVRBase) {
        return m_pEVRBase->EnumPins(ppEnum);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::FindPin(LPCWSTR Id, __out  IPin** ppPin)
{
    if (m_pEVRBase) {
        return m_pEVRBase->FindPin(Id, ppPin);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::QueryFilterInfo(__out  FILTER_INFO* pInfo)
{
    if (m_pEVRBase) {
        return m_pEVRBase->QueryFilterInfo(pInfo);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::JoinFilterGraph(__in_opt  IFilterGraph* pGraph, __in_opt  LPCWSTR pName)
{
    if (m_pEVRBase) {
        return m_pEVRBase->JoinFilterGraph(pGraph, pName);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::QueryVendorInfo(__out  LPWSTR* pVendorInfo)
{
    if (m_pEVRBase) {
        return m_pEVRBase->QueryVendorInfo(pVendorInfo);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::Stop()
{
    if (m_pEVRBase) {
        return m_pEVRBase->Stop();
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::Pause()
{
    if (m_pEVRBase) {
        return m_pEVRBase->Pause();
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::Run(REFERENCE_TIME tStart)
{
    if (m_pEVRBase) {
        return m_pEVRBase->Run(tStart);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::GetState(DWORD dwMilliSecsTimeout, __out  FILTER_STATE* State)
{
    HRESULT ReturnValue;
    if (m_pAllocatorPresenter->GetState(dwMilliSecsTimeout, State, ReturnValue)) {
        return ReturnValue;
    }
    if (m_pEVRBase) {
        return m_pEVRBase->GetState(dwMilliSecsTimeout, State);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::SetSyncSource(__in_opt  IReferenceClock* pClock)
{
    if (m_pEVRBase) {
        return m_pEVRBase->SetSyncSource(pClock);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::GetSyncSource(__deref_out_opt  IReferenceClock** pClock)
{
    if (m_pEVRBase) {
        return m_pEVRBase->GetSyncSource(pClock);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::GetClassID(__RPC__out CLSID* pClassID)
{
    if (m_pEVRBase) {
        return m_pEVRBase->GetClassID(pClassID);
    }
    return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms)
{
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy(pBmpParms, m_pVMR9AlphaBitmap, sizeof(VMR9AlphaBitmap));
    return S_OK;
}

STDMETHODIMP COuterEVR::SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms)
{
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy(m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
    m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
    m_pAllocatorPresenter->UpdateAlphaBitmap();
    return S_OK;
}

STDMETHODIMP COuterEVR::UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms)
{
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy(m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
    m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
    m_pAllocatorPresenter->UpdateAlphaBitmap();
    return S_OK;
}
