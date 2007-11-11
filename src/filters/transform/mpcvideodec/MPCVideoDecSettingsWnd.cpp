/* 
 * $Id: MPCVideoDecSettingsWnd.cpp 249 2007-09-26 11:07:22Z casimir666 $
 *
 * (C) 2006-2007 see AUTHORS
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
#include "MPCVideoDecSettingsWnd.h"
#include "..\..\..\dsutil\dsutil.h"


// ==>>> Resource identifier from "resource.h" present in mplayerc project!
#define ResStr(id) CString(MAKEINTRESOURCE(id))

#define IDB_ONOFF                       205
#define IDC_LIST_FORMAT                 11160
//

//
// CMPCVideoDecSettingsWnd
//

CMPCVideoDecSettingsWnd::CMPCVideoDecSettingsWnd()
{
}

bool CMPCVideoDecSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
	ASSERT(!m_pMDF);

	m_pMDF.Release();

	POSITION pos = pUnks.GetHeadPosition();
	while(pos && !(m_pMDF = pUnks.GetNext(pos)));
	
	if(!m_pMDF) return false;


	return true;
}

void CMPCVideoDecSettingsWnd::OnDisconnect()
{
	m_pMDF.Release();
}

bool CMPCVideoDecSettingsWnd::OnActivate()
{
	DWORD dwStyle = WS_VISIBLE|WS_CHILD|WS_BORDER|LVS_REPORT;

	CPoint p(10, 10);



	m_lvFormats.Create (dwStyle, CRect (p, CSize (320, 240)), this, IDC_LIST_FORMAT);
	m_onoff.Create(IDB_ONOFF, 12, 3, 0xffffff);
	m_lvFormats.SetImageList(&m_onoff, LVSIL_SMALL);


	m_lvFormats.SetExtendedStyle(m_lvFormats.GetExtendedStyle()|LVS_EX_FULLROWSELECT);
	m_lvFormats.InsertColumn(0, _T("Format"), LVCFMT_LEFT, 300);

	m_lvFormats.InsertItem(0, _T("WM9"));
	m_lvFormats.InsertItem(1, _T("H264"));

/*	m_note_static.Create(
		ResStr(IDS_MPEG2DECSETTINGSWND_7) +
		ResStr(IDS_MPEG2DECSETTINGSWND_8),
		dwStyle, CRect(p, CSize(320, m_fontheight * 3)), this);*/

	for(CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow())
		pWnd->SetFont(&m_font, FALSE);

	return true;
}

void CMPCVideoDecSettingsWnd::OnDeactivate()
{
}

bool CMPCVideoDecSettingsWnd::OnApply()
{
	OnDeactivate();

	if(m_pMDF)
	{
	}

	return true;
}

void CMPCVideoDecSettingsWnd::OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

	if(lpnmlv->iItem >= 0 /*&& lpnmlv->iSubItem == COL_CATEGORY*/)
	{
		CRect r;
		m_lvFormats.GetItemRect(lpnmlv->iItem, r, LVIR_ICON);
		if(r.PtInRect(lpnmlv->ptAction))
		{
			SetChecked(lpnmlv->iItem, (GetChecked(lpnmlv->iItem)&1) == 0 ? 1 : 0);
			SetDirty(TRUE);
		}
	}
}

void CMPCVideoDecSettingsWnd::SetChecked(int iItem, int iChecked)
{
	LVITEM lvi;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = iChecked;
	m_lvFormats.SetItem(&lvi);
}

int CMPCVideoDecSettingsWnd::GetChecked(int iItem)
{
	LVITEM lvi;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	m_lvFormats.GetItem(&lvi);
	return(lvi.iImage);
}

BEGIN_MESSAGE_MAP(CMPCVideoDecSettingsWnd, CInternalPropertyPageWnd)
	ON_NOTIFY(NM_CLICK, IDC_LIST_FORMAT, OnNMClickList1)
END_MESSAGE_MAP()
