/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "FontInstaller.h"

CFontInstaller::CFontInstaller()
    : pAddFontMemResourceEx(nullptr)
    , pAddFontResourceEx(nullptr)
    , pRemoveFontMemResourceEx(nullptr)
    , pRemoveFontResourceEx(nullptr)
    , pMoveFileEx(nullptr)
{
    if (HMODULE hGdi = GetModuleHandle(_T("gdi32.dll"))) {
        pAddFontMemResourceEx = (HANDLE(WINAPI*)(PVOID, DWORD, PVOID, DWORD*))GetProcAddress(hGdi, "AddFontMemResourceEx");
        pAddFontResourceEx = (int (WINAPI*)(LPCTSTR, DWORD, PVOID))GetProcAddress(hGdi, "AddFontResourceExW");
        pRemoveFontMemResourceEx = (BOOL (WINAPI*)(HANDLE))GetProcAddress(hGdi, "RemoveFontMemResourceEx");
        pRemoveFontResourceEx = (BOOL (WINAPI*)(LPCTSTR, DWORD, PVOID))GetProcAddress(hGdi, "RemoveFontResourceExW");
    }

    if (HMODULE hGdi = GetModuleHandle(_T("kernel32.dll"))) {
        pMoveFileEx = (BOOL (WINAPI*)(LPCTSTR, LPCTSTR, DWORD))GetProcAddress(hGdi, "MoveFileExW");
    }
}

CFontInstaller::~CFontInstaller()
{
    UninstallFonts();
}

bool CFontInstaller::InstallFont(const CAtlArray<BYTE>& data)
{
    return InstallFont(data.GetData(), (UINT)data.GetCount());
}

bool CFontInstaller::InstallFont(const void* pData, UINT len)
{
    return InstallFontFile(pData, len) || InstallFontMemory(pData, len);
}

void CFontInstaller::UninstallFonts()
{
    if (pRemoveFontMemResourceEx) {
        POSITION pos = m_fonts.GetHeadPosition();
        while (pos) {
            pRemoveFontMemResourceEx(m_fonts.GetNext(pos));
        }
        m_fonts.RemoveAll();
    }

    if (pRemoveFontResourceEx) {
        POSITION pos = m_files.GetHeadPosition();
        while (pos) {
            CString fn = m_files.GetNext(pos);
            pRemoveFontResourceEx(fn, FR_PRIVATE, 0);
            if (!DeleteFile(fn) && pMoveFileEx) {
                pMoveFileEx(fn, nullptr, MOVEFILE_DELAY_UNTIL_REBOOT);
            }
        }

        m_files.RemoveAll();
    }
}

bool CFontInstaller::InstallFontMemory(const void* pData, UINT len)
{
    if (!pAddFontMemResourceEx) {
        return false;
    }

    DWORD nFonts = 0;
    HANDLE hFont = pAddFontMemResourceEx((PVOID)pData, len, nullptr, &nFonts);
    if (hFont && nFonts > 0) {
        m_fonts.AddTail(hFont);
    }
    return hFont && nFonts > 0;
}

bool CFontInstaller::InstallFontFile(const void* pData, UINT len)
{
    if (!pAddFontResourceEx) {
        return false;
    }

    CFile f;
    TCHAR path[MAX_PATH], fn[MAX_PATH];
    if (!GetTempPath(MAX_PATH, path) || !GetTempFileName(path, _T("g_font"), 0, fn)) {
        return false;
    }

    if (f.Open(fn, CFile::modeWrite)) {
        f.Write(pData, len);
        f.Close();

        if (pAddFontResourceEx(fn, FR_PRIVATE, 0) > 0) {
            m_files.AddTail(fn);
            return true;
        }
    }

    DeleteFile(fn);
    return false;
}
