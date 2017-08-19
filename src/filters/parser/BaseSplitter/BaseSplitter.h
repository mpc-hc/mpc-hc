/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2017 see Authors.txt
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
#include <qnetwork.h>
#include "IKeyFrameInfo.h"
#include "IBufferInfo.h"
#include "IBitRateInfo.h"
#include "AsyncReader.h"
#include "../../../DSUtil/DSMPropertyBag.h"
#include "../../../DSUtil/FontInstaller.h"

#define MINPACKETS    100       // Beliyaal: Changed the min number of packets to allow Bluray playback over network
#define MINPACKETSIZE 256*1024  // Beliyaal: Changed the min packet size to allow Bluray playback over network
#define MAXPACKETS    2000
#define MAXPACKETSIZE 128*1024*1024

class Packet : public CAtlArray<BYTE>
{
public:
    DWORD TrackNumber;
    BOOL bDiscontinuity, bSyncPoint, bAppendable;
    static const REFERENCE_TIME INVALID_TIME = _I64_MIN;
    REFERENCE_TIME rtStart, rtStop;
    AM_MEDIA_TYPE* pmt;
    Packet()
        : TrackNumber(0)
        , bDiscontinuity(FALSE)
        , bSyncPoint(FALSE)
        , bAppendable(FALSE)
        , rtStart(0)
        , rtStop(0)
        , pmt(nullptr) {
    }
    virtual ~Packet() {
        if (pmt) {
            DeleteMediaType(pmt);
        }
    }
    virtual int GetDataSize() { return (int)GetCount(); }
    void SetData(const void* ptr, DWORD len) {
        SetCount(len);
        memcpy(GetData(), ptr, len);
    }
};

class CPacketQueue
    : public CCritSec
    , protected CAutoPtrList<Packet>
{
    int m_size;

public:
    CPacketQueue();
    void Add(CAutoPtr<Packet> p);
    CAutoPtr<Packet> Remove();
    void RemoveAll();
    int GetCount(), GetSize();
};

class CBaseSplitterFilter;

class CBaseSplitterInputPin
    : public CBasePin
{
protected:
    CComQIPtr<IAsyncReader> m_pAsyncReader;

public:
    CBaseSplitterInputPin(LPCTSTR pName, CBaseSplitterFilter* pFilter, CCritSec* pLock, HRESULT* phr);
    virtual ~CBaseSplitterInputPin();

    HRESULT GetAsyncReader(IAsyncReader** ppAsyncReader);

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    HRESULT CheckMediaType(const CMediaType* pmt);

    HRESULT CheckConnect(IPin* pPin);
    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin* pPin);

    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
};

