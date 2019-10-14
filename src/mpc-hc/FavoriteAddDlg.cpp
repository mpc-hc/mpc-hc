/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2016 see Authors.txt
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
#include "FavoriteAddDlg.h"
#include "SettingsDefines.h"
#include "AppSettings.h"


// CFavoriteAddDlg dialog

IMPLEMENT_DYNAMIC(CFavoriteAddDlg, CMPCThemeCmdUIDialog)
CFavoriteAddDlg::CFavoriteAddDlg(CString shortname, CString fullname, CWnd* pParent /*=nullptr*/)
    : CMPCThemeCmdUIDialog(CFavoriteAddDlg::IDD, pParent)
    , m_shortname(shortname)
    , m_fullname(fullname)
    , m_bRememberPos(TRUE)
    , m_bRelativeDrive(FALSE)
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
    DDX_Check(pDX, IDC_CHECK1, m_bRememberPos);
    DDX_Check(pDX, IDC_CHECK2, m_bRelativeDrive);
    fulfillThemeReqs();
}

BOOL CFavoriteAddDlg::OnInitDialog()
{
    __super::OnInitDialog();

    if (!m_shortname.IsEmpty()) {
        m_namectrl.AddString(m_shortname);
    }

    if (!m_fullname.IsEmpty()) {
        m_namectrl.AddString(m_fullname);
    }

    ::CorrectComboListWidth(m_namectrl);

    const CAppSettings& s = AfxGetAppSettings();

    m_bRememberPos = s.bFavRememberPos;
    m_bRelativeDrive = s.bFavRelativeDrive;

    UpdateData(FALSE); // Update UI

    m_namectrl.SetCurSel(0);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(CFavoriteAddDlg, CMPCThemeCmdUIDialog)
    ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
END_MESSAGE_MAP()


// CFavoriteAddDlg message handlers

void CFavoriteAddDlg::OnUpdateOk(CCmdUI* pCmdUI)
{
    UpdateData(); // Retrieve UI values

    pCmdUI->Enable(!m_name.IsEmpty());
}

void CFavoriteAddDlg::OnOK()
{
    UpdateData(); // Retrieve UI values

    // Remember settings
    CAppSettings& s = AfxGetAppSettings();

    s.bFavRememberPos = !!m_bRememberPos;
    s.bFavRelativeDrive = !!m_bRelativeDrive;

    CCmdUIDialog::OnOK();
}
