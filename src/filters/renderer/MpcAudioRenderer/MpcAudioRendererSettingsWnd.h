/*
 * $Id$
 *
 * (C) 2010-2012 see Authors.txt
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
#include "IMpcAudioRendererFilter.h"
#include <afxcmn.h>

class __declspec(uuid("1E53BA32-3BCC-4dff-9342-34E46BE3F5A5"))
	CMpcAudioRendererSettingsWnd : public CInternalPropertyPageWnd
{
private :
	CComQIPtr<IMpcAudioRendererFilter> m_pMAR;

	CButton		m_grpDefault;

	CStatic		m_txtWasapiMode;
	CButton		m_cbWasapiMode;
	CButton		m_cbMuteFastForward;

	CStatic		m_txtSoundDevice;
	CComboBox	m_cbSoundDevice;

	enum {
		IDC_PP_WASAPI_MODE = 10000,
		IDC_PP_MUTE_FAST_FORWARD,
		IDC_PP_SOUND_DEVICE,
	};

public:
	CMpcAudioRendererSettingsWnd(void);


	bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
	void OnDisconnect();
	bool OnActivate();
	void OnDeactivate();
	bool OnApply();

	HRESULT GetAvailableAudioDevices();

	static LPCTSTR GetWindowTitle() {
		return _T("Settings");
	}
	static CSize GetWindowSize() {
		return CSize(350, 325);
	}

	DECLARE_MESSAGE_MAP()
};
