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

#pragma once

#include <atlbase.h>
#include "../../../Subtitles/RTS.h"

#define SubtitleSourceName   L"MPC-HC Subtitle Source"

class CSubtitleSource
    : public CSource
    , public IFileSourceFilter
    , public IAMFilterMiscFlags
{
protected:
    CStringW m_fn;

public:
    CSubtitleSource(LPUNKNOWN lpunk, HRESULT* phr, const CLSID& clsid);
    virtual ~CSubtitleSource();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // IFileSourceFilter
    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);

    // IAMFilterMiscFlags
    STDMETHODIMP_(ULONG) GetMiscFlags();

    virtual HRESULT GetMediaType(CMediaType* pmt) = 0;

    // CBaseFilter
    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);
};

class CSubtitleStream
    : public CSourceStream
    , public CSourceSeeking
{
    CCritSec m_cSharedState;

    int m_nPosition;

    BOOL m_bDiscontinuity, m_bFlushing;

    HRESULT OnThreadStartPlay();
    HRESULT OnThreadCreate();

    void UpdateFromSeek();
    STDMETHODIMP SetRate(double dRate);

    HRESULT ChangeStart();
    HRESULT ChangeStop();
    HRESULT ChangeRate() { return S_OK; }

protected:
    CRenderedTextSubtitle m_rts;

public:
    CSubtitleStream(const WCHAR* wfn, CSubtitleSource* pParent, HRESULT* phr);
    virtual ~CSubtitleStream();

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT FillBuffer(IMediaSample* pSample);
    HRESULT GetMediaType(CMediaType* pmt);
    HRESULT CheckMediaType(const CMediaType* pmt);

    STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);
};

class __declspec(uuid("E44CA3B5-A0FF-41A0-AF16-42429B1095EA"))
    CSubtitleSourceASCII : public CSubtitleSource
{
public:
    CSubtitleSourceASCII(LPUNKNOWN lpunk, HRESULT* phr);

    HRESULT GetMediaType(CMediaType* pmt);
};

class __declspec(uuid("87864E0F-7073-4E39-B802-143DE0ED4964"))
    CSubtitleSourceUTF8 : public CSubtitleSource
{
public:
    CSubtitleSourceUTF8(LPUNKNOWN lpunk, HRESULT* phr);

    HRESULT GetMediaType(CMediaType* pmt);
};

class __declspec(uuid("18316B1A-5877-4CC4-85FD-EDE65CD489EC"))
    CSubtitleSourceSSA : public CSubtitleSource
{
public:
    CSubtitleSourceSSA(LPUNKNOWN lpunk, HRESULT* phr);

    HRESULT GetMediaType(CMediaType* pmt);
};

class __declspec(uuid("416782BC-1D87-48C0-8F65-F113A5CB8E15"))
    CSubtitleSourceASS : public CSubtitleSource
{
public:
    CSubtitleSourceASS(LPUNKNOWN lpunk, HRESULT* phr);

    HRESULT GetMediaType(CMediaType* pmt);
};

class __declspec(uuid("D7215AFC-DFE6-483B-9AF3-6BBECFF14CF4"))
    CSubtitleSourceUSF : public CSubtitleSource
{
public:
    CSubtitleSourceUSF(LPUNKNOWN lpunk, HRESULT* phr);

    HRESULT GetMediaType(CMediaType* pmt);
};

class __declspec(uuid("932E75D4-BBD4-4A0F-9071-6728FBDC4C98"))
    CSubtitleSourcePreview : public CSubtitleSource
{
public:
    CSubtitleSourcePreview(LPUNKNOWN lpunk, HRESULT* phr);

    HRESULT GetMediaType(CMediaType* pmt);
};

class __declspec(uuid("CF0D7280-527D-415E-BA02-56017484D73E"))
    CSubtitleSourceARGB : public CSubtitleSource
{
public:
    CSubtitleSourceARGB(LPUNKNOWN lpunk, HRESULT* phr);

    HRESULT GetMediaType(CMediaType* pmt);
};
