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

#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include "..\BaseSplitter\BaseSplitter.h"

#define NO_SUBTITLE_PID			1		// Fake PID use for the "No subtitle" entry

enum ElementaryStreamTypes
{
    INVALID								= 0,
    VIDEO_STREAM_MPEG1					= 0x01,
    VIDEO_STREAM_MPEG2					= 0x02,
    AUDIO_STREAM_MPEG1					= 0x03, // all layers including mp3
    AUDIO_STREAM_MPEG2					= 0x04,
    VIDEO_STREAM_H264					= 0x1b,
    AUDIO_STREAM_LPCM					= 0x80,
    AUDIO_STREAM_AC3					= 0x81,
    AUDIO_STREAM_DTS					= 0x82,
    AUDIO_STREAM_AC3_TRUE_HD			= 0x83,
    AUDIO_STREAM_AC3_PLUS				= 0x84,
    AUDIO_STREAM_DTS_HD					= 0x85,
    AUDIO_STREAM_DTS_HD_MASTER_AUDIO	= 0x86,
    PRESENTATION_GRAPHICS_STREAM		= 0x90,
    INTERACTIVE_GRAPHICS_STREAM			= 0x91,
    SUBTITLE_STREAM						= 0x92,
    SECONDARY_AUDIO_AC3_PLUS			= 0xa1,
    SECONDARY_AUDIO_DTS_HD				= 0xa2,
    VIDEO_STREAM_VC1					= 0xea
};

enum BDVM_VideoFormat
{
    BDVM_VideoFormat_Unknown = 0,
    BDVM_VideoFormat_480i = 1,
    BDVM_VideoFormat_576i = 2,
    BDVM_VideoFormat_480p = 3,
    BDVM_VideoFormat_1080i = 4,
    BDVM_VideoFormat_720p = 5,
    BDVM_VideoFormat_1080p = 6,
    BDVM_VideoFormat_576p = 7,
};

enum BDVM_FrameRate
{
    BDVM_FrameRate_Unknown = 0,
    BDVM_FrameRate_23_976 = 1,
    BDVM_FrameRate_24 = 2,
    BDVM_FrameRate_25 = 3,
    BDVM_FrameRate_29_97 = 4,
    BDVM_FrameRate_50 = 6,
    BDVM_FrameRate_59_94 = 7
};

enum BDVM_AspectRatio
{
    BDVM_AspectRatio_Unknown = 0,
    BDVM_AspectRatio_4_3 = 2,
    BDVM_AspectRatio_16_9 = 3,
    BDVM_AspectRatio_2_21 = 4
};

enum BDVM_ChannelLayout
{
    BDVM_ChannelLayout_Unknown = 0,
    BDVM_ChannelLayout_MONO = 1,
    BDVM_ChannelLayout_STEREO = 3,
    BDVM_ChannelLayout_MULTI = 6,
    BDVM_ChannelLayout_COMBO = 12
};

enum BDVM_SampleRate : byte
{
    BDVM_SampleRate_Unknown = 0,
    BDVM_SampleRate_48 = 1,
    BDVM_SampleRate_96 = 4,
    BDVM_SampleRate_192 = 5,
    BDVM_SampleRate_48_192 = 12,
    BDVM_SampleRate_48_96 = 14
};

class CMpegSplitterFile : public CBaseSplitterFileEx
{
	CAtlMap<WORD, BYTE> m_pid2pes;
	CMpegSplitterFile::avchdr avch;
	bool m_bIsHdmv;


	HRESULT Init();

	void OnComplete();

public:
	const CString &m_FileName;
	CMpegSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr, bool bIsHdmv, const CString &_FileName);

	REFERENCE_TIME NextPTS(DWORD TrackNum);

	CCritSec m_csProps;

	enum {us, ps, ts, es, pva} m_type;

	REFERENCE_TIME m_rtMin, m_rtMax;
	__int64 m_posMin, m_posMax;
	int m_rate; // byte/sec

	struct stream
	{
		CMpegSplitterFile *m_pFile;
		CMediaType mt;
		WORD pid;
		BYTE pesid, ps1id;
		bool operator < (const stream &_Other) const;
		struct stream() {pid = pesid = ps1id = 0;}
		operator DWORD() const {return pid ? pid : ((pesid<<8)|ps1id);}
		bool operator == (const struct stream& s) const {return (DWORD)*this == (DWORD)s;}
	};

	enum {video, audio, subpic, unknown};

	class CStreamList : public CAtlList<stream>
	{
	public:
		void Insert(stream& s, CMpegSplitterFile *_pFile)
		{
			s.m_pFile = _pFile;
			for(POSITION pos = GetHeadPosition(); pos; GetNext(pos))
			{
				stream& s2 = GetAt(pos);
				if(s < s2) {InsertBefore(pos, s); return;}
			}

			AddTail(s);
		}

		static CStringW ToString(int type)
		{
			return 
				type == video ? L"Video" : 
				type == audio ? L"Audio" : 
				type == subpic ? L"Subtitle" : 
				L"Unknown";
		}

		const stream* FindStream(int pid)
		{
			for(POSITION pos = GetHeadPosition(); pos; GetNext(pos))
			{
				const stream& s = GetAt(pos);
				if(s.pid == pid) return &s;
			}

			return NULL;
		}

	} m_streams[unknown];

	HRESULT SearchStreams(__int64 start, __int64 stop);
	DWORD AddStream(WORD pid, BYTE pesid, DWORD len);
	void  AddHdmvPGStream(WORD pid, const char* language_code);
	CAtlList<stream>* GetMasterStream();

	struct program
	{
		WORD					program_number;
		struct stream
		{
			WORD					pid;
			ElementaryStreamTypes	type;

		};
		stream streams[64];
		struct program() {memset(this, 0, sizeof(*this));}
	};

	CAtlMap<WORD, program> m_programs;

	class CClipInfo
	{
	public:
		CClipInfo()
		{
			memset(this, 0, sizeof(*this));
		}
		WORD m_PID;
		ElementaryStreamTypes m_Type;
		char m_Language[4];
		// Valid for video types
		BDVM_VideoFormat		m_VideoFormat;
		BDVM_FrameRate			m_FrameRate;
		BDVM_AspectRatio		m_AspectRatio;
		// Valid for audio types
		BDVM_ChannelLayout		m_ChannelLayout;
		BDVM_SampleRate			m_SampleRate;
	};
	CAtlMap<WORD, CClipInfo> m_ClipInfo;
	CString m_LastClipInfoFile;
	void ReadClipInfo();

	void UpdatePrograms(const trhdr& h);
	const program* FindProgram(WORD pid, int &iStream, const CClipInfo * &_pClipInfo);

	
};
