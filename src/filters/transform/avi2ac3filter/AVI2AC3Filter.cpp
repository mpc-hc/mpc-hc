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
#include <atlbase.h>
#include "avi2ac3filter.h"
#include "..\..\..\DSUtil\DSUtil.h"

#include <initguid.h>
#include "..\..\..\..\include\moreuuids.h"

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_WAVE_DOLBY_AC3},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_WAVE_DTS},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CAVI2AC3Filter), L"AVI<->AC3/DTS", MERIT_NORMAL, countof(sudpPins), sudpPins}
};

CFactoryTemplate g_Templates[] =
{
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CAVI2AC3Filter>, NULL, &sudFilter[0]}
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

#include "..\..\FilterApp.h"

CFilterApp theApp;

#endif

//
// CAVI2AC3Filter
//

CAVI2AC3Filter::CAVI2AC3Filter(LPUNKNOWN lpunk, HRESULT* phr)
	: CTransformFilter(NAME("CAVI2AC3Filter"), lpunk, __uuidof(this))	
{
	if(phr) *phr = S_OK;
}

CAVI2AC3Filter::~CAVI2AC3Filter()
{
}

HRESULT CAVI2AC3Filter::Transform(IMediaSample* pSample, IMediaSample* pOutSample)
{
	HRESULT hr;

	BYTE* pIn = NULL;
	if(FAILED(hr = pSample->GetPointer(&pIn))) return hr;
	BYTE* pInOrg = pIn;

	long len = pSample->GetActualDataLength();
	if(len <= 0) return S_FALSE;

	BYTE* pOut = NULL;
	if(FAILED(hr = pOutSample->GetPointer(&pOut))) return hr;
	BYTE* pOutOrg = pOut;

	int size = pOutSample->GetSize();

	if((CheckAC3(&m_pInput->CurrentMediaType()) || CheckDTS(&m_pInput->CurrentMediaType()))
	&& (CheckWAVEAC3(&m_pOutput->CurrentMediaType()) || CheckWAVEDTS(&m_pOutput->CurrentMediaType())))
	{
		if(*(DWORD*)pIn == 0xBA010000)
		{
			pIn += 14;
		}

		if(*(DWORD*)pIn == 0xBD010000)
		{
			pIn += 8 + 1 + pIn[8] + 1 + 3;
		}

		len -= (pInOrg - pIn);

		if(size < len) return E_FAIL;

		memcpy(pOut, pIn, len);
		pOut += len;
	}
	else if((CheckWAVEAC3(&m_pInput->CurrentMediaType()) || CheckWAVEDTS(&m_pInput->CurrentMediaType()))
	&& (CheckAC3(&m_pOutput->CurrentMediaType()) || CheckDTS(&m_pOutput->CurrentMediaType())))
	{
		if((m_pOutput->CurrentMediaType().majortype == MEDIATYPE_DVD_ENCRYPTED_PACK
			|| m_pOutput->CurrentMediaType().majortype == MEDIATYPE_MPEG2_PES)
		&& (len + 12 + 3) >= 0x10000) // damn, this can happen if the interleave time is too big
		{
			REFERENCE_TIME rtStart = 0, rtStop = 1;
			bool fHasTime = (S_OK == pSample->GetTime(&rtStart, &rtStop));

			bool fDiscontinuity = (S_OK == pOutSample->IsDiscontinuity());

			int pos = 0;
			while(pos < len)
			{
				int curlen = min(len - pos, 2013);
				pos += 2013;

				CComPtr<IMediaSample> pOutSample;
				hr = InitializeOutputSample(pSample, &pOutSample);

				if(fDiscontinuity)
				{
					if(fHasTime)
					{
						rtStop = rtStart + (rtStop - rtStart) * curlen / len;
						pOutSample->SetTime(&rtStart, &rtStop);
					}

					fDiscontinuity = false;
				}
				else
				{
					pOutSample->SetTime(NULL, NULL);
					pOutSample->SetDiscontinuity(FALSE);
				}

				BYTE* pOut = NULL;
				if(FAILED(hr = pOutSample->GetPointer(&pOut))) return hr;
				BYTE* pOutOrg = pOut;

				int size = pOutSample->GetSize();

				const GUID* majortype = &m_pOutput->CurrentMediaType().majortype;
				const GUID* subtype = &m_pOutput->CurrentMediaType().subtype;

				if(*majortype == MEDIATYPE_DVD_ENCRYPTED_PACK)
				{
					if(size < curlen + 32 + 3) return E_FAIL;

					BYTE PESHeader[] = 
					{
						0x00,0x00,0x01,0xBA,			// PES id
						0x44,0x00,0x04,0x00,0x04,0x01,	// SCR (0)
						0x01,0x89,0xC3,0xF8,			// mux rate (1260000 bytes/sec, 22bits), marker (2bits), reserved (~0, 5bits), stuffing (0, 3bits)
					};

					memcpy(pOut, &PESHeader, sizeof(PESHeader));
					pOut += sizeof(PESHeader);

					majortype = &MEDIATYPE_MPEG2_PES;
				}

				if(*majortype == MEDIATYPE_MPEG2_PES)
				{
					if(size < curlen + 20 + 3) return E_FAIL;

					BYTE Private1Header[] = 
					{
						0x00,0x00,0x01,0xBD,			// private stream 1 id
						0x07,0xEC,						// packet length (TODO: modify it later)
						0x81,0x80,						// marker, original, PTS - flags
						0x08,							// packet data starting offset
						0x21,0x00,0x01,0x00,0x01,		// PTS (0)
						0xFF,0xFF,0xFF,					// stuffing
						0x80,							// stream id (0)
						0x01,0x00,0x01,					// no idea about the first byte, the sencond+third seem to show the ac3/dts header sync offset plus one (dvd2avi doesn't output it to the ac3/dts file so we have to put it back)
					};

					int packetlen = curlen + 12 + 3;
					ASSERT(packetlen <= 0xffff);
					Private1Header[4] = (packetlen>>8)&0xff;
					Private1Header[5] = packetlen&0xff;

					if(*subtype == MEDIASUBTYPE_DTS)
					{
						Private1Header[17] += 8;
					}

					if(*subtype == MEDIASUBTYPE_DOLBY_AC3)
					{
						for(int i = 0; i < curlen; i++)
						{
							if(*(DWORD*)&pIn[i] == 0x770B)
							{
								i++;
								Private1Header[19] = (i>>8)&0xff;
								Private1Header[20] = i&0xff;
								break;
							}
						}
					}
					else if(*subtype == MEDIASUBTYPE_DTS)
					{
						for(int i = 0; i < curlen; i++)
						{
							if(*(DWORD*)&pIn[i] == 0x0180FE7F)
							{
								i++;
								Private1Header[19] = (i>>8)&0xff;
								Private1Header[20] = i&0xff;
								break;
							}
						}
					}

					memcpy(pOut, &Private1Header, sizeof(Private1Header));
					pOut += sizeof(Private1Header);

					majortype = &MEDIATYPE_Audio;
				}

				if(*majortype == MEDIATYPE_Audio)
				{
					if(size < curlen) return E_FAIL;
					memcpy(pOut, pIn, curlen);
					pIn += curlen;
					pOut += curlen;
				}

				pOutSample->SetActualDataLength(pOut - pOutOrg);

				hr = m_pOutput->Deliver(pOutSample);
			}

			return S_FALSE;
		}
		else // phew, we can do the easier way
		{
			const GUID* majortype = &m_pOutput->CurrentMediaType().majortype;
			const GUID* subtype = &m_pOutput->CurrentMediaType().subtype;

			if(*majortype == MEDIATYPE_DVD_ENCRYPTED_PACK)
			{
				if(size < len + 32 + 3) return E_FAIL;

				BYTE PESHeader[] = 
				{
					0x00,0x00,0x01,0xBA,			// PES id
					0x44,0x00,0x04,0x00,0x04,0x01,	// SCR (0)
					0x01,0x89,0xC3,0xF8,			// mux rate (1260000 bytes/sec, 22bits), marker (2bits), reserved (~0, 5bits), stuffing (0, 3bits)
				};

				memcpy(pOut, &PESHeader, sizeof(PESHeader));
				pOut += sizeof(PESHeader);

				majortype = &MEDIATYPE_MPEG2_PES;
			}

			if(*majortype == MEDIATYPE_MPEG2_PES)
			{
				if(size < len + 20 + 3) return E_FAIL;

				BYTE Private1Header[] = 
				{
					0x00,0x00,0x01,0xBD,			// private stream 1 id
					0x07,0xEC,						// packet length (TODO: modify it later)
					0x81,0x80,						// marker, original, PTS - flags
					0x08,							// packet data starting offset
					0x21,0x00,0x01,0x00,0x01,		// PTS (0)
					0xFF,0xFF,0xFF,					// stuffing
					0x80,							// stream id (0)
					0x01,0x00,0x01,					// no idea about the first byte, the sencond+third seem to show the ac3/dts header sync offset plus one (dvd2avi doesn't output it to the ac3/dts file so we have to put it back)
				};

				int packetlen = len + 12 + 3;
				ASSERT(packetlen <= 0xffff);
				Private1Header[4] = (packetlen>>8)&0xff;
				Private1Header[5] = packetlen&0xff;

				if(*subtype == MEDIASUBTYPE_DTS)
				{
					Private1Header[17] += 8;
				}

				memcpy(pOut, &Private1Header, sizeof(Private1Header));
				pOut += sizeof(Private1Header);

				majortype = &MEDIATYPE_Audio;
			}

			if(*majortype == MEDIATYPE_Audio)
			{
				if(size < len) return E_FAIL;

				memcpy(pOut, pIn, len);
				pIn += len;
				pOut += len;
			}
		}
	}
	else
	{
		return E_FAIL;
	}

	pOutSample->SetActualDataLength(pOut - pOutOrg);

	return S_OK;
}

