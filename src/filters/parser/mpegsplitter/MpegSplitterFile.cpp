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

#include "StdAfx.h"
#include <mmreg.h>
#include "MpegSplitterFile.h"

#include <initguid.h>
#include <moreuuids.h>

#define MEGABYTE 1024*1024
#define ISVALIDPID(pid) (pid >= 0x10 && pid < 0x1fff)

CMpegSplitterFile::CMpegSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr)
	: CBaseSplitterFileEx(pAsyncReader, hr, DEFAULT_CACHE_LENGTH, false)
	, m_type(us)
	, m_rate(0)
{
	if(SUCCEEDED(hr)) hr = Init();
}

HRESULT CMpegSplitterFile::Init()
{
	HRESULT hr;

	// get the type first

	m_type = us;

	Seek(0);

	if(m_type == us)
	{
		if(BitRead(32, true) == 'TFrc') Seek(0x67c);
		int cnt = 0, limit = 4;
		for(trhdr h; cnt < limit && Read(h); cnt++) Seek(h.next);
		if(cnt >= limit) m_type = ts;
	}

	Seek(0);

	if(m_type == us)
	{
		int cnt = 0, limit = 4;
		for(pvahdr h; cnt < limit && Read(h); cnt++) Seek(GetPos() + h.length);
		if(cnt >= limit) m_type = pva;
	}

	Seek(0);

	if(m_type == us)
	{
		BYTE b;
		for(int i = 0; (i < 4 || GetPos() < 65536) && m_type == us && NextMpegStartCode(b); i++)
		{
			if(b == 0xba)
			{
				pshdr h;
				if(Read(h)) 
				{
					m_type = ps;
					m_rate = h.bitrate/8;
					break;
				}
			}
			else if((b&0xe0) == 0xc0 // audio, 110xxxxx, mpeg1/2/3
				|| (b&0xf0) == 0xe0 // video, 1110xxxx, mpeg1/2
				// || (b&0xbd) == 0xbd) // private stream 1, 0xbd, ac3/dts/lpcm/subpic
				|| b == 0xbd) // private stream 1, 0xbd, ac3/dts/lpcm/subpic
			{
				peshdr h;
				if(Read(h, b) && BitRead(24, true) == 0x000001)
				{
					m_type = es;
				}
			}
		}
	}

	Seek(0);

	if(m_type == us)
	{
		return E_FAIL;
	}

	//

	// min/max pts & bitrate

	m_rtMin = m_posMin = _I64_MAX;
	m_rtMax = m_posMax = 0;

	if(IsRandomAccess() || IsStreaming())
	{
		if(IsStreaming())
		{
			for(int i = 0; i < 20 || i < 50 && S_OK != HasMoreData(1024*100, 100); i++);
		}

		CAtlList<__int64> fps;
		for(int i = 0, j = 5; i <= j; i++)
			fps.AddTail(i*GetLength()/j);

		for(__int64 pfp = 0; fps.GetCount(); )
		{
			__int64 fp = fps.RemoveHead();
			fp = min(GetLength() - MEGABYTE/8, fp);
			fp = max(pfp, fp);
			__int64 nfp = fp + (pfp == 0 ? 5*MEGABYTE : MEGABYTE/8);
			if(FAILED(hr = SearchStreams(fp, nfp)))
				return hr;
			pfp = nfp;
		}
	}
	else
	{
		if(FAILED(hr = SearchStreams(0, MEGABYTE/8)))
			return hr;
	}

	if(m_posMax - m_posMin <= 0 || m_rtMax - m_rtMin <= 0)
		return E_FAIL;

	int indicated_rate = m_rate;
	int detected_rate = 10000000i64 * (m_posMax - m_posMin) / (m_rtMax - m_rtMin);
	// normally "detected" should always be less than "indicated", but sometimes it can be a few percent higher (+10% is allowed here)
	// (update: also allowing +/-50k/s)
	if(indicated_rate == 0 || ((float)detected_rate / indicated_rate) < 1.1
	|| abs(detected_rate - indicated_rate) < 50*1024)
		m_rate = detected_rate;
	else ; // TODO: in this case disable seeking, or try doing something less drastical...

#ifndef DEBUG
	if(m_streams[video].GetCount() && m_streams[subpic].GetCount())
	{
		stream s;
		s.mt.majortype = MEDIATYPE_Video;
		s.mt.subtype = MEDIASUBTYPE_DVD_SUBPICTURE;
		s.mt.formattype = FORMAT_None;
		m_streams[subpic].Insert(s);
	}
#endif

	Seek(0);

	return S_OK;
}

