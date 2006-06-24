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

#include <afxwin.h>
#include <atlcoll.h>
#include "IGraphBuilder2.h"

// CMediaTypesDlg dialog

class CMediaTypesDlg : public CResizableDialog
{
//	DECLARE_DYNAMIC(CMediaTypesDlg)

private:
	CComPtr<IGraphBuilderDeadEnd> m_pGBDE;
	enum {UNKNOWN, VIDEO, AUDIO} m_type;
	GUID m_subtype;
	void AddLine(CString str = _T("\n"));
	void AddMediaType(AM_MEDIA_TYPE* pmt);

public:
	CMediaTypesDlg(IGraphBuilderDeadEnd* pGBDE, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMediaTypesDlg();

// Dialog Data
	enum { IDD = IDD_MEDIATYPES_DLG };
	CComboBox m_pins;
	CEdit m_report;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnUpdateButton1(CCmdUI* pCmdUI);
};
