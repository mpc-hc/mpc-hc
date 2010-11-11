/*
 * $Id$
 *
 * (C) 2006-2010 see AUTHORS
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

	enum
	{
		IDC_PP_WASAPI_MODE = 10000,
		IDC_PP_MUTE_FAST_FORWARD,
	};

public:
	CMpcAudioRendererSettingsWnd(void);


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
