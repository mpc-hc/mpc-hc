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
#include "FileVersionInfo.h"

bool FileVersionInfo::LoadInfo(LPCTSTR filePath, VS_FIXEDFILEINFO& fileInfo)
{
    bool success = false;

    // Get the buffer size required for the version information
    DWORD dwFileVersionInfoSize = GetFileVersionInfoSize(filePath, nullptr);
    if (dwFileVersionInfoSize) {
        // Allocate the buffer
        BYTE* lpData = (BYTE*)DEBUG_NEW BYTE[dwFileVersionInfoSize];
        if (lpData) {
            // Load the file-version information
            if (GetFileVersionInfo(filePath, 0, dwFileVersionInfoSize, (LPVOID)lpData)) {
                // Parse the version information
                VS_FIXEDFILEINFO* lpInfo;
                UINT unInfoLen;
                if (VerQueryValue((LPVOID)lpData, _T("\\"), (LPVOID*)&lpInfo, &unInfoLen)
                        && unInfoLen == sizeof(VS_FIXEDFILEINFO)) {
                    fileInfo = *lpInfo;
                    success = true;
                }
            }

            delete [] lpData;
        }
    }

    return success;
}

CString FileVersionInfo::GetFileVersionStr(LPCTSTR filePath)
{
    VS_FIXEDFILEINFO fileInfo;
    CString strFileVersion;

    if (LoadInfo(filePath, fileInfo)) {
        strFileVersion = FormatVersionString(fileInfo.dwFileVersionLS, fileInfo.dwFileVersionMS);
    }

    return strFileVersion;
}

QWORD FileVersionInfo::GetFileVersionNum(LPCTSTR filePath)
{
    VS_FIXEDFILEINFO fileInfo;
    QWORD qwFileVersion = 0;

    if (LoadInfo(filePath, fileInfo)) {
        qwFileVersion = ((QWORD)fileInfo.dwFileVersionMS << 32) | fileInfo.dwFileVersionLS;
    }

    return qwFileVersion;
}

CString FileVersionInfo::FormatVersionString(DWORD dwVersionNumberLow, DWORD dwVersionNumberHigh)
{
    CString strFileVersion;
    strFileVersion.Format(_T("%lu.%lu.%lu.%lu"),
                          (dwVersionNumberHigh & 0xFFFF0000) >> 16,
                          (dwVersionNumberHigh & 0x0000FFFF),
                          (dwVersionNumberLow & 0xFFFF0000) >> 16,
                          (dwVersionNumberLow & 0x0000FFFF));
    return strFileVersion;
}

CString FileVersionInfo::FormatVersionString(QWORD qwVersionNumber)
{
    return FormatVersionString(qwVersionNumber & DWORD_MAX, (qwVersionNumber >> 32) & DWORD_MAX);
}
