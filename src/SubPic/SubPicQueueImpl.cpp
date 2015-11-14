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
#include <algorithm>
#include <intsafe.h>
#include "SubPicQueueImpl.h"
#include "../DSUtil/DSUtil.h"

#define SUBPIC_TRACE_LEVEL 0


//
// CSubPicQueueImpl
//

const double CSubPicQueueImpl::DEFAULT_FPS = 25.0;

CSubPicQueueImpl::CSubPicQueueImpl(SubPicQueueSettings settings, ISubPicAllocator* pAllocator, HRESULT* phr)
    : CUnknown(NAME("CSubPicQueueImpl"), nullptr)
    , m_fps(DEFAULT_FPS)
    , m_rtTimePerFrame(std::llround(10000000.0 / DEFAULT_FPS))
    , m_rtTimePerSubFrame(std::llround(10000000.0 / (DEFAULT_FPS * settings.nAnimationRate / 100.0)))
    , m_rtNow(0)
    , m_settings(settings)
    , m_pAllocator(pAllocator)
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

    m_pSubPicProviderWithSharedLock = std::make_shared<SubPicProviderWithSharedLock>(pSubPicProvider);

    Invalidate();

    return S_OK;
}

STDMETHODIMP CSubPicQueueImpl::GetSubPicProvider(ISubPicProvider** pSubPicProvider)
{
    CheckPointer(pSubPicProvider, E_POINTER);

    CAutoLock cAutoLock(&m_csSubPicProvider);

    if (m_pSubPicProviderWithSharedLock && m_pSubPicProviderWithSharedLock->pSubPicProvider) {
        *pSubPicProvider = m_pSubPicProviderWithSharedLock->pSubPicProvider;
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
        REFERENCE_TIME rtRender;
        if (bIsAnimated) {
            // This is some sort of hack to avoid rendering the wrong frame
            // when the start time is slightly mispredicted by the queue
            rtRender = (rtStart + rtStop) / 2;
        } else {
            rtRender = rtStart + std::llround((rtStop - rtStart - 1) * m_settings.nRenderAtWhenAnimationIsDisabled / 100.0);
        }
        hr = pSubPicProvider->Render(spd, rtRender, fps, r);

        pSubPic->SetStart(rtStart);
        pSubPic->SetStop(rtStop);

        pSubPic->Unlock(r);
    }

    return hr;
}

//
// CSubPicQueue
//

CSubPicQueue::CSubPicQueue(SubPicQueueSettings settings, ISubPicAllocator* pAllocator, HRESULT* phr)
    : CSubPicQueueImpl(settings, pAllocator, phr)
    , m_bExitThread(false)
    , m_rtNowLast(LONGLONG_ERROR)
    , m_bInvalidate(false)
    , m_rtInvalidate(0)
{
    if (phr && FAILED(*phr)) {
        return;
    }

    if (m_settings.nSize < 1) {
        if (phr) {
            *phr = E_INVALIDARG;
        }
        return;
    }

    CAMThread::Create();
}

CSubPicQueue::~CSubPicQueue()
{
    m_bExitThread = true;
    SetSubPicProvider(nullptr);
    CAMThread::Close();
    if (m_pAllocator) {
        m_pAllocator->FreeStatic();
    }
}

// ISubPicQueue

STDMETHODIMP CSubPicQueue::SetFPS(double fps)
{
    HRESULT hr = __super::SetFPS(fps);
    if (FAILED(hr)) {
        return hr;
    }

    m_rtTimePerFrame = std::llround(10000000.0 / m_fps);
    m_rtTimePerSubFrame = std::llround(10000000.0 / (m_fps * m_settings.nAnimationRate / 100.0));

    m_runQueueEvent.Set();

    return S_OK;
}

STDMETHODIMP CSubPicQueue::SetTime(REFERENCE_TIME rtNow)
{
    HRESULT hr = __super::SetTime(rtNow);
    if (FAILED(hr)) {
        return hr;
    }

    // We want the queue to stay sorted so if we seek in the past, we invalidate
    if (m_rtNowLast >= 0 && m_rtNowLast - m_rtNow >= m_rtTimePerFrame) {
        Invalidate(m_rtNow);
    }

    m_rtNowLast = m_rtNow;

    m_runQueueEvent.Set();

    return S_OK;
}

