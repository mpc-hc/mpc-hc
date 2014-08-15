/*
 * (C) 2014 see Authors.txt
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
#include <stdio.h>
#include "StdioFile64.h"


IMPLEMENT_DYNAMIC(CStdioFile64, CStdioFile)

ULONGLONG CStdioFile64::GetPosition() const
{
    ASSERT_VALID(this);
    ASSERT(m_pStream != nullptr);

    long long pos = _ftelli64(m_pStream);
    if (pos == -1) {
        AfxThrowFileException(CFileException::invalidFile, _doserrno, m_strFileName);
    }
    return pos;
}

ULONGLONG CStdioFile64::GetLength() const
{
    ASSERT_VALID(this);

    LONGLONG nCurrent;
    LONGLONG nLength;
    LONG nResult;

    nCurrent = _ftelli64(m_pStream);
    if (nCurrent == -1) {
        AfxThrowFileException(CFileException::invalidFile, _doserrno, m_strFileName);
    }

    nResult = _fseeki64(m_pStream, 0, SEEK_END);
    if (nResult != 0) {
        AfxThrowFileException(CFileException::badSeek, _doserrno, m_strFileName);
    }

    nLength = _ftelli64(m_pStream);
    if (nLength == -1) {
        AfxThrowFileException(CFileException::invalidFile, _doserrno, m_strFileName);
    }
    nResult = _fseeki64(m_pStream, nCurrent, SEEK_SET);
    if (nResult != 0) {
        AfxThrowFileException(CFileException::badSeek, _doserrno, m_strFileName);
    }

    return nLength;
}

ULONGLONG CStdioFile64::Seek(LONGLONG lOff, UINT nFrom)
{
    ASSERT_VALID(this);
    ASSERT(nFrom == begin || nFrom == end || nFrom == current);
    ASSERT(m_pStream != nullptr);

    if (_fseeki64(m_pStream, lOff, nFrom) != 0) {
        AfxThrowFileException(CFileException::badSeek, _doserrno, m_strFileName);
    }

    long long pos = _ftelli64(m_pStream);
    return pos;
}
