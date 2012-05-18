/*
 * $Id$
 *
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

#pragma once

#include "ComPropertyPage.h"


interface IComPropertyPageDirty {
	virtual void OnSetDirty(bool fDirty) = 0;
};

// CComPropertySheet

class CComPropertySheet : public CPropertySheet, public IComPropertyPageDirty
{
	DECLARE_DYNAMIC(CComPropertySheet)

	CComPtr<IPropertyPageSite> m_pSite;
	CInterfaceList<ISpecifyPropertyPages> m_spp;
	CAutoPtrList<CComPropertyPage> m_pages;
	CSize m_size;

public:
	CComPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CComPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CComPropertySheet();

	int AddPages(ISpecifyPropertyPages* pSPP);
	bool AddPage(IPropertyPage* pPage, IUnknown* pUnk);

	void OnActivated(CPropertyPage* pPage);

	// IComPropertyPageDirty
	void OnSetDirty(bool fDirty) {
		if (CPropertyPage* p = GetActivePage()) {
			p->SetModified(fDirty);
		}
	}

	virtual BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
};
