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
#include <algorithm>
#include <intsafe.h>
#include "SubPicQueueImpl.h"
#include "../DSUtil/DSUtil.h"

#define SUBPIC_TRACE_LEVEL 0


//
// CSubPicQueueImpl
//

CSubPicQueueImpl::CSubPicQueueImpl(ISubPicAllocator* pAllocator, HRESULT* phr)
    : CUnknown(NAME("CSubPicQueueImpl"), nullptr)
    , m_pAllocator(pAllocator)
    , m_rtNow(0)
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

CSubPicQueueImpl::~CSubPicQueueImpl()
{
}

STDMETHODIMP CSubPicQueueImpl::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(ISubPicQueue)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// ISubPicQueue

STDMETHODIMP CSubPicQueueImpl::SetSubPicProvider(ISubPicProvider* pSubPicProvider)
{
    CAutoLock cAutoLock(&m_csSubPicProvider);

    m_pSubPicProvider = pSubPicProvider;

    Invalidate();

    return S_OK;
}

STDMETHODIMP CSubPicQueueImpl::GetSubPicProvider(ISubPicProvider** pSubPicProvider)
{
    CheckPointer(pSubPicProvider, E_POINTER);

    CAutoLock cAutoLock(&m_csSubPicProvider);

    if (m_pSubPicProvider) {
        *pSubPicProvider = m_pSubPicProvider;
        (*pSubPicProvider)->AddRef();
    }

    return *pSubPicProvider ? S_OK : E_FAIL;
}

STDMETHODIMP CSubPicQueueImpl::SetFPS(double fps)
{
    m_fps = fps;

    return S_OK;
}

STDMETHODIMP CSubPicQueueImpl::SetTime(REFERENCE_TIME rtNow)
{
    m_rtNow = rtNow;

    return S_OK;
}

// private

HRESULT CSubPicQueueImpl::RenderTo(ISubPic* pSubPic, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, double fps, BOOL bIsAnimated)
{
    CheckPointer(pSubPic, E_POINTER);

    HRESULT hr = E_FAIL;
    CComPtr<ISubPicProvider> pSubPicProvider;
    if (FAILED(GetSubPicProvider(&pSubPicProvider)) || !pSubPicProvider) {
        return hr;
    }

    if (pSubPic->GetInverseAlpha()) {
        hr = pSubPic->ClearDirtyRect(0x00000000);
    } else {
        hr = pSubPic->ClearDirtyRect(0xFF000000);
    }

    SubPicDesc spd;
    if (SUCCEEDED(hr)) {
        hr = pSubPic->Lock(spd);
    }
    if (SUCCEEDED(hr)) {
        CRect r(0, 0, 0, 0);
        hr = pSubPicProvider->Render(spd, bIsAnimated ? rtStart : ((rtStart + rtStop) / 2), fps, r);

        pSubPic->SetStart(rtStart);
        pSubPic->SetStop(rtStop);

        pSubPic->Unlock(r);
    }

    return hr;
}

//
// CSubPicQueue
//

#pragma warning(push)
#pragma warning(disable: 4351) // new behavior: elements of array 'array' will be default initialized
CSubPicQueue::CSubPicQueue(int nMaxSubPic, bool bDisableAnim, ISubPicAllocator* pAllocator, HRESULT* phr)
    : CSubPicQueueImpl(pAllocator, phr)
    , m_nMaxSubPic(nMaxSubPic)
    , m_bDisableAnim(bDisableAnim)
    , m_rtNowLast(0)
    , m_rtQueueMin(0)
    , m_rtQueueMax(0)
    , m_rtInvalidate(0)
    , m_bBreakBuffering(false)
    , m_hThreadEvents()
{
    if (phr && FAILED(*phr)) {
        return;
    }

    if (m_nMaxSubPic < 1) {
        if (phr) {
            *phr = E_INVALIDARG;
        }
        return;
    }

    m_bBreakBuffering = false;
    for (ptrdiff_t i = 0; i < EVENT_COUNT; i++) {
        m_hThreadEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }
    CAMThread::Create();
}
#pragma warning(pop)

