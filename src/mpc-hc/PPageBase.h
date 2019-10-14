/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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

#include "ResizableLib/ResizableDialog.h"
#include "CMPCThemeToolTipCtrl.h"

// CPPageBase dialog

class CPPageBase : public CCmdUIPropertyPage
{
    DECLARE_DYNAMIC(CPPageBase)

protected:
    CMPCThemeToolTipCtrl m_wndToolTip;
    std::map<UINT, CImageList> m_buttonIcons;

    static bool FillComboToolTip(CComboBox& comboBox, TOOLTIPTEXT* pTTT);

    void CreateToolTip();

    void SetButtonIcon(UINT nIDButton, UINT nIDIcon);

public:
    CPPageBase(UINT nIDTemplate, UINT nIDCaption = 0);
    virtual ~CPPageBase();

    // Dialog Data

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnSetActive();
    virtual BOOL OnApply();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnDestroy();
};
