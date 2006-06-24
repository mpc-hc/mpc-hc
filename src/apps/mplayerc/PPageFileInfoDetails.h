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
#include "..\..\subpic\ISubPic.h"
#include "afxwin.h"

// CPPageFileInfoDetails dialog

class CPPageFileInfoDetails : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPageFileInfoDetails)

private:
	CComPtr<IFilterGraph> m_pFG;
	CComPtr<ISubPicAllocatorPresenter> m_pCAP;

	HICON m_hIcon;
    
	void InitEncoding();

public:
	CPPageFileInfoDetails(CString fn, IFilterGraph* pFG, ISubPicAllocatorPresenter* pCAP);
	virtual ~CPPageFileInfoDetails();

// Dialog Data
	enum { IDD = IDD_FILEPROPDETAILS };

	CStatic m_icon;
	CString m_fn;
	CString m_type;
	CString m_size;
	CString m_time;
	CString m_res;
	CString m_created;
	CEdit m_encoding;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
};
