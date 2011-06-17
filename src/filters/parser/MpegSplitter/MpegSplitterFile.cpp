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
#include "MpegSplitterFile.h"

#include <initguid.h>
#include <moreuuids.h>

#define MEGABYTE 1024*1024

CMpegSplitterFile::CMpegSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr, bool bIsHdmv, CHdmvClipInfo &ClipInfo, int guid_flag, bool ForcedSub)
	: CBaseSplitterFileEx(pAsyncReader, hr, DEFAULT_CACHE_LENGTH, false, true)
	, m_type(us)
	, m_rate(0)
	, m_rtMin(0), m_rtMax(0)
	, m_posMin(0), m_posMax(0)
	, m_bIsHdmv(bIsHdmv)
	, m_ClipInfo(ClipInfo)
	, m_nVC1_GuidFlag(guid_flag)
	, m_ForcedSub(ForcedSub)
{
	if(SUCCEEDED(hr)) {
		hr = Init(pAsyncReader);
	}
}

HRESULT CMpegSplitterFile::Init(IAsyncReader* pAsyncReader)
{
	HRESULT hr;

	// get the type first

	m_type = us;

	Seek(0);

	if(m_type == us) {
		if(BitRead(32, true) == 'TFrc') {
			Seek(0x67c);
		}
		int cnt = 0, limit = 4;
		for(trhdr h; cnt < limit && Read(h); cnt++) {
			Seek(h.next);
		}
		if(cnt >= limit) {
			m_type = ts;
		}
	}

	Seek(0);

	if(m_type == us) {
		if(BitRead(32, true) == 'TFrc') {
			Seek(0xE80);
		}
		int cnt = 0, limit = 4;
		for(trhdr h; cnt < limit && Read(h); cnt++) {
			Seek(h.next);
		}
		if(cnt >= limit) {
			m_type = ts;
		}
	}

	Seek(0);

	if(m_type == us) {
		int cnt = 0, limit = 4;
		for(pvahdr h; cnt < limit && Read(h); cnt++) {
			Seek(GetPos() + h.length);
		}
		if(cnt >= limit) {
			m_type = pva;
		}
	}

	Seek(0);

	if(m_type == us) {
		BYTE b;
		for(int i = 0; (i < 4 || GetPos() < 65536) && m_type == us && NextMpegStartCode(b); i++) {
			if(b == 0xba) {
				pshdr h;
				if(Read(h)) {
					m_type = ps;
					m_rate = int(h.bitrate/8);
					break;
				}
			} else if((b&0xe0) == 0xc0 // audio, 110xxxxx, mpeg1/2/3
					  || (b&0xf0) == 0xe0 // video, 1110xxxx, mpeg1/2
					  // || (b&0xbd) == 0xbd) // private stream 1, 0xbd, ac3/dts/lpcm/subpic
					  || b == 0xbd) { // private stream 1, 0xbd, ac3/dts/lpcm/subpic
				peshdr h;
				if(Read(h, b) && BitRead(24, true) == 0x000001) {
					m_type = es;
				}
			}
		}
	}

	Seek(0);

	if(m_type == us) {
		return E_FAIL;
	}

	// min/max pts & bitrate
	m_rtMin = m_posMin = _I64_MAX;
	m_rtMax = m_posMax = 0;

	if(IsRandomAccess() || IsStreaming()) {
		if(IsStreaming()) {
			for(int i = 0; i < 20 || i < 50 && S_OK != HasMoreData(1024*100, 100); i++) {
				;
			}
		}

		CAtlList<__int64> fps;
		for(int i = 0, j = 5; i <= j; i++) {
			fps.AddTail(i*GetLength()/j);
		}

		for(__int64 pfp = 0; fps.GetCount(); ) {
			__int64 fp = fps.RemoveHead();
			fp = min(GetLength() - MEGABYTE/8, fp);
			fp = max(pfp, fp);
			__int64 nfp = fp + (pfp == 0 ? 10*MEGABYTE : MEGABYTE/8);
			if(FAILED(hr = SearchStreams(fp, nfp, pAsyncReader))) {
				return hr;
			}
			pfp = nfp;
		}
	} else {
		if(FAILED(hr = SearchStreams(0, MEGABYTE/8, pAsyncReader))) {
			return hr;
		}
	}

	if(m_posMax - m_posMin <= 0 || m_rtMax - m_rtMin <= 0) {
		return E_FAIL;
	}

	int indicated_rate = m_rate;
	int detected_rate = int(10000000i64 * (m_posMax - m_posMin) / (m_rtMax - m_rtMin));
	// normally "detected" should always be less than "indicated", but sometimes it can be a few percent higher (+10% is allowed here)
	// (update: also allowing +/-50k/s)
	if(indicated_rate == 0 || ((float)detected_rate / indicated_rate) < 1.1 || abs(detected_rate - indicated_rate) < 50*1024) {
		m_rate = detected_rate;
	} else {
		;    // TODO: in this case disable seeking, or try doing something less drastical...
	}

	// Add fake Subtitle stream ...
	if(m_streams[video].GetCount()) {
		if (!m_bIsHdmv && m_streams[subpic].GetCount()) {
			stream s;
			s.pid = NO_SUBTITLE_PID;
			s.mt.majortype = m_streams[subpic].GetHead().mt.majortype;
			s.mt.subtype = m_streams[subpic].GetHead().mt.subtype;
			s.mt.formattype = m_streams[subpic].GetHead().mt.formattype;
			m_streams[subpic].Insert(s, this);
		} else {
			AddHdmvPGStream(NO_SUBTITLE_PID, "---");
		}
	}

	Seek(0);

	return S_OK;
}

