/*
 * $Id$
 *
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

#include "../../InternalPropertyPage.h"
#include "IMPCVideoDecFilter.h"
#include <afxcmn.h>

class __declspec(uuid("D5AA0389-D274-48e1-BF50-ACB05A56DDE0"))
	CMPCVideoDecSettingsWnd : public CInternalPropertyPageWnd
{
	CComQIPtr<IMPCVideoDecFilter> m_pMDF;

	CButton		m_grpFFMpeg;
	CStatic		m_txtThreadNumber;
	CComboBox	m_cbThreadNumber;
	CStatic		m_txtDiscardMode;
	CComboBox	m_cbDiscardMode;
	CStatic		m_txtErrorRecognition;
	CComboBox	m_cbErrorRecognition;
	CStatic		m_txtIDCTAlgo;
	CComboBox	m_cbIDCTAlgo;

	CButton		m_grpDXVA;
	CStatic		m_txtDXVAMode;
	CEdit		m_edtDXVAMode;
	CStatic		m_txtVideoCardDescription;
	CEdit		m_edtVideoCardDescription;

	CButton		m_cbARMode;

	CStatic		m_txtDXVACompatibilityCheck;
	CComboBox	m_cbDXVACompatibilityCheck;

	CButton		m_cbDXVA_SD;

	enum {
		IDC_PP_THREAD_NUMBER = 10000,
		IDC_PP_ENABLE_DEBLOCKING,
		IDC_PP_DISCARD_MODE,
		IDC_PP_ERROR_RECOGNITION,
		IDC_PP_AR,
		IDC_PP_DXVA_CHECK,
		IDC_PP_DXVA_SD
	};

public:
	CMPCVideoDecSettingsWnd();

	bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
	void OnDisconnect();
	bool OnActivate();
	void OnDeactivate();
	bool OnApply();

	static LPCTSTR GetWindowTitle() {
		return _T("Settings");
	}
	static CSize GetWindowSize() {
		return CSize(350, 325);
	}

	DECLARE_MESSAGE_MAP()
};


class __declspec(uuid("3C395D46-8B0F-440d-B962-2F4A97355453"))
	CMPCVideoDecCodecWnd : public CInternalPropertyPageWnd
{
	CComQIPtr<IMPCVideoDecFilter> m_pMDF;

	CButton		m_grpSelectedCodec;
	CCheckListBox	m_lstCodecs;
	CImageList	m_onoff;

public:
	CMPCVideoDecCodecWnd();

	bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
	void OnDisconnect();
	bool OnActivate();
	void OnDeactivate();
	bool OnApply();

	static LPCTSTR GetWindowTitle() {
		return _T("Codecs");
	}
	static CSize GetWindowSize() {
		return CSize(350, 300);
	}

	DECLARE_MESSAGE_MAP()
};
