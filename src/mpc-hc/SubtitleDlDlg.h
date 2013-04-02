/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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

#include "ISDb.h"
#include "resource.h"
#include "ResizableLib/ResizableDialog.h"

class CInternetSession;

class CSubtitleDlDlg : public CResizableDialog
{
private:
    struct isdb_movie_parsed {
        CString titles;
        CString name;
        CString language;
        CString format;
        CString disc;
        DWORD_PTR ptr;
        bool checked;
    };

    typedef struct THREADSTRUCT {
        HWND hWND;
        CInternetSession is;
        CStringA url;
        CStringA raw_list;
        CStringA ticket;
        CList<isdb_movie> raw_movies;
    } THREADSTRUCT, *PTHREADSTRUCT;

    typedef struct PARAMSORT {
        PARAMSORT(HWND hWnd, int colIndex, bool ascending) :
            m_hWnd(hWnd),
            m_colIndex(colIndex),
            m_ascending(ascending)
        {}
        HWND m_hWnd;
        int m_colIndex;
        bool m_ascending;
    } PARAMSORT, *PPARAMSORT;

    typedef struct DEFPARAMSORT {
        DEFPARAMSORT(HWND hWnd, CString filename) :
            m_hWnd(hWnd),
            m_filename(filename)
        {}
        HWND m_hWnd;
        CString m_filename;
        CMap <CString, LPCTSTR, int, int> m_langPos;
    } DEFPARAMSORT, *PDEFPARAMSORT;

    enum {
        COL_FILENAME,
        COL_LANGUAGE,
        COL_FORMAT,
        COL_DISC,
        COL_TITLES
    };
    PARAMSORT ps;
    DEFPARAMSORT defps;
    PTHREADSTRUCT m_pTA;

    CArray<isdb_movie_parsed> m_parsed_movies;
    CString m_url;
    bool m_fReplaceSubs;

    CListCtrl m_list;
    CList<isdb_subtitle> m_selsubs;
    CStatusBarCtrl m_status;

    void SetStatus(const CString& status);
    bool Parse();
    void LoadList();

    static UINT RunThread(LPVOID pParam);
    static int CALLBACK SortCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    static int CALLBACK DefSortCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    static size_t StrMatch(LPCTSTR a, LPCTSTR b);
    static CString LangCodeToName(LPCSTR code);
public:
    explicit CSubtitleDlDlg(CWnd* pParent, const CStringA& url, const CString& filename);
    virtual ~CSubtitleDlDlg();

    enum { IDD = IDD_SUBTITLEDL_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnOK();

    void DownloadSelectedSubtitles();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnParse();
    afx_msg void OnFailedConnection();
    afx_msg void OnUpdateOk(CCmdUI* pCmdUI);
    afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnDoubleClickSubtitle(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKeyPressedSubtitle(NMHDR* pNMHDR, LRESULT* pResult);
};