void CMpegSplitterFile::OnComplete(IAsyncReader* pAsyncReader)
{
	__int64 pos = GetPos();

	if(SUCCEEDED(SearchStreams(GetLength() - 500*1024, GetLength(), pAsyncReader))) {
		int indicated_rate = m_rate;
		int detected_rate = int(m_rtMax > m_rtMin ? 10000000i64 * (m_posMax - m_posMin) / (m_rtMax - m_rtMin) : 0);
		// normally "detected" should always be less than "indicated", but sometimes it can be a few percent higher (+10% is allowed here)
		// (update: also allowing +/-50k/s)
		if(indicated_rate == 0 || ((float)detected_rate / indicated_rate) < 1.1 || abs(detected_rate - indicated_rate) < 50*1024) {
			m_rate = detected_rate;
		} else {
			;    // TODO: in this case disable seeking, or try doing something less drastical...
		}
	}

	Seek(pos);
}

REFERENCE_TIME CMpegSplitterFile::NextPTS(DWORD TrackNum)
{
	REFERENCE_TIME rt = -1;
	__int64 rtpos = -1;

	BYTE b;

	while(GetRemaining()) {
		if(m_type == ps || m_type == es) {
			if(!NextMpegStartCode(b)) {	// continue;
				ASSERT(0);
				break;
			}

			rtpos = GetPos()-4;

#if (EVO_SUPPORT == 0)
			if(b >= 0xbd && b < 0xf0)
#else
			if((b >= 0xbd && b < 0xf0) || (b == 0xfd))
#endif
			{
				peshdr h;
				if(!Read(h, b) || !h.len) {
					continue;
				}

				__int64 pos = GetPos();

				if(h.fpts && AddStream(0, b, h.len) == TrackNum) {
					ASSERT(h.pts >= m_rtMin && h.pts <= m_rtMax);
					rt = h.pts;
					break;
				}

				Seek(pos + h.len);
			}
		} else if(m_type == ts) {
			trhdr h;
			if(!Read(h)) {
				continue;
			}

			rtpos = GetPos()-4;

			if(h.payload && h.payloadstart && ISVALIDPID(h.pid)) {
				peshdr h2;
				if(NextMpegStartCode(b, 4) && Read(h2, b)) { // pes packet
					if(h2.fpts && AddStream(h.pid, b, DWORD(h.bytes - (GetPos() - rtpos)) == TrackNum)) {
						ASSERT(h2.pts >= m_rtMin && h2.pts <= m_rtMax);
						rt = h2.pts;
						break;
					}
				}
			}

			Seek(h.next);
		} else if(m_type == pva) {
			pvahdr h;
			if(!Read(h)) {
				continue;
			}

			if(h.fpts) {
				rt = h.pts;
				break;
			}
		}
	}

	if(rtpos >= 0) {
		Seek(rtpos);
	}
	if(rt >= 0) {
		rt -= m_rtMin;
	}

	return rt;
}

