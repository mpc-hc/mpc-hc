/*****************************************************************
|
|    AP4 - odda Atoms 
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
#include "Ap4Utils.h"
#include "Ap4OddaAtom.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_OddaAtom)

/*----------------------------------------------------------------------
|   AP4_OddaAtom::Create
+---------------------------------------------------------------------*/
AP4_OddaAtom*
AP4_OddaAtom::Create(AP4_UI64         size, 
                     AP4_ByteStream&  stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_OddaAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_OddaAtom::AP4_OddaAtom
+---------------------------------------------------------------------*/
AP4_OddaAtom::AP4_OddaAtom(AP4_UI64        size, 
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_ODDA, size, true, version, flags)
{
    // data length
    stream.ReadUI64(m_EncryptedDataLength);

    // get the source stream position
    AP4_Position position; 
    stream.Tell(position);

    // create a substream to represent the payload
    m_EncryptedPayload = new AP4_SubStream(stream, position, m_EncryptedDataLength);
    
    // seek to the end
    stream.Seek(position+m_EncryptedDataLength);
}

/*----------------------------------------------------------------------
|   AP4_OddaAtom::AP4_OddaAtom
+---------------------------------------------------------------------*/
AP4_OddaAtom::AP4_OddaAtom(AP4_ByteStream& encrypted_payload) :
    AP4_Atom(AP4_ATOM_TYPE_ODDA, 0, true, 0, 0)
{
    // encrypted data length
    encrypted_payload.GetSize(m_EncryptedDataLength);
    
    // update our size 
    SetSize(AP4_FULL_ATOM_HEADER_SIZE_64+8+m_EncryptedDataLength, true);
    
    // keep a reference to the encrypted payload
    m_EncryptedPayload = &encrypted_payload;
    m_EncryptedPayload->AddReference();
}

/*----------------------------------------------------------------------
|   AP4_OddaAtom::~AP4_OddaAtom
+---------------------------------------------------------------------*/
AP4_OddaAtom::~AP4_OddaAtom()
{
    if (m_EncryptedPayload) m_EncryptedPayload->Release();
}


/*----------------------------------------------------------------------
|   AP4_OddaAtom::SetEncryptedPayload
+---------------------------------------------------------------------*/
AP4_Result
AP4_OddaAtom::SetEncryptedPayload(AP4_ByteStream& stream, AP4_LargeSize length)
{
    // keep a reference to the stream
    if (m_EncryptedPayload) {
        m_EncryptedPayload->Release();
    }
    m_EncryptedPayload = &stream;
    m_EncryptedPayload->AddReference();
    
    // update the size
    m_EncryptedDataLength = length;
    SetSize(AP4_FULL_ATOM_HEADER_SIZE_64 + 8 + length, true);
    if (m_Parent) m_Parent->OnChildChanged(this);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_OddaAtom::SetEncryptedPayload
+---------------------------------------------------------------------*/
AP4_Result
AP4_OddaAtom::SetEncryptedPayload(AP4_ByteStream& stream)
{
    // the new encrypted data length is the size of the stream
    AP4_LargeSize length;
    AP4_Result result = stream.GetSize(length);
    if (AP4_FAILED(result)) return result;
    
    return SetEncryptedPayload(stream, length);
}

/*----------------------------------------------------------------------
|   AP4_OddaAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_OddaAtom::WriteFields(AP4_ByteStream& stream)
{
    // write the content type
    AP4_CHECK(stream.WriteUI64(m_EncryptedDataLength));

    // check that we have a source stream
    // and a normal size
    if (m_EncryptedPayload == NULL || GetSize() < 8) {
        return AP4_FAILURE;
    }

    // rewind the encrypted stream
    AP4_CHECK(m_EncryptedPayload->Seek(0));

    // copy the encrypted stream to the output
    AP4_CHECK(m_EncryptedPayload->CopyTo(stream, m_EncryptedDataLength));

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_OddaAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_OddaAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("encrypted_data_length", (AP4_UI32)m_EncryptedDataLength);
    return AP4_SUCCESS;
}