STDMETHODIMP CSubPicQueue::Invalidate(REFERENCE_TIME rtInvalidate /*= -1*/)
{
    std::unique_lock<std::mutex> lockQueue(m_mutexQueue);

#if SUBPIC_TRACE_LEVEL > 0
    TRACE(_T("Invalidate: %f\n"), double(rtInvalidate) / 10000000.0);
#endif

    m_bInvalidate = true;
    m_rtInvalidate = rtInvalidate;
    m_rtNowLast = LONGLONG_ERROR;

    {
        std::lock_guard<std::mutex> lockSubpic(m_mutexSubpic);
        if (m_pSubPic && m_pSubPic->GetStop() > rtInvalidate) {
            m_pSubPic.Release();
        }
    }

    while (!m_queue.IsEmpty() && m_queue.GetTail()->GetStop() > rtInvalidate) {
#if SUBPIC_TRACE_LEVEL > 2
        const CComPtr<ISubPic>& pSubPic = m_queue.GetTail();
        REFERENCE_TIME rtStart = pSubPic->GetStart();
        REFERENCE_TIME rtStop = pSubPic->GetStop();
        REFERENCE_TIME rtSegmentStop = pSubPic->GetSegmentStop();
        TRACE(_T("  %f -> %f -> %f\n"), double(rtStart) / 10000000.0, double(rtStop) / 10000000.0, double(rtSegmentStop) / 10000000.0);
#endif
        m_queue.RemoveTailNoReturn();
    }

    // If we invalidate in the past, always give the queue a chance to re-render the modified subtitles
    if (rtInvalidate >= 0 && rtInvalidate < m_rtNow) {
        m_rtNow = rtInvalidate;
    }

    lockQueue.unlock();
    m_condQueueFull.notify_one();
    m_runQueueEvent.Set();

    return S_OK;
}

STDMETHODIMP_(bool) CSubPicQueue::LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& ppSubPic)
{
    // Old version of LookupSubPic, keep legacy behavior and never try to block
    return LookupSubPic(rtNow, false, ppSubPic);
}

