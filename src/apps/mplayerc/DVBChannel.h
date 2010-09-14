/*
 * $Id$
 *
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#pragma once

#define FORMAT_VERSION_0			0
#define FORMAT_VERSION_CURRENT		1

#define DVB_MAX_AUDIO		10
#define DVB_MAX_SUBTITLE	10

typedef enum
{
	DVB_MPV		= 0x00,
	DVB_H264	= 0x01,
	DVB_MPA		= 0x02,
	DVB_AC3		= 0x03,
	DVB_EAC3	= 0x04,
	DVB_PSI		= 0x80,
	DVB_TIF		= 0x81,
	DVB_EPG		= 0x82,
	DVB_PMT		= 0x83,
	DVB_SUB		= 0x83,
	DVB_SUBTITLE= 0xFE,
	DVB_UNKNOWN	= 0xFF
} DVB_STREAM_TYPE;


typedef struct
{
	ULONG				PID;
	DVB_STREAM_TYPE		Type;
	PES_STREAM_TYPE		PesType;
	CString				Language;

	LCID	GetLCID()
	{
		return ISO6392ToLcid(CStringA(Language));
	};
} DVBStreamInfo;

class CDVBChannel
{
public:
	CDVBChannel(void);
	~CDVBChannel(void);

	void			FromString(CString strValue);
	CString			ToString();

	LPCTSTR			GetName()					const
	{
		return m_strName;
	};
	ULONG			GetFrequency()				const
	{
		return m_ulFrequency;
	};
	int				GetPrefNumber()				const
	{
		return m_nPrefNumber;
	};
	int				GetOriginNumber()			const
	{
		return m_nOriginNumber;
	};
	ULONG			GetONID()					const
	{
		return m_ulONID;
	};
	ULONG			GetTSID()					const
	{
		return m_ulTSID;
	};
	ULONG			GetSID()					const
	{
		return m_ulSID;
	};
	ULONG			GetPMT()					const
	{
		return m_ulPMT;
	};
	ULONG			GetPCR()					const
	{
		return m_ulPCR;
	};
	ULONG			GetVideoPID()				const
	{
		return m_ulVideoPID;
	};
	DVB_STREAM_TYPE	GetVideoType()				const
	{
		return m_nVideoType;
	}
	ULONG			GetDefaultAudioPID()		const
	{
		return m_Audios[0].PID; /* TODO : fa*/
	};
	DVB_STREAM_TYPE	GetDefaultAudioType()		const
	{
		return m_Audios[0].Type;
	}
	ULONG			GetDefaultSubtitlePID()		const
	{
		return m_Subtitles[0].PID; /* TODO : fa*/
	};
	int				GetAudioCount()				const
	{
		return m_nAudioCount;
	};
	int				GetSubtitleCount()			const
	{
		return m_nSubtitleCount;
	};
	DVBStreamInfo*	GetAudio(int nIndex)
	{
		return &m_Audios[nIndex];
	};
	DVBStreamInfo*	GetSubtitle(int nIndex)
	{
		return &m_Subtitles[nIndex];
	};
	bool			HasName()
	{
		return !m_strName.IsEmpty();
	};
	bool			IsEncrypted()
	{
		return m_bEncrypted;
	};

	void			SetName(BYTE* Value);
	void			SetName(LPCTSTR Value)
	{
		m_strName = Value;
	};
	void			SetFrequency(ULONG Value)
	{
		m_ulFrequency = Value;
	};
	void			SetPrefNumber(int Value)
	{
		m_nPrefNumber = Value;
	};
	void			SetOriginNumber(int Value)
	{
		m_nOriginNumber = m_nPrefNumber = Value;
	};
	void			SetEncrypted(bool Value)
	{
		m_bEncrypted = Value;
	};
	void			SetONID(ULONG Value)
	{
		m_ulONID = Value;
	};
	void			SetTSID(ULONG Value)
	{
		m_ulTSID = Value;
	};
	void			SetSID(ULONG Value)
	{
		m_ulSID = Value;
	};
	void			SetPMT(ULONG Value)
	{
		m_ulPMT = Value;
	};
	void			SetPCR(ULONG Value)
	{
		m_ulPCR = Value;
	};
	void			SetVideoPID(ULONG Value)
	{
		m_ulVideoPID = Value;
	};

	void			AddStreamInfo (ULONG ulPID, DVB_STREAM_TYPE nType, PES_STREAM_TYPE nPesType, LPCTSTR strLanguage);

private :
	CString			m_strName;
	ULONG			m_ulFrequency;
	int				m_nPrefNumber;
	int				m_nOriginNumber;
	bool			m_bEncrypted;
	ULONG			m_ulONID;
	ULONG			m_ulTSID;
	ULONG			m_ulSID;
	ULONG			m_ulPMT;
	ULONG			m_ulPCR;
	ULONG			m_ulVideoPID;
	DVB_STREAM_TYPE	m_nVideoType;
	int				m_nAudioCount;
	int				m_nSubtitleCount;
	DVBStreamInfo	m_Audios[DVB_MAX_AUDIO];
	DVBStreamInfo	m_Subtitles[DVB_MAX_SUBTITLE];
};
