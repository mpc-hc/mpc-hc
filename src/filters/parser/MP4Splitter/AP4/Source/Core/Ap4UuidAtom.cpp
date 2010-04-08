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
AP4_UuidAtom::AP4_UuidAtom(AP4_UI64 size, const AP4_UI08* uuid) :
    AP4_Atom(AP4_ATOM_TYPE_UUID, size)
{
    AP4_CopyMemory(m_Uuid, uuid, 16);
}

/*----------------------------------------------------------------------
|   AP4_UuidAtom::AP4_UuidAtom
+---------------------------------------------------------------------*/
AP4_UuidAtom::AP4_UuidAtom(AP4_UI64 size, const AP4_UI08* uuid, AP4_UI32 version, AP4_UI32 flags) :
    AP4_Atom(AP4_ATOM_TYPE_UUID, size, false, version, flags)
{
    AP4_CopyMemory(m_Uuid, uuid, 16);
}

/*----------------------------------------------------------------------
|   AP4_UuidAtom::AP4_UuidAtom
+---------------------------------------------------------------------*/
AP4_UuidAtom::AP4_UuidAtom(AP4_UI64 size, bool is_full, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_UUID, size)
{
    if(is_full)
    {
        m_IsFull = true;
        ReadFullHeader(stream, m_Version, m_Flags);
    }
}

/*----------------------------------------------------------------------
|   AP4_UuidAtom::GetHeaderSize
+---------------------------------------------------------------------*/
AP4_Size
AP4_UuidAtom::GetHeaderSize() const
{
    return (m_IsFull ? AP4_FULL_UUID_ATOM_HEADER_SIZE : AP4_UUID_ATOM_HEADER_SIZE) + (m_Size32 == 1 ? 8 : 0);
}

/*----------------------------------------------------------------------
|   AP4_UuidAtom::WriteHeader
+---------------------------------------------------------------------*/
AP4_Result
AP4_UuidAtom::WriteHeader(AP4_ByteStream& stream)
{
    AP4_Result result;

    // write the size
    result = stream.WriteUI32(m_Size32);
    if(AP4_FAILED(result)) return result;

    // write the type
    result = stream.WriteUI32(m_Type);
    if(AP4_FAILED(result)) return result;

    // handle 64-bit sizes
    if(m_Size32 == 1)
    {
        result = stream.WriteUI64(m_Size64);
        if(AP4_FAILED(result)) return result;
    }

    // write the extended type
    result = stream.Write(m_Uuid, 16);
    if(AP4_FAILED(result)) return result;

    // for full atoms, write version and flags
    if(m_IsFull)
    {
        result = stream.WriteUI08(m_Version);
        if(AP4_FAILED(result)) return result;
        result = stream.WriteUI24(m_Flags);
        if(AP4_FAILED(result)) return result;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_UuidAtom::InspectHeader
+---------------------------------------------------------------------*/
AP4_Result
AP4_UuidAtom::InspectHeader(AP4_AtomInspector& inspector)
{
    char uuid[37];
    uuid[36] = '\0';
    char* dst = uuid;
    for(unsigned int i = 0; i < 16; i++)
    {
        *dst++ = AP4_NibbleHex(m_Uuid[i] >> 4);
        *dst++ = AP4_NibbleHex(m_Uuid[i] & 0x0F);
        if(i == 5 || i == 7 || i == 9 || i == 11) *dst++ = '-';
    }

    // write atom name
    char name[7];
    name[0] = '[';
    AP4_FormatFourCharsPrintable(&name[1], m_Type);
    name[5] = ']';
    name[6] = '\0';
    char header[128];
    char extra[32] = "";
    if(m_IsFull)
    {
        if(m_Version && m_Flags)
        {
            AP4_FormatString(extra, sizeof(extra),
                             ", version=%d, flags=%x",
                             m_Version,
                             m_Flags);
        }
        else if(m_Version)
        {
            AP4_FormatString(extra, sizeof(extra),
                             ", version=%d",
                             m_Version);
        }
        else if(m_Flags)
        {
            AP4_FormatString(extra, sizeof(extra),
                             ", flags=%x",
                             m_Flags);
        }
    }
    AP4_FormatString(header, sizeof(header),
                     "{%s} size=%ld+%lld%s",
                     uuid,
                     GetHeaderSize(),
                     GetSize() - GetHeaderSize(),
                     extra);
    inspector.StartElement(name, header);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_UnknownUuidAtom::AP4_UnknownUuidAtom
+---------------------------------------------------------------------*/
AP4_UnknownUuidAtom::AP4_UnknownUuidAtom(AP4_UI64 size, AP4_ByteStream& stream) :
    AP4_UuidAtom(size, false, stream)
{
    // store the data
    m_Data.SetDataSize((AP4_Size)size - GetHeaderSize());
    stream.Read(m_Data.UseData(), m_Data.GetDataSize());
}

/*----------------------------------------------------------------------
|   AP4_UnknownUuidAtom::AP4_UnknownUuidAtom
+---------------------------------------------------------------------*/
AP4_UnknownUuidAtom::AP4_UnknownUuidAtom(AP4_UI64 size, const AP4_UI08* uuid, AP4_ByteStream& stream) :
    AP4_UuidAtom(size, uuid)
{
    // store the data
    m_Data.SetDataSize((AP4_Size)size - GetHeaderSize());
    stream.Read(m_Data.UseData(), m_Data.GetDataSize());
}

/*----------------------------------------------------------------------
|   AP4_UnknownUuidAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_UnknownUuidAtom::WriteFields(AP4_ByteStream& stream)
{
    // write the data
    return stream.Write(m_Data.GetData(), m_Data.GetDataSize());
}

