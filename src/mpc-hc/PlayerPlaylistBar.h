/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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
#include "CMPCThemePlayerBar.h"
#include "CMPCThemePlayerListCtrl.h"
#include "Playlist.h"
#include "DropTarget.h"
#include "../Subtitles/TextFile.h"
#include "CMPCThemeInlineEdit.h"


class OpenMediaData;

class CMainFrame;

class CPlayerPlaylistBar : public CMPCThemePlayerBar, public CDropClient
{
    DECLARE_DYNAMIC(CPlayerPlaylistBar)

private:
    enum { COL_NAME, COL_TIME };

    CMainFrame* m_pMainFrame;
    CMPCThemeInlineEdit m_edit;

    CFont m_font;
    void ScaleFont();

    CImageList m_fakeImageList;
    CMPCThemePlayerListCtrl m_list;

    int m_itemHeight = 0;
    EventClient m_eventc;
    void EventCallback(MpcEvent ev);

    int m_nTimeColWidth;
    void ResizeListColumn();

    void AddItem(CString fn, CAtlList<CString>* subs);
    void AddItem(CAtlList<CString>& fns, CAtlList<CString>* subs, CString label = _T(""), CString ydl_src = _T(""));
    void ParsePlayList(CString fn, CAtlList<CString>* subs);
    void ParsePlayList(CAtlList<CString>& fns, CAtlList<CString>* subs, CString label = _T(""), CString ydl_src = _T(""));
    void ResolveLinkFiles(CAtlList<CString>& fns);

    bool ParseBDMVPlayList(CString fn);

    bool ParseMPCPlayList(CString fn);
    bool SaveMPCPlayList(CString fn, CTextFile::enc e, bool fRemovePath);

    void SetupList();
    void UpdateList();
    void EnsureVisible(POSITION pos);
    int FindItem(const POSITION pos) const;
    POSITION FindPos(int i);

    CImageList* m_pDragImage;
    BOOL m_bDragging;
    int m_nDragIndex, m_nDropIndex;
    CPoint m_ptDropPoint;

    void DropItemOnList();

    bool m_bHiddenDueToFullscreen;

    CDropTarget m_dropTarget;
    void OnDropFiles(CAtlList<CString>& slFiles, DROPEFFECT) override;
    DROPEFFECT OnDropAccept(COleDataObject*, DWORD, CPoint) override;

public:
    CPlayerPlaylistBar(CMainFrame* pMainFrame);
    virtual ~CPlayerPlaylistBar();

    BOOL Create(CWnd* pParentWnd, UINT defDockBarID);

    virtual void ReloadTranslatableResources();

    virtual void LoadState(CFrameWnd* pParent);
    virtual void SaveState();

    bool IsHiddenDueToFullscreen() const;
    void SetHiddenDueToFullscreen(bool bHidenDueToFullscreen);

    CPlaylist m_pl;

    INT_PTR GetCount() const;
    int GetSelIdx() const;
    void SetSelIdx(int i);
    bool IsAtEnd();
    bool GetCur(CPlaylistItem& pli) const;
    CPlaylistItem* GetCur();
    CString GetCurFileName();
    bool SetNext();
    bool SetPrev();
    void SetFirstSelected();
    void SetFirst();
    void SetLast();
    void SetCurValid(bool fValid);
    void SetCurTime(REFERENCE_TIME rt);
    void Randomize();

    void Refresh();
    bool Empty();

    void Open(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs = nullptr);
    void Append(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs = nullptr, CString label = _T(""), CString ydl_src = _T(""));

    void Open(CStringW vdn, CStringW adn, int vinput, int vchannel, int ainput);
    void Append(CStringW vdn, CStringW adn, int vinput, int vchannel, int ainput);

    OpenMediaData* GetCurOMD(REFERENCE_TIME rtStart = 0);

    void LoadPlaylist(LPCTSTR filename);
    void SavePlaylist();

    bool SelectFileInPlaylist(LPCTSTR filename);
    bool DeleteFileInPlaylist(POSITION pos, bool recycle = true);

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLvnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnKeydownList(NMHDR* pNMHDR, LRESULT* pResult);
    //  afx_msg void OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult);
    void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg BOOL OnPlayPlay(UINT nID);
    afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);
    afx_msg void OnLvnBeginlabeleditList(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnLvnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);
    afx_msg void OnXButtonUp(UINT nFlags, UINT nButton, CPoint point);
    afx_msg void OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point);
};
