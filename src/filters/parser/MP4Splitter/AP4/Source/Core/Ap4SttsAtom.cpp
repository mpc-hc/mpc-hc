/*****************************************************************
|
|    AP4 - stts Atoms
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
#include "Ap4SttsAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_SttsAtom)

/*----------------------------------------------------------------------
|   AP4_SttsAtom::Create
+---------------------------------------------------------------------*/
AP4_SttsAtom*
AP4_SttsAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version != 0) return NULL;
    return new AP4_SttsAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_SttsAtom::AP4_SttsAtom
+---------------------------------------------------------------------*/
AP4_SttsAtom::AP4_SttsAtom() :
    AP4_Atom(AP4_ATOM_TYPE_STTS, AP4_FULL_ATOM_HEADER_SIZE + 4, 0, 0)
{
    m_LookupCache.entry_index = 0;
    m_LookupCache.sample      = 0;
    m_LookupCache.dts         = 0;
}

/*----------------------------------------------------------------------
|   AP4_SttsAtom::AP4_SttsAtom
+---------------------------------------------------------------------*/
AP4_SttsAtom::AP4_SttsAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_STTS, size, version, flags)
{
    m_LookupCache.entry_index = 0;
    m_LookupCache.sample      = 0;
    m_LookupCache.dts         = 0;

    AP4_UI32 entry_count;
    stream.ReadUI32(entry_count);
    while(entry_count--)
    {
        AP4_UI32 sample_count;
        AP4_UI32 sample_duration;
        if(stream.ReadUI32(sample_count)    == AP4_SUCCESS &&
           stream.ReadUI32(sample_duration) == AP4_SUCCESS)
        {
            m_Entries.Append(AP4_SttsTableEntry(sample_count,
                                                sample_duration));
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_SttsAtom::GetDts
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::GetDts(AP4_Ordinal sample, AP4_UI64& dts, AP4_UI32* duration)
{
    // default value
    dts = 0;
    if(duration) *duration = 0;

    // sample indexes start at 1
    if(sample == 0) return AP4_ERROR_OUT_OF_RANGE;

    // check the lookup cache
    AP4_Ordinal lookup_start  = 0;
    AP4_Ordinal sample_start = 0;
    AP4_UI64    dts_start    = 0;
    if(sample >= m_LookupCache.sample)
    {
        // start from the cached entry
        lookup_start = m_LookupCache.entry_index;
        sample_start = m_LookupCache.sample;
        dts_start    = m_LookupCache.dts;
    }

    // look from the last known point
    for(AP4_Ordinal i = lookup_start; i < m_Entries.ItemCount(); i++)
    {
        AP4_SttsTableEntry& entry = m_Entries[i];

        // check if we have reached the sample
        if(sample <= sample_start + entry.m_SampleCount)
        {
            // we are within the sample range for the current entry
            dts = dts_start + (AP4_UI64)(sample - 1 - sample_start) * (AP4_UI64)entry.m_SampleDuration;
            if(duration) *duration = entry.m_SampleDuration;

            // update the lookup cache
            m_LookupCache.entry_index = i;
            m_LookupCache.sample      = sample_start;
            m_LookupCache.dts         = dts_start;

            return AP4_SUCCESS;
        }

        // update the sample and dts bases
        sample_start += entry.m_SampleCount;
        dts_start    += entry.m_SampleCount * entry.m_SampleDuration;
    }

    // sample is greater than the number of samples
    return AP4_ERROR_OUT_OF_RANGE;
}

/*----------------------------------------------------------------------
|   AP4_SttsAtom::AddEntry
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::AddEntry(AP4_UI32 sample_count, AP4_UI32 sample_duration)
{
    m_Entries.Append(AP4_SttsTableEntry(sample_count, sample_duration));
    m_Size32 += 8;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_SttsAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::WriteFields(AP4_ByteStream& stream)
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
        result = stream.WriteUI32(m_Entries[i].m_SampleDuration);
        if(AP4_FAILED(result)) return result;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_SttsAtom::GetSampleIndexForTimeStamp
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::GetSampleIndexForTimeStamp(AP4_UI64      ts,
        AP4_Ordinal&  sample_index)
{
    // init
    AP4_Cardinal entry_count = m_Entries.ItemCount();
    AP4_UI64 accumulated = 0;
    sample_index = 0;

    for(AP4_Ordinal i = 0; i < entry_count; i++)
    {
        AP4_UI64 next_accumulated =
            accumulated +
            (AP4_UI64)m_Entries[i].m_SampleCount *
            (AP4_UI64)m_Entries[i].m_SampleDuration;

        // check if the ts is in the range of this entry
        if(ts < next_accumulated)
        {
            sample_index += (AP4_UI32)((ts - accumulated) / m_Entries[i].m_SampleDuration);
            return AP4_SUCCESS;
        }

        // update accumulated and sample
        accumulated = next_accumulated;
        sample_index += m_Entries[i].m_SampleCount;
    }

    // ts not in range of the table
    return AP4_FAILURE;
}

/*----------------------------------------------------------------------
|   AP4_SttsAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry_count", m_Entries.ItemCount());

    if(inspector.GetVerbosity() >= 1)
    {
        char header[32];
        char value[256];
        for(AP4_Ordinal i = 0; i < m_Entries.ItemCount(); i++)
        {
            AP4_FormatString(header, sizeof(header), "entry %8d", i);
            AP4_FormatString(value, sizeof(value),
                             "sample_count=%d, sample_duration=%d",
                             m_Entries[i].m_SampleCount,
                             m_Entries[i].m_SampleDuration);
            inspector.AddField(header, value);
        }
    }

    return AP4_SUCCESS;
}
