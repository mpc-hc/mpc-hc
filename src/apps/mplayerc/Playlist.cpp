/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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

#include "stdafx.h"
#include "mplayerc.h"
#include "Playlist.h"
#include "SettingsDefines.h"

//
// CPlaylistItem
//

UINT CPlaylistItem::m_globalid  = 0;

CPlaylistItem::CPlaylistItem()
	: m_type(file)
	, m_fInvalid(false)
	, m_duration(0)
	, m_vinput(-1)
	, m_vchannel(-1)
	, m_ainput(-1)
	, m_country(0)
{
	m_id = m_globalid++;
}

CPlaylistItem::~CPlaylistItem()
{
}

CPlaylistItem::CPlaylistItem(const CPlaylistItem& pli)
{
	*this = pli;
}

CPlaylistItem& CPlaylistItem::operator = (const CPlaylistItem& pli)
{
	if(this != &pli) {
		m_id = pli.m_id;
		m_label = pli.m_label;
		m_fns.RemoveAll();
		m_fns.AddTailList(&pli.m_fns);
		m_subs.RemoveAll();
		m_subs.AddTailList(&pli.m_subs);
		m_type = pli.m_type;
		m_fInvalid = pli.m_fInvalid;
		m_duration = pli.m_duration;
		m_vinput = pli.m_vinput;
		m_vchannel = pli.m_vchannel;
		m_ainput = pli.m_ainput;
		m_country = pli.m_country;
	}
	return(*this);
}

POSITION CPlaylistItem::FindFile(LPCTSTR path)
{
	POSITION pos = m_fns.GetHeadPosition();
	while(pos) {
		if (m_fns.GetAt(pos).CompareNoCase(path) == 0) {
			return pos;
		}
		m_fns.GetNext(pos);
	}
	return(NULL);
}

static CString StripPath(CString path)
{
	CString p = path;
	p.Replace('\\', '/');
	p = p.Mid(p.ReverseFind('/')+1);
	return(p.IsEmpty() ? path : p);
}

CString CPlaylistItem::GetLabel(int i)
{
	CString str;

	if(i == 0) {
		if(!m_label.IsEmpty()) {
			str = m_label;
		} else if(!m_fns.IsEmpty()) {
			str = StripPath(m_fns.GetHead());
		}
	} else if(i == 1) {
		if(m_fInvalid) {
			return _T("Invalid");
		}

		if(m_type == file) {
			REFERENCE_TIME rt = m_duration;

			if(rt > 0) {
				rt /= 10000000;
				int ss = int(rt%60);
				rt /= 60;
				int mm = int(rt%60);
				rt /= 60;
				int hh = int(rt);

				str.Format(_T("%02d:%02d:%02d"), hh, mm, ss);
			}
		} else if(m_type == device) {
			// TODO
		}

	}

	return str;
}

bool FindFileInList(CAtlList<CString>& sl, CString fn)
{
	bool fFound = false;
	POSITION pos = sl.GetHeadPosition();
	while(pos && !fFound) {
		if(!sl.GetNext(pos).CompareNoCase(fn)) {
			fFound = true;
		}
	}
	return(fFound);
}

void CPlaylistItem::AutoLoadFiles()
{
	if(m_fns.IsEmpty()) {
		return;
	}

	CString fn = m_fns.GetHead();

	if(AfxGetAppSettings().fAutoloadAudio && fn.Find(_T("://")) < 0) {
		int i = fn.ReverseFind('.');
		if(i > 0) {
			CMediaFormats& mf = AfxGetAppSettings().m_Formats;

			CString ext = fn.Mid(i+1).MakeLower();

			if(!mf.FindExt(ext, true)) {
				CString path = fn;
				path.Replace('/', '\\');
				path = path.Left(path.ReverseFind('\\')+1);

				WIN32_FIND_DATA fd = {0};
				HANDLE hFind = FindFirstFile(fn.Left(i) + _T("*.*"), &fd);
				if(hFind != INVALID_HANDLE_VALUE) {
					do {
						if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
							continue;
						}

						CString fullpath = path + fd.cFileName;
						CString ext2 = fullpath.Mid(fullpath.ReverseFind('.')+1).MakeLower();
						if(!FindFileInList(m_fns, fullpath) && ext != ext2
								&& mf.FindExt(ext2, true) && mf.IsUsingEngine(fullpath, DirectShow)) {
							m_fns.AddTail(fullpath);
						}
					} while(FindNextFile(hFind, &fd));

					FindClose(hFind);
				}
			}
		}
	}

	if(AfxGetAppSettings().fAutoloadSubtitles) {
		CString& pathList = AfxGetAppSettings().strSubtitlePaths;

		CAtlArray<CString> paths;

		int pos = 0;
		do {
			CString path = pathList.Tokenize(_T(";"), pos);
			paths.Add(path);
		} while(pos != -1);

		CString dir = fn;
		dir.Replace('\\', '/');
		int	l = fn.GetLength(), l2 = l;
		l2 = dir.ReverseFind('.');
		l = dir.ReverseFind('/') + 1;
		if(l2 < l) {
			l2 = l;
		}
		CString title = dir.Mid(l, l2-l);
		paths.Add(title.GetString());

		CAtlArray<SubFile> ret;
		GetSubFileNames(fn, paths, ret);

		for(int i = 0; i < ret.GetCount(); i++) {
			if(!FindFileInList(m_subs, ret[i].fn)) {
				m_subs.AddTail(ret[i].fn);
			}
		}
	}
}

//
// CPlaylist
//

