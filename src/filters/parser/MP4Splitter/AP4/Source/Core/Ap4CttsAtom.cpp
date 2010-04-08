/*****************************************************************
|
|    AP4 - ctts Atoms
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
#include "Ap4CttsAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_CttsAtom)

/*----------------------------------------------------------------------
|   AP4_CttsAtom::Create
+---------------------------------------------------------------------*/
AP4_CttsAtom*
AP4_CttsAtom::Create(AP4_UI32 size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version != 0) return NULL;
    return new AP4_CttsAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_CttsAtom::AP4_CttsAtom
+---------------------------------------------------------------------*/
AP4_CttsAtom::AP4_CttsAtom() :
    AP4_Atom(AP4_ATOM_TYPE_CTTS, AP4_FULL_ATOM_HEADER_SIZE + 4, 0, 0)
{
    m_LookupCache.sample      = 0;
    m_LookupCache.entry_index = 0;
}

/*----------------------------------------------------------------------
|   AP4_CttsAtom::AP4_CttsAtom
+---------------------------------------------------------------------*/
AP4_CttsAtom::AP4_CttsAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_CTTS, size, version, flags)
{
    m_LookupCache.sample      = 0;
    m_LookupCache.entry_index = 0;

    AP4_UI32 entry_count;
    stream.ReadUI32(entry_count);
    m_Entries.SetItemCount(entry_count);
    unsigned char* buffer = new unsigned char[entry_count*8];
    AP4_Result result = stream.Read(buffer, entry_count * 8);
    if(AP4_FAILED(result))
    {
        delete[] buffer;
        return;
    }
    //bool use_quicktime_format = false;
    //AP4_SI32 quicktime_min_offset = 0;
    for(unsigned i = 0; i < entry_count; i++)
    {
        m_Entries[i].m_SampleCount  = AP4_BytesToUInt32BE(&buffer[i*8  ]);
        AP4_UI32 offset             = AP4_BytesToUInt32BE(&buffer[i*8+4]);
        //if (offset & 0x80000000) {
        //    use_quicktime_format = true;
        //    AP4_SI32 noffset = (AP4_SI32)offset;
        //    if (noffset < quicktime_min_offset) quicktime_min_offset = noffset;
        //}
        m_Entries[i].m_SampleOffset = offset;
    }
    delete[] buffer;

    // in the quicktime format, the offsets can be positive or negative, so
    // we need to adjust for them here
    //if (use_quicktime_format) {
    //    for (unsigned i=0; i<entry_count; i++) {
    //        m_Entries[i].m_SampleOffset -= quicktime_min_offset;
    //    }
    //}
}

/*----------------------------------------------------------------------
|   AP4_CttsAtom::AddEntry
+---------------------------------------------------------------------*/
AP4_Result
AP4_CttsAtom::AddEntry(AP4_UI32 count, AP4_UI32 cts_offset)
{
    m_Entries.Append(AP4_CttsTableEntry(count, cts_offset));
    m_Size32 += 8;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_CttsAtom::GetCtsOffset
+---------------------------------------------------------------------*/
AP4_Result
AP4_CttsAtom::GetCtsOffset(AP4_Ordinal sample, AP4_UI32& cts_offset)
{
    // default value
    cts_offset = 0;

    // sample indexes start at 1
    if(sample == 0) return AP4_ERROR_OUT_OF_RANGE;

    // check the lookup cache
    AP4_Ordinal lookup_start = 0;
    AP4_Ordinal sample_start = 0;
    if(sample >= m_LookupCache.sample)
    {
        // start from the cached entry
        lookup_start = m_LookupCache.entry_index;
        sample_start = m_LookupCache.sample;
    }

    for(AP4_Ordinal i = lookup_start; i < m_Entries.ItemCount(); i++)
    {
        AP4_CttsTableEntry& entry = m_Entries[i];

        // check if we have reached the sample
        if(sample <= sample_start + entry.m_SampleCount)
        {
            // we are within the sample range for the current entry
            cts_offset = entry.m_SampleOffset;

            // update the lookup cache
            m_LookupCache.entry_index = i;
            m_LookupCache.sample      = sample_start;

            return AP4_SUCCESS;
        }

        // update the upper bound
        sample_start += entry.m_SampleCount;
    }

    // sample is greater than the number of samples
    return AP4_ERROR_OUT_OF_RANGE;
}

/*----------------------------------------------------------------------
|   AP4_CttsAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_CttsAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // write the entry count
    AP4_Cardinal entry_count = m_Entries.ItemCount();
    result = stream.WriteUI32(entry_count);
    if(AP4_FAILED(result)) return result;

    // write the entries
    for(AP4_Ordinal i = 0; i < entry_count; i++)
    {
        // sample count
        result = stream.WriteUI32(m_Entries[i].m_SampleCount);
        if(AP4_FAILED(result)) return result;

        // time offset
        result = stream.WriteUI32(m_Entries[i].m_SampleOffset);
        if(AP4_FAILED(result)) return result;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_CttsAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_CttsAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry_count", m_Entries.ItemCount());

    if(inspector.GetVerbosity() >= 2)
    {
        char header[32];
        char value[64];
        for(AP4_Ordinal i = 0; i < m_Entries.ItemCount(); i++)
        {
            AP4_FormatString(header, sizeof(header), "entry %8d", i);
            AP4_FormatString(value, sizeof(value), "count=%d, offset=%d",
                             m_Entries[i].m_SampleCount,
                             m_Entries[i].m_SampleOffset);
            inspector.AddField(header, value);
        }
    }

    return AP4_SUCCESS;
}
