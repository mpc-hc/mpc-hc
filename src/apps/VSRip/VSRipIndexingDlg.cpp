/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of vsrip.
 *
 * Vsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <afxpriv.h>
#include "VSRip.h"
#include "VSRipIndexingDlg.h"

// CVSRipIndexingDlg dialog

IMPLEMENT_DYNAMIC(CVSRipIndexingDlg, CVSRipPage)
CVSRipIndexingDlg::CVSRipIndexingDlg(IVSFRipper* pVSFRipper, CWnd* pParent /*=NULL*/)
    : CVSRipPage(pVSFRipper, CVSRipIndexingDlg::IDD, pParent)
    , m_bBeep(FALSE), m_bExit(FALSE)
    , m_fFinished(false)
    , m_fAuto(false)
{
}

CVSRipIndexingDlg::~CVSRipIndexingDlg()
{
}

void CVSRipIndexingDlg::DoDataExchange(CDataExchange* pDX)
{
    CVSRipPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS1, m_progress);
    DDX_Control(pDX, IDC_EDIT1, m_log);
    DDX_Check(pDX, IDC_CHECK1, m_bExit);
    DDX_Check(pDX, IDC_CHECK2, m_bBeep);
}

STDMETHODIMP CVSRipIndexingDlg::OnMessage(LPCTSTR msg)
{
    if(CEdit* pLog = (CEdit*)CEdit::FromHandle(m_log.m_hWnd))
    {
        CString str = msg;
        str += _T("\r\n");
        int len = pLog->GetWindowTextLength();
        pLog->SetSel(len, len);
        pLog->ReplaceSel(str);
    }

    return S_OK;
}

STDMETHODIMP CVSRipIndexingDlg::OnProgress(double progress)
{
    if(CProgressCtrl* pProgress = (CProgressCtrl*)CProgressCtrl::FromHandle(m_progress.m_hWnd))
    {
        pProgress->SetPos((int)(progress * 100));
    }

    return S_OK;
}

STDMETHODIMP CVSRipIndexingDlg::OnFinished(bool fSucceeded)
{
    m_fFinished = fSucceeded;

    GetParent()->PostMessage(WM_KICKIDLE); // and kick it hard :)

    if(m_fFinished && m_bBeep) MessageBeep(-1);
    if(m_fFinished && m_bExit) GetParent()->PostMessage(WM_COMMAND, IDCANCEL);

    if(!fSucceeded)
    {
        VSFRipperData rd;
        m_pVSFRipper->GetRipperData(rd);
        if(rd.fCloseIgnoreError) GetParent()->PostMessage(WM_COMMAND, IDCANCEL);
    }

    return S_OK;
}

BEGIN_MESSAGE_MAP(CVSRipIndexingDlg, CVSRipPage)
    ON_BN_CLICKED(IDC_BUTTON1, OnIndex)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateIndex)
    ON_WM_SHOWWINDOW()
    ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
    ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
END_MESSAGE_MAP()


// CVSRipIndexingDlg message handlers

void CVSRipIndexingDlg::OnIndex()
{
    if(S_OK == m_pVSFRipper->IsIndexing())
    {
        m_pVSFRipper->Abort(false);
    }
    else
    {
        m_progress.SetRange(0, 100);
        m_progress.SetPos(0);
        m_log.SetWindowText(_T(""));
        m_log.SetMargins(0, 0);

        m_pVSFRipper->Index();
    }

    GetParent()->PostMessage(WM_KICKIDLE); // and kick it hard :)
}

void CVSRipIndexingDlg::OnUpdateIndex(CCmdUI* pCmdUI)
{
    pCmdUI->SetText(S_OK == m_pVSFRipper->IsIndexing() ? _T("&Stop") : _T("Re&start"));
}

void CVSRipIndexingDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __super::OnShowWindow(bShow, nStatus);

    m_fFinished = false;

    if(bShow)
    {
        VSFRipperData rd;
        m_pVSFRipper->GetRipperData(rd);
        m_bBeep = rd.fBeep;
        m_bExit = rd.fClose;
        m_fAuto = rd.fAuto;
        UpdateData(FALSE);

        if(S_OK != m_pVSFRipper->IsIndexing())
        {
            if(!m_fAuto)
            {
                m_progress.SetRange(0, 100);
                m_progress.SetPos(0);
                m_log.SetWindowText(_T(""));
                m_log.SetMargins(0, 0);
            }

            m_pVSFRipper->Index();
        }
    }
    else
    {
        VSFRipperData rd;
        m_pVSFRipper->GetRipperData(rd);
        UpdateData();
        rd.fBeep = !m_bBeep;
        rd.fClose = !!m_bExit;
        m_pVSFRipper->UpdateRipperData(rd);
    }
}

void CVSRipIndexingDlg::OnBnClickedCheck2()
{
    UpdateData();
}

void CVSRipIndexingDlg::OnBnClickedCheck1()
{
    UpdateData();
}
