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

// FavoritAddDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "FavoriteAddDlg.h"


// CFavoriteAddDlg dialog

IMPLEMENT_DYNAMIC(CFavoriteAddDlg, CCmdUIDialog)
CFavoriteAddDlg::CFavoriteAddDlg(CString shortname, CString fullname, CWnd* pParent /*=NULL*/)
	: CCmdUIDialog(CFavoriteAddDlg::IDD, pParent)
	, m_shortname(shortname)
	, m_fullname(fullname)
	, m_fRememberPos(TRUE)
{
}

CFavoriteAddDlg::~CFavoriteAddDlg()
{
}

void CFavoriteAddDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_namectrl);
	DDX_CBString(pDX, IDC_COMBO1, m_name);	
	DDX_Check(pDX, IDC_CHECK1, m_fRememberPos);
}

BOOL CFavoriteAddDlg::OnInitDialog()
{
	__super::OnInitDialog();

	if(!m_shortname.IsEmpty()) m_namectrl.AddString(m_shortname);
	if(!m_fullname.IsEmpty()) m_namectrl.AddString(m_fullname);
	m_namectrl.SetCurSel(0);

	::CorrectComboListWidth(m_namectrl, GetFont());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(CFavoriteAddDlg, CCmdUIDialog)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
END_MESSAGE_MAP()


// CFavoriteAddDlg message handlers

void CFavoriteAddDlg::OnUpdateOk(CCmdUI* pCmdUI)
{
	UpdateData();
	pCmdUI->Enable(!m_name.IsEmpty());
}
