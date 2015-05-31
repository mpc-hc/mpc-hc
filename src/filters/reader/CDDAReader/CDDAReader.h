/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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

#include <vector>
#include <atlbase.h>
#include "winddk/devioctl.h"
#include "winddk/ntddcdrm.h"
#include <qnetwork.h>
#include "AsyncReader/asyncio.h"
#include "AsyncReader/asyncrdr.h"

#define CCDDAReaderName L"MPC-HC CDDA Reader"

struct ChunkHeader {
    UINT chunkID;
    long chunkSize;
};

#define RIFFID 'FFIR'
#define WAVEID 'EVAW'
struct RIFFChunk {
    ChunkHeader hdr;
    UINT WAVE;
};

#define FormatID ' tmf'
struct FormatChunk {
    ChunkHeader hdr;
    PCMWAVEFORMAT pcm;
};

#define DataID 'atad'
struct DataChunk {
    ChunkHeader hdr;
};

struct WAVEChunck {
    RIFFChunk riff;
    FormatChunk frm;
    DataChunk data;
};

class CCDDAStream : public CAsyncStream
{
private:
    CCritSec m_csLock;

    LONGLONG m_llPosition, m_llLength;

    HANDLE m_hDrive;
    CDROM_TOC m_TOC;
    UINT m_nFirstSector, m_nStartSector, m_nStopSector;
    bool m_bDTS;

    WAVEChunck m_header;

    std::vector<BYTE> m_buff;
public:
    CCDDAStream();
    virtual ~CCDDAStream();

    CString m_discTitle, m_trackTitle;
    CString m_discArtist, m_trackArtist;

    bool Load(const WCHAR* fnw);

    bool IsDTS() const { return m_bDTS; };

    // overrides
    HRESULT SetPointer(LONGLONG llPos);
    HRESULT Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead);
    LONGLONG Size(LONGLONG* pSizeAvailable);
    DWORD Alignment();
    void Lock();
    void Unlock();
};

class __declspec(uuid("54A35221-2C8D-4a31-A5DF-6D809847E393"))
    CCDDAReader
    : public CAsyncReader
    , public IFileSourceFilter
    , public IAMMediaContent
{
    CCDDAStream m_stream;
    CStringW m_fn;

public:
    CCDDAReader(IUnknown* pUnk, HRESULT* phr);
    ~CCDDAReader();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // CBaseFilter

    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);

    // IFileSourceFilter

    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);

    // IAMMediaContent

    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

    STDMETHODIMP get_AuthorName(BSTR* pbstrAuthorName);
    STDMETHODIMP get_Title(BSTR* pbstrTitle);
    STDMETHODIMP get_Rating(BSTR* pbstrRating);
    STDMETHODIMP get_Description(BSTR* pbstrDescription);
    STDMETHODIMP get_Copyright(BSTR* pbstrCopyright);
    STDMETHODIMP get_BaseURL(BSTR* pbstrBaseURL);
    STDMETHODIMP get_LogoURL(BSTR* pbstrLogoURL);
    STDMETHODIMP get_LogoIconURL(BSTR* pbstrLogoURL);
    STDMETHODIMP get_WatermarkURL(BSTR* pbstrWatermarkURL);
    STDMETHODIMP get_MoreInfoURL(BSTR* pbstrMoreInfoURL);
    STDMETHODIMP get_MoreInfoBannerImage(BSTR* pbstrMoreInfoBannerImage);
    STDMETHODIMP get_MoreInfoBannerURL(BSTR* pbstrMoreInfoBannerURL);
    STDMETHODIMP get_MoreInfoText(BSTR* pbstrMoreInfoText);
};
