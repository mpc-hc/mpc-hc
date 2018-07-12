/*
 * (C) 2015-2016 see Authors.txt
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
#include "CrashReporter.h"
#if !defined(_DEBUG) && USE_DRDUMP_CRASH_REPORTER
#include "VersionInfo.h"
#include "mpc-hc_config.h"
#include "DoctorDump/CrashRpt.h"
#include "mplayerc.h"


using namespace crash_rpt;

namespace CrashReporter
{
    static bool g_bEnabled = false;
    static CrashRpt g_crashReporter(L"CrashReporter\\crashrpt.dll");

    static CrashProcessingCallbackResult CALLBACK CrashProcessingCallback(CrashProcessingCallbackStage stage,
            ExceptionInfo* pExceptionInfo,
            LPVOID pUserData);
}
#endif

void CrashReporter::Enable(LPCTSTR langDll /*= nullptr*/)
{
#if !defined(_DEBUG) && USE_DRDUMP_CRASH_REPORTER
    crash_rpt::ApplicationInfo appInfo = {
        sizeof(appInfo),
        "31c48823-ce52-401b-8425-888388161757",
        "mpc-hc",
        L"MPC-HC",
        L"MPC-HC Team",
        {
            USHORT(VersionInfo::GetMajorNumber()),
            USHORT(VersionInfo::GetMinorNumber()),
            USHORT(VersionInfo::GetPatchNumber()),
            USHORT(VersionInfo::GetRevisionNumber()),
        },
        0,
        nullptr
    };

    const MINIDUMP_TYPE dumpType = MINIDUMP_TYPE(
#if ENABLE_FULLDUMP
                                       MiniDumpWithFullMemory |
                                       MiniDumpWithTokenInformation |
#else
                                       MiniDumpWithIndirectlyReferencedMemory |
                                       MiniDumpWithDataSegs |
#endif // ENABLE_FULLDUMP
                                       MiniDumpWithHandleData |
                                       MiniDumpWithThreadInfo |
                                       MiniDumpWithProcessThreadData |
                                       MiniDumpWithFullMemoryInfo |
                                       MiniDumpWithUnloadedModules |
                                       MiniDumpIgnoreInaccessibleMemory
                                   );

    crash_rpt::custom_data_collection::Settings dataCollectionSettings = {
        sizeof(dataCollectionSettings),
        _T("CrashReporter\\CrashReporterDialog.dll"),               // Path of the DLL
        "CreateCrashDialog",                                        // Function name
        (LPBYTE)langDll,                                            // No user-defined data
        langDll ? DWORD(_tcslen(langDll) + 1)* sizeof(TCHAR) : 0    // Size of user-defined data
    };

    crash_rpt::HandlerSettings handlerSettings = {
        sizeof(handlerSettings),
        FALSE,      // Don't keep the dumps
        FALSE,      // Don't open the problem page in the browser
        FALSE,      // Don't use WER (for now)
        0,          // Anonymous submitter
        FALSE,      // Ask before sending additional info
        TRUE,       // Override the "full" dump settings
        dumpType,   // "Full" dump custom settings
        nullptr,    // No lang file (for now)
        nullptr,    // Default path for SendRpt
        nullptr,    // Default path for DbgHelp
        CrashProcessingCallback,    // Callback function
        nullptr,    // No user defined parameter for the callback function
        &dataCollectionSettings // Use our custom dialog
    };

    g_bEnabled = g_crashReporter.InitCrashRpt(&appInfo, &handlerSettings);
#endif
}

void CrashReporter::Disable()
{
#if !defined(_DEBUG) && USE_DRDUMP_CRASH_REPORTER
    g_bEnabled = false;
#endif
}

bool CrashReporter::IsEnabled()
{
#if !defined(_DEBUG) && USE_DRDUMP_CRASH_REPORTER
    return g_bEnabled;
#else
    return false;
#endif
}

#if !defined(_DEBUG) && USE_DRDUMP_CRASH_REPORTER
CrashProcessingCallbackResult CALLBACK CrashReporter::CrashProcessingCallback(CrashProcessingCallbackStage stage,
        ExceptionInfo* pExceptionInfo,
        LPVOID pUserData)
{
    return g_bEnabled ? DoDefaultActions : SkipSendReportReturnDefaultResult;
}
#endif
