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

#pragma once

#include <Version.h>
#include "afxwin.h"


class UpdateCheckerDlg : public CDialog
{
	DECLARE_DYNAMIC(UpdateCheckerDlg)

public:
	UpdateCheckerDlg(bool updateAvailable, const Version& latestVersion, CWnd* pParent = NULL);
	virtual ~UpdateCheckerDlg();

	enum { IDD = IDD_UPDATE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg virtual BOOL OnInitDialog();
	afx_msg virtual void OnOK();

	DECLARE_MESSAGE_MAP()
private:
	bool m_updateAvailable;
	CString m_text;
	CStatic m_icon;
	CButton m_okButton;
	CButton m_cancelButton;
};
