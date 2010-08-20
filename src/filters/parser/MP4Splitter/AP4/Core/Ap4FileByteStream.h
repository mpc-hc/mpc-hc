/*****************************************************************
|
|    AP4 - FileByteStream 
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

#ifndef _AP4_FILE_BYTE_STREAM_H_
#define _AP4_FILE_BYTE_STREAM_H_

/*----------------------------------------------------------------------
|       AP4_FileByteStream
+---------------------------------------------------------------------*/
class AP4_FileByteStream: public AP4_ByteStream
{
public:
    // types
    typedef enum {
        STREAM_MODE_READ,
        STREAM_MODE_WRITE
    } Mode;

    // methods
    AP4_FileByteStream(const char* name, Mode mode);

    // AP4_ByteStream methods
    AP4_Result Read(void*    buffer, 
                   AP4_Size  bytesToRead, 
                   AP4_Size* bytesRead) {
        return m_Delegate->Read(buffer, bytesToRead, bytesRead);
    }
    AP4_Result Write(const void* buffer, 
                    AP4_Size     bytesToWrite, 
                    AP4_Size*    bytesWritten) {
        return m_Delegate->Write(buffer, bytesToWrite, bytesWritten);
    }
    AP4_Result Seek(AP4_Offset offset)  { return m_Delegate->Seek(offset); }
    AP4_Result Tell(AP4_Offset& offset) { return m_Delegate->Tell(offset); }
    AP4_Result GetSize(AP4_Size& size)  { return m_Delegate->GetSize(size);}

    // AP4_Referenceable methods
    void AddReference() { m_Delegate->AddReference(); }
    void Release()      { m_Delegate->Release();      }

protected:
    // methods
    virtual ~AP4_FileByteStream() {
        delete m_Delegate;
    }

    // members
    AP4_ByteStream* m_Delegate;
};

#endif // _AP4_FILE_BYTE_STREAM_H_








