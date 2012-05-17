/*****************************************************************
|
|    AP4 - stts Atoms 
|
|    Copyright 2003 Gilles Boccon-Gibod & Julien Boeuf
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
#include "Ap4SttsAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_SttsAtom::AP4_SttsAtom
+---------------------------------------------------------------------*/
AP4_SttsAtom::AP4_SttsAtom() :
    AP4_Atom(AP4_ATOM_TYPE_STTS, AP4_FULL_ATOM_HEADER_SIZE+4, true)
{
}

/*----------------------------------------------------------------------
|       AP4_SttsAtom::AP4_SttsAtom
+---------------------------------------------------------------------*/
AP4_SttsAtom::AP4_SttsAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_STTS, size, true, stream)
{
    AP4_UI32 entry_count;
    stream.ReadUI32(entry_count);
    while (entry_count--) {
        AP4_UI32 sample_count;
        AP4_UI32 sample_duration;
        if (stream.ReadUI32(sample_count)    == AP4_SUCCESS &&
            stream.ReadUI32(sample_duration) == AP4_SUCCESS) {
// mpc-hc custom code start
            if((int)sample_duration < 0) {
                sample_duration = 0;
            }
// mpc-hc custom code end
            m_Entries.Append(AP4_SttsTableEntry(sample_count,
                                                sample_duration));
        }
    }
}

/*----------------------------------------------------------------------
|       AP4_SttsAtom::GetDts
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::GetDts(AP4_Ordinal sample, AP4_TimeStamp& dts, AP4_Duration& duration)
{
    AP4_Ordinal sample_count_in_entry = sample;
    dts = 0;

    for (AP4_UI32 i = 0; i < m_Entries.ItemCount(); i++) {
        AP4_SttsTableEntry& entry = m_Entries[i];

        // check if we have the correct entry 
        if (sample_count_in_entry <= entry.m_SampleCount) {
            dts += (sample_count_in_entry - 1) * entry.m_SampleDuration;
            duration = entry.m_SampleDuration;
            return AP4_SUCCESS;
        } else {
            dts += entry.m_SampleCount * entry.m_SampleDuration;
            sample_count_in_entry -= entry.m_SampleCount;
        }
    }

    // sample is greater than the number of samples
    return AP4_FAILURE;
}

/*----------------------------------------------------------------------
|       AP4_SttsAtom::AddEntry
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::AddEntry(AP4_UI32 sample_count, AP4_UI32 sample_duration)
{
    m_Entries.Append(AP4_SttsTableEntry(sample_count, sample_duration));
    m_Size += 8;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_SttsAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // write the entry count
    AP4_Cardinal entry_count = m_Entries.ItemCount();
    result = stream.WriteUI32(entry_count);
    if (AP4_FAILED(result)) return result;

    // write the entries
    for (AP4_Ordinal i=0; i<entry_count; i++) {
        // sample count
        result = stream.WriteUI32(m_Entries[i].m_SampleCount);
        if (AP4_FAILED(result)) return result;

        // time offset
        result = stream.WriteUI32(m_Entries[i].m_SampleDuration);
        if (AP4_FAILED(result)) return result;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_SttsAtom::GetSampleIndexForTimeStamp
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::GetSampleIndexForTimeStamp(AP4_TimeStamp ts, AP4_Ordinal& sample)
{
    // init
    AP4_Cardinal entry_count = m_Entries.ItemCount();
    AP4_Duration accumulated = 0;
    sample = 0;
    
    for (AP4_Ordinal i=0; i<entry_count; i++) {
        AP4_Duration next_accumulated = accumulated 
            + m_Entries[i].m_SampleCount * m_Entries[i].m_SampleDuration;
        
        // check if the ts is in the range of this entry
        if (ts < next_accumulated) {
            sample += (AP4_Ordinal) ((ts - accumulated) / m_Entries[i].m_SampleDuration);
            return AP4_SUCCESS;
        }

        // update accumulated and sample
        accumulated = next_accumulated;
        sample += m_Entries[i].m_SampleCount;
    }

    // ts not in range of the table
    return AP4_FAILURE;
}

/*----------------------------------------------------------------------
|       AP4_SttsAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_SttsAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry_count", m_Entries.ItemCount());

    return AP4_SUCCESS;
}
