/* 
 *	Copyright (C) 2003-2006 Gabest
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
 *
 */

#include "stdafx.h"
#include "D2VSource.h"
#include "mpeg2dec.h"
#include "..\..\..\DSUtil\DSUtil.h"

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] =
{
	{&MEDIATYPE_Video, &MEDIASUBTYPE_YUY2}
};

const AMOVIESETUP_PIN sudOpPin[] =
{
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CD2VSource), L"D2VSource", MERIT_NORMAL, countof(sudOpPin), sudOpPin}
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CD2VSource>, NULL, &sudFilter[0]}
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{47CE0591-C4D5-4b41-BED7-28F59AD76228}"), 
		_T("0"), _T("0,18,,4456443241564950726F6A65637446696C65")); // "DVD2AVIProjectFile"

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{47CE0591-C4D5-4b41-BED7-28F59AD76228}"), 
		_T("Source Filter"), _T("{47CE0591-C4D5-4b41-BED7-28F59AD76228}"));

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".d2v"), 
		_T("Source Filter"), _T("{47CE0591-C4D5-4b41-BED7-28F59AD76228}"));

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{47CE0591-C4D5-4b41-BED7-28F59AD76228}"));
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".d2v"));

	return AMovieDllRegisterServer2(FALSE);
}

#include "..\..\FilterApp.h"

CFilterApp theApp;

#endif

//
// CD2VSource
//

CD2VSource::CD2VSource(LPUNKNOWN lpunk, HRESULT* phr)
	: CBaseSource<CD2VStream>(NAME("CD2VSource"), lpunk, phr, __uuidof(this))
{
	if(phr) *phr = S_OK;
}

CD2VSource::~CD2VSource()
{
}

//
// CD2VStream
//

CD2VStream::CD2VStream(const WCHAR* fn, CSource* pParent, HRESULT* phr) 
	: CBaseStream(NAME("D2VSourceStream"), pParent, phr)
	, m_pFrameBuffer(NULL)
{
	CAutoLock cAutoLock(&m_cSharedState);

	m_pDecoder.Attach(new CMPEG2Dec());
	if(!m_pDecoder)
	{
		if(phr) *phr = E_OUTOFMEMORY;
		return;
	}

	if(!m_pDecoder->Open(CString(fn), CMPEG2Dec::YUY2))
	{
		if(phr) *phr = E_FAIL;
		return;
	}

	if(!m_pFrameBuffer.Allocate(m_pDecoder->Clip_Width*m_pDecoder->Clip_Height*4))
	{
		if(phr) *phr = E_OUTOFMEMORY;
		return;
	}

	m_AvgTimePerFrame = 10000000000i64/m_pDecoder->VF_FrameRate;
	m_rtDuration = m_rtStop = m_AvgTimePerFrame*m_pDecoder->VF_FrameLimit;

	if(phr) *phr = m_rtDuration > 0 ? S_OK : E_FAIL;
}

CD2VStream::~CD2VStream()
{
	CAutoLock cAutoLock(&m_cSharedState);
}

HRESULT CD2VStream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
//    CAutoLock cAutoLock(m_pFilter->pStateLock());

    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

	int w, h, bpp;
	if(!GetDim(w, h, bpp))
		return E_FAIL;

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = w*h*bpp>>3;

    ALLOCATOR_PROPERTIES Actual;
    if(FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) return hr;

    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;
    ASSERT(Actual.cBuffers == pProperties->cBuffers);

    return NOERROR;
}

HRESULT CD2VStream::FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len)
{
	if(!m_pDecoder)
		return S_FALSE;

	AM_MEDIA_TYPE* pmt;
	if(SUCCEEDED(pSample->GetMediaType(&pmt)) && pmt)
	{
		CMediaType mt(*pmt);
		SetMediaType(&mt);

		DeleteMediaType(pmt);
	}

	int w, h, bpp;
	if(!GetDim(w, h, bpp))
		return S_FALSE;

	BYTE* pIn = m_pFrameBuffer;

	int pitchIn, pitchOut = 0;

	pitchIn = m_pDecoder->Clip_Width*bpp>>3;
	pitchOut = w*bpp>>3;

	m_pDecoder->Decode(pIn, (unsigned long)(nFrame), pitchIn);

	for(int y = 0, p = min(pitchIn, pitchOut); 
		y < h; 
		y++, pIn += pitchIn, pOut += pitchOut)
	{
		memcpy(pOut, pIn, p);
	}

	len = pitchOut*h;

	return S_OK;
}

HRESULT CD2VStream::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if(iPosition < 0) return E_INVALIDARG;
    if(iPosition > 0) return VFW_S_NO_MORE_ITEMS;

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetSubtype(&MEDIASUBTYPE_YUY2);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
	memset(vih, 0, sizeof(VIDEOINFOHEADER));
	vih->AvgTimePerFrame = m_AvgTimePerFrame;
	vih->bmiHeader.biSize = sizeof(vih->bmiHeader);
	vih->bmiHeader.biWidth = m_pDecoder->Clip_Width;
	vih->bmiHeader.biHeight = m_pDecoder->Clip_Height;
	vih->bmiHeader.biPlanes = 1;
	vih->bmiHeader.biBitCount = 16;
	vih->bmiHeader.biCompression = '2YUY';
	vih->bmiHeader.biSizeImage = vih->bmiHeader.biWidth*abs(vih->bmiHeader.biHeight)*vih->bmiHeader.biBitCount>>3;

	pmt->SetSampleSize(vih->bmiHeader.biSizeImage);

    return NOERROR;
}

HRESULT CD2VStream::SetMediaType(const CMediaType* pmt)
{
	if(m_pDecoder)
	{
		if(pmt->subtype == MEDIASUBTYPE_YUY2)
			m_pDecoder->m_dstFormat = CMPEG2Dec::YUY2;
		else
			return E_FAIL;
	}

	return CSourceStream::SetMediaType(pmt);
}

HRESULT CD2VStream::CheckMediaType(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Video
		&& pmt->subtype == MEDIASUBTYPE_YUY2
		&& pmt->formattype == FORMAT_VideoInfo
		? S_OK
		: E_INVALIDARG;
}

STDMETHODIMP CD2VStream::Notify(IBaseFilter* pSender, Quality q)
{
	if(q.Late > 0 && q.Late < 100000000)
	{
		CAutoLock cAutoLockShared(&m_cSharedState);

        m_rtSampleTime += (q.Late/m_AvgTimePerFrame)*m_AvgTimePerFrame;
        m_rtPosition += (q.Late/m_AvgTimePerFrame)*m_AvgTimePerFrame;
	}

	return S_OK;
}

//

bool CD2VStream::GetDim(int& w, int& h, int& bpp)
{
	if(m_mt.formattype == FORMAT_VideoInfo)
	{
		w = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biWidth;
		h = abs(((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biHeight);
		bpp = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biBitCount;
	}
	else if(m_mt.formattype == FORMAT_VideoInfo2)
	{
		w = ((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biWidth;
		h = abs(((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biHeight);
		bpp = ((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biBitCount;
	}
	else
	{
		return(false);
	}

	return(true);
}
