/*****************************************************************
|
|    AP4 - iKMS Atoms 
|
|    Copyright 2002-2005 Gilles Boccon-Gibod & Julien Boeuf
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
#include "Ap4IkmsAtom.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_IkmsAtom::AP4_IkmsAtom
+---------------------------------------------------------------------*/
AP4_IkmsAtom::AP4_IkmsAtom(const char* kms_uri) :
    AP4_Atom(AP4_ATOM_TYPE_IKMS, AP4_FULL_ATOM_HEADER_SIZE, true),
    m_KmsUri(kms_uri)
{
    m_Size += m_KmsUri.length()+1;
}

/*----------------------------------------------------------------------
|       AP4_IkmsAtom::AP4_IkmsAtom
+---------------------------------------------------------------------*/
AP4_IkmsAtom::AP4_IkmsAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_IKMS, size, true, stream)
{
    AP4_Size string_size = size-AP4_FULL_ATOM_HEADER_SIZE;
    if (string_size) {
        char* str = new char[string_size];
        stream.Read(str, string_size);
        str[string_size-1] = '\0'; // force null-termination
        m_KmsUri = str;
        delete[] str;
    }
}

/*----------------------------------------------------------------------
|       AP4_IkmsAtom::Clone
+---------------------------------------------------------------------*/
AP4_Atom* 
AP4_IkmsAtom::Clone()
{
    return new AP4_IkmsAtom(m_KmsUri.c_str());
}

/*----------------------------------------------------------------------
|       AP4_IkmsAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IkmsAtom::WriteFields(AP4_ByteStream& stream)
{
    // kms uri
    AP4_Result result = stream.Write(m_KmsUri.c_str(), m_KmsUri.length()+1);
    if (AP4_FAILED(result)) return result;

    // pad with zeros if necessary
    AP4_Size padding = m_Size-(AP4_FULL_ATOM_HEADER_SIZE+m_KmsUri.length()+1);
    while (padding--) stream.WriteUI08(0);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_IkmsAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IkmsAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("kms_uri", m_KmsUri.c_str());

    return AP4_SUCCESS;
}
