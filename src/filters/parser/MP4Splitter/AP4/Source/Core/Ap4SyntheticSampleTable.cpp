/*****************************************************************
|
|    AP4 - Synthetic Sample Table
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
#include "Ap4Types.h"
#include "Ap4Atom.h"
#include "Ap4SyntheticSampleTable.h"
#include "Ap4Sample.h"

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::AP4_SyntheticSampleTable()
+---------------------------------------------------------------------*/
AP4_SyntheticSampleTable::AP4_SyntheticSampleTable(AP4_Cardinal chunk_size) :
    m_ChunkSize(chunk_size?chunk_size:AP4_SYNTHETIC_SAMPLE_TABLE_DEFAULT_CHUNK_SIZE)
{
    m_LookupCache.m_Sample = 0;
    m_LookupCache.m_Chunk  = 0;
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::~AP4_SyntheticSampleTable()
+---------------------------------------------------------------------*/
AP4_SyntheticSampleTable::~AP4_SyntheticSampleTable()
{
    m_SampleDescriptions.DeleteReferences();
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::GetSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_SyntheticSampleTable::GetSample(AP4_Ordinal sample_index, AP4_Sample& sample)
{
    if (sample_index >= m_Samples.ItemCount()) return AP4_ERROR_OUT_OF_RANGE;

    sample = m_Samples[sample_index];
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::GetSampleCount
+---------------------------------------------------------------------*/
AP4_Cardinal 
AP4_SyntheticSampleTable::GetSampleCount()
{
    return m_Samples.ItemCount();
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::GetSampleChunkPosition
+---------------------------------------------------------------------*/
AP4_Result   
AP4_SyntheticSampleTable::GetSampleChunkPosition(
    AP4_Ordinal  sample_index, 
    AP4_Ordinal& chunk_index,
    AP4_Ordinal& position_in_chunk)
{
    // default values
    chunk_index       = 0;
    position_in_chunk = 0;
    
    // check parameters
    if (sample_index >= m_Samples.ItemCount()) return AP4_ERROR_OUT_OF_RANGE;
    
    // look for the chunk 
    AP4_Ordinal sample_cursor = 0;
    AP4_Ordinal chunk_cursor  = 0;
    if (sample_index >= m_LookupCache.m_Sample) {
        sample_cursor = m_LookupCache.m_Sample;
        chunk_cursor  = m_LookupCache.m_Chunk;
    }
    for (; 
         chunk_cursor < m_SamplesInChunk.ItemCount(); 
         sample_cursor += m_SamplesInChunk[chunk_cursor++]) {
        if (sample_cursor+m_SamplesInChunk[chunk_cursor] > sample_index) {
            chunk_index = chunk_cursor;
            position_in_chunk = sample_index-sample_cursor;
            m_LookupCache.m_Sample = sample_cursor;
            m_LookupCache.m_Chunk  = chunk_cursor;
            return AP4_SUCCESS;
        }
    }
    
    return AP4_ERROR_OUT_OF_RANGE;
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::GetSampleDescriptionCount
+---------------------------------------------------------------------*/
AP4_Cardinal 
AP4_SyntheticSampleTable::GetSampleDescriptionCount()
{
    return m_SampleDescriptions.ItemCount();
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::GetSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription* 
AP4_SyntheticSampleTable::GetSampleDescription(AP4_Ordinal index)
{
    SampleDescriptionHolder* holder;
    if (AP4_SUCCEEDED(m_SampleDescriptions.Get(index, holder))) {
        return holder->m_SampleDescription;
    } else {
        return NULL;
    }
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::AddSampleDescription
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SyntheticSampleTable::AddSampleDescription(AP4_SampleDescription* description,
                                               bool                   transfer_ownership)
{
    return m_SampleDescriptions.Add(new SampleDescriptionHolder(description, transfer_ownership));
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::AddSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SyntheticSampleTable::AddSample(AP4_ByteStream& data_stream,
                                    AP4_Position    offset,
                                    AP4_Size        size,
                                    AP4_UI32        duration,
                                    AP4_Ordinal     description_index,
                                    AP4_UI64        dts,
                                    AP4_UI32        cts_delta,
                                    bool            sync)
{
    // decide if we need to start a new chunk or increment the last one
    if (m_SamplesInChunk.ItemCount() == 0 ||
        m_SamplesInChunk[m_SamplesInChunk.ItemCount()-1] >= m_ChunkSize ||
        m_Samples.ItemCount() == 0 ||
        m_Samples[m_Samples.ItemCount()-1].GetDescriptionIndex() != description_index) {
        m_SamplesInChunk.Append(1);
    } else {
        ++m_SamplesInChunk[m_SamplesInChunk.ItemCount()-1];
    }
    
    // compute the timestamps
    if (m_Samples.ItemCount() > 0) {
        AP4_Sample* prev_sample = &m_Samples[m_Samples.ItemCount()-1];
        if (dts == 0) {
            if (prev_sample->GetDuration() == 0) {
                // can't compute the DTS for this sample
                return AP4_ERROR_INVALID_PARAMETERS;
            }
            dts = prev_sample->GetDts()+prev_sample->GetDuration();
        } else {
            if (prev_sample->GetDuration() == 0) {
                // update the previous sample
                if (dts <= prev_sample->GetDts()) return AP4_ERROR_INVALID_PARAMETERS;
                prev_sample->SetDuration((AP4_UI32)(dts-prev_sample->GetDts()));
            } else {
                if (dts != prev_sample->GetDts()+prev_sample->GetDuration()) {
                    // mismatch
                    return AP4_ERROR_INVALID_PARAMETERS;
                }
            }
        }
    }
    
    // add the sample to the table
    AP4_Sample sample(data_stream, offset, size, duration, description_index, dts, cts_delta, sync);
    return m_Samples.Append(sample);
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::GetSampleIndexForTimeStamp
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SyntheticSampleTable::GetSampleIndexForTimeStamp(AP4_UI64     /* ts */, 
                                                     AP4_Ordinal& /* index */)
{
    return AP4_ERROR_NOT_SUPPORTED;
}

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable::GetNearestSyncSampleIndex
+---------------------------------------------------------------------*/
AP4_Ordinal  
AP4_SyntheticSampleTable::GetNearestSyncSampleIndex(AP4_Ordinal sample_index, bool before)
{
    if (before) {
        for (int i=sample_index; i>=0; i--) {
            if (m_Samples[i].IsSync()) return i;
        }
        // not found?
        return 0;
    } else {
        AP4_Cardinal entry_count = m_Samples.ItemCount();
        for (unsigned int i=sample_index; i<entry_count; i++) {
            if (m_Samples[i].IsSync()) return i;
        }
        // not found?
        return m_Samples.ItemCount();
    }
}
