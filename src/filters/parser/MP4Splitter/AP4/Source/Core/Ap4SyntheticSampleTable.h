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

#ifndef _AP4_SYNTHETIC_SAMPLE_TABLE_H_
#define _AP4_SYNTHETIC_SAMPLE_TABLE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Array.h"
#include "Ap4List.h"
#include "Ap4Sample.h"
#include "Ap4SampleTable.h"
#include "Ap4SampleDescription.h"

/*----------------------------------------------------------------------
|   forward declarations
+---------------------------------------------------------------------*/
class AP4_ByteStream;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_Cardinal AP4_SYNTHETIC_SAMPLE_TABLE_DEFAULT_CHUNK_SIZE = 10;

/*----------------------------------------------------------------------
|   AP4_SyntheticSampleTable
+---------------------------------------------------------------------*/
class AP4_SyntheticSampleTable : public AP4_SampleTable
{
public:
    // methods
    AP4_SyntheticSampleTable(AP4_Cardinal chunk_size
                             = AP4_SYNTHETIC_SAMPLE_TABLE_DEFAULT_CHUNK_SIZE);
    virtual ~AP4_SyntheticSampleTable();

    // AP4_SampleTable methods
    virtual AP4_Result GetSample(AP4_Ordinal index, AP4_Sample& sample);
    virtual AP4_Cardinal GetSampleCount();
    virtual AP4_Result   GetSampleChunkPosition(AP4_Ordinal  sample_index,
            AP4_Ordinal& chunk_index,
            AP4_Ordinal& position_in_chunk);
    virtual AP4_Cardinal GetSampleDescriptionCount();
    virtual AP4_SampleDescription* GetSampleDescription(AP4_Ordinal index);
    virtual AP4_Result GetSampleIndexForTimeStamp(AP4_UI64 ts, AP4_Ordinal& index);
    virtual AP4_Ordinal  GetNearestSyncSampleIndex(AP4_Ordinal index, bool before);

    // methods
    /**
     * Add a sample description to the sample table.
     * Each added sample description will have the next available index, starting at 0
     *
     * @param description Pointer to the sample description to add
     * @param transfer_ownership Boolean flag indicating whether the ownership of the
     * sample description object is transfered to the sample table object (true by default).
     * If true, the sample table object will own the sample description object, and will
     * delete it when it is itself deleted. If false, the ownership remains with the caller,
     * and only a referencing pointer is kept, thus the caller must ensure that the object
     * is not deleted before the sample table is deleted.
     */
    virtual AP4_Result AddSampleDescription(AP4_SampleDescription* description,
                                            bool                   transfer_ownership = true);

    /**
     * Add a sample to the sample table, where the sample duration is given
     *
     * @param data_stream The byte stream that contains the sample data. The sample
     * object added to the track will keep a reference to that byte stream.
     * @param offset Position of the first byte of sample data within the stream
     * @param size Size in bytes of the sample data
     * @param duration Duration of the sample (in the timescale of the media). This
     * value can be 0 if the duration is not known. In that case, the next sample
     * added to the table MUST have a non-zero value for the DTS (decoding timestamp),
     * which will allow the actual duration of this sample to be computed.
     * @param description_index Index of the sample description that applies to
     * this sample (typically 0).
     * @param dts DTS (decoding timestamp) of the sample. If this value is 0, and there
     * already are samples in the table, the DTS of the sample will be automatically
     * computed based on the DTS and duration of the preceding sample. If this value is
     * not equal to the DTS+duration of the preceding sample, the duration of the
     * preceding sample is automatically adjusted, unless it has a non-zero value, in which
     * case AP4_ERROR_INVALID_PARAMETERS is returned.
     * The DTS of the first sample in the table MUST always be 0.
     * @param cts_delta Difference between the CTS (composition/display timestamp) and DTS
     * (decoding timestamp) of the sample (in the timescale of the media)
     * @param sync Boolean flag indicating whether this is a sync sample or not.
     */
    virtual AP4_Result AddSample(AP4_ByteStream& data_stream,
                                 AP4_Position    offset,
                                 AP4_Size        size,
                                 AP4_UI32        duration,
                                 AP4_Ordinal     description_index,
                                 AP4_UI64        dts,
                                 AP4_UI32        cts_delta,
                                 bool            sync);

private:
    // classes
    class SampleDescriptionHolder
    {
    public:
        SampleDescriptionHolder(AP4_SampleDescription* description, bool is_owned) :
            m_SampleDescription(description), m_IsOwned(is_owned) {}
        ~SampleDescriptionHolder()
        {
            if(m_IsOwned) delete m_SampleDescription;
        }
        AP4_SampleDescription* m_SampleDescription;
        bool                   m_IsOwned;
    };

    // members
    AP4_Array<AP4_Sample>             m_Samples;
    AP4_List<SampleDescriptionHolder> m_SampleDescriptions;
    AP4_Cardinal                      m_ChunkSize;
    AP4_Array<AP4_UI32>               m_SamplesInChunk;
    struct
    {
        AP4_Ordinal m_Sample;
        AP4_Ordinal m_Chunk;
    } m_LookupCache;
};

#endif // _AP4_SYNTHETIC_SAMPLE_TABLE_H_
