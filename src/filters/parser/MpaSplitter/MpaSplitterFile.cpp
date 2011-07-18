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
#include "MpaSplitterFile.h"

#include <initguid.h>
#include <moreuuids.h>

//

static const LPCTSTR s_genre[] = {
	_T("Blues"), _T("Classic Rock"), _T("Country"), _T("Dance"),
	_T("Disco"), _T("Funk"), _T("Grunge"), _T("Hip-Hop"),
	_T("Jazz"), _T("Metal"), _T("New Age"), _T("Oldies"),
	_T("Other"), _T("Pop"), _T("R&B"), _T("Rap"),
	_T("Reggae"), _T("Rock"), _T("Techno"), _T("Industrial"),
	_T("Alternative"), _T("Ska"), _T("Death Metal"), _T("Pranks"),
	_T("Soundtrack"), _T("Euro-Techno"), _T("Ambient"), _T("Trip-Hop"),
	_T("Vocal"), _T("Jazz+Funk"), _T("Fusion"), _T("Trance"),
	_T("Classical"), _T("Instrumental"), _T("Acid"), _T("House"),
	_T("Game"), _T("Sound Clip"), _T("Gospel"), _T("Noise"),
	_T("Alternative Rock"), _T("Bass"), _T("Soul"), _T("Punk"),
	_T("Space"), _T("Meditative"), _T("Instrumental Pop"), _T("Instrumental Rock"),
	_T("Ethnic"), _T("Gothic"), _T("Darkwave"), _T("Techno-Industrial"),
	_T("Electronic"), _T("Pop-Folk"), _T("Eurodance"), _T("Dream"),
	_T("Southern Rock"), _T("Comedy"), _T("Cult"), _T("Gangsta"),
	_T("Top 40"), _T("Christian Rap"), _T("Pop/Funk"), _T("Jungle"),
	_T("Native US"), _T("Cabaret"), _T("New Wave"), _T("Psychadelic"),
	_T("Rave"), _T("Showtunes"), _T("Trailer"), _T("Lo-Fi"),
	_T("Tribal"), _T("Acid Punk"), _T("Acid Jazz"), _T("Polka"),
	_T("Retro"), _T("Musical"), _T("Rock & Roll"), _T("Hard Rock"),
	_T("Folk"), _T("Folk-Rock"), _T("National Folk"), _T("Swing"),
	_T("Fast Fusion"), _T("Bebob"), _T("Latin"), _T("Revival"),
	_T("Celtic"), _T("Bluegrass"), _T("Avantgarde"), _T("Gothic Rock"),
	_T("Progressive Rock"), _T("Psychedelic Rock"), _T("Symphonic Rock"), _T("Slow Rock"),
	_T("Big Band"), _T("Chorus"), _T("Easy Listening"), _T("Acoustic"),
	_T("Humour"), _T("Speech"), _T("Chanson"), _T("Opera"),
	_T("Chamber Music"), _T("Sonata"), _T("Symphony"), _T("Booty Bass"),
	_T("Primus"), _T("Porn Groove"), _T("Satire"), _T("Slow Jam"),
	_T("Club"), _T("Tango"), _T("Samba"), _T("Folklore"),
	_T("Ballad"), _T("Power Ballad"), _T("Rhytmic Soul"), _T("Freestyle"),
	_T("Duet"), _T("Punk Rock"), _T("Drum Solo"), _T("Acapella"),
	_T("Euro-House"), _T("Dance Hall"), _T("Goa"), _T("Drum & Bass"),
	_T("Club-House"), _T("Hardcore"), _T("Terror"), _T("Indie"),
	_T("BritPop"), _T("Negerpunk"), _T("Polsk Punk"), _T("Beat"),
	_T("Christian Gangsta"), _T("Heavy Metal"), _T("Black Metal"),
	_T("Crossover"), _T("Contemporary C"), _T("Christian Rock"), _T("Merengue"), _T("Salsa"),
	_T("Thrash Metal"), _T("Anime"), _T("JPop"), _T("SynthPop"),
};

//

CMpaSplitterFile::CMpaSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr)
	: CBaseSplitterFileEx(pAsyncReader, hr, DEFAULT_CACHE_LENGTH, false)
	, m_mode(none)
	, m_rtDuration(0)
	, m_startpos(0)
	, m_endpos(0)
	, m_totalbps(0)
{
	if(SUCCEEDED(hr)) {
		hr = Init();
	}
}

