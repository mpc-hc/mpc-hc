/*****************************************************************
|
|    AP4 - Sample Table Interface
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
#include "Ap4SampleTable.h"
#include "Ap4ContainerAtom.h"
#include "Ap4StsdAtom.h"
#include "Ap4StszAtom.h"
#include "Ap4StscAtom.h"
#include "Ap4StcoAtom.h"
#include "Ap4Co64Atom.h"
#include "Ap4SttsAtom.h"
#include "Ap4StssAtom.h"
#include "Ap4CttsAtom.h"
#include "Ap4Sample.h"

/*----------------------------------------------------------------------
|   AP4_SampleTable::GenerateStblAtom
+---------------------------------------------------------------------*/
AP4_Result 
AP4_SampleTable::GenerateStblAtom(AP4_ContainerAtom*& stbl)
{
    // create the stbl container
    stbl = new AP4_ContainerAtom(AP4_ATOM_TYPE_STBL);

    // create the stsd atom
    AP4_StsdAtom* stsd = new AP4_StsdAtom(this);

    // create the stsz atom
    AP4_StszAtom* stsz = new AP4_StszAtom();

    // create the stsc atom
    AP4_StscAtom* stsc = new AP4_StscAtom();

    // create the stts atom
    AP4_SttsAtom* stts = new AP4_SttsAtom();

    // create the stss atom
    AP4_StssAtom* stss = new AP4_StssAtom();
    
    // declare the ctts atom (may be created later)
    AP4_CttsAtom* ctts = NULL;
    
    // start chunk table
    AP4_Ordinal             current_chunk_index              = 0;
    AP4_Size                current_chunk_size               = 0;
    AP4_Position            current_chunk_offset             = 0;
    AP4_Cardinal            current_samples_in_chunk         = 0;
    AP4_Ordinal             current_sample_description_index = 0;
    AP4_UI32                current_duration                 = 0;
    AP4_Cardinal            current_duration_run             = 0;
    AP4_UI32                current_cts_delta                = 0;
    AP4_Cardinal            current_cts_delta_run            = 0;
    AP4_Array<AP4_Position> chunk_offsets;

    // process all the samples
    bool         all_samples_are_sync = false;
    AP4_Cardinal sample_count = GetSampleCount();
    for (AP4_Ordinal i=0; i<sample_count; i++) {
        AP4_Sample sample;
        GetSample(i, sample);
        
        // update DTS table
        AP4_UI32 new_duration = sample.GetDuration();
        if (new_duration != current_duration && current_duration_run != 0) {
            // emit a new stts entry
            stts->AddEntry(current_duration_run, current_duration);
            
            // reset the run count
            current_duration_run = 0;
        } 
        ++current_duration_run;
        current_duration = new_duration;
        
        // update CTS table
        AP4_UI32 new_cts_delta = sample.GetCtsDelta();
        if (new_cts_delta != current_cts_delta && current_cts_delta_run != 0) {
            // create a ctts atom if we don't have one
            if (ctts == NULL) ctts = new AP4_CttsAtom();
            
            //emit a new ctts entry
            ctts->AddEntry(current_cts_delta_run, current_cts_delta);
            
            // reset the run count
            current_cts_delta_run = 0;
        }
        ++current_cts_delta_run;
        current_cts_delta = new_cts_delta;
        
        // add an entry into the stsz atom
        stsz->AddEntry(sample.GetSize());
        
        // update the sync sample table
        if (sample.IsSync()) {
            stss->AddEntry(i+1);
            if (i==0) all_samples_are_sync = true;
        } else {
            all_samples_are_sync = false;
        }
        
        // see in which chunk this sample is
        AP4_Ordinal chunk_index = 0;
        AP4_Ordinal position_in_chunk = 0;
        AP4_Result  result = GetSampleChunkPosition(i, chunk_index, position_in_chunk);
        if (AP4_SUCCEEDED(result)) {
            if (chunk_index != current_chunk_index && current_samples_in_chunk != 0) {
                // new chunk
                chunk_offsets.Append(current_chunk_offset);
                current_chunk_offset += current_chunk_size;

                stsc->AddEntry(1, 
                               current_samples_in_chunk,
                               current_sample_description_index+1);

                current_samples_in_chunk = 0;
                current_chunk_size       = 0;
            }
            current_chunk_index = chunk_index;
        }

        // store the sample description index
        current_sample_description_index = sample.GetDescriptionIndex();
                
        // adjust the current chunk info
        current_chunk_size += sample.GetSize();
        ++current_samples_in_chunk;        
    }

    // finish the stts table
    stts->AddEntry(current_duration_run, current_duration);

    // finish the ctts table if we have one
    if (ctts) {
        AP4_ASSERT(current_cts_delta_run != 0);
        
        // add a ctts entry
        ctts->AddEntry(current_cts_delta_run, current_cts_delta);
    } 
    
    // process any unfinished chunk
    if (current_samples_in_chunk != 0) {
        // new chunk
        chunk_offsets.Append(current_chunk_offset);
        stsc->AddEntry(1, 
                       current_samples_in_chunk,
                       current_sample_description_index+1);
    }

    // attach the children of stbl
    stbl->AddChild(stsd);
    stbl->AddChild(stsz);
    stbl->AddChild(stsc);
    stbl->AddChild(stts);
    if (ctts) stbl->AddChild(ctts);
    if (!all_samples_are_sync && stss->GetEntries().ItemCount() != 0) {
        stbl->AddChild(stss);
    } else {
        delete stss;
    }
    
    // see if we need a co64 or an stco atom
    AP4_Size  chunk_count = chunk_offsets.ItemCount();
    if (current_chunk_offset <= 0xFFFFFFFF) {
        // make an array of 32-bit entries
        AP4_UI32* chunk_offsets_32 = new AP4_UI32[chunk_count];
        for (unsigned int i=0; i<chunk_count; i++) {
            chunk_offsets_32[i] = (AP4_UI32)chunk_offsets[i];
        }
        // create the stco atom
        AP4_StcoAtom* stco = new AP4_StcoAtom(&chunk_offsets_32[0], chunk_count);
        stbl->AddChild(stco);

        delete[] chunk_offsets_32;
    } else {
        // create the co64 atom
        AP4_Co64Atom* co64 = new AP4_Co64Atom(&chunk_offsets[0], chunk_count);
        stbl->AddChild(co64);
    }


    return AP4_SUCCESS;
}
