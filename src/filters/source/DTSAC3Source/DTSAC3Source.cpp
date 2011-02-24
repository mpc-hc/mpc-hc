/*
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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
#include "DTSAC3Source.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_DTS},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DTS},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_DOLBY_AC3},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DOLBY_AC3},
};

const AMOVIESETUP_PIN sudOpPin[] = {
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
#ifdef DDPLUS_ONLY
	{&__uuidof(CDTSAC3Source), L"MPC - DD+ Source", MERIT_NORMAL, countof(sudOpPin), sudOpPin}
#else
	{&__uuidof(CDTSAC3Source), L"MPC - DTS/AC3/DD+ Source", MERIT_NORMAL, countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
#endif
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CDTSAC3Source>, NULL, &sudFilter[0]}
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"),
		_T("0"), _T("0,4,,7FFE8001"));

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"),
		_T("1"), _T("0,2,,0B77"));

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"),
		_T("2"), _T("0,2,,770B"));

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

#ifndef DDPLUS_ONLY
	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".dts"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".ac3"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));
#endif
	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".ddp"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".ec3"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));
#ifndef DDPLUS_ONLY
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".dts"));
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".ac3"));
#endif
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".ddp"));
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".ec3"));

	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CDTSAC3Source
//

CDTSAC3Source::CDTSAC3Source(LPUNKNOWN lpunk, HRESULT* phr)
	: CBaseSource<CDTSAC3Stream>(NAME("CDTSAC3Source"), lpunk, phr, __uuidof(this))
{
}

CDTSAC3Source::~CDTSAC3Source()
{
}

// CDTSAC3Stream

CDTSAC3Stream::CDTSAC3Stream(const WCHAR* wfn, CSource* pParent, HRESULT* phr)
	: CBaseStream(NAME("CDTSAC3Stream"), pParent, phr)
	, m_nFileOffset(0)
{
	CAutoLock cAutoLock(&m_cSharedState);

	m_subtype = GUID_NULL;
	m_wFormatTag = 0;
	m_streamid = 0;

	CString fn(wfn);

	if(!m_file.Open(fn, CFile::modeRead|CFile::shareDenyNone)) {
		if(phr) {
			*phr = E_FAIL;
		}
		return;
	}

	DWORD id = 0;
	if(m_file.Read(&id, sizeof(id)) != sizeof(id)
			|| id != 0x0180FE7F && (WORD)id != 0x0b77 && (WORD)id != 0x770b) {
		if(phr) {
			*phr = E_FAIL;
		}
		return;
	}

	if(id == 0x0180FE7F) {
#ifdef DDPLUS_ONLY
		//Temporary patch to disable DTS source
		if(phr) {
			*phr = E_FAIL;
		}
		return;
#endif
		BYTE buff[8];
		m_file.Read(buff, 8);

		int frametype = (buff[0]>>7); // 1
		int deficitsamplecount = (buff[0]>>2)&31; // 5
		int crcpresent = (buff[0]>>1)&1; // 1
		int npcmsampleblocks = ((buff[0]&1)<<6)|(buff[1]>>2); // 7
		int framebytes = (((buff[1]&3)<<12)|(buff[2]<<4)|(buff[3]>>4)) + 1; // 14
		int audiochannelarrangement = (buff[3]&15)<<2|(buff[4]>>6); // 6
		int freq = (buff[4]>>2)&15; // 4
		int transbitrate = ((buff[4]&3)<<3)|(buff[5]>>5); // 5
		UNUSED_ALWAYS(frametype);
		UNUSED_ALWAYS(deficitsamplecount);
		UNUSED_ALWAYS(crcpresent);
		UNUSED_ALWAYS(npcmsampleblocks);
		UNUSED_ALWAYS(audiochannelarrangement);

		const int freqtbl[16] = {
			0, 8000, 16000, 32000, 0, 0,
			  11025, 22050, 44100, 0, 0,
			  12000, 24000, 48000, 0, 0
		};

		const int bitratetbl[32] = {
			  32000,   56000,   64000,   96000,
			 112000,  128000,  192000,  224000,
			 256000,  320000,  384000,  448000,
			 512000,  576000,  640000,  768000,
			 960000, 1024000, 1152000, 1280000,
			1344000, 1408000, 1411200, 1472000,
			1536000, 1920000, 2048000, 3072000,
			3840000, 0 /*open*/, 0 /*variable*/, 0 /*lossless*/
		// [15]  768000 is actually 754500 for DVD
		// [24] 1536000 is actually 1509000 for ???
		// [24] 1536000 is actually 1509750 for DVD
		// [22] 1411200 is actually 1234800 for 14-bit DTS-CD audio
		};

