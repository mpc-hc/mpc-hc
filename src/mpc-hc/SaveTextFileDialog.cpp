/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
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
#include "SaveTextFileDialog.h"


// CSaveTextFileDialog

IMPLEMENT_DYNAMIC(CSaveTextFileDialog, CFileDialog)
CSaveTextFileDialog::CSaveTextFileDialog(
	CTextFile::enc e,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
	LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(FALSE, lpszDefExt, lpszFileName,
				OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR,
				lpszFilter, pParentWnd, 0, FALSE),
	m_e(e)
{
	SetTemplate(0, IDD_SAVETEXTFILEDIALOGTEMPL);
}

CSaveTextFileDialog::~CSaveTextFileDialog()
{
}

void CSaveTextFileDialog::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_COMBO1, m_encoding);
	__super::DoDataExchange(pDX);
}

BOOL CSaveTextFileDialog::OnInitDialog()
{
	__super::OnInitDialog();

	m_encoding.SetItemData(m_encoding.AddString(_T("ANSI")), CTextFile::ASCII);
	m_encoding.SetItemData(m_encoding.AddString(_T("Unicode 16-LE")), CTextFile::LE16);
	m_encoding.SetItemData(m_encoding.AddString(_T("Unicode 16-BE")), CTextFile::BE16);
	m_encoding.SetItemData(m_encoding.AddString(_T("UTF-8")), CTextFile::UTF8);

	switch (m_e) {
		default:
		case CTextFile::ASCII:
			m_encoding.SetCurSel(0);
			break;
		case CTextFile::LE16:
			m_encoding.SetCurSel(1);
			break;
		case CTextFile::BE16:
			m_encoding.SetCurSel(2);
			break;
		case CTextFile::UTF8:
			m_encoding.SetCurSel(3);
			break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CSaveTextFileDialog, CFileDialog)
END_MESSAGE_MAP()

// CSaveTextFileDialog message handlers

BOOL CSaveTextFileDialog::OnFileNameOK()
{
	m_e = (CTextFile::enc)m_encoding.GetItemData(m_encoding.GetCurSel());
	return __super::OnFileNameOK();
}
