/*
 * (C) 2006-2012 see Authors.txt
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
#include "VMR9AllocatorPresenter.h"
#include "OuterVMR.h"

using namespace DSObjects;

STDMETHODIMP COuterVMR9::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
    if (CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR) {
        return pWC9->GetNativeVideoSize(lpWidth, lpHeight, lpARWidth, lpARHeight);
    }

    return E_NOTIMPL;
}

STDMETHODIMP COuterVMR9::GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect)
{
    if (CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR) {
        return pWC9->GetVideoPosition(lpSRCRect, lpDSTRect);
    }

    return E_NOTIMPL;
}

STDMETHODIMP COuterVMR9::GetAspectRatioMode(DWORD* lpAspectRatioMode)
{
    if (CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR) {
        *lpAspectRatioMode = VMR_ARMODE_NONE;
        return S_OK;
    }

    return E_NOTIMPL;
}

// IVideoWindow
STDMETHODIMP COuterVMR9::get_Width(long* pWidth)
{
    if (CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR) {
        CRect s, d;
        HRESULT hr = pWC9->GetVideoPosition(&s, &d);
        *pWidth = d.Width();
        return hr;
    }

    return E_NOTIMPL;
}

STDMETHODIMP COuterVMR9::get_Height(long* pHeight)
{
    if (CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR) {
        CRect s, d;
        HRESULT hr = pWC9->GetVideoPosition(&s, &d);
        *pHeight = d.Height();
        return hr;
    }

    return E_NOTIMPL;
}

// IBasicVideo2
STDMETHODIMP COuterVMR9::GetSourcePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
{
    // DVD Nav. bug workaround fix
    {
        *pLeft = *pTop = 0;
        return GetVideoSize(pWidth, pHeight);
    }
    /*
    if (CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
        CRect s, d;
        HRESULT hr = pWC9->GetVideoPosition(&s, &d);
        *pLeft = s.left;
        *pTop = s.top;
        *pWidth = s.Width();
        *pHeight = s.Height();
        return hr;
    }
    return E_NOTIMPL;
    */
}

STDMETHODIMP COuterVMR9::GetDestinationPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
{
    if (CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR) {
        CRect s, d;
        HRESULT hr = pWC9->GetVideoPosition(&s, &d);
        *pLeft = d.left;
        *pTop = d.top;
        *pWidth = d.Width();
        *pHeight = d.Height();
        return hr;
    }

    return E_NOTIMPL;
}

STDMETHODIMP COuterVMR9::GetVideoSize(long* pWidth, long* pHeight)
{
    if (CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR) {
        LONG aw, ah;
        //return pWC9->GetNativeVideoSize(pWidth, pHeight, &aw, &ah);
        // DVD Nav. bug workaround fix
        HRESULT hr = pWC9->GetNativeVideoSize(pWidth, pHeight, &aw, &ah);
        *pWidth = *pHeight * aw / ah;
        return hr;
    }

    return E_NOTIMPL;
}

STDMETHODIMP COuterVMR9::GetPreferredAspectRatio(long* plAspectX, long* plAspectY)
{
    if (CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR) {
        LONG w, h;
        return pWC9->GetNativeVideoSize(&w, &h, plAspectX, plAspectY);
    }

    return E_NOTIMPL;
}

// IVMRMixerBitmap9
STDMETHODIMP COuterVMR9::GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms)
{
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy(pBmpParms, m_pVMR9AlphaBitmap, sizeof(VMR9AlphaBitmap));
    return S_OK;
}

STDMETHODIMP COuterVMR9::SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms)
{
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy(m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
    m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
    m_pAllocatorPresenter->UpdateAlphaBitmap();
    return S_OK;
}

STDMETHODIMP COuterVMR9::UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms)
{
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy(m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
    m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
    m_pAllocatorPresenter->UpdateAlphaBitmap();
    return S_OK;
}
