/*
 * $Id$
 *
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

#include "stdafx.h"
#include "resource.h"
#include "PlayerBar.h"


IMPLEMENT_DYNAMIC(CPlayerBar, CSizingControlBarG)
CPlayerBar::CPlayerBar(void)
{
}

CPlayerBar::~CPlayerBar(void)
{
}

BEGIN_MESSAGE_MAP(CPlayerBar, CSizingControlBarG)
END_MESSAGE_MAP()

BOOL CPlayerBar::Create(LPCTSTR lpszWindowName, CWnd* pParentWnd, UINT nID, UINT defDockBarID, CString const& strSettingName)
{
	m_defDockBarID = defDockBarID;
	m_strSettingName = strSettingName;

	return __super::Create(lpszWindowName, pParentWnd, nID);
}

void CPlayerBar::LoadState(CFrameWnd *pParent)
{
	CWinApp* pApp = AfxGetApp();

	CRect r;
	pParent->GetWindowRect(r);
	CRect rDesktop;
	GetDesktopWindow()->GetWindowRect(&rDesktop);

	CString section = _T("ToolBars\\") + m_strSettingName;

	__super::LoadState(section + _T("\\State"));

	UINT dockBarID = pApp->GetProfileInt(section, _T("DockState"), m_defDockBarID);

	if (dockBarID == AFX_IDW_DOCKBAR_FLOAT) {
		CPoint p;
		p.x = pApp->GetProfileInt(section, _T("DockPosX"), r.right);
		p.y = pApp->GetProfileInt(section, _T("DockPosY"), r.top);
		if (p.x < rDesktop.left) {
			p.x = rDesktop.left;
		}
		if (p.y < rDesktop.top) {
			p.y = rDesktop.top;
		}
		if (p.x >= rDesktop.right) {
			p.x = rDesktop.right-1;
		}
		if (p.y >= rDesktop.bottom) {
			p.y = rDesktop.bottom-1;
		}
		pParent->FloatControlBar(this, p);
	} else {
		pParent->DockControlBar(this, dockBarID);
	}
}

void CPlayerBar::SaveState()
{
	CWinApp* pApp = AfxGetApp();

	CString section = _T("ToolBars\\") + m_strSettingName;

	__super::SaveState(section + _T("\\State"));

	UINT dockBarID = GetParent()->GetDlgCtrlID();

	if (dockBarID == AFX_IDW_DOCKBAR_FLOAT) {
		CRect r;
		GetParent()->GetParent()->GetWindowRect(r);
		pApp->WriteProfileInt(section, _T("DockPosX"), r.left);
		pApp->WriteProfileInt(section, _T("DockPosY"), r.top);
	}

	pApp->WriteProfileInt(section, _T("DockState"), dockBarID);
}