#define	DTS_MAGIC_NUMBER	6	//magic number to make sonic audio decoder 4.2 happy

		m_nSamplesPerSec = freqtbl[freq];
		m_nBytesPerFrame = framebytes*DTS_MAGIC_NUMBER;

		//int bitrate = bitratetbl[transbitrate];
		//if      (transbitrate == 15 && framebytes == 1006) bitrate =  754500;
		//else if (transbitrate == 24 && framebytes == 2012) bitrate = 1509000;
		//else if (transbitrate == 24 && framebytes == 2013) bitrate = 1509750;
		//else if (transbitrate == 22 && framebytes == 3584) bitrate = 1234800;
		__int64 bitrate = bitratetbl[transbitrate];
		if (framebytes<=256)
			bitrate = bitrate * framebytes / 256;
		else if (framebytes<=512)
			bitrate = bitrate * framebytes / 512;
		else if (framebytes<=1024)
			bitrate = bitrate * framebytes / 1024;
		else if (framebytes<=2048)
			bitrate = bitrate * framebytes / 2048;
		else if (framebytes<=4096)
			bitrate = bitrate * framebytes / 4096;

		m_nAvgBytesPerSec = (bitrate + 4)/8;
		if (bitrate!=0) m_AvgTimePerFrame = 10000000i64 * m_nBytesPerFrame * 8 / bitrate;
		else m_AvgTimePerFrame = 0;

		m_subtype = MEDIASUBTYPE_DTS;
		m_wFormatTag = WAVE_FORMAT_DVD_DTS;
		m_streamid = 0x88;
	} else {
		BYTE info, info1, bsid;
		if((BYTE)id == 0x77) {
			m_file.Seek(1, CFile::current);    // LE
		}
		m_file.Read(&info, 1);
		m_file.Read(&info1, 1);
		bsid = (info1>>3);

		if(bsid>=0 && bsid<=8) {	//AC3
#ifdef DDPLUS_ONLY
			//Temporary patch to disable AC3 source
			if(phr) {
				*phr = E_FAIL;
			}
			return;
#endif
			BYTE freq = info>>6;
			BYTE bitrate = info&0x3f;

			if(bitrate >= 38) {
				if(phr) {
					*phr = E_FAIL;
				}
				return;
			}

			int freqtbl[] = {48000,44100,32000,48000};

			int bitratetbl[] = {
				32000,32000,40000,40000,48000,48000,56000,56000,64000,64000,
				80000,80000,96000,96000,112000,112000,128000,128000,160000,160000,
				192000,192000,224000,224000,256000,256000,320000,320000,384000,384000,
				448000,448000,512000,512000,576000,576000,640000,640000
			};

#define	AC3_MAGIC_NUMBER	3	//magic number to make sonic audio decoder 4.2 happy

			m_nSamplesPerSec = freqtbl[freq];
			m_nAvgBytesPerSec = (bitratetbl[bitrate] + 4) / 8;
			m_nBytesPerFrame = m_nAvgBytesPerSec*32/1000*AC3_MAGIC_NUMBER;
			m_AvgTimePerFrame = 10000000i64 * m_nBytesPerFrame * 8 / bitratetbl[bitrate];

			m_subtype = MEDIASUBTYPE_DOLBY_AC3;
			m_wFormatTag = WAVE_FORMAT_DOLBY_AC3;
			m_streamid = 0x80;

		} else if(bsid>=11 && bsid <=16) {	//DD+
			BYTE fscod = info>>6;
			BYTE numblkscod = (info&0x30)>>4;
			if(fscod == 3) {
				fscod = numblkscod+3;
				numblkscod = 3;
			}

			int freqtbl[] = {48000,44100,32000,22400,22050,16000,48000};
			m_nSamplesPerSec = freqtbl[fscod];
			m_nBytesPerFrame = (2+(id >> 23)+((id&0x00070000)>>7))*6;
			int timetbl[] = {320000, 640000, 960000, 1920000};
			m_AvgTimePerFrame = timetbl[numblkscod];

			m_subtype = MEDIASUBTYPE_DOLBY_AC3;
			m_wFormatTag = WAVE_FORMAT_DOLBY_AC3;
			m_streamid = 0xC0;
		} else {
			if(phr) {
				*phr = E_FAIL;
			}
			return;
		}

	}

	m_rtDuration = m_AvgTimePerFrame * m_file.GetLength() / m_nBytesPerFrame;
	m_rtStop = m_rtDuration;
}

