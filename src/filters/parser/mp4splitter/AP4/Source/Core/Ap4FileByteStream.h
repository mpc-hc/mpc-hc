/*****************************************************************
|
|    AP4 - FileByteStream 
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
#include "Ap4Types.h"
#include "Ap4ByteStream.h"
#include "Ap4Config.h"

#ifndef _AP4_FILE_BYTE_STREAM_H_
#define _AP4_FILE_BYTE_STREAM_H_

/*----------------------------------------------------------------------
|   AP4_FileByteStream
+---------------------------------------------------------------------*/
class AP4_FileByteStream: public AP4_ByteStream
{
public:
    // types
    typedef enum {
        STREAM_MODE_READ        = 0,
        STREAM_MODE_WRITE       = 1,
        STREAM_MODE_READ_WRITE  = 2
    } Mode;

    /**
     * Create a stream from a file (opened or created).
     *
     * @param name Name of the file open or create
     * @param mode Mode to use for the file
     * @param stream Refrence to a pointer where the stream object will
     * be returned
     * @return AP4_SUCCESS if the file can be opened or created, or an error code if
     * it cannot
     */
    static AP4_Result Create(const char* name, Mode mode, AP4_ByteStream*& stream);
    
    // constructors
    AP4_FileByteStream(AP4_ByteStream* delegate) : m_Delegate(delegate) {}
    
#if !defined(AP4_CONFIG_NO_EXCEPTIONS)
    /**
     * @deprecated
     */
    AP4_FileByteStream(const char* name, Mode mode);
#endif

    // AP4_ByteStream methods
    AP4_Result ReadPartial(void*    buffer, 
                           AP4_Size  bytesToRead, 
                           AP4_Size& bytesRead) {
        return m_Delegate->ReadPartial(buffer, bytesToRead, bytesRead);
    }
    AP4_Result WritePartial(const void* buffer, 
                            AP4_Size    bytesToWrite, 
                            AP4_Size&   bytesWritten) {
        return m_Delegate->WritePartial(buffer, bytesToWrite, bytesWritten);
    }
    AP4_Result Seek(AP4_Position position)  { return m_Delegate->Seek(position); }
    AP4_Result Tell(AP4_Position& position) { return m_Delegate->Tell(position); }
    AP4_Result GetSize(AP4_LargeSize& size) { return m_Delegate->GetSize(size);  }
    AP4_Result Flush()                      { return m_Delegate->Flush();        }

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








