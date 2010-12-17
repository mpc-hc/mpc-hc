/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include "InternalPropertyPage.h"
#include <afxcmn.h>

class __declspec(uuid("A1EB391C-6089-4A87-9988-BE50872317D4"))
	CPinInfoWnd : public CInternalPropertyPageWnd
{
	CComQIPtr<IBaseFilter> m_pBF;

	enum {
		IDC_PP_COMBO1 = 10000,
		IDC_PP_EDIT1,
	};

	CStatic m_pin_static;
	CComboBox m_pin_combo;
	CEdit m_info_edit;

	void AddLine(CString str);

public:
	CPinInfoWnd();

	bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
	void OnDisconnect();
	bool OnActivate();
	void OnDeactivate();
	bool OnApply();

	static LPCTSTR GetWindowTitle() {
		return _T("Pin Info");
	}
	static CSize GetWindowSize() {
		return CSize(500, 300);
	}

	DECLARE_MESSAGE_MAP()

	void OnCbnSelchangeCombo1();
};
