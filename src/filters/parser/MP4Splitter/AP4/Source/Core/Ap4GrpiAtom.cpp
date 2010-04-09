/*****************************************************************
|
|    AP4 - ohdr Atoms 
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
#include "Ap4GrpiAtom.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_GrpiAtom)

/*----------------------------------------------------------------------
|   AP4_GrpiAtom::Create
+---------------------------------------------------------------------*/
AP4_GrpiAtom*
AP4_GrpiAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_GrpiAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_GrpiAtom::AP4_GrpiAtom
+---------------------------------------------------------------------*/
AP4_GrpiAtom::AP4_GrpiAtom(AP4_UI08        key_encryption_method, 
                           const char*     group_id,
                           const AP4_UI08* group_key,
                           AP4_Size        group_key_length) :
    AP4_Atom(AP4_ATOM_TYPE_GRPI, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_KeyEncryptionMethod(key_encryption_method),
    m_GroupId(group_id),
    m_GroupKey(group_key, group_key_length)
{
    m_Size32 += 2+1+2+m_GroupId.GetLength()+group_key_length;
}

/*----------------------------------------------------------------------
|   AP4_GrpiAtom::AP4_GrpiAtom
+---------------------------------------------------------------------*/
AP4_GrpiAtom::AP4_GrpiAtom(AP4_UI32        size, 
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_GRPI, size, false, version, flags)
{
    // group id length
    AP4_UI16 group_id_length = 0;
    stream.ReadUI16(group_id_length);

    // encryption method
    stream.ReadUI08(m_KeyEncryptionMethod);
    
    // group key length
    AP4_UI16 group_key_length = 0;
    stream.ReadUI16(group_key_length);

    // group id
    char* group_id = new char[group_id_length];
    stream.Read(group_id, group_id_length);
    m_GroupId.Assign(group_id, group_id_length);
    delete[] group_id;

    // group key
    m_GroupKey.SetDataSize(group_key_length);
    stream.Read(m_GroupKey.UseData(), group_key_length);
}

/*----------------------------------------------------------------------
|   AP4_GrpiAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_GrpiAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_CHECK(stream.WriteUI16((AP4_UI16)m_GroupId.GetLength()));
    AP4_CHECK(stream.WriteUI08(m_KeyEncryptionMethod));
    AP4_CHECK(stream.WriteUI16((AP4_UI16)m_GroupKey.GetDataSize()));
    AP4_CHECK(stream.Write(m_GroupId.GetChars(), m_GroupId.GetLength()));
    AP4_CHECK(stream.Write(m_GroupKey.GetData(), m_GroupKey.GetDataSize()));
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_GrpiAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_GrpiAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("key encryption method", m_KeyEncryptionMethod);
    inspector.AddField("group id",              m_GroupId.GetChars());
    inspector.AddField("group key",             m_GroupKey.GetData(), 
                                                m_GroupKey.GetDataSize());
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_GrpiAtom::Clone
+---------------------------------------------------------------------*/
AP4_Atom* 
AP4_GrpiAtom::Clone()
{
    return new AP4_GrpiAtom(m_KeyEncryptionMethod,
                            m_GroupId.GetChars(),
                            m_GroupKey.GetData(),
                            m_GroupKey.GetDataSize());
}
