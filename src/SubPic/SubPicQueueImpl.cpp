/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
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
#include "SubPicQueueImpl.h"
#include "../DSUtil/DSUtil.h"

//
// CSubPicQueueImpl
//

CSubPicQueueImpl::CSubPicQueueImpl(ISubPicAllocator* pAllocator, HRESULT* phr)
	: CUnknown(NAME("CSubPicQueueImpl"), NULL)
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

	//	if(m_pSubPicProvider != pSubPicProvider)
	{
		m_pSubPicProvider = pSubPicProvider;

		Invalidate();
	}

	return S_OK;
}

STDMETHODIMP CSubPicQueueImpl::GetSubPicProvider(ISubPicProvider** pSubPicProvider)
{
	if (!pSubPicProvider) {
		return E_POINTER;
	}

	CAutoLock cAutoLock(&m_csSubPicProvider);

	if (m_pSubPicProvider) {
		*pSubPicProvider = m_pSubPicProvider;
		(*pSubPicProvider)->AddRef();
	}

	return !!*pSubPicProvider ? S_OK : E_FAIL;
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
	HRESULT hr = E_FAIL;

	if (!pSubPic) {
		return hr;
	}

	CComPtr<ISubPicProvider> pSubPicProvider;
	if (FAILED(GetSubPicProvider(&pSubPicProvider)) || !pSubPicProvider) {
		return hr;
	}

	SubPicDesc spd;
	hr = pSubPic->ClearDirtyRect(0xFF000000);
	if (SUCCEEDED(hr)) {
		hr = pSubPic->Lock(spd);
	}
	if (SUCCEEDED(hr)) {
		CRect r(0,0,0,0);
		hr = pSubPicProvider->Render(spd, bIsAnimated ? rtStart : ((rtStart+rtStop)/2), fps, r);

		pSubPic->SetStart(rtStart);
		pSubPic->SetStop(rtStop);

		pSubPic->Unlock(r);
	}

	return hr;
}

//
// CSubPicQueue
//

CSubPicQueue::CSubPicQueue(int nMaxSubPic, BOOL bDisableAnim, ISubPicAllocator* pAllocator, HRESULT* phr)
	: CSubPicQueueImpl(pAllocator, phr)
	, m_nMaxSubPic(nMaxSubPic)
	, m_bDisableAnim(bDisableAnim)
	,m_rtQueueMin(0)
	,m_rtQueueMax(0)
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

	m_fBreakBuffering = false;
	for (ptrdiff_t i = 0; i < EVENT_COUNT; i++) {
		m_ThreadEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	CAMThread::Create();
}

CSubPicQueue::~CSubPicQueue()
{
	m_fBreakBuffering = true;
	SetEvent(m_ThreadEvents[EVENT_EXIT]);
	CAMThread::Close();
	for (ptrdiff_t i = 0; i < EVENT_COUNT; i++) {
		CloseHandle(m_ThreadEvents[i]);
	}
}

// ISubPicQueue

STDMETHODIMP CSubPicQueue::SetFPS(double fps)
{
	HRESULT hr = __super::SetFPS(fps);
	if (FAILED(hr)) {
		return hr;
	}

	SetEvent(m_ThreadEvents[EVENT_TIME]);

	return S_OK;
}

STDMETHODIMP CSubPicQueue::SetTime(REFERENCE_TIME rtNow)
{
	HRESULT hr = __super::SetTime(rtNow);
	if (FAILED(hr)) {
		return hr;
	}

	SetEvent(m_ThreadEvents[EVENT_TIME]);

	return S_OK;
}

STDMETHODIMP CSubPicQueue::Invalidate(REFERENCE_TIME rtInvalidate)
{
	{
		//		CAutoLock cQueueLock(&m_csQueueLock);
		//		RemoveAll();

		m_rtInvalidate = rtInvalidate;
		m_fBreakBuffering = true;
#if DSubPicTraceLevel > 0
		TRACE(_T("Invalidate: %f\n"), double(rtInvalidate) / 10000000.0);
#endif

		SetEvent(m_ThreadEvents[EVENT_TIME]);
	}

	return S_OK;
}

