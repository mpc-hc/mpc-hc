/*
 * (C) 2012 see Authors.txt
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
#include "IAviSplitterFilter.h"
#include "resource.h"
#include <afxcmn.h>

class __declspec(uuid("A008C72A-0D51-461D-9043-C823B3A7D4CC"))
    CAviSplitterSettingsWnd : public CInternalPropertyPageWnd
{
private:
    CComQIPtr<IAviSplitterFilter> m_pASF;

    CButton     m_cbNonInterleavedFilesSupport;

    enum {
        IDC_PP_NON_INTERLEAVED_FILES_SUPPORT = 10000,
    };

public:
    CAviSplitterSettingsWnd();

    bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
    void OnDisconnect();
    bool OnActivate();
    void OnDeactivate();
    bool OnApply();

    static LPCTSTR GetWindowTitle() { return MAKEINTRESOURCE(IDS_FILTER_SETTINGS_CAPTION); }
    static CSize GetWindowSize() { return CSize(280, 33); }

    DECLARE_MESSAGE_MAP()
};
