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

#include "PPageBase.h"
#include "FloatEdit.h"
#include "StaticLink.h"

// CPPageWebServer dialog

class CPPageWebServer : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageWebServer)

private:
	CString GetMPCDir();
	CString GetCurWebRoot();
	bool PickDir(CString& dir);

public:
	CPPageWebServer();
	virtual ~CPPageWebServer();

// Dialog Data
	enum { IDD = IDD_PPAGEWEBSERVER };
	BOOL m_fEnableWebServer;
	int m_nWebServerPort;
	CIntEdit m_nWebServerPortCtrl;
	CStaticLink m_launch;
	BOOL m_fWebServerPrintDebugInfo;
	BOOL m_fWebServerUseCompression;
	BOOL m_fWebServerLocalhostOnly;
	BOOL m_fWebRoot;
	CString m_WebRoot;
	CString m_WebServerCGI;
	CString m_WebDefIndex;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnUpdateButton2(CCmdUI* pCmdUI);
};
