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
#include "BaseSplitterFileEx.h"
#include <MMReg.h>
#include "../../../DSUtil/DSUtil.h"
#include "../../../DSUtil/GolombBuffer.h"
#include <InitGuid.h>
#include <moreuuids.h>

//
// CBaseSplitterFileEx
//

CBaseSplitterFileEx::CBaseSplitterFileEx(IAsyncReader* pReader, HRESULT& hr, int cachelen, bool fRandomAccess, bool fStreaming)
	: CBaseSplitterFile(pReader, hr, cachelen, fRandomAccess, fStreaming)
	, m_tslen(0)
	,m_rtPTSOffset(0)
{
}

CBaseSplitterFileEx::~CBaseSplitterFileEx()
{
}

//

bool CBaseSplitterFileEx::NextMpegStartCode(BYTE& code, __int64 len)
{
	BitByteAlign();
	DWORD dw = (DWORD)-1;
	do {
		if(len-- == 0 || !GetRemaining()) {
			return false;
		}
		dw = (dw << 8) | (BYTE)BitRead(8);
	} while((dw&0xffffff00) != 0x00000100);
	code = (BYTE)(dw&0xff);
	return true;
}

//

#define MARKER if(BitRead(1) != 1) {ASSERT(0); return false;}

bool CBaseSplitterFileEx::Read(pshdr& h)
{
	memset(&h, 0, sizeof(h));

	BYTE b = (BYTE)BitRead(8, true);

	if((b&0xf1) == 0x21) {
		h.type = mpeg1;

		EXECUTE_ASSERT(BitRead(4) == 2);

		h.scr = 0;
		h.scr |= BitRead(3) << 30;
		MARKER; // 32..30
		h.scr |= BitRead(15) << 15;
		MARKER; // 29..15
		h.scr |= BitRead(15);
		MARKER;
		MARKER; // 14..0
		h.bitrate = BitRead(22);
		MARKER;
	} else if((b&0xc4) == 0x44) {
		h.type = mpeg2;

		EXECUTE_ASSERT(BitRead(2) == 1);

		h.scr = 0;
		h.scr |= BitRead(3) << 30;
		MARKER; // 32..30
		h.scr |= BitRead(15) << 15;
		MARKER; // 29..15
		h.scr |= BitRead(15);
		MARKER; // 14..0
		h.scr = (h.scr*300 + BitRead(9)) * 10 / 27;
		MARKER;
		h.bitrate = BitRead(22);
		MARKER;
		MARKER;
		BitRead(5); // reserved
		UINT64 stuffing = BitRead(3);
		while(stuffing-- > 0) {
			EXECUTE_ASSERT(BitRead(8) == 0xff);
		}
	} else {
		return false;
	}

	h.bitrate *= 400;

	return true;
}

bool CBaseSplitterFileEx::Read(pssyshdr& h)
{
	memset(&h, 0, sizeof(h));

	WORD len = (WORD)BitRead(16);
	MARKER;
	h.rate_bound = (DWORD)BitRead(22);
	MARKER;
	h.audio_bound = (BYTE)BitRead(6);
	h.fixed_rate = !!BitRead(1);
	h.csps = !!BitRead(1);
	h.sys_audio_loc_flag = !!BitRead(1);
	h.sys_video_loc_flag = !!BitRead(1);
	MARKER;
	h.video_bound = (BYTE)BitRead(5);

	EXECUTE_ASSERT((BitRead(8)&0x7f) == 0x7f); // reserved (should be 0xff, but not in reality)

	for(len -= 6; len > 3; len -= 3) { // TODO: also store these, somewhere, if needed
		UINT64 stream_id = BitRead(8);
		UNUSED_ALWAYS(stream_id);
		EXECUTE_ASSERT(BitRead(2) == 3);
		UINT64 p_std_buff_size_bound = (BitRead(1)?1024:128)*BitRead(13);
		UNUSED_ALWAYS(p_std_buff_size_bound);
	}

	return true;
}

bool CBaseSplitterFileEx::Read(peshdr& h, BYTE code)
{
	memset(&h, 0, sizeof(h));

	if(!(code >= 0xbd && code < 0xf0 || code == 0xfd)) { // 0xfd => blu-ray (.m2ts)
		return false;
	}

	h.len = (WORD)BitRead(16);

	if(code == 0xbe || code == 0xbf) {
		return true;
	}

	// mpeg1 stuffing (ff ff .. , max 16x)
	for(int i = 0; i < 16 && BitRead(8, true) == 0xff; i++) {
		BitRead(8);
		if(h.len) {
			h.len--;
		}
	}

	h.type = (BYTE)BitRead(2, true) == mpeg2 ? mpeg2 : mpeg1;

	if(h.type == mpeg1) {
		BYTE b = (BYTE)BitRead(2);

		if(b == 1) {
			h.std_buff_size = (BitRead(1)?1024:128)*BitRead(13);
			if(h.len) {
				h.len -= 2;
			}
			b = (BYTE)BitRead(2);
		}

		if(b == 0) {
			h.fpts = (BYTE)BitRead(1);
			h.fdts = (BYTE)BitRead(1);
		}
	} else if(h.type == mpeg2) {
		EXECUTE_ASSERT(BitRead(2) == mpeg2);
		h.scrambling = (BYTE)BitRead(2);
		h.priority = (BYTE)BitRead(1);
		h.alignment = (BYTE)BitRead(1);
		h.copyright = (BYTE)BitRead(1);
		h.original = (BYTE)BitRead(1);
		h.fpts = (BYTE)BitRead(1);
		h.fdts = (BYTE)BitRead(1);
		h.escr = (BYTE)BitRead(1);
		h.esrate = (BYTE)BitRead(1);
		h.dsmtrickmode = (BYTE)BitRead(1);
		h.morecopyright = (BYTE)BitRead(1);
		h.crc = (BYTE)BitRead(1);
		h.extension = (BYTE)BitRead(1);
		h.hdrlen = (BYTE)BitRead(8);
	} else {
		if(h.len) while(h.len-- > 0) {
				BitRead(8);
			}
		return false;
	}

	if(h.fpts) {
		if(h.type == mpeg2) {
			// Temporary(dirty) fix - needs more testing
			//BYTE b = (BYTE)BitRead(4);
			//if(!(h.fdts && b == 3 || !h.fdts && b == 2)) {ASSERT(0); return false;}
			BitRead(4);
		}

		h.pts = 0;
		h.pts |= BitRead(3) << 30;
		MARKER; // 32..30
		h.pts |= BitRead(15) << 15;
		MARKER; // 29..15
		h.pts |= BitRead(15);
		MARKER; // 14..0
		h.pts = 10000*h.pts/90 + m_rtPTSOffset;
	}

	if(h.fdts) {
		// Temporary(dirty) fix - needs more testing
		//if((BYTE)BitRead(4) != 1) {ASSERT(0); return false;}
		BitRead(4);

		h.dts = 0;
		h.dts |= BitRead(3) << 30;
		MARKER; // 32..30
		h.dts |= BitRead(15) << 15;
		MARKER; // 29..15
		h.dts |= BitRead(15);
		MARKER; // 14..0
		h.dts = 10000*h.dts/90 + m_rtPTSOffset;
	}

	// skip to the end of header

	if(h.type == mpeg1) {
		if(!h.fpts && !h.fdts && BitRead(4) != 0xf) {
			/*ASSERT(0);*/ return false;
		}

		if(h.len) {
			h.len--;
			if(h.pts) {
				h.len -= 4;
			}
			if(h.dts) {
				h.len -= 5;
			}
		}
	}

	if(h.type == mpeg2) {
		if(h.len) {
			h.len -= 3+h.hdrlen;
		}

		int left = h.hdrlen;
		if(h.fpts) {
			left -= 5;
		}
		if(h.fdts) {
			left -= 5;
		}
		while(left-- > 0) {
			BitRead(8);
		}
		/*
				// mpeg2 stuffing (ff ff .. , max 32x)
				while(BitRead(8, true) == 0xff) {BitRead(8); if(h.len) h.len--;}
				Seek(GetPos()); // put last peeked byte back for Read()

				// FIXME: this doesn't seems to be here,
				// infact there can be ff's as part of the data
				// right at the beginning of the packet, which
				// we should not skip...
		*/
	}

	return true;
}

