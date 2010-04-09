/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
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
#include "ConvertResDlg.h"


// CConvertResDlg dialog

CConvertResDlg::CConvertResDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CConvertResDlg::IDD, pParent)
	, m_name(_T(""))
	, m_desc(_T(""))
{
}

CConvertResDlg::~CConvertResDlg()
{
}

void CConvertResDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT3, m_name);
	DDX_Text(pDX, IDC_COMBO1, m_mime);
	DDX_Control(pDX, IDC_COMBO1, m_mimectrl);
	DDX_Text(pDX, IDC_EDIT2, m_desc);
}

BOOL CConvertResDlg::OnInitDialog()
{
	__super::OnInitDialog();

	AddAnchor(IDC_EDIT3, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_COMBO1, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_EDIT2, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_CENTER);
	AddAnchor(IDCANCEL, BOTTOM_CENTER);

	CRegKey key;
	CString str(_T("MIME\\Database\\Content Type"));
	if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, str, KEY_READ))
	{
		CAtlStringMap<bool> mimes;

		TCHAR buff[256];
		DWORD len = countof(buff);
		for(int i = 0; ERROR_SUCCESS == key.EnumKey(i, buff, &len); i++, len = countof(buff))
		{
			CRegKey mime;
			TCHAR ext[64];
			ULONG len = countof(ext);
			if(ERROR_SUCCESS == mime.Open(HKEY_CLASSES_ROOT, str + _T("\\") + buff, KEY_READ)
			&& ERROR_SUCCESS == mime.QueryStringValue(_T("Extension"), ext, &len))
			{
				CString mime = CString(buff).MakeLower();
				mimes[mime] = true;
				m_mimectrl.AddString(mime);
			}
		}

		static TCHAR* moremimes[] = 
		{
			_T("application/octet-stream"),
			_T("application/zip"),
			_T("application/rar"),
			_T("application/x-truetype-font"),
		};

		for(int i = 0; i < countof(moremimes); i++)
            if(!mimes.Lookup(moremimes[i]))
				m_mimectrl.AddString(moremimes[i]);
	}

	m_desc.Replace(_T("\n"), _T("\r\n"));

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CConvertResDlg::OnOK()
{
	UpdateData();

	m_name.Trim();
	m_mime.Trim();
	m_desc.Replace(_T("\r\n"), _T("\r"));
	m_desc.Trim();

	__super::OnOK();
}

BEGIN_MESSAGE_MAP(CConvertResDlg, CResizableDialog)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOK)
END_MESSAGE_MAP()

// CConvertResDlg message handlers

void CConvertResDlg::OnUpdateOK(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDlgItem(IDC_EDIT3)->GetWindowTextLength() > 0 && GetDlgItem(IDC_COMBO1)->GetWindowTextLength() > 0);
}
