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

#pragma once

#include <afxcoll.h>
#include "PlayerListCtrl.h"
#include "Playlist.h"


class OpenMediaData;

class CPlayerPlaylistBar : public CSizingControlBarG
{
    DECLARE_DYNAMIC(CPlayerPlaylistBar)

private:
    enum {COL_NAME, COL_TIME};

    CImageList m_fakeImageList;
    CPlayerListCtrl m_list;

    int m_nTimeColWidth;
    void ResizeListColumn();

    void AddItem(CString fn, CAtlList<CString>* subs);
    void AddItem(CAtlList<CString>& fns, CAtlList<CString>* subs);
    void ParsePlayList(CString fn, CAtlList<CString>* subs);
    void ParsePlayList(CAtlList<CString>& fns, CAtlList<CString>* subs);
    void ResolveLinkFiles( CAtlList<CString> &fns );

    bool ParseBDMVPlayList(CString fn);

    bool ParseMPCPlayList(CString fn);
    bool SaveMPCPlayList(CString fn, CTextFile::enc e, bool fRemovePath);

    void SetupList();
    void UpdateList();
    void EnsureVisible(POSITION pos);
    int FindItem(POSITION pos);
    POSITION FindPos(int i);

    CImageList* m_pDragImage;
    BOOL m_bDragging;
    int m_nDragIndex, m_nDropIndex;
    CPoint m_ptDropPoint;

    void DropItemOnList();

public:
    CPlayerPlaylistBar();
    virtual ~CPlayerPlaylistBar();

    BOOL Create(CWnd* pParentWnd);

    CPlaylist m_pl;

    int GetCount();
    int GetSelIdx();
    void SetSelIdx(int i);
    bool IsAtEnd();
    bool GetCur(CPlaylistItem& pli);
    CString GetCur();
    void SetNext();
    void SetPrev();
    void SetFirstSelected();
    void SetFirst();
    void SetLast();
    void SetCurValid(bool fValid);
    void SetCurTime(REFERENCE_TIME rt);

    void Refresh();
    void Empty();

    void Open(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs = NULL);
    void Append(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs = NULL);

    void Open(CStringW vdn, CStringW adn, int vinput, int vchannel, int ainput);
    void Append(CStringW vdn, CStringW adn, int vinput, int vchannel, int ainput);

    OpenMediaData* GetCurOMD(REFERENCE_TIME rtStart = 0);

    void LoadPlaylist(LPCTSTR filename);
    void SavePlaylist();

    bool SelectFileInPlaylist(LPCTSTR filename);

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLvnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnKeydownList(NMHDR* pNMHDR, LRESULT* pResult);
//	afx_msg void OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg BOOL OnFileClosePlaylist(UINT nID);
    afx_msg BOOL OnPlayPlay(UINT nID);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnLvnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
};
