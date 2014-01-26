/*
 * (C) 2013-2014 see Authors.txt
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
#include "VersionInfo.h"
#include "version.h"

bool VersionInfo::IsNightly()
{
#if MPC_NIGHTLY_RELEASE
    return true;
#else
    return false;
#endif
}

CString VersionInfo::GetNightlyWord()
{
#if MPC_NIGHTLY_RELEASE
    return MPC_VERSION_NIGHTLY;
#else
    return _T("");
#endif
}

bool VersionInfo::Is64Bit()
{
#ifdef _WIN64
    return true;
#else
    return false;
#endif
}

CString VersionInfo::GetVersionString()
{
    return MPC_VERSION_STR;
}

CString VersionInfo::GetFullVersionString()
{
    return MPC_VERSION_STR_FULL;
}

CString VersionInfo::GetBuildDateString()
{
    return _T(__DATE__) _T(" ") _T(__TIME__);
}

unsigned VersionInfo::GetMajorNumber()
{
    return MPC_VERSION_MAJOR;
}

unsigned VersionInfo::GetMinorNumber()
{
    return MPC_VERSION_MINOR;
}

unsigned VersionInfo::GetPatchNumber()
{
    return MPC_VERSION_PATCH;
}

unsigned VersionInfo::GetRevisionNumber()
{
    return MPC_VERSION_REV;
}
