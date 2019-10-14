/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2017 see Authors.txt
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
#include "MainFrm.h"
#include "PPageDVD.h"


struct {
    LCID   lcid;
    LPCSTR name;
}

static constexpr LCIDNameList[] = {
    {0x0000, "Default"},
    {0x0436, "Afrikaans"},
    {0x041c, "Albanian"},
    {0x0401, "Arabic (Saudi Arabia)"},
    {0x0801, "Arabic (Iraq)"},
    {0x0c01, "Arabic (Egypt)"},
    {0x1001, "Arabic (Libya)"},
    {0x1401, "Arabic (Algeria)"},
    {0x1801, "Arabic (Morocco)"},
    {0x1c01, "Arabic (Tunisia)"},
    {0x2001, "Arabic (Oman)"},
    {0x2401, "Arabic (Yemen)"},
    {0x2801, "Arabic (Syria)"},
    {0x2c01, "Arabic (Jordan)"},
    {0x3001, "Arabic (Lebanon)"},
    {0x3401, "Arabic (Kuwait)"},
    {0x3801, "Arabic (U.A.E.)"},
    {0x3c01, "Arabic (Bahrain)"},
    {0x4001, "Arabic (Qatar)"},
    {0x042b, "Armenian"},
    {0x042c, "Azeri (Latin)"},
    {0x082c, "Azeri (Cyrillic)"},
    {0x042d, "Basque"},
    {0x0423, "Belarusian"},
    {0x0402, "Bulgarian"},
    {0x0455, "Burmese"},
    {0x0403, "Catalan"},
    {0x0404, "Chinese (Taiwan)"},
    {0x0804, "Chinese (PRC)"},
    {0x0c04, "Chinese (Hong Kong SAR, PRC)"},
    {0x1004, "Chinese (Singapore)"},
    {0x1404, "Chinese (Macau SAR)"},
    {0x041a, "Croatian"},
    {0x0405, "Czech"},
    {0x0406, "Danish"},
    {0x0465, "Divehi"},
    {0x0413, "Dutch (Netherlands)"},
    {0x0813, "Dutch (Belgium)"},
    {0x0409, "English (United States)"},
    {0x0809, "English (United Kingdom)"},
    {0x0c09, "English (Australian)"},
    {0x1009, "English (Canadian)"},
    {0x1409, "English (New Zealand)"},
    {0x1809, "English (Ireland)"},
    {0x1c09, "English (South Africa)"},
    {0x2009, "English (Jamaica)"},
    {0x2409, "English (Caribbean)"},
    {0x2809, "English (Belize)"},
    {0x2c09, "English (Trinidad)"},
    {0x3009, "English (Zimbabwe)"},
    {0x3409, "English (Philippines)"},
    {0x0425, "Estonian"},
    {0x0438, "Faeroese"},
    {0x0429, "Farsi"},
    {0x040b, "Finnish"},
    {0x040c, "French (Standard)"},
    {0x080c, "French (Belgian)"},
    {0x0c0c, "French (Canadian)"},
    {0x100c, "French (Switzerland)"},
    {0x140c, "French (Luxembourg)"},
    {0x180c, "French (Monaco)"},
    {0x0456, "Galician"},
    {0x0437, "Georgian"},
    {0x0407, "German (Standard)"},
    {0x0807, "German (Switzerland)"},
    {0x0c07, "German (Austria)"},
    {0x1007, "German (Luxembourg)"},
    {0x1407, "German (Liechtenstein)"},
    {0x0408, "Greek"},
    {0x0447, "Gujarati"},
    {0x040d, "Hebrew"},
    {0x0439, "Hindi"},
    {0x040e, "Hungarian"},
    {0x040f, "Icelandic"},
    {0x0421, "Indonesian"},
    {0x0410, "Italian (Standard)"},
    {0x0810, "Italian (Switzerland)"},
    {0x0411, "Japanese"},
    {0x044b, "Kannada"},
    {0x0457, "Konkani"},
    {0x0412, "Korean"},
    {0x0812, "Korean (Johab)"},
    {0x0440, "Kyrgyz"},
    {0x0426, "Latvian"},
    {0x0427, "Lithuanian"},
    {0x0827, "Lithuanian (Classic)"},
    {0x042f, "FYRO Macedonian"},
    {0x043e, "Malay (Malaysian)"},
    {0x083e, "Malay (Brunei Darussalam)"},
    {0x044e, "Marathi"},
    {0x0450, "Mongolian"},
    {0x0414, "Norwegian (Bokmal)"},
    {0x0814, "Norwegian (Nynorsk)"},
    {0x0415, "Polish"},
    {0x0416, "Portuguese (Brazil)"},
    {0x0816, "Portuguese (Portugal)"},
    {0x0446, "Punjabi"},
    {0x0418, "Romanian"},
    {0x0419, "Russian"},
    {0x044f, "Sanskrit"},
    {0x0c1a, "Serbian (Cyrillic)"},
    {0x081a, "Serbian (Latin)"},
    {0x041b, "Slovak"},
    {0x0424, "Slovenian"},
    {0x040a, "Spanish (Spain, Traditional Sort)"},
    {0x080a, "Spanish (Mexican)"},
    {0x0c0a, "Spanish (Spain, International Sort)"},
    {0x100a, "Spanish (Guatemala)"},
    {0x140a, "Spanish (Costa Rica)"},
    {0x180a, "Spanish (Panama)"},
    {0x1c0a, "Spanish (Dominican Republic)"},
    {0x200a, "Spanish (Venezuela)"},
    {0x240a, "Spanish (Colombia)"},
    {0x280a, "Spanish (Peru)"},
    {0x2c0a, "Spanish (Argentina)"},
    {0x300a, "Spanish (Ecuador)"},
    {0x340a, "Spanish (Chile)"},
    {0x380a, "Spanish (Uruguay)"},
    {0x3c0a, "Spanish (Paraguay)"},
    {0x400a, "Spanish (Bolivia)"},
    {0x440a, "Spanish (El Salvador)"},
    {0x480a, "Spanish (Honduras)"},
    {0x4c0a, "Spanish (Nicaragua)"},
    {0x500a, "Spanish (Puerto Rico)"},
    {0x0430, "Sutu"},
    {0x0441, "Swahili (Kenya)"},
    {0x041d, "Swedish"},
    {0x081d, "Swedish (Finland)"},
    {0x045a, "Syriac"},
    {0x0449, "Tamil"},
    {0x0444, "Tatar (Tatarstan)"},
    {0x044a, "Telugu"},
    {0x041e, "Thai"},
    {0x041f, "Turkish"},
    {0x0422, "Ukrainian"},
    {0x0420, "Urdu (Pakistan)"},
    {0x0820, "Urdu (India)"},
    {0x0443, "Uzbek (Latin)"},
    {0x0843, "Uzbek (Cyrillic)"},
    {0x042a, "Vietnamese"}
};


