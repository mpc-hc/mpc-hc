/*****************************************************************
|
|    AP4 - MPEG2 Transport Streams
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

#ifndef _AP4_MPEG2_TS_H_
#define _AP4_MPEG2_TS_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|   classes
+---------------------------------------------------------------------*/
class AP4_ByteStream;
class AP4_Sample;
class AP4_SampleDescription;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_PID_PMT         = 0x100;
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_PID_AUDIO       = 0x101;
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_PID_VIDEO       = 0x102;
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_STREAM_ID_AUDIO = 0xc0;
const AP4_UI16 AP4_MPEG2_TS_DEFAULT_STREAM_ID_VIDEO = 0xe0;

const AP4_UI08 AP4_MPEG2_STREAM_TYPE_ISO_IEC_13818_7 = 0x0F;
const AP4_UI08 AP4_MPEG2_STREAM_TYPE_AVC             = 0x1B;

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter
+---------------------------------------------------------------------*/
/**
 * This class is a simple implementation of a converter that can 
 * convert MP4 audio and video access units into an MPEG2 transport
 * stream.
 * It currently only supports one audio tracks with MPEG4 AAC LC, and one
 * video track with MPEG4 AVC.
 */
class AP4_Mpeg2TsWriter
{
public:
    // classes
    class Stream {
    public:
        Stream(AP4_UI16 pid) : m_PID(pid), m_ContinuityCounter(0) {}
        virtual ~Stream() {}
        
        AP4_UI16 GetPID() { return m_PID; }
        void WritePacketHeader(bool            payload_start, 
                               unsigned int&   payload_size,
                               bool            with_pcr,
                               AP4_UI64        pcr,
                               AP4_ByteStream& output);
        
    private:
        unsigned int m_PID;
        unsigned int m_ContinuityCounter;
    };
    
    class SampleStream : public Stream {
    public:
        SampleStream(AP4_UI16 pid, AP4_UI16 stream_id, AP4_UI08 stream_type, AP4_UI32 timescale) :
            Stream(pid), 
            m_StreamId(stream_id),
            m_StreamType(stream_type),
            m_TimeScale(timescale) {}
        
        virtual AP4_Result WritePES(const unsigned char* data, 
                                    unsigned int         data_size, 
                                    AP4_UI64             dts, 
                                    bool                 with_dts, 
                                    AP4_UI64             pts, 
                                    bool                 with_pcr, 
                                    AP4_ByteStream&      output);
        virtual AP4_Result WriteSample(AP4_Sample&            sample, 
                                       AP4_SampleDescription* sample_description,
                                       bool                   with_pcr, 
                                       AP4_ByteStream&        output) = 0;
        
        unsigned int m_StreamId;
        AP4_UI08     m_StreamType;
        AP4_UI32     m_TimeScale;
    };
    
    // constructor
    AP4_Mpeg2TsWriter();
    ~AP4_Mpeg2TsWriter();
    
    Stream* GetPAT() { return m_PAT; }
    Stream* GetPMT() { return m_PMT; }
    AP4_Result WritePAT(AP4_ByteStream& output);
    AP4_Result WritePMT(AP4_ByteStream& output);
    AP4_Result SetAudioStream(AP4_UI32 timescale, SampleStream*& stream);
    AP4_Result SetVideoStream(AP4_UI32 timescale, SampleStream*& stream);
    
private:
    Stream*       m_PAT;
    Stream*       m_PMT;
    SampleStream* m_Audio;
    SampleStream* m_Video;
};

#endif // _AP4_MPEG2_TS_H_
