/*****************************************************************
|
|    AP4 - stsd Atoms 
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
#include "Ap4StsdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"
#include "Ap4SampleEntry.h"
#include "Ap4SampleTable.h"
#include "Ap4SampleDescription.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_StsdAtom)

/*----------------------------------------------------------------------
|   AP4_StsdAtom::Create
+---------------------------------------------------------------------*/
AP4_StsdAtom*
AP4_StsdAtom::Create(AP4_Size         size, 
                     AP4_ByteStream&  stream, 
                     AP4_AtomFactory& atom_factory)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_StsdAtom(size, version, flags, stream, atom_factory);
}

/*----------------------------------------------------------------------
|   AP4_StsdAtom::AP4_StsdAtom
+---------------------------------------------------------------------*/
AP4_StsdAtom::AP4_StsdAtom(AP4_SampleTable* sample_table) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_STSD, (AP4_UI32)0, (AP4_UI32)0)
{
    m_Size32 += 4;
    AP4_Cardinal sample_description_count = sample_table->GetSampleDescriptionCount();
    m_SampleDescriptions.EnsureCapacity(sample_description_count);
    for (AP4_Ordinal i=0; i<sample_description_count; i++) {
        // clear the cache entry
        m_SampleDescriptions.Append(NULL);

        // create an entry for the description
        AP4_SampleDescription* sample_description = sample_table->GetSampleDescription(i);
        AP4_Atom* entry = sample_description->ToAtom();
        m_Children.Add(entry);

        // update the size
        m_Size32 += (AP4_UI32)entry->GetSize();
    }
}

/*----------------------------------------------------------------------
|   AP4_StsdAtom::AP4_StsdAtom
+---------------------------------------------------------------------*/
AP4_StsdAtom::AP4_StsdAtom(AP4_UI32         size,
                           AP4_UI32         version,
                           AP4_UI32         flags,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_STSD, size, false, version, flags)
{
    // read the number of entries
    AP4_UI32 entry_count;
    stream.ReadUI32(entry_count);

    // save and switch the factory's context
    atom_factory.PushContext(m_Type);

    // read all entries
    AP4_LargeSize bytes_available = size-AP4_FULL_ATOM_HEADER_SIZE-4;
    for (unsigned int i=0; i<entry_count; i++) {
        AP4_Atom* atom;
        if (AP4_SUCCEEDED(atom_factory.CreateAtomFromStream(stream, 
                                                            bytes_available,
                                                            atom))) {
            atom->SetParent(this);
            m_Children.Add(atom);
        }
    }

    // restore the saved context
    atom_factory.PopContext();

    // initialize the sample description cache
    m_SampleDescriptions.EnsureCapacity(m_Children.ItemCount());
    for (AP4_Ordinal i=0; i<m_Children.ItemCount(); i++) {
        m_SampleDescriptions.Append(NULL);
    }
}

/*----------------------------------------------------------------------
|   AP4_StsdAtom::~AP4_StsdAtom
+---------------------------------------------------------------------*/
AP4_StsdAtom::~AP4_StsdAtom()
{
    for (AP4_Ordinal i=0; i<m_SampleDescriptions.ItemCount(); i++) {
        delete m_SampleDescriptions[i];
    }
}

/*----------------------------------------------------------------------
|   AP4_StsdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_StsdAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // entry count
    result = stream.WriteUI32(m_Children.ItemCount());
    if (AP4_FAILED(result)) return result;

    // entries
    return m_Children.Apply(AP4_AtomListWriter(stream));
}

/*----------------------------------------------------------------------
|   AP4_StsdAtom::OnChildChanged
+---------------------------------------------------------------------*/
void
AP4_StsdAtom::OnChildChanged(AP4_Atom*)
{
    // remcompute our size
    AP4_UI64 size = GetHeaderSize()+4;
    m_Children.Apply(AP4_AtomSizeAdder(size));
    m_Size32 = (AP4_UI32)size;

    // update our parent
    if (m_Parent) m_Parent->OnChildChanged(this);
}

/*----------------------------------------------------------------------
|   AP4_StsdAtom::GetSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription*
AP4_StsdAtom::GetSampleDescription(AP4_Ordinal index)
{
    // check index
    if (index >= m_Children.ItemCount()) return NULL;

    // return the description if we already have it in the internal table
    if (m_SampleDescriptions[index]) return m_SampleDescriptions[index];

    // create and cache a sample description for this entry
    AP4_Atom* entry;
    m_Children.Get(index, entry);
    AP4_SampleEntry* sample_entry = AP4_DYNAMIC_CAST(AP4_SampleEntry, entry);
    if (sample_entry == NULL) {
        m_SampleDescriptions[index] = new AP4_UnknownSampleDescription(entry);
    } else {
        m_SampleDescriptions[index] = sample_entry->ToSampleDescription();
    }
    return m_SampleDescriptions[index];
}

/*----------------------------------------------------------------------
|   AP4_StsdAtom::GetSampleEntry
+---------------------------------------------------------------------*/
AP4_SampleEntry*
AP4_StsdAtom::GetSampleEntry(AP4_Ordinal index)
{
    // check index
    if (index >= m_Children.ItemCount()) return NULL;

    // return the sample entry
    AP4_Atom* entry;
    m_Children.Get(index, entry);
    return AP4_DYNAMIC_CAST(AP4_SampleEntry, entry);
}

/*----------------------------------------------------------------------
|   AP4_StsdAtom::GetSampleDescriptionCount
+---------------------------------------------------------------------*/
AP4_Cardinal
AP4_StsdAtom::GetSampleDescriptionCount()
{
    return m_Children.ItemCount();
}

/*----------------------------------------------------------------------
|   AP4_StsdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_StsdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry-count", m_Children.ItemCount());
    
    // inspect children
    m_Children.Apply(AP4_AtomListInspector(inspector));

    return AP4_SUCCESS;
}
