/*
 * $Id$
 *
 * (C) 2012 see Authors.txt
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
#include "UpdateCheckerDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(UpdateCheckerDlg, CDialog)

UpdateCheckerDlg::UpdateCheckerDlg(Update_Status updateStatus, const Version& latestVersion, CWnd* pParent /*=NULL*/)
	: CDialog(UpdateCheckerDlg::IDD, pParent), m_updateStatus(updateStatus)
{
	switch (updateStatus) {
		case UPDATER_UPDATE_AVAILABLE:
			m_text.Format(IDS_NEW_UPDATE_AVAILABLE, latestVersion.major, latestVersion.minor, latestVersion.patch, latestVersion.revision);
			break;
		case UPDATER_LATEST_STABLE:
			m_text.Format(IDS_USING_LATEST_STABLE);
			break;
		case UPDATER_NEWER_VERSION:
			m_text.Format(IDS_USING_NEWER_VERSION,
						  UpdateChecker::MPC_VERSION.major, UpdateChecker::MPC_VERSION.minor, UpdateChecker::MPC_VERSION.patch, UpdateChecker::MPC_VERSION.revision,
						  latestVersion.major, latestVersion.minor, latestVersion.patch, latestVersion.revision);
			break;
		case UPDATER_ERROR:
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
		case UPDATER_UPDATE_AVAILABLE:
			m_icon.SetIcon(LoadIcon(NULL, IDI_QUESTION));
			break;
		case UPDATER_LATEST_STABLE:
		case UPDATER_NEWER_VERSION:
		case UPDATER_ERROR:
			m_icon.SetIcon(LoadIcon(NULL, (m_updateStatus == UPDATER_ERROR) ? IDI_WARNING : IDI_INFORMATION));
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
	if (m_updateStatus == UPDATER_UPDATE_AVAILABLE) {
		ShellExecute(NULL, _T("open"), _T("http://mpc-hc.sourceforge.net/download-media-player-classic-hc.html"), NULL, NULL, SW_SHOWNORMAL);
	}

	__super::OnOK();
}