STDMETHODIMP_(bool) CSubPicQueue::LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic> &ppSubPic)
{

	CAutoLock cQueueLock(&m_csQueueLock);

	REFERENCE_TIME rtBestStop = 0x7fffffffffffffffi64;
	POSITION pos = m_Queue.GetHeadPosition();
#if DSubPicTraceLevel > 2
	TRACE("Find: ");
#endif
	while (pos) {
		CComPtr<ISubPic> pSubPic = m_Queue.GetNext(pos);
		REFERENCE_TIME rtStart = pSubPic->GetStart();
		REFERENCE_TIME rtStop = pSubPic->GetStop();
		REFERENCE_TIME rtSegmentStop = pSubPic->GetSegmentStop();
		if (rtNow >= rtStart && rtNow < rtSegmentStop) {
			REFERENCE_TIME Diff = rtNow - rtStop;
			if (Diff < rtBestStop) {
				rtBestStop = Diff;
				//				TRACE("   %f->%f", double(Diff) / 10000000.0, double(rtStop) / 10000000.0);
				ppSubPic = pSubPic;
			}
#if DSubPicTraceLevel > 2
			else {
				TRACE("   !%f->%f", double(Diff) / 10000000.0, double(rtStop) / 10000000.0);
			}
#endif
		}
#if DSubPicTraceLevel > 2
		else {
			TRACE("   !!%f->%f", double(rtStart) / 10000000.0, double(rtSegmentStop) / 10000000.0);
		}
#endif

	}
#if DSubPicTraceLevel > 2
	TRACE("\n");
#endif
	if (!ppSubPic) {
#if DSubPicTraceLevel > 1
		TRACE("NO Display: %f\n", double(rtNow) / 10000000.0);
#endif
	} else {
#if DSubPicTraceLevel > 0
		REFERENCE_TIME rtStart = (ppSubPic)->GetStart();
		REFERENCE_TIME rtSegmentStop = (ppSubPic)->GetSegmentStop();
		CRect r;
		(ppSubPic)->GetDirtyRect(&r);
		TRACE("Display: %f->%f   %f    %dx%d\n", double(rtStart) / 10000000.0, double(rtSegmentStop) / 10000000.0, double(rtNow) / 10000000.0, r.Width(), r.Height());
#endif
	}

	return(!!ppSubPic);
}

STDMETHODIMP CSubPicQueue::GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
	CAutoLock cQueueLock(&m_csQueueLock);

	nSubPics = m_Queue.GetCount();
	rtNow = m_rtNow;
	rtStart = m_rtQueueMin;
	if (rtStart == 0x7fffffffffffffffi64) {
		rtStart = 0;
	}
	rtStop = m_rtQueueMax;
	if (rtStop == 0xffffffffffffffffi64) {
		rtStop = 0;
	}

	return S_OK;
}

