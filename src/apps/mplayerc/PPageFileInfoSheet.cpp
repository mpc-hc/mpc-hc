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

// PPageFileInfoSheet.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include "PPageFileInfoSheet.h"

// CPPageFileInfoSheet

IMPLEMENT_DYNAMIC(CPPageFileInfoSheet, CPropertySheet)
CPPageFileInfoSheet::CPPageFileInfoSheet(CString fn, CMainFrame* pParentWnd)
	: CPropertySheet(ResStr(IDS_PROPSHEET_PROPERTIES), pParentWnd, 0)
	, m_clip(fn, pParentWnd->pGB)
	, m_details(fn, pParentWnd->pGB, pParentWnd->m_pCAP)
	, m_res(fn, pParentWnd->pGB)
{
	AddPage(&m_clip);
	AddPage(&m_details);

	BeginEnumFilters(pParentWnd->pGB, pEF, pBF)
	{
		if(CComQIPtr<IDSMResourceBag> pRB = pBF)
		if(pRB && pRB->ResGetCount() > 0)
		{
			AddPage(&m_res);
			break;
		}
	}
	EndEnumFilters
}

CPPageFileInfoSheet::~CPPageFileInfoSheet()
{
}


BEGIN_MESSAGE_MAP(CPPageFileInfoSheet, CPropertySheet)
END_MESSAGE_MAP()

// CPPageFileInfoSheet message handlers

BOOL CPPageFileInfoSheet::OnInitDialog()
{
	BOOL fRet = __super::OnInitDialog();
	
	GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_HIDE);
	GetDlgItem(IDOK)->SetWindowText(_T("Close"));

	CRect r;
	GetDlgItem(ID_APPLY_NOW)->GetWindowRect(&r);
	ScreenToClient(r);
	GetDlgItem(IDOK)->MoveWindow(r);

	return fRet;
}

