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

#include "stdafx.h"
#include "MiniDump.h"
#include "resource.h"
#include <DbgHelp.h>

#include "Version.h"


CMiniDump	_Singleton;
bool		CMiniDump::m_bMiniDumpEnabled = false;


typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)( HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
                                        );


CMiniDump::CMiniDump()
{
#ifndef _DEBUG

    SetUnhandledExceptionFilter( UnhandledExceptionFilter );

#ifndef _WIN64
    // Enable catching in CRT (http://blog.kalmbachnet.de/?postid=75)
//	PreventSetUnhandledExceptionFilter();
#endif
#endif
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter( LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter )
{
    return NULL;
}

BOOL CMiniDump::PreventSetUnhandledExceptionFilter()
{
    HMODULE hKernel32 = LoadLibrary( _T("kernel32.dll") );
    if ( hKernel32 == NULL )
        return FALSE;

    void *pOrgEntry = GetProcAddress( hKernel32, "SetUnhandledExceptionFilter" );
    if ( pOrgEntry == NULL )
        return FALSE;

    unsigned char newJump[ 100 ];
    DWORD dwOrgEntryAddr = (DWORD) pOrgEntry;
    dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
    void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
    DWORD dwNewEntryAddr = (DWORD) pNewFunc;
    DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

    newJump[ 0 ] = 0xE9;  // JMP absolute
    memcpy( &newJump[ 1 ], &dwRelativeAddr, sizeof(pNewFunc) );
    SIZE_T bytesWritten;
    BOOL bRet = WriteProcessMemory( GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten );
    FreeLibrary( hKernel32 );
    return bRet;
}

LONG WINAPI CMiniDump::UnhandledExceptionFilter( _EXCEPTION_POINTERS *lpTopLevelExceptionFilter )
{
    LONG	retval	= EXCEPTION_CONTINUE_SEARCH;
    HWND	hParent = NULL;
    HMODULE	hDll	= NULL;
    _TCHAR	szResult[ 800 ];
    _TCHAR	szDbgHelpPath[ _MAX_PATH ];

    if ( !m_bMiniDumpEnabled )
        return 0;

    // firstly see if dbghelp.dll is around and has the function we need
    // look next to the EXE first, as the one in System32 might be old
    // (e.g. Windows 2000)

    if ( GetModuleFileName(NULL, szDbgHelpPath, _MAX_PATH) )
    {
        _TCHAR *pSlash = _tcsrchr( szDbgHelpPath, _T('\\') );
        if ( pSlash != NULL )
        {
            _tcscpy_s( pSlash + 1, _MAX_PATH + szDbgHelpPath - pSlash, _T("DBGHELP.DLL") );
            hDll = ::LoadLibrary( szDbgHelpPath );
        }
    }

    if ( hDll == NULL )
    {
        // load any version we can
        hDll = ::LoadLibrary( _T("DBGHELP.DLL") );
    }

    if ( hDll != NULL )
    {
        MINIDUMPWRITEDUMP pMiniDumpWriteDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
        if ( pMiniDumpWriteDump != NULL )
        {
            _TCHAR szDumpPath[ _MAX_PATH ];
            _TCHAR szVersion[ 40 ];

            GetModuleFileName( NULL, szDumpPath, _MAX_PATH );
            _stprintf_s( szVersion, countof(szVersion), _T(".%d.%d.%d.%d"), VERSION_MAJOR, VERSION_MINOR, VERSION_REV, VERSION_PATCH );
            _tcscat_s( szDumpPath, _MAX_PATH, szVersion );
            _tcscat_s( szDumpPath, _MAX_PATH, _T(".dmp") );

            // create the file
            HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL, NULL );

            if ( hFile != INVALID_HANDLE_VALUE )
            {
                _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

                ExInfo.ThreadId = ::GetCurrentThreadId();
                ExInfo.ExceptionPointers = lpTopLevelExceptionFilter;
                ExInfo.ClientPointers = NULL;

                // write the dump
                BOOL bOK = pMiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
                if ( bOK )
                {
                    _stprintf_s( szResult, countof(szResult), ResStr(IDS_MPC_CRASH), szDumpPath );
                    retval = EXCEPTION_EXECUTE_HANDLER;
                }
                else
                {
                    _stprintf_s( szResult, countof(szResult), ResStr(IDS_MPC_MINIDUMP_FAIL), szDumpPath, GetLastError() );
                }

                ::CloseHandle( hFile );
            }
            else
            {
                _stprintf_s( szResult, countof(szResult), ResStr(IDS_MPC_MINIDUMP_FAIL), szDumpPath, GetLastError() );
            }
        }
        FreeLibrary( hDll );
    }

    if ( szResult )
        MessageBox( NULL, szResult, _T("MPC-HC Mini Dump"), MB_OK );

    return retval;
}
