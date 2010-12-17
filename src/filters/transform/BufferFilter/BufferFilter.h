/*
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include <atlbase.h>

interface __declspec(uuid("63EF0035-3FFE-4c41-9230-4346E028BE20"))
IBufferFilter :
public IUnknown {
	STDMETHOD(SetBuffers) (int nBuffers) = 0;
	STDMETHOD_(int, GetBuffers) () = 0;
	STDMETHOD_(int, GetFreeBuffers) () = 0;
	STDMETHOD(SetPriority) (DWORD dwPriority = THREAD_PRIORITY_NORMAL) = 0;
};

class __declspec(uuid("DA2B3D77-2F29-4fd2-AC99-DEE4A8A13BF0"))
	CBufferFilter : public CTransformFilter, public IBufferFilter
{
	int m_nSamplesToBuffer;

public:
	CBufferFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CBufferFilter();

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// IBufferFilter
	STDMETHODIMP SetBuffers(int nBuffers);
	STDMETHODIMP_(int) GetBuffers();
	STDMETHODIMP_(int) GetFreeBuffers();
	STDMETHODIMP SetPriority(DWORD dwPriority = THREAD_PRIORITY_NORMAL);

	HRESULT Receive(IMediaSample* pSample);
	HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);
	HRESULT StopStreaming();
};

class CBufferFilterOutputPin : public CTransformOutputPin
{
	class CBufferFilterOutputQueue : public COutputQueue
	{
	public:
		CBufferFilterOutputQueue(IPin* pInputPin, HRESULT* phr,
								 DWORD dwPriority = THREAD_PRIORITY_NORMAL,
								 BOOL bAuto = FALSE, BOOL bQueue = TRUE,
								 LONG lBatchSize = 1, BOOL bBatchExact = FALSE,
								 LONG lListSize = DEFAULTCACHE,
								 bool bFlushingOpt = false)
			: COutputQueue(pInputPin, phr, bAuto, bQueue, lBatchSize, bBatchExact, lListSize, dwPriority, bFlushingOpt) {
		}

		int GetQueueCount() {
			return m_List ? m_List->GetCount() : -1;
		}

		bool SetPriority(DWORD dwPriority) {
			return m_hThread ? !!::SetThreadPriority(m_hThread, dwPriority) : false;
		}
	};

public:
	CBufferFilterOutputPin(CTransformFilter* pFilter, HRESULT* phr);

	CAutoPtr<CBufferFilterOutputQueue> m_pOutputQueue;

	HRESULT Active();
	HRESULT Inactive();

	HRESULT Deliver(IMediaSample* pMediaSample);
	HRESULT DeliverEndOfStream();
	HRESULT DeliverBeginFlush();
	HRESULT DeliverEndFlush();
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
};
