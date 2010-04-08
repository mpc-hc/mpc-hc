/*****************************************************************
|
|    AP4 - File Processor
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
#include "Ap4Processor.h"
#include "Ap4AtomSampleTable.h"
#include "Ap4MovieFragment.h"
#include "Ap4FragmentSampleTable.h"
#include "Ap4TfhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Movie.h"
#include "Ap4Array.h"
#include "Ap4Sample.h"
#include "Ap4TrakAtom.h"
#include "Ap4TfraAtom.h"
#include "Ap4DataBuffer.h"
#include "Ap4Debug.h"

/*----------------------------------------------------------------------
|   types
+---------------------------------------------------------------------*/
struct AP4_SampleLocator
{
    AP4_SampleLocator() :
        m_TrakIndex(0),
        m_SampleTable(NULL),
        m_SampleIndex(0),
        m_ChunkIndex(0) {}
    AP4_Ordinal          m_TrakIndex;
    AP4_AtomSampleTable* m_SampleTable;
    AP4_Ordinal          m_SampleIndex;
    AP4_Ordinal          m_ChunkIndex;
    AP4_Sample           m_Sample;
};

struct AP4_SampleCursor
{
    AP4_SampleCursor() : m_EndReached(false) {}
    AP4_SampleLocator m_Locator;
    bool              m_EndReached;
};

struct AP4_MoofLocator
{
    AP4_MoofLocator(AP4_ContainerAtom* moof, AP4_UI64 offset) :
        m_Moof(moof),
        m_Offset(offset) {}
    AP4_ContainerAtom* m_Moof;
    AP4_UI64           m_Offset;
};