CDTSAC3Stream::~CDTSAC3Stream()
{
}

HRESULT CDTSAC3Stream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
	ASSERT(pAlloc);
	ASSERT(pProperties);

	HRESULT hr = NOERROR;

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = m_nBytesPerFrame+35;

	ALLOCATOR_PROPERTIES Actual;
	if(FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) {
		return hr;
	}

	if(Actual.cbBuffer < pProperties->cbBuffer) {
		return E_FAIL;
	}
	ASSERT(Actual.cBuffers == pProperties->cBuffers);

	return NOERROR;
}

HRESULT CDTSAC3Stream::FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len)
{
	BYTE* pOutOrg = pOut;

	const GUID* majortype = &m_mt.majortype;
	const GUID* subtype = &m_mt.subtype;
	UNUSED_ALWAYS(subtype);

	if(*majortype == MEDIATYPE_DVD_ENCRYPTED_PACK) {
		BYTE PESHeader[] = {
			0x00,0x00,0x01,0xBA,			// PES id
			0x44,0x00,0x04,0x00,0x04,0x01,	// SCR (0)
			0x01,0x89,0xC3,0xF8,			// mux rate (1260000 bytes/sec, 22bits), marker (2bits), reserved (~0, 5bits), stuffing (0, 3bits)
		};

		memcpy(pOut, &PESHeader, sizeof(PESHeader));
		pOut += sizeof(PESHeader);

		majortype = &MEDIATYPE_MPEG2_PES;
	}

	if(*majortype == MEDIATYPE_MPEG2_PES) {
		BYTE Private1Header[] = {
			0x00,0x00,0x01,0xBD,			// private stream 1 id
			(m_nBytesPerFrame+15)>>8,(m_nBytesPerFrame+15)&255,	// packet length
			0x81,0x80,						// marker, original, PTS - flags
			0x08,							// packet data starting offset
			0x21,0x00,0x01,0x00,0x01,		// PTS (0)
			0xFF,0xFF,0xFF,					// stuffing
			m_streamid,						// stream id (0)
			0x01,0x00,0x01,					// no idea about this (might be the current sector on the disc), but dvd2avi doesn't output it to the ac3/dts file so we have to put it back
		};

		memcpy(pOut, &Private1Header, sizeof(Private1Header));
		pOut += sizeof(Private1Header);

		majortype = &MEDIATYPE_Audio;
	}

	if(*majortype == MEDIATYPE_Audio) {
		m_file.Seek(m_nFileOffset + nFrame*m_nBytesPerFrame, CFile::begin);
		if(m_file.Read(pOut, m_nBytesPerFrame) < m_nBytesPerFrame) {
			return S_FALSE;
		}
		pOut += m_nBytesPerFrame;
	}

	len = pOut - pOutOrg;

	return S_OK;
}

