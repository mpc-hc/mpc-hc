/*
 * $Id$
 *
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "mplayerc.h"
#include "MiniDump.h"
#include "resource.h"
#include <DbgHelp.h>
#include "Version.h"
#include "../DSUtil/WinAPIUtils.h"


CMiniDump	_Singleton;
bool		CMiniDump::m_bMiniDumpEnabled = true;


typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)( HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
		CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
										);


CMiniDump::CMiniDump()
{
#ifndef _DEBUG

	SetUnhandledExceptionFilter( UnhandledExceptionFilter );

//#ifndef _WIN64
	// Enable catching in CRT (http://blog.kalmbachnet.de/?postid=75)
	//	PreventSetUnhandledExceptionFilter();
//#endif
#endif
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter( LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter )
{
	return NULL;
}

BOOL CMiniDump::PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary( _T("kernel32.dll") );
	if ( hKernel32 == NULL ) {
		return FALSE;
	}

	void *pOrgEntry = GetProcAddress( hKernel32, "SetUnhandledExceptionFilter" );
	if ( pOrgEntry == NULL ) {
		return FALSE;
	}

	unsigned char newJump[ 100 ];
	DWORD_PTR dwOrgEntryAddr = (DWORD_PTR)pOrgEntry;
	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
	void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
	DWORD_PTR dwNewEntryAddr = (DWORD_PTR)pNewFunc;
	DWORD_PTR dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

	newJump[ 0 ] = 0xE9;  // JMP absolute
	memcpy( &newJump[ 1 ], &dwRelativeAddr, sizeof(pNewFunc) );
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory( GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten );
	FreeLibrary( hKernel32 );
	return bRet;
}

LONG WINAPI CMiniDump::UnhandledExceptionFilter( _EXCEPTION_POINTERS *lpTopLevelExceptionFilter )
{
	LONG    retval = EXCEPTION_CONTINUE_SEARCH;
	HMODULE hDll   = NULL;
	TCHAR   szResult[800];
	CString strDumpPath;

	if ( !m_bMiniDumpEnabled ) {
		return retval;
	}

	hDll = ::LoadLibrary( _T("dbghelp.dll") );

	if ( hDll != NULL ) {
		MINIDUMPWRITEDUMP pMiniDumpWriteDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if ( pMiniDumpWriteDump != NULL ) {
			if (!AfxGetMyApp()->IsIniValid()) {
				HRESULT hr = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, strDumpPath.GetBuffer(_MAX_PATH));
				if (FAILED(hr)) {
					return retval;
				}

				strDumpPath.ReleaseBuffer();
				strDumpPath.Append(_T("\\Media Player Classic\\"));
				strDumpPath.Append(AfxGetApp()->m_pszExeName);
				strDumpPath.Append(_T(".exe"));
			} else {
				GetModuleFileName(NULL, strDumpPath.GetBuffer(_MAX_PATH), _MAX_PATH);
				strDumpPath.ReleaseBuffer();
			}
			strDumpPath.AppendFormat(_T(".%d.%d.%d.%d.dmp"), MPC_VERSION_NUM);

			// create the file
			HANDLE hFile = ::CreateFile(strDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
										FILE_ATTRIBUTE_NORMAL, NULL);

			if ( hFile != INVALID_HANDLE_VALUE ) {
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = lpTopLevelExceptionFilter;
				ExInfo.ClientPointers = NULL;

				// write the dump
				BOOL bOK = pMiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
				if ( bOK ) {
					_stprintf_s( szResult, _countof(szResult), ResStr(IDS_MPC_CRASH), strDumpPath );
					retval = EXCEPTION_EXECUTE_HANDLER;
				} else {
					_stprintf_s( szResult, _countof(szResult), ResStr(IDS_MPC_MINIDUMP_FAIL), strDumpPath, GetLastError() );
				}

				::CloseHandle( hFile );
			} else {
				_stprintf_s( szResult, _countof(szResult), ResStr(IDS_MPC_MINIDUMP_FAIL), strDumpPath, GetLastError() );
			}
		}
		FreeLibrary( hDll );
	}

	if (szResult) {
		switch (MessageBox(NULL, szResult, _T("MPC-HC Mini Dump"), retval ? MB_YESNO : MB_OK)) {
			case IDYES:
				ShellExecute(NULL, _T("open"), _T("http://sourceforge.net/apps/trac/mpc-hc/wiki/Bugs_-_Reporting"), NULL, NULL, SW_SHOWDEFAULT);
				ExploreToFile(strDumpPath);
				break;
			case IDNO:
				retval = EXCEPTION_CONTINUE_SEARCH; // rethrow the exception to make easier attaching a debugger
				break;
		}
	}

	return retval;
}