/*----------------------------------------------------------------------
|   AP4_Processor::ProcessFragments
+---------------------------------------------------------------------*/
AP4_Result
AP4_Processor::ProcessFragments(AP4_MoovAtom*              moov,
                                AP4_List<AP4_MoofLocator>& moofs,
                                AP4_ContainerAtom*         mfra,
                                AP4_ByteStream&            input,
                                AP4_ByteStream&            output)
{
    // FIXME: this only works for non-changing moofs

    for(AP4_List<AP4_MoofLocator>::Item* item = moofs.FirstItem();
        item;
        item = item->GetNext())
    {
        AP4_MoofLocator*   locator     = item->GetData();
        AP4_ContainerAtom* moof        = locator->m_Moof;
        AP4_UI64           moof_offset = locator->m_Offset;
        AP4_UI64           mdat_payload_offset = moof_offset + moof->GetSize() + 8;
        AP4_MovieFragment* fragment    = new AP4_MovieFragment(moof);
        AP4_Sample         sample;
        AP4_DataBuffer     sample_data_in;
        AP4_DataBuffer     sample_data_out;
        AP4_Result         result;

        // process all the traf atoms
        AP4_Array<AP4_Processor::FragmentHandler*> handlers;
        for(; AP4_Atom* atom = moof->GetChild(AP4_ATOM_TYPE_TRAF, handlers.ItemCount());)
        {
            AP4_ContainerAtom* traf = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);
            AP4_Processor::FragmentHandler* handler = CreateFragmentHandler(traf);
            if(handler) result = handler->ProcessFragment();
            handlers.Append(handler);
        }

        // write the moof
        AP4_UI64 moof_out_start = 0;
        output.Tell(moof_out_start);
        bool moof_has_changed = false;
        moof->Write(output);

        // process all track runs
        for(unsigned int i = 0; i < handlers.ItemCount(); i++)
        {
            AP4_FragmentSampleTable* sample_table = NULL;
            AP4_Processor::FragmentHandler* handler = handlers[i];

            // get the track ID
            AP4_ContainerAtom* traf = AP4_DYNAMIC_CAST(AP4_ContainerAtom, moof->GetChild(AP4_ATOM_TYPE_TRAF, i));
            AP4_TfhdAtom* tfhd      = AP4_DYNAMIC_CAST(AP4_TfhdAtom, traf->GetChild(AP4_ATOM_TYPE_TFHD, i));

            // create a sample table object so we can read the sample data
            result = fragment->CreateSampleTable(moov, tfhd->GetTrackId(), &input, moof_offset, mdat_payload_offset, sample_table);
            if(AP4_FAILED(result)) return result;

            // compute the mdat size
            AP4_UI64 mdat_size = 0;
            for(unsigned int j = 0; j < sample_table->GetSampleCount(); j++)
            {
                result = sample_table->GetSample(j, sample);
                if(AP4_FAILED(result)) return result;
                mdat_size += sample.GetSize();
            }

            // write an mdat header
            if(mdat_size > 0xFFFFFFFF - 8)
            {
                // we don't support large mdat fragments
                return AP4_ERROR_OUT_OF_RANGE;
            }
            if(mdat_size)
            {
                output.WriteUI32((AP4_UI32)(8 + mdat_size));
                output.WriteUI32(AP4_ATOM_TYPE_MDAT);
            }

#if defined(AP4_DEBUG)
            AP4_Position before;
            output.Tell(before);
#endif

            // write the mdat
            for(unsigned int j = 0; j < sample_table->GetSampleCount(); j++)
            {
                result = sample_table->GetSample(j, sample);
                if(AP4_FAILED(result)) return result;
                sample.ReadData(sample_data_in);

                // process the sample data
                if(handler)
                {
                    result = handler->ProcessSample(sample_data_in, sample_data_out);
                    if(AP4_FAILED(result)) return result;

                    // write the sample data
                    result = output.Write(sample_data_out.GetData(), sample_data_out.GetDataSize());
                    if(AP4_FAILED(result)) return result;

                    // give the handler a chance to update the atoms
                    result = handler->FinishFragment();
                    if(AP4_SUCCEEDED(result)) moof_has_changed = true;
                }
                else
                {
                    // write the sample data (unmodified)
                    result = output.Write(sample_data_in.GetData(), sample_data_in.GetDataSize());
                    if(AP4_FAILED(result)) return result;
                }
            }

#if defined(AP4_DEBUG)
            AP4_Position after;
            output.Tell(after);
            AP4_ASSERT(after - before == mdat_size);
#endif
            delete sample_table;
        }

        // update the moof if needed
        AP4_UI64 mdat_out_end = 0;
        output.Tell(mdat_out_end);
        if(moof_has_changed)
        {
            output.Seek(moof_out_start);
            moof->Write(output);
            output.Seek(mdat_out_end);
        }

        // update the mfra if we have one
        if(mfra)
        {
            for(AP4_List<AP4_Atom>::Item* mfra_item = mfra->GetChildren().FirstItem();
                mfra_item;
                mfra_item = mfra_item->GetNext())
            {
                if(mfra_item->GetData()->GetType() != AP4_ATOM_TYPE_TFRA) continue;
                AP4_TfraAtom* tfra = AP4_DYNAMIC_CAST(AP4_TfraAtom, mfra_item->GetData());
                if(tfra == NULL) continue;
                AP4_Array<AP4_TfraAtom::Entry>& entries     = tfra->GetEntries();
                AP4_Cardinal                    entry_count = entries.ItemCount();
                for(unsigned int i = 0; i < entry_count; i++)
                {
                    if(entries[i].m_MoofOffset == locator->m_Offset)
                    {
                        entries[i].m_MoofOffset = moof_out_start;
                    }
                }
            }
        }

        delete fragment;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Processor::Process
+---------------------------------------------------------------------*/
AP4_Result
AP4_Processor::Process(AP4_ByteStream&   input,
                       AP4_ByteStream&   output,
                       ProgressListener* listener,
                       AP4_AtomFactory&  atom_factory)
{
    // read all atoms.
    // keep all atoms except [mdat]
    // keep a ref to [moov]
    // put [moof] atoms in a separate list
    AP4_AtomParent              top_level;
    AP4_MoovAtom*               moov = NULL;
    AP4_ContainerAtom*          mfra = NULL;
    AP4_List<AP4_MoofLocator>   moofs;
    AP4_UI64                    stream_offset = 0;
    for(AP4_Atom* atom = NULL;
        AP4_SUCCEEDED(atom_factory.CreateAtomFromStream(input, atom));
        input.Tell(stream_offset))
    {
        if(atom->GetType() == AP4_ATOM_TYPE_MDAT)
        {
            continue;
        }
        else if(atom->GetType() == AP4_ATOM_TYPE_MOOV)
        {
            moov = (AP4_MoovAtom*)atom;
        }
        else if(atom->GetType() == AP4_ATOM_TYPE_MOOF)
        {
            AP4_ContainerAtom* moof = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);
            if(moof)
            {
                moofs.Add(new AP4_MoofLocator(moof, stream_offset));
            }
            continue;
        }
        else if(atom->GetType() == AP4_ATOM_TYPE_MFRA)
        {
            mfra = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);
            continue;
        }
        top_level.AddChild(atom);
    }

    // initialize the processor
    AP4_Result result = Initialize(top_level, input);
    if(AP4_FAILED(result)) return result;

    // process the tracks if we have a moov atom
    AP4_Array<AP4_SampleLocator> locators;
    AP4_Cardinal                 track_count       = 0;
    AP4_List<AP4_TrakAtom>*      trak_atoms        = NULL;
    AP4_LargeSize                mdat_payload_size = 0;
    TrackHandler**               handlers          = NULL;
    AP4_SampleCursor*            cursors           = NULL;
    if(moov)
    {
        // build an array of track sample locators
        trak_atoms = &moov->GetTrakAtoms();
        track_count = trak_atoms->ItemCount();
        cursors = new AP4_SampleCursor[track_count];
        handlers = new TrackHandler*[track_count];
        for(AP4_Ordinal i = 0; i < track_count; i++)
        {
            handlers[i] = NULL;
        }

        unsigned int index = 0;
        for(AP4_List<AP4_TrakAtom>::Item* item = trak_atoms->FirstItem(); item; item = item->GetNext())
        {
            AP4_TrakAtom* trak = item->GetData();

            // find the stsd atom
            AP4_ContainerAtom* stbl = AP4_DYNAMIC_CAST(AP4_ContainerAtom, trak->FindChild("mdia/minf/stbl"));
            if(stbl == NULL) continue;

            // see if there's an external data source for this track
            AP4_ByteStream* trak_data_stream = &input;
            for(AP4_List<ExternalTrackData>::Item* ditem = m_ExternalTrackData.FirstItem(); ditem; ditem = ditem->GetNext())
            {
                ExternalTrackData* tdata = ditem->GetData();
                if(tdata->m_TrackId == trak->GetId())
                {
                    trak_data_stream = tdata->m_MediaData;
                    break;
                }
            }

            // create the track handler
            handlers[index] = CreateTrackHandler(trak);
            cursors[index].m_Locator.m_TrakIndex   = index;
            cursors[index].m_Locator.m_SampleTable = new AP4_AtomSampleTable(stbl, *trak_data_stream);
            cursors[index].m_Locator.m_SampleIndex = 0;
            cursors[index].m_Locator.m_ChunkIndex  = 0;
            if(cursors[index].m_Locator.m_SampleTable->GetSampleCount())
            {
                cursors[index].m_Locator.m_SampleTable->GetSample(0, cursors[index].m_Locator.m_Sample);
            }
            else
            {
                cursors[index].m_EndReached = true;
            }

            index++;
        }

        // figure out the layout of the chunks
        for(;;)
        {
            // see which is the next sample to write
            AP4_UI64 min_offset = (AP4_UI64)(-1);
            int cursor = -1;
            for(unsigned int i = 0; i < track_count; i++)
            {
                if(!cursors[i].m_EndReached &&
                   cursors[i].m_Locator.m_Sample.GetOffset() <= min_offset)
                {
                    min_offset = cursors[i].m_Locator.m_Sample.GetOffset();
                    cursor = i;
                }
            }

            // stop if all cursors are exhausted
            if(cursor == -1) break;

            // append this locator to the layout list
            AP4_SampleLocator& locator = cursors[cursor].m_Locator;
            locators.Append(locator);

            // move the cursor to the next sample
            locator.m_SampleIndex++;
            if(locator.m_SampleIndex == locator.m_SampleTable->GetSampleCount())
            {
                // mark this track as completed
                cursors[cursor].m_EndReached = true;
            }
            else
            {
                // get the next sample info
                locator.m_SampleTable->GetSample(locator.m_SampleIndex, locator.m_Sample);
                AP4_Ordinal skip, sdesc;
                locator.m_SampleTable->GetChunkForSample(locator.m_SampleIndex,
                        locator.m_ChunkIndex,
                        skip, sdesc);
            }
        }

        // update the stbl atoms and compute the mdat size
        int current_track = -1;
        int current_chunk = -1;
        AP4_Position current_chunk_offset = 0;
        AP4_Size current_chunk_size = 0;
        for(AP4_Ordinal i = 0; i < locators.ItemCount(); i++)
        {
            AP4_SampleLocator& locator = locators[i];
            if((int)locator.m_TrakIndex  != current_track ||
               (int)locator.m_ChunkIndex != current_chunk)
            {
                // start a new chunk for this track
                current_chunk_offset += current_chunk_size;
                current_chunk_size = 0;
                current_track = locator.m_TrakIndex;
                current_chunk = locator.m_ChunkIndex;
                locator.m_SampleTable->SetChunkOffset(locator.m_ChunkIndex, current_chunk_offset);
            }
            AP4_Size sample_size;
            TrackHandler* handler = handlers[locator.m_TrakIndex];
            if(handler)
            {
                sample_size = handler->GetProcessedSampleSize(locator.m_Sample);
                locator.m_SampleTable->SetSampleSize(locator.m_SampleIndex, sample_size);
            }
            else
            {
                sample_size = locator.m_Sample.GetSize();
            }
            current_chunk_size += sample_size;
            mdat_payload_size  += sample_size;
        }

        // process the tracks (ex: sample descriptions processing)
        for(AP4_Ordinal i = 0; i < track_count; i++)
        {
            TrackHandler* handler = handlers[i];
            if(handler) handler->ProcessTrack();
        }
    }

    // finalize the processor
    Finalize(top_level);

    // calculate the size of all atoms combined
    AP4_UI64 atoms_size = 0;
    top_level.GetChildren().Apply(AP4_AtomSizeAdder(atoms_size));

    // see if we need a 64-bit or 32-bit mdat
    AP4_Size mdat_header_size = AP4_ATOM_HEADER_SIZE;
    if(mdat_payload_size + mdat_header_size > 0xFFFFFFFF)
    {
        // we need a 64-bit size
        mdat_header_size += 8;
    }

    // adjust the chunk offsets
    for(AP4_Ordinal i = 0; i < track_count; i++)
    {
        AP4_TrakAtom* trak;
        trak_atoms->Get(i, trak);
        trak->AdjustChunkOffsets(atoms_size + mdat_header_size);
    }

    // write all atoms
    top_level.GetChildren().Apply(AP4_AtomListWriter(output));

    // write mdat header
    if(mdat_payload_size)
    {
        if(mdat_header_size == AP4_ATOM_HEADER_SIZE)
        {
            // 32-bit size
            output.WriteUI32((AP4_UI32)(mdat_header_size + mdat_payload_size));
            output.WriteUI32(AP4_ATOM_TYPE_MDAT);
        }
        else
        {
            // 64-bit size
            output.WriteUI32(1);
            output.WriteUI32(AP4_ATOM_TYPE_MDAT);
            output.WriteUI64(mdat_header_size + mdat_payload_size);
        }
    }