void CMpegSplitterFile::OnComplete()
{
	__int64 pos = GetPos();

	if(SUCCEEDED(SearchStreams(GetLength() - 500*1024, GetLength())))
	{
		int indicated_rate = m_rate;
		int detected_rate = 10000000i64 * (m_posMax - m_posMin) / (m_rtMax - m_rtMin);
		// normally "detected" should always be less than "indicated", but sometimes it can be a few percent higher (+10% is allowed here)
		// (update: also allowing +/-50k/s)
		if(indicated_rate == 0 || ((float)detected_rate / indicated_rate) < 1.1
		|| abs(detected_rate - indicated_rate) < 50*1024)
			m_rate = detected_rate;
		else ; // TODO: in this case disable seeking, or try doing something less drastical...
	}

	Seek(pos);
}

REFERENCE_TIME CMpegSplitterFile::NextPTS(DWORD TrackNum)
{
	REFERENCE_TIME rt = -1;
	__int64 rtpos = -1;

	BYTE b;

	while(GetRemaining())
	{
		if(m_type == ps || m_type == es)
		{
			if(!NextMpegStartCode(b)) // continue;
				{ASSERT(0); break;}

			rtpos = GetPos()-4;

			if(b >= 0xbd && b < 0xf0)
			{
				peshdr h;
				if(!Read(h, b) || !h.len) continue;

				__int64 pos = GetPos();

				if(h.fpts && AddStream(0, b, h.len) == TrackNum)
				{
					ASSERT(h.pts >= m_rtMin && h.pts <= m_rtMax);
					rt = h.pts;
					break;
				}

				Seek(pos + h.len);
			}
		}
		else if(m_type == ts)
		{
			trhdr h;
			if(!Read(h)) continue;

			rtpos = GetPos()-4;

			if(h.payload && h.payloadstart && ISVALIDPID(h.pid))
			{
				peshdr h2;
				if(NextMpegStartCode(b, 4) && Read(h2, b)) // pes packet
				{
					if(h2.fpts && AddStream(h.pid, b, h.bytes - (GetPos() - rtpos)) == TrackNum)
					{
						ASSERT(h2.pts >= m_rtMin && h2.pts <= m_rtMax);
						rt = h2.pts;
						break;
					}
				}
			}

			Seek(h.next);
		}
		else if(m_type == pva)
		{
			pvahdr h;
			if(!Read(h)) continue;

			if(h.fpts)
			{
				rt = h.pts;
				break;
			}
		}
	}

	if(rtpos >= 0) Seek(rtpos);
	if(rt >= 0) rt -= m_rtMin;

	return rt;
}

