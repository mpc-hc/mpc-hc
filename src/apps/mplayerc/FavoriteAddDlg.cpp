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
#include "FavoriteAddDlg.h"
#include "SettingsDefines.h"


// CFavoriteAddDlg dialog

IMPLEMENT_DYNAMIC(CFavoriteAddDlg, CCmdUIDialog)
CFavoriteAddDlg::CFavoriteAddDlg(CString shortname, CString fullname, CWnd* pParent /*=NULL*/)
    : CCmdUIDialog(CFavoriteAddDlg::IDD, pParent)
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
}

BOOL CFavoriteAddDlg::OnInitDialog()
{
    __super::OnInitDialog();

    if(!m_shortname.IsEmpty())
        m_namectrl.AddString(m_shortname);

    if(!m_fullname.IsEmpty())
        m_namectrl.AddString(m_fullname);

    ::CorrectComboListWidth(m_namectrl, GetFont());

    m_bRememberPos = AfxGetApp()->GetProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_REMEMBERPOS, TRUE);
    m_bRelativeDrive = AfxGetApp()->GetProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_RELATIVEDRIVE, FALSE);

    UpdateData(FALSE); // Update UI

    m_namectrl.SetCurSel(0);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(CFavoriteAddDlg, CCmdUIDialog)
    ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
END_MESSAGE_MAP()


// CFavoriteAddDlg message handlers

void CFavoriteAddDlg::OnUpdateOk(CCmdUI *pCmdUI)
{
    UpdateData(); // Retrieve UI values

    pCmdUI->Enable(!m_name.IsEmpty());
}

void CFavoriteAddDlg::OnOK()
{
    UpdateData(); // Retrieve UI values

    // Remember settings
    AfxGetApp()->WriteProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_REMEMBERPOS, m_bRememberPos);
    AfxGetApp()->WriteProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_RELATIVEDRIVE, m_bRelativeDrive);

    CCmdUIDialog::OnOK();
}