HRESULT CMpaSplitterFile::Init()
{
	m_startpos = 0;
	m_endpos = GetLength();

	Seek(0);

	if(BitRead(24, true) == 0x000001) {
		return E_FAIL;
	}

	if(m_endpos > 128 && IsRandomAccess()) {
		Seek(m_endpos - 128);

		if(BitRead(24) == 'TAG') {
			m_endpos -= 128;

			CStringA str;

			// title
			ByteRead((BYTE*)str.GetBufferSetLength(30), 30);
			m_tags['TIT2'] = CStringW(str).Trim();

			// artist
			ByteRead((BYTE*)str.GetBufferSetLength(30), 30);
			m_tags['TPE1'] = CStringW(str).Trim();

			// album
			ByteRead((BYTE*)str.GetBufferSetLength(30), 30);
			m_tags['TALB'] = CStringW(str).Trim();

			// year
			ByteRead((BYTE*)str.GetBufferSetLength(4), 4);
			m_tags['TYER'] = CStringW(str).Trim();

			// comment
			ByteRead((BYTE*)str.GetBufferSetLength(30), 30);
			m_tags['COMM'] = CStringW(str).Trim();

			// track
			LPCSTR s = str;
			if(s[28] == 0 && s[29] != 0) {
				m_tags['TRCK'].Format(L"%d", s[29]);
			}

			// genre
			BYTE genre = (BYTE)BitRead(8);
			if(genre < countof(s_genre)) {
				m_tags['TCON'] = CStringW(s_genre[genre]);
			}
		}
	}

	Seek(0);

	bool MP3_find = false;

	while(BitRead(24, true) == 'ID3') {
		MP3_find = true;

		BitRead(24);

		BYTE major = (BYTE)BitRead(8);
		BYTE revision = (BYTE)BitRead(8);
		UNUSED_ALWAYS(revision);

		BYTE flags = (BYTE)BitRead(8);
		UNUSED_ALWAYS(flags);
		DWORD size = 0;
		if(BitRead(1) != 0) {
			return E_FAIL;
		}
		size |= BitRead(7) << 21;
		if(BitRead(1) != 0) {
			return E_FAIL;
		}
		size |= BitRead(7) << 14;
		if(BitRead(1) != 0) {
			return E_FAIL;
		}
		size |= BitRead(7) << 7;
		if(BitRead(1) != 0) {
			return E_FAIL;
		}
		size |= BitRead(7);

		m_startpos = GetPos() + size;

		// TODO: read extended header

		if(major <= 4) {
			__int64 pos = GetPos();

			while(pos < m_startpos) {
				Seek(pos);

				DWORD tag = (DWORD)BitRead(32);
				DWORD size = 0;
				size |= BitRead(8) << 24;
				size |= BitRead(8) << 16;
				size |= BitRead(8) << 8;
				size |= BitRead(8);
				WORD flags = (WORD)BitRead(16);
				UNUSED_ALWAYS(flags);

				pos += 4+4+2+size;

				if(!size || pos >= m_startpos) {
					break;
				}

				if(tag == 'TIT2'
						|| tag == 'TPE1'
						|| tag == 'TALB'
						|| tag == 'TYER'
						|| tag == 'COMM'
						|| tag == 'TRCK') {
					BYTE encoding = (BYTE)BitRead(8);
					size--;

					WORD bom = (WORD)BitRead(16, true);

					CStringA str;
					CStringW wstr;

					if(encoding > 0 && size >= 2 && bom == 0xfffe) {
						BitRead(16);
						size = (size - 2) / 2;
						ByteRead((BYTE*)wstr.GetBufferSetLength(size), size*2);
						m_tags[tag] = wstr.Trim();
					} else if(encoding > 0 && size >= 2 && bom == 0xfeff) {
						BitRead(16);
						size = (size - 2) / 2;
						ByteRead((BYTE*)wstr.GetBufferSetLength(size), size*2);
						for(int i = 0, j = wstr.GetLength(); i < j; i++) {
							wstr.SetAt(i, (wstr[i]<<8)|(wstr[i]>>8));
						}
						m_tags[tag] = wstr.Trim();
					} else {
						ByteRead((BYTE*)str.GetBufferSetLength(size), size);
						m_tags[tag] = (encoding > 0 ? UTF8To16(str) : CStringW(str)).Trim();
					}
				}
			}
		}

		Seek(m_startpos);

		for(int i = 0; i < (1<<20) && m_startpos < m_endpos && BitRead(8, true) == 0; i++) {
			BitRead(8), m_startpos++;
		}
	}

	__int64 searchlen;
	__int64 startpos;
	__int64 syncpos;

	
	if(!MP3_find) { // if no 'ID3' tag at begin of file
		Seek(m_startpos);
		searchlen = min(m_endpos - m_startpos, 0x200);
		if(Read(m_mpahdr, searchlen, true, &m_mt)) {
			MP3_find = true;
		}
	}

	if(MP3_find) {
		__int64 startpos_mp3 = m_startpos;
		while (m_mode == none) {
			searchlen = min(m_endpos - startpos_mp3, 0x200);
			Seek(startpos_mp3);

			// If we fail to see sync bytes, we reposition here and search again
			syncpos = startpos_mp3 + searchlen;

			// Check for a valid MPA header
			if(Read(m_mpahdr, searchlen, true, &m_mt)) {
				m_mode = mpa;

				syncpos = GetPos();
				startpos = syncpos - 4;

				// make sure the first frame is followed by another of the same kind (validates m_mpahdr basically)
				Seek(startpos + m_mpahdr.FrameSize);
				if(!Sync(4)) {
					m_mode = none;
				} else {
					break;
				}
			}

			// If we have enough room to search for a valid header, then skip ahead and try again
			if (m_endpos - syncpos >= 8) {
				startpos_mp3 = syncpos;
			} else {
				break;
			}
		}
	}

	searchlen = min(m_endpos - m_startpos, m_startpos > 0 ? 0x200 : 7);
	Seek(m_startpos);

	if(m_mode == none && Read(m_aachdr, searchlen, &m_mt)) {
		m_mode = mp4a;

		startpos = GetPos() - (m_aachdr.fcrc?7:9);

		// make sure the first frame is followed by another of the same kind (validates m_aachdr basically)
		Seek(startpos + m_aachdr.aac_frame_length);
		if(!Sync(9)) {
			m_mode = none;
		}
	}

	if(m_mode == none) {
		return E_FAIL;
	}

	m_startpos = startpos;

	int FrameSize;
	REFERENCE_TIME rtFrameDur, rtPrevDur = -1;
	clock_t start = clock();
	int i = 0;
	while(Sync(FrameSize, rtFrameDur) && (clock() - start) < CLOCKS_PER_SEC) {
		TRACE(_T("%I64d\n"), m_rtDuration);
		Seek(GetPos() + FrameSize);
		i = rtPrevDur == m_rtDuration ? i+1 : 0;
		if(i == 10) {
			break;
		}
		rtPrevDur = m_rtDuration;
	}

	return S_OK;
}