HRESULT CMpegSplitterFile::SearchStreams(__int64 start, __int64 stop)
{
	Seek(start);
	stop = min(stop, GetLength());

	while(GetPos() < stop)
	{
		BYTE b;

		if(m_type == ps || m_type == es)
		{
			if(!NextMpegStartCode(b)) continue;

			if(b == 0xba) // program stream header
			{
				pshdr h;
				if(!Read(h)) continue;
			}
			else if(b == 0xbb) // program stream system header
			{
				pssyshdr h;
				if(!Read(h)) continue;
			}
			else if(b >= 0xbd && b < 0xf0) // pes packet
			{
				peshdr h;
				if(!Read(h, b)) continue;

				if(h.type == mpeg2 && h.scrambling) {ASSERT(0); return E_FAIL;}

				if(h.fpts)
				{
					if(m_rtMin == _I64_MAX) {m_rtMin = h.pts; m_posMin = GetPos();}
					if(m_rtMin < h.pts && m_rtMax < h.pts) {m_rtMax = h.pts; m_posMax = GetPos();}
/*
int rate = 10000000i64 * (m_posMax - m_posMin) / (m_rtMax - m_rtMin); 
if(m_rate == 0) m_rate = rate;
TRACE(_T("rate = %d (%d), (h.pts = %I64d)\n"), rate, rate - m_rate, h.pts);
m_rate = rate;
*/
				}

				__int64 pos = GetPos();
				AddStream(0, b, h.len);
				if(h.len) Seek(pos + h.len);
			}
		}
		else if(m_type == ts)
		{
			trhdr h;
			if(!Read(h)) continue;

			// if(h.scrambling) {ASSERT(0); return E_FAIL;}

			__int64 pos = GetPos();

			if(h.payload && h.payloadstart)
			{
				UpdatePrograms(h);
			}
				
			if(h.payload && ISVALIDPID(h.pid))
			{
				peshdr h2;
				if(h.payloadstart && NextMpegStartCode(b, 4) && Read(h2, b)) // pes packet
				{
					if(h2.type == mpeg2 && h2.scrambling) {ASSERT(0); return E_FAIL;}

					if(h2.fpts)
					{
						if(m_rtMin == _I64_MAX) {m_rtMin = h2.pts; m_posMin = GetPos();}
						if(m_rtMin < h2.pts && m_rtMax < h2.pts) {m_rtMax = h2.pts; m_posMax = GetPos();}
					}
				}
				else
				{
					b = 0;
				}

				AddStream(h.pid, b, h.bytes - (GetPos() - pos));
			}

			Seek(h.next);
		}
		else if(m_type == pva)
		{
			pvahdr h;
			if(!Read(h)) continue;

			if(h.fpts)
			{
				if(m_rtMin == _I64_MAX) {m_rtMin = h.pts; m_posMin = GetPos();}
				if(m_rtMin < h.pts && m_rtMax < h.pts) {m_rtMax = h.pts; m_posMax = GetPos();}
			}

			__int64 pos = GetPos();
			if(h.streamid == 1) AddStream(h.streamid, 0xe0, h.length);
			else if(h.streamid == 2) AddStream(h.streamid, 0xc0, h.length);
			if(h.length) Seek(pos + h.length);
		}
	}

	return S_OK;
}

