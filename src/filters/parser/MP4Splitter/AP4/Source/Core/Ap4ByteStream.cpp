/*****************************************************************
|
|    AP4 - Byte Stream support
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
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
|   includes
+---------------------------------------------------------------------*/
#include "Ap4ByteStream.h"
#include "Ap4Utils.h"
#include "Ap4Debug.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const int AP4_BYTE_STREAM_COPY_BUFFER_SIZE = 65536;
const int AP4_MEMORY_BYTE_STREAM_MAX_SIZE  = 0x4000000; // 64 megs

/*----------------------------------------------------------------------
|   AP4_ByteStream::Read
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::Read(void* buffer, AP4_Size bytes_to_read)
{
    // shortcut
    if (bytes_to_read == 0) return AP4_SUCCESS;
    
    // read until failure
    AP4_Size bytes_read;
    while (bytes_to_read) {
        AP4_Result result = ReadPartial(buffer, bytes_to_read, bytes_read);
        if (AP4_FAILED(result)) return result;
        if (bytes_read == 0) return AP4_ERROR_INTERNAL;
        AP4_ASSERT(bytes_read <= bytes_to_read);
        bytes_to_read -= bytes_read;
        buffer = (void*)(((AP4_Byte*)buffer)+bytes_read);
    }
    
    return AP4_SUCCESS;
}  

/*----------------------------------------------------------------------
|   AP4_Stream::Write
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::Write(const void* buffer, AP4_Size bytes_to_write)
{
    // shortcut
    if (bytes_to_write == 0) return AP4_SUCCESS;
    
    // write until failure
    AP4_Size bytes_written;
    while (bytes_to_write) {
        AP4_Result result = WritePartial(buffer, bytes_to_write, bytes_written);
        if (AP4_FAILED(result)) return result;
        if (bytes_written == 0) return AP4_ERROR_INTERNAL;
        AP4_ASSERT(bytes_written <= bytes_to_write);
        bytes_to_write -= bytes_written;
        buffer = (const void*)(((const AP4_Byte*)buffer)+bytes_written);
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::WriteString
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::WriteString(const char* buffer)
{
    AP4_Size string_length = static_cast<AP4_Size>(strlen(buffer));

    // shortcut
    if ((buffer == NULL) || (string_length == 0)) return AP4_SUCCESS;

    // write the string
    return Write((const void*)buffer, string_length);
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::WriteDouble
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::WriteDouble(double value)
{
    unsigned char buffer[8];

    // convert value to bytes
    AP4_BytesFromDoubleBE(buffer, value);

    // write bytes to the stream
    return Write((void*)buffer, 8);
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::WriteUI64
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::WriteUI64(AP4_UI64 value)
{
    unsigned char buffer[8];

    // convert value to bytes
    AP4_BytesFromUInt64BE(buffer, value);

    // write bytes to the stream
    return Write((void*)buffer, 8);
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::WriteUI32
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::WriteUI32(AP4_UI32 value)
{
    unsigned char buffer[4];

    // convert value to bytes
    AP4_BytesFromUInt32BE(buffer, value);

    // write bytes to the stream
    return Write((void*)buffer, 4);
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::WriteUI24
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::WriteUI24(AP4_UI32 value)
{
    unsigned char buffer[3];

    // convert value to bytes
    AP4_BytesFromUInt24BE(buffer, value);

    // write bytes to the stream
    return Write((void*)buffer, 3);
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::WriteUI16
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::WriteUI16(AP4_UI16 value)
{
    unsigned char buffer[2];

    // convert value to bytes
    AP4_BytesFromUInt16BE(buffer, value);

    // write bytes to the stream
    return Write((void*)buffer, 2);
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::WriteUI08
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::WriteUI08(AP4_UI08 value)
{
    return Write((void*)&value, 1);
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::ReadUI64
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::ReadUI64(AP4_UI64& value)
{
    unsigned char buffer[8];

    // read bytes from the stream
    AP4_Result result;
    result = Read((void*)buffer, 8);
    if (AP4_FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = AP4_BytesToUInt64BE(buffer);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::ReadDouble
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::ReadDouble(double& value)
{
    unsigned char buffer[8];

    // read bytes from the stream
    AP4_Result result;
    result = Read((void*)buffer, 8);
    if (AP4_FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = AP4_BytesToDoubleBE(buffer);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::ReadUI32
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::ReadUI32(AP4_UI32& value)
{
    unsigned char buffer[4];

    // read bytes from the stream
    AP4_Result result;
    result = Read((void*)buffer, 4);
    if (AP4_FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = AP4_BytesToUInt32BE(buffer);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::ReadUI24
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::ReadUI24(AP4_UI32& value)
{
    unsigned char buffer[3];

    // read bytes from the stream
    AP4_Result result;
    result = Read((void*)buffer, 3);
    if (AP4_FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = AP4_BytesToUInt24BE(buffer);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::ReadUI16
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::ReadUI16(AP4_UI16& value)
{
    unsigned char buffer[2];

    // read bytes from the stream
    AP4_Result result;
    result = Read((void*)buffer, 2);
    if (AP4_FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = AP4_BytesToUInt16BE(buffer);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::ReadUI08
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::ReadUI08(AP4_UI08& value)
{
    unsigned char buffer[1];

    // read bytes from the stream
    AP4_Result result;
    result = Read((void*)buffer, 1);
    if (AP4_FAILED(result)) {        
        value = 0;
        return result;
    }

    // convert bytes to value
    value = buffer[0];
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::ReadString
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::ReadString(char* buffer, AP4_Size size)
{
    if (buffer == NULL || size == 0) {
        return AP4_ERROR_INVALID_PARAMETERS;
    }

    AP4_Size bytes_read = 0;
    while (bytes_read < size-1) {      
        AP4_Result result;
        result = Read(&buffer[bytes_read], 1);
        if (AP4_FAILED(result)) {
            buffer[bytes_read] = '\0';
            return result;
        }
        if (buffer[bytes_read] == '\0') {
            // end of string
            return AP4_SUCCESS;
        }
        bytes_read++;
    }

    // the string was not null terminated, terminate it
    buffer[size-1] = '\0';
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ByteStream::CopyTo
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::CopyTo(AP4_ByteStream& stream, AP4_LargeSize size)
{
    unsigned char buffer[AP4_BYTE_STREAM_COPY_BUFFER_SIZE];
    while (size) {
        AP4_Size bytes_read;
        AP4_Size bytes_to_read;
        AP4_Result result;

        // decide how much to read
        if (size >= sizeof(buffer)) {
            bytes_to_read = sizeof(buffer);
        } else {
            bytes_to_read = (AP4_Size)size;
        }

        // read up to one buffer full
        result = ReadPartial(buffer, bytes_to_read, bytes_read);
        if (AP4_FAILED(result)) return result;

        // copy to destination
        if (bytes_read != 0) {
            result = stream.Write(buffer, bytes_read);
            if (AP4_FAILED(result)) return result;
        }

        // update the size
        size -= bytes_read;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_SubStream::AP4_SubStream
+---------------------------------------------------------------------*/
AP4_SubStream::AP4_SubStream(AP4_ByteStream& container, 
                             AP4_Position    offset, 
                             AP4_LargeSize   size) :
    m_Container(container),
    m_Offset(offset),
    m_Size(size),
    m_Position(0),
    m_ReferenceCount(1)
{
    m_Container.AddReference();
}