STDMETHODIMP_(bool) CSubPicQueue::LookupSubPic(REFERENCE_TIME rtNow, bool bAdviseBlocking, CComPtr<ISubPic>& ppSubPic)
{
    bool bStopSearch = false;

    {
        std::lock_guard<std::mutex> lock(m_mutexSubpic);

        // See if we can reuse the latest subpic
        if (m_pSubPic) {
            REFERENCE_TIME rtSegmentStart = m_pSubPic->GetSegmentStart();
            REFERENCE_TIME rtSegmentStop = m_pSubPic->GetSegmentStop();

            if (rtSegmentStart <= rtNow && rtNow < rtSegmentStop) {
                ppSubPic = m_pSubPic;

                REFERENCE_TIME rtStart = m_pSubPic->GetStart();
                REFERENCE_TIME rtStop = m_pSubPic->GetStop();

                if (rtStart <= rtNow && rtNow < rtStop) {
#if SUBPIC_TRACE_LEVEL > 2
                    TRACE(_T("LookupSubPic: Exact match on the latest subpic\n"));
#endif
                    bStopSearch = true;
                } else {
#if SUBPIC_TRACE_LEVEL > 2
                    TRACE(_T("LookupSubPic: Possible match on the latest subpic\n"));
#endif
                }
            } else if (rtSegmentStop <= rtNow) {
                m_pSubPic.Release();
            }
        }
    }

    bool bTryBlocking = bAdviseBlocking || !m_settings.bAllowDroppingSubpic;
    while (!bStopSearch) {
        // Look for the subpic in the queue
        {
            std::unique_lock<std::mutex> lock(m_mutexQueue);

#if SUBPIC_TRACE_LEVEL > 2
            TRACE(_T("LookupSubPic: Searching the queue\n"));
#endif

            while (!m_queue.IsEmpty() && !bStopSearch) {
                const CComPtr<ISubPic>& pSubPic = m_queue.GetHead();
                REFERENCE_TIME rtSegmentStart = pSubPic->GetSegmentStart();

                if (rtSegmentStart > rtNow) {
#if SUBPIC_TRACE_LEVEL > 2
                    TRACE(_T("rtSegmentStart > rtNow, stopping the search\n"));
#endif
                    bStopSearch = true;
                } else { // rtSegmentStart <= rtNow
                    bool bRemoveFromQueue = true;
                    REFERENCE_TIME rtStart = pSubPic->GetStart();
                    REFERENCE_TIME rtStop = pSubPic->GetStop();
                    REFERENCE_TIME rtSegmentStop = pSubPic->GetSegmentStop();

                    if (rtSegmentStop <= rtNow) {
#if SUBPIC_TRACE_LEVEL > 2
                        TRACE(_T("Removing old subpic (rtNow=%f): %f -> %f -> %f\n"),
                              double(rtNow) / 10000000.0, double(rtStart) / 10000000.0,
                              double(rtStop) / 10000000.0, double(rtSegmentStop) / 10000000.0);
#endif
                    } else { // rtNow < rtSegmentStop
                        if (rtStart <= rtNow && rtNow < rtStop) {
#if SUBPIC_TRACE_LEVEL > 2
                            TRACE(_T("Exact match found in the queue\n"));
#endif
                            ppSubPic = pSubPic;
                            bStopSearch = true;
                        } else if (rtNow >= rtStop) {
                            // Reuse old subpic
                            ppSubPic = pSubPic;
                        } else { // rtNow < rtStart
                            if (!ppSubPic || ppSubPic->GetStop() <= rtNow) {
                                // Should be really rare that we use a subpic in advance
                                // unless we mispredicted the timing slightly
                                ppSubPic = pSubPic;
                            } else {
                                bRemoveFromQueue = false;
                            }
                            bStopSearch = true;
                        }
                    }

                    if (bRemoveFromQueue) {
                        m_queue.RemoveHeadNoReturn();
                    }
                }
            }

            lock.unlock();
            m_condQueueFull.notify_one();
        }

        // If we didn't get any subpic yet and blocking is advised, just try harder to get one
        if (!ppSubPic && bTryBlocking) {
            bTryBlocking = false;
            bStopSearch = true;

            auto pSubPicProviderWithSharedLock = GetSubPicProviderWithSharedLock();
            if (pSubPicProviderWithSharedLock && SUCCEEDED(pSubPicProviderWithSharedLock->Lock())) {
                auto& pSubPicProvider = pSubPicProviderWithSharedLock->pSubPicProvider;
                double fps = m_fps;
                if (POSITION pos = pSubPicProvider->GetStartPosition(rtNow, fps)) {
                    REFERENCE_TIME rtStart = pSubPicProvider->GetStart(pos, fps);
                    REFERENCE_TIME rtStop = pSubPicProvider->GetStop(pos, fps);
                    if (rtStart <= rtNow && rtNow < rtStop) {
                        bStopSearch = false;
                    }
                }
                pSubPicProviderWithSharedLock->Unlock();

                if (!bStopSearch) {
                    std::unique_lock<std::mutex> lock(m_mutexQueue);

                    auto queueReady = [this, rtNow]() {
                        return ((int)m_queue.GetCount() == m_settings.nSize)
                               || (!m_queue.IsEmpty() && m_queue.GetTail()->GetStop() > rtNow);
                    };

                    m_condQueueReady.wait(lock, queueReady);
                }
            }
        } else {
            bStopSearch = true;
        }
    }

    if (ppSubPic) {
        // Save the subpic for later reuse
        std::lock_guard<std::mutex> lock(m_mutexSubpic);
        m_pSubPic = ppSubPic;

#if SUBPIC_TRACE_LEVEL > 0
        REFERENCE_TIME rtStart = ppSubPic->GetStart();
        REFERENCE_TIME rtStop = ppSubPic->GetStart();
        REFERENCE_TIME rtSegmentStop = ppSubPic->GetSegmentStop();
        CRect r;
        ppSubPic->GetDirtyRect(&r);
        TRACE(_T("Display at %f: %f -> %f -> %f (%dx%d)\n"),
              double(rtNow) / 10000000.0, double(rtStart) / 10000000.0, double(rtStop) / 10000000.0, double(rtSegmentStop) / 10000000.0,
              r.Width(), r.Height());
#endif
    } else {
#if SUBPIC_TRACE_LEVEL > 1
        TRACE(_T("No subpicture to display at %f\n"), double(rtNow) / 10000000.0);
#endif
    }

    return !!ppSubPic;
}

