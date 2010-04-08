/*****************************************************************
|
|    AP4 - iKMS Atoms
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
#include "Ap4IkmsAtom.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_IkmsAtom)

/*----------------------------------------------------------------------
|   AP4_IkmsAtom::Create
+---------------------------------------------------------------------*/
AP4_IkmsAtom*
AP4_IkmsAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version > 1) return NULL;
    return new AP4_IkmsAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_IkmsAtom::AP4_IkmsAtom
+---------------------------------------------------------------------*/
AP4_IkmsAtom::AP4_IkmsAtom(const char* kms_uri,
                           AP4_UI32    kms_id,
                           AP4_UI32    kms_version) :
    AP4_Atom(AP4_ATOM_TYPE_IKMS, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_KmsUri(kms_uri),
    m_KmsId(kms_id),
    m_KmsVersion(kms_version)
{
    m_Size32 += m_KmsUri.GetLength() + 1;
}

/*----------------------------------------------------------------------
|   AP4_IkmsAtom::AP4_IkmsAtom
+---------------------------------------------------------------------*/
AP4_IkmsAtom::AP4_IkmsAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_IKMS, size, version, flags)
{
    AP4_Size string_size = size - AP4_FULL_ATOM_HEADER_SIZE;
    if(m_Version == 1 && string_size >= 8)
    {
        string_size -= 8;
        stream.ReadUI32(m_KmsId);
        stream.ReadUI32(m_KmsVersion);
    }
    else
    {
        m_KmsId      = 0;
        m_KmsVersion = 0;
    }
    if(string_size)
    {
        char* str = new char[string_size];
        stream.Read(str, string_size);
        str[string_size-1] = '\0'; // force null-termination
        m_KmsUri = str;
        delete[] str;
    }
}

/*----------------------------------------------------------------------
|   AP4_IkmsAtom::Clone
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_IkmsAtom::Clone()
{
    return new AP4_IkmsAtom(m_KmsUri.GetChars(), m_KmsId, m_KmsVersion);
}

/*----------------------------------------------------------------------
|   AP4_IkmsAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IkmsAtom::WriteFields(AP4_ByteStream& stream)
{
    // handler version 1
    if(m_Version == 1)
    {
        stream.WriteUI32(m_KmsId);
        stream.WriteUI32(m_KmsVersion);
    }

    // kms uri
    AP4_Result result = stream.Write(m_KmsUri.GetChars(), m_KmsUri.GetLength() + 1);
    if(AP4_FAILED(result)) return result;

    // pad with zeros if necessary
    AP4_Size padding = m_Size32 - (AP4_FULL_ATOM_HEADER_SIZE + m_KmsUri.GetLength() + 1);
    if(m_Version == 1) padding -= 8;
    while(padding--) stream.WriteUI08(0);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IkmsAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IkmsAtom::InspectFields(AP4_AtomInspector& inspector)
{
    if(m_Version == 1)
    {
        char id[5];
        AP4_FormatFourChars(id, m_KmsId);
        inspector.AddField("kms_id", id);
        inspector.AddField("kms_version", m_KmsVersion);
    }
    inspector.AddField("kms_uri", m_KmsUri.GetChars());

    return AP4_SUCCESS;
}