CSubPicQueue::~CSubPicQueue()
{
    m_bBreakBuffering = true;
    SetEvent(m_hThreadEvents[EVENT_EXIT]);
    CAMThread::Close();
    for (ptrdiff_t i = 0; i < EVENT_COUNT; i++) {
        CloseHandle(m_hThreadEvents[i]);
    }
}

// ISubPicQueue

STDMETHODIMP CSubPicQueue::SetFPS(double fps)
{
    HRESULT hr = __super::SetFPS(fps);
    if (FAILED(hr)) {
        return hr;
    }

    SetEvent(m_hThreadEvents[EVENT_TIME]);

    return S_OK;
}

STDMETHODIMP CSubPicQueue::SetTime(REFERENCE_TIME rtNow)
{
    HRESULT hr = __super::SetTime(rtNow);
    if (FAILED(hr)) {
        return hr;
    }

    SetEvent(m_hThreadEvents[EVENT_TIME]);

    return S_OK;
}

STDMETHODIMP CSubPicQueue::Invalidate(REFERENCE_TIME rtInvalidate /*= -1*/)
{
    // CAutoLock cQueueLock(&m_csQueueLock);

    m_rtInvalidate = rtInvalidate;
    m_bBreakBuffering = true;
#if SUBPIC_TRACE_LEVEL > 0
    TRACE(_T("Invalidate: %f\n"), double(rtInvalidate) / 10000000.0);
#endif

    SetEvent(m_hThreadEvents[EVENT_TIME]);

    return S_OK;
}

STDMETHODIMP_(bool) CSubPicQueue::LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& ppSubPic)
{
    CAutoLock cQueueLock(&m_csQueueLock);

    REFERENCE_TIME rtBestStop = LONGLONG_MAX;
    POSITION pos = m_queue.GetHeadPosition();
#if SUBPIC_TRACE_LEVEL > 2
    TRACE(_T("Find: "));
#endif
    while (pos) {
        CComPtr<ISubPic> pSubPic = m_queue.GetNext(pos);
        REFERENCE_TIME rtStart = pSubPic->GetStart();
        REFERENCE_TIME rtStop = pSubPic->GetStop();
        REFERENCE_TIME rtSegmentStop = pSubPic->GetSegmentStop();
        if (rtNow >= rtStart && rtNow < rtSegmentStop) {
            REFERENCE_TIME diff = rtNow - rtStop;
            if (diff < rtBestStop) {
                rtBestStop = diff;
                ppSubPic = pSubPic;
            }
#if SUBPIC_TRACE_LEVEL > 2
            else {
                TRACE(_T("   !%f->%f"), double(diff) / 10000000.0, double(rtStop) / 10000000.0);
            }
#endif
        }
#if SUBPIC_TRACE_LEVEL > 2
        else {
            TRACE(_T("   !!%f->%f"), double(rtStart) / 10000000.0, double(rtSegmentStop) / 10000000.0);
        }
#endif
    }
#if SUBPIC_TRACE_LEVEL > 2
    TRACE(_T("\n"));
#endif
    if (!ppSubPic) {
#if SUBPIC_TRACE_LEVEL > 1
        TRACE(_T("NO Display: %f\n"), double(rtNow) / 10000000.0);
#endif
    } else {
#if SUBPIC_TRACE_LEVEL > 0
        REFERENCE_TIME rtStart = (ppSubPic)->GetStart();
        REFERENCE_TIME rtSegmentStop = (ppSubPic)->GetSegmentStop();
        CRect r;
        (ppSubPic)->GetDirtyRect(&r);
        TRACE(_T("Display: %f->%f   %f    %dx%d\n"), double(rtStart) / 10000000.0, double(rtSegmentStop) / 10000000.0, double(rtNow) / 10000000.0, r.Width(), r.Height());
#endif
    }

    return !!ppSubPic;
}

