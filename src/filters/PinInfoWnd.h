/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
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

protected:
    virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