STDMETHODIMP CSubPicQueue::GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
    std::lock_guard<std::mutex> lock(m_mutexQueue);

    nSubPics = (int)m_queue.GetCount();
    rtNow = m_rtNow;
    if (nSubPics) {
        rtStart = m_queue.GetHead()->GetStart();
        rtStop = m_queue.GetTail()->GetStop();
    } else {
        rtStart = rtStop = 0;
    }

    return S_OK;
}

STDMETHODIMP CSubPicQueue::GetStats(int nSubPic, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
    std::lock_guard<std::mutex> lock(m_mutexQueue);

    HRESULT hr = E_INVALIDARG;

    if (nSubPic >= 0 && nSubPic < (int)m_queue.GetCount()) {
        if (POSITION pos = m_queue.FindIndex(nSubPic)) {
            rtStart = m_queue.GetAt(pos)->GetStart();
            rtStop = m_queue.GetAt(pos)->GetStop();
            hr = S_OK;
        } else {
            // Can't happen
            ASSERT(FALSE);
        }
    } else {
        rtStart = rtStop = -1;
    }

    return hr;
}

// private

bool CSubPicQueue::EnqueueSubPic(CComPtr<ISubPic>& pSubPic, bool bBlocking)
{
    auto canAddToQueue = [this]() {
        return (int)m_queue.GetCount() < m_settings.nSize;
    };

    bool bAdded = false;

    std::unique_lock<std::mutex> lock(m_mutexQueue);
    if (bBlocking) {
        // Wait for enough room in the queue
        m_condQueueFull.wait(lock, canAddToQueue);
    }

    if (canAddToQueue()) {
        if (m_bInvalidate && pSubPic->GetStop() > m_rtInvalidate) {
#if SUBPIC_TRACE_LEVEL > 1
            TRACE(_T("Subtitle Renderer Thread: Dropping rendered subpic because of invalidation\n"));
#endif
        } else {
            m_queue.AddTail(pSubPic);
            lock.unlock();
            m_condQueueReady.notify_one();
            bAdded = true;
        }
        pSubPic.Release();
    }

    return bAdded;
}

REFERENCE_TIME CSubPicQueue::GetCurrentRenderingTime()
{
    REFERENCE_TIME rtNow = -1;

    {
        std::lock_guard<std::mutex> lock(m_mutexQueue);

        if (!m_queue.IsEmpty()) {
            rtNow = m_queue.GetTail()->GetStop();
        }
    }

    return std::max(rtNow, m_rtNow);
}

// overrides

