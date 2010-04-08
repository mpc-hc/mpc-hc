/*****************************************************************
|
|    AP4 - Linear Sample Reader
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
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
#include "Ap4LinearReader.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|   AP4_LinearReader::AP4_LinearReader
+---------------------------------------------------------------------*/
AP4_LinearReader::AP4_LinearReader(AP4_Movie& movie, AP4_Size max_buffer) :
    m_Movie(movie),
    m_BufferFullness(0),
    m_BufferFullnessPeak(0),
    m_MaxBufferFullness(max_buffer)
{
}

/*----------------------------------------------------------------------
|   AP4_LinearReader::~AP4_LinearReader
+---------------------------------------------------------------------*/
AP4_LinearReader::~AP4_LinearReader()
{
    for(unsigned int i = 0; i < m_Trackers.ItemCount(); i++)
    {
        delete m_Trackers[i];
    }
}

/*----------------------------------------------------------------------
|   AP4_LinearReader::EnableTrack
+---------------------------------------------------------------------*/
AP4_Result
AP4_LinearReader::EnableTrack(AP4_UI32 track_id)
{
    // check if we don't already have this
    if(FindTracker(track_id)) return AP4_SUCCESS;

    // find the track in the movie
    AP4_Track* track = m_Movie.GetTrack(track_id);
    if(track == NULL) return AP4_ERROR_NO_SUCH_ITEM;

    // create a new entry for the track
    return m_Trackers.Append(new Tracker(track));
}

