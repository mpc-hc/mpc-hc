/*
 * (C) 2006-2014, 2016 see Authors.txt
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
#include "QT9AllocatorPresenter.h"
#include "../../../DSUtil/vd.h"

using namespace DSObjects;

//
// CQT9AllocatorPresenter
//

CQT9AllocatorPresenter::CQT9AllocatorPresenter(HWND hWnd, bool bFullscreen, HRESULT& hr, CString& _Error)
    : CDX9AllocatorPresenter(hWnd, bFullscreen, hr, false, _Error)
{
}

STDMETHODIMP CQT9AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IQTVideoSurface)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CQT9AllocatorPresenter::AllocSurfaces()
{
    CheckPointer(m_pD3DDev, E_POINTER);

    HRESULT hr;

    m_pVideoSurfaceOff = nullptr;

    if (FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
                        m_nativeVideoSize.cx, m_nativeVideoSize.cy, D3DFMT_X8R8G8B8,
                        D3DPOOL_DEFAULT, &m_pVideoSurfaceOff, nullptr))) {
        return hr;
    }

    return __super::AllocSurfaces();
}

void CQT9AllocatorPresenter::DeleteSurfaces()
{
    m_pVideoSurfaceOff = nullptr;

    __super::DeleteSurfaces();
}

// IQTVideoSurface

STDMETHODIMP CQT9AllocatorPresenter::BeginBlt(const BITMAP& bm)
{
    CAutoLock cAutoLock(this);
    CAutoLock cRenderLock(&m_RenderLock);
    DeleteSurfaces();
    SetVideoSize(CSize(bm.bmWidth, abs(bm.bmHeight)));
    if (FAILED(AllocSurfaces())) {
        return E_FAIL;
    }
    return S_OK;
}

STDMETHODIMP CQT9AllocatorPresenter::DoBlt(const BITMAP& bm)
{
    if (!m_pVideoSurface[m_nCurSurface] || !m_pVideoSurfaceOff) {
        return E_FAIL;
    }

    bool fOk = false;

    D3DSURFACE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    if (FAILED(m_pVideoSurfaceOff->GetDesc(&desc))) {
        return E_FAIL;
    }

    UINT w = (UINT)bm.bmWidth;
    UINT h = abs(bm.bmHeight);
    int bpp = bm.bmBitsPixel;
    int dbpp =
        desc.Format == D3DFMT_R8G8B8 || desc.Format == D3DFMT_X8R8G8B8 || desc.Format == D3DFMT_A8R8G8B8 ? 32 :
        desc.Format == D3DFMT_R5G6B5 ? 16 : 0;

    if ((bpp == 16 || bpp == 24 || bpp == 32) && w == desc.Width && h == desc.Height) {
        D3DLOCKED_RECT r;
        if (SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, nullptr, 0))) {
            BitBltFromRGBToRGB(
                w, h,
                (BYTE*)r.pBits, r.Pitch, dbpp,
                (BYTE*)bm.bmBits, bm.bmWidthBytes, bm.bmBitsPixel);
            m_pVideoSurfaceOff->UnlockRect();
            fOk = true;
        }
    }

    if (!fOk) {
        m_pD3DDev->ColorFill(m_pVideoSurfaceOff, nullptr, 0);

        HDC hDC;
        if (SUCCEEDED(m_pVideoSurfaceOff->GetDC(&hDC))) {
            CString str;
            str.Format(_T("Sorry, this color format is not supported"));

            SetBkColor(hDC, 0);
            SetTextColor(hDC, 0x404040);
            TextOut(hDC, 10, 10, str, str.GetLength());

            m_pVideoSurfaceOff->ReleaseDC(hDC);
        }
    }

    m_pD3DDev->StretchRect(m_pVideoSurfaceOff, nullptr, m_pVideoSurface[m_nCurSurface], nullptr, D3DTEXF_NONE);

    Paint(true);

    return S_OK;
}
