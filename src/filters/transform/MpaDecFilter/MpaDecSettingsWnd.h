/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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

#include "../../InternalPropertyPage.h"
#include "MpaDecFilter.h"
#include <afxcmn.h>

class __declspec(uuid("24103041-884B-4772-B0D3-A600E7CBFEC7"))
	CMpaDecSettingsWnd : public CInternalPropertyPageWnd
{
	CComQIPtr<IMpaDecFilter> m_pMDF;

	int m_outputformat;
	int m_ac3spkcfg;
	bool m_ac3drc;
	int m_dtsspkcfg;
	bool m_dtsdrc;
	bool m_aacdownmix;
	DolbyDigitalMode m_ddmode;

	enum {
		IDC_PP_RADIO1 = 10000,
		IDC_PP_RADIO2,
		IDC_PP_RADIO3,
		IDC_PP_RADIO4,
		IDC_PP_COMBO1,
		IDC_PP_COMBO2,
		IDC_PP_COMBO3,
		IDC_PP_CHECK1,
		IDC_PP_CHECK2,
		IDC_PP_CHECK3,
		IDC_PP_CHECK4,
		IDC_PP_CHECK5
	};

	CStatic m_outputformat_static;
	CComboBox m_outputformat_combo;
	CStatic m_ac3spkcfg_static;
	CButton m_ac3spkcfg_radio[2];
	CComboBox m_ac3spkcfg_combo;
	CButton m_ac3spkcfg_check;
	CButton m_ac3lfe_check;
	CStatic m_dtsspkcfg_static;
	CButton m_dtsspkcfg_radio[2];
	CComboBox m_dtsspkcfg_combo;
	CButton m_dtsspkcfg_check;
	CStatic m_aacspkcfg_static;
	CButton m_dtslfe_check;
	CButton m_aacdownmix_check;

public:
	CMpaDecSettingsWnd();

	bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
	void OnDisconnect();
	bool OnActivate();
	void OnDeactivate();
	bool OnApply();
	LPCTSTR GetDolbyMode (DolbyDigitalMode ddmode);

	static LPCTSTR GetWindowTitle() {
		return _T("Settings");
	}
	static CSize GetWindowSize() {
		return CSize(320, 320);
	}

	DECLARE_MESSAGE_MAP()
};