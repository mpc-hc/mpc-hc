/*****************************************************************
|
|    AP4 - Descriptors
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
#include "Ap4Descriptor.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_Descriptor::MinHeaderSize
+---------------------------------------------------------------------*/
AP4_Size
AP4_Descriptor::MinHeaderSize(AP4_Size payload_size)
{
    // compute how many bytes are needed to encode the payload size
    // plus tag
    return 2+(payload_size/128);
}

/*----------------------------------------------------------------------
|       AP4_Descriptor::AP4_Descriptor
+---------------------------------------------------------------------*/
AP4_Descriptor::AP4_Descriptor(Tag      tag, 
                               AP4_Size header_size, 
                               AP4_Size payload_size) :
    m_Tag(tag),
    m_HeaderSize(header_size),
    m_PayloadSize(payload_size)
{
    AP4_ASSERT(header_size >= 1+1);
    AP4_ASSERT(header_size <= 1+4);
}

/*----------------------------------------------------------------------
|       AP4_Descriptor::Write
+---------------------------------------------------------------------*/
AP4_Result
AP4_Descriptor::Write(AP4_ByteStream& stream)
{
    AP4_Result result;

    // write the tag
    result = stream.WriteUI08(m_Tag);
    if (AP4_FAILED(result)) return result;

    // write the size
    AP4_ASSERT(m_HeaderSize-1 <= 8);
    AP4_ASSERT(m_HeaderSize >= 2);
    unsigned int size = m_PayloadSize;
    unsigned char bytes[8];

    // last bytes of the encoded size
    bytes[m_HeaderSize-2] = size&0x7F;

    // leading bytes of the encoded size
    for (int i=m_HeaderSize-3; i>=0; i--) {
        // move to the next 7 bits
        size >>= 7;

        // output a byte with a top bit marker
        bytes[i] = (size&0x7F) | 0x80;
    }

    result = stream.Write(bytes, m_HeaderSize-1);
    if (AP4_FAILED(result)) return result;

    // write the fields
    WriteFields(stream);

    return result;
}

/*----------------------------------------------------------------------
|       AP4_Descriptor::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_Descriptor::Inspect(AP4_AtomInspector& inspector)
{
    char name[6];
    AP4_StringFormat(name, sizeof(name), "#[%02x]", m_Tag);
    char info[64];
    AP4_StringFormat(info, sizeof(info), "size=%ld+%ld",
        GetHeaderSize(),
        m_PayloadSize);
    inspector.StartElement(name, info);
    inspector.EndElement();

    return AP4_SUCCESS;
}