#if defined(AP4_DEBUG)
    AP4_Position before;
    output.Tell(before);
#endif

    // write the samples
    if(moov)
    {
        AP4_Sample     sample;
        AP4_DataBuffer data_in;
        AP4_DataBuffer data_out;
        for(unsigned int i = 0; i < locators.ItemCount(); i++)
        {
            AP4_SampleLocator& locator = locators[i];
            locator.m_Sample.ReadData(data_in);
            TrackHandler* handler = handlers[locator.m_TrakIndex];
            if(handler)
            {
                result = handler->ProcessSample(data_in, data_out);
                if(AP4_FAILED(result)) return result;
                output.Write(data_out.GetData(), data_out.GetDataSize());
            }
            else
            {
                output.Write(data_in.GetData(), data_in.GetDataSize());
            }

            // notify the progress listener
            if(listener)
            {
                listener->OnProgress(i + 1, locators.ItemCount());
            }
        }

        // cleanup
        for(AP4_Ordinal i = 0; i < track_count; i++)
        {
            delete cursors[i].m_Locator.m_SampleTable;
            delete handlers[i];
        }
        delete[] cursors;
        delete[] handlers;
    }

#if defined(AP4_DEBUG)
    AP4_Position after;
    output.Tell(after);
    AP4_ASSERT(after - before == mdat_payload_size);
#endif

    // process the fragments, if any
    result = ProcessFragments(moov, moofs, mfra, input, output);
    if(AP4_FAILED(result)) return result;

    // write the mfra atom at the end if we have one
    if(mfra)
    {
        mfra->Write(output);
    }

    // cleanup
    moofs.DeleteReferences();
    delete mfra;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Processor:Initialize
+---------------------------------------------------------------------*/
AP4_Result
AP4_Processor::Initialize(AP4_AtomParent&   /* top_level */,
                          AP4_ByteStream&   /* stream    */,
                          ProgressListener* /* listener  */)
{
    // default implementation: do nothing
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Processor:Finalize
+---------------------------------------------------------------------*/
AP4_Result
AP4_Processor::Finalize(AP4_AtomParent&   /* top_level */,
                        ProgressListener* /* listener */)
{
    // default implementation: do nothing
    return AP4_SUCCESS;
}
