/*****************************************************************
|
|    AP4 - Byte Stream support
|
|    Copyright 2002 Gilles Boccon-Gibod
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
#include "Ap4ByteStream.h"
#include "Ap4Utils.h"
#include "Ap4Debug.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
const int AP4_BYTE_STREAM_COPY_BUFFER_SIZE = 4096;

/*----------------------------------------------------------------------
|       AP4_ByteStream::WriteString
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
|       AP4_ByteStream::WriteUI64
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
|       AP4_ByteStream::WriteUI32
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
|       AP4_ByteStream::WriteUI24
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
|       AP4_ByteStream::WriteUI16
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
|       AP4_ByteStream::WriteUI08
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::WriteUI08(AP4_UI08 value)
{
    return Write((void*)&value, 1);
}

/*----------------------------------------------------------------------
|       AP4_ByteStream::ReadUI64
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
|       AP4_ByteStream::ReadUI32
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
|       AP4_ByteStream::ReadUI24
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
|       AP4_ByteStream::ReadUI16
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
|       AP4_ByteStream::ReadUI08
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
|       AP4_ByteStream::ReadString
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
        result = Read(&buffer[bytes_read], 1, NULL);
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
|       AP4_ByteStream::CopyTo
+---------------------------------------------------------------------*/
AP4_Result
AP4_ByteStream::CopyTo(AP4_ByteStream& stream, AP4_Size size)
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
            bytes_to_read = size;
        }

        // read up to one buffer full
        result = Read(buffer, bytes_to_read, &bytes_read);
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
|       AP4_SubStream::AP4_SubStream
+---------------------------------------------------------------------*/
AP4_SubStream::AP4_SubStream(AP4_ByteStream& container, 
                             AP4_Offset      offset, 
                             AP4_Size        size) :
    m_Container(container),
    m_Offset(offset),
    m_Size(size),
    m_Position(0),
    m_ReferenceCount(1)
{
    m_Container.AddReference();
}

/*----------------------------------------------------------------------
|       AP4_SubStream::~AP4_SubStream
+---------------------------------------------------------------------*/
AP4_SubStream::~AP4_SubStream()
{
    m_Container.Release();
}

/*----------------------------------------------------------------------
|       AP4_SubStream::Read
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SubStream::Read(void*     buffer, 
                    AP4_Size  bytes_to_read, 
                    AP4_Size* bytes_read)
{
    // default values
    if (bytes_read) *bytes_read = 0;

    // shortcut
    if (bytes_to_read == 0) {
        return AP4_SUCCESS;
    }

    // clamp to range
    if (m_Position+bytes_to_read > m_Size) {
        bytes_to_read = m_Size - m_Position;
    }

    // check for end of substream
    if (bytes_to_read == 0) {
        return AP4_ERROR_EOS;
    }

    // seek inside container
    //AP4_Result result;
    //result = m_Container.Seek(m_Offset+m_Position);
    //if (AP4_FAILED(result)) {
    //    return result;
    //}

    // read from the container
    AP4_Size local_bytes_read;
    AP4_Result result = m_Container.Read(buffer, bytes_to_read, &local_bytes_read);
    if (bytes_read) *bytes_read = local_bytes_read;
    if (AP4_SUCCEEDED(result)) {
        m_Position += local_bytes_read;
    }
    return result;
}

/*----------------------------------------------------------------------
|       AP4_SubStream::Write
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SubStream::Write(const void* buffer, 
                     AP4_Size    bytes_to_write, 
                     AP4_Size*   bytes_written)
{
    // default values
    if (bytes_written) *bytes_written = 0;

    // shortcut
    if (bytes_to_write == 0) {
        return AP4_SUCCESS;
    }

    // clamp to range
    if (m_Position+bytes_to_write > m_Size) {
        bytes_to_write = m_Size - m_Position;
    }

    // check for en of substream
    if (bytes_to_write == 0) {
        return AP4_ERROR_EOS;
    }

    // seek inside container
    //AP4_Result result;
    //result = m_Container.Seek(m_Offset+m_Position);
    //if (AP4_FAILED(result)) return result;

    // write to container
    AP4_Size local_bytes_written;
    AP4_Result result = m_Container.Write(buffer, bytes_to_write, &local_bytes_written);
    if (bytes_written) *bytes_written = local_bytes_written;
    if (AP4_SUCCEEDED(result)) {
        m_Position += local_bytes_written;
    }
    return result;
}

/*----------------------------------------------------------------------
|       AP4_SubStream::Seek
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SubStream::Seek(AP4_Offset offset)
{
    if (offset > m_Size) return AP4_FAILURE;
    AP4_Result result;
    result = m_Container.Seek(m_Offset+offset);
    if (AP4_SUCCEEDED(result)) {
        m_Position = offset;
    }
    return result;
}

/*----------------------------------------------------------------------
|       AP4_SubStream::AddReference
+---------------------------------------------------------------------*/
void
AP4_SubStream::AddReference()
{
    m_ReferenceCount++;
}

