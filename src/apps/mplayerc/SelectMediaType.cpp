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

// SelectMediaType.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SelectMediaType.h"
#include "..\..\DSUtil\DSUtil.h"


// CSelectMediaType dialog

IMPLEMENT_DYNAMIC(CSelectMediaType, CCmdUIDialog)
CSelectMediaType::CSelectMediaType(CAtlArray<GUID>& guids, GUID guid, CWnd* pParent /*=NULL*/)
	: CCmdUIDialog(CSelectMediaType::IDD, pParent)
	, m_guids(guids), m_guid(guid)
{
	m_guidstr = CStringFromGUID(guid);
}

CSelectMediaType::~CSelectMediaType()
{
}

void CSelectMediaType::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_COMBO1, m_guidstr);
	DDX_Control(pDX, IDC_COMBO1, m_guidsctrl);
}


BEGIN_MESSAGE_MAP(CSelectMediaType, CCmdUIDialog)
	ON_CBN_EDITCHANGE(IDC_COMBO1, OnCbnEditchangeCombo1)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOK)
END_MESSAGE_MAP()


// CSelectMediaType message handlers

BOOL CSelectMediaType::OnInitDialog()
{
	CCmdUIDialog::OnInitDialog();

	for(int i = 0; i < m_guids.GetCount(); i++)
	{
		m_guidsctrl.AddString(GetMediaTypeName(m_guids[i]));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectMediaType::OnCbnEditchangeCombo1()
{
	UpdateData();
	int i = m_guidsctrl.FindStringExact(0, m_guidstr);
	if(i >= 0)
	{
		DWORD sel = m_guidsctrl.GetEditSel();
		m_guidsctrl.SetCurSel(i);
		m_guidsctrl.SetEditSel(sel,sel);
	}
}

void CSelectMediaType::OnUpdateOK(CCmdUI* pCmdUI)
{
	UpdateData();

	pCmdUI->Enable(!m_guidstr.IsEmpty() && (m_guidsctrl.GetCurSel() >= 0 || GUIDFromCString(m_guidstr) != GUID_NULL));
}

void CSelectMediaType::OnOK()
{
	UpdateData();

	int i = m_guidsctrl.GetCurSel();
	m_guid = i >= 0 ? m_guids[i] : GUIDFromCString(m_guidstr);

	CCmdUIDialog::OnOK();
}