/*----------------------------------------------------------------------
|   AP4_SubStream::~AP4_SubStream
+---------------------------------------------------------------------*/
AP4_SubStream::~AP4_SubStream()
{
    m_Container.Release();
}

/*----------------------------------------------------------------------
|   AP4_SubStream::ReadPartial
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SubStream::ReadPartial(void*     buffer, 
                           AP4_Size  bytes_to_read, 
                           AP4_Size& bytes_read)
{
    // default values
    bytes_read = 0;

    // shortcut
    if (bytes_to_read == 0) {
        return AP4_SUCCESS;
    }

    // clamp to range
    if (m_Position+bytes_to_read > m_Size) {
        bytes_to_read = (AP4_Size)(m_Size - m_Position);
    }

    // check for end of substream
    if (bytes_to_read == 0) {
        return AP4_ERROR_EOS;
    }

    // seek inside container
    AP4_Result result;
    result = m_Container.Seek(m_Offset+m_Position);
    if (AP4_FAILED(result)) {
        return result;
    }

    // read from the container
    result = m_Container.ReadPartial(buffer, bytes_to_read, bytes_read);
    if (AP4_SUCCEEDED(result)) {
        m_Position += bytes_read;
    }
    return result;
}

/*----------------------------------------------------------------------
|   AP4_SubStream::WritePartial
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SubStream::WritePartial(const void* buffer, 
                            AP4_Size    bytes_to_write, 
                            AP4_Size&   bytes_written)
{
    // default values
    bytes_written = 0;

    // shortcut
    if (bytes_to_write == 0) {
        return AP4_SUCCESS;
    }

    // clamp to range
    if (m_Position+bytes_to_write > m_Size) {
        bytes_to_write = (AP4_Size)(m_Size - m_Position);
    }

    // check for en of substream
    if (bytes_to_write == 0) {
        return AP4_ERROR_EOS;
    }

    // seek inside container
    AP4_Result result;
    result = m_Container.Seek(m_Offset+m_Position);
    if (AP4_FAILED(result)) return result;

    // write to container
    result = m_Container.WritePartial(buffer, bytes_to_write, bytes_written);
    if (AP4_SUCCEEDED(result)) {
        m_Position += bytes_written;
    }
    return result;
}

/*----------------------------------------------------------------------
|   AP4_SubStream::Seek
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SubStream::Seek(AP4_Position position)
{
    if (position == m_Position) return AP4_SUCCESS;
    if (position > m_Size) return AP4_FAILURE;
    m_Position = position;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_SubStream::AddReference
+---------------------------------------------------------------------*/
void
AP4_SubStream::AddReference()
{
    m_ReferenceCount++;
}

