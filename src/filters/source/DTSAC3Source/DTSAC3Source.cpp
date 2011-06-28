/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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
#include <atlpath.h>
#include <stdint.h>
#include "../../transform/MpaDecFilter/libdca/include/dts.h"

#define RIFF_DWORD          0x46464952
#define AC3_SYNC_WORD           0x770b

#define AC3_CHANNEL                  0
#define AC3_MONO                     1
#define AC3_STEREO                   2
#define AC3_3F                       3
#define AC3_2F1R                     4
#define AC3_3F1R                     5
#define AC3_2F2R                     6
#define AC3_3F2R                     7
#define AC3_CHANNEL1                 8
#define AC3_CHANNEL2                 9
#define AC3_DOLBY                   10
#define AC3_CHANNEL_MASK            15
#define AC3_LFE                     16

#define EAC3_FRAME_TYPE_INDEPENDENT  0
#define EAC3_FRAME_TYPE_DEPENDENT    1
#define EAC3_FRAME_TYPE_AC3_CONVERT  2
#define EAC3_FRAME_TYPE_RESERVED     3

bool isDTSSync(const DWORD sync)
{
	if (sync == 0x0180fe7f || //16 bits and big endian bitstream
		sync == 0x80017ffe || //16 bits and little endian bitstream
		sync == 0x00e8ff1f || //14 bits and big endian bitstream
		sync == 0xe8001fff)   //14 bits and little endian bitstream
		return true;
	else
		return false;
}

DWORD ParseWAVECDHeader(const BYTE wh[44])
{
	if (*(DWORD*)wh != 0x46464952 //"RIFF"
		|| *(DWORDLONG*)(wh+8) != 0x20746d6645564157 //"WAVEfmt "
		|| *(DWORD*)(wh+36) != 0x61746164) { //"data"
		return 0;
 	}
	PCMWAVEFORMAT pcmwf = *(PCMWAVEFORMAT*)(wh+20);
	if (pcmwf.wf.wFormatTag != 1
		|| pcmwf.wf.nChannels != 2
		|| pcmwf.wf.nSamplesPerSec != 44100
		|| pcmwf.wf.nAvgBytesPerSec != 176400
		|| pcmwf.wf.nBlockAlign != 4
		|| pcmwf.wBitsPerSample != 16) {
		return 0;
	}
	return *(DWORD*)(wh+40); //return size of "data"
}

int ParseAC3Header(const BYTE *buf, int *samplerate, int *channels, int *samples, int *bitrate)
{
	if (*(WORD*)buf != 0x770b) // syncword
		return 0;

	if (buf[5] >> 3 >= 12)   // bsid
		return 0;

	static const int rates[] = {
		 32,  40,  48,  56,  64,  80,  96, 112, 128, 160,
		192, 224, 256, 320, 384, 448, 512, 576, 640
	};
	static const unsigned char lfeon[8] = {0x10, 0x10, 0x04, 0x04, 0x04, 0x01, 0x04, 0x01};
	static const unsigned char halfrate[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3};

	int frmsizecod = buf[4] & 0x3F;
	if (frmsizecod >= 38) 
		return 0;

	int half = halfrate[buf[5] >> 3];
	int rate  = rates[frmsizecod >> 1];
	*bitrate  = (rate * 1000) >> half;
	int bytes;
	switch (buf[4] & 0xc0) {
		case 0:
			*samplerate = 48000 >> half;
			bytes       = 4 * rate;
			break;
		case 0x40:
			*samplerate = 44100 >> half;
			bytes       = 2 * (320 * rate / 147 + (frmsizecod & 1));
			break;
		case 0x80:
			*samplerate = 32000 >> half;
			bytes       = 6 * rate;
			break;
		default:
			return 0;
	}
	
	unsigned char acmod    = buf[6] >> 5;
	unsigned char flags = ((((buf[6] & 0xf8) == 0x50) ? AC3_DOLBY : acmod) | ((buf[6] & lfeon[acmod]) ? AC3_LFE : 0));
	switch (flags & AC3_CHANNEL_MASK) {
		case AC3_MONO:
			*channels = 1;
			break;
		case AC3_CHANNEL:
		case AC3_STEREO:
		case AC3_CHANNEL1:
		case AC3_CHANNEL2:
		case AC3_DOLBY:
			*channels = 2;
			break;
		case AC3_2F1R:
		case AC3_3F:
			*channels = 3;
			break;
		case AC3_3F1R:
		case AC3_2F2R:
			*channels = 4;
			break;
		case AC3_3F2R:
			*channels = 5;
			break;
	}
	if (flags & AC3_LFE) (*channels)++;
	
	*samples = 1536;
	return bytes;
}

