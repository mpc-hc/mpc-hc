/*****************************************************************
|
|    AP4 - Data Buffer
|
|    Copyright 2002-2005 Gilles Boccon-Gibod & Julien Boeuf
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4DataBuffer.h"

/*----------------------------------------------------------------------
|       AP4_DataBuffer::AP4_DataBuffer
+---------------------------------------------------------------------*/
AP4_DataBuffer::AP4_DataBuffer() :
    m_BufferIsLocal(true),
    m_Buffer(NULL),
    m_BufferSize(0),
    m_DataSize(0)
{
}

/*----------------------------------------------------------------------
|       AP4_DataBuffer::AP4_DataBuffer
+---------------------------------------------------------------------*/
AP4_DataBuffer::AP4_DataBuffer(AP4_Size buffer_size) :
    m_BufferIsLocal(true),
    m_Buffer(NULL),
    m_BufferSize(buffer_size),
    m_DataSize(0)
{
    m_Buffer = new AP4_Byte[buffer_size];
}

/*----------------------------------------------------------------------
|       AP4_DataBuffer::AP4_DataBuffer
+---------------------------------------------------------------------*/
AP4_DataBuffer::AP4_DataBuffer(const AP4_DataBuffer& other) :
    m_BufferIsLocal(true),
    m_Buffer(NULL),
    m_BufferSize(other.m_DataSize),
    m_DataSize(other.m_DataSize)
{
    m_Buffer = new AP4_Byte[m_BufferSize];
    memcpy(m_Buffer, other.m_Buffer, m_BufferSize);
}

/*----------------------------------------------------------------------
|       AP4_DataBuffer::~AP4_DataBuffer
+---------------------------------------------------------------------*/
AP4_DataBuffer::~AP4_DataBuffer()
{
    if (m_BufferIsLocal) {
        delete[] m_Buffer;
    }
}

/*----------------------------------------------------------------------
|       AP4_DataBuffer::SetBuffer
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataBuffer::SetBuffer(AP4_Byte* buffer, AP4_Size buffer_size)
{
    if (m_BufferIsLocal) {
        // destroy the local buffer
        delete[] m_Buffer;
    }

    // we're now using an external buffer
    m_BufferIsLocal = false;
    m_Buffer = buffer;
    m_BufferSize = buffer_size;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_DataBuffer::SetBufferSize
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataBuffer::SetBufferSize(AP4_Size buffer_size)
{
    if (m_BufferIsLocal) {
        return ReallocateBuffer(buffer_size);
    } else {
        return AP4_FAILURE; // you cannot change the
                            // buffer management mode
    }
}

/*----------------------------------------------------------------------
|       AP4_DataBuffer::SetDataSize
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataBuffer::SetDataSize(AP4_Size size)
{
    if (size > m_BufferSize) {
        if (m_BufferIsLocal) {
            AP4_Result result = ReallocateBuffer(size);
            if (AP4_FAILED(result)) return result;
        } else { 
            return AP4_FAILURE;
        }
    }
    m_DataSize = size;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_DataBuffer::SetData
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataBuffer::SetData(AP4_Byte* data, AP4_Size size)
{
    if (size > m_BufferSize) {
        if (m_BufferIsLocal) {
            AP4_Result result = ReallocateBuffer(size);
            if (AP4_FAILED(result)) return result;
        } else {
            return AP4_FAILURE;
        }
    }
    memcpy(m_Buffer, data, size);
    m_DataSize = size;

    return AP4_SUCCESS;
}


/*----------------------------------------------------------------------
|       AP4_DataBuffer::ReallocateBuffer
+---------------------------------------------------------------------*/
AP4_Result
AP4_DataBuffer::ReallocateBuffer(AP4_Size size)
{
    // check that the existing data fits
    if (m_DataSize > size) return AP4_FAILURE;

    // allocate a new buffer
    AP4_Byte* new_buffer = new AP4_Byte[size];

    // copy the contents of the previous buffer ,is any
	if (m_Buffer && m_DataSize) {
		memcpy(new_buffer, m_Buffer, m_DataSize);
	}

    // destroy the previous buffer
    delete[] m_Buffer;

    // use the new buffer
    m_Buffer = new_buffer;
    m_BufferSize = size;

    return AP4_SUCCESS;
}