STDMETHODIMP CSubPicQueue::GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
    CAutoLock cQueueLock(&m_csQueueLock);

    nSubPics = (int)m_queue.GetCount();
    rtNow = m_rtNow;
    rtStart = m_rtQueueMin;
    if (rtStart == LONGLONG_MAX) {
        rtStart = 0;
    }
    rtStop = m_rtQueueMax;
    if (rtStop == LONGLONG_ERROR) {
        rtStop = 0;
    }

    return S_OK;
}

STDMETHODIMP CSubPicQueue::GetStats(int nSubPic, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
    CAutoLock cQueueLock(&m_csQueueLock);

    rtStart = rtStop = -1;

    if (nSubPic >= 0 && nSubPic < (int)m_queue.GetCount()) {
        if (POSITION pos = m_queue.FindIndex(nSubPic)) {
            rtStart = m_queue.GetAt(pos)->GetStart();
            rtStop = m_queue.GetAt(pos)->GetStop();
        }
    } else {
        return E_INVALIDARG;
    }

    return S_OK;
}

// private

REFERENCE_TIME CSubPicQueue::UpdateQueue()
{
    CAutoLock cQueueLock(&m_csQueueLock);

    REFERENCE_TIME rtNow = m_rtNow;
    REFERENCE_TIME rtNowCompare = rtNow;

    if (rtNow < m_rtNowLast) {
        m_queue.RemoveAll();
        m_rtNowLast = rtNow;
    } else {
        m_rtNowLast = rtNow;

        m_rtQueueMin = LONGLONG_MAX;
        m_rtQueueMax = LONGLONG_ERROR;

        POSITION savePos = nullptr;
        {
            POSITION pos = m_queue.GetHeadPosition();
            REFERENCE_TIME rtBestStop = LONGLONG_MAX;
            while (pos) {
                POSITION thisPos = pos;
                ISubPic* pSubPic = m_queue.GetNext(pos);
                REFERENCE_TIME rtStart = pSubPic->GetStart();
                REFERENCE_TIME rtStop = pSubPic->GetStop();
                REFERENCE_TIME rtSegmentStop = pSubPic->GetSegmentStop();
                if (rtNow >= rtStart && rtNow < rtSegmentStop) {
                    REFERENCE_TIME diff = rtNow - rtStop;
                    if (diff < rtBestStop) {
                        rtBestStop = diff;
                        savePos = thisPos;
                    }
                }
            }
        }

#if SUBPIC_TRACE_LEVEL > 3
        if (savePos) {
            ISubPic* pSubPic = m_queue.GetAt(savePos);
            REFERENCE_TIME rtStart = pSubPic->GetStart();
            REFERENCE_TIME rtStop = pSubPic->GetStop();
            TRACE(_T("Save: %f->%f\n"), double(rtStart) / 10000000.0, double(rtStop) / 10000000.0);
        }
#endif
        {
            POSITION pos = m_queue.GetHeadPosition();
            while (pos) {
                POSITION thisPos = pos;
                ISubPic* pSubPic = m_queue.GetNext(pos);

                REFERENCE_TIME rtStart = pSubPic->GetStart();
                REFERENCE_TIME rtStop = pSubPic->GetStop();

                if (rtStop <= rtNowCompare && thisPos != savePos) {
#if SUBPIC_TRACE_LEVEL > 0
                    TRACE(_T("Remove: %f->%f\n"), double(rtStart) / 10000000.0, double(rtStop) / 10000000.0);
#endif
                    m_queue.RemoveAt(thisPos);
                    continue;
                }
                if (rtStop > rtNow) {
                    rtNow = rtStop;
                }
                m_rtQueueMin = std::min(m_rtQueueMin, rtStart);
                m_rtQueueMax = std::max(m_rtQueueMax, rtStop);
            }
        }
    }

    return rtNow;
}

