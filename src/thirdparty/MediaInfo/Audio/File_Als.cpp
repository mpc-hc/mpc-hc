// File_Als - Info for MPEG-4 ALS files
// Copyright (C) 2009-2009 Lionel Duchateau, kurtnoise@free.fr
// Copyright (C) 2009-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG-4 ALS files
// http://www.nue.tu-berlin.de/forschung/projekte/lossless/mp4als.html
// http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio_Lossless_Coding_%28ALS%29
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_ALS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Als.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Als::File_Als()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Als::Streams_Finish()
{
    //Filling
    int64u CompressedSize=File_Size-TagsSize;
    float32 CompressionRatio=((float32)UncompressedSize)/CompressedSize;

    Fill(Stream_Audio, 0, Audio_StreamSize, CompressedSize);
    Fill(Stream_Audio, 0, Audio_CompressionRatio, CompressionRatio);

    File__Tags_Helper::Streams_Finish();
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Als::FileHeader_Begin()
{
    if (!File__Tags_Helper::FileHeader_Begin())
        return false;

    //Synchro
    if (Buffer_Offset+4>Buffer_Size)
        return false;
    if (CC4(Buffer+Buffer_Offset)!=0x414C5300) //"ALS\0"
    {
        File__Tags_Helper::Reject("ALS");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void File_Als::FileHeader_Parse()
{
    //Parsing
    int32u SampleRate, Samples;
    int16u Channels;
    int8u BitsPerSample, FileType;
    Skip_C4(                                                    "signature");
    Get_B4 (SampleRate,                                         "sample rate");
    Get_B4 (Samples,                                            "samples");
    Get_B2 (Channels,                                           "channels-1"); Param_Info(Channels+1, " channel(s)");
    BS_Begin();
    Get_S1 (3, FileType,                                        "file type"); // WAV, RIFF, AIFF
    Get_S1 (3, BitsPerSample,                                   "bits per sample"); Param_Info((BitsPerSample+1)*8, " bits");
    Skip_SB(                                                    "floating point");
    Skip_SB(                                                    "samples are big-endian");
    BS_End();

    FILLING_BEGIN()
        if (SampleRate==0)
            return;
        Duration=((int64u)Samples)*1000/SampleRate;
        if (Duration==0)
            return;
        UncompressedSize=Samples*Channels*(BitsPerSample+1/8);
        if (UncompressedSize==0)
            return;

        File__Tags_Helper::Accept("ALS");

        File__Tags_Helper::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "ALS");
        Fill(Stream_Audio, 0, Audio_Codec, "ALS");
        Fill(Stream_Audio, 0, Audio_Resolution, (BitsPerSample+1)*8);
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels+1);
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, SampleRate);
        Fill(Stream_Audio, 0, Audio_Duration, Duration);

        //No more need data
        File__Tags_Helper::Finish("ALS");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_ALS_YES

