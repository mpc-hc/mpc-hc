/*
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

// CCmdUIDialog dialog

#include <afxdlgs.h>

class CCmdUIDialog : public CDialog
{
	DECLARE_DYNAMIC(CCmdUIDialog)

public:
	CCmdUIDialog();
	CCmdUIDialog(UINT nIDTemplate, CWnd* pParent = NULL);
	CCmdUIDialog(LPCTSTR lpszTemplateName, CWnd* pParent = NULL);
	virtual ~CCmdUIDialog();

protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnKickIdle();
	afx_msg void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);
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

