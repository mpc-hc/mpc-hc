/*****************************************************************
|
|    AP4 - url Atoms 
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
#include "Ap4UrlAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   AP4_UrlAtom::Create
+---------------------------------------------------------------------*/
AP4_UrlAtom*
AP4_UrlAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_UrlAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_UrlAtom::AP4_UrlAtom
+---------------------------------------------------------------------*/
AP4_UrlAtom::AP4_UrlAtom() :
    AP4_Atom(AP4_ATOM_TYPE_URL, AP4_FULL_ATOM_HEADER_SIZE, 0, 1)
{
}

/*----------------------------------------------------------------------
|   AP4_UrlAtom::AP4_UrlAtom
+---------------------------------------------------------------------*/
AP4_UrlAtom::AP4_UrlAtom(AP4_UI32        size, 
                         AP4_UI32        version,
                         AP4_UI32        flags,
                         AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_URL, size, version, flags)
{
    if ((m_Flags & 1) == 0) {
        // not self contained
        AP4_Size str_size = size-AP4_FULL_ATOM_HEADER_SIZE;
        if (str_size > 0) {
            char* str = new char[str_size];
            stream.Read(str, str_size);
            str[str_size-1] = '\0'; // force null-termination
            m_Url = str;
            delete[] str;
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_UrlAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_UrlAtom::WriteFields(AP4_ByteStream& stream)
{
    if (m_Flags & 1) {
        // local ref (self contained)
        return AP4_SUCCESS;
    } else {
        // url (not self contained)
        if (m_Size32 > AP4_FULL_ATOM_HEADER_SIZE) {
            AP4_Result result = stream.Write(m_Url.GetChars(), m_Url.GetLength()+1);
            if (AP4_FAILED(result)) return result;

            // pad with zeros if necessary
            AP4_Size padding = m_Size32-(AP4_FULL_ATOM_HEADER_SIZE+m_Url.GetLength()+1);
            while (padding--) stream.WriteUI08(0);
        }
        return AP4_SUCCESS;
    }
}

/*----------------------------------------------------------------------
|   AP4_UrlAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_UrlAtom::InspectFields(AP4_AtomInspector& inspector)
{
    if (m_Flags & 1) {
        inspector.AddField("location", "[local to file]");
    } else {
        inspector.AddField("location", m_Url.GetChars());
    }

    return AP4_SUCCESS;
}
