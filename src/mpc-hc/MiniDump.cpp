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
#include <DbgHelp.h>
#include <atlpath.h>
#include "mplayerc.h"
#include "resource.h"
#include "WinAPIUtils.h"
#include "WinApiFunc.h"
#include "VersionInfo.h"
#include "mpc-hc_config.h"
#include "MiniDump.h"

void CMiniDump::Enable()
{
#ifndef _DEBUG
    SetUnhandledExceptionFilter(UnhandledExceptionFilter);
#endif
};

void CMiniDump::Disable()
{
#ifndef _DEBUG
    SetUnhandledExceptionFilter(nullptr);
#endif
};

#ifndef _DEBUG
LONG WINAPI CMiniDump::UnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers)
{
    LONG retval = EXCEPTION_CONTINUE_SEARCH;

#if ENABLE_MINIDUMP || ENABLE_FULLDUMP
    CString strResult;
    CPath   dumpPath;
    bool    bDumpCreated = false;

    const WinapiFunc<decltype(MiniDumpWriteDump)> fnMiniDumpWriteDump = { "DbgHelp.dll", "MiniDumpWriteDump" };

    if (fnMiniDumpWriteDump && AfxGetMyApp()->GetAppSavePath(dumpPath) && FileExists(dumpPath) || CreateDirectory(dumpPath, nullptr)) {
        dumpPath.Append(CString(AfxGetApp()->m_pszExeName) + _T(".exe.") + VersionInfo::GetVersionString() + _T(".dmp"));

        HANDLE hFile = ::CreateFile(dumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION ExInfo = { ::GetCurrentThreadId(), pExceptionPointers, FALSE };
            MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(
#if ENABLE_FULLDUMP
                                         MiniDumpWithFullMemory |
                                         MiniDumpWithHandleData |
                                         MiniDumpWithThreadInfo |
                                         MiniDumpWithProcessThreadData |
                                         MiniDumpWithFullMemoryInfo |
                                         MiniDumpWithUnloadedModules |
                                         MiniDumpIgnoreInaccessibleMemory |
                                         MiniDumpWithTokenInformation
#else
                                         MiniDumpWithHandleData |
                                         MiniDumpWithFullMemoryInfo |
                                         MiniDumpWithThreadInfo |
                                         MiniDumpWithUnloadedModules |
                                         MiniDumpWithProcessThreadData
#endif // ENABLE_FULLDUMP
                                     );

            bDumpCreated = !!fnMiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, dumpType, (pExceptionPointers ? &ExInfo : nullptr), nullptr, nullptr);

            ::CloseHandle(hFile);
        }
    }

    if (bDumpCreated) {
        strResult.Format(ResStr(IDS_MPC_CRASH), dumpPath);
        retval = EXCEPTION_EXECUTE_HANDLER;
    } else {
        strResult.Format(ResStr(IDS_MPC_MINIDUMP_FAIL), dumpPath, GetLastError());
    }

    switch (MessageBox(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), strResult, _T("MPC-HC - Mini Dump"), (bDumpCreated ? MB_YESNO : MB_OK) | MB_TOPMOST)) {
        case IDYES:
            ShellExecute(nullptr, _T("open"), BUGS_URL, nullptr, nullptr, SW_SHOWDEFAULT);
            ExploreToFile(dumpPath);
            break;
        case IDNO:
            retval = EXCEPTION_CONTINUE_SEARCH; // rethrow the exception to make easier attaching a debugger
            break;
    }
#else
    if (MessageBox(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), ResStr(IDS_MPC_BUG_REPORT), ResStr(IDS_MPC_BUG_REPORT_TITLE), MB_YESNO | MB_TOPMOST) == IDYES) {
        ShellExecute(nullptr, _T("open"), DOWNLOAD_URL, nullptr, nullptr, SW_SHOWDEFAULT);
    }
#endif // DISABLE_MINIDUMP

    return retval;
}
#endif
