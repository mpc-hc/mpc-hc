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

// PPageSheet.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PPageSheet.h"

// CPPageSheet

IMPLEMENT_DYNAMIC(CPPageSheet, CTreePropSheet)

CPPageSheet::CPPageSheet(LPCTSTR pszCaption, IFilterGraph* pFG, CWnd* pParentWnd, UINT idPage)
	: CTreePropSheet(pszCaption, pParentWnd, 0)
	, m_audioswitcher(pFG)
{
	AddPage(&m_player);
	AddPage(&m_formats);
	AddPage(&m_acceltbl);
	AddPage(&m_logo);
	AddPage(&m_playback);
	AddPage(&m_dvd);
	AddPage(&m_output);
	AddPage(&m_webserver);
	AddPage(&m_internalfilters);
	AddPage(&m_audioswitcher);
	AddPage(&m_externalfilters);
	AddPage(&m_subtitles);
	AddPage(&m_substyle);
	AddPage(&m_subdb);
	AddPage(&m_tweaks);
	AddPage(&m_casimir);

	EnableStackedTabs(FALSE);

	SetTreeViewMode(TRUE, TRUE, FALSE);

	if(idPage || (idPage = AfxGetApp()->GetProfileInt(ResStr(IDS_R_SETTINGS), _T("LastUsedPage"), 0)))
	{
		for(int i = 0; i < GetPageCount(); i++)
		{
			if(GetPage(i)->m_pPSP->pszTemplate == MAKEINTRESOURCE(idPage))
			{
				SetActivePage(i);
				break;
			}
		}
	}
}

CPPageSheet::~CPPageSheet()
{
}

CTreeCtrl* CPPageSheet::CreatePageTreeObject()
{
	return new CTreePropSheetTreeCtrl();
}

BEGIN_MESSAGE_MAP(CPPageSheet, CTreePropSheet)
END_MESSAGE_MAP()

BOOL CPPageSheet::OnInitDialog()
{
	BOOL bResult = __super::OnInitDialog();

	if(CTreeCtrl* pTree = GetPageTreeControl())
	{
		for(HTREEITEM node = pTree->GetRootItem(); node; node = pTree->GetNextSiblingItem(node))
			pTree->Expand(node, TVE_EXPAND);
	}

	return bResult;
}

// CTreePropSheetTreeCtrl

IMPLEMENT_DYNAMIC(CTreePropSheetTreeCtrl, CTreeCtrl)
CTreePropSheetTreeCtrl::CTreePropSheetTreeCtrl()
{
}

CTreePropSheetTreeCtrl::~CTreePropSheetTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CTreePropSheetTreeCtrl, CTreeCtrl)
END_MESSAGE_MAP()

// CTreePropSheetTreeCtrl message handlers


BOOL CTreePropSheetTreeCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.dwExStyle |= WS_EX_CLIENTEDGE;
//	cs.style &= ~TVS_LINESATROOT;

	return __super::PreCreateWindow(cs);
}

