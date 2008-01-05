/* 
 * $Id: MPCVideoDecSettingsWnd.h 249 2007-09-26 11:07:22Z casimir666 $
 *
 * (C) 2006-2007 see AUTHORS
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

#pragma once

#include "..\..\InternalPropertyPage.h"
#include "IMPCVideoDecFilter.h"
#include <afxcmn.h>

[uuid("D5AA0389-D274-48e1-BF50-ACB05A56DDE0")]
class CMPCVideoDecSettingsWnd : public CInternalPropertyPageWnd
{
	CComQIPtr<IMPCVideoDecFilter> m_pMDF;

	CButton		m_grpFFMpeg;
	CStatic		m_txtThreadNumber;
	CComboBox	m_cbThreadNumber;
	CStatic		m_txtDiscardMode;
	CComboBox	m_cbDiscardMode;
	CStatic		m_txtErrorResilience;
	CComboBox	m_cbErrorResilience;
	CStatic		m_txtIDCTAlgo;
	CComboBox	m_cbIDCTAlgo;
	
	CButton		m_grpDXVA;
	CButton		m_chkEnableDXVA;
	CStatic		m_txtDXVAMode;
	CEdit		m_edtDXVAMode;

	enum 
	{
		IDC_PP_THREAD_NUMBER = 10000,
		IDC_PP_ENABLE_DXVA,
		IDC_PP_ENABLE_DEBLOCKING,
		IDC_PP_DISCARD_MODE,
		IDC_PP_ERROR_RESILIENCE,
	};

public:
	CMPCVideoDecSettingsWnd();
	
	bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
	void OnDisconnect();
	bool OnActivate();
	void OnDeactivate();
	bool OnApply();

	static LPCTSTR GetWindowTitle() {return _T("Settings");}
	static CSize GetWindowSize() {return CSize(320, 240);}

	DECLARE_MESSAGE_MAP()
};