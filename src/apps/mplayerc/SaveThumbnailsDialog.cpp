/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
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
#include "mplayerc.h"
#include "SaveThumbnailsDialog.h"

#define CAP_SELECTION(_MIN,_VAL,_MAX)	min (max (_MIN, _VAL), _MAX)

// CSaveThumbnailsDialog

IMPLEMENT_DYNAMIC(CSaveThumbnailsDialog, CFileDialog)
CSaveThumbnailsDialog::CSaveThumbnailsDialog(
	int rows, int cols, int width,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
	LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(FALSE, lpszDefExt, lpszFileName, 
			OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST, 
			lpszFilter, pParentWnd, 0),
	m_rows(rows), m_cols(cols), m_width(width)
{
	m_pCustom.Attach (GetIFileDialogCustomize());

	if (m_pCustom)
	{
		CString			strTmp;

		strTmp.Format (_T("%d"), m_rows);
        m_pCustom->StartVisualGroup (IDS_THUMB_ROWNUMBER, ResStr(IDS_THUMB_ROWNUMBER)); 
		m_pCustom->AddEditBox (IDC_EDIT1, strTmp);
        m_pCustom->EndVisualGroup();

		strTmp.Format (_T("%d"), m_width);
        m_pCustom->StartVisualGroup (IDS_THUMB_COLNUMBER, ResStr(IDS_THUMB_COLNUMBER)); 
		m_pCustom->AddEditBox (IDC_EDIT3, strTmp);
        m_pCustom->EndVisualGroup();

		strTmp.Format (_T("%d"), m_cols);
        m_pCustom->StartVisualGroup (IDS_THUMB_IMAGE_WIDTH, ResStr(IDS_THUMB_IMAGE_WIDTH)); 
		m_pCustom->AddEditBox (IDC_EDIT2, strTmp);
        m_pCustom->EndVisualGroup();

	}
	else if(m_ofn.lStructSize == sizeof(OPENFILENAME))
	{
		SetTemplate(0, IDD_SAVETHUMBSDIALOGTEMPL);
	}
	else /*if(m_ofn.lStructSize == OPENFILENAME_SIZE_VERSION_400)*/
	{
		SetTemplate(0, IDD_SAVETHUMBSDIALOGTEMPL_400);
	}
}

CSaveThumbnailsDialog::~CSaveThumbnailsDialog()
{
	m_pCustom = NULL;
}

void CSaveThumbnailsDialog::DoDataExchange(CDataExchange* pDX)
{
	if (!m_pCustom)
	{
		DDX_Control(pDX, IDC_SPIN1, m_rowsctrl);
		DDX_Control(pDX, IDC_SPIN2, m_colsctrl);
		DDX_Control(pDX, IDC_SPIN3, m_widthctrl);	
	}
	__super::DoDataExchange(pDX);
}

BOOL CSaveThumbnailsDialog::OnInitDialog()
{
	__super::OnInitDialog();

	m_rowsctrl.SetRange(1, 8);
	m_colsctrl.SetRange(1, 8);
	m_widthctrl.SetRange(256, 2048);
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
	if (!m_pCustom)
	{
		m_rows = m_rowsctrl.GetPos();
		m_cols = m_colsctrl.GetPos();
		m_width = m_widthctrl.GetPos();
	}
	else
	{
		// Vista !
		WCHAR*		pstrTemp;

		m_pCustom->GetEditBoxText(IDC_EDIT1, &pstrTemp);
		m_rows = CAP_SELECTION (1, _wtol (pstrTemp), 8);
		CoTaskMemFree (pstrTemp);

		m_pCustom->GetEditBoxText(IDC_EDIT2, &pstrTemp);
		m_cols = CAP_SELECTION (1, _wtol (pstrTemp), 8);
		CoTaskMemFree (pstrTemp);

		m_pCustom->GetEditBoxText(IDC_EDIT3, &pstrTemp);
		m_width = CAP_SELECTION (256, _wtol (pstrTemp), 2048);
		CoTaskMemFree (pstrTemp);
	}

	return __super::OnFileNameOK();
}