HRESULT CMpegSplitterFile::SearchStreams(__int64 start, __int64 stop, IAsyncReader* pAsyncReader)
{
	Seek(start);
	stop = min(stop, GetLength());

	while(GetPos() < stop) {
		BYTE b;

		if(m_type == ps || m_type == es) {
			if(!NextMpegStartCode(b)) {
				continue;
			}

			if(b == 0xba) { // program stream header
				pshdr h;
				if(!Read(h)) {
					continue;
				}
			} else if(b == 0xbb) { // program stream system header
				pssyshdr h;
				if(!Read(h)) {
					continue;
				}
			}
#if (EVO_SUPPORT == 0)
			else if(b >= 0xbd && b < 0xf0) // pes packet
#else
			else if((b >= 0xbd && b < 0xf0) || (b == 0xfd)) // pes packet
#endif
			{
				peshdr h;
				if(!Read(h, b)) {
					continue;
				}

				if(h.type == mpeg2 && h.scrambling) {
					ASSERT(0);
					return E_FAIL;
				}

				if(h.fpts) {
					if(m_rtMin == _I64_MAX) {
						m_rtMin = h.pts;
						m_posMin = GetPos();
					}
					if(m_rtMin < h.pts && m_rtMax < h.pts) {
						m_rtMax = h.pts;
						m_posMax = GetPos();
					}
				}

				__int64 pos = GetPos();
				AddStream(0, b, h.len);
				if(h.len) {
					Seek(pos + h.len);
				}
			}
		} else if(m_type == ts) {
			trhdr h;
			if(!Read(h)) {
				continue;
			}

			__int64 pos = GetPos();

			UpdatePrograms(h);

			if(h.payload && ISVALIDPID(h.pid)) {
				peshdr h2;
				if(h.payloadstart && NextMpegStartCode(b, 4) && Read(h2, b)) { // pes packet
					if(h2.type == mpeg2 && h2.scrambling) {
						ASSERT(0);
						return E_FAIL;
					}

					if(h2.fpts) {
						if(m_rtMin == _I64_MAX) {
							m_rtMin = h2.pts;
							m_posMin = GetPos();
							//TRACE ("m_rtMin(SearchStreams)=%S\n", ReftimeToString(m_rtMin));
						}

						if(m_rtMin < h2.pts && m_rtMax < h2.pts) {
							m_rtMax = h2.pts;
							m_posMax = GetPos();
							// Ugly code : to support BRD H264 seamless playback, CMultiFiles need to update m_rtPTSOffset variable
							// each time a new part is open...
							// use this code only if Blu-ray is detected
							if (m_ClipInfo.IsHdmv()) {
								for (int i=0; i<m_ClipInfo.GetStreamNumber(); i++) {
									CHdmvClipInfo::Stream* stream = m_ClipInfo.GetStreamByIndex(i);
									if (stream->m_Type == VIDEO_STREAM_H264 && m_rtMin == 116506666) {
										CComQIPtr<ISyncReader>	pReader = pAsyncReader;
										if (pReader) pReader->SetPTSOffset (&m_rtPTSOffset);
										//TRACE ("UPDATE m_rtPTSOffset(SearchStreams)=%S\n", ReftimeToString(m_rtPTSOffset));
										//TRACE ("m_rtMin(Boucle)=%S\n", ReftimeToString(m_rtMin));
										//TRACE ("stream=%d\n", stream->m_Type);
										//TRACE ("m_rtMax(Boucle)=%S\n", ReftimeToString(m_rtMax));
										//TRACE ("m_rtMax - m_rtMin(Boucle)=%S\n", ReftimeToString(m_rtMax - m_rtMin));
									}
								}
							}
							//TRACE ("m_rtMax(SearchStreams)=%S\n", ReftimeToString(m_rtMax));
						}
					}
				} else {
					b = 0;
				}

				AddStream(h.pid, b, DWORD(h.bytes - (GetPos() - pos)));
			}

			Seek(h.next);
		} else if(m_type == pva) {
			pvahdr h;
			if(!Read(h)) {
				continue;
			}

			if(h.fpts) {
				if(m_rtMin == _I64_MAX) {
					m_rtMin = h.pts;
					m_posMin = GetPos();
				}

				if(m_rtMin < h.pts && m_rtMax < h.pts) {
					m_rtMax = h.pts;
					m_posMax = GetPos();
				}
			}

			__int64 pos = GetPos();

			if(h.streamid == 1) {
				AddStream(h.streamid, 0xe0, h.length);
			} else if(h.streamid == 2) {
				AddStream(h.streamid, 0xc0, h.length);
			}

			if(h.length) {
				Seek(pos + h.length);
			}
		}
	}

	return S_OK;
}

