/*****************************************************************
|
|    AP4 - Fragment Based Sample Tables
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
|
|
|    This atom is part of AP4 (MP4 Audio Processing Library).
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
|    along with Bento4|GPL; see the atom COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4FragmentSampleTable.h"
#include "Ap4ByteStream.h"
#include "Ap4Sample.h"
#include "Ap4TrexAtom.h"
#include "Ap4TfhdAtom.h"
#include "Ap4ContainerAtom.h"
#include "Ap4TrunAtom.h"
#include "Ap4MovieFragment.h"

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable::AP4_FragmentSampleTable
+---------------------------------------------------------------------*/
AP4_FragmentSampleTable::AP4_FragmentSampleTable(AP4_ContainerAtom* traf, 
                                                 AP4_TrexAtom*      trex,
                                                 AP4_ByteStream*    sample_stream,
                                                 AP4_Position       moof_offset,
                                                 AP4_Position       mdat_payload_offset,
                                                 AP4_UI64           dts_origin)
{
    AP4_TfhdAtom* tfhd = AP4_DYNAMIC_CAST(AP4_TfhdAtom, traf->GetChild(AP4_ATOM_TYPE_TFHD));
    if (tfhd == NULL) return;
    
    // count all the samples and reserve space for them
    unsigned int sample_count = 0;
    for (AP4_List<AP4_Atom>::Item* item = traf->GetChildren().FirstItem();
                                   item;
                                   item = item->GetNext()) {
        AP4_Atom* atom = item->GetData();
        if (atom->GetType() == AP4_ATOM_TYPE_TRUN) {
            AP4_TrunAtom* trun = AP4_DYNAMIC_CAST(AP4_TrunAtom, atom);
            sample_count += trun->GetEntries().ItemCount();
        }
    }    
    m_Samples.EnsureCapacity(sample_count);
    
    // process all the trun atoms
    for (AP4_List<AP4_Atom>::Item* item = traf->GetChildren().FirstItem();
                                   item;
                                   item = item->GetNext()) {
        AP4_Atom* atom = item->GetData();
        if (atom->GetType() == AP4_ATOM_TYPE_TRUN) {
            AP4_TrunAtom* trun = AP4_DYNAMIC_CAST(AP4_TrunAtom, atom);
            AP4_Result result = AddTrun(trun, 
                                        tfhd, 
                                        trex, 
                                        sample_stream, 
                                        moof_offset,
                                        mdat_payload_offset,
                                        dts_origin);
            if (AP4_FAILED(result)) return;
        }
    }    
}

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable::~AP4_FragmentSampleTable
+---------------------------------------------------------------------*/
AP4_FragmentSampleTable::~AP4_FragmentSampleTable()
{
}

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable::AddTrun
+---------------------------------------------------------------------*/
AP4_Result
AP4_FragmentSampleTable::AddTrun(AP4_TrunAtom*   trun, 
                                 AP4_TfhdAtom*   tfhd, 
                                 AP4_TrexAtom*   trex,
                                 AP4_ByteStream* sample_stream,
                                 AP4_Position    moof_offset,
                                 AP4_Position&   payload_offset,
                                 AP4_UI64&       dts_origin)
{
    AP4_Flags tfhd_flags = tfhd->GetFlags();
    AP4_Flags trun_flags = trun->GetFlags();
    
    // update the number of samples
    unsigned int start = m_Samples.ItemCount();
    m_Samples.SetItemCount(start + trun->GetEntries().ItemCount());
        
    // base data offset
    AP4_Position data_offset = 0;
    if (tfhd_flags & AP4_TFHD_FLAG_BASE_DATA_OFFSET_PRESENT) {
        data_offset = tfhd->GetBaseDataOffset();
    } else {
        data_offset = moof_offset;
    }
    if (trun_flags & AP4_TRUN_FLAG_DATA_OFFSET_PRESENT) {
        data_offset += trun->GetDataOffset();
    }         
    // MS hack
    if (data_offset == moof_offset) {
        data_offset = payload_offset;
    } else {
        payload_offset = data_offset;
    }
        
    // sample description index
    AP4_UI32 sample_description_index = 0;
    if (tfhd_flags & AP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX_PRESENT) {
        sample_description_index = tfhd->GetSampleDescriptionIndex();
    } else if (trex) {
        sample_description_index = trex->GetDefaultSampleDescriptionIndex();
    }        
       
    // default sample size
    AP4_UI32 default_sample_size = 0;
    if (tfhd_flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_SIZE_PRESENT) {
        default_sample_size = tfhd->GetDefaultSampleSize();
    } else if (trex) {
        default_sample_size = trex->GetDefaultSampleSize();
    }
    
    // default sample duration
    AP4_UI32 default_sample_duration = 0;
    if (tfhd_flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_DURATION_PRESENT) {
        default_sample_duration = tfhd->GetDefaultSampleDuration();
    } else if (trex) {
        default_sample_duration = trex->GetDefaultSampleDuration();
    }
    
    // default sample flags
    AP4_UI32 default_sample_flags = 0;
    if (tfhd_flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_FLAGS_PRESENT) {
        default_sample_flags = tfhd->GetDefaultSampleFlags();
    } else if (trex) {
        default_sample_flags = trex->GetDefaultSampleFlags();
    }

    // parse all trun entries to setup the samples
    AP4_UI64 dts = dts_origin;
    for (unsigned int i=0; i<trun->GetEntries().ItemCount(); i++) {
        const AP4_TrunAtom::Entry& entry  = trun->GetEntries()[i];
        AP4_Sample&                sample = m_Samples[start+i];
        
        // sample size
        if (trun_flags & AP4_TRUN_FLAG_SAMPLE_SIZE_PRESENT) {
            sample.SetSize(entry.sample_size);
        } else {
            sample.SetSize(default_sample_size);
        }
        payload_offset += sample.GetSize(); // update the payload offset
        
        // sample duration
        if (trun_flags & AP4_TRUN_FLAG_SAMPLE_DURATION_PRESENT) {
            sample.SetDuration(entry.sample_duration);
        } else {
            sample.SetDuration(default_sample_duration);
        }

        // sample flags
        AP4_UI32 sample_flags = default_sample_flags;
        if (i==0 && (trun_flags & AP4_TRUN_FLAG_FIRST_SAMPLE_FLAGS_PRESENT)) {
            sample_flags = trun->GetFirstSampleFlags();
        } else if (trun_flags & AP4_TRUN_FLAG_SAMPLE_FLAGS_PRESENT) {
            sample_flags = entry.sample_flags;
        }
        if ((sample_flags & AP4_FRAG_FLAG_SAMPLE_IS_DIFFERENCE) == 0) {
            sample.SetSync(true);
        } else {
            sample.SetSync(false);
        }
        
        // sample description index
        sample.SetDescriptionIndex(sample_description_index);
        
        // data stream
        if (sample_stream) sample.SetDataStream(*sample_stream);
        
        // data offset
        sample.SetOffset(data_offset);
        data_offset += sample.GetSize();
        
        // dts and cts
        sample.SetDts(dts);
        if (trun_flags & AP4_TRUN_FLAG_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT) {
            sample.SetCts(dts+entry.sample_composition_time_offset);
        } else {
            sample.SetCts(dts);
        }
        
        // update the counters
        dts        += sample.GetDuration();
        m_Duration += sample.GetDuration();
    }
    
    // update the dts
    dts_origin = dts;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable::GetSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_FragmentSampleTable::GetSample(AP4_Ordinal index, 
                                   AP4_Sample& sample)
{
    if (index >= m_Samples.ItemCount()) return AP4_ERROR_OUT_OF_RANGE;

    // copy the sample
    sample = m_Samples[index];

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable::GetSampleCount
+---------------------------------------------------------------------*/
AP4_Cardinal
AP4_FragmentSampleTable::GetSampleCount()
{
    return m_Samples.ItemCount();
}

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable::GetSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription*
AP4_FragmentSampleTable::GetSampleDescription(AP4_Ordinal /*index*/)
{
    return NULL; // FIXME
}

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable::GetSampleDescriptionCount
+---------------------------------------------------------------------*/
AP4_Cardinal
AP4_FragmentSampleTable::GetSampleDescriptionCount()
{
    return 1; // FIXME
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleChunkPosition
+---------------------------------------------------------------------*/
AP4_Result   
AP4_FragmentSampleTable::GetSampleChunkPosition(AP4_Ordinal  sample_index, 
                                                AP4_Ordinal& chunk_index,
                                                AP4_Ordinal& position_in_chunk)
{
    chunk_index       = 0;
    position_in_chunk = sample_index;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable::GetSampleIndexForTimeStamp
+---------------------------------------------------------------------*/
AP4_Result 
AP4_FragmentSampleTable::GetSampleIndexForTimeStamp(AP4_UI64     /*ts*/, 
                                                    AP4_Ordinal& sample_index)
{
    sample_index = 0; // FIXME
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable::GetNearestSyncSampleIndex
+---------------------------------------------------------------------*/
AP4_Ordinal  
AP4_FragmentSampleTable::GetNearestSyncSampleIndex(AP4_Ordinal /*sample_index*/, bool /*before*/)
{
    return 0; // FIXME
}