int CSubPicQueue::GetQueueCount()
{
    CAutoLock cQueueLock(&m_csQueueLock);

    return (int)m_queue.GetCount();
}

void CSubPicQueue::AppendQueue(ISubPic* pSubPic)
{
    CAutoLock cQueueLock(&m_csQueueLock);

    m_queue.AddTail(pSubPic);
}

// overrides

DWORD CSubPicQueue::ThreadProc()
{
    bool bDisableAnim = m_bDisableAnim;
    SetThreadName(DWORD(-1), "Subtitle Renderer Thread");
    SetThreadPriority(m_hThread, bDisableAnim ? THREAD_PRIORITY_LOWEST : THREAD_PRIORITY_ABOVE_NORMAL);

    bool bAgain = true;
    for (;;) {
        DWORD ret = WaitForMultipleObjects(EVENT_COUNT, m_hThreadEvents, FALSE, bAgain ? 0 : INFINITE);
        bAgain = false;

        if (ret == WAIT_TIMEOUT) {
            ;
        } else if ((ret - WAIT_OBJECT_0) != EVENT_TIME) {
            break;
        }
        double fps = m_fps;
        REFERENCE_TIME rtTimePerFrame = (REFERENCE_TIME)(10000000.0 / fps);
        REFERENCE_TIME rtNow = UpdateQueue();

        int nMaxSubPic = m_nMaxSubPic;

        CComPtr<ISubPicProvider> pSubPicProvider;
        if (SUCCEEDED(GetSubPicProvider(&pSubPicProvider)) && pSubPicProvider
                && SUCCEEDED(pSubPicProvider->Lock())) {
            for (POSITION pos = pSubPicProvider->GetStartPosition(rtNow, fps);
                    pos && !m_bBreakBuffering && GetQueueCount() < nMaxSubPic;
                    pos = pSubPicProvider->GetNext(pos)) {
                REFERENCE_TIME rtStart = pSubPicProvider->GetStart(pos, fps);
                REFERENCE_TIME rtStop = pSubPicProvider->GetStop(pos, fps);

                if (m_rtNow >= rtStart) {
                    if (m_rtNow >= rtStop) {
                        continue;
                    }
                }

                // we are already one minute ahead, this should be enough
                if (rtStart >= m_rtNow + 60 * 10000000i64) {
                    break;
                }

                if (rtNow < rtStop) {
                    REFERENCE_TIME rtCurrent = std::max(rtNow, rtStart);
                    bool bIsAnimated = pSubPicProvider->IsAnimated(pos) && !bDisableAnim;
                    while (rtCurrent < rtStop) {
                        SIZE    maxTextureSize, virtualSize;
                        POINT   virtualTopLeft;
                        HRESULT hr2;

                        if (SUCCEEDED(hr2 = pSubPicProvider->GetTextureSize(pos, maxTextureSize, virtualSize, virtualTopLeft))) {
                            m_pAllocator->SetMaxTextureSize(maxTextureSize);
                        }

                        CComPtr<ISubPic> pStatic;
                        if (FAILED(m_pAllocator->GetStatic(&pStatic))) {
                            break;
                        }

                        HRESULT hr;
                        if (bIsAnimated) {
                            REFERENCE_TIME rtEndThis = std::min(rtCurrent + rtTimePerFrame, rtStop);
                            hr = RenderTo(pStatic, rtCurrent, rtEndThis, fps, bIsAnimated);
                            pStatic->SetSegmentStart(rtStart);
                            pStatic->SetSegmentStop(rtStop);
#if SUBPIC_TRACE_LEVEL > 0
                            CRect r;
                            pStatic->GetDirtyRect(&r);
                            TRACE(_T("Render: %f->%f    %f->%f      %dx%d\n"), double(rtCurrent) / 10000000.0, double(rtEndThis) / 10000000.0, double(rtStart) / 10000000.0, double(rtStop) / 10000000.0, r.Width(), r.Height());
#endif
                            rtCurrent = rtEndThis;
                        } else {
                            hr = RenderTo(pStatic, rtStart, rtStop, fps, bIsAnimated);
                            // Non-animated subtitles aren't part of a segment
                            pStatic->SetSegmentStart(0);
                            pStatic->SetSegmentStop(0);
                            rtCurrent = rtStop;
                        }
#if SUBPIC_TRACE_LEVEL > 0
                        if (m_rtNow > rtCurrent) {
                            TRACE(_T("BEHIND\n"));
                        }
#endif

                        if (FAILED(hr)) {
                            break;
                        }

                        if (S_OK != hr) { // subpic was probably empty
                            continue;
                        }

                        CComPtr<ISubPic> pDynamic;
                        if (FAILED(m_pAllocator->AllocDynamic(&pDynamic))
                                || FAILED(pStatic->CopyTo(pDynamic))) {
                            break;
                        }

                        if (SUCCEEDED(hr2)) {
                            pDynamic->SetVirtualTextureSize(virtualSize, virtualTopLeft);
                        }

                        RelativeTo relativeTo;
                        if (SUCCEEDED(pSubPicProvider->GetRelativeTo(pos, relativeTo))) {
                            pDynamic->SetRelativeTo(relativeTo);
                        }

                        AppendQueue(pDynamic);
                        bAgain = true;

                        if (GetQueueCount() >= nMaxSubPic) {
                            break;
                        }
                    }
                }
            }

            pSubPicProvider->Unlock();
        }

        if (m_bBreakBuffering) {
            bAgain = true;
            CAutoLock cQueueLock(&m_csQueueLock);

            REFERENCE_TIME rtInvalidate = m_rtInvalidate;

            POSITION pos = m_queue.GetHeadPosition();
            while (pos) {
                POSITION thisPos = pos;
                ISubPic* pSubPic = m_queue.GetNext(pos);

                REFERENCE_TIME rtStart = pSubPic->GetStart();
                REFERENCE_TIME rtStop = pSubPic->GetStop();
                REFERENCE_TIME rtSegmentStop = pSubPic->GetSegmentStop();

                if (rtSegmentStop > rtInvalidate) {
#if SUBPIC_TRACE_LEVEL >= 0
                    TRACE(_T("Removed subtitle because of invalidation: %f -> %f (%f)\n"),
                          double(rtStart) / 10000000.0, double(rtStop) / 10000000.0, double(rtSegmentStop) / 10000000.0);
#endif
                    m_queue.RemoveAt(thisPos);
                    continue;
                }
            }

            m_bBreakBuffering = false;
        }
    }

    return 0;
}

