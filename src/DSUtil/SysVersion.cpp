/*
 * (C) 2012-2014 see Authors.txt
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
#include "SysVersion.h"

static OSVERSIONINFOEX InitFullVersion()
{
    OSVERSIONINFOEX fullVersion;
    ZeroMemory(&fullVersion, sizeof(OSVERSIONINFOEX));
    fullVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((LPOSVERSIONINFO)&fullVersion);

    return fullVersion;
}

static bool InitIs64Bit()
{
#if defined(_WIN64)
    return true;  // 64-bit programs run only on Win64
#else
    // 32-bit programs run on both 32-bit and 64-bit Windows
    // so must sniff
    BOOL f64 = FALSE;
    return (IsWow64Process(GetCurrentProcess(), &f64) && f64);
#endif
}

OSVERSIONINFOEX SysVersion::GetFullVersion()
{
    static const OSVERSIONINFOEX fullVersion = InitFullVersion();
    return fullVersion;
}

DWORD SysVersion::GetVersion()
{
    static const DWORD version = MAKEWORD(GetFullVersion().dwMinorVersion, GetFullVersion().dwMajorVersion);
    return version;
}

bool SysVersion::IsXPOrLater()
{
    return (GetVersion() >= 0x0501);
}

bool SysVersion::IsVista()
{
    return (GetVersion() == 0x0600);
}

bool SysVersion::IsVistaOrLater()
{
    return (GetVersion() >= 0x0600);
}

bool SysVersion::Is7()
{
    return (GetVersion() == 0x0601);
}

bool SysVersion::Is7OrLater()
{
    return (GetVersion() >= 0x0601);
}

bool SysVersion::Is8()
{
    return (GetVersion() == 0x0602);
}

bool SysVersion::Is8OrLater()
{
    return (GetVersion() >= 0x0602);
}

bool SysVersion::Is64Bit()
{
    const bool bIs64Bit = InitIs64Bit();
    return bIs64Bit;
}