// CPPageDVD dialog

IMPLEMENT_DYNAMIC(CPPageDVD, CMPCThemePPageBase)
CPPageDVD::CPPageDVD()
    : CMPCThemePPageBase(CPPageDVD::IDD, CPPageDVD::IDD)
    , m_iDVDLocation(0)
    , m_iDVDLangType(0)
    , m_idMenuLang(0)
    , m_idAudioLang(0)
    , m_idSubtitlesLang(0)
    , m_fClosedCaptions(FALSE)
{
}

CPPageDVD::~CPPageDVD()
{
}

void CPPageDVD::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_RADIO1, m_iDVDLocation);
    DDX_Radio(pDX, IDC_RADIO3, m_iDVDLangType);
    DDX_Control(pDX, IDC_LIST1, m_lcids);
    DDX_Text(pDX, IDC_DVDPATH, m_dvdpath);
    DDX_Control(pDX, IDC_DVDPATH, m_dvdpathctrl);
    DDX_Control(pDX, IDC_BUTTON1, m_dvdpathselctrl);
    DDX_Check(pDX, IDC_CHECK2, m_fClosedCaptions);
}

void CPPageDVD::UpdateLCIDList()
{
    UpdateData();

    LCID lcid = m_iDVDLangType == 0 ? m_idMenuLang
                : m_iDVDLangType == 1 ? m_idAudioLang
                : m_idSubtitlesLang;

    for (int i = 0; i < m_lcids.GetCount(); i++) {
        if (m_lcids.GetItemData(i) == lcid) {
            m_lcids.SetCurSel(i);
            m_lcids.SetTopIndex(i);
            break;
        }
    }
}

