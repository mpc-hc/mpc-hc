/*
 * $Id$
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


// CPPageFileMediaInfo dialog

class CPPageFileMediaInfo : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPageFileMediaInfo)

public:
	CPPageFileMediaInfo(CString fn);   // standard constructor
	virtual ~CPPageFileMediaInfo();

// Dialog Data
	enum { IDD = IDD_FILEMEDIAINFO };

	CEdit m_mediainfo;	
	CString m_fn;
	CFont* m_pCFont;

	CString MI_Text;

	static bool HasMediaInfo();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
