/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
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

#pragma once

#include <atlbase.h>

#define BufferFilterName L"MPC-HC Buffer Filter"

interface __declspec(uuid("63EF0035-3FFE-4c41-9230-4346E028BE20"))
    IBufferFilter :
    public IUnknown
{
    STDMETHOD(SetBuffers)(int nBuffers) PURE;
    STDMETHOD_(int, GetBuffers)() PURE;
    STDMETHOD_(int, GetFreeBuffers)() PURE;
    STDMETHOD(SetPriority)(DWORD dwPriority = THREAD_PRIORITY_NORMAL) PURE;
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

        int GetQueueCount() { return m_List ? m_List->GetCount() : -1; }

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
