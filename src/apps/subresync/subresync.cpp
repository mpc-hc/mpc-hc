/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of subresync.
 *
 * Subresync is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Subresync is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "subresync.h"
#include "subresyncDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSubresyncApp

BEGIN_MESSAGE_MAP(CSubresyncApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSubresyncApp construction

CSubresyncApp::CSubresyncApp()
{
}


// The one and only CSubresyncApp object

CSubresyncApp theApp;


// CSubresyncApp initialization

BOOL CSubresyncApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	SetRegistryKey(_T("Gabest"));

	CoInitialize(NULL);

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	CSubresyncDlg dlg(cmdInfo.m_strFileName);
	m_pMainWnd = &dlg;

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CSubresyncApp::ExitInstance()
{
	CoUninitialize();

	return CWinApp::ExitInstance();
}
