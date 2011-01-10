/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
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
#include "OpenDlg.h"
#include "OpenFileDlg.h"


// COpenDlg dialog

//IMPLEMENT_DYNAMIC(COpenDlg, CResizableDialog)
COpenDlg::COpenDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(COpenDlg::IDD, pParent)
	, m_path(_T(""))
	, m_path2(_T(""))
	, m_fMultipleFiles(false)
	, m_fAppendPlaylist(FALSE)
{
}

COpenDlg::~COpenDlg()
{
}

void COpenDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_mrucombo);
	DDX_CBString(pDX, IDC_COMBO1, m_path);
	DDX_Control(pDX, IDC_COMBO2, m_mrucombo2);
	DDX_CBString(pDX, IDC_COMBO2, m_path2);
	DDX_Control(pDX, IDC_STATIC1, m_label2);
	DDX_Check(pDX, IDC_CHECK1, m_fAppendPlaylist);
}


BEGIN_MESSAGE_MAP(COpenDlg, CResizableDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedBrowsebutton)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedBrowsebutton2)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdateDub)
	ON_UPDATE_COMMAND_UI(IDC_COMBO2, OnUpdateDub)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateDub)
END_MESSAGE_MAP()


// COpenDlg message handlers

BOOL COpenDlg::OnInitDialog()
{
	__super::OnInitDialog();

	CRecentFileList& MRU = AfxGetAppSettings().MRU;
	MRU.ReadList();
	m_mrucombo.ResetContent();
	for(int i = 0; i < MRU.GetSize(); i++)
		if(!MRU[i].IsEmpty()) {
			m_mrucombo.AddString(MRU[i]);
		}
	CorrectComboListWidth(m_mrucombo, GetFont());

	CRecentFileList& MRUDub = AfxGetAppSettings().MRUDub;
	MRUDub.ReadList();
	m_mrucombo2.ResetContent();
	for(int i = 0; i < MRUDub.GetSize(); i++)
		if(!MRUDub[i].IsEmpty()) {
			m_mrucombo2.AddString(MRUDub[i]);
		}
	CorrectComboListWidth(m_mrucombo2, GetFont());

	if(m_mrucombo.GetCount() > 0) {
		m_mrucombo.SetCurSel(0);
	}

	AddAnchor(m_mrucombo, TOP_LEFT, TOP_RIGHT);
	AddAnchor(m_mrucombo2, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_BUTTON1, TOP_RIGHT);
	AddAnchor(IDC_BUTTON2, TOP_RIGHT);
	AddAnchor(IDOK, TOP_RIGHT);
	AddAnchor(IDCANCEL, TOP_RIGHT);
	AddAnchor(IDC_STATIC1, TOP_LEFT, TOP_RIGHT);

	CRect r;
	GetWindowRect(r);
	CSize s = r.Size();
	SetMinTrackSize(s);
	s.cx = 1000;
	SetMaxTrackSize(s);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

static CString GetFileName(CString str)
{
	CPath p = str;
	p.StripPath();
	return (LPCTSTR)p;
}

void COpenDlg::OnBnClickedBrowsebutton()
{
	UpdateData();

	CString filter;
	CAtlArray<CString> mask;
	AfxGetAppSettings().m_Formats.GetFilter(filter, mask);

	COpenFileDlg fd(mask, true, NULL, m_path,
					OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT|OFN_ENABLEINCLUDENOTIFY,
					filter, this);
	if(fd.DoModal() != IDOK) {
		return;
	}

	m_fns.RemoveAll();

	POSITION pos = fd.GetStartPosition();
	while(pos) {
		/*
				CString str = fd.GetNextPathName(pos);
				POSITION insertpos = m_fns.GetTailPosition();
				while(insertpos && GetFileName(str).CompareNoCase(GetFileName(m_fns.GetAt(insertpos))) <= 0)
					m_fns.GetPrev(insertpos);
				if(!insertpos) m_fns.AddHead(str);
				else m_fns.InsertAfter(insertpos, str);
		*/
		m_fns.AddTail(fd.GetNextPathName(pos));
	}

	if(m_fns.GetCount() > 1
			|| m_fns.GetCount() == 1
			&& (m_fns.GetHead()[m_fns.GetHead().GetLength()-1] == '\\'
				|| m_fns.GetHead()[m_fns.GetHead().GetLength()-1] == '*')) {
		m_fMultipleFiles = true;
		EndDialog(IDOK);
		return;
	}

	m_mrucombo.SetWindowText(fd.GetPathName());
}

void COpenDlg::OnBnClickedBrowsebutton2()
{
	UpdateData();

	CString filter;
	CAtlArray<CString> mask;
	AfxGetAppSettings().m_Formats.GetAudioFilter(filter, mask);

	COpenFileDlg fd(mask, false, NULL, m_path2,
					OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_ENABLEINCLUDENOTIFY,
					filter, this);

	if(fd.DoModal() != IDOK) {
		return;
	}

	m_mrucombo2.SetWindowText(fd.GetPathName());
}

void COpenDlg::OnBnClickedOk()
{
	UpdateData();

	m_fns.RemoveAll();
	m_fns.AddTail(m_path);
	if(m_mrucombo2.IsWindowEnabled()) {
		m_fns.AddTail(m_path2);
	}

	m_fMultipleFiles = false;

	OnOK();
}

void COpenDlg::OnUpdateDub(CCmdUI* pCmdUI)
{
	m_mrucombo.GetWindowText(m_path);
	pCmdUI->Enable(AfxGetAppSettings().m_Formats.GetEngine(m_path) == DirectShow);
}
