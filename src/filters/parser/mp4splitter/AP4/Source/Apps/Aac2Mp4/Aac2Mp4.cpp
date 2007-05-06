/*****************************************************************
|
|    AP4 - AAC to MP4 Converter
|
|    Copyright 2003 Gilles Boccon-Gibod & Julien Boeuf
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
#include <stdio.h>
#include <stdlib.h>

#include "Ap4.h"
#include "Ap4File.h"
#include "Ap4FileWriter.h"
#include "Ap4FileByteStream.h"
#include "Ap4SyntheticSampleTable.h"
#include "Ap4AdtsParser.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define BANNER "AAC to MP4 Converter - Version 0.1a - (c) 2002-2005 Gilles Boccon-Gibod"
 
/*----------------------------------------------------------------------
|       PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
            BANNER 
            "\n\nusage: aac2mp4 [options] <input> <output>\n");
    exit(1);
}

/*----------------------------------------------------------------------
|       main
+---------------------------------------------------------------------*/
int
main(int argc, char** argv)
{
    AP4_Result result;

    if (argc < 2) {
        PrintUsageAndExit();
    }
    
    // open the input
    AP4_ByteStream* input;
    try {
        input = new AP4_FileByteStream(argv[1], AP4_FileByteStream::STREAM_MODE_READ);
    } catch (AP4_Exception&) {
        AP4_Debug("ERROR: cannot open input (%s)\n", argv[1]);
        return 1;
    }

    // open the output
    AP4_ByteStream* output = new AP4_FileByteStream(
        argv[2],
        AP4_FileByteStream::STREAM_MODE_WRITE);
    
    // create a sample table
    AP4_SyntheticSampleTable* sample_table = new AP4_SyntheticSampleTable();

    // create an ADTS parser
    AP4_AdtsParser parser;
    bool           initialized = false;
    unsigned int   sample_description_index = 0;

    // read from the input, feed, and get AAC frames
    AP4_UI32     sample_rate = 0;
    AP4_Cardinal sample_count = 0;
    bool eos = false;
    for(;;) {
        // try to get a frame
        AP4_AacFrame frame;
        result = parser.FindFrame(frame);
        if (AP4_SUCCEEDED(result)) {
            AP4_Debug("AAC frame [%06d]: size = %d, %d kHz, %d ch\n",
                       sample_count,
                       frame.m_Info.m_FrameLength,
                       frame.m_Info.m_SamplingFrequency,
                       frame.m_Info.m_ChannelConfiguration);
            if (!initialized) {
                initialized = true;

                // create a sample description for our samples
                AP4_DataBuffer dsi;
                unsigned char aac_dsi[2] = {0x12, 0x10};
                dsi.SetData(aac_dsi, 2);
                AP4_MpegAudioSampleDescription* sample_description = 
                    new AP4_MpegAudioSampleDescription(
                    AP4_MPEG4_AUDIO_OTI,   // object type
                    frame.m_Info.m_SamplingFrequency,
                    16,                    // sample size
                    frame.m_Info.m_ChannelConfiguration,
                    &dsi,                  // decoder info
                    6144,                  // buffer size
                    128000,                // max bitrate
                    128000);               // average bitrate
                sample_description_index = sample_table->AddSampleDescription(sample_description);
                sample_rate = frame.m_Info.m_SamplingFrequency;
            }

            AP4_MemoryByteStream* sample_data = new AP4_MemoryByteStream(frame.m_Info.m_FrameLength);
            frame.m_Source->ReadBytes(sample_data->GetBuffer(), frame.m_Info.m_FrameLength);
            printf("%02x %02x %02x %02x\n", 
                sample_data->GetBuffer()[0],
                sample_data->GetBuffer()[1],
                sample_data->GetBuffer()[2],
                sample_data->GetBuffer()[3]);
            sample_table->AddSample(*sample_data, 0, frame.m_Info.m_FrameLength, sample_description_index);
            sample_data->Release();
            sample_count++;
        } else {
            if (eos) break;
        }

        // read some data and feed the parser
        AP4_UI08 input_buffer[4096];
        AP4_Size bytes_read = 0;
        AP4_Size to_read = parser.GetBytesFree();
        if (to_read) {
            if (to_read > sizeof(input_buffer)) to_read = sizeof(input_buffer);
            result = input->Read(input_buffer, to_read, &bytes_read);
            if (AP4_SUCCEEDED(result)) {
                AP4_Size to_feed = bytes_read;
                result = parser.Feed(input_buffer, &to_feed);
                if (AP4_FAILED(result)) {
                    AP4_Debug("ERROR: parser.Feed() failed (%d)\n", result);
                    return 1;
                }
            } else {
                if (result == AP4_ERROR_EOS) {
                    eos = true;
                }
            }
        }
   }

    // create an audio track
    AP4_Track* track = new AP4_Track(AP4_Track::TYPE_AUDIO, 
                                     sample_table, 
                                     0,     // track id
                                     sample_rate, // movie time scale              
                                     sample_rate, // track time scale
                                     sample_count*1024, // track duration
                                     "eng", // language
                                     0, 0); // width, height

    // create a movie
    AP4_Movie* movie = new AP4_Movie();

    // add the track to the movie
    movie->AddTrack(track);

    // create a multimedia file
    AP4_File* file = new AP4_File(movie);

    // create a writer to write the file
    AP4_FileWriter* writer = new AP4_FileWriter(*file);

    // write the file to the output
    writer->Write(*output);

    delete writer;
    delete file;
    delete output;
    
    return 0;
}
