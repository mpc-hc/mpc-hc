/*
 * (C) 2012-2014 see Authors.txt
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

#include <afxwin.h>
#include "resource.h"
#include "UpdateChecker.h"
#include <afxinet.h>
#include "text.h"
#include <algorithm>
#include <vector>

class CDownloadThread;

class UpdateCheckerDlg : public CDialog
{
    DECLARE_DYNAMIC(UpdateCheckerDlg)

public:
    UpdateCheckerDlg(Update_Status updateStatus, UpdateChecker *updateChecker, CWnd* pParent = nullptr);
    virtual ~UpdateCheckerDlg();

    enum { IDD = IDD_UPDATE_DIALOG };

    void DownloadStarted(double total);
    void DownloadProgress(double done, double total, double speed);
    void DownloadFinished(double total, const CString &file);
    void DownloadAborted();
    void DownloadFailed();

protected:
    afx_msg virtual void DoDataExchange(CDataExchange* pDX);
    afx_msg virtual BOOL OnInitDialog();
    afx_msg virtual BOOL DestroyWindow();
    afx_msg void OnOpenDownloadPage();
    afx_msg void OnUpdateLater();
    afx_msg void OnIgnoreUpdate();
    afx_msg void OnCancelUpdate();

    DECLARE_MESSAGE_MAP()
private:
    Update_Status m_updateStatus;
    UpdateChecker *m_updateChecker;
    CString m_text;
    CStatic m_icon;
    CProgressCtrl m_progress;
    CButton m_dlButton;
    CButton m_laterButton;
    CButton m_ignoreButton;
    CButton m_cancelButton;
    CDownloadThread *m_pDownloadThread;
public:
};


class CWinThreadProc {
public:
    CWinThreadProc() : m_bAbort(false), m_pThread(nullptr) {}
    ~CWinThreadProc() { }
    operator CWinThread*() const { return m_pThread; }

    bool IsThreadRunning() { return m_pThread != nullptr; }
    volatile bool IsThreadAborting() { return m_bAbort; }

    void CreateThread() { if (!IsThreadRunning()) { m_pThread = AfxBeginThread(_ThreadProc, this); } }
    void AbortThread() { if (IsThreadRunning()) { m_bAbort = true; } }
    void WaitThread() { if (IsThreadRunning()) { DWORD dwWait = ::WaitForSingleObjectEx(*m_pThread, INFINITE, TRUE); } }

private:
    static UINT _ThreadProc(LPVOID pThreadParams) {
        CWinThreadProc* pThread = (CWinThreadProc*)pThreadParams;
        pThread->ThreadProc();
        pThread->m_pThread = nullptr;
        pThread->m_bAbort = false;
        return 0;
    }
    virtual void ThreadProc() PURE;

    CWinThread *m_pThread;
    volatile bool m_bAbort;
};


class CDownloadThread : public CWinThreadProc {
public:
    CDownloadThread(UpdateChecker *updateChecker, UpdateCheckerDlg * pUpdateCheckerDlg) { m_updateChecker = updateChecker; m_pUpdateCheckerDlg = pUpdateCheckerDlg; }
private:
    UpdateCheckerDlg *m_pUpdateCheckerDlg;
    UpdateChecker *m_updateChecker;

    virtual void ThreadProc();
};
