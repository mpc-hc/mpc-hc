/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
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
    return(*this);
}

POSITION CPlaylistItem::FindFile(LPCTSTR path)
{
    POSITION pos = m_fns.GetHeadPosition();
    while(pos)
    {
        if(m_fns.GetAt(pos).CompareNoCase(path) == 0)
            return pos;
        m_fns.GetNext(pos);
    }
    return(NULL);
}

static CString StripPath(CString path)
{
    CString p = path;
    p.Replace('\\', '/');
    p = p.Mid(p.ReverseFind('/') + 1);
    return(p.IsEmpty() ? path : p);
}

CString CPlaylistItem::GetLabel(int i)
{
    CString str;

    if(i == 0)
    {
        if(!m_label.IsEmpty()) str = m_label;
        else if(!m_fns.IsEmpty()) str = StripPath(m_fns.GetHead());
    }
    else if(i == 1)
    {
        if(m_fInvalid) return _T("Invalid");

        if(m_type == file)
        {
            REFERENCE_TIME rt = m_duration;

            if(rt > 0)
            {
                rt /= 10000000;
                int ss = int(rt % 60);
                rt /= 60;
                int mm = int(rt % 60);
                rt /= 60;
                int hh = int(rt);

                str.Format(_T("%02d:%02d:%02d"), hh, mm, ss);
            }
        }
        else if(m_type == device)
        {
            // TODO
        }

    }

    return str;
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
    if(pos)
    {
        __super::RemoveAt(pos);
        if(m_pos == pos)
        {
            m_pos = NULL;
            return(true);
        }
    }

    return(false);
}

typedef struct
{
    UINT n;
    POSITION pos;
} plsort_t;

static int compare(const void* arg1, const void* arg2)
{
    UINT a1 = ((plsort_t*)arg1)->n;
    UINT a2 = ((plsort_t*)arg2)->n;
    return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}

typedef struct
{
    LPCTSTR str;
    POSITION pos;
} plsort2_t;

int compare2(const void* arg1, const void* arg2)
{
    return _tcsicmp(((plsort2_t*)arg1)->str, ((plsort2_t*)arg2)->str);
}

void CPlaylist::SortById()
{
    CAtlArray<plsort_t> a;
    a.SetCount(GetCount());
    POSITION pos = GetHeadPosition();
    for(int i = 0; pos; i++, GetNext(pos))
        a[i].n = GetAt(pos).m_id, a[i].pos = pos;
    qsort(a.GetData(), a.GetCount(), sizeof(plsort_t), compare);
    for(int i = 0; i < a.GetCount(); i++)
    {
        AddTail(GetAt(a[i].pos));
        __super::RemoveAt(a[i].pos);
        if(m_pos == a[i].pos) m_pos = GetTailPosition();
    }
}

void CPlaylist::SortByName()
{
    CAtlArray<plsort2_t> a;
    a.SetCount(GetCount());
    POSITION pos = GetHeadPosition();
    for(int i = 0; pos; i++, GetNext(pos))
    {
        CString& fn = GetAt(pos).m_fns.GetHead();
        a[i].str = (LPCTSTR)fn + max(fn.ReverseFind('/'), fn.ReverseFind('\\')) + 1;
        a[i].pos = pos;
    }
    qsort(a.GetData(), a.GetCount(), sizeof(plsort2_t), compare2);
    for(int i = 0; i < a.GetCount(); i++)
    {
        AddTail(GetAt(a[i].pos));
        __super::RemoveAt(a[i].pos);
        if(m_pos == a[i].pos) m_pos = GetTailPosition();
    }
}

void CPlaylist::SortByPath()
{
    CAtlArray<plsort2_t> a;
    a.SetCount(GetCount());
    POSITION pos = GetHeadPosition();
    for(int i = 0; pos; i++, GetNext(pos))
        a[i].str = GetAt(pos).m_fns.GetHead(), a[i].pos = pos;
    qsort(a.GetData(), a.GetCount(), sizeof(plsort2_t), compare2);
    for(int i = 0; i < a.GetCount(); i++)
    {
        AddTail(GetAt(a[i].pos));
        __super::RemoveAt(a[i].pos);
        if(m_pos == a[i].pos) m_pos = GetTailPosition();
    }
}

void CPlaylist::Randomize()
{
    CAtlArray<plsort_t> a;
    a.SetCount(GetCount());
    srand((unsigned int)time(NULL));
    POSITION pos = GetHeadPosition();
    for(int i = 0; pos; i++, GetNext(pos))
        a[i].n = rand(), a[i].pos = pos;
    qsort(a.GetData(), a.GetCount(), sizeof(plsort_t), compare);
    CList<CPlaylistItem> pl;
    for(int i = 0; i < a.GetCount(); i++)
    {
        AddTail(GetAt(a[i].pos));
        __super::RemoveAt(a[i].pos);
        if(m_pos == a[i].pos)
            m_pos = GetTailPosition();
    }
}

POSITION CPlaylist::GetPos()
{
    return(m_pos);
}

void CPlaylist::SetPos(POSITION pos)
{
    m_pos = pos;
}

#define Rand(a, b) rand()%(b-a+1)+a

POSITION CPlaylist::Shuffle()
{
    CAtlArray<plsort2_t> a;
    a.SetCount(GetCount());
    srand((unsigned)time(NULL));
    POSITION pos = GetHeadPosition();
    for(int i = 0; pos; i++, GetNext(pos))
        a[i].pos = pos;

    pos = GetPos();
    int rnd = Rand(0, a.GetCount() - 1);
    while(pos == a[rnd].pos) rnd = Rand(0, a.GetCount() - 1);

    return a[rnd].pos;
}

CPlaylistItem& CPlaylist::GetNextWrap(POSITION& pos)
{
    if(AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("ShufflePlaylistItems"), FALSE) && GetCount() > 2)
    {
        pos = Shuffle();
    }
    else
    {
        GetNext(pos);
        if(!pos) pos = GetHeadPosition();
    }

    return(GetAt(pos));
}

CPlaylistItem& CPlaylist::GetPrevWrap(POSITION& pos)
{
    GetPrev(pos);
    if(!pos) pos = GetTailPosition();
    return(GetAt(pos));
}
