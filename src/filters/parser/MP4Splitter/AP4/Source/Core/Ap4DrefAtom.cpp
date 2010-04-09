/*****************************************************************
|
|    AP4 - dref Atoms 
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
#include "Ap4DrefAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"
#include "Ap4ContainerAtom.h"

/*----------------------------------------------------------------------
|   AP4_DrefAtom::Create
+---------------------------------------------------------------------*/
AP4_DrefAtom* 
AP4_DrefAtom::Create(AP4_UI32         size,
                     AP4_ByteStream&  stream,
                     AP4_AtomFactory& atom_factory)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_DrefAtom(size, version, flags, stream, atom_factory);
}

/*----------------------------------------------------------------------
|   AP4_DrefAtom::AP4_DrefAtom
+---------------------------------------------------------------------*/
AP4_DrefAtom::AP4_DrefAtom(AP4_Atom** refs, AP4_Cardinal refs_count) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_DREF, (AP4_UI32)0, (AP4_UI32)0)
{
    m_Size32 += 4;
    for (unsigned i=0; i<refs_count; i++) {
        m_Children.Add(refs[i]);
        m_Size32 += (AP4_UI32)refs[i]->GetSize();
    }
}

/*----------------------------------------------------------------------
|   AP4_DrefAtom::AP4_DrefAtom
+---------------------------------------------------------------------*/
AP4_DrefAtom::AP4_DrefAtom(AP4_UI32         size,
                           AP4_UI32         version,
                           AP4_UI32         flags,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_DREF, size, false, version, flags)
{
    // read the number of entries
    AP4_UI32 entry_count;
    stream.ReadUI32(entry_count);

    // read children
    AP4_LargeSize bytes_available = size-AP4_FULL_ATOM_HEADER_SIZE-4;
    while (entry_count--) {
        AP4_Atom* atom; 
        while (AP4_SUCCEEDED(atom_factory.CreateAtomFromStream(stream, 
                                                               bytes_available,
                                                               atom))) {
            m_Children.Add(atom);
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_DrefAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DrefAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // write the number of entries
    result = stream.WriteUI32(m_Children.ItemCount());
    if (AP4_FAILED(result)) return result;

    // write the children
    return m_Children.Apply(AP4_AtomListWriter(stream));
}
