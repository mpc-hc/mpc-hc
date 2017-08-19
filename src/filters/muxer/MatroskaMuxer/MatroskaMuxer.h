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
#include <atlcoll.h>
#include "MatroskaFile.h"

#define MAXCLUSTERTIME 1000
#define MAXBLOCKS 50

#define MatroskaMuxerName L"MPC Matroska Muxer"

class CMatroskaMuxerInputPin : public CBaseInputPin
{
    CAutoPtr<MatroskaWriter::TrackEntry> m_pTE;
    CAutoPtrArray<MatroskaWriter::CBinary> m_pVorbisHdrs;

    bool m_fActive;
    CCritSec m_csReceive;

    REFERENCE_TIME m_rtLastStart, m_rtLastStop;

public:
    CMatroskaMuxerInputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
    virtual ~CMatroskaMuxerInputPin();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    MatroskaWriter::TrackEntry* GetTrackEntry() { return m_pTE; }

    REFERENCE_TIME m_rtDur;

    CCritSec m_csQueue;
    CAutoPtrList<MatroskaWriter::BlockGroup> m_blocks;
    bool m_fEndOfStreamReceived;

    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin* pPin);
    HRESULT Active(), Inactive();

    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();

    STDMETHODIMP Receive(IMediaSample* pSample);
    STDMETHODIMP EndOfStream();
};

class CMatroskaMuxerOutputPin : public CBaseOutputPin
{
public:
    CMatroskaMuxerOutputPin(LPCTSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
    virtual ~CMatroskaMuxerOutputPin();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties);

    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);

    STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);
};

interface __declspec(uuid("38E2D43D-915D-493C-B373-888DB16EE3DC"))
    IMatroskaMuxer :
    public IUnknown
{
    STDMETHOD(CorrectTimeOffset)(bool fNegative, bool fPositive) PURE;
    // TODO: chapters
};

class __declspec(uuid("1E1299A2-9D42-4F12-8791-D79E376F4143"))
    CMatroskaMuxerFilter
    : public CBaseFilter
    , public CCritSec
    , public CAMThread
    , public IAMFilterMiscFlags
    , public IMediaSeeking
    , public IMatroskaMuxer
{
protected:
    CAutoPtrList<CMatroskaMuxerInputPin> m_pInputs;
    CAutoPtr<CMatroskaMuxerOutputPin> m_pOutput;

    REFERENCE_TIME m_rtCurrent;

    bool m_fNegative, m_fPositive;

    enum { CMD_EXIT, CMD_RUN };
    DWORD ThreadProc();

public:
    CMatroskaMuxerFilter(LPUNKNOWN pUnk, HRESULT* phr);
    virtual ~CMatroskaMuxerFilter();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    void AddInput();
    UINT GetTrackNumber(const CBasePin* pPin);

    int GetPinCount();
    CBasePin* GetPin(int n);

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

    // IAMFilterMiscFlags

    STDMETHODIMP_(ULONG) GetMiscFlags();

    // IMediaSeeking

    STDMETHODIMP GetCapabilities(DWORD* pCapabilities);
    STDMETHODIMP CheckCapabilities(DWORD* pCapabilities);
    STDMETHODIMP IsFormatSupported(const GUID* pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID* pFormat);
    STDMETHODIMP GetTimeFormat(GUID* pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID* pFormat);
    STDMETHODIMP SetTimeFormat(const GUID* pFormat);
    STDMETHODIMP GetDuration(LONGLONG* pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG* pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG* pCurrent);
    STDMETHODIMP ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat,
                                   LONGLONG Source, const GUID* pSourceFormat);
    STDMETHODIMP SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags,
                              LONGLONG* pStop, DWORD dwStopFlags);
    STDMETHODIMP GetPositions(LONGLONG* pCurrent, LONGLONG* pStop);
    STDMETHODIMP GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest);
    STDMETHODIMP SetRate(double dRate);
    STDMETHODIMP GetRate(double* pdRate);
    STDMETHODIMP GetPreroll(LONGLONG* pllPreroll);

    // IMatroskaMuxer

    STDMETHODIMP CorrectTimeOffset(bool fNegative, bool fPositive);
};