STDMETHODIMP CSubPicQueue::GetStats(int nSubPic, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
	CAutoLock cQueueLock(&m_csQueueLock);

	rtStart = rtStop = -1;

	if (nSubPic >= 0 && nSubPic < (int)m_Queue.GetCount()) {
		if (POSITION pos = m_Queue.FindIndex(nSubPic)) {
			rtStart = m_Queue.GetAt(pos)->GetStart();
			rtStop = m_Queue.GetAt(pos)->GetStop();
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
		m_Queue.RemoveAll();
		m_rtNowLast = rtNow;
	} else {
		m_rtNowLast = rtNow;

		m_rtQueueMin = 0x7fffffffffffffffi64;
		m_rtQueueMax = 0xffffffffffffffffi64;

		POSITION SavePos = 0;
		{
			POSITION Iter = m_Queue.GetHeadPosition();
			REFERENCE_TIME rtBestStop = 0x7fffffffffffffffi64;
			while (Iter) {
				POSITION ThisPos = Iter;
				ISubPic *pSubPic = m_Queue.GetNext(Iter);
				REFERENCE_TIME rtStart = pSubPic->GetStart();
				REFERENCE_TIME rtStop = pSubPic->GetStop();
				REFERENCE_TIME rtSegmentStop = pSubPic->GetSegmentStop();
				if (rtNow >= rtStart && rtNow < rtSegmentStop) {
					REFERENCE_TIME Diff = rtNow - rtStop;
					if (Diff < rtBestStop) {
						rtBestStop = Diff;
						SavePos = ThisPos;
					}
				}
			}
		}

#if DSubPicTraceLevel > 3
		if (SavePos) {
			ISubPic *pSubPic = GetAt(SavePos);
			REFERENCE_TIME rtStart = pSubPic->GetStart();
			REFERENCE_TIME rtStop = pSubPic->GetStop();
			TRACE("Save: %f->%f\n", double(rtStart) / 10000000.0, double(rtStop) / 10000000.0);
		}
#endif
		{
			POSITION Iter = m_Queue.GetHeadPosition();
			while (Iter) {
				POSITION ThisPos = Iter;
				ISubPic *pSubPic = m_Queue.GetNext(Iter);

				REFERENCE_TIME rtStart = pSubPic->GetStart();
				REFERENCE_TIME rtStop = pSubPic->GetStop();

				if (rtStop <= rtNowCompare && ThisPos != SavePos) {
#if DSubPicTraceLevel > 0
					TRACE("Remove: %f->%f\n", double(rtStart) / 10000000.0, double(rtStop) / 10000000.0);
#endif
					m_Queue.RemoveAt(ThisPos);
					continue;
				}
				if (rtStop > rtNow) {
					rtNow = rtStop;
				}
				m_rtQueueMin = min(m_rtQueueMin, rtStart);
				m_rtQueueMax = max(m_rtQueueMax, rtStop);
			}
		}
	}

	return(rtNow);
}

int CSubPicQueue::GetQueueCount()
{
	CAutoLock cQueueLock(&m_csQueueLock);

	return m_Queue.GetCount();
}

void CSubPicQueue::AppendQueue(ISubPic* pSubPic)
{
	CAutoLock cQueueLock(&m_csQueueLock);

	m_Queue.AddTail(pSubPic);
}

// overrides

DWORD CSubPicQueue::ThreadProc()
{
	BOOL bDisableAnim = m_bDisableAnim;
	SetThreadPriority(m_hThread, bDisableAnim ? THREAD_PRIORITY_LOWEST : THREAD_PRIORITY_ABOVE_NORMAL/*THREAD_PRIORITY_BELOW_NORMAL*/);

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
		REFERENCE_TIME rtTimePerFrame = 10000000.0/fps;
		REFERENCE_TIME rtNow = UpdateQueue();

		int nMaxSubPic = m_nMaxSubPic;

		CComPtr<ISubPicProvider> pSubPicProvider;
		if (SUCCEEDED(GetSubPicProvider(&pSubPicProvider)) && pSubPicProvider
				&& SUCCEEDED(pSubPicProvider->Lock())) {
			for (POSITION pos = pSubPicProvider->GetStartPosition(rtNow, fps);
					pos && !m_fBreakBuffering && GetQueueCount() < nMaxSubPic;
					pos = pSubPicProvider->GetNext(pos)) {
				REFERENCE_TIME rtStart = pSubPicProvider->GetStart(pos, fps);
				REFERENCE_TIME rtStop = pSubPicProvider->GetStop(pos, fps);

				if (m_rtNow >= rtStart) {
					//						m_fBufferUnderrun = true;
					if (m_rtNow >= rtStop) {
						continue;
					}
				}

				if (rtStart >= m_rtNow + 60*10000000i64) { // we are already one minute ahead, this should be enough
					break;
				}

				if (rtNow < rtStop) {
					REFERENCE_TIME rtCurrent = max(rtNow, rtStart);
					bool bIsAnimated = pSubPicProvider->IsAnimated(pos) && !bDisableAnim;
					while (rtCurrent < rtStop) {

						SIZE	MaxTextureSize, VirtualSize;
						POINT	VirtualTopLeft;
						HRESULT	hr2;
						if (SUCCEEDED (hr2 = pSubPicProvider->GetTextureSize(pos, MaxTextureSize, VirtualSize, VirtualTopLeft))) {
							m_pAllocator->SetMaxTextureSize(MaxTextureSize);
						}

						CComPtr<ISubPic> pStatic;
						if (FAILED(m_pAllocator->GetStatic(&pStatic))) {
							break;
						}

						HRESULT hr;
						if (bIsAnimated) {
							if (rtCurrent < m_rtNow + rtTimePerFrame) {
								rtCurrent = min(m_rtNow + rtTimePerFrame, rtStop-1);
							}

							REFERENCE_TIME rtEndThis = min(rtCurrent + rtTimePerFrame, rtStop);
							hr = RenderTo(pStatic, rtCurrent, rtEndThis, fps, bIsAnimated);
							pStatic->SetSegmentStart(rtStart);
							pStatic->SetSegmentStop(rtStop);
#if DSubPicTraceLevel > 0
							CRect r;
							pStatic->GetDirtyRect(&r);
							TRACE("Render: %f->%f    %f->%f      %dx%d\n", double(rtCurrent) / 10000000.0, double(rtEndThis) / 10000000.0, double(rtStart) / 10000000.0, double(rtStop) / 10000000.0, r.Width(), r.Height());
#endif
							rtCurrent = rtEndThis;


						} else {
							hr = RenderTo(pStatic, rtStart, rtStop, fps, bIsAnimated);
							rtCurrent = rtStop;
						}
#if DSubPicTraceLevel > 0
						if (m_rtNow > rtCurrent) {
							TRACE("BEHIND\n");
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

						if (SUCCEEDED (hr2)) {
							pDynamic->SetVirtualTextureSize (VirtualSize, VirtualTopLeft);
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

		if (m_fBreakBuffering) {
			bAgain = true;
			CAutoLock cQueueLock(&m_csQueueLock);

			REFERENCE_TIME rtInvalidate = m_rtInvalidate;

			POSITION Iter = m_Queue.GetHeadPosition();
			while (Iter) {
				POSITION ThisPos = Iter;
				ISubPic *pSubPic = m_Queue.GetNext(Iter);

				REFERENCE_TIME rtStart = pSubPic->GetStart();
				REFERENCE_TIME rtStop = pSubPic->GetStop();

				if (rtStop > rtInvalidate) {
#if DSubPicTraceLevel >= 0
					TRACE(_T("Removed subtitle because of invalidation: %f->%f\n"), double(rtStart) / 10000000.0, double(rtStop) / 10000000.0);
#endif
					m_Queue.RemoveAt(ThisPos);
					continue;
				}
			}

			/*
						while(GetCount() && GetTail()->GetStop() > rtInvalidate)
						{
							if(GetTail()->GetStart() < rtInvalidate) GetTail()->SetStop(rtInvalidate);
							else
							{
								RemoveTail();
							}
						}
			*/

			m_fBreakBuffering = false;
		}
	}

	return(0);
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

STDMETHODIMP CSubPicQueueNoThread::Invalidate(REFERENCE_TIME rtInvalidate)
{
	CAutoLock cQueueLock(&m_csLock);

	m_pSubPic = NULL;

	return S_OK;
}

STDMETHODIMP_(bool) CSubPicQueueNoThread::LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic> &ppSubPic)
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
		CComPtr<ISubPicProvider> pSubPicProvider;
		GetSubPicProvider(&pSubPicProvider);
		if (pSubPicProvider) {
			double fps = m_fps;

			POSITION pos = pSubPicProvider->GetStartPosition(rtNow, fps);
			if (pos != 0) {
				REFERENCE_TIME rtStart = pSubPicProvider->GetStart(pos, fps);
				REFERENCE_TIME rtStop = pSubPicProvider->GetStop(pos, fps);

				if (pSubPicProvider->IsAnimated(pos)) {
					rtStart = rtNow;
					rtStop = rtNow+1;
				}

				if (rtStart <= rtNow && rtNow < rtStop) {
					SIZE	MaxTextureSize, VirtualSize;
					POINT	VirtualTopLeft;
					HRESULT	hr2;
					if (SUCCEEDED (hr2 = pSubPicProvider->GetTextureSize(pos, MaxTextureSize, VirtualSize, VirtualTopLeft))) {
						m_pAllocator->SetMaxTextureSize(MaxTextureSize);
					}

					if (m_pAllocator->IsDynamicWriteOnly()) {
						CComPtr<ISubPic> pStatic;
						HRESULT hr = m_pAllocator->GetStatic(&pStatic);
						if (SUCCEEDED(hr)) {
							hr = RenderTo(pStatic, rtStart, rtStop, fps, false);
						}
						if (SUCCEEDED(hr)) {
							hr = pStatic->CopyTo(pSubPic);
						}
						if (SUCCEEDED(hr)) {
							ppSubPic = pSubPic;
						}
					} else {
						if (SUCCEEDED(RenderTo(m_pSubPic, rtStart, rtStop, fps, false))) {
							ppSubPic = pSubPic;
						}
					}
					if (SUCCEEDED(hr2)) {
						pSubPic->SetVirtualTextureSize (VirtualSize, VirtualTopLeft);
					}
				}
			}

			if (ppSubPic) {
				CAutoLock cAutoLock(&m_csLock);

				m_pSubPic = ppSubPic;
			}
		}
	}

	return(!!ppSubPic);
}

STDMETHODIMP CSubPicQueueNoThread::GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
	CAutoLock cAutoLock(&m_csLock);

	nSubPics = 0;
	rtNow = m_rtNow;
	rtStart = rtStop = 0;

	if (m_pSubPic) {
		nSubPics = 1;
		rtStart = m_pSubPic->GetStart();
		rtStop = m_pSubPic->GetStop();
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
