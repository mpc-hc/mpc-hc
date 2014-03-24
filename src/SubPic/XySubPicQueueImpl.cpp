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
#include "XySubPicQueueImpl.h"
#include "XySubPicProvider.h"

#define SUBPIC_TRACE_LEVEL 0

//
// CXySubPicQueueNoThread
//

CXySubPicQueue::CXySubPicQueue(int nMaxSubPic, ISubPicAllocator* pAllocator, HRESULT* phr)
    : CSubPicQueue(nMaxSubPic, false, pAllocator, phr)
    , m_llSubId(0)
{
}

CXySubPicQueue::~CXySubPicQueue()
{
}

// ISubPicQueue

STDMETHODIMP CXySubPicQueue::Invalidate(REFERENCE_TIME rtInvalidate)
{
    m_llSubId = 0;
    return __super::Invalidate(rtInvalidate);
}

// overrides

DWORD CXySubPicQueue::ThreadProc()
{
    SetThreadName(DWORD(-1), "Subtitle Renderer Thread");
    SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);

    bool bAgain = true;
    for (;;) {
        DWORD Ret = WaitForMultipleObjects(EVENT_COUNT, m_ThreadEvents, FALSE, bAgain ? 0 : INFINITE);
        bAgain = false;

        if (Ret == WAIT_TIMEOUT) {
            ;
        } else if ((Ret - WAIT_OBJECT_0) != EVENT_TIME) {
            break;
        }
        double fps = m_fps;
        REFERENCE_TIME rtTimePerFrame = (REFERENCE_TIME)(10000000.0 / fps);
        REFERENCE_TIME rtNow = UpdateQueue();

        int nMaxSubPic = m_nMaxSubPic;

        CComPtr<ISubPicProvider> pSubPicProvider;
        GetSubPicProvider(&pSubPicProvider);
        CComQIPtr<IXyCompatProvider> pXySubPicProvider = pSubPicProvider;
        if (pXySubPicProvider && SUCCEEDED(pSubPicProvider->Lock())) {
            for (REFERENCE_TIME rtStart = rtNow; !m_fBreakBuffering && GetQueueCount() < nMaxSubPic; rtStart += rtTimePerFrame) {
                REFERENCE_TIME rtStop = rtStart + rtTimePerFrame;

                if (m_rtNow >= rtStart) {
                    if (m_rtNow >= rtStop) {
                        continue;
                    }
                }

                if (rtStart >= m_rtNow + 60 * 10000000i64) {    // we are already one minute ahead, this should be enough
                    break;
                }

                if (m_rtNow > rtStop) {
                    TRACE(_T("BEHIND\n"));
                }

                HRESULT hr = pXySubPicProvider->RequestFrame(rtStart, rtStop, INFINITE);
                if (SUCCEEDED(hr)) {
                    ULONGLONG id;
                    hr = pXySubPicProvider->GetID(&id);
                    if (SUCCEEDED(hr)) {
                        SIZE    MaxTextureSize, VirtualSize;
                        POINT   VirtualTopLeft;
                        HRESULT hr2;

                        if (SUCCEEDED(hr2 = pSubPicProvider->GetTextureSize(0, MaxTextureSize, VirtualSize, VirtualTopLeft))) {
                            m_pAllocator->SetMaxTextureSize(MaxTextureSize);
                        }

                        if (m_llSubId == id && !m_Queue.IsEmpty()) { // same subtitle as last time
                            CComPtr<ISubPic> pSubPic = m_Queue.GetTail();
                            pSubPic->SetStop(rtStop);
#if SUBPIC_TRACE_LEVEL > 1
                            CRect r;
                            pSubPic->GetDirtyRect(&r);
                            TRACE(_T("Skip:   %f->%f      %dx%d\n"), double(pSubPic->GetStart()) / 10000000.0, double(pSubPic->GetStop()) / 10000000.0, r.Width(), r.Height());
#endif
                            continue;
                        } else {
                            CComPtr<ISubPic> pStatic;
                            if (FAILED(m_pAllocator->GetStatic(&pStatic))) {
                                break;
                            }

                            pStatic->SetInverseAlpha(true);
                            hr = RenderTo(pStatic, rtStart, rtStop, fps, true);
#if SUBPIC_TRACE_LEVEL > 0
                            CRect r;
                            pStatic->GetDirtyRect(&r);
                            TRACE(_T("Render: %f->%f      %dx%d\n"), double(rtStart) / 10000000.0, double(rtStop) / 10000000.0, r.Width(), r.Height());
#endif
                            if (FAILED(hr)) {
                                break;
                            }

                            CComPtr<ISubPic> pDynamic;
                            if (FAILED(m_pAllocator->AllocDynamic(&pDynamic))
                                    || FAILED(pStatic->CopyTo(pDynamic))) {
                                break;
                            }

                            if (SUCCEEDED(hr2)) {
                                pDynamic->SetVirtualTextureSize(VirtualSize, VirtualTopLeft);
                            }

                            RelativeTo relativeTo;
                            if (SUCCEEDED(pSubPicProvider->GetRelativeTo(0, relativeTo))) {
                                pDynamic->SetRelativeTo(relativeTo);
                            }

                            AppendQueue(pDynamic);
                            m_llSubId = id;
                            bAgain = true;
                        }
                    }
                }
            }

            pSubPicProvider->Unlock();
        }

        if (m_fBreakBuffering) {
            bAgain = true;
            CAutoLock cQueueLock(&m_csQueueLock);

            REFERENCE_TIME rtInvalidate = m_rtInvalidate;

            POSITION Iter = m_Queue.GetHeadPosition();
            while (Iter) {
                POSITION ThisPos = Iter;
                ISubPic* pSubPic = m_Queue.GetNext(Iter);

                REFERENCE_TIME rtStart = pSubPic->GetStart();
                REFERENCE_TIME rtStop = pSubPic->GetStop();

                if (rtStop > rtInvalidate) {
#if SUBPIC_TRACE_LEVEL >= 0
                    TRACE(_T("Removed subtitle because of invalidation: %f->%f\n"), double(rtStart) / 10000000.0, double(rtStop) / 10000000.0);
#endif
                    m_Queue.RemoveAt(ThisPos);
                    continue;
                }
            }

            /*
            while (GetCount() && GetTail()->GetStop() > rtInvalidate)
            {
                if (GetTail()->GetStart() < rtInvalidate) GetTail()->SetStop(rtInvalidate);
                else
                {
                    RemoveTail();
                }
            }
            */

            m_fBreakBuffering = false;
        }
    }

    return 0;
}

