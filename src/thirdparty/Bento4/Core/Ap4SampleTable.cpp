/*****************************************************************
|
|    AP4 - Sample Table Interface
|
|    Copyright 2003-2004 Gilles Boccon-Gibod & Julien Boeuf
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
#include "Ap4SampleTable.h"
#include "Ap4ContainerAtom.h"
#include "Ap4StsdAtom.h"
#include "Ap4StszAtom.h"
#include "Ap4StscAtom.h"
#include "Ap4StcoAtom.h"
#include "Ap4SttsAtom.h"

/*----------------------------------------------------------------------
|       AP4_SampleTable::GenerateStblAtom
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

    // start chunk table
    AP4_Cardinal        samples_in_chunk = 0;
    AP4_Offset          current_chunk_offset = 0;
    AP4_Size            current_chunk_size = 0;
    AP4_Array<AP4_UI32> chunk_offsets;

    // process all the samples
    AP4_Cardinal sample_count = GetSampleCount();
    for (AP4_Ordinal i=0; i<sample_count; i++) {
        AP4_Sample sample;
        GetSample(i, sample);
        
        // add an entry into the stsz atom
        stsz->AddEntry(sample.GetSize());
        
        // adjust the current chunk info
        current_chunk_size += sample.GetSize();

        // count the sample
        samples_in_chunk++;
        if (samples_in_chunk == 10) {
            // new chunk
            chunk_offsets.Append(current_chunk_offset);
            stsc->AddEntry(1, 10, 1);
            samples_in_chunk = 0;

            // adjust the chunk offset
            current_chunk_offset += current_chunk_size;
            current_chunk_size = 0;
        }
    }

    // process any unfinished chunk
    if (samples_in_chunk != 0) {
        // new chunk
        chunk_offsets.Append(current_chunk_offset);
        stsc->AddEntry(1, samples_in_chunk, 1);
    }

    // create the stco atom
    AP4_StcoAtom* stco = new AP4_StcoAtom(&chunk_offsets[0], 
                                          chunk_offsets.ItemCount());

    // create the stts atom (for now, we assume sample of equal duration)
    AP4_SttsAtom* stts = new AP4_SttsAtom();
    stts->AddEntry(sample_count, 1000); // FIXME

    // attach the children of stbl
    stbl->AddChild(stsd);
    stbl->AddChild(stsz);
    stbl->AddChild(stsc);
    stbl->AddChild(stco);
    stbl->AddChild(stts);

    return AP4_SUCCESS;
}
