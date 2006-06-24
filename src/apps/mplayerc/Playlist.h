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

#include <afxcoll.h>

class CPlaylistItem
{
	static UINT m_globalid;

public:
	UINT m_id;
	CString m_label;
	CAtlList<CString> m_fns;
	CAtlList<CString> m_subs;
	enum type_t {file, device} m_type;
	REFERENCE_TIME m_duration; 
	int m_vinput, m_vchannel;
	int m_ainput;
	long m_country;

	bool m_fInvalid;

public:
	CPlaylistItem();
	virtual ~CPlaylistItem();

	CPlaylistItem(const CPlaylistItem& pli);
	CPlaylistItem& operator = (const CPlaylistItem& pli);

	POSITION FindFile(CString path);

	CString GetLabel(int i = 0);
};

class CPlaylist : public CList<CPlaylistItem>
{
protected:
	POSITION m_pos;

public:
	CPlaylist();
	virtual ~CPlaylist();

	void RemoveAll();
	bool RemoveAt(POSITION pos);

	void SortById(), SortByName(), SortByPath(), Randomize();

	POSITION GetPos();
	void SetPos(POSITION pos);
	CPlaylistItem& GetNextWrap(POSITION& pos);
	CPlaylistItem& GetPrevWrap(POSITION& pos);
};
