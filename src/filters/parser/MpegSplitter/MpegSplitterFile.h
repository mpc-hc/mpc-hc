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

#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include "../BaseSplitter/BaseSplitter.h"
#include "../../../DSUtil/GolombBuffer.h"

#define NO_SUBTITLE_PID			1		// Fake PID use for the "No subtitle" entry
#define NO_SUBTITLE_NAME		_T("No subtitle")

#define ISVALIDPID(pid) (pid >= 0x10 && pid < 0x1fff)

//#define MVC_SUPPORT

class CMpegSplitterFile : public CBaseSplitterFileEx
{
	CAtlMap<WORD, BYTE> m_pid2pes;
	CAtlMap<WORD, CMpegSplitterFile::avchdr> avch;
	bool m_bIsHdmv;
	bool m_init;

	HRESULT Init(IAsyncReader* pAsyncReader);

	void OnComplete(IAsyncReader* pAsyncReader);

public:
	CHdmvClipInfo &m_ClipInfo;
	CMpegSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr, bool bIsHdmv, CHdmvClipInfo &ClipInfo, int guid_flag, bool ForcedSub);

	REFERENCE_TIME NextPTS(DWORD TrackNum);

	CCritSec m_csProps;

	enum {us, ps, ts, es, pva} m_type;

	REFERENCE_TIME m_rtMin, m_rtMax;
	__int64 m_posMin, m_posMax;
	int m_rate; // byte/sec

	int m_nVC1_GuidFlag;
	bool m_ForcedSub;

	struct stream {
		CMpegSplitterFile *m_pFile;
		CMediaType mt;
		WORD pid;
		BYTE pesid, ps1id;
		bool operator < (const stream &_Other) const;
		struct stream() {
			pid = pesid = ps1id = 0;
		}
		operator DWORD() const {
			return pid ? pid : ((pesid<<8)|ps1id);
		}
		bool operator == (const struct stream& s) const {
			return (DWORD)*this == (DWORD)s;
		}
	};

	enum {video, audio, subpic, 
#if defined(MVC_SUPPORT)
		stereo, 
#endif
		unknown};

	class CStreamList : public CAtlList<stream>
	{
	public:
		void Insert(stream& s, CMpegSplitterFile *_pFile) {
			s.m_pFile = _pFile;
			for(POSITION pos = GetHeadPosition(); pos; GetNext(pos)) {
				stream& s2 = GetAt(pos);
				if(s < s2) {
					InsertBefore(pos, s);
					return;
				}
			}

			AddTail(s);
		}

		void Replace(stream& source, stream& dest, CMpegSplitterFile *_pFile) {
			source.m_pFile = _pFile;
			dest.m_pFile = _pFile;
			for(POSITION pos = GetHeadPosition(); pos; GetNext(pos)) {
				stream& s = GetAt(pos);
				if(source == s) {
					SetAt(pos, dest);
					return;
				}
			}
		}

		static CStringW ToString(int type) {
			return
				type == video ? L"Video" :
				type == audio ? L"Audio" :
				type == subpic ? L"Subtitle" :
#if defined(MVC_SUPPORT)
				type == stereo ? L"Stereo" : 
#endif
				L"Unknown";
		}

		const stream* FindStream(int pid) {
			for(POSITION pos = GetHeadPosition(); pos; GetNext(pos)) {
				const stream& s = GetAt(pos);
				if(s.pid == pid) {
					return &s;
				}
			}

			return NULL;
		}

	} m_streams[unknown];

	HRESULT SearchStreams(__int64 start, __int64 stop, IAsyncReader* pAsyncReader);
	DWORD AddStream(WORD pid, BYTE pesid, DWORD len);
	void  AddHdmvPGStream(WORD pid, const char* language_code);
	CAtlList<stream>* GetMasterStream();
	bool IsHdmv() {
		return m_bIsHdmv;
	};

	struct program {
		WORD					program_number;
		struct stream {
			WORD				pid;
			PES_STREAM_TYPE		type;

		};
		stream streams[64];
		struct program() {
			memset(this, 0, sizeof(*this));
		}

		BYTE	ts_buffer[1024];
		WORD	ts_len_cur, ts_len_packet;
	};

	CAtlMap<WORD, program> m_programs;

	void UpdatePrograms(const trhdr& h, bool UpdateLang = true);
	void UpdatePrograms(CGolombBuffer gb, WORD pid, bool UpdateLang = true);
	const program* FindProgram(WORD pid, int &iStream, const CHdmvClipInfo::Stream * &_pClipInfo);

	CAtlMap<DWORD, CString> m_pPMT_Lang;
};