DWORD CMpegSplitterFile::AddStream(WORD pid, BYTE pesid, DWORD len)
{
	if(pid)
	{
		if(pesid) m_pid2pes[pid] = pesid;
		else m_pid2pes.Lookup(pid, pesid);
	}

	stream s;
	s.pid = pid;
	s.pesid = pesid;

	int type = unknown;

	if(pesid >= 0xe0 && pesid < 0xf0) // mpeg video
	{
		__int64 pos = GetPos();

		if(type == unknown)
		{
			CMpegSplitterFile::seqhdr h;
			if(!m_streams[video].Find(s) && Read(h, len, &s.mt))
				type = video;
		}

		Seek(pos);

		if(type == unknown)
		{
			CMpegSplitterFile::avchdr h;
			if(!m_streams[video].Find(s) && Read(h, len, &s.mt))
				type = video;
		}
	}
	else if(pesid >= 0xc0 && pesid < 0xe0) // mpeg audio
	{
		__int64 pos = GetPos();

		if(type == unknown)
		{
			CMpegSplitterFile::mpahdr h;
			if(!m_streams[audio].Find(s) && Read(h, len, false, &s.mt))
				type = audio;
		}

		Seek(pos);

		if(type == unknown)
		{
			CMpegSplitterFile::aachdr h;
			if(!m_streams[audio].Find(s) && Read(h, len, &s.mt))
				type = audio;
		}
	}
	else if (pid == 0x1100) // AVCHD ac3 audio
	{
		if(!m_streams[audio].Find(s))
		{
			__int64 pos = GetPos();

			if(type == unknown)
			{
				CMpegSplitterFile::ac3hdr h;
				if(Read(h, len, &s.mt))
					type = audio;
			}

			Seek(pos);

			if(type == unknown)
			{
				CMpegSplitterFile::dtshdr h;
				if(Read(h, len, &s.mt))
					type = audio;
			}
		}
	}
	else if(pesid == 0xbd) // private stream 1
	{
		if(s.pid)
		{
			if(!m_streams[audio].Find(s))
			{
				__int64 pos = GetPos();

				if(type == unknown)
				{
					CMpegSplitterFile::ac3hdr h;
					if(Read(h, len, &s.mt))
						type = audio;
				}

				Seek(pos);

				if(type == unknown)
				{
					CMpegSplitterFile::dtshdr h;
					if(Read(h, len, &s.mt))
						type = audio;
				}
			}
		}
		else
		{
			BYTE b = (BYTE)BitRead(8, true);
			WORD w = (WORD)BitRead(16, true);
			DWORD dw = (DWORD)BitRead(32, true);

			if(b >= 0x80 && b < 0x88 || w == 0x0b77) // ac3
			{
				s.ps1id = (b >= 0x80 && b < 0x88) ? (BYTE)(BitRead(32) >> 24) : 0x80;
		
				CMpegSplitterFile::ac3hdr h;
				if(!m_streams[audio].Find(s) && Read(h, len, &s.mt))
					type = audio;
			}
			else if(b >= 0x88 && b < 0x90 || dw == 0x7ffe8001) // dts
			{
				s.ps1id = (b >= 0x88 && b < 0x90) ? (BYTE)(BitRead(32) >> 24) : 0x88;

				CMpegSplitterFile::dtshdr h;
				if(!m_streams[audio].Find(s) && Read(h, len, &s.mt))
					type = audio;
			}
			else if(b >= 0xa0 && b < 0xa8) // lpcm
			{
				s.ps1id = (b >= 0xa0 && b < 0xa8) ? (BYTE)(BitRead(32) >> 24) : 0xa0;
				
				CMpegSplitterFile::lpcmhdr h;
				if(Read(h, &s.mt) && !m_streams[audio].Find(s)) // note the reversed order, the header should be stripped always even if it's not a new stream
					type = audio;
			}
			else if(b >= 0x20 && b < 0x40) // DVD subpic
			{
				s.ps1id = (BYTE)BitRead(8);

				CMpegSplitterFile::dvdspuhdr h;
				if(!m_streams[subpic].Find(s) && Read(h, &s.mt))
					type = subpic;
			}
			else if(b >= 0x70 && b < 0x80) // SVCD subpic
			{
				s.ps1id = (BYTE)BitRead(8);

				CMpegSplitterFile::svcdspuhdr h;
				if(!m_streams[subpic].Find(s) && Read(h, &s.mt))
					type = subpic;
			}
			else if(b >= 0x00 && b < 0x10) // CVD subpic
			{
				s.ps1id = (BYTE)BitRead(8);

				CMpegSplitterFile::cvdspuhdr h;
				if(!m_streams[subpic].Find(s) && Read(h, &s.mt))
					type = subpic;
			}
			else if(w == 0xffa0 || w == 0xffa1) // ps2-mpg audio
			{
				s.ps1id = (BYTE)BitRead(8);
				s.pid = (WORD)((BitRead(8) << 8) | BitRead(16)); // pid = 0xa000 | track id

				CMpegSplitterFile::ps2audhdr h;
				if(!m_streams[audio].Find(s) && Read(h, &s.mt))
					type = audio;
			}
			else if(w == 0xff90) // ps2-mpg ac3 or subtitles
			{
				s.ps1id = (BYTE)BitRead(8);
				s.pid = (WORD)((BitRead(8) << 8) | BitRead(16)); // pid = 0x9000 | track id

				w = (WORD)BitRead(16, true);

				if(w == 0x0b77)
				{
					CMpegSplitterFile::ac3hdr h;
					if(!m_streams[audio].Find(s) && Read(h, len, &s.mt))
						type = audio;
				}
				else if(w == 0x0000) // usually zero...
				{
					CMpegSplitterFile::ps2subhdr h;
					if(!m_streams[subpic].Find(s) && Read(h, &s.mt))
						type = subpic;
				}
			}
		}
	}
	else if(pesid == 0xbe) // padding
	{
	}
	else if(pesid == 0xbf) // private stream 2
	{
	}

	if(type != unknown && !m_streams[type].Find(s))
	{
		if(s.pid)
		{
			for(int i = 0; i < unknown; i++)
			{
				if(m_streams[i].Find(s)) {/*ASSERT(0);*/ return s;}
			}
		}

		m_streams[type].Insert(s);
	}

	return s;
}