class CBaseSplitterOutputPin
    : public CBaseOutputPin
    , public IDSMPropertyBagImpl
    , protected CAMThread
    , public IMediaSeeking
    , public IBitRateInfo
{
protected:
    CAtlArray<CMediaType> m_mts;
    int m_nBuffers;

private:
    CPacketQueue m_queue;

    HRESULT m_hrDeliver;

    bool m_fFlushing, m_fFlushed;
    CAMEvent m_eEndFlush;

    enum { CMD_EXIT };
    DWORD ThreadProc();

    void MakeISCRHappy();

    // please only use DeliverPacket from the derived class
    HRESULT GetDeliveryBuffer(IMediaSample** ppSample, REFERENCE_TIME* pStartTime, REFERENCE_TIME* pEndTime, DWORD dwFlags);
    HRESULT Deliver(IMediaSample* pSample);

    // IBitRateInfo
    struct BitRateInfo {
        UINT64 nTotalBytesDelivered         = 0;
        REFERENCE_TIME rtTotalTimeDelivered = 0;
        UINT64 nBytesSinceLastDeliverTime   = 0;
        REFERENCE_TIME rtLastDeliverTime    = Packet::INVALID_TIME;
        DWORD nCurrentBitRate               = 0;
        DWORD nAverageBitRate               = 0;
    } m_BitRate;

    int m_QueueMaxPackets;

protected:
    REFERENCE_TIME m_rtStart;

    // override this if you need some second level stream specific demuxing (optional)
    // the default implementation will send the sample as is
    virtual HRESULT DeliverPacket(CAutoPtr<Packet> p);

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

public:
    CBaseSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName,
                           CBaseFilter* pFilter, CCritSec* pLock,
                           HRESULT* phr, int nBuffers = 0,
                           int QueueMaxPackets = MAXPACKETS);
    CBaseSplitterOutputPin(LPCWSTR pName, CBaseFilter* pFilter,
                           CCritSec* pLock, HRESULT* phr,
                           int nBuffers = 0,
                           int QueueMaxPackets = MAXPACKETS);
    virtual ~CBaseSplitterOutputPin();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    HRESULT SetName(LPCWSTR pName);

    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);
    CMediaType& CurrentMediaType() { return m_mt; }

    STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);

    // Queueing

    HANDLE GetThreadHandle() {
        ASSERT(m_hThread != nullptr);
        return m_hThread;
    }
    void SetThreadPriority(int nPriority) {
        if (m_hThread) {
            ::SetThreadPriority(m_hThread, nPriority);
        }
    }

    HRESULT Active();
    HRESULT Inactive();

    HRESULT DeliverBeginFlush();
    HRESULT DeliverEndFlush();
    HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    int QueueCount();
    int QueueSize();
    HRESULT QueueEndOfStream();
    HRESULT QueuePacket(CAutoPtr<Packet> p);

    // returns true for everything which (the lack of) would not block other streams (subtitle streams, basically)
    virtual bool IsDiscontinuous();

    // returns IStreamsSwitcherInputPin::IsActive(), when it can find one downstream
    bool IsActive();

    // IBitRateInfo

    STDMETHODIMP_(DWORD) GetCurrentBitRate() { return m_BitRate.nCurrentBitRate; }
    STDMETHODIMP_(DWORD) GetAverageBitRate() { return m_BitRate.nAverageBitRate; }
};

class CBaseSplitterFilter
    : public CBaseFilter
    , public CCritSec
    , public IDSMPropertyBagImpl
    , public IDSMResourceBagImpl
    , public IDSMChapterBagImpl
    , protected CAMThread
    , public IFileSourceFilter
    , public IMediaSeeking
    , public IAMOpenProgress
    , public IAMMediaContent
    , public IAMExtendedSeeking
    , public IKeyFrameInfo
    , public IBufferInfo
{
    CCritSec m_csPinMap;
    CAtlMap<DWORD, CBaseSplitterOutputPin*> m_pPinMap;

    CCritSec m_csmtnew;
    CAtlMap<DWORD, CMediaType> m_mtnew;

    CAutoPtrList<CBaseSplitterOutputPin> m_pRetiredOutputs;

    CComQIPtr<ISyncReader> m_pSyncReader;

protected:
    CStringW m_fn;

    CAutoPtr<CBaseSplitterInputPin> m_pInput;
    CAutoPtrList<CBaseSplitterOutputPin> m_pOutputs;

    CBaseSplitterOutputPin* GetOutputPin(DWORD TrackNum);
    DWORD GetOutputTrackNum(CBaseSplitterOutputPin* pPin);
    HRESULT AddOutputPin(DWORD TrackNum, CAutoPtr<CBaseSplitterOutputPin> pPin);
    HRESULT RenameOutputPin(DWORD TrackNumSrc, DWORD TrackNumDst, const AM_MEDIA_TYPE* pmt);
    virtual HRESULT DeleteOutputs();
    virtual HRESULT CreateOutputs(IAsyncReader* pAsyncReader) = 0; // override this ...
    virtual LPCTSTR GetPartFilename(IAsyncReader* pAsyncReader);

    LONGLONG m_nOpenProgress;
    bool m_fAbort;

    REFERENCE_TIME m_rtDuration; // derived filter should set this at the end of CreateOutputs
    REFERENCE_TIME m_rtStart, m_rtStop, m_rtCurrent, m_rtNewStart, m_rtNewStop;
    double m_dRate;

    CAtlList<UINT64> m_bDiscontinuitySent;
    CAtlList<CBaseSplitterOutputPin*> m_pActivePins;

    CAMEvent m_eEndFlush;
    bool m_fFlushing;

    void DeliverBeginFlush();
    void DeliverEndFlush();
    HRESULT DeliverPacket(CAutoPtr<Packet> p);

    int m_priority;

    CFontInstaller m_fontinst;

    int m_QueueMaxPackets;

protected:
    enum { CMD_EXIT, CMD_SEEK };
    DWORD ThreadProc();

    // ... and also override all these too
    virtual bool DemuxInit() = 0;
    virtual void DemuxSeek(REFERENCE_TIME rt) = 0;
    virtual bool DemuxLoop() = 0;
    virtual bool BuildPlaylist(LPCTSTR pszFileName, CAtlList<CHdmvClipInfo::PlaylistItem>& Items) { return false; };
    virtual bool BuildChapters(LPCTSTR pszFileName, CAtlList<CHdmvClipInfo::PlaylistItem>& PlaylistItems, CAtlList<CHdmvClipInfo::PlaylistChapter>& Items) { return false; };

public:
    CBaseSplitterFilter(LPCTSTR pName, LPUNKNOWN pUnk, HRESULT* phr,
                        const CLSID& clsid, int QueueMaxPackets = MAXPACKETS);
    virtual ~CBaseSplitterFilter();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    bool IsAnyPinDrying();

    HRESULT BreakConnect(PIN_DIRECTION dir, CBasePin* pPin);
    HRESULT CompleteConnect(PIN_DIRECTION dir, CBasePin* pPin);

    int GetPinCount();
    CBasePin* GetPin(int n);

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

    // IFileSourceFilter

    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);

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

