/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of asf2mkv.
 *
 * Asf2mkv is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Asf2mkv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "asf2mkv.h"
#include "asf2mkvDlg.h"
#include "asf2mkv.h"


// Casf2mkvApp

BEGIN_MESSAGE_MAP(Casf2mkvApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// Casf2mkvApp construction

Casf2mkvApp::Casf2mkvApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Casf2mkvApp object

Casf2mkvApp theApp;


// Casf2mkvApp initialization

BOOL Casf2mkvApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Gabest"));

	HRESULT hr;
	if(FAILED(hr = OleInitialize(0)))
	{
		AfxMessageBox(_T("OleInitialize failed!"));
		return FALSE;
	}

	Casf2mkvDlg dlg;
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

int Casf2mkvApp::ExitInstance()
{
	OleUninitialize();

	return CWinApp::ExitInstance();
}
