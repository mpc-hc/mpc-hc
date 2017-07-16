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
#include "RM9AllocatorPresenter.h"
#include "../../../DSUtil/vd.h"

using namespace DSObjects;

//
// CRM9AllocatorPresenter
//

CRM9AllocatorPresenter::CRM9AllocatorPresenter(HWND hWnd, bool bFullscreen, HRESULT& hr, CString& _Error)
    : CDX9AllocatorPresenter(hWnd, bFullscreen, hr, false, _Error)
{
    ZeroMemory(&m_bitmapInfo, sizeof(m_bitmapInfo));
    ZeroMemory(&m_lastBitmapInfo, sizeof(m_lastBitmapInfo));
}

STDMETHODIMP CRM9AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI2(IRMAVideoSurface)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CRM9AllocatorPresenter::AllocSurfaces()
{
    CAutoLock cAutoLock(this);
    CAutoLock cRenderLock(&m_RenderLock);

    CheckPointer(m_pD3DDev, E_POINTER);

    m_pVideoSurfaceOff  = nullptr;
    m_pVideoSurfaceYUY2 = nullptr;

    HRESULT hr;

    if (FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
                        m_nativeVideoSize.cx, m_nativeVideoSize.cy, D3DFMT_X8R8G8B8,
                        D3DPOOL_DEFAULT, &m_pVideoSurfaceOff, nullptr))) {
        return hr;
    }

    m_pD3DDev->ColorFill(m_pVideoSurfaceOff, nullptr, 0);

    if (FAILED(m_pD3DDev->CreateOffscreenPlainSurface(
                   m_nativeVideoSize.cx, m_nativeVideoSize.cy, D3DFMT_YUY2,
                   D3DPOOL_DEFAULT, &m_pVideoSurfaceYUY2, nullptr))) {
        m_pVideoSurfaceYUY2 = nullptr;
    }

    if (m_pVideoSurfaceYUY2) {
        m_pD3DDev->ColorFill(m_pVideoSurfaceOff, nullptr, 0x80108010);
    }

    return __super::AllocSurfaces();
}

void CRM9AllocatorPresenter::DeleteSurfaces()
{
    CAutoLock cAutoLock(this);
    CAutoLock cRenderLock(&m_RenderLock);
    m_pVideoSurfaceOff = nullptr;
    m_pVideoSurfaceYUY2 = nullptr;
    __super::DeleteSurfaces();
}

// IRMAVideoSurface

