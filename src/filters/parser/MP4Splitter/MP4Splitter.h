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

#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include "MP4SplitterFile.h"
#include "../BaseSplitter/BaseSplitter.h"

#define MP4SplitterName L"MPC MP4/MOV Splitter"
#define MP4SourceName   L"MPC MP4/MOV Source"

class __declspec(uuid("61F47056-E400-43d3-AF1E-AB7DFFD4C4AD"))
    CMP4SplitterFilter : public CBaseSplitterFilter
{
    struct trackpos {
        DWORD /*AP4_Ordinal*/ index;
        unsigned __int64 /*AP4_TimeStamp*/ ts;
    };
    CAtlMap<DWORD, trackpos> m_trackpos;
    CSize m_framesize;

protected:
    CAutoPtr<CMP4SplitterFile> m_pFile;
    HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

    bool DemuxInit();
    void DemuxSeek(REFERENCE_TIME rt);
    bool DemuxLoop();

public:
    CMP4SplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);
    virtual ~CMP4SplitterFilter();

    // CBaseFilter
    STDMETHODIMP_(HRESULT) QueryFilterInfo(FILTER_INFO* pInfo);

    // IKeyFrameInfo

    STDMETHODIMP_(HRESULT) GetKeyFrameCount(UINT& nKFs);
    STDMETHODIMP_(HRESULT) GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs);
};

class __declspec(uuid("3CCC052E-BDEE-408a-BEA7-90914EF2964B"))
    CMP4SourceFilter : public CMP4SplitterFilter
{
public:
    CMP4SourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

// for raw mpeg4 elementary streams:

class __declspec(uuid("D3D9D58B-45B5-48AB-B199-B8C40560AEC7"))
    CMPEG4VideoSplitterFilter : public CBaseSplitterFilter
{
    __int64 m_seqhdrsize;
    int NextStartCode();
    void SkipUserData();

protected:
    CAutoPtr<CBaseSplitterFileEx> m_pFile;
    HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

    bool DemuxInit();
    void DemuxSeek(REFERENCE_TIME rt);
    bool DemuxLoop();

public:
    CMPEG4VideoSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("E2B98EEA-EE55-4E9B-A8C1-6E5288DF785A"))
    CMPEG4VideoSourceFilter : public CMPEG4VideoSplitterFilter
{
public:
    CMPEG4VideoSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

class CMP4SplitterOutputPin : public CBaseSplitterOutputPin, protected CCritSec
{
public:
    CMP4SplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
    virtual ~CMP4SplitterOutputPin();

    HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    HRESULT DeliverPacket(CAutoPtr<Packet> p);
    HRESULT DeliverEndFlush();
};