bool CBaseSplitterFileEx::Read(seqhdr& h, int len, CMediaType* pmt)
{
	__int64 endpos = GetPos() + len; // - sequence header length

	BYTE id = 0;

	while(GetPos() < endpos && id != 0xb3) {
		if(!NextMpegStartCode(id, len)) {
			return false;
		}
	}

	if(id != 0xb3) {
		return false;
	}

	__int64 shpos = GetPos() - 4;

	h.width = (WORD)BitRead(12);
	h.height = (WORD)BitRead(12);
	h.ar = BitRead(4);
	static int ifps[16] = {0, 1126125, 1125000, 1080000, 900900, 900000, 540000, 450450, 450000, 0, 0, 0, 0, 0, 0, 0};
	h.ifps = ifps[BitRead(4)];
	h.bitrate = (DWORD)BitRead(18);
	MARKER;
	h.vbv = (DWORD)BitRead(10);
	h.constrained = BitRead(1);

	h.fiqm = BitRead(1);
	if(h.fiqm)
		for(int i = 0; i < countof(h.iqm); i++) {
			h.iqm[i] = (BYTE)BitRead(8);
		}

	h.fniqm = BitRead(1);
	if(h.fniqm)
		for(int i = 0; i < countof(h.niqm); i++) {
			h.niqm[i] = (BYTE)BitRead(8);
		}

	__int64 shlen = GetPos() - shpos;

	static float ar[] = {
		1.0000f,1.0000f,0.6735f,0.7031f,0.7615f,0.8055f,0.8437f,0.8935f,
		0.9157f,0.9815f,1.0255f,1.0695f,1.0950f,1.1575f,1.2015f,1.0000f
	};

	h.arx = (int)((float)h.width / ar[h.ar] + 0.5);
	h.ary = h.height;

	mpeg_t type = mpeg1;

	__int64 shextpos = 0, shextlen = 0;

	if(NextMpegStartCode(id, 8) && id == 0xb5) { // sequence header ext
		shextpos = GetPos() - 4;

		h.startcodeid = BitRead(4);
		h.profile_levelescape = BitRead(1); // reserved, should be 0
		h.profile = BitRead(3);
		h.level = BitRead(4);
		h.progressive = BitRead(1);
		h.chroma = BitRead(2);
		h.width |= (BitRead(2)<<12);
		h.height |= (BitRead(2)<<12);
		h.bitrate |= (BitRead(12)<<18);
		MARKER;
		h.vbv |= (BitRead(8)<<10);
		h.lowdelay = BitRead(1);
		h.ifps = (DWORD)(h.ifps * (BitRead(2)+1) / (BitRead(5)+1));

		shextlen = GetPos() - shextpos;

		struct {
			DWORD x, y;
		} ar[] = {{h.width,h.height},{4,3},{16,9},{221,100},{h.width,h.height}};
		int i = min(max(h.ar, 1), 5)-1;
		h.arx = ar[i].x;
		h.ary = ar[i].y;

		type = mpeg2;
	}

	h.ifps = 10 * h.ifps / 27;
	h.bitrate = h.bitrate == (1<<30)-1 ? 0 : h.bitrate * 400;

	DWORD a = h.arx, b = h.ary;
	while(a) {
		DWORD tmp = a;
		a = b % tmp;
		b = tmp;
	}
	if(b) {
		h.arx /= b, h.ary /= b;
	}

	if(!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Video;

	if(type == mpeg1) {
		pmt->subtype = MEDIASUBTYPE_MPEG1Payload;
		pmt->formattype = FORMAT_MPEGVideo;
		int len = FIELD_OFFSET(MPEG1VIDEOINFO, bSequenceHeader) + shlen + shextlen;
		MPEG1VIDEOINFO* vi = (MPEG1VIDEOINFO*)DNew BYTE[len];
		memset(vi, 0, len);
		vi->hdr.dwBitRate = h.bitrate;
		vi->hdr.AvgTimePerFrame = h.ifps;
		vi->hdr.bmiHeader.biSize = sizeof(vi->hdr.bmiHeader);
		vi->hdr.bmiHeader.biWidth = h.width;
		vi->hdr.bmiHeader.biHeight = h.height;
		vi->hdr.bmiHeader.biXPelsPerMeter = h.width * h.ary;
		vi->hdr.bmiHeader.biYPelsPerMeter = h.height * h.arx;
		vi->cbSequenceHeader = shlen + shextlen;
		Seek(shpos);
		ByteRead((BYTE*)&vi->bSequenceHeader[0], shlen);
		if(shextpos && shextlen) {
			Seek(shextpos);
		}
		ByteRead((BYTE*)&vi->bSequenceHeader[0] + shlen, shextlen);
		pmt->SetFormat((BYTE*)vi, len);
		delete [] vi;
	} else if(type == mpeg2) {
		pmt->subtype = MEDIASUBTYPE_MPEG2_VIDEO;
		pmt->formattype = FORMAT_MPEG2_VIDEO;
		int len = FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + shlen + shextlen;
		MPEG2VIDEOINFO* vi = (MPEG2VIDEOINFO*)DNew BYTE[len];
		memset(vi, 0, len);
		vi->hdr.dwBitRate = h.bitrate;
		vi->hdr.AvgTimePerFrame = h.ifps;
		vi->hdr.dwPictAspectRatioX = h.arx;
		vi->hdr.dwPictAspectRatioY = h.ary;
		vi->hdr.bmiHeader.biSize = sizeof(vi->hdr.bmiHeader);
		vi->hdr.bmiHeader.biWidth = h.width;
		vi->hdr.bmiHeader.biHeight = h.height;
		vi->dwProfile = h.profile;
		vi->dwLevel = h.level;
		vi->cbSequenceHeader = shlen + shextlen;
		Seek(shpos);
		ByteRead((BYTE*)&vi->dwSequenceHeader[0], shlen);
		if(shextpos && shextlen) {
			Seek(shextpos);
		}
		ByteRead((BYTE*)&vi->dwSequenceHeader[0] + shlen, shextlen);
		pmt->SetFormat((BYTE*)vi, len);
		delete [] vi;
	} else {
		return false;
	}

	return true;
}

bool CBaseSplitterFileEx::Read(mpahdr& h, int len, bool fAllowV25, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	int syncbits = fAllowV25 ? 11 : 12;

	for(; len >= 4 && BitRead(syncbits, true) != (1<<syncbits) - 1; len--) {
		BitRead(8);
	}

	if(len < 4) {
		return false;
	}

	h.sync = BitRead(11);
	h.version = BitRead(2);
	h.layer = BitRead(2);
	h.crc = BitRead(1);
	h.bitrate = BitRead(4);
	h.freq = BitRead(2);
	h.padding = BitRead(1);
	h.privatebit = BitRead(1);
	h.channels = BitRead(2);
	h.modeext = BitRead(2);
	h.copyright = BitRead(1);
	h.original = BitRead(1);
	h.emphasis = BitRead(2);

	if(h.version == 1 || h.layer == 0 || h.freq == 3 || h.bitrate == 15 || h.emphasis == 2) {
		return false;
	}

	if(h.version == 3 && h.layer == 2) {
		if((h.bitrate == 1 || h.bitrate == 2 || h.bitrate == 3 || h.bitrate == 5) && h.channels != 3
				&& (h.bitrate >= 11 && h.bitrate <= 14) && h.channels == 3) {
			return false;
		}
	}

	h.layer = 4 - h.layer;

	//

	static int brtbl[][5] = {
		{0,0,0,0,0},
		{32,32,32,32,8},
		{64,48,40,48,16},
		{96,56,48,56,24},
		{128,64,56,64,32},
		{160,80,64,80,40},
		{192,96,80,96,48},
		{224,112,96,112,56},
		{256,128,112,128,64},
		{288,160,128,144,80},
		{320,192,160,160,96},
		{352,224,192,176,112},
		{384,256,224,192,128},
		{416,320,256,224,144},
		{448,384,320,256,160},
		{0,0,0,0,0},
	};

	static int brtblcol[][4] = {{0,3,4,4},{0,0,1,2}};
	int bitrate = 1000*brtbl[h.bitrate][brtblcol[h.version&1][h.layer]];
	if(bitrate == 0) {
		return false;
	}

	static int freq[][4] = {{11025,0,22050,44100},{12000,0,24000,48000},{8000,0,16000,32000}};

	bool l3ext = h.layer == 3 && !(h.version&1);

	h.nSamplesPerSec = freq[h.freq][h.version];
	h.FrameSize = h.layer == 1
				  ? (12 * bitrate / h.nSamplesPerSec + h.padding) * 4
				  : (l3ext ? 72 : 144) * bitrate / h.nSamplesPerSec + h.padding;
	h.rtDuration = 10000000i64 * (h.layer == 1 ? 384 : l3ext ? 576 : 1152) / h.nSamplesPerSec;// / (h.channels == 3 ? 1 : 2);
	h.nBytesPerSec = bitrate / 8;

	if(!pmt) {
		return true;
	}

	/*int*/ len = h.layer == 3
				  ? sizeof(WAVEFORMATEX/*MPEGLAYER3WAVEFORMAT*/) // no need to overcomplicate this...
				  : sizeof(MPEG1WAVEFORMAT);
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)DNew BYTE[len];
	memset(wfe, 0, len);
	wfe->cbSize = len - sizeof(WAVEFORMATEX);

	if(h.layer == 3) {
		wfe->wFormatTag = WAVE_FORMAT_MP3;

		/*		MPEGLAYER3WAVEFORMAT* f = (MPEGLAYER3WAVEFORMAT*)wfe;
				f->wfx.wFormatTag = WAVE_FORMAT_MP3;
				f->wID = MPEGLAYER3_ID_UNKNOWN;
				f->fdwFlags = h.padding ? MPEGLAYER3_FLAG_PADDING_ON : MPEGLAYER3_FLAG_PADDING_OFF; // _OFF or _ISO ?
		*/
	} else {
		MPEG1WAVEFORMAT* f = (MPEG1WAVEFORMAT*)wfe;
		f->wfx.wFormatTag = WAVE_FORMAT_MPEG;
		f->fwHeadMode = 1 << h.channels;
		f->fwHeadModeExt = 1 << h.modeext;
		f->wHeadEmphasis = h.emphasis+1;
		if(h.privatebit) {
			f->fwHeadFlags |= ACM_MPEG_PRIVATEBIT;
		}
		if(h.copyright) {
			f->fwHeadFlags |= ACM_MPEG_COPYRIGHT;
		}
		if(h.original) {
			f->fwHeadFlags |= ACM_MPEG_ORIGINALHOME;
		}
		if(h.crc == 0) {
			f->fwHeadFlags |= ACM_MPEG_PROTECTIONBIT;
		}
		if(h.version == 3) {
			f->fwHeadFlags |= ACM_MPEG_ID_MPEG1;
		}
		f->fwHeadLayer = 1 << (h.layer-1);
		f->dwHeadBitrate = bitrate;
	}

	wfe->nChannels = h.channels == 3 ? 1 : 2;
	wfe->nSamplesPerSec = h.nSamplesPerSec;
	wfe->nBlockAlign = h.FrameSize;
	wfe->nAvgBytesPerSec = h.nBytesPerSec;

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = FOURCCMap(wfe->wFormatTag);
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)wfe, sizeof(WAVEFORMATEX) + wfe->cbSize);

	delete [] wfe;

	return true;
}

