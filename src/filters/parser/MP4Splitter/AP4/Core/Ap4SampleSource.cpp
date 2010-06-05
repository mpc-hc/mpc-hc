/*****************************************************************
|
|    AP4 - Sample Source Interface
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
#include "Ap4SampleSource.h"
#include "Ap4Track.h"
#include "Ap4SampleDescription.h"
#include "Ap4DataBuffer.h"

/*----------------------------------------------------------------------
|   AP4_TrackSampleSource
+---------------------------------------------------------------------*/
AP4_TrackSampleSource::AP4_TrackSampleSource(AP4_Track* track) :
    m_Track(track),
    m_SampleIndex(0)
{
}
    
/*----------------------------------------------------------------------
|   AP4_TrackSampleSource
+---------------------------------------------------------------------*/
AP4_UI32
AP4_TrackSampleSource::GetTimeScale()
{
    return m_Track->GetMediaTimeScale();
}

/*----------------------------------------------------------------------
|   AP4_TrackSampleSource::GetDurationMs
+---------------------------------------------------------------------*/
AP4_UI32    
AP4_TrackSampleSource::GetDurationMs()
{
    return m_Track->GetDurationMs();
}

/*----------------------------------------------------------------------
|   AP4_TrackSampleSource::GetTrackId
+---------------------------------------------------------------------*/
AP4_UI32    
AP4_TrackSampleSource::GetTrackId()
{
    return m_Track->GetId();
}

/*----------------------------------------------------------------------
|   AP4_TrackSampleSource::ReadNextSample
+---------------------------------------------------------------------*/
AP4_Result  
AP4_TrackSampleSource::ReadNextSample(AP4_Sample& sample, AP4_DataBuffer& buffer)
{
    AP4_Result result = m_Track->ReadSample(m_SampleIndex, sample, buffer);
    if (AP4_SUCCEEDED(result)) ++m_SampleIndex;
    return result;
}

/*----------------------------------------------------------------------
|   AP4_TrackSampleSource::GetNearestSyncSampleIndex
+---------------------------------------------------------------------*/
AP4_Ordinal 
AP4_TrackSampleSource::GetNearestSyncSampleIndex(AP4_Ordinal indx, bool before)
{
    return m_Track->GetNearestSyncSampleIndex(indx, before);
}

/*----------------------------------------------------------------------
|   AP4_TrackSampleSource::GetSampleIndexForTimeStampMs
+---------------------------------------------------------------------*/
AP4_Result  
AP4_TrackSampleSource::GetSampleIndexForTimeStampMs(AP4_UI32 timestamp, AP4_Ordinal& indx)
{
    return m_Track->GetSampleIndexForTimeStampMs(timestamp, indx);
}

/*----------------------------------------------------------------------
|   AP4_TrackSampleSource::SetSampleIndex
+---------------------------------------------------------------------*/
AP4_Result  
AP4_TrackSampleSource::SetSampleIndex(AP4_Ordinal indx)
{
    if (indx >= m_Track->GetSampleCount()) return AP4_ERROR_OUT_OF_RANGE;
    m_SampleIndex = indx;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_TrackSampleSource::GetSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription* 
AP4_TrackSampleSource::GetSampleDescription(AP4_Ordinal indx)
{
    return m_Track->GetSampleDescription(indx);
}
    
