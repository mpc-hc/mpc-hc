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
#include "MainFrm.h"
#include "PPageSubDB.h"
#include "ISDb.h"


// CPPageSubDB dialog

IMPLEMENT_DYNAMIC(CPPageSubDB, CPPageBase)
CPPageSubDB::CPPageSubDB()
    : CPPageBase(CPPageSubDB::IDD, CPPageSubDB::IDD)
    , m_ISDb(_T(""))
{
}

CPPageSubDB::~CPPageSubDB()
{
}

void CPPageSubDB::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO1, m_ISDbCombo);
    DDX_CBString(pDX, IDC_COMBO1, m_ISDb);
}

BEGIN_MESSAGE_MAP(CPPageSubDB, CPPageBase)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateButton1)
END_MESSAGE_MAP()


// CPPageSubDB message handlers

BOOL CPPageSubDB::OnInitDialog()
{
    __super::OnInitDialog();

    AppSettings& s = AfxGetAppSettings();

    m_ISDb = s.ISDb;
    m_ISDbCombo.AddString(m_ISDb);
    if(m_ISDb.CompareNoCase(_T("www.opensubtitles.org/isdb")))
        m_ISDbCombo.AddString(_T("www.opensubtitles.org/isdb"));

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageSubDB::OnApply()
{
    UpdateData();

    AppSettings& s = AfxGetAppSettings();

    s.ISDb = m_ISDb;
    s.ISDb.TrimRight('/');

    return __super::OnApply();
}

void CPPageSubDB::OnBnClickedButton1()
{
    CString ISDb, ver, msg, str;

    m_ISDbCombo.GetWindowText(ISDb);
    ISDb.TrimRight('/');

    ver.Format(_T("ISDb v%d"), ISDb_PROTOCOL_VERSION);

    CWebTextFile wtf;
    if(wtf.Open(_T("http://") + ISDb + _T("/test.php")) && wtf.ReadString(str) && str == ver)
    {
        msg = ResStr(IDS_PPSDB_URLCORRECT);
    }
    else if(str.Find(_T("ISDb v")) == 0)
    {
        msg = ResStr(IDS_PPSDB_PROTOCOLERR);
    }
    else
    {
        msg = ResStr(IDS_PPSDB_BADURL);
    }

    AfxMessageBox(msg, MB_OK);
}

void CPPageSubDB::OnUpdateButton1(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_ISDbCombo.GetWindowTextLength() > 0);
}