int ParseEAC3Header(const BYTE *buf, int *samplerate, int *channels, int *samples, int *frametype)
{
	if (*(WORD*)buf != 0x770b) // syncword
		return 0;
		
	if (buf[5] >> 3 != 16)   // bsid
		return 0;

	static const int sample_rates[] = { 48000, 44100, 32000, 24000, 22050, 16000 };
	static const int channels_tbl[]     = { 2, 1, 2, 3, 3, 4, 4, 5 };
	static const int samples_tbl[]      = { 256, 512, 768, 1536 };
	
	int fscod  =  buf[4] >> 6;
	int fscod2 = (buf[4] >> 4) & 0x03;
	
	if (fscod == 0x03 && fscod2 == 0x03)
		return 0;

	int acmod = (buf[4] >> 1) & 0x07;
	int lfeon =  buf[4] & 0x01;

	*frametype    = (buf[2] >> 6) & 0x03;
	if (*frametype == EAC3_FRAME_TYPE_RESERVED)
		return 0;
	//int sub_stream_id = (buf[2] >> 3) & 0x07;
	*samplerate        = sample_rates[fscod == 0x03 ? 3 + fscod2 : fscod];
	int bytes         = (((buf[2] & 0x03) << 8) + buf[3] + 1) * 2;
	*channels          = channels_tbl[acmod] + lfeon;
	*samples           = (fscod == 0x03) ? 1536 : samples_tbl[fscod2];

	return bytes;
}

