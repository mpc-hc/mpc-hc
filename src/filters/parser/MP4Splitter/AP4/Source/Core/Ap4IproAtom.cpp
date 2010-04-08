/*****************************************************************
|
|    AP4 - ipro Atoms
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
#include "Ap4IproAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"
#include "Ap4SampleEntry.h"
#include "Ap4SampleTable.h"
#include "Ap4SampleDescription.h"

/*----------------------------------------------------------------------
|   AP4_IproAtom::Create
+---------------------------------------------------------------------*/
AP4_IproAtom*
AP4_IproAtom::Create(AP4_Size         size,
                     AP4_ByteStream&  stream,
                     AP4_AtomFactory& atom_factory)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version != 0) return NULL;
    return new AP4_IproAtom(size, version, flags, stream, atom_factory);
}

/*----------------------------------------------------------------------
|   AP4_IproAtom::AP4_IproAtom
+---------------------------------------------------------------------*/
AP4_IproAtom::AP4_IproAtom(AP4_UI32         size,
                           AP4_UI32         version,
                           AP4_UI32         flags,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_IPRO, size, false, version, flags)
{
    // read the number of entries
    AP4_UI16 entry_count;
    stream.ReadUI16(entry_count);

    // read all entries
    AP4_LargeSize bytes_available = size - AP4_FULL_ATOM_HEADER_SIZE - 2;
    for(unsigned int i = 0; i < entry_count; i++)
    {
        AP4_Atom* atom;
        if(AP4_SUCCEEDED(atom_factory.CreateAtomFromStream(stream,
                         bytes_available,
                         atom)))
        {
            atom->SetParent(this);
            m_Children.Add(atom);
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_IproAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IproAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // entry count
    result = stream.WriteUI16(m_Children.ItemCount());
    if(AP4_FAILED(result)) return result;

    // entries
    return m_Children.Apply(AP4_AtomListWriter(stream));
}

/*----------------------------------------------------------------------
|   AP4_IproAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IproAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry-count", m_Children.ItemCount());

    // inspect children
    m_Children.Apply(AP4_AtomListInspector(inspector));

    return AP4_SUCCESS;
}
