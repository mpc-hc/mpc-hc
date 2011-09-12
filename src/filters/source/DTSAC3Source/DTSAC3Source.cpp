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
#include <MMReg.h>
#include <ks.h>
#include <InitGuid.h>
#include <uuids.h>
#include <moreuuids.h>
#include "DTSAC3Source.h"
#include "../../../DSUtil/DSUtil.h"
#include <atlpath.h>
#include <stdint.h>
#include <libdca/include/dts.h>

#define RIFF_DWORD          0x46464952
#define AC3_SYNC_WORD           0x770b
#define TRUEHD_SYNC_WORD    0xba6f72f8
#define MLP_SYNC_WORD       0xbb6f72f8
#define IEC61937_SYNC_WORD  0x4e1ff872

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

enum {
	unknown,
	AC3,
	EAC3,
	TrueHD,
	TrueHDAC3,
	MLP,
	DTS,
	DTSHD,
	DTSPaded,
	SPDIF_AC3,
};

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

int ParseAC3Header(const BYTE *buf, int *samplerate, int *channels, int *framelength, int *bitrate)
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
	
	*framelength = 1536;
	return bytes;
}

int ParseEAC3Header(const BYTE *buf, int *samplerate, int *channels, int *framelength, int *frametype)
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
	*samplerate       = sample_rates[fscod == 0x03 ? 3 + fscod2 : fscod];
	int bytes         = (((buf[2] & 0x03) << 8) + buf[3] + 1) * 2;
	*channels         = channels_tbl[acmod] + lfeon;
	*framelength      = (fscod == 0x03) ? 1536 : samples_tbl[fscod2];

	return bytes;
}

int ParseTrueHDHeader(const BYTE *buf, int *samplerate, int *channels, int *framelength) 
{
	static const int sampling_rates[]     = { 48000, 96000, 192000, 0, 0, 0, 0, 0, 44100, 88200, 176400, 0, 0, 0, 0, 0 };
	static const int channel_count[13] = {//   LR    C   LFE  LRs LRvh  LRc LRrs  Cs   Ts  LRsd  LRw  Cvh  LFE2
												2,   1,   1,   2,   2,   2,   2,   1,   1,   2,   2,   1,   1 };
	
	DWORD sync = *(DWORD*)(buf+4);
	if (sync != TRUEHD_SYNC_WORD)  // syncword
		return 0;
	int m_size  = (((buf[0] << 8) | buf[1]) & 0xfff) * 2;
	
	*samplerate             = sampling_rates[buf[8] >> 4];
	*framelength            = 40 << ((buf[8] >> 4) & 0x07);
	int chanmap_substream_1 = ((buf[ 9] & 0x0f) << 1) | (buf[10] >> 7);
	int chanmap_substream_2 = ((buf[10] & 0x1f) << 8) |  buf[11];
    int channel_map         = chanmap_substream_2 ? chanmap_substream_2 : chanmap_substream_1;
	*channels = 0;
	for (int i = 0; i < 13; ++i)
		*channels += channel_count[i] * ((channel_map >> i) & 1);

	return m_size;
}

