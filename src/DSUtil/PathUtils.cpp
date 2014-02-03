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
#include "PathUtils.h"

namespace PathUtils
{
    CString BaseName(LPCTSTR path)
    {
        CPath cp(path);
        cp.RemoveBackslash();
        cp.StripPath();
        return cp;
    }

    CString DirName(LPCTSTR path)
    {
        CPath cp(path);
        cp.RemoveBackslash();
        cp.RemoveFileSpec();
        return cp;
    }

    CString FileName(LPCTSTR path)
    {
        CPath cp(path);
        cp.StripPath();
        cp.RemoveExtension();
        cp.RemoveBackslash();
        return cp;
    }

    CString FileExt(LPCTSTR path)
    {
        return CPath(path).GetExtension();
    }

    CString GetModulePath(HMODULE hModule)
    {
        CString ret;
        int pos, len = MAX_PATH - 1;
        for (;;) {
            pos = GetModuleFileName(hModule, ret.GetBuffer(len), len);
            if (pos == len) {
                // buffer was too small, enlarge it and try again
                len *= 2;
                ret.ReleaseBuffer(0);
                continue;
            }
            ret.ReleaseBuffer(pos);
            break;
        }
        ASSERT(!ret.IsEmpty());
        return ret;
    }

    CString GetAfxModulePath(bool bWithModuleName/* = false*/)
    {
        CString ret = GetModulePath(AfxGetInstanceHandle());
        if (!bWithModuleName) {
            ret = DirName(ret);
        }
        return ret;
    }

    CString GetProgramPath(bool bWithExeName/* = false*/)
    {
        CString ret = GetModulePath(nullptr);
        if (!bWithExeName) {
            ret = DirName(ret);
        }
        return ret;
    }

    CString CombinePaths(LPCTSTR dir, LPCTSTR path)
    {
        CPath cp;
        cp.Combine(dir, path);
        return cp;
    }

    CString FilterInvalidCharsFromFileName(LPCTSTR fn, TCHAR replacementChar /*= _T('_')*/)
    {
        CString ret = fn;
        int iLength = ret.GetLength();
        LPTSTR buff = ret.GetBuffer();

        for (int i = 0; i < iLength; i++) {
            switch (buff[i]) {
                case _T('<'):
                case _T('>'):
                case _T(':'):
                case _T('"'):
                case _T('/'):
                case _T('\\'):
                case _T('|'):
                case _T('?'):
                case _T('*'):
                    buff[i] = replacementChar;
                    break;
                default:
                    // Do nothing
                    break;
            }
        }

        ret.ReleaseBuffer();

        return ret;
    }

    bool IsInDir(LPCTSTR path, LPCTSTR dir)
    {
        return !!CPath(path).IsPrefix(dir);
    }

    CString ToRelative(LPCTSTR dir, const LPCTSTR path, bool* pbRelative/* = nullptr*/)
    {
        CPath cp;
        BOOL rel = cp.RelativePathTo(dir, FILE_ATTRIBUTE_DIRECTORY, path, 0);
        if (pbRelative) {
            *pbRelative = !!rel;
        }
        return cp;
    }

    bool IsRelative(LPCTSTR path)
    {
        return !!CPath(path).IsRelative();
    }

    bool Exists(LPCTSTR path)
    {
        return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
    }

    bool IsFile(LPCTSTR path)
    {
        DWORD attr = GetFileAttributes(path);
        return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool IsDir(LPCTSTR path)
    {
        DWORD attr = GetFileAttributes(path);
        return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool CreateDirRecursive(LPCTSTR path)
    {
        bool ret = IsDir(path) || CreateDirectory(path, nullptr);
        if (!ret) {
            ret = CreateDirRecursive(DirName(path)) && CreateDirectory(path, nullptr);
        }
        return ret;
    }
}
