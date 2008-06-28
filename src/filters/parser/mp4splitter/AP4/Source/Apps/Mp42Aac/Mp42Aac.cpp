/*****************************************************************
|
|    AP4 - MP4 to AAC File Converter
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
#include "Ap4FileByteStream.h"
#include "Ap4Atom.h"
#include "Ap4File.h"
#include "Ap4Sample.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define BANNER "MP4 To AAC File Converter - Version 0.1a\n"\
               "(c) 2003-2005 Gilles Boccon-Gibod & Julien Boeuf"
 
/*----------------------------------------------------------------------
|       PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
            BANNER 
            "\n\nusage: ap42aac [options] <input> <output>\n");
    exit(1);
}

/*----------------------------------------------------------------------
|       WriteAdtsHeader
+---------------------------------------------------------------------*/
static AP4_Result
WriteAdtsHeader(AP4_ByteStream* output, unsigned int frame_size)
{
	unsigned char bits[7];

	bits[0] = 0xFF;
	bits[1] = 0xF1; // 0xF9 (MPEG2)
	bits[2] = 0x50;
	bits[3] = 0x80 | ((frame_size+7) >> 11);
    bits[4] = ((frame_size+7) >> 3)&0xFF;
	bits[5] = (((frame_size+7) << 5)&0xFF) | 0x1F;
	bits[6] = 0xFC;

	return output->Write(bits, 7);

	/*
0:  syncword 12 always: '111111111111' 
12: ID 1 0: MPEG-4, 1: MPEG-2 
13: layer 2 always: '00' 
15: protection_absent 1  
16: profile 2  
18: sampling_frequency_index 4  
22: private_bit 1  
23: channel_configuration 3  
26: original/copy 1  
27: home 1  
28: emphasis 2 only if ID == 0 

ADTS Variable header: these can change from frame to frame 
28: copyright_identification_bit 1  
29: copyright_identification_start 1  
30: aac_frame_length 13 length of the frame including header (in bytes) 
43: adts_buffer_fullness 11 0x7FF indicates VBR 
54: no_raw_data_blocks_in_frame 2  
ADTS Error check 
crc_check 16 only if protection_absent == 0 
*/
}

/*----------------------------------------------------------------------
|       main
+---------------------------------------------------------------------*/
int
main(int argc, char** argv)
{
    if (argc < 3) {
        PrintUsageAndExit();
    }
    
	// create the input stream
    AP4_ByteStream* input = 
        new AP4_FileByteStream(argv[1],
                               AP4_FileByteStream::STREAM_MODE_READ);

	// create the output stream
    AP4_ByteStream* output =
        new AP4_FileByteStream(argv[2],
                               AP4_FileByteStream::STREAM_MODE_WRITE);

	// open the file
    AP4_File* input_file = new AP4_File(*input);   

    // get the movie
    AP4_Movie* movie = input_file->GetMovie();
    if (movie != NULL) {
        AP4_List<AP4_Track>& tracks = movie->GetTracks();
        AP4_Debug("Found %d Tracks\n", tracks.ItemCount());
	    // get audio track
        AP4_Track* audio_track = movie->GetTrack(AP4_Track::TYPE_AUDIO);
        if (audio_track != NULL) {
		    // show info
            AP4_Debug("Audio Track:\n");
            AP4_Debug("  duration: %ld ms\n", audio_track->GetDurationMs());
            AP4_Debug("  sample count: %ld\n", audio_track->GetSampleCount());

		    AP4_Sample     sample;
            AP4_DataBuffer data;
		    AP4_Ordinal    index = 0;
            while (AP4_SUCCEEDED(audio_track->ReadSample(index, sample, data))) {
			    WriteAdtsHeader(output, sample.GetSize());
                output->Write(data.GetData(), data.GetDataSize());
                AP4_Debug("  [%d] writing %ld bytes of data...\n", 
                            index, sample.GetSize());
			    index++;
            }
        } else {
            AP4_Debug("No Audio Track found\n");    
        }
    } else {
        AP4_Debug("No Movie in file\n");
    }

    delete input_file;
    input->Release();
    output->Release();

    return 0;
}

