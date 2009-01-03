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

#include "stdafx.h"
#include <mmreg.h>
#include <ks.h>
#include <initguid.h>
#include <uuids.h>
#include <moreuuids.h>
#include "flacsource.h"
#include "..\..\..\DSUtil\DSUtil.h"

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_FLAC_FRAMED}
};

const AMOVIESETUP_PIN sudOpPin[] =
{
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CFlacSource), L"MPC - Flac Source", MERIT_NORMAL, countof(sudOpPin), sudOpPin}
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CFlacSource>, NULL, &sudFilter[0]}
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"), 
		_T("0"), _T("0,4,,664C6143"));

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"), 
		_T("Source Filter"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"));

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".flac"), 
		_T("Source Filter"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"));

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"));
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".flac"));

	return AMovieDllRegisterServer2(FALSE);
}

#include "..\..\FilterApp.h"

CFilterApp theApp;

#endif

//
// CFlacSource
//

CFlacSource::CFlacSource(LPUNKNOWN lpunk, HRESULT* phr)
	: CBaseSource<CFlacStream>(NAME("CFlacSource"), lpunk, phr, __uuidof(this))
{
}

CFlacSource::~CFlacSource()
{
}

// CFlacStream

CFlacStream::CFlacStream(const WCHAR* wfn, CSource* pParent, HRESULT* phr) 
	: CBaseStream(NAME("CFlacStream"), pParent, phr)
	, m_nFileOffset(0)
{
	CAutoLock cAutoLock(&m_cSharedState);

	m_subtype		= MEDIASUBTYPE_FLAC_FRAMED;
	m_wFormatTag	= WAVE_FORMAT_FLAC;
	m_streamid		= 0;

	CString fn(wfn);

	if(!m_file.Open(fn, CFile::modeRead|CFile::shareDenyWrite))
	{
		if(phr) *phr = E_FAIL;
		return;
	}

	//CT> Added header check (for FLAC files with ID3 v1/2 tags in them)
	//    We'll look in the first 128kb of the file
	unsigned long locStart = 0;
	int locHeaderFound = 0;
	for(int j = 0; !locHeaderFound && j < 128; j++)	
	{
		unsigned char locTempBuf[1024]={0,};
		m_file.Read((char*)&locTempBuf, sizeof(locTempBuf));
		unsigned char* locPtr = locTempBuf;
		for(int i = 0; i < 1023; i++) 
		{
			if(locPtr[i]=='f' && locPtr[i+1]=='L' && locPtr[i+2]=='a' && locPtr[i+3]=='C')
			{
				locHeaderFound = 1;
				locStart = i + (j * 1024);
				break;
			}
		}
	}
	if(!locHeaderFound)
	{
		if(phr) *phr = E_FAIL;
		return;
	}


	m_file.Seek (locStart, CFile::begin);

	unsigned char locBuff[64];
	m_file.Read((char*)&locBuff, 64);
	const unsigned char FLAC_CHANNEL_MASK	= 14;	//00001110
	const unsigned char FLAC_BPS_START_MASK = 1;	//00000001
	const unsigned char FLAC_BPS_END_MASK	= 240;  //11110000

	m_nChannels			 = (((locBuff[20]) & FLAC_CHANNEL_MASK) >> 1) + 1;
	m_nSamplesPerSec	 = (charArrToULong(&locBuff[18])) >> 12;
	m_wBitsPerSample	 =	(((locBuff[20] & FLAC_BPS_START_MASK) << 4)	| ((locBuff[21] & FLAC_BPS_END_MASK) >> 4)) + 1;	
	__int64 mTotalNumSamples = (((__int64)(locBuff[21] % 16)) << 32) + ((__int64)(charArrToULong(&locBuff[22])));

	m_nAvgBytesPerSec	= (m_nChannels * (m_wBitsPerSample >> 3)) * m_nSamplesPerSec;
	
	m_nMaxFrameSize		= locBuff[15]<<16 | locBuff[16]<<8 | locBuff[17];
	m_nBytesPerFrame	= m_nMaxFrameSize;	// TODO <==
	m_AvgTimePerFrame	= 10000000i64 * m_nBytesPerFrame / m_nAvgBytesPerSec;

/*	
//m_AvgTimePerFrame = 10000000i64 * m_nBytesPerFrame * 8 / bitratetbl[transbitrate];
	m_nBytesPerFrame;
	m_AvgTimePerFrame;
*/

	m_rtDuration = (mTotalNumSamples * UNITS) / m_nSamplesPerSec;
	m_rtStop	 = m_rtDuration;
}

CFlacStream::~CFlacStream()
{
}

unsigned long CFlacStream::charArrToULong(unsigned char* inCharArray)
{
	//Turns the next four bytes from the pointer in a long MSB (most sig. byte first/leftmost)
	unsigned long locVal = 0;
	for (int i = 0; i < 4; i++) 
	{
		locVal <<= 8;
		locVal += inCharArray[i];
	}
	return locVal;
}

HRESULT CFlacStream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = m_nMaxFrameSize;

    ALLOCATOR_PROPERTIES Actual;
    if(FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) return hr;

    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;
    ASSERT(Actual.cBuffers == pProperties->cBuffers);

    return NOERROR;
}

HRESULT CFlacStream::FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len)
{
	BYTE* pOutOrg = pOut;

	const GUID* majortype = &m_mt.majortype;
	const GUID* subtype = &m_mt.subtype;

	if(*majortype == MEDIATYPE_Audio)
	{
		m_file.Seek(m_nFileOffset + nFrame*m_nBytesPerFrame, CFile::begin);
		if(m_file.Read(pOut, m_nBytesPerFrame) < m_nBytesPerFrame) return S_FALSE;
		pOut += m_nBytesPerFrame;
	}

	len = pOut - pOutOrg;

	return S_OK;
}


HRESULT CFlacStream::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

	if(iPosition == 0)
	{
		pmt->majortype			= MEDIATYPE_Audio;
		pmt->subtype			= m_subtype;
		pmt->formattype			= FORMAT_WaveFormatEx;
		WAVEFORMATEX* wfe		= (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		memset(wfe, 0, sizeof(WAVEFORMATEX));
		wfe->cbSize = sizeof(WAVEFORMATEX);
		wfe->wFormatTag			= m_wFormatTag;
		wfe->nSamplesPerSec		= m_nSamplesPerSec;
		wfe->nAvgBytesPerSec	= m_nAvgBytesPerSec;
		wfe->nChannels			= m_nChannels;
		wfe->nBlockAlign		= 1;
		wfe->wBitsPerSample		= m_wBitsPerSample;
	}
	else
	{
		return VFW_S_NO_MORE_ITEMS;
	}

    pmt->SetTemporalCompression(FALSE);

	return S_OK;
}

HRESULT CFlacStream::CheckMediaType(const CMediaType* pmt)
{
	if (   pmt->majortype  == MEDIATYPE_Audio
		&& pmt->subtype    == MEDIASUBTYPE_FLAC_FRAMED
		&& pmt->formattype == FORMAT_WaveFormatEx
		&& ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_FLAC)
		return S_OK;
	else
		return E_INVALIDARG;
}
