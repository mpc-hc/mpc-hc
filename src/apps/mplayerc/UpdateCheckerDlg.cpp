/*
 * $Id$
 *
 * (C) 2012 see AUTHORS
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
#include "UpdateCheckerDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(UpdateCheckerDlg, CDialog)

UpdateCheckerDlg::UpdateCheckerDlg(Update_Status updateStatus, const Version& latestVersion, CWnd* pParent /*=NULL*/)
	: CDialog(UpdateCheckerDlg::IDD, pParent), m_updateStatus(updateStatus)
{
	switch (updateStatus) {
		case UPDATE_AVAILABLE:
			m_text.Format(IDS_NEW_UPDATE_AVAILABLE, latestVersion.major, latestVersion.minor, latestVersion.patch, latestVersion.revision);
			break;
		case UPDATE_NOT_AVAILABLE:
			m_text.Format(IDS_NO_NEW_UPDATE);
			break;
		case UPDATE_ERROR:
			m_text.Format(IDS_UPDATE_ERROR);
			break;
		default:
			ASSERT(0); // should never happen
	}
}

UpdateCheckerDlg::~UpdateCheckerDlg()
{
}

void UpdateCheckerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_UPDATE_DLG_TEXT, m_text);
	DDX_Control(pDX, IDC_UPDATE_ICON, m_icon);
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Control(pDX, IDCANCEL, m_cancelButton);
}


BEGIN_MESSAGE_MAP(UpdateCheckerDlg, CDialog)
END_MESSAGE_MAP()

BOOL UpdateCheckerDlg::OnInitDialog()
{
	__super::OnInitDialog();

	switch (m_updateStatus) {
		case UPDATE_AVAILABLE:
			m_icon.SetIcon(LoadIcon(NULL, IDI_QUESTION));
			break;
		case UPDATE_NOT_AVAILABLE:
		case UPDATE_ERROR:
			m_icon.SetIcon(LoadIcon(NULL, (m_updateStatus == UPDATE_NOT_AVAILABLE) ? IDI_INFORMATION : IDI_WARNING));
			m_okButton.ShowWindow(SW_HIDE);
			m_cancelButton.SetWindowText(ResStr(IDS_UPDATE_CLOSE));
			m_cancelButton.SetFocus();
			break;
		default:
			ASSERT(0); // should never happen
	}

	return TRUE;
}

void UpdateCheckerDlg::OnOK()
{
	if (m_updateStatus == UPDATE_AVAILABLE) {
		ShellExecute(NULL, _T("open"), _T("http://mpc-hc.sourceforge.net/download-media-player-classic-hc.html"), NULL, NULL, SW_SHOWNORMAL);
	}

	__super::OnOK();
}
