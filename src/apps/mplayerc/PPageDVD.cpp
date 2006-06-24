/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

// PPageDVD.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PPageDVD.h"

struct
{
	LCID lcid;
	TCHAR name[64];
} 
LCIDNameList[] =
{
	{0x0000, _T("Default")},
	{0x0436, _T("Afrikaans")},
	{0x041c, _T("Albanian")},
	{0x0401, _T("Arabic (Saudi Arabia)")},
	{0x0801, _T("Arabic (Iraq)")},
	{0x0c01, _T("Arabic (Egypt)")},
	{0x1001, _T("Arabic (Libya)")},
	{0x1401, _T("Arabic (Algeria)")},
	{0x1801, _T("Arabic (Morocco)")},
	{0x1c01, _T("Arabic (Tunisia)")},
	{0x2001, _T("Arabic (Oman)")},
	{0x2401, _T("Arabic (Yemen)")},
	{0x2801, _T("Arabic (Syria)")},
	{0x2c01, _T("Arabic (Jordan)")},
	{0x3001, _T("Arabic (Lebanon)")},
	{0x3401, _T("Arabic (Kuwait)")},
	{0x3801, _T("Arabic (U.A.E.)")},
	{0x3c01, _T("Arabic (Bahrain)")},
	{0x4001, _T("Arabic (Qatar)")},
	{0x042b, _T("Armenian")},
	{0x042c, _T("Azeri (Latin)")},
	{0x082c, _T("Azeri (Cyrillic)")},
	{0x042d, _T("Basque")},
	{0x0423, _T("Belarusian")},
	{0x0402, _T("Bulgarian")},
	{0x0455, _T("Burmese")},
	{0x0403, _T("Catalan")},
	{0x0404, _T("Chinese (Taiwan)")},
	{0x0804, _T("Chinese (PRC)")},
	{0x0c04, _T("Chinese (Hong Kong SAR, PRC)")},
	{0x1004, _T("Chinese (Singapore)")},
	{0x1404, _T("Chinese (Macau SAR)")},
	{0x041a, _T("Croatian")},
	{0x0405, _T("Czech")},
	{0x0406, _T("Danish")},
	{0x0465, _T("Divehi")},
	{0x0413, _T("Dutch (Netherlands)")},
	{0x0813, _T("Dutch (Belgium)")},
	{0x0409, _T("English (United States)")},
	{0x0809, _T("English (United Kingdom)")},
	{0x0c09, _T("English (Australian)")},
	{0x1009, _T("English (Canadian)")},
	{0x1409, _T("English (New Zealand)")},
	{0x1809, _T("English (Ireland)")},
	{0x1c09, _T("English (South Africa)")},
	{0x2009, _T("English (Jamaica)")},
	{0x2409, _T("English (Caribbean)")},
	{0x2809, _T("English (Belize)")},
	{0x2c09, _T("English (Trinidad)")},
	{0x3009, _T("English (Zimbabwe)")},
	{0x3409, _T("English (Philippines)")},
	{0x0425, _T("Estonian")},
	{0x0438, _T("Faeroese")},
	{0x0429, _T("Farsi")},
	{0x040b, _T("Finnish")},
	{0x040c, _T("French (Standard)")},
	{0x080c, _T("French (Belgian)")},
	{0x0c0c, _T("French (Canadian)")},
	{0x100c, _T("French (Switzerland)")},
	{0x140c, _T("French (Luxembourg)")},
	{0x180c, _T("French (Monaco)")},
	{0x0456, _T("Galician")},
	{0x0437, _T("Georgian")},
	{0x0407, _T("German (Standard)")},
	{0x0807, _T("German (Switzerland)")},
	{0x0c07, _T("German (Austria)")},
	{0x1007, _T("German (Luxembourg)")},
	{0x1407, _T("German (Liechtenstein)")},
	{0x0408, _T("Greek")},
	{0x0447, _T("Gujarati")},
	{0x040d, _T("Hebrew")},
	{0x0439, _T("Hindi")},
	{0x040e, _T("Hungarian")},
	{0x040f, _T("Icelandic")},
	{0x0421, _T("Indonesian")},
	{0x0410, _T("Italian (Standard)")},
	{0x0810, _T("Italian (Switzerland)")},
	{0x0411, _T("Japanese")},
	{0x044b, _T("Kannada")},
	{0x0457, _T("Konkani")},
	{0x0412, _T("Korean")},
	{0x0812, _T("Korean (Johab)")},
	{0x0440, _T("Kyrgyz")},
	{0x0426, _T("Latvian")},
	{0x0427, _T("Lithuanian")},
	{0x0827, _T("Lithuanian (Classic)")},
	{0x042f, _T("FYRO Macedonian")},
	{0x043e, _T("Malay (Malaysian)")},
	{0x083e, _T("Malay (Brunei Darussalam)")},
	{0x044e, _T("Marathi")},
	{0x0450, _T("Mongolian")},
	{0x0414, _T("Norwegian (Bokmal)")},
	{0x0814, _T("Norwegian (Nynorsk)")},
	{0x0415, _T("Polish")},
	{0x0416, _T("Portuguese (Brazil)")},
	{0x0816, _T("Portuguese (Portugal)")},
	{0x0446, _T("Punjabi")},
	{0x0418, _T("Romanian")},
	{0x0419, _T("Russian")},
	{0x044f, _T("Sanskrit")},
	{0x0c1a, _T("Serbian (Cyrillic)")},
	{0x081a, _T("Serbian (Latin)")},
	{0x041b, _T("Slovak")},
	{0x0424, _T("Slovenian")},
	{0x040a, _T("Spanish (Spain, Traditional Sort)")},
	{0x080a, _T("Spanish (Mexican)")},
	{0x0c0a, _T("Spanish (Spain, International Sort)")},
	{0x100a, _T("Spanish (Guatemala)")},
	{0x140a, _T("Spanish (Costa Rica)")},
	{0x180a, _T("Spanish (Panama)")},
	{0x1c0a, _T("Spanish (Dominican Republic)")},
	{0x200a, _T("Spanish (Venezuela)")},
	{0x240a, _T("Spanish (Colombia)")},
	{0x280a, _T("Spanish (Peru)")},
	{0x2c0a, _T("Spanish (Argentina)")},
	{0x300a, _T("Spanish (Ecuador)")},
	{0x340a, _T("Spanish (Chile)")},
	{0x380a, _T("Spanish (Uruguay)")},
	{0x3c0a, _T("Spanish (Paraguay)")},
	{0x400a, _T("Spanish (Bolivia)")},
	{0x440a, _T("Spanish (El Salvador)")},
	{0x480a, _T("Spanish (Honduras)")},
	{0x4c0a, _T("Spanish (Nicaragua)")},
	{0x500a, _T("Spanish (Puerto Rico)")},
	{0x0430, _T("Sutu")},
	{0x0441, _T("Swahili (Kenya)")},
	{0x041d, _T("Swedish")},
	{0x081d, _T("Swedish (Finland)")},
	{0x045a, _T("Syriac")},
	{0x0449, _T("Tamil")},
	{0x0444, _T("Tatar (Tatarstan)")},
	{0x044a, _T("Telugu")},
	{0x041e, _T("Thai")},
	{0x041f, _T("Turkish")},
	{0x0422, _T("Ukrainian")},
	{0x0420, _T("Urdu (Pakistan)")},
	{0x0820, _T("Urdu (India)")},
	{0x0443, _T("Uzbek (Latin)")},
	{0x0843, _T("Uzbek (Cyrillic)")},
	{0x042a, _T("Vietnamese")}
};


