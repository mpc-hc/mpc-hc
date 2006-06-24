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

#include "afxwin.h"

// CSelectMediaType dialog

class CSelectMediaType : public CCmdUIDialog
{
	DECLARE_DYNAMIC(CSelectMediaType)

private:
	CAtlArray<GUID>& m_guids;

public:
	CSelectMediaType(CAtlArray<GUID>& guids, GUID guid, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectMediaType();

	GUID m_guid;

// Dialog Data
	enum { IDD = IDD_SELECTMEDIATYPE };
	CString m_guidstr;
	CComboBox m_guidsctrl;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnEditchangeCombo1();
	afx_msg void OnUpdateOK(CCmdUI* pCmdUI);
};
