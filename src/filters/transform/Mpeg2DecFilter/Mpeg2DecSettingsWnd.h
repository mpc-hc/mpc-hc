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
    bool m_readARFromStream;

    enum {
        IDC_PP_COMBO1 = 10000,
        IDC_PP_SLIDER1,
        IDC_PP_SLIDER2,
        IDC_PP_SLIDER3,
        IDC_PP_SLIDER4,
        IDC_PP_CHECK1,
        IDC_PP_CHECK2,
        IDC_PP_CHECK3,
        IDC_PP_CHECK4,
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
    CButton m_readARFromStream_check;
    CStatic m_note_static;

    void UpdateProcampValues();

public:
    CMpeg2DecSettingsWnd();

    bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
    void OnDisconnect();
    bool OnActivate();
    void OnDeactivate();
    bool OnApply();

    static LPCTSTR GetWindowTitle() {
        return _T("Settings");
    }
    static CSize GetWindowSize() {
        return CSize(320, 240);
    }

    DECLARE_MESSAGE_MAP()

    afx_msg void OnButtonProcampPc2Tv();
    afx_msg void OnButtonProcampReset();
    afx_msg void OnButtonInterlaced();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};