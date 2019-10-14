/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
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

#pragma once

#include "CMPCThemeComPropertyPage.h"
#include "CMPCThemeUtil.h"

interface IComPropertyPageDirty
{
    virtual void OnSetDirty(bool fDirty) = 0;
};

// CComPropertySheet

class CComPropertySheet : public CPropertySheet
    , public IComPropertyPageDirty
    , public CMPCThemeUtil
{
    DECLARE_DYNAMIC(CComPropertySheet)

    CComPtr<IPropertyPageSite> m_pSite;
    CInterfaceList<ISpecifyPropertyPages> m_spp;
    CAutoPtrList<CMPCThemeComPropertyPage> m_pages;
    CSize m_size;

public:
    CComPropertySheet(UINT nIDCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
    CComPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
    virtual ~CComPropertySheet();

    int AddPages(ISpecifyPropertyPages* pSPP, ULONG uIgnorePage = ULONG(-1));
    bool AddPage(IPropertyPage* pPage, IUnknown* pUnk);

    void OnActivated(CPropertyPage* pPage);

    // IComPropertyPageDirty
    void OnSetDirty(bool fDirty) {
        if (CPropertyPage* p = GetActivePage()) {
            p->SetModified(fDirty);
        }
    }

    virtual BOOL OnInitDialog();
    void fulfillThemeReqs();

    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

protected:
    DECLARE_MESSAGE_MAP()
};