//
// CXySubPicQueueNoThread
//

CXySubPicQueueNoThread::CXySubPicQueueNoThread(ISubPicAllocator* pAllocator, HRESULT* phr)
    : CSubPicQueueNoThread(pAllocator, phr)
    , m_llSubId(0)
{
}

CXySubPicQueueNoThread::~CXySubPicQueueNoThread()
{
}

// ISubPicQueue

STDMETHODIMP CXySubPicQueueNoThread::Invalidate(REFERENCE_TIME rtInvalidate)
{
    m_llSubId = 0;
    return __super::Invalidate(rtInvalidate);
}

STDMETHODIMP_(bool) CXySubPicQueueNoThread::LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& ppSubPic)
{
    CComPtr<ISubPic> pSubPic;
    CComPtr<ISubPicProvider> pSubPicProvider;
    GetSubPicProvider(&pSubPicProvider);
    CComQIPtr<IXyCompatProvider> pXySubPicProvider = pSubPicProvider;

    {
        CAutoLock cAutoLock(&m_csLock);

        pSubPic = m_pSubPic;
    }

    if (pXySubPicProvider) {
        double fps = m_fps;
        REFERENCE_TIME rtTimePerFrame = static_cast<REFERENCE_TIME>(10000000.0 / fps);

        REFERENCE_TIME rtStart = rtNow;
        REFERENCE_TIME rtStop = rtNow + rtTimePerFrame;

        HRESULT hr = pXySubPicProvider->RequestFrame(rtStart, rtStop, (DWORD)(1000.0 / fps));
        if (SUCCEEDED(hr)) {
            ULONGLONG id;
            hr = pXySubPicProvider->GetID(&id);
            if (SUCCEEDED(hr)) {
                bool    bAllocSubPic = !pSubPic;
                SIZE    MaxTextureSize, VirtualSize;
                POINT   VirtualTopLeft;
                HRESULT hr2;
                if (SUCCEEDED(hr2 = pSubPicProvider->GetTextureSize(0, MaxTextureSize, VirtualSize, VirtualTopLeft))) {
                    m_pAllocator->SetMaxTextureSize(MaxTextureSize);
                    if (!bAllocSubPic) {
                        // Ensure the previously allocated subpic is big enough to hold the subtitle to be rendered
                        SIZE maxSize;
                        bAllocSubPic = FAILED(pSubPic->GetMaxSize(&maxSize)) || maxSize.cx < MaxTextureSize.cx || maxSize.cy < MaxTextureSize.cy;
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

                if (!bAllocSubPic && m_llSubId == id) { // same subtitle as last time
                    pSubPic->SetStop(rtStop);
                    ppSubPic = pSubPic;
                } else if (m_pAllocator->IsDynamicWriteOnly()) {
                    CComPtr<ISubPic> pStatic;
                    hr = m_pAllocator->GetStatic(&pStatic);
                    if (SUCCEEDED(hr)) {
                        pStatic->SetInverseAlpha(true);
                        hr = RenderTo(pStatic, rtStart, rtStop, fps, true);
                    }
                    if (SUCCEEDED(hr)) {
                        hr = pStatic->CopyTo(pSubPic);
                    }
                    if (SUCCEEDED(hr)) {
                        ppSubPic = pSubPic;
                        m_llSubId = id;
                    }
                } else {
                    pSubPic->SetInverseAlpha(true);
                    if (SUCCEEDED(RenderTo(pSubPic, rtStart, rtStop, fps, true))) {
                        ppSubPic = pSubPic;
                        m_llSubId = id;
                    }
                }

                if (ppSubPic) {
                    if (SUCCEEDED(hr2)) {
                        ppSubPic->SetVirtualTextureSize(VirtualSize, VirtualTopLeft);
                    }

                    RelativeTo relativeTo;
                    if (SUCCEEDED(pSubPicProvider->GetRelativeTo(0, relativeTo))) {
                        ppSubPic->SetRelativeTo(relativeTo);
                    }
                }
            }
        }
    }

    return !!ppSubPic;
}