bool CBaseSplitterFileEx::Read(latm_aachdr& h, int len, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	for(; len >= 3 && BitRead(11, true) != 0x2b7; len--) {
		BitRead(8);
	}
	
	if(len < 3) {
		return false;
	}

	// Disable AAC latm stream support until make correct header parsing ...
	return true;
}

bool CBaseSplitterFileEx::Read(aachdr& h, int len, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	__int64 pos = 0;
	int found_fake_sync = 0;	

	for(;;) {
		for(; len >= 7 && BitRead(12, true) != 0xfff; len--) {
			BitRead(8);
		}

		if(len < 7) {
			return false;
		}

		pos = GetPos();

		h.sync = BitRead(12);
		h.version = BitRead(1);
		h.layer = BitRead(2);
		h.fcrc = BitRead(1);
		h.profile = BitRead(2);
		h.freq = BitRead(4);
		h.privatebit = BitRead(1);
		h.channels = BitRead(3);
		h.original = BitRead(1);
		h.home = BitRead(1);

		h.copyright_id_bit = BitRead(1);
		h.copyright_id_start = BitRead(1);
		h.aac_frame_length = BitRead(13);
		h.adts_buffer_fullness = BitRead(11);
		h.no_raw_data_blocks_in_frame = BitRead(2);

		if(h.fcrc == 0) {
			h.crc = BitRead(16);
		}

		if(h.layer != 0 || h.freq >= 12 || h.aac_frame_length <= (h.fcrc == 0 ? 9 : 7)) {
			if(found_fake_sync) // skip only one "fake" sync. TODO - find better way to detect and skip "fake" sync
				return false;
			found_fake_sync++;
			Seek(pos + 1);
			len--;
			continue;
		}

		h.FrameSize = h.aac_frame_length - (h.fcrc == 0 ? 9 : 7);
		static int freq[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};
		h.nBytesPerSec = h.aac_frame_length * freq[h.freq] / 1024; // ok?
		h.rtDuration = 10000000i64 * 1024 / freq[h.freq]; // ok?

		if(!pmt) {
			return true;
		}

		WAVEFORMATEX* wfe = (WAVEFORMATEX*)DNew BYTE[sizeof(WAVEFORMATEX)+5];
		memset(wfe, 0, sizeof(WAVEFORMATEX)+5);
		wfe->wFormatTag = WAVE_FORMAT_AAC;
		wfe->nChannels = h.channels <= 6 ? h.channels : 2;
		wfe->nSamplesPerSec = freq[h.freq];
		wfe->nBlockAlign = h.aac_frame_length;
		wfe->nAvgBytesPerSec = h.nBytesPerSec;
		wfe->cbSize = MakeAACInitData((BYTE*)(wfe+1), h.profile, wfe->nSamplesPerSec, wfe->nChannels);

		pmt->majortype = MEDIATYPE_Audio;
		pmt->subtype = MEDIASUBTYPE_AAC;
		pmt->formattype = FORMAT_WaveFormatEx;
		pmt->SetFormat((BYTE*)wfe, sizeof(WAVEFORMATEX)+wfe->cbSize);

		delete [] wfe;

		return true;
	}
}

