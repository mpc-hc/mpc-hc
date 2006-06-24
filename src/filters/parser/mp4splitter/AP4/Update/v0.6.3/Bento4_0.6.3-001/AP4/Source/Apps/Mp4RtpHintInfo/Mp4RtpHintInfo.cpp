/*****************************************************************
|
|    AP4 - MP4 Rtp Hint Info
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
#include <stdio.h>
#include <stdlib.h>

#include "Ap4.h"
#include "Ap4HintTrackReader.h"
#include "Ap4ByteStream.h"
#include "Ap4File.h"
#include "Ap4Movie.h"
#include "Ap4FileByteStream.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define BANNER "MP4 Rtp Hint Info - Version 0.1a\n"\
    "(c) 2002-2005 Gilles Boccon-Gibod & Julien Boeuf"

/*----------------------------------------------------------------------
|       PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
        BANNER 
        "\n\nusage: mp4rtphintinfo [options] <input>\n");
    exit(1);
}

/*----------------------------------------------------------------------
|       DumpRtpPackets
+---------------------------------------------------------------------*/
static AP4_Result
DumpRtpPackets(AP4_HintTrackReader& reader, const char* file_name)
{
    // create the output stream
    AP4_ByteStream* output;
    try {
        output = new AP4_FileByteStream(file_name,
            AP4_FileByteStream::STREAM_MODE_WRITE);
    } catch (AP4_Exception) {
        fprintf(stderr, "ERROR: cannot open output file (%s)\n", file_name);
        return AP4_FAILURE;
    }

    // read the packets from the reader and write them in the output stream
    AP4_DataBuffer data(1500);
    AP4_TimeStamp ts;
    while(AP4_SUCCEEDED(reader.GetNextPacket(data, ts))) {
        output->Write(data.GetData(), data.GetDataSize());
        AP4_Debug("#########\n\tpacket contains %d bytes\n", data.GetDataSize());
        AP4_Debug("\tsent at time stamp %d\n\n", ts);
    }
    
    output->Release();

    return AP4_SUCCESS;
}


/*----------------------------------------------------------------------
|       main
+---------------------------------------------------------------------*/
int
main(int argc, char** argv)
{
    AP4_Result result = AP4_SUCCESS;

    // parse the command line
    if (argc != 2) PrintUsageAndExit();

    // create the input stream
    AP4_ByteStream* input;
    try {
        input = new AP4_FileByteStream(argv[1],
            AP4_FileByteStream::STREAM_MODE_READ);
    } catch (AP4_Exception) {
        fprintf(stderr, "ERROR: cannot open input file (%s)\n", argv[1]);
        return 1;
    }

    AP4_File* file = new AP4_File(*input);
  

    AP4_Movie* movie = file->GetMovie();
    if (movie != NULL) {
        // get a hint track reader
        AP4_Track* hint_track = movie->GetTrack(AP4_Track::TYPE_HINT, 1);
        if (hint_track == NULL) {
            AP4_Debug("No hint track in this movie\n");
            return AP4_FAILURE;
        }
        AP4_HintTrackReader reader(*hint_track, *movie, 0x01020304);
        AP4_String rtp_file_name(argv[1]);
        rtp_file_name += ".rtp";

        // display the sdp
        AP4_String sdp;
        reader.GetSdpText(sdp);
        AP4_Debug("sdp:\n%s\n\n", sdp.c_str());

        // dump the packet
        result = DumpRtpPackets(reader, rtp_file_name.c_str());
        if (AP4_FAILED(result)) goto bail;

    } else {
        AP4_Debug("No movie found in the file\n");
        return AP4_FAILURE;
    }

bail:
    delete file;
    input->Release();

    return result;
}
