/*

*  (C) 2017 see Authors.txt
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
#include "PPageAudioExtract.h"
#include "MainFrm.h"
#include <string>
 

IMPLEMENT_DYNAMIC(CPPageAudioExtract, CPPageBase)

CPPageAudioExtract::CPPageAudioExtract()
    : CPPageBase(CPPageAudioExtract::IDD, CPPageAudioExtract::IDD)
{
}


CPPageAudioExtract::~CPPageAudioExtract()
{
}

BEGIN_MESSAGE_MAP(CPPageAudioExtract, CPPageBase)
    ON_BN_CLICKED(IDC_BTN_AUDIO_FOLDER, &CPPageAudioExtract::OnBnClickedBtnAudioFolder)
    ON_CBN_SELCHANGE(IDC_MP3_BITRATE, &CPPageAudioExtract::OnCbnSelchangeMp3Bitrate)
    ON_CBN_SELCHANGE(IDC_MP3_STEREO_MODE, &CPPageAudioExtract::OnCbnSelchangeMp3StereoMode)
END_MESSAGE_MAP()

void CPPageAudioExtract::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);

}

void CPPageAudioExtract::OnBnClickedBtnAudioFolder()
{
    CString vStr;

    CEdit *vEdit;
    vEdit = (CEdit *)GetDlgItem(IDC_EDIT_AUDIO_OUTPUT_FOLDER);
    vEdit->GetWindowText(vStr);

    CString strTitle(StrRes(IDS_MAINFRM_DIR_TITLE));
    CString path;

    CFileDialog dlg(TRUE);
    IFileOpenDialog* openDlgPtr = dlg.GetIFileOpenDialog();

    if (openDlgPtr != nullptr) {
        openDlgPtr->SetTitle(strTitle);
        openDlgPtr->SetFileName(vStr);
        openDlgPtr->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        if (FAILED(openDlgPtr->Show(m_hWnd))) {
            openDlgPtr->Release();
            return;
        }
        openDlgPtr->Release();

        path = dlg.GetFolderPath();

    }
    else {
        return;
    }


    if (path[path.GetLength() - 1] != '\\') {
        path += _T('\\');
    }

    vEdit->SetWindowText(path);
}


void CPPageAudioExtract::OnCbnSelchangeMp3Bitrate()
{
    // TODO: Add your control notification handler code here
    SetModified(TRUE);
}


void CPPageAudioExtract::OnCbnSelchangeMp3StereoMode()
{
    // TODO: Add your control notification handler code here
    SetModified(TRUE);
}


BOOL CPPageAudioExtract::OnInitDialog()
{
    //int MP3_BitRates[] = { 128, 160, 192, 256, 320 };
    //std::string MP3_StereoModes[] = { "Mono","Stereo" };

    __super::OnInitDialog();


    const CAppSettings& s = AfxGetAppSettings();


    CEdit *pEdit;
    CComboBox *pComb;

    pEdit = (CEdit *)GetDlgItem(IDC_EDIT_AUDIO_OUTPUT_FOLDER);
    pEdit->SetWindowText(s.strAudioRecordPath);

    pComb = (CComboBox *)GetDlgItem(IDC_MP3_BITRATE);
    pComb->SetCurSel(s.iMP3BitRateidx);
    pComb = (CComboBox *)GetDlgItem(IDC_MP3_STEREO_MODE);
    pComb->SetCurSel(s.iMP3SteroeModeidx);

//    CreateToolTip();
//    EnableToolTips(TRUE);

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
}


BOOL CPPageAudioExtract::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();

    CEdit *pEdit;
    CComboBox *pComb;
    CString strPath; //vszPath[MAX_PATH + 1];

    pEdit = (CEdit *)GetDlgItem(IDC_EDIT_AUDIO_OUTPUT_FOLDER);
    pEdit->GetWindowText(strPath); // MAX_PATH);
    s.strAudioRecordPath = strPath;

    pComb = (CComboBox *)GetDlgItem(IDC_MP3_BITRATE);
    s.iMP3BitRateidx=pComb->GetCurSel();

    pComb = (CComboBox *)GetDlgItem(IDC_MP3_STEREO_MODE);
    s.iMP3SteroeModeidx=pComb->GetCurSel();


    return __super::OnApply();
}