DWORD CSubPicQueue::ThreadProc()
{
    bool bDisableAnim = m_settings.bDisableSubtitleAnimation;
    SetThreadName(DWORD(-1), "Subtitle Renderer Thread");
    SetThreadPriority(m_hThread, bDisableAnim ? THREAD_PRIORITY_LOWEST : THREAD_PRIORITY_ABOVE_NORMAL);

    bool bWaitForEvent = false;
    for (; !m_bExitThread;) {
        // When we have nothing to render, we just wait a bit
        if (bWaitForEvent) {
            bWaitForEvent = false;
            m_runQueueEvent.Wait();
        }

        auto pSubPicProviderWithSharedLock = GetSubPicProviderWithSharedLock();
        if (pSubPicProviderWithSharedLock && SUCCEEDED(pSubPicProviderWithSharedLock->Lock())) {
            auto& pSubPicProvider = pSubPicProviderWithSharedLock->pSubPicProvider;
            double fps = m_fps;
            REFERENCE_TIME rtTimePerFrame = m_rtTimePerFrame;
            REFERENCE_TIME rtTimePerSubFrame = m_rtTimePerSubFrame;
            m_bInvalidate = false;
            CComPtr<ISubPic> pSubPic;

            REFERENCE_TIME rtStartRendering = GetCurrentRenderingTime();
            POSITION pos = pSubPicProvider->GetStartPosition(rtStartRendering, fps);
            if (!pos) {
                bWaitForEvent = true;
            }
            for (; pos; pos = pSubPicProvider->GetNext(pos)) {
                REFERENCE_TIME rtStart = pSubPicProvider->GetStart(pos, fps);
                REFERENCE_TIME rtStop = pSubPicProvider->GetStop(pos, fps);

                // We are already one minute ahead, this should be enough
                if (rtStart >= m_rtNow + 60 * 10000000i64) {
                    bWaitForEvent = true;
                    break;
                }

                REFERENCE_TIME rtCurrent = std::max(rtStart, rtStartRendering);
                if (rtCurrent > m_rtNow) {
                    // Round current time to the next estimated video frame timing
                    REFERENCE_TIME rtCurrentRounded = (rtCurrent / rtTimePerFrame) * rtTimePerFrame;
                    if (rtCurrentRounded < rtCurrent) {
                        rtCurrent = rtCurrentRounded + rtTimePerFrame;
                    }
                } else {
                    rtCurrent = m_rtNow;
                }

                // Check that we aren't late already...
                if (rtCurrent < rtStop) {
                    bool bIsAnimated = pSubPicProvider->IsAnimated(pos) && !bDisableAnim;
                    bool bStopRendering = false;

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

                        REFERENCE_TIME rtStopReal;
                        if (rtStop == ISubPicProvider::UNKNOWN_TIME) { // Special case for subtitles with unknown end time
                            // Force a one frame duration
                            rtStopReal = rtCurrent + rtTimePerFrame;
                        } else {
                            rtStopReal = rtStop;
                        }

                        HRESULT hr;
                        if (bIsAnimated) {
                            // 3/4 is a magic number we use to avoid reusing the wrong frame due to slight
                            // misprediction of the frame end time
                            hr = RenderTo(pStatic, rtCurrent, std::min(rtCurrent + rtTimePerSubFrame * 3 / 4, rtStopReal), fps, bIsAnimated);
                            // Set the segment start and stop timings
                            pStatic->SetSegmentStart(rtStart);
                            // The stop timing can be moved so that the duration from the current start time
                            // of the subpic to the segment end is always at least one video frame long. This
                            // avoids missing subtitle frame due to rounding errors in the timings.
                            // At worst this can cause a segment to be displayed for one more frame than expected
                            // but it's much less annoying than having the subtitle disappearing for one frame
                            pStatic->SetSegmentStop(std::max(rtCurrent + rtTimePerFrame, rtStopReal));
                            rtCurrent = std::min(rtCurrent + rtTimePerSubFrame, rtStopReal);
                        } else {
                            hr = RenderTo(pStatic, rtStart, rtStopReal, fps, bIsAnimated);
                            // Non-animated subtitles aren't part of a segment
                            pStatic->SetSegmentStart(ISubPic::INVALID_TIME);
                            pStatic->SetSegmentStop(ISubPic::INVALID_TIME);
                            rtCurrent = rtStopReal;
                        }

                        if (FAILED(hr)) {
                            break;
                        }

#if SUBPIC_TRACE_LEVEL > 1
                        CRect r;
                        pStatic->GetDirtyRect(&r);
                        TRACE(_T("Subtitle Renderer Thread: Render %f -> %f -> %f -> %f (%dx%d)\n"),
                              double(rtStart) / 10000000.0, double(pStatic->GetStart()) / 10000000.0,
                              double(pStatic->GetStop()) / 10000000.0, double(rtStop) / 10000000.0,
                              r.Width(), r.Height());
#endif

                        pSubPic.Release();
                        if (FAILED(m_pAllocator->AllocDynamic(&pSubPic))
                                || FAILED(pStatic->CopyTo(pSubPic))) {
                            break;
                        }

                        if (SUCCEEDED(hr2)) {
                            pSubPic->SetVirtualTextureSize(virtualSize, virtualTopLeft);
                        }

                        RelativeTo relativeTo;
                        if (SUCCEEDED(pSubPicProvider->GetRelativeTo(pos, relativeTo))) {
                            pSubPic->SetRelativeTo(relativeTo);
                        }

                        // Try to enqueue the subpic, if the queue is full stop rendering
                        if (!EnqueueSubPic(pSubPic, false)) {
                            bStopRendering = true;
                            break;
                        }

                        if (m_rtNow > rtCurrent) {
#if SUBPIC_TRACE_LEVEL > 0
                            TRACE(_T("Subtitle Renderer Thread: the queue is late, trying to catch up...\n"));
#endif
                            rtCurrent = m_rtNow;
                        }
                    }

                    if (bStopRendering) {
                        break;
                    }
                } else {
#if SUBPIC_TRACE_LEVEL > 0
                    TRACE(_T("Subtitle Renderer Thread: the queue is late, trying to catch up...\n"));
#endif
                }
            }

            pSubPicProviderWithSharedLock->Unlock();

            // If we couldn't enqueue the subpic before, wait for some room in the queue
            // but unsure to unlock the subpicture provider first to avoid deadlocks
            if (pSubPic) {
                EnqueueSubPic(pSubPic, true);
            }
        } else {
            bWaitForEvent = true;
        }
    }

    return 0;
}

