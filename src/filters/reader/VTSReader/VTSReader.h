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
#include "AsyncReader/asyncio.h"
#include "AsyncReader/asyncrdr.h"

#include "ITrackInfo.h"
#include "../../../DSUtil/DSMPropertyBag.h"

#define VTSReaderName L"MPC-HC VTS Reader"

class CVobFile;

class CVTSStream : public CAsyncStream
{
private:
    CCritSec m_csLock;

    CAutoPtr<CVobFile> m_vob;
    int m_off;

public:
    CVTSStream();
    virtual ~CVTSStream();

    bool Load(const WCHAR* fnw);

    HRESULT SetPointer(LONGLONG llPos);
    HRESULT Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead);
    LONGLONG Size(LONGLONG* pSizeAvailable);
    DWORD Alignment();
    void Lock();
    void Unlock();

    BSTR GetTrackName(UINT aTrackIdx);
    int GetChaptersCount();
    REFERENCE_TIME GetChapterOffset(UINT ChapterNumber);
};

class __declspec(uuid("773EAEDE-D5EE-4fce-9C8F-C4F53D0A2F73"))
    CVTSReader
    : public CAsyncReader
    , public IFileSourceFilter
    , public ITrackInfo
    , public IDSMChapterBagImpl
{
    CVTSStream m_stream;
    CStringW m_fn;

public:
    CVTSReader(IUnknown* pUnk, HRESULT* phr);
    ~CVTSReader();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // CBaseFilter

    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);

    // IFileSourceFilter
    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);

    // ITrackInfo

    STDMETHODIMP_(UINT) GetTrackCount();
    STDMETHODIMP_(BOOL) GetTrackInfo(UINT aTrackIdx, struct TrackElement* pStructureToFill);
    STDMETHODIMP_(BOOL) GetTrackExtendedInfo(UINT aTrackIdx, void* pStructureToFill);
    STDMETHODIMP_(BSTR) GetTrackName(UINT aTrackIdx);
    STDMETHODIMP_(BSTR) GetTrackCodecID(UINT aTrackIdx);
    STDMETHODIMP_(BSTR) GetTrackCodecName(UINT aTrackIdx);
    STDMETHODIMP_(BSTR) GetTrackCodecInfoURL(UINT aTrackIdx);
    STDMETHODIMP_(BSTR) GetTrackCodecDownloadURL(UINT aTrackIdx);
};
