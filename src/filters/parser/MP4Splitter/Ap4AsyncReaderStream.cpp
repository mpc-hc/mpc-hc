/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "Ap4AsyncReaderStream.h"

AP4_AsyncReaderStream::AP4_AsyncReaderStream(CBaseSplitterFile* pFile)
    : m_refs(1)
    , m_pFile(pFile)
{
    ASSERT(pFile);
}

AP4_AsyncReaderStream::~AP4_AsyncReaderStream()
{
    ASSERT(m_refs == 0);
}

void AP4_AsyncReaderStream::AddReference()
{
    ASSERT(m_refs > 0);
    ++m_refs;
}

void AP4_AsyncReaderStream::Release()
{
    ASSERT(m_refs > 0);
    if (--m_refs == 0) {
        delete this;
    }
}

AP4_Result AP4_AsyncReaderStream::Read(void* buffer, AP4_Size bytesToRead, AP4_Size* bytesRead)
{
    __int64 bytesAvail = m_pFile->GetRemaining();

    if (bytesAvail < (LONGLONG)bytesToRead) {
        if (bytesRead) {
            *bytesRead = bytesAvail;
        }
        bytesToRead = bytesAvail;
    }

    if (bytesAvail == 0) {
        return AP4_ERROR_EOS;
    }

    if (FAILED(m_pFile->ByteRead((BYTE*)buffer, bytesToRead))) {
        if (bytesRead) {
            *bytesRead = 0;
        }
        return AP4_ERROR_READ_FAILED;
    }

    if (bytesRead) {
        *bytesRead = bytesToRead;
    }

    return AP4_SUCCESS;
}

AP4_Result AP4_AsyncReaderStream::Write(const void* buffer, AP4_Size bytesToWrite, AP4_Size* bytesWritten)
{
    return AP4_ERROR_WRITE_FAILED;
}

AP4_Result AP4_AsyncReaderStream::Seek(AP4_Offset offset)
{
    m_pFile->Seek(offset);
    return m_pFile->GetPos() == offset ? AP4_SUCCESS : AP4_FAILURE;
}

AP4_Result AP4_AsyncReaderStream::Tell(AP4_Offset& offset)
{
    offset = (AP4_Offset)m_pFile->GetPos();
    return AP4_SUCCESS;
}

AP4_Result AP4_AsyncReaderStream::GetSize(AP4_Size& size)
{
    size = (AP4_Size)m_pFile->GetLength();
    return AP4_SUCCESS;
}
