/*****************************************************************
|
|    AP4 - schm Atoms
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
#include "Ap4SchmAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_SchmAtom)

/*----------------------------------------------------------------------
|   AP4_SchmAtom::Create
+---------------------------------------------------------------------*/
AP4_SchmAtom*
AP4_SchmAtom::Create(AP4_Size                   size,
                     AP4_Array<AP4_Atom::Type>* context,
                     AP4_ByteStream&            stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version != 0) return NULL;
    if(size < AP4_FULL_ATOM_HEADER_SIZE + 6) return NULL;

    // check the context to see if this is a short form atom or not
    bool short_form = false;
    if(size < AP4_FULL_ATOM_HEADER_SIZE + 8) short_form = true;
    if(context)
    {
        AP4_Size context_depth = context->ItemCount();
        if(context_depth >= 2 &&
           (*context)[context_depth-2] == AP4_ATOM_TYPE('m', 'r', 'l', 'n'))
        {
            short_form = true;
        }
    }

    return new AP4_SchmAtom(size, version, flags, short_form, stream);
}

/*----------------------------------------------------------------------
|   AP4_SchmAtom::AP4_SchmAtom
+---------------------------------------------------------------------*/
AP4_SchmAtom::AP4_SchmAtom(AP4_UI32    scheme_type,
                           AP4_UI32    scheme_version,
                           const char* scheme_uri,
                           bool        short_form) :
    AP4_Atom(AP4_ATOM_TYPE_SCHM, AP4_FULL_ATOM_HEADER_SIZE + 4 + (short_form ? 2 : 4), 0, 0),
    m_AtomHasShortForm(short_form),
    m_SchemeType(scheme_type),
    m_SchemeVersion(scheme_version)
{
    if(scheme_uri)
    {
        m_SchemeUri = scheme_uri;
        m_Flags = 1;
        m_Size32 += m_SchemeUri.GetLength() + 1;
    }
}

/*----------------------------------------------------------------------
|   AP4_SchmAtom::AP4_SchmAtom
+---------------------------------------------------------------------*/
AP4_SchmAtom::AP4_SchmAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           bool            short_form,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_SCHM, size, version, flags),
    m_AtomHasShortForm(short_form)
{
    stream.ReadUI32(m_SchemeType);
    if(short_form)
    {
        AP4_UI16 short_version;
        stream.ReadUI16(short_version);
        m_SchemeVersion = short_version;
    }
    else
    {
        stream.ReadUI32(m_SchemeVersion);
    }
    if(m_Flags & 1)
    {
        int str_size = size - (AP4_FULL_ATOM_HEADER_SIZE + 8);
        if(str_size > 0)
        {
            char* str = new char[str_size];
            stream.Read(str, str_size);
            str[str_size-1] = '\0'; // force null-termination
            m_SchemeUri = str;
            delete[] str;
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_SchmAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_SchmAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // scheme type
    result = stream.WriteUI32(m_SchemeType);
    if(AP4_FAILED(result)) return result;

    // scheme version
    if(m_AtomHasShortForm)
    {
        result = stream.WriteUI16((AP4_UI16)m_SchemeVersion);
        if(AP4_FAILED(result)) return result;
    }
    else
    {
        result = stream.WriteUI32(m_SchemeVersion);
        if(AP4_FAILED(result)) return result;
    }

    // uri if needed
    if(m_Flags & 1)
    {
        result = stream.Write(m_SchemeUri.GetChars(), m_SchemeUri.GetLength() + 1);
        if(AP4_FAILED(result)) return result;

        // pad with zeros if necessary
        AP4_Size fields_size = 4 + (m_AtomHasShortForm ? 2 : 4);
        AP4_Size padding = m_Size32 - (AP4_FULL_ATOM_HEADER_SIZE + fields_size + m_SchemeUri.GetLength() + 1);
        while(padding--) stream.WriteUI08(0);
    }

    return result;
}

/*----------------------------------------------------------------------
|   AP4_SchmAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_SchmAtom::InspectFields(AP4_AtomInspector& inspector)
{
    char st[5];
    AP4_FormatFourChars(st, m_SchemeType);
    inspector.AddField("scheme_type", st);
    if(m_AtomHasShortForm)
    {
        inspector.AddField("scheme_version (short)", m_SchemeVersion);
    }
    else
    {
        inspector.AddField("scheme_version", m_SchemeVersion);
    }
    if(m_Flags & 1)
    {
        inspector.AddField("scheme_uri", m_SchemeUri.GetChars());
    }

    return AP4_SUCCESS;
}