/*----------------------------------------------------------------------
|   AP4_LinearReader::SetSampleIndex
+---------------------------------------------------------------------*/
AP4_Result
AP4_LinearReader::SetSampleIndex(AP4_UI32 track_id, AP4_UI32 sample_index)
{
    Tracker* tracker = FindTracker(track_id);
    if(tracker == NULL) return AP4_ERROR_INVALID_PARAMETERS;
    assert(tracker->m_Track);
    delete tracker->m_NextSample;
    tracker->m_NextSample = NULL;
    if(sample_index >= tracker->m_Track->GetSampleCount())
    {
        return AP4_ERROR_OUT_OF_RANGE;
    }
    tracker->m_Eos = false;
    tracker->m_NextSampleIndex = sample_index;

    // empty any queued samples
    for(AP4_List<SampleBuffer>::Item* item = tracker->m_Samples.FirstItem();
        item;
        item = item->GetNext())
    {
        SampleBuffer* buffer = item->GetData();
        m_BufferFullness -= buffer->m_Sample->GetSize();
        delete buffer;
    }
    tracker->m_Samples.Clear();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_LinearReader::Advance
+---------------------------------------------------------------------*/
AP4_Result
AP4_LinearReader::Advance()
{
    // first, check if we have space to advance
    if(m_BufferFullness >= m_MaxBufferFullness)
    {
        return AP4_ERROR_NOT_ENOUGH_SPACE;
    }

    AP4_UI64 min_offset = (AP4_UI64)(-1);
    Tracker* next_tracker = NULL;
    for(unsigned int i = 0; i < m_Trackers.ItemCount(); i++)
    {
        Tracker* tracker = m_Trackers[i];
        if(tracker->m_Eos) continue;

        // get the next sample unless we have it already
        if(tracker->m_NextSample == NULL)
        {
            if(tracker->m_NextSampleIndex >= tracker->m_Track->GetSampleCount())
            {
                tracker->m_Eos = true;
                continue;
            }
            tracker->m_NextSample = new AP4_Sample();
            AP4_Result result = tracker->m_Track->GetSample(tracker->m_NextSampleIndex, *tracker->m_NextSample);
            if(AP4_FAILED(result))
            {
                tracker->m_Eos = true;
                delete tracker->m_NextSample;
                tracker->m_NextSample = NULL;
                continue;
            }
        }
        assert(tracker->m_NextSample);

        AP4_UI64 offset = tracker->m_NextSample->GetOffset();
        if(offset < min_offset)
        {
            min_offset = offset;
            next_tracker = tracker;
        }
    }

    if(next_tracker)
    {
        // read the sample into a buffer
        assert(next_tracker->m_NextSample);
        SampleBuffer* buffer = new SampleBuffer(next_tracker->m_NextSample);
        AP4_Result result = buffer->m_Sample->ReadData(buffer->m_Data);
        if(AP4_FAILED(result)) return result;

        // add the buffer to the queue
        next_tracker->m_Samples.Add(buffer);
        m_BufferFullness += next_tracker->m_NextSample->GetSize();
        if(m_BufferFullness > m_BufferFullnessPeak)
        {
            m_BufferFullnessPeak = m_BufferFullness;
        }
        next_tracker->m_NextSample = NULL;
        next_tracker->m_NextSampleIndex++;
        return AP4_SUCCESS;
    }

    return AP4_ERROR_EOS;
}

/*----------------------------------------------------------------------
|   AP4_LinearReader::PopSample
+---------------------------------------------------------------------*/
bool
AP4_LinearReader::PopSample(Tracker*        tracker,
                            AP4_Sample&     sample,
                            AP4_DataBuffer& sample_data)
{
    SampleBuffer* head = NULL;
    if(AP4_SUCCEEDED(tracker->m_Samples.PopHead(head)))
    {
        assert(head->m_Sample);
        sample = *head->m_Sample;
        sample_data.SetData(head->m_Data.GetData(), head->m_Data.GetDataSize());
        assert(m_BufferFullness >= sample.GetSize());
        m_BufferFullness -= sample.GetSize();
        delete head;
        return true;
    }

    return false;
}

/*----------------------------------------------------------------------
|   AP4_LinearReader::ReadNextSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_LinearReader::ReadNextSample(AP4_UI32        track_id,
                                 AP4_Sample&     sample,
                                 AP4_DataBuffer& sample_data)
{
    if(m_Trackers.ItemCount() == 0)
    {
        return AP4_ERROR_NO_SUCH_ITEM;
    }

    // look for a sample from a specific track
    Tracker* tracker = FindTracker(track_id);
    if(tracker == NULL) return AP4_ERROR_INVALID_PARAMETERS;
    for(;;)
    {
        if(tracker->m_Eos) return AP4_ERROR_EOS;

        // pop a sample if we can
        if(PopSample(tracker, sample, sample_data)) return AP4_SUCCESS;

        AP4_Result result = Advance();
        if(AP4_FAILED(result)) return result;
    }

    return AP4_ERROR_EOS;
}

/*----------------------------------------------------------------------
|   AP4_LinearReader::ReadNextSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_LinearReader::ReadNextSample(AP4_Sample&     sample,
                                 AP4_DataBuffer& sample_data,
                                 AP4_UI32&       track_id)
{
    if(m_Trackers.ItemCount() == 0)
    {
        track_id = 0;
        return AP4_ERROR_NO_SUCH_ITEM;
    }

    // return the oldest buffered sample, if any
    AP4_UI64 min_offset = (AP4_UI64)(-1);
    Tracker* next_tracker = NULL;
    for(;;)
    {
        for(unsigned int i = 0; i < m_Trackers.ItemCount(); i++)
        {
            Tracker* tracker = m_Trackers[i];
            if(tracker->m_Eos) continue;

            AP4_List<SampleBuffer>::Item* item = tracker->m_Samples.FirstItem();
            if(item)
            {
                AP4_UI64 offset = item->GetData()->m_Sample->GetOffset();
                if(offset < min_offset)
                {
                    min_offset = offset;
                    next_tracker = tracker;
                }
            }
        }

        // return the sample if we have found a tracker
        if(next_tracker)
        {
            PopSample(next_tracker, sample, sample_data);
            track_id = next_tracker->m_Track->GetId();
            return AP4_SUCCESS;
        }

        // nothing found, read one more sample
        AP4_Result result = Advance();
        if(AP4_FAILED(result)) return result;
    }

    return AP4_ERROR_EOS;
}

/*----------------------------------------------------------------------
|   AP4_LinearReader::FindTracker
+---------------------------------------------------------------------*/
AP4_LinearReader::Tracker*
AP4_LinearReader::FindTracker(AP4_UI32 track_id)
{
    for(unsigned int i = 0; i < m_Trackers.ItemCount(); i++)
    {
        if(m_Trackers[i]->m_Track->GetId() == track_id) return m_Trackers[i];
    }

    // not found
    return NULL;
}

