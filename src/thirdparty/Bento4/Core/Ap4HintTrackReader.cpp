/*****************************************************************
|
|    AP4 - Hint Track Reader
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
#include <stdlib.h>
#include <time.h>
#include "Ap4HintTrackReader.h"
#include "Ap4DataBuffer.h"
#include "Ap4Track.h"
#include "Ap4Movie.h"
#include "Ap4SdpAtom.h"
#include "Ap4RtpHint.h"
#include "Ap4TrakAtom.h"
#include "Ap4TrefTypeAtom.h"
#include "Ap4TimsAtom.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::AP4_HintTrackReader
+---------------------------------------------------------------------*/
AP4_HintTrackReader::AP4_HintTrackReader(AP4_Track& hint_track, 
                                         AP4_Movie& movie, 
                                         AP4_UI32 ssrc /* = 0 */) :
    m_HintTrack(hint_track),
    m_MediaTrack(NULL),
    m_MediaTimeScale(0),
    m_RtpSampleData(NULL),
    m_Ssrc(ssrc),
    m_SampleIndex(0),
    m_PacketIndex(0),
    m_RtpSequenceStart(0),
    m_RtpTimeStampStart(0),
    m_RtpTimeScale(0)
{
    // check the type
    if (m_HintTrack.GetType() != AP4_Track::TYPE_HINT) 
        throw AP4_Exception(AP4_ERROR_INVALID_TRACK_TYPE);

    // get the media track
    AP4_TrakAtom* hint_trak_atom = hint_track.GetTrakAtom();
    AP4_Atom* atom = hint_trak_atom->FindChild("tref/hint");
    if (atom != NULL) {
        AP4_UI32 media_track_id = ((AP4_TrefTypeAtom*) atom)->m_TrackIds[0];
        m_MediaTrack = movie.GetTrack(media_track_id);

        // get the media time scale
        m_MediaTimeScale = m_MediaTrack->GetMediaTimeScale();
    }

    // initiate random generator
    srand(time(NULL));

    // rtp sequence start init TODO!!
    m_RtpSequenceStart = rand();

    // rtp timestamp start init TODO!!
    m_RtpTimeStampStart = rand();

    // rtp time scale
    atom = hint_trak_atom->FindChild("mdia/minf/stbl/rtp /tims");
    if (atom) {
        AP4_TimsAtom* tims = (AP4_TimsAtom*)atom;
        m_RtpTimeScale = tims->GetTimeScale();
    }

    // generate a random ssrc if = 0
    if (m_Ssrc == 0) {
        m_Ssrc = rand();
    }

    // get the first sample
    GetRtpSample(0);
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::~AP4_HintTrackReader
+---------------------------------------------------------------------*/
AP4_HintTrackReader::~AP4_HintTrackReader()
{
    delete m_RtpSampleData;
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::GetRtpSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_HintTrackReader::GetRtpSample(AP4_Ordinal index)
{
    // get the sample
    AP4_Result result = m_HintTrack.GetSample(index, m_CurrentHintSample);
    if (AP4_FAILED(result)) return result;

    // renew the sample data
    delete m_RtpSampleData;
    AP4_ByteStream& rtp_data_stream = *m_CurrentHintSample.GetDataStream();
    rtp_data_stream.Seek(m_CurrentHintSample.GetOffset());
    m_RtpSampleData = new AP4_RtpSampleData(rtp_data_stream,
                                            m_CurrentHintSample.GetSize());

    // reinit the packet index
    m_PacketIndex = 0;

    // release the stream
    rtp_data_stream.Release();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::GetCurrentTimeStampMs
+---------------------------------------------------------------------*/
AP4_TimeStamp
AP4_HintTrackReader::GetCurrentTimeStampMs()
{
    return AP4_ConvertTime(m_CurrentHintSample.GetCts(), 
                           m_HintTrack.GetMediaTimeScale(),
                           1000);
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::Rewind
+---------------------------------------------------------------------*/
AP4_Result
AP4_HintTrackReader::Rewind()
{
    m_SampleIndex = 0;
    return GetRtpSample(m_SampleIndex);
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::GetSdpText
+---------------------------------------------------------------------*/
AP4_Result
AP4_HintTrackReader::GetSdpText(AP4_String& sdp_text)
{
    AP4_Atom* sdp_atom = m_HintTrack.GetTrakAtom()->FindChild("udta/hnti/sdp ");
    if (sdp_atom == NULL) return AP4_FAILURE;

    // C cast is OK because we know the type of the atom
    sdp_text = ((AP4_SdpAtom*) sdp_atom)->GetSdpText(); 
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::SeekToTimeStampMs
+---------------------------------------------------------------------*/
AP4_Result
AP4_HintTrackReader::SeekToTimeStampMs(AP4_TimeStamp desired_ts, 
                                       AP4_TimeStamp& actual_ts)
{
    // get the sample index
    AP4_Cardinal index;
    AP4_Result result = m_HintTrack.GetSampleIndexForTimeStampMs(desired_ts, index);
    if (AP4_FAILED(result)) return result;

    // get the current sample based on the index and renew the sample data
    result = GetRtpSample(index);
    if (AP4_FAILED(result)) return result;

    // set the actual ts
    actual_ts = GetCurrentTimeStampMs();
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::GetNextPacket
+---------------------------------------------------------------------*/
AP4_Result
AP4_HintTrackReader::GetNextPacket(AP4_DataBuffer& packet_data, 
                                   AP4_TimeStamp& ts_ms)
{
    AP4_Result result = AP4_SUCCESS;

    // get the next rtp sample if needed
    AP4_List<AP4_RtpPacket>* packets = &m_RtpSampleData->GetPackets();
    while (m_PacketIndex == packets->ItemCount()) { // while: handle the 0 packet case
        result = GetRtpSample(++m_SampleIndex);
        if (AP4_FAILED(result)) return result;
        packets = &m_RtpSampleData->GetPackets();
    }

    // get the packet
    AP4_RtpPacket* packet;
    result = packets->Get(m_PacketIndex++, packet);
    if (AP4_FAILED(result)) return result;

    // build it
    result = BuildRtpPacket(packet, packet_data);
    if (AP4_FAILED(result)) return result;

    // set the time stamp
    ts_ms = GetCurrentTimeStampMs();

    return result;
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::BuildRtpPacket
+---------------------------------------------------------------------*/
AP4_Result
AP4_HintTrackReader::BuildRtpPacket(AP4_RtpPacket* packet, 
                                    AP4_DataBuffer& packet_data)
{
    // set the data size
    AP4_Result result = packet_data.SetDataSize(packet->GetConstructedDataSize());
    if (AP4_FAILED(result)) return result;

    // now write
    AP4_ByteStream* stream = 
        new AP4_MemoryByteStream(packet_data.UseData(), packet_data.GetDataSize()); 

    // header + ssrc
    stream->WriteUI08(0x80 | (packet->GetPBit() << 5) | (packet->GetXBit() << 4));
    stream->WriteUI08((packet->GetMBit() << 7) | packet->GetPayloadType());
    stream->WriteUI16(m_RtpSequenceStart + packet->GetSequenceSeed());
    stream->WriteUI32(m_RtpTimeStampStart + m_CurrentHintSample.GetCts() + packet->GetTimeStampOffset());
    stream->WriteUI32(m_Ssrc);

    AP4_List<AP4_RtpConstructor>::Item* constructors_it 
        = packet->GetConstructors().FirstItem();
    while (constructors_it != NULL) {
        AP4_RtpConstructor* constructor = constructors_it->GetData();

        // add data to the packet according to the constructor
        switch (constructor->GetType()) {
            case AP4_RTP_CONSTRUCTOR_TYPE_NOOP:
                // nothing to do here
                break;
            case AP4_RTP_CONSTRUCTOR_TYPE_IMMEDIATE:
                result = WriteImmediateRtpData(
                    (AP4_ImmediateRtpConstructor*) constructor, stream);
                if (AP4_FAILED(result)) return result;
                break;
            case AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE:
                result = WriteSampleRtpData(
                    (AP4_SampleRtpConstructor*) constructor, stream);
                if (AP4_FAILED(result)) return result;
                break;
            case AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE_DESC:
                return AP4_ERROR_NOT_SUPPORTED_YET;
            default:
                // unknown constructor type
                return AP4_FAILURE;
        }

        // iterate
        constructors_it = constructors_it->GetNext();
    }

    // release the stream
    stream->Release();

    return result;
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::WriteImmediateRtpData
+---------------------------------------------------------------------*/
AP4_Result
AP4_HintTrackReader::WriteImmediateRtpData(AP4_ImmediateRtpConstructor* constructor, 
                                           AP4_ByteStream* data_stream)
{
    const AP4_DataBuffer& data_buffer = constructor->GetData();
    return data_stream->Write(data_buffer.GetData(), data_buffer.GetDataSize());
}

/*----------------------------------------------------------------------
|       AP4_HintTrackReader::WriteSampleRtpData
+---------------------------------------------------------------------*/
AP4_Result
AP4_HintTrackReader::WriteSampleRtpData(AP4_SampleRtpConstructor* constructor, 
                                        AP4_ByteStream* data_stream)
{
    AP4_Track* referenced_track = NULL;
    if (constructor->GetTrackRefIndex() == 0xFF) {
        // data is in the hint track
        referenced_track = &m_HintTrack;
    } else {
        // check if we have a media track
        if (m_MediaTrack == NULL) return AP4_FAILURE;
        referenced_track = m_MediaTrack;
    }

    // write the sample data
    AP4_Sample sample;
    AP4_Result result = referenced_track->GetSample(constructor->GetSampleNum()-1, // adjust
                                                    sample);
    AP4_DataBuffer buffer(constructor->GetLength());
    result = sample.ReadData(
        buffer, constructor->GetLength(), constructor->GetSampleOffset());
    if (AP4_FAILED(result)) return result;

    // write the data
    return data_stream->Write(buffer.GetData(), buffer.GetDataSize());
}

