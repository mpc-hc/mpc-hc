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

#define CCDXAReaderName L"MPC-HC CDXA Reader"

class CCDXAStream : public CAsyncStream
{
private:
    enum {
        RIFFCDXA_HEADER_SIZE = 44,  // usually...
        RAW_SYNC_SIZE = 12,         // 00 FF .. FF 00
        RAW_HEADER_SIZE = 4,
        RAW_SUBHEADER_SIZE = 8,
        RAW_DATA_SIZE = 2324,
        RAW_EDC_SIZE = 4,
        RAW_SECTOR_SIZE = 2352
    };

    CCritSec m_csLock;

    HANDLE m_hFile;
    LONGLONG m_llPosition, m_llLength;
    int m_nFirstSector;

    int m_nBufferedSector;
    BYTE m_sector[RAW_SECTOR_SIZE];

    bool LookForMediaSubType();

public:
    CCDXAStream();
    virtual ~CCDXAStream();

    bool Load(const WCHAR* fnw);

    HRESULT SetPointer(LONGLONG llPos);
    HRESULT Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead);
    LONGLONG Size(LONGLONG* pSizeAvailable);
    DWORD Alignment();
    void Lock();
    void Unlock();

    GUID m_subtype;
};

class __declspec(uuid("D367878E-F3B8-4235-A968-F378EF1B9A44"))
    CCDXAReader
    : public CAsyncReader
    , public IFileSourceFilter
{
    CCDXAStream m_stream;
    CStringW m_fn;

public:
    CCDXAReader(IUnknown* pUnk, HRESULT* phr);
    ~CCDXAReader();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // CBaseFilter

    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);

    // IFileSourceFilter

    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);
};
