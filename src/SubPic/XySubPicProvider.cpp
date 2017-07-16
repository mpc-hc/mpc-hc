/*
 * (C) 2003-2006 Gabest
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
#include "XySubPicProvider.h"

CXySubPicProvider::CXySubPicProvider(ISubRenderProvider* provider)
    : CUnknown(NAME("CXySubPicProvider"), nullptr)
    , m_pSubRenderProvider(provider)
    , m_pSubFrame(nullptr)
    , m_rtStart(0)
    , m_rtStop(0)
{
    m_hEvtDelivered = CreateEvent(nullptr, false, false, nullptr);
}

CXySubPicProvider::~CXySubPicProvider()
{
    CloseHandle(m_hEvtDelivered);
}

STDMETHODIMP CXySubPicProvider::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(ISubPicProvider)
        QI(IXyCompatProvider)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// IXyCompatProvider

STDMETHODIMP CXySubPicProvider::DeliverFrame(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context, ISubRenderFrame* subtitleFrame)
{
    m_rtStart = start;
    m_rtStop = stop;
    m_pSubFrame = subtitleFrame;

    SetEvent(m_hEvtDelivered);
    return S_OK;
}

STDMETHODIMP CXySubPicProvider::RequestFrame(REFERENCE_TIME start, REFERENCE_TIME stop, DWORD timeout)
{
    HRESULT hr = E_FAIL;

    if (FAILED(hr = m_pSubRenderProvider->RequestFrame(start, stop, nullptr))) {
        return hr;
    }

    if (WaitForSingleObject(m_hEvtDelivered, timeout) == WAIT_TIMEOUT) {
        return E_ABORT;
    }

    return S_OK;
}

STDMETHODIMP CXySubPicProvider::GetID(ULONGLONG* id)
{
    return m_pSubFrame ? m_pSubFrame->GetBitmap(0, id, nullptr, nullptr, nullptr, nullptr) : E_FAIL;
}

// ISubPicProvider

STDMETHODIMP CXySubPicProvider::Lock()
{
    m_csSubRenderProvider.Lock();
    return S_OK;
}

STDMETHODIMP CXySubPicProvider::Unlock()
{
    m_csSubRenderProvider.Unlock();
    return S_OK;
}

STDMETHODIMP CXySubPicProvider::Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox)
{
    if (spd.bpp != 32) {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

    if (!m_pSubFrame || (m_rtStart > rt || rt >= m_rtStop)) {
        return E_FAIL;
    }

    POINT p;
    SIZE sz;
    BYTE* s;
    int pitch;
    HRESULT hr = m_pSubFrame->GetBitmap(0, nullptr, &p, &sz, (LPCVOID*)(&s), &pitch);
    if (FAILED(hr)) {
        return hr;
    }

    CRect rcDirty;
    rcDirty.IntersectRect(CRect(p, sz), CRect(0, 0, spd.w, spd.h));
    int w = rcDirty.Width(), h = rcDirty.Height();
    BYTE* d = spd.bits + spd.pitch * rcDirty.top + rcDirty.left * 4; // move pointer to dirty rect
    for (ptrdiff_t j = 0; j < h; j++, s += pitch, d += spd.pitch) {
        memcpy(d, s, w * 4);
    }

    bbox = rcDirty;
    return S_OK;
}

STDMETHODIMP CXySubPicProvider::GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft)
{
    CRect outputRect, clipRect;
    if (m_pSubFrame && SUCCEEDED(m_pSubFrame->GetOutputRect(&outputRect)) && SUCCEEDED(m_pSubFrame->GetClipRect(&clipRect))) {
        MaxTextureSize = outputRect.Size();
        VirtualSize = clipRect.Size();
        VirtualTopLeft = clipRect.TopLeft();
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP CXySubPicProvider::GetRelativeTo(POSITION pos, RelativeTo& relativeTo)
{
    relativeTo = BEST_FIT;
    return S_OK;
}
