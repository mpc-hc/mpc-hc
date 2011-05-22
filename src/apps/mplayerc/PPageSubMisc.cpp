/*
 * $Id$
 *
 * (C) 2006-2011 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
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
#include "PPageSubMisc.h"
#include "ISDb.h"

// CPPageSubMisc dialog

IMPLEMENT_DYNAMIC(CPPageSubMisc, CPPageBase)

CPPageSubMisc::CPPageSubMisc()
	: CPPageBase(CPPageSubMisc::IDD, CPPageSubMisc::IDD)
	, m_fPrioritizeExternalSubtitles(FALSE)
	, m_fDisableInternalSubtitles(FALSE)
	, m_szAutoloadPaths("")
	, m_ISDb(_T(""))
{

}

CPPageSubMisc::~CPPageSubMisc()
{
}

void CPPageSubMisc::DoDataExchange(CDataExchange* pDX)
{
	CPPageBase::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, m_fPrioritizeExternalSubtitles);
	DDX_Check(pDX, IDC_CHECK2, m_fDisableInternalSubtitles);
	DDX_Text(pDX, IDC_EDIT1, m_szAutoloadPaths);
	DDX_Control(pDX, IDC_COMBO1, m_ISDbCombo);
	DDX_CBString(pDX, IDC_COMBO1, m_ISDb);
}

BOOL CPPageSubMisc::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	m_fPrioritizeExternalSubtitles = s.fPrioritizeExternalSubtitles;
	m_fDisableInternalSubtitles = s.fDisableInternalSubtitles;
	m_szAutoloadPaths = s.strSubtitlePaths;

	m_ISDb = s.strISDb;
	m_ISDbCombo.AddString(m_ISDb);
	if (m_ISDb.CompareNoCase(_T("www.opensubtitles.org/isdb"))) {
		m_ISDbCombo.AddString(_T("www.opensubtitles.org/isdb"));
	}

	UpdateData(FALSE);

	return TRUE;
}

BOOL CPPageSubMisc::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.fPrioritizeExternalSubtitles = m_fPrioritizeExternalSubtitles;
	s.fDisableInternalSubtitles = m_fDisableInternalSubtitles;
	s.strSubtitlePaths = m_szAutoloadPaths;

	s.strISDb = m_ISDb;
	s.strISDb.TrimRight('/');

	return __super::OnApply();
}


BEGIN_MESSAGE_MAP(CPPageSubMisc, CPPageBase)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateButton2)
END_MESSAGE_MAP()

void CPPageSubMisc::OnBnClickedButton1()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();
	m_szAutoloadPaths = s.strSubtitlePaths;

	UpdateData(FALSE);
}

void CPPageSubMisc::OnBnClickedButton2()
{
	CString ISDb, ver, msg, str;

	m_ISDbCombo.GetWindowText(ISDb);
	ISDb.TrimRight('/');

	ver.Format(_T("ISDb v%d"), ISDb_PROTOCOL_VERSION);

	CWebTextFile wtf;
	if (wtf.Open(_T("http://") + ISDb + _T("/test.php")) && wtf.ReadString(str) && str == ver) {
		msg = ResStr(IDS_PPSDB_URLCORRECT);
	} else if (str.Find(_T("ISDb v")) == 0) {
		msg = ResStr(IDS_PPSDB_PROTOCOLERR);
	} else {
		msg = ResStr(IDS_PPSDB_BADURL);
	}

	AfxMessageBox(msg, MB_OK);
}

void CPPageSubMisc::OnUpdateButton2(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ISDbCombo.GetWindowTextLength() > 0);
}