bool CAVI2AC3Filter::CheckAC3(const CMediaType* pmt)
{
	return (pmt->majortype == MEDIATYPE_Audio
			|| pmt->majortype == MEDIATYPE_MPEG2_PES 
			|| pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK)
		&& pmt->subtype == MEDIASUBTYPE_DOLBY_AC3;
}

bool CAVI2AC3Filter::CheckDTS(const CMediaType* pmt)
{
	return (pmt->majortype == MEDIATYPE_Audio
			|| pmt->majortype == MEDIATYPE_MPEG2_PES 
			|| pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK)
		&& pmt->subtype == MEDIASUBTYPE_DTS;
}

bool CAVI2AC3Filter::CheckWAVEAC3(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Audio
		&& pmt->subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3
		&& pmt->formattype == FORMAT_WaveFormatEx
		&& ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_DOLBY_AC3;
}

bool CAVI2AC3Filter::CheckWAVEDTS(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Audio
		&& pmt->subtype == MEDIASUBTYPE_WAVE_DTS
		&& pmt->formattype == FORMAT_WaveFormatEx
		&& ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_DVD_DTS;
}

HRESULT CAVI2AC3Filter::CheckInputType(const CMediaType* mtIn)
{
	bool fWaveFormatEx = !!(mtIn->formattype == FORMAT_WaveFormatEx);

	return CheckAC3(mtIn) && fWaveFormatEx || CheckDTS(mtIn) && fWaveFormatEx
		|| CheckWAVEAC3(mtIn) || CheckWAVEDTS(mtIn)
		? S_OK
		: VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CAVI2AC3Filter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	return CheckAC3(mtIn) && CheckWAVEAC3(mtOut)
		|| CheckWAVEAC3(mtIn) && CheckAC3(mtOut)
		|| CheckDTS(mtIn) && CheckWAVEDTS(mtOut)
		|| CheckWAVEDTS(mtIn) && CheckDTS(mtOut)
		? S_OK
		: VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CAVI2AC3Filter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	CComPtr<IMemAllocator> pAllocatorIn;
	m_pInput->GetAllocator(&pAllocatorIn);
	if(!pAllocatorIn) return E_UNEXPECTED;

	pAllocatorIn->GetProperties(pProperties);

	pProperties->cBuffers = 2;
	pProperties->cbBuffer = max(pProperties->cbBuffer, 1024*1024); // this should be enough...
	pProperties->cbAlign = 1;
	pProperties->cbPrefix = 0;

	HRESULT hr;
	ALLOCATOR_PROPERTIES Actual;
    if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) 
		return hr;

    return(pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
		? E_FAIL
		: NOERROR);
}

