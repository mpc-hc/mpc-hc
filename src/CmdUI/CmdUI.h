/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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

// CCmdUI dialog

#include <afxdlgs.h>

class CCmdUIDialog : public CDialog
{
    DECLARE_DYNAMIC(CCmdUIDialog)

public:
    CCmdUIDialog();
    CCmdUIDialog(UINT nIDTemplate, CWnd* pParent = nullptr);
    CCmdUIDialog(LPCTSTR lpszTemplateName, CWnd* pParent = nullptr);
    virtual ~CCmdUIDialog();

protected:
    virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnKickIdle();
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
};

// CCmdUIPropertyPage

class CCmdUIPropertyPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CCmdUIPropertyPage)

public:
    CCmdUIPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0);   // standard constructor
    virtual ~CCmdUIPropertyPage();

protected:
    virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnKickIdle();
};