STDMETHODIMP CRM9AllocatorPresenter::Blt(UCHAR* pImageData, RMABitmapInfoHeader* pBitmapInfo, REF(PNxRect) inDestRect, REF(PNxRect) inSrcRect)
{
    if (!m_pVideoSurface[m_nCurSurface] || !m_pVideoSurfaceOff) {
        return E_FAIL;
    }

    bool fRGB = false;
    bool fYUY2 = false;

    CRect src((RECT*)&inSrcRect), dst((RECT*)&inDestRect), src2(CPoint(0, 0), src.Size());
    if (src.Width() > dst.Width() || src.Height() > dst.Height()) {
        return E_FAIL;
    }

    D3DSURFACE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    if (FAILED(m_pVideoSurfaceOff->GetDesc(&desc))) {
        return E_FAIL;
    }

    int dbpp =
        desc.Format == D3DFMT_R8G8B8 || desc.Format == D3DFMT_X8R8G8B8 || desc.Format == D3DFMT_A8R8G8B8 ? 32 :
        desc.Format == D3DFMT_R5G6B5 ? 16 : 0;

    if (pBitmapInfo->biCompression == '024I') {
        DWORD pitch = pBitmapInfo->biWidth;
        DWORD size = pitch * abs(pBitmapInfo->biHeight);

        BYTE* y = pImageData                   + src.top * pitch + src.left;
        BYTE* u = pImageData + size            + src.top * (pitch / 2) + src.left / 2;
        BYTE* v = pImageData + size + size / 4 + src.top * (pitch / 2) + src.left / 2;

        if (m_pVideoSurfaceYUY2) {
            D3DLOCKED_RECT r;
            if (SUCCEEDED(m_pVideoSurfaceYUY2->LockRect(&r, src2, 0))) {
                BitBltFromI420ToYUY2(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, y, u, v, pitch);
                m_pVideoSurfaceYUY2->UnlockRect();
                fYUY2 = true;
            }
        } else {
            D3DLOCKED_RECT r;
            if (SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, src2, 0))) {
                BitBltFromI420ToRGB(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, dbpp, y, u, v, pitch);
                m_pVideoSurfaceOff->UnlockRect();
                fRGB = true;
            }
        }
    } else if (pBitmapInfo->biCompression == '2YUY') {
        DWORD w = pBitmapInfo->biWidth;
        DWORD h = abs(pBitmapInfo->biHeight);
        DWORD pitch = pBitmapInfo->biWidth * 2;
        UNREFERENCED_PARAMETER(w);
        UNREFERENCED_PARAMETER(h);

        BYTE* yvyu = pImageData + src.top * pitch + src.left * 2;

        if (m_pVideoSurfaceYUY2) {
            D3DLOCKED_RECT r;
            if (SUCCEEDED(m_pVideoSurfaceYUY2->LockRect(&r, src2, 0))) {
                BitBltFromYUY2ToYUY2(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, yvyu, pitch);
                m_pVideoSurfaceYUY2->UnlockRect();
                fYUY2 = true;
            }
        } else {
            D3DLOCKED_RECT r;
            if (SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, src2, 0))) {
                BitBltFromYUY2ToRGB(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, dbpp, yvyu, pitch);
                m_pVideoSurfaceOff->UnlockRect();
                fRGB = true;
            }
        }
    } else if (pBitmapInfo->biCompression == 0 || pBitmapInfo->biCompression == 3
               || pBitmapInfo->biCompression == 'BGRA') {
        DWORD w = pBitmapInfo->biWidth;
        DWORD h = abs(pBitmapInfo->biHeight);
        DWORD pitch = pBitmapInfo->biWidth * pBitmapInfo->biBitCount >> 3;
        UNREFERENCED_PARAMETER(w);
        UNREFERENCED_PARAMETER(h);

        BYTE* rgb = pImageData + src.top * pitch + src.left * (pBitmapInfo->biBitCount >> 3);

        D3DLOCKED_RECT r;
        if (SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, src2, 0))) {
            BYTE* pBits = (BYTE*)r.pBits;
            if (pBitmapInfo->biHeight > 0) {
                pBits += r.Pitch * (src.Height() - 1);
                r.Pitch = -r.Pitch;
            }
            BitBltFromRGBToRGB(src.Width(), src.Height(), pBits, r.Pitch, dbpp, rgb, pitch, pBitmapInfo->biBitCount);
            m_pVideoSurfaceOff->UnlockRect();
            fRGB = true;
        }
    }

    if (!fRGB && !fYUY2) {
        m_pD3DDev->ColorFill(m_pVideoSurfaceOff, nullptr, 0);

        HDC hDC;
        if (SUCCEEDED(m_pVideoSurfaceOff->GetDC(&hDC))) {
            CString str;
            str.Format(_T("Sorry, this format is not supported"));

            SetBkColor(hDC, 0);
            SetTextColor(hDC, 0x404040);
            TextOut(hDC, 10, 10, str, str.GetLength());

            m_pVideoSurfaceOff->ReleaseDC(hDC);

            fRGB = true;
        }
    }

    if (fRGB) {
        m_pD3DDev->StretchRect(m_pVideoSurfaceOff, src2, m_pVideoSurface[m_nCurSurface], dst, D3DTEXF_NONE);
    }
    if (fYUY2) {
        m_pD3DDev->StretchRect(m_pVideoSurfaceYUY2, src2, m_pVideoSurface[m_nCurSurface], dst, D3DTEXF_NONE);
    }

    Paint(true);

    return PNR_OK;
}

STDMETHODIMP CRM9AllocatorPresenter::BeginOptimizedBlt(RMABitmapInfoHeader* pBitmapInfo)
{
    CAutoLock cAutoLock(this);
    CAutoLock cRenderLock(&m_RenderLock);
    DeleteSurfaces();
    SetVideoSize(CSize(pBitmapInfo->biWidth, abs(pBitmapInfo->biHeight)));
    if (FAILED(AllocSurfaces())) {
        return E_FAIL;
    }
    return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::OptimizedBlt(UCHAR* pImageBits, REF(PNxRect) rDestRect, REF(PNxRect) rSrcRect)
{
    return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::EndOptimizedBlt()
{
    return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
    return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
    ulType = RMA_I420;
    return PNR_OK;
}