bool CBaseSplitterFileEx::Read(ac3hdr& h, int len, CMediaType* pmt, bool find_sync)
{
	static int freq[] = {48000, 44100, 32000, 0};

	bool e_ac3 = false;

	__int64 startpos = GetPos();
	
	memset(&h, 0, sizeof(h));

	// Parse TrueHD header
	BYTE buf[20];
	int  m_channels;
	int  m_samplerate;
	int  m_framelength;

	int fsize = 0;
	ByteRead(buf, 20);
	
	fsize = ParseTrueHDHeader(buf, &m_samplerate, &m_channels, &m_framelength);
	if(fsize) {

		if(!pmt) {
			return true;
		}

		int m_bitrate   = int ((fsize) * 8i64 * m_samplerate / m_framelength);

		pmt->majortype = MEDIATYPE_Audio;
		pmt->subtype = MEDIASUBTYPE_DOLBY_TRUEHD;
		pmt->formattype = FORMAT_WaveFormatEx;
		
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		wfe->wFormatTag      = WAVE_FORMAT_UNKNOWN;
		wfe->nChannels       = m_channels;
		wfe->nSamplesPerSec  = m_samplerate;
		wfe->nAvgBytesPerSec = (m_bitrate + 4) /8;
		wfe->nBlockAlign     = fsize < WORD_MAX ? fsize : WORD_MAX;
		wfe->cbSize = 0;

		pmt->SetSampleSize(0);

		return true;
	}

	Seek(startpos);

	if(find_sync) {
		for(; len >= 7 && BitRead(16, true) != 0x0b77; len--) {
			BitRead(8);
		}
	}

	if(len < 7) {
		return false;
	}

	h.sync = (WORD)BitRead(16);
	if(h.sync != 0x0B77) {
		return false;
	}

	_int64 pos = GetPos();
	h.crc1 = (WORD)BitRead(16);
	h.fscod = BitRead(2);
	h.frmsizecod = BitRead(6);
	h.bsid = BitRead(5);
	if(h.bsid > 16) {
		return false;
	}
	if(h.bsid <= 10) {
		/* Normal AC-3 */
		if(h.fscod == 3) {
			return false;
		}
		if(h.frmsizecod > 37) {
			return false;
		}
		h.bsmod = BitRead(3);
		h.acmod = BitRead(3);
		if(h.acmod == 2) {
			h.dsurmod = BitRead(2);
		}
		if((h.acmod & 1) && h.acmod != 1) {
			h.cmixlev = BitRead(2);
		}
		if(h.acmod & 4) {
			h.surmixlev = BitRead(2);
		}
		h.lfeon = BitRead(1);
		h.sr_shift = max(h.bsid, 8) - 8;
	} else {
		/* Enhanced AC-3 */
		e_ac3 = true;
		Seek(pos);
		h.frame_type = BitRead(2);
		h.substreamid = BitRead(3);
		if(h.frame_type || h.substreamid) {
			return false;
		}
		h.frame_size = (BitRead(11) + 1) << 1;
		if(h.frame_size < 7) {
			return false;
		}
		h.sr_code = BitRead(2);
		if(h.sr_code == 3) {
			int sr_code2 = BitRead(2);
			if(sr_code2 == 3) {
				return false;
			}
			h.sample_rate = freq[sr_code2] / 2;
			h.sr_shift = 1;
		} else {
			static int eac3_blocks[4] = {1, 2, 3, 6};
			h.num_blocks = eac3_blocks[BitRead(2)];
			h.sample_rate = freq[h.sr_code];
			h.sr_shift = 0;
		}
		h.acmod = BitRead(3);
		h.lfeon = BitRead(1);
	}

	if(!pmt) {
		return true;
	}

	WAVEFORMATEX wfe;
	memset(&wfe, 0, sizeof(wfe));
	wfe.wFormatTag = WAVE_FORMAT_DOLBY_AC3;

	static int channels[] = {2, 1, 2, 3, 3, 4, 4, 5};
	wfe.nChannels = channels[h.acmod] + h.lfeon;

	if(!e_ac3) {
		wfe.nSamplesPerSec = freq[h.fscod] >> h.sr_shift;
		static int rate[] = {32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 576, 640, 768, 896, 1024, 1152, 1280};
		wfe.nAvgBytesPerSec = ((rate[h.frmsizecod>>1] * 1000) >> h.sr_shift) / 8;
	} else {
		wfe.nSamplesPerSec = h.sample_rate;
		wfe.nAvgBytesPerSec = h.frame_size * h.sample_rate / (h.num_blocks * 256);
	}

	wfe.nBlockAlign = (WORD)(1536 * wfe.nAvgBytesPerSec / wfe.nSamplesPerSec);

	pmt->majortype = MEDIATYPE_Audio;
	if(e_ac3) {
		pmt->subtype = MEDIASUBTYPE_DOLBY_DDPLUS;
	} else {
		pmt->subtype = MEDIASUBTYPE_DOLBY_AC3;
	}
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(dtshdr& h, int len, CMediaType* pmt, bool find_sync)
{
	memset(&h, 0, sizeof(h));

	if(find_sync) {
		for(; len >= 10 && BitRead(32, true) != 0x7ffe8001; len--) {
			BitRead(8);
		}
	}

	if(len < 10) {
		return false;
	}

	h.sync = (DWORD)BitRead(32);
	if(h.sync != 0x7ffe8001) {
		return false;
	}

	h.frametype = BitRead(1);
	h.deficitsamplecount = BitRead(5);
	h.fcrc = BitRead(1);
	h.nblocks = BitRead(7)+1;
	h.framebytes = (WORD)BitRead(14)+1;
	h.amode = BitRead(6);
	h.sfreq = BitRead(4);
	h.rate = BitRead(5);

	h.downmix = BitRead(1);
	h.dynrange = BitRead(1);
	h.timestamp = BitRead(1);
	h.aux_data = BitRead(1);
	h.hdcd = BitRead(1);
	h.ext_descr = BitRead(3);
	h.ext_coding = BitRead(1);
	h.aspf = BitRead(1);
	h.lfe = BitRead(2);
	h.predictor_history = BitRead(1);


	if(!pmt) {
		return true;
	}

	WAVEFORMATEX wfe;
	memset(&wfe, 0, sizeof(wfe));
	wfe.wFormatTag = WAVE_FORMAT_DVD_DTS;

	static int channels[] = {1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 8, 8};

	if(h.amode < countof(channels)) {
		wfe.nChannels = channels[h.amode];
		if (h.lfe > 0) {
			++wfe.nChannels;
		}
	}

	static int freq[] = {0,8000,16000,32000,0,0,11025,22050,44100,0,0,12000,24000,48000,0,0};
	wfe.nSamplesPerSec = freq[h.sfreq];

	/*static int rate[] = {
		  32000,   56000,   64000,   96000,
		 112000,  128000,  192000,  224000,
		 256000,  320000,  384000,  448000,
		 512000,  576000,  640000,  768000,
		 960000, 1024000, 1152000, 1280000,
		1344000, 1408000, 1411200, 1472000,
		1536000, 1920000, 2048000, 3072000,
		3840000, 0, 0, 0 //open, variable, lossless
	};
	int nom_bitrate = rate[h.rate];*/

	__int64 bitrate = h.framebytes * 8 * wfe.nSamplesPerSec / (h.nblocks*32);

	wfe.nAvgBytesPerSec = (bitrate + 4) / 8;
	wfe.nBlockAlign = h.framebytes;

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = MEDIASUBTYPE_DTS;
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(hdmvlpcmhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	h.size			= BitRead(16);
	h.channels		= BitRead(4);
	h.samplerate	= BitRead(4);
	h.bitpersample	= BitRead(2);

	if (h.channels==0 || h.channels==2 ||
			(h.samplerate != 1 && h.samplerate!= 4  && h.samplerate!= 5) ||
			h.bitpersample<0 || h.bitpersample>3) {
		return false;
	}

	if(!pmt) {
		return true;
	}

	WAVEFORMATEX_HDMV_LPCM wfe;
	wfe.wFormatTag = WAVE_FORMAT_PCM;

	static int channels[] = {0, 1, 0, 2, 3, 3, 4, 4, 5, 6, 7, 8};
	wfe.nChannels	 = channels[h.channels];
	wfe.channel_conf = h.channels;

	static int freq[] = {0, 48000, 0, 0, 96000, 192000};
	wfe.nSamplesPerSec = freq[h.samplerate];

	static int bitspersample[] = {0, 16, 20, 24};
	wfe.wBitsPerSample = bitspersample[h.bitpersample];

	wfe.nBlockAlign		= wfe.nChannels*wfe.wBitsPerSample>>3;
	wfe.nAvgBytesPerSec = wfe.nBlockAlign*wfe.nSamplesPerSec;

	pmt->majortype	= MEDIATYPE_Audio;
	pmt->subtype	= MEDIASUBTYPE_HDMV_LPCM_AUDIO;
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(lpcmhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	h.emphasis = BitRead(1);
	h.mute = BitRead(1);
	h.reserved1 = BitRead(1);
	h.framenum = BitRead(5);
	h.quantwordlen = BitRead(2);
	h.freq = BitRead(2);
	h.reserved2 = BitRead(1);
	h.channels = BitRead(3);
	h.drc = (BYTE)BitRead(8);

	if(h.quantwordlen == 3 || h.reserved1 || h.reserved2) {
		return false;
	}

	if(!pmt) {
		return true;
	}

	WAVEFORMATEX wfe;
	memset(&wfe, 0, sizeof(wfe));
	wfe.wFormatTag = WAVE_FORMAT_PCM;
	wfe.nChannels = h.channels+1;
	static int freq[] = {48000, 96000, 44100, 32000};
	wfe.nSamplesPerSec = freq[h.freq];
	switch (h.quantwordlen) {
		case 0:
			wfe.wBitsPerSample = 16;
			break;
		case 1:
			wfe.wBitsPerSample = 20;
			break;
		case 2:
			wfe.wBitsPerSample = 24;
			break;
	}
	wfe.nBlockAlign = (wfe.nChannels*2*wfe.wBitsPerSample) / 8;
	wfe.nAvgBytesPerSec = (wfe.nBlockAlign*wfe.nSamplesPerSec) / 2;

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = MEDIASUBTYPE_DVD_LPCM_AUDIO;
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	// TODO: what to do with dvd-audio lpcm?

	return true;
}

bool CBaseSplitterFileEx::Read(dvdspuhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if(!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = MEDIASUBTYPE_DVD_SUBPICTURE;
	pmt->formattype = FORMAT_None;

	return true;
}

bool CBaseSplitterFileEx::Read(hdmvsubhdr& h, CMediaType* pmt, const char* language_code)
{
	memset(&h, 0, sizeof(h));

	if(!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Subtitle;
	pmt->subtype = MEDIASUBTYPE_HDMVSUB;
	pmt->formattype = FORMAT_None;

	SUBTITLEINFO* psi = (SUBTITLEINFO*)pmt->AllocFormatBuffer(sizeof(SUBTITLEINFO));
	if (psi) {
		memset(psi, 0, pmt->FormatLength());
		strcpy(psi->IsoLang, language_code ? language_code : "eng");
	}

	return true;
}

bool CBaseSplitterFileEx::Read(svcdspuhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if(!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = MEDIASUBTYPE_SVCD_SUBPICTURE;
	pmt->formattype = FORMAT_None;

	return true;
}

bool CBaseSplitterFileEx::Read(cvdspuhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if(!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = MEDIASUBTYPE_CVD_SUBPICTURE;
	pmt->formattype = FORMAT_None;

	return true;
}

bool CBaseSplitterFileEx::Read(ps2audhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if(BitRead(16, true) != 'SS') {
		return false;
	}

	__int64 pos = GetPos();

	while(BitRead(16, true) == 'SS') {
		DWORD tag = (DWORD)BitRead(32, true);
		DWORD size = 0;

		if(tag == 'SShd') {
			BitRead(32);
			ByteRead((BYTE*)&size, sizeof(size));
			ASSERT(size == 0x18);
			Seek(GetPos());
			ByteRead((BYTE*)&h, sizeof(h));
		} else if(tag == 'SSbd') {
			BitRead(32);
			ByteRead((BYTE*)&size, sizeof(size));
			break;
		}
	}

	Seek(pos);

	if(!pmt) {
		return true;
	}

	WAVEFORMATEXPS2 wfe;
	wfe.wFormatTag =
		h.unk1 == 0x01 ? WAVE_FORMAT_PS2_PCM :
		h.unk1 == 0x10 ? WAVE_FORMAT_PS2_ADPCM :
		WAVE_FORMAT_UNKNOWN;
	wfe.nChannels = (WORD)h.channels;
	wfe.nSamplesPerSec = h.freq;
	wfe.wBitsPerSample = 16; // always?
	wfe.nBlockAlign = wfe.nChannels*wfe.wBitsPerSample>>3;
	wfe.nAvgBytesPerSec = wfe.nBlockAlign*wfe.nSamplesPerSec;
	wfe.dwInterleave = h.interleave;

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = FOURCCMap(wfe.wFormatTag);
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(ps2subhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if(!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Subtitle;
	pmt->subtype = MEDIASUBTYPE_PS2_SUB;
	pmt->formattype = FORMAT_None;

	return true;
}

bool CBaseSplitterFileEx::Read(trhdr& h, bool fSync)
{
	memset(&h, 0, sizeof(h));

	BitByteAlign();

	if(m_tslen == 0) {
		__int64 pos = GetPos();

		for(int i = 0; i < 192; i++) {
			if(BitRead(8, true) == 0x47) {
				__int64 pos = GetPos();
				Seek(pos + 188);
				if(BitRead(8, true) == 0x47) {
					m_tslen = 188;    // TS stream
					break;
				}
				Seek(pos + 192);
				if(BitRead(8, true) == 0x47) {
					m_tslen = 192;    // M2TS stream
					break;
				}
			}

			BitRead(8);
		}

		Seek(pos);

		if(m_tslen == 0) {
			return false;
		}
	}

	if(fSync) {
		for(int i = 0; i < m_tslen; i++) {
			if(BitRead(8, true) == 0x47) {
				if(i == 0) {
					break;
				}
				Seek(GetPos()+m_tslen);
				if(BitRead(8, true) == 0x47) {
					Seek(GetPos()-m_tslen);
					break;
				}
			}

			BitRead(8);

			if(i == m_tslen-1) {
				return false;
			}
		}
	}

	if(BitRead(8, true) != 0x47) {
		return false;
	}

	h.next = GetPos() + m_tslen;

	h.sync = (BYTE)BitRead(8);
	h.error = BitRead(1);
	h.payloadstart = BitRead(1);
	h.transportpriority = BitRead(1);
	h.pid = BitRead(13);
	h.scrambling = BitRead(2);
	h.adapfield = BitRead(1);
	h.payload = BitRead(1);
	h.counter = BitRead(4);

	h.bytes = 188 - 4;

	if(h.adapfield) {
		h.length = (BYTE)BitRead(8);

		if(h.length > 0) {
			h.discontinuity = BitRead(1);
			h.randomaccess = BitRead(1);
			h.priority = BitRead(1);
			h.fPCR = BitRead(1);
			h.OPCR = BitRead(1);
			h.splicingpoint = BitRead(1);
			h.privatedata = BitRead(1);
			h.extension = BitRead(1);

			int i = 1;

			if(h.fPCR) {
				h.PCR = BitRead(33);
				BitRead(6);
				UINT64 PCRExt = BitRead(9);
				h.PCR = (h.PCR*300 + PCRExt) * 10 / 27;
				i += 6;
			}

			ASSERT(i <= h.length);

			for(; i < h.length; i++) {
				BitRead(8);
			}
		}

		h.bytes -= h.length+1;

		if(h.bytes < 0) {
			ASSERT(0);
			return false;
		}
	}
	return true;
}

bool CBaseSplitterFileEx::Read(trsechdr& h)
{
	memset(&h, 0, sizeof(h));

	BYTE pointer_field = BitRead(8);
	while(pointer_field-- > 0) {
		BitRead(8);
	}
	h.table_id = BitRead(8);
	h.section_syntax_indicator = BitRead(1);
	h.zero = BitRead(1);
	h.reserved1 = BitRead(2);
	h.section_length = BitRead(12);
	h.transport_stream_id = BitRead(16);
	h.reserved2 = BitRead(2);
	h.version_number = BitRead(5);
	h.current_next_indicator = BitRead(1);
	h.section_number = BitRead(8);
	h.last_section_number = BitRead(8);
	return h.section_syntax_indicator == 1 && h.zero == 0;
}

bool CBaseSplitterFileEx::Read(pvahdr& h, bool fSync)
{
	memset(&h, 0, sizeof(h));

	BitByteAlign();

	if(fSync) {
		for(int i = 0; i < 65536; i++) {
			if((BitRead(64, true)&0xfffffc00ffe00000i64) == 0x4156000055000000i64) {
				break;
			}
			BitRead(8);
		}
	}

	if((BitRead(64, true)&0xfffffc00ffe00000i64) != 0x4156000055000000i64) {
		return false;
	}

	h.sync = (WORD)BitRead(16);
	h.streamid = (BYTE)BitRead(8);
	h.counter = (BYTE)BitRead(8);
	h.res1 = (BYTE)BitRead(8);
	h.res2 = BitRead(3);
	h.fpts = BitRead(1);
	h.postbytes = BitRead(2);
	h.prebytes = BitRead(2);
	h.length = (WORD)BitRead(16);

	if(h.length > 6136) {
		return false;
	}

	__int64 pos = GetPos();

	if(h.streamid == 1 && h.fpts) {
		h.pts = 10000*BitRead(32)/90 + m_rtPTSOffset;
	} else if(h.streamid == 2 && (h.fpts || (BitRead(32, true)&0xffffffe0) == 0x000001c0)) {
		BYTE b;
		if(!NextMpegStartCode(b, 4)) {
			return false;
		}
		peshdr h2;
		if(!Read(h2, b)) {
			return false;
		}
		// Maybe bug, code before: if(h.fpts = h2.fpts) h.pts = h2.pts;
		h.fpts = h2.fpts;
		if(h.fpts) {
			h.pts = h2.pts;
		}
	}

	BitRead(8*h.prebytes);

	h.length -= GetPos() - pos;

	return true;
}

int CBaseSplitterFileEx::HrdParameters(CGolombBuffer& gb)
{
	unsigned int cnt = gb.UExpGolombRead();	// cpb_cnt_minus1
	if(cnt > 32U) 
		return -1;
	gb.BitRead(4);							// bit_rate_scale
	gb.BitRead(4);							// cpb_size_scale

	for(unsigned int i = 0; i <= cnt; i++ ) {
		gb.UExpGolombRead();				// bit_rate_value_minus1
		gb.UExpGolombRead();				// cpb_size_value_minus1
		gb.BitRead(1);						// cbr_flag
	}

	gb.BitRead(5);							// initial_cpb_removal_delay_length_minus1
	gb.BitRead(5);							// cpb_removal_delay_length_minus1
	gb.BitRead(5);							// dpb_output_delay_length_minus1
	gb.BitRead(5);							// time_offset_length
	
	return 0;
}

void CBaseSplitterFileEx::RemoveMpegEscapeCode(BYTE* dst, BYTE* src, int length)
{
	int		si=0;
	int		di=0;
	while(si+2<length) {
		//remove escapes (very rare 1:2^22)
		if(src[si+2]>3) {
			dst[di++]= src[si++];
			dst[di++]= src[si++];
		} else if(src[si]==0 && src[si+1]==0) {
			if(src[si+2]==3) { //escape
				dst[di++]= 0;
				dst[di++]= 0;
				si+=3;
				continue;
			} else { //next start code
				return;
			}
		}

		dst[di++]= src[si++];
	}
}

bool CBaseSplitterFileEx::Read(avchdr& h, int len, CMediaType* pmt)
{
	__int64 endpos = GetPos() + len;
	__int64 nalstartpos = GetPos();
	//__int64 nalendpos;
	bool repeat = false;

	// First try search for the start code
	DWORD _dwStartCode = BitRead(32, true);
	while(GetPos() < endpos+4 &&
			(_dwStartCode & 0xFFFFFF1F) != 0x101 &&		// Coded slide of a non-IDR
			(_dwStartCode & 0xFFFFFF1F) != 0x105 &&		// Coded slide of an IDR
			(_dwStartCode & 0xFFFFFF1F) != 0x107 &&		// Sequence Parameter Set
			(_dwStartCode & 0xFFFFFF1F) != 0x108 &&		// Picture Parameter Set
			(_dwStartCode & 0xFFFFFF1F) != 0x109 &&		// Access Unit Delimiter
			(_dwStartCode & 0xFFFFFF1F) != 0x10f		// Subset Sequence Parameter Set (MVC)
		) {
		BitRead(8);
		_dwStartCode = BitRead(32, true);
	}
	if(GetPos() >= endpos+4) {
		return false;
	}

	// At least a SPS (normal or subset) and a PPS is required
	while(GetPos() < endpos+4 && (!(h.spspps[index_sps].complete || h.spspps[index_subsetsps].complete) || !h.spspps[index_pps1].complete || repeat))
	{
		BYTE id = h.lastid;
		repeat = false;

		// Get array index from NAL unit type
		spsppsindex index = index_unknown;

		if((id&0x60) != 0) {
			if((id&0x9f) == 0x07) {
				index = index_sps;
			} else if((id&0x9f) == 0x0F) {
				index = index_subsetsps;
			} else if((id&0x9f) == 0x08) {
				index = h.spspps[index_pps1].complete ? index_pps2 : index_pps1;
			}
		}

		// Search for next start code
		DWORD dwStartCode = BitRead(32, true);
		while(GetPos() < endpos+4 && (dwStartCode != 0x00000001) && (dwStartCode & 0xFFFFFF00) != 0x00000100) {
			BitRead(8);
			dwStartCode = BitRead(32, true);
		}

		//nalendpos = GetPos();

		// Skip start code
		__int64 pos;
		if(GetPos() < endpos+4) {
			if (dwStartCode == 0x00000001)
				BitRead(32);
			else
				BitRead(24);

			pos = GetPos();
			h.lastid = BitRead(8);
		} else {
			pos = GetPos()-4;
		}

		// The SPS or PPS might be fragmented, copy data into buffer until NAL is complete
		if(index >= 0) {
			if(h.spspps[index].complete) {
				// Don't handle SPS/PPS twice
				continue;
			} else if(pos > nalstartpos) {
				// Copy into buffer
				Seek(nalstartpos);
				unsigned int bufsize = countof(h.spspps[index].buffer);
				int len = min(bufsize - h.spspps[index].size, pos - nalstartpos);
				ByteRead(h.spspps[index].buffer+h.spspps[index].size, len);
				Seek(pos);
				h.spspps[index].size += len;

				//ASSERT(h.spspps[index].size < bufsize); // disable for better debug ...

				if(h.spspps[index].size >= bufsize || dwStartCode == 0x00000001 || (dwStartCode & 0xFFFFFF00) == 0x00000100) {
					if (Read(h, index)) {
						h.spspps[index].complete = true;
						h.spspps[index].size -= 4;
					} else {
						h.spspps[index].size = 0;
					}
				}

				repeat = true;
			}
		}

		nalstartpos = pos;
	}

	// Exit and wait for next packet if there is no SPS and PPS yet
	if((!h.spspps[index_sps].complete && !h.spspps[index_subsetsps].complete) || !h.spspps[index_pps1].complete || repeat) {

		return false;
	}

	if(!pmt) {
		return true;
	}

	{
		// Calculate size of extra data
		int extra = 0;
		for(int i = 0; i < 4; i++) {
			if(h.spspps[i].complete)
				extra += 2+(h.spspps[i].size);
		}

		pmt->majortype = MEDIATYPE_Video;
		if (h.spspps[index_subsetsps].complete && !h.spspps[index_sps].complete) {
			pmt->subtype = FOURCCMap('CVME');  // MVC stream without base view
		} else if (h.spspps[index_subsetsps].complete && h.spspps[index_sps].complete) {
			pmt->subtype = FOURCCMap('CVMA');  // MVC stream with base view
		} else {
			pmt->subtype = FOURCCMap('1CVA');  // AVC stream
		}
		//pmt->subtype = MEDIASUBTYPE_H264;		// TODO : put MEDIASUBTYPE_H264 to support Windows 7 decoder !
		pmt->formattype = FORMAT_MPEG2_VIDEO;
		int len = FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + extra;
		MPEG2VIDEOINFO* vi = (MPEG2VIDEOINFO*)DNew BYTE[len];
		memset(vi, 0, len);
		// vi->hdr.dwBitRate = ;
		vi->hdr.AvgTimePerFrame = h.AvgTimePerFrame;
		if(!h.sar.num) h.sar.num = 1;
		if(!h.sar.den) h.sar.den = 1;
		CSize aspect(h.width * h.sar.num, h.height * h.sar.den);
		int lnko = LNKO(aspect.cx, aspect.cy);
		if(lnko > 1) {
			aspect.cx /= lnko, aspect.cy /= lnko;
		}

		if(aspect.cx * 2 < aspect.cy) {
			delete[] vi;
			return false;
		}

		vi->hdr.dwPictAspectRatioX = aspect.cx;
		vi->hdr.dwPictAspectRatioY = aspect.cy;
		vi->hdr.bmiHeader.biSize = sizeof(vi->hdr.bmiHeader);
		vi->hdr.bmiHeader.biWidth = h.width;
		vi->hdr.bmiHeader.biHeight = h.height;
		if (h.spspps[index_subsetsps].complete && !h.spspps[index_sps].complete) {
			vi->hdr.bmiHeader.biCompression = 'CVME';
		} else if (h.spspps[index_subsetsps].complete && h.spspps[index_sps].complete) {
			vi->hdr.bmiHeader.biCompression = 'CVMA';
		} else {
			vi->hdr.bmiHeader.biCompression = '1CVA';
		}
		vi->dwProfile = h.profile;
		vi->dwFlags = 4; // ?
		vi->dwLevel = h.level;
		vi->cbSequenceHeader = extra;

		// Copy extra data
		BYTE* p = (BYTE*)&vi->dwSequenceHeader[0];
		for(int i = 0; i < 4; i++) {
			if(h.spspps[i].complete) {
				*p++ = (h.spspps[i].size) >> 8;
				*p++ = (h.spspps[i].size) & 0xff;
				memcpy(p, h.spspps[i].buffer, h.spspps[i].size);
				p += h.spspps[i].size;
			}
		}

		pmt->SetFormat((BYTE*)vi, len);
		delete [] vi;
	}

	return true;
}

bool CBaseSplitterFileEx::Read(avchdr& h, spsppsindex index)
{
	static BYTE profiles[] = {44, 66, 77, 88, 100, 110, 118, 122, 128, 144, 244};
	static BYTE levels[] = {10, 11, 12, 13, 20, 21, 22, 30, 31, 32, 40, 41, 42, 50, 51};

	// Only care about SPS and subset SPS
	if(index != index_sps && index != index_subsetsps)
		return true;

	// Manage escape codes
	BYTE buffer[MAX_SPSPPS];
	RemoveMpegEscapeCode(buffer, h.spspps[index].buffer, MAX_SPSPPS);
	CGolombBuffer gb(buffer, MAX_SPSPPS);

	gb.BitRead(8);							// nal_unit_type
	h.profile = (BYTE)gb.BitRead(8);
	bool b_ident = false;
	for(int i = 0; i<sizeof(profiles); i++) {
		if(h.profile == profiles[i]) {
			b_ident = true;
			break;
		}
	}
	if(!b_ident)
		return false;

	gb.BitRead(8);
	h.level = (BYTE)gb.BitRead(8);
	b_ident = false;
	for(int i = 0; i<sizeof(levels); i++) {
		if(h.level == levels[i]) {
			b_ident = true;
			break;
		}
	}
	if(!b_ident)
		return false;

	unsigned int sps_id = gb.UExpGolombRead();	// seq_parameter_set_id
	if(sps_id >= 32)
		return false;

	UINT64 chroma_format_idc = 0;
	if(h.profile >= 100) {					// high profile
		chroma_format_idc = gb.UExpGolombRead();
		if(chroma_format_idc  == 3) {		// chroma_format_idc
			gb.BitRead(1);					// residue_transform_flag
		}

		gb.UExpGolombRead();				// bit_depth_luma_minus8
		gb.UExpGolombRead();				// bit_depth_chroma_minus8

		gb.BitRead(1);						// qpprime_y_zero_transform_bypass_flag

		if(gb.BitRead(1)) {					// seq_scaling_matrix_present_flag
			for(int i = 0; i < 8; i++) {
				if(gb.BitRead(1)) {			// seq_scaling_list_present_flag
					for(int j = 0, size = i < 6 ? 16 : 64, next = 8; j < size && next != 0; ++j) {
						next = (next + gb.SExpGolombRead() + 256) & 255;
					}
				}
			}
		}
	}

	gb.UExpGolombRead();					// log2_max_frame_num_minus4

	UINT64 pic_order_cnt_type = gb.UExpGolombRead();

	if(pic_order_cnt_type == 0) {
		gb.UExpGolombRead();				// log2_max_pic_order_cnt_lsb_minus4
	} else if(pic_order_cnt_type == 1) {
		gb.BitRead(1);						// delta_pic_order_always_zero_flag
		gb.SExpGolombRead();				// offset_for_non_ref_pic
		gb.SExpGolombRead();				// offset_for_top_to_bottom_field
		UINT64 num_ref_frames_in_pic_order_cnt_cycle = gb.UExpGolombRead();
		if(num_ref_frames_in_pic_order_cnt_cycle >= 256)
			return false;
		for(int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
			gb.SExpGolombRead();			// offset_for_ref_frame[i]
		}
	} else if(pic_order_cnt_type != 2) {
		return false;
	}

	UINT64 ref_frame_count = gb.UExpGolombRead();					// num_ref_frames
	if (ref_frame_count > 30) 
		return false;
	gb.BitRead(1);							// gaps_in_frame_num_value_allowed_flag

	UINT64 pic_width_in_mbs_minus1 = gb.UExpGolombRead();
	UINT64 pic_height_in_map_units_minus1 = gb.UExpGolombRead();
	BYTE frame_mbs_only_flag = (BYTE)gb.BitRead(1);

	if(!frame_mbs_only_flag) {
		gb.BitRead(1);						// mb_adaptive_frame_field_flag
	}

	BYTE direct_8x8_inference_flag = (BYTE)gb.BitRead(1);							// direct_8x8_inference_flag
	if(!frame_mbs_only_flag && !direct_8x8_inference_flag) {
		return false;
	}

	if(gb.BitRead(1)) {						// frame_cropping_flag
		h.crop_left = gb.UExpGolombRead();				// frame_cropping_rect_left_offset
		h.crop_right = gb.UExpGolombRead();				// frame_cropping_rect_right_offset
		h.crop_top = gb.UExpGolombRead();				// frame_cropping_rect_top_offset
		h.crop_bottom = gb.UExpGolombRead();				// frame_cropping_rect_bottom_offset
	}

	if(gb.BitRead(1)) {						// vui_parameters_present_flag
		if(gb.BitRead(1)) {					// aspect_ratio_info_present_flag
			BYTE aspect_ratio_idc = gb.BitRead(8); // aspect_ratio_idc
			if(255==(BYTE)aspect_ratio_idc) {
				h.sar.num = gb.BitRead(16);				// sar_width
				h.sar.den = gb.BitRead(16);				// sar_height
			} else if(aspect_ratio_idc < 17) {
				h.sar.num = pixel_aspect[aspect_ratio_idc][0];
				h.sar.den = pixel_aspect[aspect_ratio_idc][1];
			} else {
				return false;
			}
		} else {
			h.sar.num = 1;
			h.sar.den = 1;
		}

		if(gb.BitRead(1)) {					// overscan_info_present_flag
			gb.BitRead(1);					// overscan_appropriate_flag
		}

		if(gb.BitRead(1)) {					// video_signal_type_present_flag
			gb.BitRead(3);					// video_format
			gb.BitRead(1);					// video_full_range_flag
			if(gb.BitRead(1)) {				// colour_description_present_flag
				gb.BitRead(8);				// colour_primaries
				gb.BitRead(8);				// transfer_characteristics
				gb.BitRead(8);				// matrix_coefficients
			}
		}
		if(gb.BitRead(1)) {					// chroma_location_info_present_flag
			gb.UExpGolombRead();			// chroma_sample_loc_type_top_field
			gb.UExpGolombRead();			// chroma_sample_loc_type_bottom_field
		}
		if(gb.BitRead(1)) {					// timing_info_present_flag
			__int64 num_units_in_tick	= gb.BitRead(32);
			__int64 time_scale			= gb.BitRead(32);
			/*long fixed_frame_rate_flag	= */gb.BitRead(1);

			// Trick for weird parameters
			if ((num_units_in_tick < 1000) || (num_units_in_tick > 1001)) {
				if ((time_scale % num_units_in_tick != 0) && ((time_scale*1001) % num_units_in_tick == 0)) {
					time_scale			= (time_scale * 1001) / num_units_in_tick;
					num_units_in_tick	= 1001;
				} else {
					time_scale			= (time_scale * 1000) / num_units_in_tick;
					num_units_in_tick	= 1000;
				}
			}
			time_scale = time_scale / 2;	// VUI consider fields even for progressive stream : divide by 2!

			if (time_scale) {
				h.AvgTimePerFrame = (10000000I64*num_units_in_tick)/time_scale;
			}
		}

		bool nalflag = !!gb.BitRead(1);		// nal_hrd_parameters_present_flag
		if(nalflag) {
			if(HrdParameters(gb)<0)	
				return false;
		}
		bool vlcflag = !!gb.BitRead(1);		// vlc_hrd_parameters_present_flag
		if(vlcflag) {
			if(HrdParameters(gb)<0) 
				return false;
		}
		if(nalflag || vlcflag) {
			 gb.BitRead(1);					// low_delay_hrd_flag
		}

		gb.BitRead(1);						// pic_struct_present_flag
		if(gb.BitRead(1)) {					// bitstream_restriction_flag
			gb.BitRead(1);					// motion_vectors_over_pic_boundaries_flag
			gb.UExpGolombRead();			// max_bytes_per_pic_denom
			gb.UExpGolombRead();			// max_bits_per_mb_denom
			gb.UExpGolombRead();			// log2_max_mv_length_horizontal
			gb.UExpGolombRead();			// log2_max_mv_length_vertical
			UINT64 num_reorder_frames = gb.UExpGolombRead();			// num_reorder_frames
			gb.UExpGolombRead();			// max_dec_frame_buffering

			if(gb.GetSize() < gb.GetPos()){
				num_reorder_frames = 0;
			}
			if(num_reorder_frames > 16U) 
				return false;
		}
	}

	if(index == index_subsetsps) {
		if(h.profile == 83 || h.profile == 86) {
			// TODO: SVC extensions
			return false;
		} else if(h.profile == 118 || h.profile == 128) {
			gb.BitRead(1);													// bit_equal_to_one

			// seq_parameter_set_mvc_extension
			h.views = (unsigned int) gb.UExpGolombRead()+1;

			/*
			for(unsigned int i = 0; i < h.views; i++) {
				gb.UExpGolombRead();										// view_id
			}

			for(unsigned int i = 1; i < h.views; i++) {
				for(int j = 0; j < gb.UExpGolombRead(); j++) {				// num_anchor_refs_l0
					gb.UExpGolombRead();									// anchor_refs_l0
				}
				for(int j = 0; j < gb.UExpGolombRead(); j++) {				// num_anchor_refs_l1
					gb.UExpGolombRead();									// anchor_refs_l1
				}
			}

			for(unsigned int i = 1; i < h.views; i++) {
				for(int j = 0; j < gb.UExpGolombRead(); j++) {				// num_non_anchor_refs_l0
					gb.UExpGolombRead();									// non_anchor_refs_l0
				}
				for(int j = 0; j < gb.UExpGolombRead(); j++) {				// num_non_anchor_refs_l1
					gb.UExpGolombRead();									// non_anchor_refs_l1
				}
			}

			for(unsigned int i = 0; i <= gb.UExpGolombRead(); i++) {		// num_level_values_signalled_minus1
				gb.BitRead(8);												// level_idc
				for(int j = 0; j <= gb.UExpGolombRead(); j++) {				// num_applicable_ops_minus1
					gb.BitRead(3);											// applicable_op_temporal_id
					for(int k = 0; k <= gb.UExpGolombRead(); k++) {			// applicable_op_num_target_views_minus1
						gb.UExpGolombRead();								// applicable_op_traget_view_id
					}
					gb.UExpGolombRead();									// applicable_op_num_views_minus1
				}
			}

			if(gb.BitRead(1)) {													// mvc_vui_parameters_present_flag
				// mvc_vui_parameters_extension
				for(unsigned int i = 0; i <= gb.UExpGolombRead(); i++) {		// vui_mvc_num_ops_minus1
					gb.BitRead(3);
					for(unsigned int j = 0; j <= gb.UExpGolombRead(); j++) {	// vui_mvc_num_target_output_views_minus1
						gb.UExpGolombRead();									// vui_mvc_view_id
					}
					if(gb.BitRead(1)) {											// vui_mvc_timing_info_present_flag
						gb.BitRead(32);											// vui_mvc_num_units_in_tick
						gb.BitRead(32);											// vui_mvc_time_scale
						gb.BitRead(1);											// vui_mvc_fixed_frame_rate_flag
					}
					bool nalflag = gb.BitRead(1);								// vui_mvc_nal_hrd_parameters_present_flag
					if(nalflag) {
						HrdParameters(gb);
					}
					bool vclflag = gb.BitRead(1);								// vui_mvc_vcl_hrd_parameters_present_flag
					if(vclflag) {
						HrdParameters(gb);
					}
					if(nalflag || vclflag) {
						gb.BitRead(1);											// vui_mvc_low_delay_hrd_flag
					}
					gb.BitRead(1);												// vui_mvc_pic_struct_present_flag
				}
			}
			*/
		}
	}

	UINT64 mb_Width = pic_width_in_mbs_minus1 + 1;
	UINT64 mb_Height = (pic_height_in_map_units_minus1 + 1) * (2 - frame_mbs_only_flag);
	BYTE CHROMA444 = (chroma_format_idc == 3);

	h.width = 16 * mb_Width - (2>>CHROMA444) * min(h.crop_right, (8<<CHROMA444)-1);
	if(frame_mbs_only_flag) {
		h.height = 16 * mb_Height - (2>>CHROMA444) * min(h.crop_bottom, (8<<CHROMA444)-1);
	} else {
		h.height = 16 * mb_Height - (4>>CHROMA444) * min(h.crop_bottom, (8<<CHROMA444)-1);
	}

	if(h.height<100 || h.width<100) {
		return false;
	}

	if(h.height == 1088) {
		h.height = 1080;	// Prevent blur lines 
	}

	return true;
}

bool CBaseSplitterFileEx::Read(vc1hdr& h, int len, CMediaType* pmt, int guid_flag)
{
	memset(&h, 0, sizeof(h));

	__int64 endpos = GetPos() + len; // - sequence header length
	__int64 extrapos = 0, extralen = 0;
	int		nFrameRateNum = 0, nFrameRateDen = 1;

	if (GetPos() < endpos+4 && BitRead(32, true) == 0x0000010F) {
		extrapos = GetPos();

		BitRead(32);

		h.profile	= BitRead(2);

		// Check if advanced profile
		if (h.profile != 3) {
			return false;
		}

		h.level = BitRead (3);
		h.chromaformat = BitRead (2);

		// (fps-2)/4 (->30)
		h.frmrtq_postproc	= BitRead (3); //common
		// (bitrate-32kbps)/64kbps
		h.bitrtq_postproc	= BitRead (5); //common
		h.postprocflag		= BitRead (1); //common

		h.width				= (BitRead (12) + 1) << 1;
		h.height			= (BitRead (12) + 1) << 1;

		h.broadcast			= BitRead (1);
		h.interlace			= BitRead (1);
		h.tfcntrflag		= BitRead (1);
		h.finterpflag		= BitRead (1);
		BitRead (1); // reserved
		h.psf				= BitRead (1);
		if(BitRead (1)) {
			int ar = 0;
			BitRead (14);
			BitRead (14);
			if(BitRead (1)) {
				ar = BitRead (4);
			}
			if(ar && ar < 14) {
				h.sar.num = pixel_aspect[ar][0];
				h.sar.den = pixel_aspect[ar][1];
			} else if(ar == 15) {
				h.sar.num = BitRead (8);
				h.sar.den = BitRead (8);
			}

			// Read framerate
			const int	ff_vc1_fps_nr[5] = { 24, 25, 30, 50, 60 },
										   ff_vc1_fps_dr[2] = { 1000, 1001 };

			if(BitRead (1)) {
				if(BitRead (1)) {
					nFrameRateNum = 32;
					nFrameRateDen = BitRead (16) + 1;
				} else {
					int nr, dr;
					nr = BitRead (8);
					dr = BitRead (4);
					if(nr && nr < 8 && dr && dr < 3) {
						nFrameRateNum = ff_vc1_fps_dr[dr - 1];
						nFrameRateDen = ff_vc1_fps_nr[nr - 1] * 1000;
					}
				}
			}

		}

		Seek(extrapos+4);
		extralen = 0;
		long	parse = 0;

		while (GetPos() < endpos+4 && ((parse == 0x0000010E) || (parse & 0xFFFFFF00) != 0x00000100)) {
			parse = (parse<<8) | BitRead(8);
			extralen++;
		}
	}

	if(!extrapos || !extralen) {
		return false;
	}

	if(!pmt) {
		return true;
	}

	{
		pmt->majortype = MEDIATYPE_Video;
		switch (guid_flag) {
			case 1: pmt->subtype = FOURCCMap('1CVW');
				break;
			case 2: pmt->subtype = MEDIASUBTYPE_WVC1_CYBERLINK;
				break;
			case 3: pmt->subtype = MEDIASUBTYPE_WVC1_ARCSOFT;
				break;
		}
		pmt->formattype = FORMAT_VIDEOINFO2;
		int len = sizeof(VIDEOINFOHEADER2) + extralen + 1;
		VIDEOINFOHEADER2* vi = (VIDEOINFOHEADER2*)DNew BYTE[len];
		memset(vi, 0, len);
		vi->AvgTimePerFrame = (10000000I64*nFrameRateNum)/nFrameRateDen;

		if(!h.sar.num) h.sar.num = 1;
		if(!h.sar.den) h.sar.den = 1;
		CSize aspect = CSize(h.width * h.sar.num, h.height * h.sar.den);
		if(h.width == h.sar.num && h.height == h.sar.den) {
			aspect = CSize(h.width, h.height);
		}
		int lnko = LNKO(aspect.cx, aspect.cy);
		if(lnko > 1) {
			aspect.cx /= lnko, aspect.cy /= lnko;
		}

		vi->dwPictAspectRatioX = aspect.cx;
		vi->dwPictAspectRatioY = aspect.cy;
		vi->bmiHeader.biSize = sizeof(vi->bmiHeader);
		vi->bmiHeader.biWidth = h.width;
		vi->bmiHeader.biHeight = h.height;
		vi->bmiHeader.biCompression = '1CVW';
		BYTE* p = (BYTE*)vi + sizeof(VIDEOINFOHEADER2);
		*p++ = 0;
		Seek(extrapos);
		ByteRead(p, extralen);
		pmt->SetFormat((BYTE*)vi, len);
		delete [] vi;
	}
	return true;
}

bool CBaseSplitterFileEx::Read(dvbsub& h, int len, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if ((BitRead(32, true) & 0xFFFFFF00) == 0x20000f00) {
		static const SUBTITLEINFO SubFormat = { 0, "", L"" };

		pmt->majortype		= MEDIATYPE_Subtitle;
		pmt->subtype		= MEDIASUBTYPE_DVB_SUBTITLES;
		pmt->formattype		= FORMAT_None;
		pmt->SetFormat ((BYTE*)&SubFormat, sizeof(SUBTITLEINFO));

		return true;
	}

	return false;
}

/*

To see working buffer in debugger, look :
	- m_pCache.m_p	 for the cached buffer
	- m_pos			 for current read position

*/
