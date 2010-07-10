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
#include "VSRip.h"
#include <atlcoll.h>
#include "../../../include/winddk/devioctl.h"
#include "../../../include/winddk/ntddcdrm.h"
#include "VSRipFileDlg.h"


// CVSRipFileDlg dialog

IMPLEMENT_DYNAMIC(CVSRipFileDlg, CVSRipPage)
CVSRipFileDlg::CVSRipFileDlg(IVSFRipper* pVSFRipper, CWnd* pParent /*=NULL*/)
    : CVSRipPage(pVSFRipper, CVSRipFileDlg::IDD, pParent)
{
}

CVSRipFileDlg::~CVSRipFileDlg()
{
}

void CVSRipFileDlg::DoDataExchange(CDataExchange* pDX)
{
    CVSRipPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT1, m_infn);
    DDX_Text(pDX, IDC_EDIT2, m_outfn);
    DDX_Control(pDX, IDC_EDIT3, m_log);
}

BEGIN_MESSAGE_MAP(CVSRipFileDlg, CVSRipPage)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
END_MESSAGE_MAP()

STDMETHODIMP CVSRipFileDlg::OnMessage(LPCTSTR msg)
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

// CVSRipFileDlg message handlers

void CVSRipFileDlg::OnBnClickedButton1()
{
    CFileDialog fd(TRUE, NULL, NULL,
                   OFN_EXPLORER|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,
                   _T("Video Title Set IFO file (*.ifo)|*.ifo|"), this, 0);

    if(fd.DoModal() == IDOK)
    {
        m_log.SetWindowText(_T(""));
        m_log.SetMargins(0, 0);

        CString fn = fd.GetPathName();
        if(FAILED(m_pVSFRipper->SetInput(fn))) fn.Empty();
        m_infn = fn;

        UpdateData(FALSE);
    }

}

void CVSRipFileDlg::OnBnClickedButton2()
{
    CString fn = m_infn.Mid(m_infn.ReverseFind('\\')+1);
    int i = fn.ReverseFind('.');
    if(i > 0) fn = fn.Left(i);

    CFileDialog fd(FALSE, NULL, fn,
                   OFN_EXPLORER|OFN_ENABLESIZING|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST,
                   _T("VobSub index file (*.idx)|*.idx|"), this, 0);

    if(fd.DoModal() == IDOK)
    {
        CString fn = fd.GetPathName();
        if(FAILED(m_pVSFRipper->SetOutput(fn))) fn.Empty();
        m_outfn = fn;

        UpdateData(FALSE);
    }
}
