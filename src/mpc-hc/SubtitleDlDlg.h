/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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

#include "resource.h"
#include "ResizableLib/ResizableDialog.h"
#include <vector>
#include <list>

class CMainFrame;
struct SubtitlesInfo;
class SubtitlesProvider;
typedef std::list<SubtitlesInfo> SubtitlesList;
enum SRESULT;

class CSubtitleDlDlg : public CResizableDialog
{
private:
    enum {
        COL_PROVIDER,
        COL_FILENAME,
        COL_LANGUAGE,
        //COL_FORMAT,
        COL_HEARINGIMPAIRED,
        COL_DOWNLOADS,
        COL_DISC,
        COL_TITLES,
#ifdef _DEBUG
        COL_SCORE,
#endif
        COL_TOTAL_COLUMNS,
    };

    struct PARAMSORT {
        PARAMSORT(HWND hWnd, int nSortColumn, int fSortOrder)
            : m_hWnd(hWnd), m_nSortColumn(nSortColumn), m_fSortOrder(fSortOrder) {}
        HWND m_hWnd;
        int m_nSortColumn;
        int m_fSortOrder;
    };
    typedef PARAMSORT* PPARAMSORT;

    PARAMSORT m_ps;
    bool m_fReplaceSubs;

    CListCtrl m_list;
    CProgressCtrl m_progress;
    CStatusBarCtrl m_status;
    CMainFrame& m_pMainFrame;
    SubtitlesList m_Subtitles;

    static int CALLBACK SortCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    void SetStatusText(const CString& status, BOOL bPropagate = TRUE);
    void SetListViewSortColumn();
    void DownloadSelectedSubtitles();

public:
    CSubtitleDlDlg(CWnd* pParentWnd);
    virtual ~CSubtitleDlDlg();
    enum { IDD = IDD_SUBTITLEDL_DLG };


protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnOK();


    DECLARE_MESSAGE_MAP()

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnFailedConnection();
    afx_msg void OnUpdateOk(CCmdUI* pCmdUI);
    afx_msg void OnUpdateRefresh(CCmdUI* pCmdUI);
    afx_msg void OnAbort();
    afx_msg void OnRefresh();
    afx_msg void OnOptions();
    afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnDoubleClickSubtitle(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKeyPressedSubtitle(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRightClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

    afx_msg LRESULT OnSearch(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSearching(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDownloading(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDownloaded(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCompleted(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFinished(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFailed(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnClear(WPARAM wParam, LPARAM lParam);

public:
    void DoSearch(INT _nCount);
    void DoSearching(SubtitlesInfo& _fileInfo);
    void DoDownloading(SubtitlesInfo& _fileInfo);
    void DoDownloaded(SubtitlesInfo& _fileInfo);
    void DoCompleted(SRESULT _result, SubtitlesList& _subtitlesList);
    void DoFinished(BOOL _bAborted, BOOL _bShowDialog);
    void DoFailed();
    void DoClear();
};
