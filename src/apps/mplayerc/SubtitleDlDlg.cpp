/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
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
#include "SubtitleDlDlg.h"
#include <algorithm> // For std::sort


// CSubtitleDlDlg dialog

//IMPLEMENT_DYNAMIC(CSubtitleDlDlg, CResizableDialog)
CSubtitleDlDlg::CSubtitleDlDlg(CList<isdb_movie>& movies, CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSubtitleDlDlg::IDD, pParent),
	iColumn(-1),
	bSortDirection(false)
{
	m_movies.AddTail(&movies);

	// Parse
	POSITION pos = m_movies.GetHeadPosition();
	while (pos)
	{
		isdb_movie& m = m_movies.GetNext(pos);
		isdb_movie_Parsed p;

		CStringA titlesA = Implode(m.titles, '|');
		titlesA.Replace("|", ", ");
		p.titles = UTF8To16(titlesA);
		p.checked = false;

		POSITION pos2 = m.subs.GetHeadPosition();
		while (pos2) {
			isdb_subtitle& s = m.subs.GetNext(pos2);
			p.name = UTF8To16(s.name);
			p.language = s.language;
			p.format = s.format;
			p.disc.Format(_T("%d/%d"), s.disc_no, s.discs);
			p.ptr = (DWORD_PTR)&s;

			m_moviesParsed.Add(p);
		}
	}
}

CSubtitleDlDlg::~CSubtitleDlDlg()
{
}

void CSubtitleDlDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CSubtitleDlDlg, CResizableDialog)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CSubtitleDlDlg::OnHdnItemclickList1)
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

	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_list.InsertColumn(COL_FILENAME, _T("File"), LVCFMT_LEFT, 160);
	m_list.InsertColumn(COL_LANGUAGE, _T("Language"), LVCFMT_CENTER, 80);
	m_list.InsertColumn(COL_FORMAT, _T("Format"), LVCFMT_CENTER, 50);
	m_list.InsertColumn(COL_DISC, _T("Disc"), LVCFMT_CENTER, 50);
	m_list.InsertColumn(COL_TITLES, _T("Title(s)"), LVCFMT_LEFT, 300);

	BuildList();

	m_selsubs.RemoveAll();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSubtitleDlDlg::OnOK()
{
	for (int i = 0; i < m_list.GetItemCount(); ++i)
	{
		if (m_list.GetCheck(i)) {
			m_selsubs.AddTail(*(isdb_subtitle*)m_list.GetItemData(i));
		}
	}

	m_fReplaceSubs = IsDlgButtonChecked(IDC_CHECK1) == BST_CHECKED;

	__super::OnOK();
}

void CSubtitleDlDlg::OnUpdateOk(CCmdUI* pCmdUI)
{
	bool fEnable = false;
	for (int i = 0; !fEnable && i < m_list.GetItemCount(); ++i) {
		fEnable = !!m_list.GetCheck(i);
	}

	pCmdUI->Enable(fEnable);
}


void CSubtitleDlDlg::OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;
	
	if ( phdr->iItem != iColumn )
	{
		iColumn = phdr->iItem;
		bSortDirection = false;
	}
	else
	{
		bSortDirection = !bSortDirection;
	}

	SortList();
}

void CSubtitleDlDlg::BuildList( void )
{
	for (INT_PTR i = 0; i < m_moviesParsed.GetCount(); ++i)
	{
		isdb_movie_Parsed& m = m_moviesParsed[i];

		int iItem = m_list.InsertItem(i, _T(""));
		m_list.SetItemData(iItem, (DWORD_PTR)m.ptr);
		m_list.SetItemText(iItem, COL_FILENAME, m.name);
		m_list.SetItemText(iItem, COL_LANGUAGE, m.language);
		m_list.SetItemText(iItem, COL_FORMAT, m.format);
		m_list.SetItemText(iItem, COL_DISC, m.disc);
		m_list.SetItemText(iItem, COL_TITLES, m.titles);
		m_list.SetCheck(iItem, m.checked);
	}
}

struct sort_cmp {
	sort_cmp(int col, bool dir) : iColumn(col), bSortDirection(dir) {}

	bool operator()(const CSubtitleDlDlg::isdb_movie_Parsed& a, const CSubtitleDlDlg::isdb_movie_Parsed& b) const
	{
		bool result = false;

		// Should this macro be a function instead ?
		#define dir_cmp(l, r) bSortDirection ? (r) < (l) : (l) < (r)
		
		switch (iColumn)
		{
		case CSubtitleDlDlg::COL_FILENAME:
			result = dir_cmp(a.name, b.name);
			break;
		case CSubtitleDlDlg::COL_LANGUAGE:
			result = dir_cmp(a.language, b.language);
			break;
		case CSubtitleDlDlg::COL_FORMAT:
			result = dir_cmp(a.format, b.format);
			break;
		case CSubtitleDlDlg::COL_DISC:
			result = dir_cmp(a.disc, b.disc);
			break;
		case CSubtitleDlDlg::COL_TITLES:
			result = dir_cmp(a.titles, b.titles);
			break;
		}
		
		#undef dir_cmp
		
		return result;
	}

	const int iColumn;
	const bool bSortDirection;
};

void CSubtitleDlDlg::SortList( void )
{
	// Save checked state
	for (INT_PTR i = 0; i < m_moviesParsed.GetCount(); ++i)
	{
		m_moviesParsed[i].checked = m_list.GetCheck(i);
	}

	// Sort list
	// qsort doesn't support functors
	//qsort(m_moviesParsed.GetData(), m_moviesParsed.GetCount(), sizeof(m_moviesParsed[0]), sort_cmp_c(iColumn, bSortDirection));
	std::sort(m_moviesParsed.GetData(), m_moviesParsed.GetData() + m_moviesParsed.GetCount(), sort_cmp(iColumn, bSortDirection));

	m_list.DeleteAllItems();

	BuildList();
}
