/*****************************************************************
|
|    AP4 - odhe Atoms
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
#include "Ap4OdheAtom.h"
#include "Ap4OhdrAtom.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_OdheAtom)

/*----------------------------------------------------------------------
|   AP4_OdheAtom::Create
+---------------------------------------------------------------------*/
AP4_OdheAtom*
AP4_OdheAtom::Create(AP4_Size         size,
                     AP4_ByteStream&  stream,
                     AP4_AtomFactory& atom_factory)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version != 0) return NULL;
    return new AP4_OdheAtom(size, version, flags, stream, atom_factory);
}

/*----------------------------------------------------------------------
|   AP4_OdheAtom::AP4_OdheAtom
+---------------------------------------------------------------------*/
AP4_OdheAtom::AP4_OdheAtom(AP4_UI32         size,
                           AP4_UI32         version,
                           AP4_UI32         flags,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_ODHE, size, false, version, flags)
{
    // read the content type
    AP4_UI08 content_type_length;
    stream.ReadUI08(content_type_length);
    char content_type[256];
    stream.Read(content_type, content_type_length);
    m_ContentType.Assign(content_type, content_type_length);

    // read the children
    AP4_Size bytes_available = size - (AP4_FULL_ATOM_HEADER_SIZE + 1 + content_type_length);
    ReadChildren(atom_factory, stream, bytes_available);
}

/*----------------------------------------------------------------------
|   AP4_OdheAtom::AP4_OdheAtom
+---------------------------------------------------------------------*/
AP4_OdheAtom::AP4_OdheAtom(const char*   content_type,
                           AP4_OhdrAtom* ohdr) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_ODHE, (AP4_UI32)0, (AP4_UI32)0),
    m_ContentType(content_type)
{
    m_Size32 += 1 + m_ContentType.GetLength();
    AddChild(ohdr);
}

/*----------------------------------------------------------------------
|   AP4_OdheAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_OdheAtom::WriteFields(AP4_ByteStream& stream)
{
    // write the content type
    AP4_CHECK(stream.WriteUI08((AP4_UI08)m_ContentType.GetLength()));
    if(m_ContentType.GetLength())
    {
        AP4_CHECK(stream.Write(m_ContentType.GetChars(), m_ContentType.GetLength()));
    }

    // write the children
    return m_Children.Apply(AP4_AtomListWriter(stream));
}

/*----------------------------------------------------------------------
|   AP4_OdheAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_OdheAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("content_type", m_ContentType.GetChars());
    return InspectChildren(inspector);
}

/*----------------------------------------------------------------------
|   AP4_OdheAtom::OnChildChanged
+---------------------------------------------------------------------*/
void
AP4_OdheAtom::OnChildChanged(AP4_Atom*)
{
    // remcompute our size
    AP4_UI64 size = GetHeaderSize() + 1 + m_ContentType.GetLength();
    m_Children.Apply(AP4_AtomSizeAdder(size));
    SetSize(size);

    // update our parent
    if(m_Parent) m_Parent->OnChildChanged(this);
}

