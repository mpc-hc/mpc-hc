/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of vsrip.
 *
 * Vsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <afxpriv.h>
#include "VSRip.h"
#include "VSRipPage.h"

// CVSRipPage dialog

IMPLEMENT_DYNAMIC(CVSRipPage, CDialog)
CVSRipPage::CVSRipPage(IVSFRipper* pVSFRipper, UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
	, m_pVSFRipper(pVSFRipper)
{
	m_cRef = 1;
}

CVSRipPage::~CVSRipPage()
{
}

void CVSRipPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVSRipPage, CDialog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CVSRipPage message handlers

void CVSRipPage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	m_pVSFRipper->SetCallBack(bShow ? (IVSFRipperCallback*)this : NULL);	
}