bool CMpaSplitterFile::Sync(int limit)
{
	int FrameSize;
	REFERENCE_TIME rtDuration;
	return Sync(FrameSize, rtDuration, limit);
}

bool CMpaSplitterFile::Sync(int& FrameSize, REFERENCE_TIME& rtDuration, int limit)
{
	__int64 endpos = min(m_endpos, GetPos() + limit);

	if(m_mode == mpa) {
		while(GetPos() <= endpos - 4) {
			mpahdr h;

			if(Read(h, endpos - GetPos(), true)) {
				if (m_mpahdr.version == h.version
						&& m_mpahdr.layer == h.layer
						&& m_mpahdr.channels == h.channels) {
					Seek(GetPos() - 4);
					AdjustDuration(h.nBytesPerSec);

					FrameSize = h.FrameSize;
					rtDuration = h.rtDuration;

					return true;
				}
			} else {
				break;
			}
		}
	} else if(m_mode == mp4a) {
		while(GetPos() <= endpos - 9) {
			aachdr h;

			if(Read(h, endpos - GetPos())) {
				if (m_aachdr.version == h.version
						&& m_aachdr.layer == h.layer
						&& m_aachdr.channels == h.channels) {
					Seek(GetPos() - (h.fcrc?7:9));
					AdjustDuration(h.nBytesPerSec);
					Seek(GetPos() + (h.fcrc?7:9));

					FrameSize = h.FrameSize;
					rtDuration = h.rtDuration;

					return true;
				}
			} else {
				break;
			}
		}
	}

	return false;
}

void CMpaSplitterFile::AdjustDuration(int nBytesPerSec)
{
	ASSERT(nBytesPerSec);

	int rValue;
	if(!m_pos2bps.Lookup(GetPos(), rValue)) {
		m_totalbps += nBytesPerSec;
		if(!m_totalbps) {
			return;
		}
		m_pos2bps.SetAt(GetPos(), nBytesPerSec);
		__int64 avgbps = m_totalbps / m_pos2bps.GetCount();
		m_rtDuration = 10000000i64 * (m_endpos - m_startpos) / avgbps;
	}
}