bool CDTSAC3Stream::CheckDTS(const CMediaType* pmt)
{
	return (pmt->majortype == MEDIATYPE_Audio
			|| pmt->majortype == MEDIATYPE_MPEG2_PES
			|| pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK)
		   && pmt->subtype == MEDIASUBTYPE_DTS;
	//	return pmt->majortype == MEDIATYPE_Audio  && pmt->subtype == MEDIASUBTYPE_DTS;
}

bool CDTSAC3Stream::CheckWAVEDTS(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Audio
		   && pmt->subtype == MEDIASUBTYPE_WAVE_DTS
		   && pmt->formattype == FORMAT_WaveFormatEx
		   && ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_DVD_DTS;
}

bool CDTSAC3Stream::CheckAC3(const CMediaType* pmt)
{
	return (pmt->majortype == MEDIATYPE_Audio
			|| pmt->majortype == MEDIATYPE_MPEG2_PES
			|| pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK)
		   && pmt->subtype == MEDIASUBTYPE_DOLBY_AC3;
	//	return pmt->majortype == MEDIATYPE_Audio  && pmt->subtype == MEDIASUBTYPE_DOLBY_AC3;
}

bool CDTSAC3Stream::CheckWAVEAC3(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Audio
		   && pmt->subtype == MEDIASUBTYPE_DOLBY_AC3
		   && pmt->formattype == FORMAT_WaveFormatEx
		   && ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_DOLBY_AC3;
}

HRESULT CDTSAC3Stream::GetMediaType(int iPosition, CMediaType* pmt)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());

	if(iPosition >= 0 && iPosition < 5) {
		pmt->subtype = m_subtype;
		pmt->formattype = FORMAT_WaveFormatEx;
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		memset(wfe, 0, sizeof(WAVEFORMATEX));
		wfe->cbSize = sizeof(WAVEFORMATEX);
		wfe->wFormatTag = WAVE_FORMAT_PCM;
		wfe->nSamplesPerSec = m_nSamplesPerSec;
		wfe->nAvgBytesPerSec = m_nAvgBytesPerSec;
		wfe->nChannels = 6;

		switch(iPosition) {
			case 0:
				pmt->majortype = MEDIATYPE_Audio;
				break;
			case 1:
				pmt->ResetFormatBuffer();
				pmt->formattype = FORMAT_None;
			case 2:
				pmt->majortype = MEDIATYPE_MPEG2_PES;
				break;
			case 3:
				pmt->ResetFormatBuffer();
				pmt->formattype = FORMAT_None;
			case 4:
				pmt->majortype = MEDIATYPE_DVD_ENCRYPTED_PACK;
				break;
			default:
				return E_INVALIDARG;
		}
	} else if(iPosition == 5) {
		pmt->majortype = MEDIATYPE_Audio;
		pmt->subtype = FOURCCMap(m_wFormatTag);
		pmt->formattype = FORMAT_WaveFormatEx;
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		memset(wfe, 0, sizeof(WAVEFORMATEX));
		wfe->cbSize = sizeof(WAVEFORMATEX);
		wfe->wFormatTag = m_wFormatTag;
		wfe->nSamplesPerSec = m_nSamplesPerSec;
		wfe->nAvgBytesPerSec = m_nAvgBytesPerSec;
		wfe->nChannels = 2;
		wfe->nBlockAlign = 1;
	} else {
		return VFW_S_NO_MORE_ITEMS;
	}

	pmt->SetTemporalCompression(FALSE);

	return S_OK;
}

HRESULT CDTSAC3Stream::CheckMediaType(const CMediaType* pmt)
{
	return CheckDTS(pmt) || CheckWAVEDTS(pmt)
		   || CheckAC3(pmt) || CheckWAVEAC3(pmt)
		   ? S_OK
		   : E_INVALIDARG;
}