int ParseAC3IEC61937Header(const BYTE *buf)
{
	WORD* wbuf = (WORD*)buf;
	if (*(DWORD*)buf == IEC61937_SYNC_WORD
		&& wbuf[2] ==  0x0001
		&& wbuf[3] > 0 && wbuf[3] < (6144-8)
		&& wbuf[4] == 0x0B77 ) {
		return 6144;
	}
	return 0;
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
		_T("0"), _T("0,4,,7FFE8001")); // DTS

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"),
		_T("0"), _T("0,4,,fE7f0180")); // DTS LE

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"),
		_T("1"), _T("0,2,,0B77")); // AC3, E-AC3

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"),
		_T("2"), _T("0,16,,52494646xxxx57415645666D7420")); // RIFFxxxxWAVEfmt_ for DTSWAV

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".dts"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".ac3"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".eac3"),
		_T("Source Filter"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{B4A7BE85-551D-4594-BDC7-832B09185041}"));
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".dts"));
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".ac3"));
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".eac3"));

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
	, m_subtype(GUID_NULL)
	, m_wFormatTag(WAVE_FORMAT_UNKNOWN)
	, m_channels(0)
	, m_samplerate(0)
	, m_bitrate(0)
	, m_framesize(0)
	, m_fixedframesize(true)
	, m_framelength(0)
	, m_bitdepth(0)
	, m_streamtype(unknown)
{
	CAutoLock cAutoLock(&m_cSharedState);
	CString fn(wfn);
	CFileException	ex;
	HRESULT hr = E_FAIL;
	m_AvgTimePerFrame = 0;

	bool waveheader = false;

	do {
		if(!m_file.Open(fn, CFile::modeRead|CFile::shareDenyNone, &ex)) {
			hr	= AmHresultFromWin32 (ex.m_lOsError);
			break;
		}

		DWORD id = 0;
		DWORD id2 = 0;
		if (m_file.Read(&id, sizeof(id)) != sizeof(id) ||
			m_file.Read(&id2, sizeof(id2)) != sizeof(id2))
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
			waveheader = true;
		}

		m_dataOffset = m_file.GetPosition() - (sizeof(id) + sizeof(id2));

		// search DTS and AC3 headers (skip garbage in the beginning)
		if (!isDTSSync(id) && (WORD)id!=AC3_SYNC_WORD
			&& id!=IEC61937_SYNC_WORD && id2!=TRUEHD_SYNC_WORD) {
			if (ext != _T(".dtswav") && ext != _T(".dts") && ext != _T(".wav") && ext != _T(".ac3") && ext != _T(".eac3")) //check only specific extensions
				break;
			m_file.Seek(m_dataOffset, CFile::begin);
			BYTE buf[4100];
			int len = m_file.Read(&buf, 4100); //4100=4096+4
			if (len<100) break; //100=96+4
			bool isFound = false;
			for (int i=1; i<len-4; i++) { // looking for DTS or AC3 sync
				id = *(DWORD*)(buf+i);
				if (isDTSSync(id) || (WORD)id==AC3_SYNC_WORD) {
					isFound = true;
					m_dataOffset += i;
					break;
				}
			}
			if (!isFound) break;
		}

		m_file.Seek(m_dataOffset, CFile::begin);
		// DTS & DTS-HD
		if (isDTSSync(id)) {
			BYTE buf[16];
			if (m_file.Read(&buf, 16) != 16)
				break;

			// DTS header
			dts_state_t* m_dts_state;
			m_dts_state = dts_init(0);
			int fsize = 0, flags, targeted_bitrate;
			if((fsize = dts_syncinfo(m_dts_state, buf, &flags, &m_samplerate, &targeted_bitrate, &m_framelength)) < 96) { //minimal valid fsize = 96
				break;
			}
			m_streamtype = DTS;

			// DTS-HD header and zero padded
			unsigned long sync = -1;
			unsigned int HD_size = 0;
			int zero_bytes = 0;

			m_file.Seek(m_dataOffset+fsize, CFile::begin);
			m_file.Read(&sync, sizeof(sync));
			if (id == 0x0180fe7f && sync == 0x25205864 && m_file.Read(&buf, 8)==8) {
				unsigned char isBlownUpHeader = (buf[1]>>5)&1;
				if (isBlownUpHeader)
					HD_size = ((buf[2]&1)<<19 | buf[3]<<11 | buf[4]<<3 | buf[5]>>5) + 1;
				else
					HD_size = ((buf[2]&31)<<11 | buf[3]<<3 | buf[4]>>5) + 1;
				//TODO: get more information about DTS-HD
				m_streamtype = DTSHD;
				m_fixedframesize = false;
			} else if (sync == 0 && fsize < 2048) { // zero padded?
				m_file.Seek(m_dataOffset+2048, CFile::begin);
				m_file.Read(&sync, sizeof(sync));
				if (sync == id) {
					zero_bytes = 2048 - fsize;
					m_streamtype = DTSPaded;
				}
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
			
			// calculate actual bitrate
			m_framesize = fsize + HD_size + zero_bytes;
			m_bitrate   = int ((m_framesize) * 8i64 * m_samplerate / m_framelength);

			// calculate framesize to support a sonic audio decoder 4.3 (TODO: make otherwise)
			// sonicDTSminsize = fsize + HD_size + 4096
			int k = (m_framesize + 4096 + m_framesize - 1) / m_framesize;
			m_framesize   *= k;
			m_framelength *= k;

			if (flags & 0x70) //unknown number of channels
				m_channels = 6;
			else {
				m_channels = channels[flags & 0x0f];
				if (flags & DCA_LFE) m_channels += 1; //+LFE
			}

			if (m_bitrate!=0)
				m_AvgTimePerFrame = 10000000i64 * m_framesize * 8 / m_bitrate;
			
			m_wFormatTag = WAVE_FORMAT_DTS;
			m_subtype = MEDIASUBTYPE_DTS;
			
		// AC3 & E-AC3
		} else if ((WORD)id == AC3_SYNC_WORD) {
			BYTE buf[20];
			if (m_file.Read(&buf, 8) != 8)
				break;

			BYTE bsid = (buf[5] >> 3);
			int fsize   = 0;
			int fsize2  = 0;
			int HD_size = 0;

			//  AC3 header
			if (bsid < 12) {
				fsize = ParseAC3Header(buf, &m_samplerate, &m_channels, &m_framelength, &m_bitrate);
				if (fsize == 0) {
					break;
				}
				m_streamtype = AC3;
				// TrueHD+AC3
				m_file.Seek(m_dataOffset+fsize, CFile::begin);
				if (m_file.Read(&buf, 20) == 20) {
					int samplerate2, channels2, framelength2;
					HD_size = ParseTrueHDHeader(buf, &samplerate2, &channels2, &framelength2);
					if (HD_size > 0) {
						m_streamtype = TrueHDAC3;
						m_fixedframesize = false;
					}
				}
			// E-AC3 header
			} else if (bsid == 16) {
				int frametype;
				fsize = ParseEAC3Header(buf, &m_samplerate, &m_channels, &m_framelength, &frametype);
				if (fsize == 0) {
					break;
				}

				m_file.Seek(m_dataOffset+fsize, CFile::begin);
				if (m_file.Read(&buf, 8) == 8) {
					int samplerate2, channels2, samples2, frametype2;
					fsize2 = ParseEAC3Header(buf, &samplerate2, &channels2, &samples2, &frametype2);
					if (frametype2 != EAC3_FRAME_TYPE_DEPENDENT)
						fsize2 = 0;
				}
				m_bitrate = int (fsize * 8i64 * m_samplerate / m_framelength);
				m_streamtype = EAC3;
			} else { //unknown bsid
				break;
			}

			// calculate framesize to support a sonic audio decoder 4.3 (TODO: make otherwise)
			// sonicAC3minsize = framesize + 64
			m_framesize = (fsize + fsize2) * 2;
			m_framelength *= 2;

			if (m_bitrate!=0)
				m_AvgTimePerFrame = 10000000i64 * m_framesize * 8 / m_bitrate;

			m_wFormatTag = WAVE_FORMAT_UNKNOWN;
			m_subtype = MEDIASUBTYPE_DOLBY_AC3;

		// SPDIF AC3
		} else if (waveheader && id == IEC61937_SYNC_WORD) {
			BYTE buf[16];
			if (m_file.Read(&buf, 16) != 16)
				break;
			m_framesize = ParseAC3IEC61937Header(buf);
			if (m_framesize == 0)
				break;
			
			m_wFormatTag  = WAVE_FORMAT_DOLBY_AC3_SPDIF;
			m_channels    = 2;
			m_channels    = 44100;
			m_framesize   = 4;
			m_bitdepth    = 16;

			m_AvgTimePerFrame = 10000000i64 * 1536 / 44100;

			m_subtype = MEDIASUBTYPE_DOLBY_AC3_SPDIF;

			m_streamtype = SPDIF_AC3;

		// TrueHD
		} else if (id2 == TRUEHD_SYNC_WORD) {
			BYTE buf[20];
			if (m_file.Read(&buf, 20) != 20)
				break;
			int fsize  = 0;
			int fsize2 = 0;
			fsize = ParseTrueHDHeader(buf, &m_samplerate, &m_channels, &m_framelength);
			if (fsize == 0)
				break;
			m_streamtype = TrueHD;
			m_fixedframesize = false;

			if (m_samplerate!=0)
				m_AvgTimePerFrame = 10000000i64 * m_framelength / m_samplerate;

			// TrueHD+AC3
			if (m_file.Read(&buf, 8) == 8) {
				int samplerate2, channels2, framelength2, bitrate2;
				fsize2 = ParseAC3Header(buf, &samplerate2, &channels2, &framelength2, &bitrate2);
				if (fsize2 > 0) {
					m_streamtype = TrueHDAC3;

					if (samplerate2!=0)
						m_AvgTimePerFrame = 10000000i64 * framelength2 / samplerate2;
				}
			}

			m_framesize = fsize + fsize2;
			//m_bitdepth = 24;

			m_wFormatTag = WAVE_FORMAT_UNKNOWN;
			m_subtype = MEDIASUBTYPE_DOLBY_TRUEHD;

		} else {
			break;
		}

		m_rtDuration = m_AvgTimePerFrame * (m_file.GetLength() - m_dataOffset) / m_framesize;
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
	pProperties->cbBuffer = m_framesize;

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

	if(*majortype == MEDIATYPE_Audio) {
		m_file.Seek(m_dataOffset + nFrame*m_framesize, CFile::begin);
		if((int)m_file.Read(pOut, m_framesize) < m_framesize) {
			return S_FALSE;
		}
		pOut += m_framesize;
	}

	len = pOut - pOutOrg;

	return S_OK;
}

bool CDTSAC3Stream::CheckDTS(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Audio
		   && pmt->subtype == MEDIASUBTYPE_DTS
		   && pmt->formattype == FORMAT_WaveFormatEx
		   && ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_DTS;
}

bool CDTSAC3Stream::CheckDTS2(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Audio
		   && pmt->subtype == MEDIASUBTYPE_WAVE_DTS
		   && pmt->formattype == FORMAT_WaveFormatEx
		   && ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_DVD_DTS;
}

bool CDTSAC3Stream::CheckAC3(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Audio
		   && pmt->subtype == MEDIASUBTYPE_DOLBY_AC3
		   && pmt->formattype == FORMAT_WaveFormatEx
		   && (((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_UNKNOWN 
		   || ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_DOLBY_AC3);
}

bool CDTSAC3Stream::CheckSPDIFAC3(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Audio
		   && pmt->subtype == MEDIASUBTYPE_DOLBY_AC3_SPDIF
		   && pmt->formattype == FORMAT_WaveFormatEx
		   && ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_DOLBY_AC3_SPDIF;
}

bool CDTSAC3Stream::CheckTrueHD(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Audio
		   && pmt->subtype == MEDIASUBTYPE_DOLBY_TRUEHD
		   && pmt->formattype == FORMAT_WaveFormatEx
		   && ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_UNKNOWN;
}

HRESULT CDTSAC3Stream::GetMediaType(int iPosition, CMediaType* pmt)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());

	if (iPosition == 0) {
		pmt->majortype = MEDIATYPE_Audio;
		pmt->subtype = m_subtype;
		pmt->formattype = FORMAT_WaveFormatEx;
		
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		wfe->wFormatTag      = m_wFormatTag;
		wfe->nChannels       = m_channels;
		wfe->nSamplesPerSec  = m_samplerate;
		wfe->nAvgBytesPerSec = (m_bitrate + 4) /8;
		if (m_framesize < WORD_MAX)
			wfe->nBlockAlign = m_framesize;
		else
			wfe->nBlockAlign = WORD_MAX;
		wfe->wBitsPerSample  = m_bitdepth;
		wfe->cbSize = 0;

		if (m_streamtype == SPDIF_AC3) {
			pmt->SetSampleSize(m_framesize);
		} else if (m_streamtype == TrueHD) {
			pmt->SetSampleSize(0);
		}

	} else {
		return VFW_S_NO_MORE_ITEMS;
	}

	pmt->SetTemporalCompression(FALSE);

	return S_OK;
}

HRESULT CDTSAC3Stream::CheckMediaType(const CMediaType* pmt)
{
	return CheckDTS(pmt) || CheckDTS2(pmt)
		|| CheckAC3(pmt) || CheckSPDIFAC3(pmt)
		|| CheckTrueHD(pmt)
		   ? S_OK
		   : E_INVALIDARG;
}