// CPPageDVD dialog

IMPLEMENT_DYNAMIC(CPPageDVD, CPPageBase)
CPPageDVD::CPPageDVD()
	: CPPageBase(CPPageDVD::IDD, CPPageDVD::IDD)
	, m_iDVDLocation(0)
	, m_iDVDLangType(0)
	, m_dvdpath(_T(""))
	, m_fAutoSpeakerConf(FALSE)
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
	DDX_Check(pDX, IDC_CHECK1, m_fAutoSpeakerConf);
}

void CPPageDVD::UpdateLCIDList()
{
	UpdateData();

	LCID lcid = m_iDVDLangType == 0 ? m_idMenuLang
		: m_iDVDLangType == 1 ? m_idAudioLang
		: m_idSubtitlesLang;

	for(int i = 0; i < m_lcids.GetCount(); i++)
	{
		if(m_lcids.GetItemData(i) == lcid)
		{
			m_lcids.SetCurSel(i);
			m_lcids.SetTopIndex(i);
			break;
		}
	}
}

BEGIN_MESSAGE_MAP(CPPageDVD, CPPageBase)
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

	AppSettings& s = AfxGetAppSettings();

	m_iDVDLocation = s.fUseDVDPath ? 1 : 0;
	m_dvdpath = s.sDVDPath;
	m_iDVDLangType = 0;

	m_idMenuLang = s.idMenuLang;
	m_idAudioLang = s.idAudioLang;
	m_idSubtitlesLang = s.idSubtitlesLang;
	m_fAutoSpeakerConf = s.fAutoSpeakerConf;

	UpdateData(FALSE);

	for(int i = 0; i < countof(LCIDNameList); i++)
	{
		m_lcids.AddString(LCIDNameList[i].name);
		m_lcids.SetItemData(i, LCIDNameList[i].lcid);
	}

	UpdateLCIDList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageDVD::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.sDVDPath = m_dvdpath;
	s.fUseDVDPath = (m_iDVDLocation == 1);
	s.idMenuLang = m_idMenuLang;
	s.idAudioLang = m_idAudioLang;
	s.idSubtitlesLang = m_idSubtitlesLang;
	s.fAutoSpeakerConf = !!m_fAutoSpeakerConf;

	return __super::OnApply();
}

void CPPageDVD::OnBnClickedButton1()
{
	TCHAR path[MAX_PATH];

	BROWSEINFO bi;
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = path;
	bi.lpszTitle = _T("Select the path for the DVD:");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0; 

	LPITEMIDLIST iil;
	if(iil = SHBrowseForFolder(&bi))
	{
		SHGetPathFromIDList(iil, path);
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

	lcid = m_lcids.GetItemData(m_lcids.GetCurSel());

	SetModified();
}

void CPPageDVD::OnUpdateDVDPath(CCmdUI* pCmdUI)
{
	UpdateData();

	pCmdUI->Enable(m_iDVDLocation == 1);
}
