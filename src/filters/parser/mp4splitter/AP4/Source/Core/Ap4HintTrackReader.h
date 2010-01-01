/*****************************************************************
|
|    AP4 - Hint Track Reader
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

#ifndef _AP4_HINT_TRACK_READER_H_
#define _AP4_HINT_TRACK_READER_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Sample.h"

/*----------------------------------------------------------------------
|   class declarations
+---------------------------------------------------------------------*/
class AP4_DataBuffer;
class AP4_Movie;
class AP4_Track;
class AP4_RtpSampleData;
class AP4_RtpPacket;
class AP4_ImmediateRtpConstructor;
class AP4_SampleRtpConstructor;
class AP4_String;

/*----------------------------------------------------------------------
|   AP4_HintTrackReader
+---------------------------------------------------------------------*/
class AP4_HintTrackReader
{
public:
    // constructor and destructor
    static AP4_Result Create(AP4_Track&            hint_track, 
                             AP4_Movie&            movie, 
                             AP4_UI32              ssrc, // if 0, a random value is chosen
                             AP4_HintTrackReader*& reader);
    ~AP4_HintTrackReader();

    // methods
    AP4_Result      GetNextPacket(AP4_DataBuffer& packet, 
                                  AP4_UI32&       ts_ms);
    AP4_Result      SeekToTimeStampMs(AP4_UI32  desired_ts_ms,
                                      AP4_UI32& actual_ts_ms);
    AP4_UI32        GetCurrentTimeStampMs();
    AP4_Result      Rewind();
    AP4_Result      GetSdpText(AP4_String& sdp);
    AP4_Track*      GetMediaTrack() { return m_MediaTrack; }
    
private:
    // use the factory instead of the constructor
    AP4_HintTrackReader(AP4_Track& hint_track, 
                        AP4_Movie& movie, 
                        AP4_UI32   ssrc);
    
    // methods
    AP4_Result GetRtpSample(AP4_Ordinal index);
    AP4_Result BuildRtpPacket(AP4_RtpPacket*  packet, 
                              AP4_DataBuffer& packet_data);
    AP4_Result WriteImmediateRtpData(AP4_ImmediateRtpConstructor* constructor,
                                     AP4_ByteStream*              data_stream);
    AP4_Result WriteSampleRtpData(AP4_SampleRtpConstructor* constructor,
                                  AP4_ByteStream*           data_stream);

    // members
    AP4_Track&          m_HintTrack;
    AP4_Track*          m_MediaTrack;
    AP4_UI32            m_MediaTimeScale;
    AP4_Sample          m_CurrentHintSample;
    AP4_RtpSampleData*  m_RtpSampleData;
    AP4_UI32            m_Ssrc;
    AP4_Ordinal         m_SampleIndex;
    AP4_Ordinal         m_PacketIndex;
    AP4_UI16            m_RtpSequenceStart;
    AP4_UI32            m_RtpTimeStampStart;
    AP4_UI32            m_RtpTimeScale;
};

#endif // _AP4_HINT_TRACK_READER_H_