/*----------------------------------------------------------------------
|   AP4_SubStream::Release
+---------------------------------------------------------------------*/
void
AP4_SubStream::Release()
{
    if (--m_ReferenceCount == 0) {
        delete this;
    }
}

/*----------------------------------------------------------------------
|   AP4_MemoryByteStream::AP4_MemoryByteStream
+---------------------------------------------------------------------*/
AP4_MemoryByteStream::AP4_MemoryByteStream(AP4_Size size) :
    m_BufferIsLocal(true),
    m_Position(0),
    m_ReferenceCount(1)
{
    m_Buffer = new AP4_DataBuffer(size);
    AP4_SetMemory(m_Buffer->UseData(), 0, size);
    m_Buffer->SetDataSize(size);
}

/*----------------------------------------------------------------------
|   AP4_MemoryByteStream::AP4_MemoryByteStream
+---------------------------------------------------------------------*/
AP4_MemoryByteStream::AP4_MemoryByteStream(const AP4_UI08* buffer, AP4_Size size) :
    m_BufferIsLocal(true),
    m_Position(0),
    m_ReferenceCount(1)
{
    m_Buffer = new AP4_DataBuffer(buffer, size);
}

/*----------------------------------------------------------------------
 |   AP4_MemoryByteStream::AP4_MemoryByteStream
 +---------------------------------------------------------------------*/
AP4_MemoryByteStream::AP4_MemoryByteStream(AP4_DataBuffer& data_buffer) :
    m_BufferIsLocal(false),
    m_Position(0),
    m_ReferenceCount(1)
{
    m_Buffer = &data_buffer;
}

/*----------------------------------------------------------------------
 |   AP4_MemoryByteStream::~AP4_MemoryByteStream
 +---------------------------------------------------------------------*/
AP4_MemoryByteStream::~AP4_MemoryByteStream()
{
    if (m_BufferIsLocal) {
        delete m_Buffer;
    }
}

