/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of vsrip.
 *
 * Vsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once
#include <afxwin.h>
#include "VSRipPage.h"

// CVSRipFileDlg dialog

class CVSRipFileDlg : public CVSRipPage
{
	DECLARE_DYNAMIC(CVSRipFileDlg)

protected:
	// IVSFRipperCallback
	STDMETHODIMP OnMessage(LPCTSTR msg);

public:
	CVSRipFileDlg(IVSFRipper* pVSFRipper, CWnd* pParent = NULL);   // standard constructor
	virtual ~CVSRipFileDlg();

	virtual bool CanGoPrev()
	{
		return(false);
	}
	virtual bool CanGoNext()
	{
		return(!m_infn.IsEmpty() && !m_outfn.IsEmpty());
	}
	virtual CString GetHeaderText()
	{
		return(_T("Select input and output"));
	}
	virtual CString GetDescText()
	{
		return(_T("First choose a video title set ifo, then select an ")
			   _T("output path for the idx/sub files. Make sure the vob files ")
			   _T("have some standard naming, this util can't read your mind."));
	}

// Dialog Data
	enum { IDD = IDD_DIALOG_FILE };
	CEdit m_log;
	CString m_infn, m_outfn;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
