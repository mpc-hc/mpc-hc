/*
 * (C) 2012-2013 see Authors.txt
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

#include "stdafx.h"
#include "mplayerc.h"
#include "SaveSubtitlesFileDialog.h"
#include "SysVersion.h"


// CSaveSubtitlesFileDialog

IMPLEMENT_DYNAMIC(CSaveSubtitlesFileDialog, CSaveTextFileDialog)

CSaveSubtitlesFileDialog::CSaveSubtitlesFileDialog(
    CTextFile::enc e, int delay,
    LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
    LPCTSTR lpszFilter, CWnd* pParentWnd)
    : CSaveTextFileDialog(e, lpszDefExt, lpszFileName, lpszFilter, pParentWnd)
    , m_bDisableEncoding(false)
    , m_delay(delay)
{
    InitCustomization();
}

CSaveSubtitlesFileDialog::CSaveSubtitlesFileDialog(
    int delay,
    LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
    LPCTSTR lpszFilter, CWnd* pParentWnd)
    : CSaveTextFileDialog(CTextFile::ANSI, lpszDefExt, lpszFileName, lpszFilter, pParentWnd)
    , m_bDisableEncoding(true)
    , m_delay(delay)
{
    InitCustomization();
}

void CSaveSubtitlesFileDialog::InitCustomization()
{
    if (SysVersion::IsVistaOrLater()) {
        // customization has to be done before OnInitDialog
        IFileDialogCustomize* pfdc = GetIFileDialogCustomize();

        pfdc->StartVisualGroup(IDS_SUBFILE_DELAY, ResStr(IDS_SUBFILE_DELAY));

        CString strDelay;
        strDelay.Format(_T("%d"), m_delay);
        pfdc->AddEditBox(IDC_EDIT1, strDelay);

        pfdc->EndVisualGroup();

        if (m_bDisableEncoding) {
            pfdc->SetControlState(IDS_TEXTFILE_ENC, CDCS_INACTIVE);
            pfdc->SetControlState(IDC_COMBO1, CDCS_INACTIVE);
        }

        pfdc->Release();
    } else {
        SetTemplate(0, IDD_SAVESUBTITLESFILEDIALOGTEMPL);
    }
}

CSaveSubtitlesFileDialog::~CSaveSubtitlesFileDialog()
{
}

void CSaveSubtitlesFileDialog::DoDataExchange(CDataExchange* pDX)
{
    if (!SysVersion::IsVistaOrLater()) {
        DDX_Control(pDX, IDC_SPIN1, m_delayCtrl);
        DDX_Text(pDX, IDC_EDIT1, m_delay);
    }
    __super::DoDataExchange(pDX);
}

BOOL CSaveSubtitlesFileDialog::OnInitDialog()
{
    __super::OnInitDialog();

    if (!SysVersion::IsVistaOrLater()) {
        m_delayCtrl.SetRange32(INT_MIN, INT_MAX);
        m_delayCtrl.SetPos32(m_delay);

        if (m_bDisableEncoding) {
            GetDlgItem(IDC_STATIC1)->EnableWindow(FALSE);
            GetDlgItem(IDC_COMBO1)->EnableWindow(FALSE);
        }
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CSaveSubtitlesFileDialog, CSaveTextFileDialog)
END_MESSAGE_MAP()

// CSaveTextFileDialog message handlers

BOOL CSaveSubtitlesFileDialog::OnFileNameOK()
{
    if (SysVersion::IsVistaOrLater()) {
        IFileDialogCustomize* pfdc = GetIFileDialogCustomize();
        WCHAR* strDelay = nullptr;

        pfdc->GetEditBoxText(IDC_EDIT1, &strDelay);
        if (strDelay) {
            m_delay = _tcstol(strDelay, nullptr, 10);
            CoTaskMemFree(strDelay);
        }

        pfdc->Release();
    } else {
        UpdateData();
    }

    return __super::OnFileNameOK();
}
