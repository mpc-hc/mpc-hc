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
#include "SaveThumbnailsDialog.h"


// CSaveThumbnailsDialog

IMPLEMENT_DYNAMIC(CSaveThumbnailsDialog, CFileDialog)
CSaveThumbnailsDialog::CSaveThumbnailsDialog(
	int rows, int cols, int width,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
	LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(FALSE, lpszDefExt, lpszFileName,
				OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR,
				lpszFilter, pParentWnd, 0, FALSE),
	m_rows(rows), m_cols(cols), m_width(width)
{
	if (m_ofn.lStructSize == sizeof(OPENFILENAME)) {
		SetTemplate(0, IDD_SAVETHUMBSDIALOGTEMPL);
	} else { /*if (m_ofn.lStructSize == OPENFILENAME_SIZE_VERSION_400)*/
		SetTemplate(0, IDD_SAVETHUMBSDIALOGTEMPL_400);
	}
}

CSaveThumbnailsDialog::~CSaveThumbnailsDialog()
{
}

void CSaveThumbnailsDialog::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_SPIN1, m_rowsctrl);
	DDX_Control(pDX, IDC_SPIN2, m_colsctrl);
	DDX_Control(pDX, IDC_SPIN3, m_widthctrl);
	__super::DoDataExchange(pDX);
}

BOOL CSaveThumbnailsDialog::OnInitDialog()
{
	__super::OnInitDialog();

	m_rowsctrl.SetRange(1, 20);
	m_colsctrl.SetRange(1, 10);
	m_widthctrl.SetRange(256, 2560);
	m_rowsctrl.SetPos(m_rows);
	m_colsctrl.SetPos(m_cols);
	m_widthctrl.SetPos(m_width);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CSaveThumbnailsDialog, CFileDialog)
END_MESSAGE_MAP()

// CSaveThumbnailsDialog message handlers

BOOL CSaveThumbnailsDialog::OnFileNameOK()
{
	m_rows = m_rowsctrl.GetPos();
	m_cols = m_colsctrl.GetPos();
	m_width = m_widthctrl.GetPos();

	return __super::OnFileNameOK();
}
