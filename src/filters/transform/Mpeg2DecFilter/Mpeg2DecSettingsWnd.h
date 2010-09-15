/* 
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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
#include "IMpeg2DecFilter.h"
#include <afxcmn.h>

class __declspec(uuid("E5FB6957-65E6-491B-BB37-B25C9FE3BEA7"))
CMpeg2DecSettingsWnd : public CInternalPropertyPageWnd
{
	CComQIPtr<IMpeg2DecFilter> m_pM2DF;

	ditype m_ditype;
	float m_procamp[4];
	bool m_planaryuv;
	bool m_interlaced;
	bool m_forcedsubs;

	enum 
	{
		IDC_PP_COMBO1 = 10000,
		IDC_PP_SLIDER1,
		IDC_PP_SLIDER2,
		IDC_PP_SLIDER3,
		IDC_PP_SLIDER4,
		IDC_PP_CHECK1,
		IDC_PP_CHECK2,
		IDC_PP_CHECK3,
		IDC_PP_BUTTON1,
		IDC_PP_BUTTON2,
	};

	CStatic m_ditype_static;
	CComboBox m_ditype_combo;
	CStatic m_procamp_static[4];
	CSliderCtrl m_procamp_slider[4];
	CStatic m_procamp_value[4];
	CButton m_procamp_tv2pc;
	CButton m_procamp_reset;
	CButton m_planaryuv_check;
	CButton m_interlaced_check;
	CButton m_forcedsubs_check;
	CStatic m_note_static;

	void UpdateProcampValues();

public:
	CMpeg2DecSettingsWnd();
	
	bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
	void OnDisconnect();
	bool OnActivate();
	void OnDeactivate();
	bool OnApply();

	static LPCTSTR GetWindowTitle() {return _T("Settings");}
	static CSize GetWindowSize() {return CSize(320, 240);}

	DECLARE_MESSAGE_MAP()

	afx_msg void OnButtonProcampPc2Tv();
	afx_msg void OnButtonProcampReset();
	afx_msg void OnButtonInterlaced();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};