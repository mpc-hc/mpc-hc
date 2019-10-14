/*
 * (C) 2010-2014 see Authors.txt
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

#include "PlayerNavigationDialog.h"
#include "CMPCThemePlayerBar.h"

// CPlayerNavigationBar

class CMainFrame;

class CPlayerNavigationBar : public CMPCThemePlayerBar
{
    DECLARE_DYNAMIC(CPlayerNavigationBar)

private:
    CWnd* m_pParent;

public:
    CPlayerNavigationDialog m_navdlg;

    CPlayerNavigationBar() = delete;
    explicit CPlayerNavigationBar(CMainFrame* pMainFrame);
    virtual ~CPlayerNavigationBar() = default;

    BOOL Create(CWnd* pParentWnd, UINT defDockBarID);
    virtual void ReloadTranslatableResources();

    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
};
