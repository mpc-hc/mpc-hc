/*
 * (C) 2014 see Authors.txt
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
#include "SaveImageDialog.h"
#include "SysVersion.h"


IMPLEMENT_DYNAMIC(CSaveImageDialog, CFileDialog)
CSaveImageDialog::CSaveImageDialog(
    int nJpegQuality,
    LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
    LPCTSTR lpszFilter, CWnd* pParentWnd) :
    CFileDialog(FALSE, lpszDefExt, lpszFileName,
                OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR,
                lpszFilter, pParentWnd, 0),
    m_nJpegQuality(nJpegQuality)
{
    if (SysVersion::IsVistaOrLater()) {
        IFileDialogCustomize* pfdc = GetIFileDialogCustomize();
        CString str;

        pfdc->StartVisualGroup(IDS_IMAGE_JPEG_QUALITY, ResStr(IDS_IMAGE_JPEG_QUALITY));
        pfdc->AddText(IDS_IMAGE_QUALITY, ResStr(IDS_IMAGE_QUALITY));
        str.Format(L"%d", std::max(0, std::min(100, m_nJpegQuality)));
        pfdc->AddEditBox(IDC_EDIT1, str);
        pfdc->EndVisualGroup();

        pfdc->Release();
    } else {
        SetTemplate(0, IDD_SAVEIMAGEDIALOGTEMPL);
    }
}

CSaveImageDialog::~CSaveImageDialog()
{
}

void CSaveImageDialog::DoDataExchange(CDataExchange* pDX)
{
    if (!SysVersion::IsVistaOrLater()) {
        DDX_Control(pDX, IDC_SPIN1, m_jpegQualitySpin);
        DDX_Control(pDX, IDC_EDIT1, m_jpegQualityEdit);
    }
    __super::DoDataExchange(pDX);
}

BOOL CSaveImageDialog::OnInitDialog()
{
    __super::OnInitDialog();

    if (!SysVersion::IsVistaOrLater()) {
        m_jpegQualitySpin.SetRange32(0, 100);
        m_jpegQualitySpin.SetPos(m_nJpegQuality);

        OnTypeChange();
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CSaveImageDialog, CFileDialog)
END_MESSAGE_MAP()

BOOL CSaveImageDialog::OnFileNameOK()
{
    if (SysVersion::IsVistaOrLater()) {
        IFileDialogCustomize* pfdc = GetIFileDialogCustomize();
        WCHAR* result;

        pfdc->GetEditBoxText(IDC_EDIT1, &result);
        m_nJpegQuality = _wtoi(result);
        CoTaskMemFree(result);

        pfdc->Release();
    } else {
        m_nJpegQuality = m_jpegQualitySpin.GetPos32();
    }

    m_nJpegQuality = std::max(0, std::min(100, m_nJpegQuality));

    return __super::OnFileNameOK();
}

void CSaveImageDialog::OnTypeChange()
{
    __super::OnTypeChange();

    if (SysVersion::IsVistaOrLater()) {
        IFileDialogCustomize* pfdc = GetIFileDialogCustomize();

        if (m_pOFN->nFilterIndex == 2) { // JPEG encoding is chosen
            pfdc->SetControlState(IDS_IMAGE_JPEG_QUALITY, CDCS_ENABLEDVISIBLE);
            pfdc->SetControlState(IDS_IMAGE_QUALITY, CDCS_ENABLEDVISIBLE);
            pfdc->SetControlState(IDC_EDIT1, CDCS_ENABLEDVISIBLE);
        } else {
            pfdc->SetControlState(IDS_IMAGE_JPEG_QUALITY, CDCS_INACTIVE);
            pfdc->SetControlState(IDS_IMAGE_QUALITY, CDCS_INACTIVE);
            pfdc->SetControlState(IDC_EDIT1, CDCS_INACTIVE);
        }

        pfdc->Release();
    } else {
        if (m_pOFN->nFilterIndex == 2) { // JPEG encoding is chosen
            m_jpegQualitySpin.EnableWindow(TRUE);
            m_jpegQualityEdit.EnableWindow(TRUE);
        } else {
            m_jpegQualitySpin.EnableWindow(FALSE);
            m_jpegQualityEdit.EnableWindow(FALSE);
        }
    }
}
