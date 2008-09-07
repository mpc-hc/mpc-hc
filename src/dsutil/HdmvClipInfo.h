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

#pragma once


class CHdmvClipInfo
{
public:

	struct Stream
	{
		SHORT		stream_PID;
		BYTE		stream_coding_type;
		char		language_code[4];
		LCID		lcid;

		LPCTSTR Format();
	};

	CHdmvClipInfo(void);

	HRESULT		ReadInfo(LPCTSTR strFile);
	Stream*		FindStream(SHORT wPID);
	bool		IsHdmv()					{ return m_bIsHdmv; };
	int			GetStreamNumber()			{ return m_nStreamNumber; };
	Stream*		GetStreamByIndex(int nIndex){ return (nIndex < m_nStreamNumber) ? &m_Stream[nIndex] : NULL; };

	HRESULT		FindMainMovie(LPCTSTR strFolder, CAtlList<CString>& MainPlaylist);

private :
	DWORD		SequenceInfo_start_address;
	DWORD		ProgramInfo_start_address;

	HANDLE		m_hFile;
	int			m_nStreamNumber;
	Stream		m_Stream[50];
	bool		m_bIsHdmv;

	DWORD		ReadDword();
	SHORT		ReadShort();
	SHORT		ReadByte();
	void		ReadBuffer(BYTE* pBuff, int nLen);

	HRESULT		ReadProgramInfo();
	HRESULT		ReadPlaylist(LPCTSTR strPath, LPCTSTR strFile, REFERENCE_TIME& rtDuration, CAtlList<CString>& Playlist);
};