int ParseTrueHDHeader(const BYTE *buf) 
{
	DWORD sync = *(DWORD*)(buf+4);
	if (sync != 0xba6f72f8 && sync != 0xbb6f72f8)  // syncword
		return 0;
	int m_size  = (((buf[0] << 8) | buf[1]) & 0xfff) * 2;

	return m_size;
}

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
	{&__uuidof(CDTSAC3Source), L"MPC - DTS/AC3/DD+ Source", MERIT_NORMAL, countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
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

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".dts"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

 	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".dtswav"), //DTSWAV
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".ac3"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

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
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".dts"));
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".dtswav")); //DTSWAV
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".ac3"));
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
	, m_dataOffset(0)
{
	CAutoLock cAutoLock(&m_cSharedState);
	CString fn(wfn);
	CFileException	ex;
	HRESULT hr = E_FAIL;
	
	m_subtype = GUID_NULL;
	m_wFormatTag = 0;
	m_streamid = 0;

	do {
		if(!m_file.Open(fn, CFile::modeRead|CFile::shareDenyNone, &ex)) {
			hr	= AmHresultFromWin32 (ex.m_lOsError);
			break;
		}

		DWORD id = 0;
		if(m_file.Read(&id, sizeof(id)) != sizeof(id))
			break;

		// WAVE-CD header
		CString ext = CPath(m_file.GetFileName()).GetExtension().MakeLower();
		if (id == RIFF_DWORD) {
			if (ext != _T(".dtswav") && ext != _T(".dts") && ext != _T(".wav")) //check only specific extensions
				break;
			BYTE buf[44];
			m_file.SeekToBegin();
			if (m_file.Read(&buf, 44) != 44
				|| ParseWAVECDHeader(buf) == 0
				|| m_file.Read(&id, sizeof(id)) != sizeof(id))
				break;
		}

		m_dataOffset = m_file.GetPosition() - sizeof(id);

		// DTS & DTS-HD
		if (isDTSSync(id)) {
			BYTE buf[16];
			m_file.Seek(m_dataOffset, CFile::begin);
			if (m_file.Read(&buf, 16) != 16)
				break;
			// DTS header
			dts_state_t* m_dts_state;
			m_dts_state = dts_init(0);
			int fsize = 0, flags, samplerate, bitrate, framelength;
			if((fsize = dts_syncinfo(m_dts_state, buf, &flags, &samplerate, &bitrate, &framelength)) < 96) { //minimal valid fsize = 96
				break;
			}
			// DTS-HD header and zero padded
			unsigned long sync = -1;
			unsigned int HD_size = 0;
			bool isZeroPadded = false;
			m_file.Seek(m_dataOffset+fsize, CFile::begin);
			m_file.Read(&sync, sizeof(sync));
			if (id == 0x0180fe7f && sync == 0x25205864 && m_file.Read(&buf, 8)==8)
			{
				unsigned char isBlownUpHeader = (buf[1]>>5)&1;
				if (isBlownUpHeader)
					HD_size = ((buf[2]&1)<<19 | buf[3]<<11 | buf[4]<<3 | buf[5]>>5) + 1;
				else
					HD_size = ((buf[2]&31)<<11 | buf[3]<<3 | buf[4]>>5) + 1;
				//TODO: get more information about DTS-HD
			} else if (sync == 0 && fsize < 2048){ // zero padded?
				m_file.Seek(m_dataOffset+2048, CFile::begin);
				m_file.Read(&sync, sizeof(sync));
				if (sync == id) isZeroPadded = true;
	 		}

			/*const int bitratetbl[32] = {
				  32000,   56000,   64000,   96000,
				 112000,  128000,  192000,  224000,
				 256000,  320000,  384000,  448000,
				 512000,  576000,  640000,  768000,
				 960000, 1024000, 1152000, 1280000,
				1344000, 1408000, 1411200, 1472000,
				1536000, 1920000, 2048000, 3072000,
				3840000, 0, 0, 0 //open, variable, lossless
			// [15]  768000 is actually 754500 for DVD
			// [24] 1536000 is actually 1509000 for ???
			// [24] 1536000 is actually 1509750 for DVD
			// [22] 1411200 is actually 1234800 for 14-bit DTS-CD audio
			};*/
			const int channels[16] = {1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 8, 8};

#define	DTS_MAGIC_NUMBER	4	// magic number to make sonic audio decoder 4.3 happy (old value = 6)

			// calculate actual bitrate
			if (isZeroPadded) fsize = 2048;
			bitrate = int ((fsize + HD_size) * 8i64 * samplerate / framelength);

			m_nBytesPerFrame = (fsize + HD_size) * DTS_MAGIC_NUMBER;
			m_nAvgBytesPerSec = (bitrate + 4) / 8;
			m_nSamplesPerSec = samplerate;
			if (flags & 0x70) //unknown number of channels
				m_nChannels = 6;
			else {
				int m_nChannels = channels[flags & 0x0f];
				if (flags & DCA_LFE) m_nChannels += 1; //+LFE
			}
			if (bitrate!=0) m_AvgTimePerFrame = 10000000i64 * m_nBytesPerFrame * 8 / bitrate;
			else m_AvgTimePerFrame = 0;

			m_subtype = MEDIASUBTYPE_DTS;
			m_wFormatTag = WAVE_FORMAT_DTS;
			m_streamid = 0x88;
		// AC3 & E-AC3
		} else if ((WORD)id == AC3_SYNC_WORD) {
			BYTE buf[8];
			m_file.Seek(m_dataOffset, CFile::begin);
			if (m_file.Read(&buf, 8) != 8)
				break;

			BYTE bsid = (buf[5] >> 3);
			int samplerate;
			int bitrate;
			int channels;
			int bytes;
			int samples;

			//  AC3 header
			if (bsid < 12) {
				bytes = ParseAC3Header(buf, &samplerate, &channels, &samples, &bitrate);
				if (bytes == 0) {
					break;
				}
				// TrueHD
				if (m_file.Seek(m_dataOffset+bytes, CFile::begin) == m_dataOffset+bytes
					&& m_file.Read(&buf, 8) == 8) {
					int bytes2 = ParseTrueHDHeader(buf);
					//TODO: get more information about TrueHD
				}
				m_streamid = 0x80;
			// E-AC3 header
			} else if (bsid = 16) {
				int frametype;
				bytes = ParseEAC3Header(buf, &samplerate, &channels, &samples, &frametype);
				if (bytes == 0) {
					break;
				}

				if (m_file.Seek(m_dataOffset+bytes, CFile::begin) == m_dataOffset+bytes && m_file.Read(&buf, 8) == 8) {
					int bytes2, samplerate2, channels2, samples2, frametype2;
					bytes2 = ParseEAC3Header(buf, &samplerate2, &channels2, &samples2, &frametype2);
					if (bytes2 > 0 && frametype2 == EAC3_FRAME_TYPE_DEPENDENT)
						bytes += bytes2;
				}
				bitrate = int (bytes * 8i64 * samplerate / samples);
				m_streamid = 0xC0;
			} else { //unknown bsid
				break;
			}

#define	AC3_MAGIC_NUMBER	2	// magic number to make sonic audio decoder 4.3 happy (old value = 3)

			m_nSamplesPerSec = samplerate;
			m_nAvgBytesPerSec = (bitrate + 4) / 8;
			m_nBytesPerFrame = bytes*AC3_MAGIC_NUMBER;
			if (bitrate!=0) m_AvgTimePerFrame = 10000000i64 * m_nBytesPerFrame * 8 / bitrate;
			else m_AvgTimePerFrame = 0;
			m_subtype = MEDIASUBTYPE_DOLBY_AC3;
			m_wFormatTag = 0;
		} else {
			break;
		}

		m_rtDuration = m_AvgTimePerFrame * (m_file.GetLength() - m_dataOffset) / m_nBytesPerFrame;
		m_rtStop = m_rtDuration;

		hr = S_OK;
	}  while (false);

	if(phr) {
		*phr = hr;
	}
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
		m_file.Seek(m_dataOffset + nFrame*m_nBytesPerFrame, CFile::begin);
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
