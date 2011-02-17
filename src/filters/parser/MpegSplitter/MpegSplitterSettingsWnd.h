/*
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
#include "IMpegSplitter.h"
#include <afxcmn.h>

#include "../../../apps/mplayerc/resource.h"
// ==>>> Resource identifier from "resource.h" present in mplayerc project!
#define ResStr(id) CString(MAKEINTRESOURCE(id))

class __declspec(uuid("44FCB62D-3AEB-401C-A7E1-8A984C017923"))
	CMpegSplitterSettingsWnd : public CInternalPropertyPageWnd
{
private :
	CComQIPtr<IMpegSplitter> m_pMS;

	CButton		m_grpDefault;

public:
	CMpegSplitterSettingsWnd(void);


	bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
	void OnDisconnect();
	bool OnActivate();
	void OnDeactivate();
	bool OnApply();

	static LPCTSTR GetWindowTitle() {
		return ResStr(IDS_AG_SETTINGS);
	}
	static CSize GetWindowSize() {
		return CSize(350, 300);
	}

	DECLARE_MESSAGE_MAP()
};