//
// CSubPicQueueNoThread
//

CSubPicQueueNoThread::CSubPicQueueNoThread(ISubPicAllocator* pAllocator, HRESULT* phr)
    : CSubPicQueueImpl(pAllocator, phr)
{
}

CSubPicQueueNoThread::~CSubPicQueueNoThread()
{
}

// ISubPicQueue

STDMETHODIMP CSubPicQueueNoThread::Invalidate(REFERENCE_TIME rtInvalidate /*= -1*/)
{
    CAutoLock cQueueLock(&m_csLock);

    if (m_pSubPic && m_pSubPic->GetStop() > rtInvalidate) {
        m_pSubPic = nullptr;
    }

    return S_OK;
}

STDMETHODIMP_(bool) CSubPicQueueNoThread::LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& ppSubPic)
{
    CComPtr<ISubPic> pSubPic;

    {
        CAutoLock cAutoLock(&m_csLock);

        pSubPic = m_pSubPic;
    }

    if (pSubPic && pSubPic->GetStart() <= rtNow && rtNow < pSubPic->GetStop()) {
        ppSubPic = pSubPic;
    } else {
        CComPtr<ISubPicProvider> pSubPicProvider;
        if (SUCCEEDED(GetSubPicProvider(&pSubPicProvider)) && pSubPicProvider
                && SUCCEEDED(pSubPicProvider->Lock())) {
            double fps = m_fps;
            POSITION pos = pSubPicProvider->GetStartPosition(rtNow, fps);
            if (pos) {
                REFERENCE_TIME rtStart;
                REFERENCE_TIME rtStop;

                if (pSubPicProvider->IsAnimated(pos)) {
                    rtStart = rtNow;
                    rtStop = rtNow + 1;
                } else {
                    rtStart = pSubPicProvider->GetStart(pos, fps);
                    rtStop = pSubPicProvider->GetStop(pos, fps);
                }

                if (rtStart <= rtNow && rtNow < rtStop) {
                    bool    bAllocSubPic = !pSubPic;
                    SIZE    maxTextureSize, virtualSize;
                    POINT   virtualTopLeft;
                    HRESULT hr;
                    if (SUCCEEDED(hr = pSubPicProvider->GetTextureSize(pos, maxTextureSize, virtualSize, virtualTopLeft))) {
                        m_pAllocator->SetMaxTextureSize(maxTextureSize);
                        if (!bAllocSubPic) {
                            // Ensure the previously allocated subpic is big enough to hold the subtitle to be rendered
                            SIZE maxSize;
                            bAllocSubPic = FAILED(pSubPic->GetMaxSize(&maxSize)) || maxSize.cx < maxTextureSize.cx || maxSize.cy < maxTextureSize.cy;
                        }
                    }

                    if (bAllocSubPic) {
                        CAutoLock cAutoLock(&m_csLock);

                        m_pSubPic.Release();

                        if (FAILED(m_pAllocator->AllocDynamic(&m_pSubPic))) {
                            return false;
                        }

                        pSubPic = m_pSubPic;
                    }

                    if (m_pAllocator->IsDynamicWriteOnly()) {
                        CComPtr<ISubPic> pStatic;
                        if (SUCCEEDED(m_pAllocator->GetStatic(&pStatic))
                                && SUCCEEDED(RenderTo(pStatic, rtStart, rtStop, fps, false))
                                && SUCCEEDED(pStatic->CopyTo(pSubPic))) {
                            ppSubPic = pSubPic;
                        }
                    } else {
                        if (SUCCEEDED(RenderTo(pSubPic, rtStart, rtStop, fps, false))) {
                            ppSubPic = pSubPic;
                        }
                    }

                    if (ppSubPic) {
                        if (SUCCEEDED(hr)) {
                            ppSubPic->SetVirtualTextureSize(virtualSize, virtualTopLeft);
                        }

                        RelativeTo relativeTo;
                        if (SUCCEEDED(pSubPicProvider->GetRelativeTo(pos, relativeTo))) {
                            ppSubPic->SetRelativeTo(relativeTo);
                        }
                    }
                }
            }

            pSubPicProvider->Unlock();
        }
    }

    return !!ppSubPic;
}

STDMETHODIMP CSubPicQueueNoThread::GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
    CAutoLock cAutoLock(&m_csLock);

    rtNow = m_rtNow;

    if (m_pSubPic) {
        nSubPics = 1;
        rtStart = m_pSubPic->GetStart();
        rtStop = m_pSubPic->GetStop();
    } else {
        nSubPics = 0;
        rtStart = rtStop = 0;
    }

    return S_OK;
}

STDMETHODIMP CSubPicQueueNoThread::GetStats(int nSubPic, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
    CAutoLock cAutoLock(&m_csLock);

    if (!m_pSubPic || nSubPic != 0) {
        return E_INVALIDARG;
    }

    rtStart = m_pSubPic->GetStart();
    rtStop = m_pSubPic->GetStop();

    return S_OK;
}
