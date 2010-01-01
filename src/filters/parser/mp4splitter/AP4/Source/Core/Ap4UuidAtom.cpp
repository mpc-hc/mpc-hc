/*****************************************************************
|
|    AP4 - UUID Atoms 
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
#include "Ap4UuidAtom.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   AP4_UuidAtom::AP4_UuidAtom
+---------------------------------------------------------------------*/
AP4_UuidAtom::AP4_UuidAtom(AP4_UI64 size, AP4_ByteStream& stream) : 
    AP4_Atom(AP4_ATOM_TYPE_UUID, size),
    m_SourceStream(&stream)
{
    // read the uuid
    stream.Read(m_Uuid, 16);
    
    // store source stream position
    stream.Tell(m_SourcePosition);

    // keep a reference to the source stream
    m_SourceStream->AddReference();
}

/*----------------------------------------------------------------------
|   AP4_UuidAtom::~AP4_UuidAtom
+---------------------------------------------------------------------*/
AP4_UuidAtom::~AP4_UuidAtom()
{
    // release the source stream reference
    if (m_SourceStream) {
        m_SourceStream->Release();
    }
}

/*----------------------------------------------------------------------
|   AP4_NibbleHex
+---------------------------------------------------------------------*/
static char
AP4_NibbleHex(unsigned int nibble) 
{
    if (nibble < 10) {
        return '0'+nibble;
    } else if (nibble < 16) {
        return 'A'+(nibble-10);
    } else {
        return ' ';
    }
}

/*----------------------------------------------------------------------
|   AP4_UuidAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_UuidAtom::InspectFields(AP4_AtomInspector& inspector)
{
    char uuid[37];
    uuid[36] = '\0';
    char* dst = uuid;
    for (unsigned int i=0; i<16; i++) {
        *dst++ = AP4_NibbleHex(m_Uuid[i]>>4);
        *dst++ = AP4_NibbleHex(m_Uuid[i]&0x0F);
        if (i == 5 || i == 7 || i == 9 || i == 11) *dst++ = '-';
    }
    inspector.AddField("uuid", uuid);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_UuidAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_UuidAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // write uuid
    stream.Write(m_Uuid, 16);
    
    // check that we have a source stream
    // and a normal size
    if (m_SourceStream == NULL || GetSize() < 8+16) {
        return AP4_FAILURE;
    }

    // remember the source position
    AP4_Position position;
    m_SourceStream->Tell(position);

    // seek into the source at the stored offset
    result = m_SourceStream->Seek(m_SourcePosition);
    if (AP4_FAILED(result)) return result;

    // copy the source stream to the output
    AP4_UI64 payload_size = GetSize()-GetHeaderSize()-16;
    result = m_SourceStream->CopyTo(stream, payload_size);
    if (AP4_FAILED(result)) return result;

    // restore the original stream position
    m_SourceStream->Seek(position);

    return AP4_SUCCESS;
}

