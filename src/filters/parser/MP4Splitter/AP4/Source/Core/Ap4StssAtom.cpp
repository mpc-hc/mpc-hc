/*****************************************************************
|
|    AP4 - stss Atoms 
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
#include "Ap4StssAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_StssAtom)

/*----------------------------------------------------------------------
|   AP4_StssAtom::Create
+---------------------------------------------------------------------*/
AP4_StssAtom*
AP4_StssAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_StssAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_StssAtom::AP4_StssAtom
+---------------------------------------------------------------------*/
AP4_StssAtom::AP4_StssAtom() :
    AP4_Atom(AP4_ATOM_TYPE_STSS, AP4_FULL_ATOM_HEADER_SIZE+4, 0, 0)
{
}

/*----------------------------------------------------------------------
|   AP4_StssAtom::AP4_StssAtom
+---------------------------------------------------------------------*/
AP4_StssAtom::AP4_StssAtom(AP4_UI32        size, 
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_STSS, size, version, flags),
    m_LookupCache(0)
{
    AP4_UI32 entry_count;
    stream.ReadUI32(entry_count);
    while (entry_count--) {
        AP4_UI32 entry_sample_index;
        if (stream.ReadUI32(entry_sample_index) == AP4_SUCCESS) {
            m_Entries.Append(entry_sample_index);
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_StssAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_StssAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // entry count
    AP4_Cardinal entry_count = m_Entries.ItemCount();
    result = stream.WriteUI32(entry_count);
    if (AP4_FAILED(result)) return result;

    // entries
    for (AP4_Ordinal i=0; i<entry_count; i++) {
        result = stream.WriteUI32(m_Entries[i]);
        if (AP4_FAILED(result)) return result;
    }

    return result;
}

/*----------------------------------------------------------------------
|   AP4_StssAtom::AddEntry
+---------------------------------------------------------------------*/
AP4_Result
AP4_StssAtom::AddEntry(AP4_UI32 sample)
{
    m_Entries.Append(sample);
    m_Size32 += 4;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_StssAtom::IsSampleSync
+---------------------------------------------------------------------*/
bool
AP4_StssAtom::IsSampleSync(AP4_Ordinal sample)
{
    unsigned int entry_index = 0;

    // check bounds
    if (sample == 0 || m_Entries.ItemCount() == 0) return false;

    // see if we can start from the cached index
    if (m_Entries[m_LookupCache] <= sample) {
        entry_index = m_LookupCache;
    }

    // do a linear search
    while (entry_index < m_Entries.ItemCount() &&
           m_Entries[entry_index] <= sample) {
        if (m_Entries[entry_index] == sample) {
            m_LookupCache = entry_index;
            return true;
        }
	    entry_index++;
    }

    return false;
}

/*----------------------------------------------------------------------
|   AP4_StssAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_StssAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry_count", m_Entries.ItemCount());

    return AP4_SUCCESS;
}
    
