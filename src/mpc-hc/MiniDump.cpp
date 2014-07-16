/*
 * (C) 2008-2014 see Authors.txt
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
#include <atlpath.h>
#include "mplayerc.h"
#include "MiniDump.h"
#include "resource.h"
#include <DbgHelp.h>
#include "mpc-hc_config.h"
#include "VersionInfo.h"
#include "WinAPIUtils.h"


CMiniDump _Singleton;


typedef BOOL (WINAPI* MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
                                        );


CMiniDump::CMiniDump()
{
#ifndef _DEBUG

    Enable();

    //#ifndef _WIN64
    // Enable catching in CRT (http://blog.kalmbachnet.de/?postid=75)
    //  PreventSetUnhandledExceptionFilter();
    //#endif
#endif
}

CMiniDump::~CMiniDump()
{
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
    return nullptr;
}

BOOL CMiniDump::PreventSetUnhandledExceptionFilter()
{
    HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
    if (hKernel32 == nullptr) {
        return FALSE;
    }

    void* pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
    if (pOrgEntry == nullptr) {
        FreeLibrary(hKernel32);
        return FALSE;
    }

    unsigned char newJump[100];
    DWORD_PTR dwOrgEntryAddr = (DWORD_PTR)pOrgEntry;
    dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
    void* pNewFunc = &MyDummySetUnhandledExceptionFilter;
    DWORD_PTR dwNewEntryAddr = (DWORD_PTR)pNewFunc;
    DWORD_PTR dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

    newJump[0] = 0xE9;  // JMP absolute
    memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
    SIZE_T bytesWritten;
    BOOL bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
    FreeLibrary(hKernel32);
    return bRet;
}

LONG WINAPI CMiniDump::UnhandledExceptionFilter(_EXCEPTION_POINTERS* lpTopLevelExceptionFilter)
{
    LONG    retval = EXCEPTION_CONTINUE_SEARCH;
    BOOL    bDumpCreated = FALSE;
    HMODULE hDll = nullptr;
    TCHAR   szResult[800];
    szResult[0] = _T('\0');
    CPath   dumpPath;

#if ENABLE_MINIDUMP
    hDll = ::LoadLibrary(_T("dbghelp.dll"));

    if (hDll != nullptr) {
        MINIDUMPWRITEDUMP pMiniDumpWriteDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
        if (pMiniDumpWriteDump != nullptr && AfxGetMyApp()->GetAppSavePath(dumpPath)) {
            // Check that the folder actually exists
            if (!FileExists(dumpPath)) {
                VERIFY(CreateDirectory(dumpPath, nullptr));
            }

            CString strDumpName = AfxGetApp()->m_pszExeName;
            strDumpName.Append(_T(".exe.") + VersionInfo::GetVersionString() + _T(".dmp"));
            dumpPath.Append(strDumpName);

            // create the file
            HANDLE hFile = ::CreateFile(dumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL, nullptr);

            if (hFile != INVALID_HANDLE_VALUE) {
                _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

                ExInfo.ThreadId = ::GetCurrentThreadId();
                ExInfo.ExceptionPointers = lpTopLevelExceptionFilter;
                ExInfo.ClientPointers = FALSE;

                // write the dump
                bDumpCreated = pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, nullptr, nullptr);
                if (bDumpCreated) {
                    _stprintf_s(szResult, _countof(szResult), ResStr(IDS_MPC_CRASH), dumpPath);
                    retval = EXCEPTION_EXECUTE_HANDLER;
                } else {
                    _stprintf_s(szResult, _countof(szResult), ResStr(IDS_MPC_MINIDUMP_FAIL), dumpPath, GetLastError());
                }

                ::CloseHandle(hFile);
            } else {
                _stprintf_s(szResult, _countof(szResult), ResStr(IDS_MPC_MINIDUMP_FAIL), dumpPath, GetLastError());
            }
        }
        FreeLibrary(hDll);
    }

    if (szResult[0]) {
        switch (MessageBox(nullptr, szResult, _T("MPC-HC - Mini Dump"), (bDumpCreated ? MB_YESNO : MB_OK) | MB_TOPMOST)) {
            case IDYES:
                ShellExecute(nullptr, _T("open"), BUGS_URL, nullptr, nullptr, SW_SHOWDEFAULT);
                ExploreToFile(dumpPath);
                break;
            case IDNO:
                retval = EXCEPTION_CONTINUE_SEARCH; // rethrow the exception to make easier attaching a debugger
                break;
        }
    }
#else
    if (MessageBox(nullptr, ResStr(IDS_MPC_BUG_REPORT), ResStr(IDS_MPC_BUG_REPORT_TITLE), MB_YESNO | MB_TOPMOST) == IDYES) {
        ShellExecute(nullptr, _T("open"), DOWNLOAD_URL, nullptr, nullptr, SW_SHOWDEFAULT);
    }
#endif // DISABLE_MINIDUMP

    return retval;
}