protected:
    friend class CBaseSplitterOutputPin;
    virtual HRESULT SetPositionsInternal(void* id, LONGLONG* pCurrent,
                                         DWORD dwCurrentFlags, LONGLONG* pStop,
                                         DWORD dwStopFlags);

private:
    REFERENCE_TIME m_rtLastStart, m_rtLastStop;
    CAtlList<void*> m_LastSeekers;

public:
    // IAMOpenProgress

    STDMETHODIMP QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent);
    STDMETHODIMP AbortOperation();

    // IDispatch

    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid) { return E_NOTIMPL; }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) { return E_NOTIMPL; }

    // IAMMediaContent

    STDMETHODIMP get_AuthorName(BSTR* pbstrAuthorName);
    STDMETHODIMP get_Title(BSTR* pbstrTitle);
    STDMETHODIMP get_Rating(BSTR* pbstrRating);
    STDMETHODIMP get_Description(BSTR* pbstrDescription);
    STDMETHODIMP get_Copyright(BSTR* pbstrCopyright);
    STDMETHODIMP get_BaseURL(BSTR* pbstrBaseURL) { return E_NOTIMPL; }
    STDMETHODIMP get_LogoURL(BSTR* pbstrLogoURL) { return E_NOTIMPL; }
    STDMETHODIMP get_LogoIconURL(BSTR* pbstrLogoURL) { return E_NOTIMPL; }
    STDMETHODIMP get_WatermarkURL(BSTR* pbstrWatermarkURL) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoURL(BSTR* pbstrMoreInfoURL) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoBannerImage(BSTR* pbstrMoreInfoBannerImage) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoBannerURL(BSTR* pbstrMoreInfoBannerURL) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoText(BSTR* pbstrMoreInfoText) { return E_NOTIMPL; }

    // IAMExtendedSeeking

    STDMETHODIMP get_ExSeekCapabilities(long* pExCapabilities);
    STDMETHODIMP get_MarkerCount(long* pMarkerCount);
    STDMETHODIMP get_CurrentMarker(long* pCurrentMarker);
    STDMETHODIMP GetMarkerTime(long MarkerNum, double* pMarkerTime);
    STDMETHODIMP GetMarkerName(long MarkerNum, BSTR* pbstrMarkerName);
    STDMETHODIMP put_PlaybackSpeed(double Speed) { return E_NOTIMPL; }
    STDMETHODIMP get_PlaybackSpeed(double* pSpeed) { return E_NOTIMPL; }

    // IKeyFrameInfo

    STDMETHODIMP_(HRESULT) GetKeyFrameCount(UINT& nKFs);
    STDMETHODIMP_(HRESULT) GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs);

    // IBufferInfo

    STDMETHODIMP_(int) GetCount();
    STDMETHODIMP GetStatus(int i, int& samples, int& size);
    STDMETHODIMP_(DWORD) GetPriority();
};