DWORD CMpegSplitterFile::AddStream(WORD pid, BYTE pesid, DWORD len)
{
	if(pid) {
		if(pesid) {
			m_pid2pes[pid] = pesid;
		} else {
			m_pid2pes.Lookup(pid, pesid);
		}
	}

	stream s;
	s.pid = pid;
	s.pesid = pesid;

	int type = unknown;
	bool vc1_frame_found = false;

	if(pesid >= 0xe0 && pesid < 0xf0) { // mpeg video
		__int64 pos = GetPos();

		if(type == unknown) {
			CMpegSplitterFile::seqhdr h;
			if(!m_streams[video].Find(s) && Read(h, len, &s.mt)) {
				type = video;
			}
		}

		if(type == unknown) {
			Seek(pos);
			// PPS and SPS can be present on differents packets
			// and can also be split into multiple packets
			if (!avch.Lookup(pid))
				memset(&avch[pid], 0, sizeof(CMpegSplitterFile::avchdr));
#if defined(MVC_SUPPORT)
			if(!m_streams[video].Find(s) && !m_streams[stereo].Find(s) && Read(avch[pid], len, &s.mt))
			{
				if (avch[pid].spspps[index_subsetsps].complete)
					type = stereo;
				else
					type = video;
			}
#else
			if(!m_streams[video].Find(s) && Read(avch[pid], len, &s.mt)) {
				type = video;
			}
#endif
		}
	} else if(pesid >= 0xc0 && pesid < 0xe0) { // mpeg audio
		__int64 pos = GetPos();

		if(type == unknown) {
			CMpegSplitterFile::aachdr h;
			if(!m_streams[audio].Find(s) && Read(h, len, &s.mt)) {
				type = audio;
			}
		}

		if(type == unknown) {
			Seek(pos);
			CMpegSplitterFile::mpahdr h;
			if(!m_streams[audio].Find(s) && Read(h, len, false, &s.mt)) {
				type = audio;
			}
		}
	} else if(pesid == 0xbd || pesid == 0xfd) { // private stream 1
		if(s.pid) {
			if(!m_streams[audio].Find(s) && !m_streams[video].Find(s)) {
				__int64 pos = GetPos();

				// AC3
				if(type == unknown) {
					CMpegSplitterFile::ac3hdr h;
					if(Read(h, len, &s.mt)) {
						type = audio;
					}
				}

				// DTS
				if(type == unknown) {
					Seek(pos);
					CMpegSplitterFile::dtshdr h;
					if(Read(h, len, &s.mt)) {
						type = audio;
					}
				}

				// VC1
				if(type == unknown) {
					Seek(pos);
					CMpegSplitterFile::vc1hdr h;
					if(!m_streams[video].Find(s) && Read(h, len, &s.mt, m_nVC1_GuidFlag)) {
						type = video;
					}
				}

				// DVB subtitles
				if(type == unknown) {
					Seek(pos);
					CMpegSplitterFile::dvbsub h;
					if(!m_streams[video].Find(s) && Read(h, len, &s.mt)) {
						type = subpic;
					}
				}

				int iProgram;
				const CHdmvClipInfo::Stream *pClipInfo;
				const program* pProgram = FindProgram (s.pid, iProgram, pClipInfo);
				if((type == unknown) && (pProgram != NULL)) {
					PES_STREAM_TYPE	StreamType = INVALID;

					Seek(pos);
					StreamType = pProgram->streams[iProgram].type;

					switch (StreamType) {
						case AUDIO_STREAM_LPCM : {
							CMpegSplitterFile::hdmvlpcmhdr h;
							if(!m_streams[audio].Find(s) && Read(h, &s.mt)) {
								type = audio;
							}
						}
						break;
						case PRESENTATION_GRAPHICS_STREAM : {
							CMpegSplitterFile::hdmvsubhdr h;
							if(!m_streams[subpic].Find(s) && Read(h, &s.mt, pClipInfo ? pClipInfo->m_LanguageCode : NULL)) {
								m_bIsHdmv = true;
								type = subpic;
							}
						}
						break;
					}
				}
			}
		}
#if (EVO_SUPPORT != 0)
		else if (pesid == 0xfd) {	// TODO EVO SUPPORT
			CMpegSplitterFile::vc1hdr h;
			if(!m_streams[video].Find(s) && Read(h, len, &s.mt, m_nVC1_GuidFlag)) {
				type = video;
			}
		}
#endif
		else {
			BYTE b = (BYTE)BitRead(8, true);
			WORD w = (WORD)BitRead(16, true);
			DWORD dw = (DWORD)BitRead(32, true);

			if(b >= 0x80 && b < 0x88 || w == 0x0b77) { // ac3
				s.ps1id = (b >= 0x80 && b < 0x88) ? (BYTE)(BitRead(32) >> 24) : 0x80;

				CMpegSplitterFile::ac3hdr h;
				if(!m_streams[audio].Find(s) && Read(h, len, &s.mt)) {
					type = audio;
				}
			} else if(b >= 0x88 && b < 0x90 || dw == 0x7ffe8001) { // dts
				s.ps1id = (b >= 0x88 && b < 0x90) ? (BYTE)(BitRead(32) >> 24) : 0x88;

				CMpegSplitterFile::dtshdr h;
				if(!m_streams[audio].Find(s) && Read(h, len, &s.mt)) {
					type = audio;
				}
			} else if(b >= 0xa0 && b < 0xa8) { // lpcm
				s.ps1id = (b >= 0xa0 && b < 0xa8) ? (BYTE)(BitRead(32) >> 24) : 0xa0;

				CMpegSplitterFile::lpcmhdr h;
				if(Read(h, &s.mt) && !m_streams[audio].Find(s)) { // note the reversed order, the header should be stripped always even if it's not a new stream
					type = audio;
				}
			} else if(b >= 0x20 && b < 0x40) { // DVD subpic
				s.ps1id = (BYTE)BitRead(8);

				CMpegSplitterFile::dvdspuhdr h;
				if(!m_streams[subpic].Find(s) && Read(h, &s.mt)) {
					type = subpic;
				}
			} else if(b >= 0x70 && b < 0x80) { // SVCD subpic
				s.ps1id = (BYTE)BitRead(8);

				CMpegSplitterFile::svcdspuhdr h;
				if(!m_streams[subpic].Find(s) && Read(h, &s.mt)) {
					type = subpic;
				}
			} else if(b >= 0x00 && b < 0x10) { // CVD subpic
				s.ps1id = (BYTE)BitRead(8);

				CMpegSplitterFile::cvdspuhdr h;
				if(!m_streams[subpic].Find(s) && Read(h, &s.mt)) {
					type = subpic;
				}
			} else if(w == 0xffa0 || w == 0xffa1) { // ps2-mpg audio
				s.ps1id = (BYTE)BitRead(8);
				s.pid = (WORD)((BitRead(8) << 8) | BitRead(16)); // pid = 0xa000 | track id

				CMpegSplitterFile::ps2audhdr h;
				if(!m_streams[audio].Find(s) && Read(h, &s.mt)) {
					type = audio;
				}
			} else if(w == 0xff90) { // ps2-mpg ac3 or subtitles
				s.ps1id = (BYTE)BitRead(8);
				s.pid = (WORD)((BitRead(8) << 8) | BitRead(16)); // pid = 0x9000 | track id

				w = (WORD)BitRead(16, true);

				if(w == 0x0b77) {
					CMpegSplitterFile::ac3hdr h;
					if(!m_streams[audio].Find(s) && Read(h, len, &s.mt)) {
						type = audio;
					}
				} else if(w == 0x0000) { // usually zero...
					CMpegSplitterFile::ps2subhdr h;
					if(!m_streams[subpic].Find(s) && Read(h, &s.mt)) {
						type = subpic;
					}
				}
			}
#if (EVO_SUPPORT != 0)
			else if(b >= 0xc0 && b < 0xc8) { // dolby digital +
				s.ps1id = (BYTE)BitRead(8);
				s.pid = (WORD)((BitRead(8) << 8) | BitRead(16)); // pid = 0x9000 | track id

				w = (WORD)BitRead(16, true);

				if(w == 0x0b77) {
					CMpegSplitterFile::ac3hdr h;
					if(!m_streams[audio].Find(s) && Read(h, len, &s.mt)) {
						type = audio;
					}
				}
			}
#endif
		}
	} else if(pesid == 0xbe) { // padding
	} else if(pesid == 0xbf) { // private stream 2
	}

	if(type != unknown && !m_streams[type].Find(s)) {
		if(s.pid) {
			for(int i = 0; i < unknown; i++) {
				if(m_streams[i].Find(s)) {
					/*ASSERT(0);*/
					return s;
				}
			}
		}

		m_streams[type].Insert(s, this);
	}

	return s;
}