CPlaylist::CPlaylist()
	: m_pos(NULL)
{
}

CPlaylist::~CPlaylist()
{
}

void CPlaylist::RemoveAll()
{
	__super::RemoveAll();
	m_pos = NULL;
}

bool CPlaylist::RemoveAt(POSITION pos)
{
	if(pos) {
		__super::RemoveAt(pos);
		if(m_pos == pos) {
			m_pos = NULL;
			return(true);
		}
	}

	return(false);
}

typedef struct {
	UINT n;
	POSITION pos;
} plsort_t;

static int compare(const void* arg1, const void* arg2)
{
	UINT a1 = ((plsort_t*)arg1)->n;
	UINT a2 = ((plsort_t*)arg2)->n;
	return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}

typedef struct {
	LPCTSTR str;
	POSITION pos;
} plsort2_t;

int compare2(const void* arg1, const void* arg2)
{
	return StrCmpLogicalW(((plsort2_t*)arg1)->str, ((plsort2_t*)arg2)->str);
}

void CPlaylist::SortById()
{
	CAtlArray<plsort_t> a;
	a.SetCount(GetCount());
	POSITION pos = GetHeadPosition();
	for(int i = 0; pos; i++, GetNext(pos)) {
		a[i].n = GetAt(pos).m_id, a[i].pos = pos;
	}
	qsort(a.GetData(), a.GetCount(), sizeof(plsort_t), compare);
	for(int i = 0; i < a.GetCount(); i++) {
		AddTail(GetAt(a[i].pos));
		__super::RemoveAt(a[i].pos);
		if(m_pos == a[i].pos) {
			m_pos = GetTailPosition();
		}
	}
}

void CPlaylist::SortByName()
{
	CAtlArray<plsort2_t> a;
	a.SetCount(GetCount());
	POSITION pos = GetHeadPosition();
	for(int i = 0; pos; i++, GetNext(pos)) {
		CString& fn = GetAt(pos).m_fns.GetHead();
		a[i].str = (LPCTSTR)fn + max(fn.ReverseFind('/'), fn.ReverseFind('\\')) + 1;
		a[i].pos = pos;
	}
	qsort(a.GetData(), a.GetCount(), sizeof(plsort2_t), compare2);
	for(int i = 0; i < a.GetCount(); i++) {
		AddTail(GetAt(a[i].pos));
		__super::RemoveAt(a[i].pos);
		if(m_pos == a[i].pos) {
			m_pos = GetTailPosition();
		}
	}
}

void CPlaylist::SortByPath()
{
	CAtlArray<plsort2_t> a;
	a.SetCount(GetCount());
	POSITION pos = GetHeadPosition();
	for(int i = 0; pos; i++, GetNext(pos)) {
		a[i].str = GetAt(pos).m_fns.GetHead(), a[i].pos = pos;
	}
	qsort(a.GetData(), a.GetCount(), sizeof(plsort2_t), compare2);
	for(int i = 0; i < a.GetCount(); i++) {
		AddTail(GetAt(a[i].pos));
		__super::RemoveAt(a[i].pos);
		if(m_pos == a[i].pos) {
			m_pos = GetTailPosition();
		}
	}
}

void CPlaylist::Randomize()
{
	CAtlArray<plsort_t> a;
	a.SetCount(GetCount());
	srand((unsigned int)time(NULL));
	POSITION pos = GetHeadPosition();
	for(int i = 0; pos; i++, GetNext(pos)) {
		a[i].n = rand(), a[i].pos = pos;
	}
	qsort(a.GetData(), a.GetCount(), sizeof(plsort_t), compare);
	CList<CPlaylistItem> pl;
	for(int i = 0; i < a.GetCount(); i++) {
		AddTail(GetAt(a[i].pos));
		__super::RemoveAt(a[i].pos);
		if(m_pos == a[i].pos) {
			m_pos = GetTailPosition();
		}
	}
}

POSITION CPlaylist::GetPos() const
{
	return(m_pos);
}

void CPlaylist::SetPos(POSITION pos)
{
	m_pos = pos;
}

POSITION CPlaylist::Shuffle()
{
	static INT_PTR idx = 0;
	static INT_PTR count = 0;
	static CAtlArray<plsort_t> a;

	ASSERT(GetCount() > 2);
	// insert or remove items in playlist, or index out of bounds then recalculate
	if((count != GetCount()) || (idx >= GetCount())) {
		a.RemoveAll();
		idx = 0;
		a.SetCount(count = GetCount());

		POSITION pos = GetHeadPosition();
		for(INT_PTR i = 0; pos; i++, GetNext(pos)) {
			a[i].pos = pos;    // initialize position array
		}

		//Use Fisher-Yates shuffle algorithm
		srand((unsigned)time(NULL));
		for (INT_PTR i=0; i<(count-1); i++) {
			INT_PTR r = i + (rand() % (count-i));
			POSITION temp = a[i].pos;
			a[i].pos = a[r].pos;
			a[r].pos = temp;
		}
	}

	return a[idx++].pos;
}

CPlaylistItem& CPlaylist::GetNextWrap(POSITION& pos)
{
	if(AfxGetAppSettings().bShufflePlaylistItems && GetCount() > 2) {
		pos = Shuffle();
	} else {
		GetNext(pos);
		if(!pos) {
			pos = GetHeadPosition();
		}
	}

	return(GetAt(pos));
}

CPlaylistItem& CPlaylist::GetPrevWrap(POSITION& pos)
{
	GetPrev(pos);
	if(!pos) {
		pos = GetTailPosition();
	}
	return(GetAt(pos));
}
