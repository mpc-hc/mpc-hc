/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
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
#include <winddk/devioctl.h>
#include <winddk/ntddcdrm.h>
#include <qnetwork.h>
#include "../asyncreader/asyncio.h"
#include "../asyncreader/asyncrdr.h"

typedef struct {UINT chunkID; long chunkSize;} ChunkHeader;

#define RIFFID 'FFIR' 
#define WAVEID 'EVAW' 
typedef struct {ChunkHeader hdr; UINT WAVE;} RIFFChunk;

#define FormatID ' tmf' 
typedef struct {ChunkHeader hdr; PCMWAVEFORMAT pcm;} FormatChunk;

#define DataID 'atad'
typedef struct {ChunkHeader hdr;} DataChunk;

typedef struct {RIFFChunk riff; FormatChunk frm; DataChunk data;} WAVEChunck;

class CCDDAStream : public CAsyncStream
{
private:
    CCritSec m_csLock;

	LONGLONG m_llPosition, m_llLength;

	HANDLE m_hDrive;
	CDROM_TOC m_TOC;
	UINT m_nFirstSector, m_nStartSector, m_nStopSector;

	WAVEChunck m_header;

public:
	CCDDAStream();
	virtual ~CCDDAStream();

	CString m_discTitle, m_trackTitle;
	CString m_discArtist, m_trackArtist;

	bool Load(const WCHAR* fnw);

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

