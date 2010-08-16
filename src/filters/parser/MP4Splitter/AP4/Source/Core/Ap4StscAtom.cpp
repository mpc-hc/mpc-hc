/*****************************************************************
|
|    AP4 - stsc Atoms 
|
|    Copyright 2002-2005 Gilles Boccon-Gibod & Julien Boeuf
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
#include "Ap4StscAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_StscAtom::AP4_StscAtom
+---------------------------------------------------------------------*/
AP4_StscAtom::AP4_StscAtom() :
    AP4_Atom(AP4_ATOM_TYPE_STSC, 4+AP4_FULL_ATOM_HEADER_SIZE, true),   
    m_CachedChunkGroup(0)
{
}

/*----------------------------------------------------------------------
|       AP4_StscAtom::AP4_StscAtom
+---------------------------------------------------------------------*/
AP4_StscAtom::AP4_StscAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_STSC, size, true, stream),
    m_CachedChunkGroup(0)
{
    AP4_UI32 first_sample = 1;
    AP4_UI32 entry_count;
    stream.ReadUI32(entry_count);
    while (entry_count--) {
        AP4_UI32 first_chunk;
        AP4_UI32 samples_per_chunk;
        AP4_UI32 sample_description_index;
        if (stream.ReadUI32(first_chunk)              == AP4_SUCCESS &&
            stream.ReadUI32(samples_per_chunk)        == AP4_SUCCESS &&
            stream.ReadUI32(sample_description_index) == AP4_SUCCESS) {
            if (m_Entries.ItemCount() != 0) {
                AP4_Ordinal prev = m_Entries.ItemCount()-1;
                m_Entries[prev].m_ChunkCount = 
                    first_chunk-m_Entries[prev].m_FirstChunk;
                first_sample += 
                    m_Entries[prev].m_ChunkCount *
                    m_Entries[prev].m_SamplesPerChunk;
            }
            m_Entries.Append(AP4_StscTableEntry(first_chunk, 
                                                first_sample,
                                                samples_per_chunk, 
                                                sample_description_index));
        }
    }
}

/*----------------------------------------------------------------------
|       AP4_StscAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_StscAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // entry count
    AP4_Cardinal entry_count = m_Entries.ItemCount();
    result = stream.WriteUI32(entry_count);

    // entries
    for (AP4_Ordinal i=0; i<entry_count; i++) {
        stream.WriteUI32(m_Entries[i].m_FirstChunk);
        if (AP4_FAILED(result)) return result;
        stream.WriteUI32(m_Entries[i].m_SamplesPerChunk);
        if (AP4_FAILED(result)) return result;
        stream.WriteUI32(m_Entries[i].m_SampleDescriptionIndex);
        if (AP4_FAILED(result)) return result;
    }

    return result;
}

/*----------------------------------------------------------------------
|       AP4_StscAtom::AddEntry
+---------------------------------------------------------------------*/
AP4_Result 
AP4_StscAtom::AddEntry(AP4_Cardinal chunk_count,
                       AP4_Cardinal samples_per_chunk,
                       AP4_Ordinal  sample_description_index)
{
    AP4_Ordinal first_chunk;
    AP4_Ordinal first_sample;
    AP4_Cardinal entry_count = m_Entries.ItemCount();
    if (entry_count == 0) {
        // first entry
        first_chunk = 1;
        first_sample = 1;
    } else {
        first_chunk = m_Entries[entry_count-1].m_FirstChunk+
                      m_Entries[entry_count-1].m_ChunkCount;
        first_sample = m_Entries[entry_count-1].m_FirstSample+
                       m_Entries[entry_count-1].m_ChunkCount*
                       m_Entries[entry_count-1].m_SamplesPerChunk;
    }
    m_Entries.Append(AP4_StscTableEntry(first_chunk, first_sample, chunk_count, samples_per_chunk, sample_description_index));

    // update the atom size
    m_Size += 12;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_StscAtom::GetChunkForSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_StscAtom::GetChunkForSample(AP4_Ordinal  sample,
                                AP4_Ordinal& chunk,
                                AP4_Ordinal& skip,
                                AP4_Ordinal& sample_description)
{
    // preconditions
    AP4_ASSERT(sample > 0);

    // decide whether to start the search from the cached index
    // or from the start
    AP4_Ordinal group;
    if (m_CachedChunkGroup < m_Entries.ItemCount() &&
        m_Entries[m_CachedChunkGroup].m_FirstSample <= sample) {
        group = m_CachedChunkGroup;
    } else {
        group = 0;
    }

    // find which group of chunk contains this one
    while (group < m_Entries.ItemCount()) {
        AP4_Cardinal sample_count = 
            m_Entries[group].m_ChunkCount*m_Entries[group].m_SamplesPerChunk;
        if (sample_count == 0) {
            // unlimited samples in this group (last group)
            if (m_Entries[group].m_FirstSample > sample) {
                // something is wrong
                return AP4_ERROR_INVALID_FORMAT;
            }
        } else {
            // normal group
            if (m_Entries[group].m_FirstSample + sample_count <= sample) {
                // the sample is not in this group
                group++;
                continue;
            }
        }

        // the sample is in this group
        if (m_Entries[group].m_SamplesPerChunk == 0) {
            // something is wrong
            return AP4_ERROR_INVALID_FORMAT;
        }
        unsigned int chunk_offset = 
            ((sample-m_Entries[group].m_FirstSample) / 
            m_Entries[group].m_SamplesPerChunk);
        chunk = m_Entries[group].m_FirstChunk + chunk_offset;
        skip = sample -
            (m_Entries[group].m_FirstSample +
             m_Entries[group].m_SamplesPerChunk*chunk_offset);
        sample_description = m_Entries[group].m_SampleDescriptionIndex;

        // cache the result (to accelerate finding the right group
        // next time around
        m_CachedChunkGroup = group;

        return AP4_SUCCESS;
    }

    // chunk not found
    chunk = 0;
    skip = 0;
    sample_description = 0;
    return AP4_ERROR_OUT_OF_RANGE;
}

/*----------------------------------------------------------------------
|       AP4_StscAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_StscAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry_count", m_Entries.ItemCount());

    // dump table entries
    //for (unsigned int i=0; i<m_Entries.GetItemCount(); i++) {
    //    char dump[256];
    //    sprintf(dump, "  f=%ld, spc=%ld, sdi=%ld\n", 
    //            m_Entries[i].m_FirstChunk,
    //            m_Entries[i].m_SamplesPerChunk,
    //            m_Entries[i].m_SampleDescriptionIndex);
    //    stream.WriteString(prefix);
    //    stream.WriteString(dump);
    //}

    return AP4_SUCCESS;
}