void CMpegSplitterFile::AddHdmvPGStream(WORD pid, const char* language_code)
{
	stream s;

	s.pid		= pid;
	s.pesid		= 0xbd;

	CMpegSplitterFile::hdmvsubhdr h;
	if(!m_streams[subpic].Find(s) && Read(h, &s.mt, language_code)) {
		m_streams[subpic].Insert(s, this);
	}
}



CAtlList<CMpegSplitterFile::stream>* CMpegSplitterFile::GetMasterStream()
{
	return
		!m_streams[video].IsEmpty() ? &m_streams[video] :
		!m_streams[audio].IsEmpty() ? &m_streams[audio] :
		!m_streams[subpic].IsEmpty() ? &m_streams[subpic] :
#if defined(MVC_SUPPORT)
		!m_streams[stereo].IsEmpty() ? &m_streams[stereo] :
#endif
		NULL;
}

void CMpegSplitterFile::UpdatePrograms(const trhdr& h, bool UpdateLang)
{
	CAutoLock cAutoLock(&m_csProps);

	if(h.payload && h.payloadstart && h.pid == 0) {
		trsechdr h2;
		if(Read(h2) && h2.table_id == 0) {
			CAtlMap<WORD, bool> newprograms;

			int len = h2.section_length;
			len -= 5+4;

			for(int i = len/4; i > 0; i--) {
				WORD program_number = (WORD)BitRead(16);
				BYTE reserved = (BYTE)BitRead(3);
				WORD pid = (WORD)BitRead(13);
				UNUSED_ALWAYS(reserved);
				if(program_number != 0) {
					m_programs[pid].program_number = program_number;
					newprograms[program_number] = true;
				}
			}

			POSITION pos = m_programs.GetStartPosition();
			while(pos) {
				const CAtlMap<WORD, program>::CPair* pPair = m_programs.GetNext(pos);

				if(!newprograms.Lookup(pPair->m_value.program_number)) {
					m_programs.RemoveKey(pPair->m_key);
				}
			}
		}
	} else if(CAtlMap<WORD, program>::CPair* pPair = m_programs.Lookup(h.pid)) {
		if(h.payload && h.payloadstart) {
			trsechdr h2;
			if(Read(h2) && h2.table_id == 2) {
				int len = h2.section_length;
				len -= 5+4;

				BYTE buffer[1024];
				ByteRead(buffer, len);
				CGolombBuffer gb(buffer, len);

				int max_len = h.bytes - 9;
			
				if(len > max_len) {
					memset(pPair->m_value.ts_buffer, 0, sizeof(pPair->m_value.ts_buffer));
					pPair->m_value.ts_len_cur = max_len;
					pPair->m_value.ts_len_packet = len;
					memcpy(pPair->m_value.ts_buffer, buffer, max_len);
				} else {
					CGolombBuffer gb(buffer, len);
					UpdatePrograms(gb, h.pid, UpdateLang);
				}
			}
		} else {
			if(pPair->m_value.ts_len_cur > 0) {
				int len = pPair->m_value.ts_len_packet - pPair->m_value.ts_len_cur;
				if(len > h.bytes) {
					ByteRead(pPair->m_value.ts_buffer + pPair->m_value.ts_len_cur, h.bytes);
					pPair->m_value.ts_len_cur += h.bytes;
				} else {
					ByteRead(pPair->m_value.ts_buffer + pPair->m_value.ts_len_cur, pPair->m_value.ts_len_packet - pPair->m_value.ts_len_cur);
					CGolombBuffer gb(pPair->m_value.ts_buffer, pPair->m_value.ts_len_packet);
					UpdatePrograms(gb, h.pid, UpdateLang);
				}
			}
		}
	}
}

