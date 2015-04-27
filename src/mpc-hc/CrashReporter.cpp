/*
 * (C) 2015 see Authors.txt
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
#include "CrashReporter.h"
#include "VersionInfo.h"
#include "mpc-hc_config.h"
#include "DoctorDump/CrashRpt.h"

#ifndef _DEBUG
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

void CrashReporter::Enable()
{
#ifndef _DEBUG
    static crash_rpt::ApplicationInfo appInfo = {
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

    static crash_rpt::HandlerSettings handlerSettings = {
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
        nullptr     // No user defined parameter for the callback function
    };

    if (!g_crashReporter.IsCrashHandlingEnabled()) {
        g_bEnabled = g_crashReporter.InitCrashRpt(&appInfo, &handlerSettings);
    } else {
        g_bEnabled = true;
    }
#endif
};

void CrashReporter::Disable()
{
#ifndef _DEBUG
    g_bEnabled = false;
#endif
};

#ifndef _DEBUG
CrashProcessingCallbackResult CALLBACK CrashReporter::CrashProcessingCallback(CrashProcessingCallbackStage stage,
        ExceptionInfo* pExceptionInfo,
        LPVOID pUserData)
{
    return g_bEnabled ? DoDefaultActions : SkipSendReportReturnDefaultResult;
};
#endif