//
// CSubPicQueueNoThread
//

CSubPicQueueNoThread::CSubPicQueueNoThread(SubPicQueueSettings settings, ISubPicAllocator* pAllocator, HRESULT* phr)
    : CSubPicQueueImpl(settings, pAllocator, phr)
{
    if (phr && SUCCEEDED(*phr) && m_settings.nSize != 0) {
        *phr = E_INVALIDARG;
    }
}

CSubPicQueueNoThread::~CSubPicQueueNoThread()
{
    if (m_pAllocator) {
        m_pAllocator->FreeStatic();
    }
}

// ISubPicQueue

STDMETHODIMP CSubPicQueueNoThread::SetFPS(double fps)
{
    HRESULT hr = __super::SetFPS(fps);
    if (FAILED(hr)) {
        return hr;
    }

    if (m_settings.nAnimationRate == 100) { // Special case when rendering at full speed
        // Ensure the subtitle will really be updated every frame by setting a really small duration
        m_rtTimePerSubFrame = 1;
    } else {
        m_rtTimePerSubFrame = std::llround(10000000.0 / (m_fps * m_settings.nAnimationRate / 100.0));
    }

    return S_OK;
}

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
    // CSubPicQueueNoThread is always blocking so bAdviseBlocking doesn't matter anyway
    return LookupSubPic(rtNow, true, ppSubPic);
}

STDMETHODIMP_(bool) CSubPicQueueNoThread::LookupSubPic(REFERENCE_TIME rtNow, bool /*bAdviseBlocking*/, CComPtr<ISubPic>& ppSubPic)
{
    // CSubPicQueueNoThread is always blocking so we ignore bAdviseBlocking

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
                REFERENCE_TIME rtStop = pSubPicProvider->GetStop(pos, fps);
                bool bAnimated = pSubPicProvider->IsAnimated(pos) && !m_settings.bDisableSubtitleAnimation;

                // Special case for subtitles with unknown end time
                if (rtStop == ISubPicProvider::UNKNOWN_TIME) {
                    // Force a one frame duration
                    rtStop = rtNow + 1;
                }

                if (bAnimated) {
                    rtStart = rtNow;
                    rtStop = std::min(rtNow + m_rtTimePerSubFrame, rtStop);
                } else {
                    rtStart = pSubPicProvider->GetStart(pos, fps);
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
                                && SUCCEEDED(RenderTo(pStatic, rtStart, rtStop, fps, bAnimated))
                                && SUCCEEDED(pStatic->CopyTo(pSubPic))) {
                            ppSubPic = pSubPic;
                        }
                    } else {
                        if (SUCCEEDED(RenderTo(pSubPic, rtStart, rtStop, fps, bAnimated))) {
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
