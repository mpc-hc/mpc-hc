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
#include "FavoriteOrganizeDlg.h"


// CFavoriteOrganizeDlg dialog

//IMPLEMENT_DYNAMIC(CFavoriteOrganizeDlg, CResizableDialog)
CFavoriteOrganizeDlg::CFavoriteOrganizeDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CFavoriteOrganizeDlg::IDD, pParent)
{
}

CFavoriteOrganizeDlg::~CFavoriteOrganizeDlg()
{
}

void CFavoriteOrganizeDlg::SetupList(bool fSave)
{
	int i = m_tab.GetCurSel();

	if (fSave) {
		CAtlList<CString> sl;

		for (int j = 0; j < m_list.GetItemCount(); j++) {
			CString desc = m_list.GetItemText(j, 0);
			desc.Remove(';');
			CString str = m_sl[i].GetAt((POSITION)m_list.GetItemData(j));
			sl.AddTail(desc + str.Mid(str.Find(';')));
		}

		m_sl[i].RemoveAll();
		m_sl[i].AddTailList(&sl);
	} else {
		m_list.DeleteAllItems();

		POSITION pos = m_sl[i].GetHeadPosition(), tmp;
		while (pos) {
			tmp = pos;

			CAtlList<CString> sl;
			Explode(m_sl[i].GetNext(pos), sl, ';', 3);

			int n = m_list.InsertItem(m_list.GetItemCount(), sl.RemoveHead());
			m_list.SetItemData(n, (DWORD_PTR)tmp);

			if (!sl.IsEmpty()) {
				REFERENCE_TIME rt = 0;
				if (1 == _stscanf_s(sl.GetHead(), _T("%I64d"), &rt) && rt > 0) {
					DVD_HMSF_TIMECODE hmsf = RT2HMSF(rt);

					CString str;
					str.Format(_T("[%02d:%02d:%02d]"), hmsf.bHours, hmsf.bMinutes, hmsf.bSeconds);
					m_list.SetItemText(n, 1, str);
				}
			}
		}

		UpdateColumnsSizes();
	}
}

void CFavoriteOrganizeDlg::UpdateColumnsSizes()
{
	CRect r;
	m_list.GetClientRect(r);
	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);
	m_list.SetColumnWidth(1, max(m_list.GetColumnWidth(1), r.Width() - m_list.GetColumnWidth(0)));
}

void CFavoriteOrganizeDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_LIST2, m_list);
}


BEGIN_MESSAGE_MAP(CFavoriteOrganizeDlg, CResizableDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnTcnSelchangeTab1)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton7)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB1, OnTcnSelchangingTab1)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_ACTIVATE()
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST2, OnLvnEndlabeleditList2)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CFavoriteOrganizeDlg message handlers

BOOL CFavoriteOrganizeDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_tab.InsertItem(0, ResStr(IDS_FAVFILES));
	m_tab.InsertItem(1, ResStr(IDS_FAVDVDS));
	//	m_tab.InsertItem(2, ResStr(IDS_FAVDEVICES));
	m_tab.SetCurSel(0);

	m_list.InsertColumn(0, _T(""));
	m_list.InsertColumn(1, _T(""));
	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	AfxGetAppSettings().GetFav(FAV_FILE, m_sl[0]);
	AfxGetAppSettings().GetFav(FAV_DVD, m_sl[1]);
	AfxGetAppSettings().GetFav(FAV_DEVICE, m_sl[2]);

	SetupList(false);

	AddAnchor(IDC_TAB1, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_LIST2, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_BUTTON1, TOP_RIGHT);
	AddAnchor(IDC_BUTTON2, TOP_RIGHT);
	AddAnchor(IDC_BUTTON3, TOP_RIGHT);
	AddAnchor(IDC_BUTTON4, TOP_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFavoriteOrganizeDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetupList(false);

	m_list.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	*pResult = 0;
}

void CFavoriteOrganizeDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (nIDCtl != IDC_LIST2) {
		return;
	}

	int nItem = lpDrawItemStruct->itemID;
	CRect rcItem = lpDrawItemStruct->rcItem;

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	if (!!m_list.GetItemState(nItem, LVIS_SELECTED)) {
		FillRect(pDC->m_hDC, rcItem, CBrush(0xf1dacc));
		FrameRect(pDC->m_hDC, rcItem, CBrush(0xc56a31));
	} else {
		CBrush b;
		b.CreateSysColorBrush(COLOR_WINDOW);
		FillRect(pDC->m_hDC, rcItem, b);
	}

	CString str;
	pDC->SetTextColor(0);

	str = m_list.GetItemText(nItem, 0);
	pDC->TextOut(rcItem.left + 3, (rcItem.top+rcItem.bottom - pDC->GetTextExtent(str).cy) / 2, str);
	str = m_list.GetItemText(nItem, 1);
	if (!str.IsEmpty()) {
		pDC->TextOut(rcItem.right - pDC->GetTextExtent(str).cx - 3, (rcItem.top+rcItem.bottom - pDC->GetTextExtent(str).cy) / 2, str);
	}
}

void CFavoriteOrganizeDlg::OnBnClickedButton1()
{
	if (POSITION pos = m_list.GetFirstSelectedItemPosition()) {
		m_list.SetFocus();
		m_list.EditLabel(m_list.GetNextSelectedItem(pos));
	}
}

void CFavoriteOrganizeDlg::OnLvnEndlabeleditList2(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (pDispInfo->item.iItem >= 0 && pDispInfo->item.pszText) {
		m_list.SetItemText(pDispInfo->item.iItem, 0, pDispInfo->item.pszText);
	}
	UpdateColumnsSizes();

	*pResult = 0;
}

void CFavoriteOrganizeDlg::OnBnClickedButton2()
{
	if (POSITION pos = m_list.GetFirstSelectedItemPosition()) {
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem < 0 || nItem >= m_list.GetItemCount()) {
			return;
		}

		m_list.DeleteItem(nItem);

		nItem = min(nItem, m_list.GetItemCount()-1);

		m_list.SetSelectionMark(nItem);
		m_list.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CFavoriteOrganizeDlg::OnBnClickedButton3()
{
	if (POSITION pos = m_list.GetFirstSelectedItemPosition()) {
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem <= 0) {
			return;
		}

		DWORD_PTR data = m_list.GetItemData(nItem);
		CString strName = m_list.GetItemText(nItem, 0);
		CString strPos = m_list.GetItemText(nItem, 1);

		m_list.DeleteItem(nItem);

		nItem--;

		m_list.InsertItem(nItem, strName);
		m_list.SetItemData(nItem, data);
		m_list.SetItemText(nItem, 1, strPos);
		m_list.SetSelectionMark(nItem);
		m_list.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CFavoriteOrganizeDlg::OnBnClickedButton7()
{
	if (POSITION pos = m_list.GetFirstSelectedItemPosition()) {
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem < 0 || nItem >= m_list.GetItemCount()-1) {
			return;
		}

		DWORD_PTR data = m_list.GetItemData(nItem);
		CString strName = m_list.GetItemText(nItem, 0);
		CString strPos = m_list.GetItemText(nItem, 1);

		m_list.DeleteItem(nItem);

		nItem++;

		m_list.InsertItem(nItem, strName);
		m_list.SetItemData(nItem, data);
		m_list.SetItemText(nItem, 1, strPos);
		m_list.SetSelectionMark(nItem);
		m_list.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CFavoriteOrganizeDlg::OnTcnSelchangingTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	SetupList(true);

	*pResult = 0;
}

void CFavoriteOrganizeDlg::OnBnClickedOk()
{
	SetupList(true);

	AfxGetAppSettings().SetFav(FAV_FILE, m_sl[0]);
	AfxGetAppSettings().SetFav(FAV_DVD, m_sl[1]);
	AfxGetAppSettings().SetFav(FAV_DEVICE, m_sl[2]);

	OnOK();
}

void CFavoriteOrganizeDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	__super::OnActivate(nState, pWndOther, bMinimized);

	if (nState == WA_ACTIVE) {
		m_list.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}
}

void CFavoriteOrganizeDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if (IsWindow(m_list)) {
		m_list.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	}
}