BEGIN_MESSAGE_MAP(CPPageDVD, CMPCThemePPageBase)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO3, IDC_RADIO5, OnBnClickedLangradio123)
    ON_LBN_SELCHANGE(IDC_LIST1, OnLbnSelchangeList1)
    ON_UPDATE_COMMAND_UI(IDC_DVDPATH, OnUpdateDVDPath)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateDVDPath)
END_MESSAGE_MAP()


// CPPageDVD message handlers

BOOL CPPageDVD::OnInitDialog()
{
    __super::OnInitDialog();

    const CAppSettings& s = AfxGetAppSettings();

    m_iDVDLocation = s.fUseDVDPath ? 1 : 0;
    m_dvdpath = s.strDVDPath;
    m_iDVDLangType = 0;

    m_idMenuLang = s.idMenuLang;
    m_idAudioLang = s.idAudioLang;
    m_idSubtitlesLang = s.idSubtitlesLang;
    m_fClosedCaptions = s.fClosedCaptions;

    UpdateData(FALSE);

    for (int i = 0; i < _countof(LCIDNameList); i++) {
        m_lcids.AddString(CString(LCIDNameList[i].name));
        m_lcids.SetItemData(i, LCIDNameList[i].lcid);
    }

    UpdateLCIDList();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageDVD::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();

    s.strDVDPath = m_dvdpath;
    s.fUseDVDPath = (m_iDVDLocation == 1);
    s.idMenuLang = m_idMenuLang;
    s.idAudioLang = m_idAudioLang;
    s.idSubtitlesLang = m_idSubtitlesLang;
    s.fClosedCaptions = !!m_fClosedCaptions;

    ((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetClosedCaptions(s.fClosedCaptions);

    return __super::OnApply();
}

void CPPageDVD::OnBnClickedButton1()
{
    CString path;
    CString strTitle(StrRes(IDS_MAINFRM_46));
    CFileDialog dlg(TRUE);
    IFileOpenDialog* openDlgPtr = dlg.GetIFileOpenDialog();

    if (openDlgPtr != nullptr) {
        openDlgPtr->SetTitle(strTitle);
        openDlgPtr->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        if (FAILED(openDlgPtr->Show(m_hWnd))) {
            openDlgPtr->Release();
            return;
        }
        openDlgPtr->Release();

        path = dlg.GetFolderPath();
    }

    if (!path.IsEmpty()) {
        m_dvdpath = path;

        UpdateData(FALSE);

        SetModified();
    }
}

void CPPageDVD::OnBnClickedLangradio123(UINT nID)
{
    UpdateLCIDList();
}

void CPPageDVD::OnLbnSelchangeList1()
{
    LCID& lcid = m_iDVDLangType == 0 ? m_idMenuLang
                 : m_iDVDLangType == 1 ? m_idAudioLang
                 : m_idSubtitlesLang;

    lcid = (LCID)m_lcids.GetItemData(m_lcids.GetCurSel());

    SetModified();
}

void CPPageDVD::OnUpdateDVDPath(CCmdUI* pCmdUI)
{
    UpdateData();

    pCmdUI->Enable(m_iDVDLocation == 1);
}