HRESULT CAVI2AC3Filter::GetMediaType(int iPosition, CMediaType* pMediaType)
{
    if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	const GUID& majortype = m_pInput->CurrentMediaType().majortype;
	const GUID& subtype = m_pInput->CurrentMediaType().subtype;
	
	if(CheckAC3(&m_pInput->CurrentMediaType()) || CheckDTS(&m_pInput->CurrentMediaType()))
	{
		if(iPosition < 0) return E_INVALIDARG;
		if(iPosition > 0) return VFW_S_NO_MORE_ITEMS;

		pMediaType->majortype = MEDIATYPE_Audio;

		pMediaType->formattype = FORMAT_WaveFormatEx;
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)pMediaType->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		memset(wfe, 0, sizeof(WAVEFORMATEX));
		wfe->cbSize = sizeof(WAVEFORMATEX);
		wfe->nAvgBytesPerSec = ((WAVEFORMATEX*)m_pInput->CurrentMediaType().pbFormat)->nAvgBytesPerSec;
		wfe->nSamplesPerSec = ((WAVEFORMATEX*)m_pInput->CurrentMediaType().pbFormat)->nSamplesPerSec;
		wfe->wBitsPerSample = ((WAVEFORMATEX*)m_pInput->CurrentMediaType().pbFormat)->wBitsPerSample;
		wfe->nChannels = 2;
		wfe->nBlockAlign = 1;

		if(subtype == MEDIASUBTYPE_DOLBY_AC3)
		{
			pMediaType->subtype = MEDIASUBTYPE_WAVE_DOLBY_AC3;
			wfe->wFormatTag = WAVE_FORMAT_DOLBY_AC3;
		}
		else if(subtype == MEDIASUBTYPE_DTS)
		{
			pMediaType->subtype = MEDIASUBTYPE_WAVE_DTS;
			wfe->wFormatTag = WAVE_FORMAT_DVD_DTS;
		}
		else
		{
			return E_INVALIDARG;
		}
	}
	else if(CheckWAVEAC3(&m_pInput->CurrentMediaType()) || CheckWAVEDTS(&m_pInput->CurrentMediaType()))
	{
		if(iPosition < 0) return E_INVALIDARG;
		if(iPosition > 4) return VFW_S_NO_MORE_ITEMS;

		if(subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3)
		{
			pMediaType->subtype = MEDIASUBTYPE_DOLBY_AC3;

			pMediaType->formattype = FORMAT_WaveFormatEx;
			DOLBYAC3WAVEFORMAT* wfe = (DOLBYAC3WAVEFORMAT*)pMediaType->AllocFormatBuffer(sizeof(DOLBYAC3WAVEFORMAT));
			memset(wfe, 0, sizeof(DOLBYAC3WAVEFORMAT));
			// unfortunately we can't tell what we are going to get in transform, 
			// so we just set the most common values and hope that the ac3 decoder 
			// is flexible enough (it is usually :) to find out these from the bitstream
			wfe->wfx.cbSize = sizeof(DOLBYAC3WAVEFORMAT) - sizeof(WAVEFORMATEX);
			wfe->wfx.wFormatTag = WAVE_FORMAT_DOLBY_AC3;
			wfe->wfx.nSamplesPerSec = 48000;
			wfe->wfx.nChannels = 6;
			wfe->bBigEndian = TRUE;
		}
		else if(subtype == MEDIASUBTYPE_WAVE_DTS)
		{
			pMediaType->subtype = MEDIASUBTYPE_DTS;

			pMediaType->formattype = FORMAT_WaveFormatEx;
			WAVEFORMATEX* wfe = (WAVEFORMATEX*)pMediaType->AllocFormatBuffer(sizeof(WAVEFORMATEX));
			memset(wfe, 0, sizeof(WAVEFORMATEX));
			// same case as with ac3, but this time we don't even know the structure
			wfe->cbSize = sizeof(WAVEFORMATEX);
			wfe->wFormatTag = WAVE_FORMAT_PCM;
			wfe->nSamplesPerSec = 48000;
			wfe->nChannels = 6;
		}
		else
		{
			return E_INVALIDARG;
		}

		switch(iPosition)
		{
		case 0:
			pMediaType->majortype = MEDIATYPE_Audio;
			break;
		case 1:
			pMediaType->ResetFormatBuffer();
			pMediaType->formattype = FORMAT_None;
		case 2:
			pMediaType->majortype = MEDIATYPE_MPEG2_PES;
			break;
		case 3:
			pMediaType->ResetFormatBuffer();
			pMediaType->formattype = FORMAT_None;
		case 4:
			pMediaType->majortype = MEDIATYPE_DVD_ENCRYPTED_PACK;
			break;
		default:
			return E_INVALIDARG;
		}
	}
	else
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	return S_OK;
}
