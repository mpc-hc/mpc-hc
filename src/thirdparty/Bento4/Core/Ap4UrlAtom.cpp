/*****************************************************************
|
|    AP4 - url Atoms 
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
#include "Ap4UrlAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_UrlAtom::AP4_UrlAtom
+---------------------------------------------------------------------*/
AP4_UrlAtom::AP4_UrlAtom() :
    AP4_Atom(AP4_ATOM_TYPE_URL, AP4_FULL_ATOM_HEADER_SIZE, true)
{
    m_Flags = 1; // local ref
}

/*----------------------------------------------------------------------
|       AP4_UrlAtom::AP4_UrlAtom
+---------------------------------------------------------------------*/
AP4_UrlAtom::AP4_UrlAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_URL, size, true, stream)
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
|       AP4_UrlAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_UrlAtom::WriteFields(AP4_ByteStream& stream)
{
    if (m_Flags & 1) {
        // local ref (self contained)
        return AP4_SUCCESS;
    } else {
        // url (not self contained)
        AP4_Result result = stream.Write(m_Url.c_str(), m_Url.length()+1);
        if (AP4_FAILED(result)) return result;

        // pad with zeros if necessary
        AP4_Size padding = m_Size-(AP4_FULL_ATOM_HEADER_SIZE+m_Url.length()+1);
        while (padding--) stream.WriteUI08(0);
        return AP4_SUCCESS;
    }
}

/*----------------------------------------------------------------------
|       AP4_UrlAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_UrlAtom::InspectFields(AP4_AtomInspector& inspector)
{
    if (m_Flags & 1) {
        inspector.AddField("location", "[local to file]");
    } else {
        inspector.AddField("location", m_Url.c_str());
    }

    return AP4_SUCCESS;
}
