/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012, 2014 see Authors.txt
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

#pragma once

#include <atlcoll.h>

class CFontInstaller
{
    HANDLE(WINAPI* pAddFontMemResourceEx)(PVOID, DWORD, PVOID, DWORD*);
    int (WINAPI* pAddFontResourceEx)(LPCTSTR, DWORD, PVOID);
    BOOL (WINAPI* pRemoveFontMemResourceEx)(HANDLE);
    BOOL (WINAPI* pRemoveFontResourceEx)(LPCTSTR, DWORD, PVOID);
    BOOL (WINAPI* pMoveFileEx)(LPCTSTR, LPCTSTR, DWORD);

    CAtlList<HANDLE> m_fonts;
    CAtlList<CString> m_files;
    bool InstallFontFile(const void* pData, UINT len);

public:
    CFontInstaller();
    virtual ~CFontInstaller();

    bool InstallFont(const CAtlArray<BYTE>& data);
    bool InstallFont(const void* pData, UINT len);
    bool InstallFontMemory(const void* pData, UINT len);
    void UninstallFonts();
};
