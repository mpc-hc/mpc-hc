/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012, 2015, 2017 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <afxcoll.h>


class CPlaylistItem
{
    friend class CPlaylist;

    static UINT m_globalid;
    POSITION m_posNextShuffle;
    POSITION m_posPrevShuffle;

public:
    UINT m_id;
    CString m_label;
    bool m_bYoutubeDL;
    CString m_ydlSourceURL;
    CAtlList<CString> m_fns;
    CAtlList<CString> m_subs;
    enum type_t { file, device } m_type;
    REFERENCE_TIME m_duration;
    int m_vinput, m_vchannel;
    int m_ainput;
    long m_country;

    bool m_fInvalid;

    CPlaylistItem();
    virtual ~CPlaylistItem();

    CPlaylistItem(const CPlaylistItem& pli);
    CPlaylistItem& operator=(const CPlaylistItem& pli);

    POSITION FindFile(LPCTSTR path);
    void AutoLoadFiles();

    CString GetLabel(int i = 0);
};

class CPlaylist : protected CAtlList<CPlaylistItem>
{
protected:
    POSITION m_pos;
    bool m_bShuffle;
    POSITION m_posHeadShuffle;
    POSITION m_posTailShuffle;
    size_t m_nShuffledListSize;

    bool ReshuffleIfNeeded();

public:
    using CAtlList<CPlaylistItem>::AddHead;
    using CAtlList<CPlaylistItem>::AddTail;
    using CAtlList<CPlaylistItem>::InsertAfter;
    using CAtlList<CPlaylistItem>::InsertBefore;
    using CAtlList<CPlaylistItem>::GetHead;
    using CAtlList<CPlaylistItem>::GetTail;
    using CAtlList<CPlaylistItem>::GetHeadPosition;
    using CAtlList<CPlaylistItem>::GetTailPosition;
    using CAtlList<CPlaylistItem>::GetNext;
    using CAtlList<CPlaylistItem>::GetPrev;
    using CAtlList<CPlaylistItem>::GetAt;
    using CAtlList<CPlaylistItem>::GetCount;
    using CAtlList<CPlaylistItem>::IsEmpty;
    using CAtlList<CPlaylistItem>::MoveToHead;
    using CAtlList<CPlaylistItem>::MoveToTail;

    CPlaylist(bool bShuffle = false);
    virtual ~CPlaylist();

    bool RemoveAll();
    bool RemoveAt(POSITION pos);

    void SortById(), SortByName(), SortByPath(), Randomize();

    POSITION GetPos() const;
    void SetPos(POSITION pos);

    POSITION GetShuffleAwareHeadPosition();
    POSITION GetShuffleAwareTailPosition();

    CPlaylistItem& GetNextWrap(POSITION& pos);
    CPlaylistItem& GetPrevWrap(POSITION& pos);

    void SetShuffle(bool bEnable);
};