void CMpegSplitterFile::UpdatePrograms(CGolombBuffer gb, WORD pid, bool UpdateLang)
{
	if(CAtlMap<WORD, program>::CPair* pPair = m_programs.Lookup(pid))
	{
		memset(pPair->m_value.streams, 0, sizeof(pPair->m_value.streams));

		int len = gb.GetSize();

		BYTE reserved1 = (BYTE)gb.BitRead(3);
		WORD PCR_PID = (WORD)gb.BitRead(13);
		BYTE reserved2 = (BYTE)gb.BitRead(4);
		WORD program_info_length = (WORD)gb.BitRead(12);
		UNUSED_ALWAYS(reserved1);
		UNUSED_ALWAYS(PCR_PID);
		UNUSED_ALWAYS(reserved2);

		len -= (4 + program_info_length);
		if(len <= 0)
			return;

		while(program_info_length-- > 0) {
			gb.BitRead(8);
		}

		for(int i = 0; i < countof(pPair->m_value.streams) && len >= 5; i++) {
			BYTE stream_type = (BYTE)gb.BitRead(8);
			BYTE nreserved1 = (BYTE)gb.BitRead(3);
			WORD pid = (WORD)gb.BitRead(13);
			BYTE nreserved2 = (BYTE)gb.BitRead(4);
			WORD ES_info_length = (WORD)gb.BitRead(12);
			UNUSED_ALWAYS(nreserved1);
			UNUSED_ALWAYS(nreserved2);

			pPair->m_value.streams[i].pid	= pid;
			pPair->m_value.streams[i].type	= (PES_STREAM_TYPE)stream_type;

			if(m_ForcedSub) {
				if(stream_type == PRESENTATION_GRAPHICS_STREAM) {
					stream s;
					s.pid = pid;
					CMpegSplitterFile::hdmvsubhdr hdr;
					if(Read(hdr, &s.mt, NULL)) {
						if(!m_streams[subpic].Find(s)) {
							m_streams[subpic].Insert(s, this);
						}
					}
				}
			}

			len -= (5 + ES_info_length);
			if(len < 0)
				break;
			if(ES_info_length<=2)
				continue;

			if(UpdateLang) {
				int	info_length = ES_info_length;
				for(;;) {
					BYTE descriptor_tag = gb.BitRead(8);
					BYTE descriptor_length = gb.BitRead(8);
					info_length -= (2 + descriptor_length);
					if(info_length < 0)
						break;
					char ch[4];
					switch(descriptor_tag) {
						case 0x0a: // ISO 639 language descriptor
						case 0x56: // Teletext descriptor
						case 0x59: // Subtitling descriptor
							ch[0] = gb.BitRead(8);
							ch[1] = gb.BitRead(8);
							ch[2] = gb.BitRead(8);
							ch[3] = 0;
							for(int i = 3; i < descriptor_length; i++) {
								gb.BitRead(8);
							}
							if(!(ch[0] == 'u' && ch[1] == 'n' && ch[2] == 'd')) {
								m_pPMT_Lang[pid] = CString(ch);
							}
							break;
						default:
							for(int i = 0; i < descriptor_length; i++) {
								gb.BitRead(8);
							}
							break;
					}
					if(info_length<=2) break;
				}
			} else {
				while(ES_info_length-- > 0) {
					gb.BitRead(8);
				}
			}
		}
		pPair->m_value.ts_len_cur = 0;
		pPair->m_value.ts_len_packet = 0;
	}
}


uint32 SwapLE(const uint32 &_Value)
{
	return (_Value & 0xFF) << 24 | ((_Value>>8) & 0xFF) << 16 | ((_Value>>16) & 0xFF) << 8 | ((_Value>>24) & 0xFF) << 0;
}

uint16 SwapLE(const uint16 &_Value)
{
	return (_Value & 0xFF) << 8 | ((_Value>>8) & 0xFF) << 0;
}

const CMpegSplitterFile::program* CMpegSplitterFile::FindProgram(WORD pid, int &iStream, const CHdmvClipInfo::Stream * &_pClipInfo)
{
	_pClipInfo = m_ClipInfo.FindStream(pid);

	iStream = -1;

	POSITION pos = m_programs.GetStartPosition();

	while(pos) {
		program* p = &m_programs.GetNextValue(pos);

		for(int i = 0; i < countof(p->streams); i++) {
			if(p->streams[i].pid == pid) {
				iStream = i;
				return p;
			}
		}
	}

	return NULL;
}