/*----------------------------------------------------------------------
|   AP4_MemoryByteStream::ReadPartial
+---------------------------------------------------------------------*/
AP4_Result 
AP4_MemoryByteStream::ReadPartial(void*     buffer, 
                                  AP4_Size  bytes_to_read, 
                                  AP4_Size& bytes_read)
{
    // default values
    bytes_read = 0;

    // shortcut
    if (bytes_to_read == 0) {
        return AP4_SUCCESS;
    }

    // clamp to range
    if (m_Position+bytes_to_read > m_Buffer->GetDataSize()) {
        bytes_to_read = (AP4_Size)(m_Buffer->GetDataSize() - m_Position);
    }

    // check for end of stream
    if (bytes_to_read == 0) {
        return AP4_ERROR_EOS;
    }

    // read from the memory
    AP4_CopyMemory(buffer, m_Buffer->GetData()+m_Position, bytes_to_read);
    m_Position += bytes_to_read;

    bytes_read = bytes_to_read;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MemoryByteStream::WritePartial
+---------------------------------------------------------------------*/
AP4_Result 
AP4_MemoryByteStream::WritePartial(const void* buffer, 
                                   AP4_Size    bytes_to_write, 
                                   AP4_Size&   bytes_written)
{
    // default values
    bytes_written = 0;

    // shortcut
    if (bytes_to_write == 0) {
        return AP4_SUCCESS;
    }

    // check that we don't exceed the max
    if (m_Position+bytes_to_write > (AP4_Position)AP4_MEMORY_BYTE_STREAM_MAX_SIZE) {
        return AP4_ERROR_OUT_OF_RANGE;
    }

    // reserve space in the buffer
    AP4_Result result = m_Buffer->Reserve((AP4_Size)(m_Position+bytes_to_write));
    if (AP4_SUCCEEDED(result)) {
        m_Buffer->SetDataSize((AP4_Size)(m_Position+bytes_to_write));
    } else {
        // failed to reserve, most likely caused by a buffer that has
        // external storage
        if (m_Position+bytes_to_write > m_Buffer->GetDataSize()) {
            bytes_to_write = (AP4_Size)(m_Buffer->GetDataSize() - m_Position);
        }
    } 

    // check for en of stream
    if (bytes_to_write == 0) {
        return AP4_ERROR_EOS;
    }

    // write to memory
    AP4_CopyMemory((void*)(m_Buffer->UseData()+m_Position), buffer, bytes_to_write);
    m_Position += bytes_to_write;

    bytes_written = bytes_to_write;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MemoryByteStream::Seek
+---------------------------------------------------------------------*/
AP4_Result 
AP4_MemoryByteStream::Seek(AP4_Position position)
{
    if (position > m_Buffer->GetDataSize()) return AP4_FAILURE;
    m_Position = position;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MemoryByteStream::AddReference
+---------------------------------------------------------------------*/
void
AP4_MemoryByteStream::AddReference()
{
    m_ReferenceCount++;
}

/*----------------------------------------------------------------------
|   AP4_MemoryByteStream::Release
+---------------------------------------------------------------------*/
void
AP4_MemoryByteStream::Release()
{
    if (--m_ReferenceCount == 0) {
        delete this;
    }
}

/*----------------------------------------------------------------------
|   AP4_BufferedInputStream::AP4_BufferedInputStream
+---------------------------------------------------------------------*/
AP4_BufferedInputStream::AP4_BufferedInputStream(AP4_ByteStream& source, 
                                                 AP4_Size        buffer_size,
                                                 AP4_Size        seek_as_read_threshold) :
    m_Buffer(buffer_size),
    m_BufferPosition(0),
    m_Source(source),
    m_SourcePosition(0),
    m_SeekAsReadThreshold(seek_as_read_threshold),
    m_ReferenceCount(1)
{
    source.AddReference();
}

/*----------------------------------------------------------------------
|   AP4_BufferedInputStream::Refill
+---------------------------------------------------------------------*/
AP4_Result 
AP4_BufferedInputStream::Refill()
{
    m_BufferPosition = 0;
    AP4_Size bytes_read = 0;
    AP4_Result result = m_Source.ReadPartial(m_Buffer.UseData(), 
                                             m_Buffer.GetBufferSize(), 
                                             bytes_read);
    if (AP4_FAILED(result)) {
        m_Buffer.SetDataSize(0);
        return result;
    }
    assert(bytes_read);
    m_Buffer.SetDataSize(bytes_read);
    m_SourcePosition += bytes_read;
    
    return AP4_SUCCESS;
} 

/*----------------------------------------------------------------------
|   AP4_BufferedInputStream::ReadPartial
+---------------------------------------------------------------------*/
AP4_Result 
AP4_BufferedInputStream::ReadPartial(void*     buffer, 
                                     AP4_Size  bytes_to_read, 
                                     AP4_Size& bytes_read)
{
    // check for shortcut
    if (bytes_to_read == 0) {
        bytes_read = 0;
        return AP4_SUCCESS;
    }
    
    // compute how much data is available in the buffer
    assert(m_BufferPosition <= m_Buffer.GetDataSize());
    AP4_Size available = m_Buffer.GetDataSize()-m_BufferPosition;
    
    // refill the buffer if it is empty
    if (available == 0) {
        AP4_Result result = Refill();
        if (AP4_FAILED(result)) {
            bytes_read = 0;
            return result;
        }
        assert(m_BufferPosition == 0);
        assert(m_Buffer.GetDataSize() != 0);
        available = m_Buffer.GetDataSize()-m_BufferPosition;
    }
    
    // clamp the number of bytes to read to what's available
    if (bytes_to_read > available) bytes_to_read = available;
    bytes_read = bytes_to_read;
    
    // copy the buffered data
    AP4_CopyMemory(buffer, m_Buffer.GetData()+m_BufferPosition, bytes_to_read);
    m_BufferPosition += bytes_to_read;
    assert(m_BufferPosition <= m_Buffer.GetDataSize());
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_BufferedInputStream::WritePartial
+---------------------------------------------------------------------*/
AP4_Result 
AP4_BufferedInputStream::WritePartial(const void* /*buffer*/, 
                                      AP4_Size    /*bytes_to_write*/, 
                                      AP4_Size&   /*bytes_written*/)
{
    return AP4_ERROR_NOT_SUPPORTED;
}

/*----------------------------------------------------------------------
|   AP4_BufferedInputStream::Seek
+---------------------------------------------------------------------*/
AP4_Result 
AP4_BufferedInputStream::Seek(AP4_Position position)
{
    assert(m_SourcePosition >= m_Buffer.GetDataSize());
    assert(m_BufferPosition <= m_Buffer.GetDataSize());
    if (position < m_SourcePosition-m_Buffer.GetDataSize() || 
        position > m_SourcePosition) {
        // out of buffer
        m_BufferPosition = 0;
        m_Buffer.SetDataSize(0);
        
        // seek in the source
        if (position > m_SourcePosition && (position-m_SourcePosition <= m_SeekAsReadThreshold)) {
            char*    discard = new char[4096];
            AP4_Size to_skip = (AP4_Size)(position-m_SourcePosition);
            while (to_skip) {
                AP4_Size chunk = 4096;
                if (chunk > to_skip) chunk = to_skip;
                AP4_Result result = m_Source.Read(discard, chunk);
                if (AP4_FAILED(result)) {
                    delete[] discard;
                    return result;
                }
                m_SourcePosition += chunk;
                to_skip -= chunk;
            }
            delete[] discard;
            return AP4_SUCCESS;
        } else {
            m_SourcePosition = position;
            return m_Source.Seek(position);
        }
    }
    
    // compute the buffer position
    m_BufferPosition = (AP4_Size)(position-(m_SourcePosition-m_Buffer.GetDataSize()));
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_BufferedInputStream::Tell
+---------------------------------------------------------------------*/
AP4_Result 
AP4_BufferedInputStream::Tell(AP4_Position& position)
{
    assert(m_SourcePosition >= m_Buffer.GetDataSize());
    assert(m_BufferPosition <= m_Buffer.GetDataSize());
    position = m_SourcePosition-m_Buffer.GetDataSize()+m_BufferPosition;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_BufferedInputStream::AddReference
+---------------------------------------------------------------------*/
void
AP4_BufferedInputStream::AddReference()
{
    m_ReferenceCount++;
}

/*----------------------------------------------------------------------
|   AP4_MemoryByteStream::Release
+---------------------------------------------------------------------*/
void
AP4_BufferedInputStream::Release()
{
    if (--m_ReferenceCount == 0) {
        delete this;
    }
}
