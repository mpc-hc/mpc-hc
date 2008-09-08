/* 
 * $Id$
 *
 * (C) 2006-2007 see AUTHORS
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

#include "StdAfx.h"
#include "HdmvClipInfo.h"

extern LCID    ISO6392ToLcid(LPCSTR code);

CHdmvClipInfo::CHdmvClipInfo(void)
{
	m_nStreamNumber	= 0;
	m_hFile			= INVALID_HANDLE_VALUE;
	m_bIsHdmv		= false;
}


DWORD CHdmvClipInfo::ReadDword()
{
	return ReadByte()<<24 | ReadByte()<<16 | ReadByte()<<8 | ReadByte();
}

SHORT CHdmvClipInfo::ReadShort()
{
	return ReadByte()<<8 | ReadByte();
}

SHORT CHdmvClipInfo::ReadByte()
{
	BYTE	bVal;
	DWORD	dwRead;
	ReadFile (m_hFile, &bVal, sizeof(bVal), &dwRead, NULL);

	return bVal;
}

void CHdmvClipInfo::ReadBuffer(BYTE* pBuff, int nLen)
{
	DWORD	dwRead;
	ReadFile (m_hFile, pBuff, nLen, &dwRead, NULL);
}

HRESULT CHdmvClipInfo::ReadProgramInfo()
{
	BYTE		number_of_program_sequences;
	BYTE		number_of_streams_in_ps;
	DWORD		dwPos;

	m_nStreamNumber = 0;
	memset (&m_Stream, 0, sizeof(m_Stream));
	SetFilePointer (m_hFile, ProgramInfo_start_address, NULL, FILE_BEGIN);

	ReadDword();	//length
	ReadByte();		//reserved_for_word_align
	number_of_program_sequences		= (BYTE)ReadByte();
	for (int i=0; i<number_of_program_sequences; i++)
	{     
		ReadDword();	//SPN_program_sequence_start
		ReadShort();	//program_map_PID
		number_of_streams_in_ps = (BYTE)ReadByte();		//number_of_streams_in_ps
		ReadByte();		//reserved_for_future_use
	
		for (int stream_index=0; stream_index<number_of_streams_in_ps; stream_index++)
		{ 
			m_Stream[m_nStreamNumber].stream_PID			= ReadShort();	// stream_PID
			
			// == StreamCodingInfo
			dwPos  = SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT) + 1;
			dwPos += ReadByte();	// length
			m_Stream[m_nStreamNumber].stream_coding_type	= (BYTE)ReadByte();
			
			switch (m_Stream[m_nStreamNumber].stream_coding_type)
			{
			case 0x02 :	// Video streams
			case 0x1B :
			case 0xEA :
				break;
			case 0x80 :	// Audio streams
			case 0x81 :
			case 0x82 :
			case 0x83 :
			case 0x84 :
			case 0x85 :
			case 0x86 :
			case 0xA1 :
			case 0xA2 :
				ReadByte();
				ReadBuffer((BYTE*)m_Stream[m_nStreamNumber].language_code, 3);
				m_Stream[m_nStreamNumber].lcid = ISO6392ToLcid (m_Stream[m_nStreamNumber].language_code);
				break;
			case 0x90 :	// Presentation Graphics stream 
			case 0x91 :	// Interactive Graphics stream
				ReadBuffer((BYTE*)m_Stream[m_nStreamNumber].language_code, 3);
				m_Stream[m_nStreamNumber].lcid = ISO6392ToLcid (m_Stream[m_nStreamNumber].language_code);
				break;
			case 0x92 :	// Text subtitle stream 
				ReadByte();
				ReadBuffer((BYTE*)m_Stream[m_nStreamNumber].language_code, 3);
				m_Stream[m_nStreamNumber].lcid = ISO6392ToLcid (m_Stream[m_nStreamNumber].language_code);
				break;
			default :
				break;
			}

			m_nStreamNumber++;
			SetFilePointer(m_hFile, dwPos, NULL, FILE_BEGIN);
		}   
	}  
	return S_OK;
}


HRESULT CHdmvClipInfo::ReadInfo(LPCTSTR strFile)
{
	BYTE		Buff[100];

	m_bIsHdmv = false;
	m_hFile   = CreateFile(strFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
					       OPEN_EXISTING, FILE_ATTRIBUTE_READONLY|FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		ReadBuffer(Buff, 4);
		if (memcmp (Buff, "HDMV", 4)) return VFW_E_INVALID_FILE_FORMAT;

		ReadBuffer(Buff, 4);
		if ((memcmp (Buff, "0200", 4)!=0) && (memcmp (Buff, "0100", 4)!=0)) return VFW_E_INVALID_FILE_FORMAT;

		SequenceInfo_start_address	= ReadDword();
		ProgramInfo_start_address	= ReadDword();

		ReadProgramInfo();

		m_bIsHdmv = true;

		CloseHandle (m_hFile);
		return S_OK;
	}
	
	return AmHresultFromWin32(GetLastError());
}

CHdmvClipInfo::Stream* CHdmvClipInfo::FindStream(SHORT wPID)
{
	for (int i=0; i<m_nStreamNumber; i++)
	{
		if (m_Stream[i].stream_PID == wPID)
			return &m_Stream[i];
	}

	return NULL;
}

LPCTSTR CHdmvClipInfo::Stream::Format()
{
	switch (stream_coding_type)
	{
	case 0x02 :
		return _T("Mpeg2");
	case 0x1B :
		return _T("H264");
	case 0xEA :
		return _T("VC1");
	case 0x80 :
		return _T("LPCM");
	case 0x81 :
		return _T("AC3");
	case 0x82 :
		return _T("DTS");
	case 0x83 :
		return _T("MLP");
	case 0x84 :
		return _T("DD+");
	case 0x85 :
		return _T("DTS-HD");
	case 0x86 :
		return _T("DTS-HD XLL");
	case 0xA1 :
		return _T("DD+");
	case 0xA2 :
		return _T("DTS-HD");
	case 0x90 :
		return _T("PG");
	case 0x91 :
		return _T("IG");
	case 0x92 :
		return _T("Text");
	default :
		return _T("Unknown");
	}
}


HRESULT CHdmvClipInfo::ReadPlaylist(LPCTSTR strPath, LPCTSTR strFile, REFERENCE_TIME& rtDuration, CAtlList<CString>& Playlist)
{
	DWORD				dwPos;
	DWORD				dwTemp;
	SHORT				nPlaylistItems;
	REFERENCE_TIME		rtIn, rtOut;
	BYTE				Buff[100];
	CString				strTemp;

	strTemp.Format(_T("%sPLAYLIST\\%s"), strPath, strFile);
	m_hFile   = CreateFile(strTemp, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
					       OPEN_EXISTING, FILE_ATTRIBUTE_READONLY|FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		ReadBuffer(Buff, 4);
		if (memcmp (Buff, "MPLS", 4)) return VFW_E_INVALID_FILE_FORMAT;

		ReadBuffer(Buff, 4);
		if ((memcmp (Buff, "0200", 4)!=0) && (memcmp (Buff, "0100", 4)!=0)) return VFW_E_INVALID_FILE_FORMAT;

		dwPos = ReadDword();
		SetFilePointer(m_hFile, dwPos, NULL, FILE_BEGIN);

		ReadDword();
		ReadShort();
		dwTemp		   = ReadShort();
		nPlaylistItems = min((SHORT)dwTemp, 5);	// Main movie should be more than 5 parts...
		ReadShort();
		
		dwPos	  += 10;
		rtDuration = 0;
		for (int i=0; i<nPlaylistItems; i++)
		{		
			SetFilePointer(m_hFile, dwPos, NULL, FILE_BEGIN);
			dwPos = dwPos + ReadShort() + 2;
			ReadBuffer(Buff, 5);
			strTemp.Format(_T("%sSTREAM\\%c%c%c%c%c.M2TS"), strPath, Buff[0], Buff[1], Buff[2], Buff[3], Buff[4]);
			Playlist.AddTail (strTemp);

			ReadBuffer(Buff, 4);
			if (memcmp (Buff, "M2TS", 4)) return VFW_E_INVALID_FILE_FORMAT;
			ReadBuffer(Buff, 3);

			dwTemp	= ReadDword();
			rtIn = 20000i64*dwTemp/90;	// Carefull : 32->33 bits!

			dwTemp	= ReadDword();
			rtOut = 20000i64*dwTemp/90;	// Carefull : 32->33 bits!

			rtDuration += (rtOut - rtIn);

//			TRACE ("File : %S,  Duration : %S, Total duration  : %S\n", strName, ReftimeToString (rtOut - rtIn), ReftimeToString (rtDuration));
		}

		CloseHandle (m_hFile);
		return S_OK;
	}

	return AmHresultFromWin32(GetLastError());
}

HRESULT CHdmvClipInfo::FindMainMovie(LPCTSTR strFolder, CAtlList<CString>& MainPlaylist)
{
	HRESULT				hr		= E_FAIL;
	REFERENCE_TIME		rtMax	= 0;
	REFERENCE_TIME		rtCurrent;
	CString				strPath (strFolder);
	CString				strFilter;
	CAtlList<CString>	Playlist;
	WIN32_FIND_DATA		fd = {0};

	strPath.Replace(_T("\\PLAYLIST\\"), _T("\\"));
	strPath.Replace(_T("\\BDMV\\"),		_T("\\"));
	strPath.Replace(_T("\\STREAM\\"),		_T("\\"));
	strPath  += _T("\\BDMV\\");
	strFilter.Format (_T("%sPLAYLIST\\*.mpls"), strPath);

	HANDLE hFind = FindFirstFile(strFilter, &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			Playlist.RemoveAll();
			if (SUCCEEDED (ReadPlaylist(strPath, fd.cFileName, rtCurrent, Playlist)) && rtCurrent > rtMax)
			{
				rtMax			= rtCurrent;

				MainPlaylist.RemoveAll();
				POSITION pos = Playlist.GetHeadPosition();
				while(pos) MainPlaylist.AddTail(Playlist.GetNext(pos));

				hr				= S_OK;
			}
		}
		while(FindNextFile(hFind, &fd));
		
		FindClose(hFind);
	}

	return hr;
}