void CMpegSplitterFile::UpdatePrograms(const trhdr& h)
{
	CAutoLock cAutoLock(&m_csProps);

	if(h.pid == 0)
	{
		trsechdr h2;
		if(Read(h2) && h2.table_id == 0)
		{
			CAtlMap<WORD, bool> newprograms;

			int len = h2.section_length;
			len -= 5+4;
			for(int i = len/4; i > 0; i--)
			{
				WORD program_number = (WORD)BitRead(16);
				BYTE reserved = (BYTE)BitRead(3);
				WORD pid = (WORD)BitRead(13);
				if(program_number != 0)
				{
					m_programs[pid].program_number = program_number;
					newprograms[program_number] = true;
				}
			}

			POSITION pos = m_programs.GetStartPosition();
			while(pos)
			{
				const CAtlMap<WORD, program>::CPair* pPair = m_programs.GetNext(pos);

				if(!newprograms.Lookup(pPair->m_value.program_number))
				{
					m_programs.RemoveKey(pPair->m_key);
				}
			}
		}
	}
	else if(CAtlMap<WORD, program>::CPair* pPair = m_programs.Lookup(h.pid))
	{
		trsechdr h2;
		if(Read(h2) && h2.table_id == 2)
		{
			memset(pPair->m_value.pid, 0, sizeof(pPair->m_value.pid));

			int len = h2.section_length;
			len -= 5+4;
			BYTE reserved1 = (BYTE)BitRead(3);
			WORD PCR_PID = (WORD)BitRead(13);
			BYTE reserved2 = (BYTE)BitRead(4);
			WORD program_info_length = (WORD)BitRead(12);
			len -= 4+program_info_length;
			while(program_info_length-- > 0) BitRead(8);
			for(int i = 0; i < countof(pPair->m_value.pid) && len >= 5; i++)
			{
				BYTE stream_type = (BYTE)BitRead(8);
				BYTE reserved1 = (BYTE)BitRead(3);
				WORD pid = (WORD)BitRead(13);
				BYTE reserved2 = (BYTE)BitRead(4);
				WORD ES_info_length = (WORD)BitRead(12);
				len -= 5+ES_info_length;
				while(ES_info_length-- > 0) BitRead(8);
				pPair->m_value.pid[i] = pid;
			}
		}
	}
}

const CMpegSplitterFile::program* CMpegSplitterFile::FindProgram(WORD pid)
{
	POSITION pos = m_programs.GetStartPosition();
	while(pos)
	{
		const program* p = &m_programs.GetNextValue(pos);
		for(int i = 0; i < countof(p->pid); i++)
			if(p->pid[i] == pid) return p;
	}

	return NULL;
}
