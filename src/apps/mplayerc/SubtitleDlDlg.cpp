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
#include "SubtitleDlDlg.h"


// CSubtitleDlDlg dialog

//IMPLEMENT_DYNAMIC(CSubtitleDlDlg, CResizableDialog)
CSubtitleDlDlg::CSubtitleDlDlg(CList<isdb_movie>& movies, CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSubtitleDlDlg::IDD, pParent)
{
	m_movies.AddTail(&movies);
}

CSubtitleDlDlg::~CSubtitleDlDlg()
{
}

int CSubtitleDlDlg::GetChecked(int iItem)
{
	LVITEM lvi;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	m_list.GetItem(&lvi);
	return(lvi.iImage);
}

void CSubtitleDlDlg::SetChecked(int iItem, int iChecked)
{
	LVITEM lvi;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = iChecked;
	m_list.SetItem(&lvi);
}

void CSubtitleDlDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CSubtitleDlDlg, CResizableDialog)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnNMClickList1)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
END_MESSAGE_MAP()

// CSubtitleDlDlg message handlers

BOOL CSubtitleDlDlg::OnInitDialog()
{
	__super::OnInitDialog();


	AddAnchor(IDC_LIST1, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_CHECK1, BOTTOM_LEFT);
	AddAnchor(IDOK, BOTTOM_RIGHT);

	CSize s(200, 150);
	SetMinTrackSize(s);


	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	m_list.InsertColumn(COL_FILENAME, _T("File"), LVCFMT_LEFT, 160);
	m_list.InsertColumn(COL_LANGUAGE, _T("Language"), LVCFMT_CENTER, 80);
	m_list.InsertColumn(COL_FORMAT, _T("Format"), LVCFMT_CENTER, 50);
	m_list.InsertColumn(COL_DISC, _T("Disc"), LVCFMT_CENTER, 50);
	m_list.InsertColumn(COL_TITLES, _T("Title(s)"), LVCFMT_LEFT, 300);

	m_onoff.Create(IDB_ONOFF, 12, 3, 0xffffff);
	m_list.SetImageList(&m_onoff, LVSIL_SMALL);


	int i = 0;

	POSITION pos = m_movies.GetHeadPosition();
	while(pos) {
		isdb_movie& m = m_movies.GetNext(pos);

		CStringA titlesA = Implode(m.titles, '|');
		titlesA.Replace("|", ", ");
		CString titles = UTF8To16(titlesA);

		POSITION pos2 = m.subs.GetHeadPosition();
		while(pos2) {
			isdb_subtitle& s = m.subs.GetNext(pos2);
			CString name = UTF8To16(s.name);
			CString language = s.language;
			CString format = s.format;
			CString disc;
			disc.Format(_T("%d/%d"), s.disc_no, s.discs);

			int iItem = m_list.InsertItem(i++, _T(""));
			m_list.SetItemData(iItem, (DWORD_PTR)&s);
			m_list.SetItemText(iItem, COL_FILENAME, name);
			m_list.SetItemText(iItem, COL_LANGUAGE, language);
			m_list.SetItemText(iItem, COL_FORMAT, format);
			m_list.SetItemText(iItem, COL_DISC, disc);
			m_list.SetItemText(iItem, COL_TITLES, titles);
		}
	}

	m_selsubs.RemoveAll();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSubtitleDlDlg::OnOK()
{
	for(int i = 0; i < m_list.GetItemCount(); i++)
		if(GetChecked(i)) {
			m_selsubs.AddTail(*(isdb_subtitle*)m_list.GetItemData(i));
		}

	m_fReplaceSubs = IsDlgButtonChecked(IDC_CHECK1) == BST_CHECKED;

	__super::OnOK();
}

void CSubtitleDlDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

	if(lpnmlv->iItem >= 0) {
		CRect r;
		m_list.GetItemRect(lpnmlv->iItem, r, LVIR_ICON);
		if(r.PtInRect(lpnmlv->ptAction)) {
			SetChecked(lpnmlv->iItem, (GetChecked(lpnmlv->iItem)&1) == 0 ? 1 : 0);
		}
	}

	*pResult = 0;
}

void CSubtitleDlDlg::OnUpdateOk(CCmdUI* pCmdUI)
{
	bool fEnable = false;
	for(int i = 0; !fEnable && i < m_list.GetItemCount(); i++) {
		fEnable = !!GetChecked(i);
	}

	pCmdUI->Enable(fEnable);
}
