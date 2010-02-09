/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of vsconv.
 *
 * Vsconv is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsconv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "vsconv.h"
#include "vsconvDlg.h"
#include "../../subtitles/VobSubFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CvsconvApp

BEGIN_MESSAGE_MAP(CvsconvApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CvsconvApp construction

CvsconvApp::CvsconvApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CvsconvApp object

CvsconvApp theApp;


// CvsconvApp initialization

BOOL CvsconvApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// TODO
//	if(__argc > 1)
	{
		CString in, out;
		CVobSubFile::SubFormat sf = CVobSubFile::None;
		int iLang = -1;
		bool fIgnoreForcedOnly = false;
		bool fForcedOnly = false;

		try
		{
			for(int i = 1; i < __argc; i++)
			{
				if(__targv[i][0] == '-' || __targv[i][0] == '/')
				{
					CString sw(&__targv[i][1]);

					if(sw == _T("f"))
					{
						if(++i < __argc && __targv[i][0] != '-' && __targv[i][0] != '/')
						{
							CString fmt = CString(__targv[i]).MakeLower();

							if(fmt == _T("winsubmux"))
								sf = CVobSubFile::WinSubMux;
							else if(fmt == _T("scenarist"))
								sf = CVobSubFile::Scenarist;
							else if(fmt == _T("maestro"))
								sf = CVobSubFile::Maestro;
							else
								throw _T("Unrecognized conversion format");
						}
						else
							throw _T("No conversion format given");
					}
					else if(sw == _T("i"))
					{
						if(++i < __argc && __targv[i][0] != '-' && __targv[i][0] != '/')
							in = __targv[i];
						else
							throw _T("Missing input file");
					}
					else if(sw == _T("o"))
					{
						if(++i < __argc && __targv[i][0] != '-' && __targv[i][0] != '/')
							out = __targv[i];
						else
							throw _T("Missing output file");
					}
					else if(sw == _T("id"))
					{
						if(++i < __argc && __targv[i][0] != '-' && __targv[i][0] != '/')
							iLang = _tcstol(__targv[i], NULL, 10);
						else
							throw _T("Missing stream id");
					}
					else if(sw == _T("ignoreforcedonly"))
					{
						fIgnoreForcedOnly = true;
					}
					else if(sw == _T("forcedonly"))
					{
						fForcedOnly = true;
					}
				}
			}

			if(!in.IsEmpty() && !out.IsEmpty() && sf != CVobSubFile::None)
			{
				CVobSubFile vsf(NULL);

				if(!vsf.Open(in))
					throw _T("Can't open input");

				if(iLang >= 0 && iLang < 32)
					vsf.m_iLang = iLang;

				if(fForcedOnly)
					vsf.m_fOnlyShowForcedSubs = true;

				if(fIgnoreForcedOnly)
					vsf.m_fOnlyShowForcedSubs = false;

				if(!vsf.Save(out, sf))
					throw _T("Can't save output");

				return FALSE;
			}
		}
		catch(LPCTSTR msg)
		{
			AfxMessageBox(CString(_T("Error: ")) + msg);
		}

		AfxMessageBox(
			_T("Usage: vsconv.exe <switches>\n\n")
			_T("-f \"format\" (winsubmux, scenarist, maestro)\n")
			_T("-i \"input\"\n")
			_T("-o \"output\"\n")
			_T("-id 0-31 (optional)\n")
			_T("-ignoreforcedonly (optional)\n")
			_T("-forcedonly (optional)\n")
			);

		return FALSE;
	}

	// TODO
	return FALSE;


	CvsconvDlg dlg;
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
