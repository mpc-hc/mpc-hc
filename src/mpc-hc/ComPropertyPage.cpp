/*
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
#include "ComPropertyPage.h"
#include "ComPropertySheet.h"


// CComPropertyPage dialog

IMPLEMENT_DYNAMIC(CComPropertyPage, CPropertyPage)
CComPropertyPage::CComPropertyPage(IPropertyPage* pPage)
    : CPropertyPage(CComPropertyPage::IDD), m_pPage(pPage)
{
    PROPPAGEINFO ppi;
    m_pPage->GetPageInfo(&ppi);
    m_pPSP->pszTitle = (m_strCaption = ppi.pszTitle);
    m_psp.dwFlags |= PSP_USETITLE;
}

CComPropertyPage::~CComPropertyPage()
{
}

void CComPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}

BOOL CComPropertyPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    CRect r;
    PROPPAGEINFO ppi;
    m_pPage->GetPageInfo(&ppi);
    r = CRect(CPoint(0, 0), ppi.size);
    m_pPage->Activate(m_hWnd, r, FALSE);
    m_pPage->Show(SW_SHOW);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CComPropertyPage::OnDestroy()
{
    CPropertyPage::OnDestroy();
    m_pPage->Deactivate();
}

BOOL CComPropertyPage::OnSetActive()
{
    SetModified(S_OK == m_pPage->IsPageDirty());

    CWnd* pParent = GetParent();
    if (pParent->IsKindOf(RUNTIME_CLASS(CComPropertySheet))) {
        CComPropertySheet* pSheet = static_cast<CComPropertySheet*>(pParent);
        pSheet->OnActivated(this);
    }

    return CPropertyPage::OnSetActive();
}

BOOL CComPropertyPage::OnKillActive()
{
    SetModified(FALSE);
    return CPropertyPage::OnKillActive();
}


BEGIN_MESSAGE_MAP(CComPropertyPage, CPropertyPage)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CComPropertyPage message handlers

void CComPropertyPage::OnOK()
{
    if (S_OK == m_pPage->IsPageDirty()) {
        m_pPage->Apply();
    }
    SetModified(FALSE);
    CPropertyPage::OnOK();
}
