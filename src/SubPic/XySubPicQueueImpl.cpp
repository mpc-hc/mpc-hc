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
#include "XySubPicQueueImpl.h"
#include "../DSUtil/DSUtil.h"

#define SUBPIC_TRACE_LEVEL 0


//
// CXySubPicQueueImpl
//

CXySubPicQueueImpl::CXySubPicQueueImpl(ISubPicAllocator* pAllocator, HRESULT* phr)
    : CUnknown(NAME("CXySubPicQueueImpl"), nullptr)
    , m_pSubRenderProvider(nullptr)
    , m_pAllocator(pAllocator)
    , m_rtNow(0)
    , m_rtNowLast(0)
    , m_fps(25.0)
{
    if (phr) {
        *phr = S_OK;
    }

    if (!m_pAllocator) {
        if (phr) {
            *phr = E_FAIL;
        }
        return;
    }
}

CXySubPicQueueImpl::~CXySubPicQueueImpl()
{
}

STDMETHODIMP CXySubPicQueueImpl::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(ISubPicQueue)
        QI(ISubRenderConsumer)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// ISubPicQueue

STDMETHODIMP CXySubPicQueueImpl::SetFPS(double fps)
{
    m_fps = fps;

    return S_OK;
}

STDMETHODIMP CXySubPicQueueImpl::SetTime(REFERENCE_TIME rtNow)
{
    m_rtNow = rtNow;

    return S_OK;
}

STDMETHODIMP CXySubPicQueueImpl::Connect(ISubRenderProvider* pSubRenderProvider)
{
    CAutoLock cAutoLock(&m_csSubRenderProvider);

    m_pSubRenderProvider = pSubRenderProvider;

    Invalidate();

    return S_OK;
}

STDMETHODIMP CXySubPicQueueImpl::Disconnect()
{
    return Connect(nullptr);
}

HRESULT CXySubPicQueueImpl::RenderTo(ISubPic* pSubPic, ISubRenderFrame* pSubFrame, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
    SubPicDesc spd;
    HRESULT hr = pSubPic->ClearDirtyRect(0xFF000000);
    if (SUCCEEDED(hr)) {
        hr = pSubPic->Lock(spd);
    }
    if (SUCCEEDED(hr)) {
        // XySubFilter only supports 32-bit bitmaps
        if (spd.bpp != 32) {
            ASSERT(FALSE);
            return E_FAIL;
        }

        ULONGLONG id;
        POINT p;
        SIZE sz;
        BYTE* pixels;
        int pitch;
        hr = pSubFrame->GetBitmap(0, &id, &p, &sz, (LPCVOID*)(&pixels), &pitch);
        if (FAILED(hr)) {
            return hr;
        }
        CRect rcDirty(p.x, p.y, p.x + sz.cx, p.y + sz.cy);

        int h = rcDirty.Height();
        BYTE* ptr = spd.bits + spd.pitch * rcDirty.top + (rcDirty.left * 4); // move pointer to dirty rect
        while (h-- > 0) { // copy the dirty rect
            memcpy(ptr, pixels, 4 * rcDirty.Width());
            ptr += spd.pitch;
            pixels += pitch;
        }

        pSubPic->SetStart(rtStart);
        pSubPic->SetStop(rtStop);

        pSubPic->Unlock(rcDirty);
    }

    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}

//
// CXySubPicQueueNoThread
//

CXySubPicQueueNoThread::CXySubPicQueueNoThread(ISubPicAllocator* pAllocator, HRESULT* phr)
    : CXySubPicQueueImpl(pAllocator, phr)
{
}

CXySubPicQueueNoThread::~CXySubPicQueueNoThread()
{
}

// ISubPicQueue

STDMETHODIMP CXySubPicQueueNoThread::Invalidate(REFERENCE_TIME rtInvalidate)
{
    CAutoLock cQueueLock(&m_csLock);

    m_pSubFrame = nullptr;
    m_pSubPic = nullptr;

    return S_OK;
}

STDMETHODIMP_(bool) CXySubPicQueueNoThread::LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& ppSubPic)
{
    CComPtr<ISubPic> pSubPic;

    {
        CAutoLock cAutoLock(&m_csLock);

        if (!m_pSubPic) {
            if (FAILED(m_pAllocator->AllocDynamic(&m_pSubPic))) {
                return false;
            }
        }

        pSubPic = m_pSubPic;
    }

    if (pSubPic->GetStart() <= rtNow && rtNow < pSubPic->GetStop()) {
        ppSubPic = pSubPic;
    } else {
        CAutoLock cAutoLock(&m_csSubRenderProvider);
        if (m_pSubRenderProvider) {
            double fps = m_fps;
            REFERENCE_TIME rtTimePerFrame = (REFERENCE_TIME)(10000000.0 / fps);

            REFERENCE_TIME rtStart = rtNow;
            REFERENCE_TIME rtStop = rtNow + rtTimePerFrame;

            HANDLE hEvtDelivered = CreateEvent(nullptr, false, false, nullptr);
            HRESULT hr = m_pSubRenderProvider->RequestFrame(rtStart, rtStop, hEvtDelivered);

            if (SUCCEEDED(hr)) {
                if (WaitForSingleObject(hEvtDelivered, (DWORD)(1000.0 / fps)) != WAIT_TIMEOUT) {
                    CAutoLock cAutoLock(&m_csLock);

                    if (m_pSubFrame) {
                        if (m_pAllocator->IsDynamicWriteOnly()) {
                            CComPtr<ISubPic> pStatic;
                            hr = m_pAllocator->GetStatic(&pStatic);
                            if (SUCCEEDED(hr)) {
                                hr = RenderTo(pStatic, m_pSubFrame, rtStart, rtStop);
                            }
                            if (SUCCEEDED(hr)) {
                                hr = pStatic->CopyTo(pSubPic);
                            }
                            if (SUCCEEDED(hr)) {
                                ppSubPic = pSubPic;
                            }
                        } else {
                            if (SUCCEEDED(RenderTo(pSubPic, m_pSubFrame, rtStart, rtStop))) {
                                ppSubPic = pSubPic;
                            }
                        }
                    }
                }
            }

            if (ppSubPic) {
                CAutoLock cAutoLock(&m_csLock);

                m_pSubPic = ppSubPic;
            }

            CloseHandle(hEvtDelivered);
        }
    }

    return !!ppSubPic;
}

STDMETHODIMP CXySubPicQueueNoThread::GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
    CAutoLock cAutoLock(&m_csLock);

    nSubPics = 0;
    rtNow = m_rtNow;
    rtStart = rtStop = 0;

    if (m_pSubFrame) {
        nSubPics = 1;
        rtStart = m_rtStart;
        rtStop = m_rtStop;
    }

    return S_OK;
}

STDMETHODIMP CXySubPicQueueNoThread::GetStats(int nSubPic, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
    CAutoLock cAutoLock(&m_csLock);

    if (!m_pSubFrame || nSubPic != 0) {
        return E_INVALIDARG;
    }

    rtStart = m_rtStart;
    rtStop = m_rtStop;

    return S_OK;
}

STDMETHODIMP CXySubPicQueueNoThread::DeliverFrame(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context, ISubRenderFrame* subtitleFrame)
{
    CAutoLock cAutoLock(&m_csLock);

    m_pSubFrame = subtitleFrame;
    m_rtStart = start;
    m_rtStop = stop;

    SetEvent((HANDLE)context);

    return S_OK;
}