/*----------------------------------------------------------------------
|       AP4_SubStream::Release
+---------------------------------------------------------------------*/
void
AP4_SubStream::Release()
{
    if (--m_ReferenceCount == 0) {
        delete this;
    }
}

/*----------------------------------------------------------------------
|       AP4_MemoryByteStream::AP4_MemoryByteStream
+---------------------------------------------------------------------*/
AP4_MemoryByteStream::AP4_MemoryByteStream(AP4_Size size) :
    m_BufferIsLocal(true),
    m_Size(size),
    m_Position(0),
    m_ReferenceCount(1)
{
    m_Buffer = new AP4_UI08[size];
}

/*----------------------------------------------------------------------
|       AP4_MemoryByteStream::AP4_MemoryByteStream
+---------------------------------------------------------------------*/
AP4_MemoryByteStream::AP4_MemoryByteStream(AP4_UI08* buffer, AP4_Size size) :
    m_BufferIsLocal(false),
    m_Buffer(buffer),
    m_Size(size),
    m_Position(0),
    m_ReferenceCount(1)
{}

/*----------------------------------------------------------------------
|       AP4_MemoryByteStream::~AP4_MemoryByteStream
+---------------------------------------------------------------------*/
AP4_MemoryByteStream::~AP4_MemoryByteStream()
{
    if (m_BufferIsLocal) delete[] m_Buffer;
}

/*----------------------------------------------------------------------
|       AP4_MemoryByteStream::Read
+---------------------------------------------------------------------*/
AP4_Result 
AP4_MemoryByteStream::Read(void*     buffer, 
                           AP4_Size  bytes_to_read, 
                           AP4_Size* bytes_read)
{
    // default values
    if (bytes_read) *bytes_read = 0;

    // shortcut
    if (bytes_to_read == 0) {
        return AP4_SUCCESS;
    }

    // clamp to range
    if (m_Position+bytes_to_read > m_Size) {
        bytes_to_read = m_Size - m_Position;
    }

    // check for end of stream
    if (bytes_to_read == 0) {
        return AP4_ERROR_EOS;
    }

    // read from the memory
    memcpy(buffer, &m_Buffer[m_Position], bytes_to_read);
    m_Position += bytes_to_read;

    if (bytes_read) *bytes_read = bytes_to_read;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_MemoryByteStream::Write
+---------------------------------------------------------------------*/
AP4_Result 
AP4_MemoryByteStream::Write(const void* buffer, 
                            AP4_Size    bytes_to_write, 
                            AP4_Size*   bytes_written)
{
    // default values
    if (bytes_written) *bytes_written = 0;

    // shortcut
    if (bytes_to_write == 0) {
        return AP4_SUCCESS;
    }

    // clamp to range
    if (m_Position+bytes_to_write > m_Size) {
        bytes_to_write = m_Size - m_Position;
    }

    // check for en of stream
    if (bytes_to_write == 0) {
        return AP4_ERROR_EOS;
    }

    // write to memory
    memcpy(&m_Buffer[m_Position], buffer, bytes_to_write);
    m_Position += bytes_to_write;

    if (bytes_written) *bytes_written = bytes_to_write;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_MemoryByteStream::Seek
+---------------------------------------------------------------------*/
AP4_Result 
AP4_MemoryByteStream::Seek(AP4_Offset offset)
{
    if (offset > m_Size) return AP4_FAILURE;
    m_Position = offset;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_MemoryByteStream::AddReference
+---------------------------------------------------------------------*/
void
AP4_MemoryByteStream::AddReference()
{
    m_ReferenceCount++;
}

/*----------------------------------------------------------------------
|       AP4_MemoryByteStream::Release
+---------------------------------------------------------------------*/
void
AP4_MemoryByteStream::Release()
{
    if (--m_ReferenceCount == 0) {
        delete this;
    }
}
