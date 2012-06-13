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

#pragma once

#include "../BaseSplitter/BaseSplitter.h"

#include "Ap4.h"
#include "Ap4File.h"
#include "Ap4ByteStream.h"

class AP4_AsyncReaderStream : public AP4_ByteStream
{
    int m_refs;
    CBaseSplitterFile* m_pFile;

public:
    AP4_AsyncReaderStream(CBaseSplitterFile* pFile);
    virtual ~AP4_AsyncReaderStream();

    void AddReference();
    void Release();

    AP4_Result Read(void* buffer, AP4_Size bytesToRead, AP4_Size* bytesRead);
    AP4_Result Write(const void* buffer, AP4_Size bytesToWrite, AP4_Size* bytesWritten);
    AP4_Result Seek(AP4_Offset offset);
    AP4_Result Tell(AP4_Offset& offset);
    AP4_Result GetSize(AP4_Size& size);
};
