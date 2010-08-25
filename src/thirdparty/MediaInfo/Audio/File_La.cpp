// File_La - Info for LA files
// Copyright (C) 2009-2009 Lionel Duchateau, kurtnoise@free.fr
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
#if defined(MEDIAINFO_LA_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_La.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_La::File_La()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_La::Streams_Finish()
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
bool File_La::FileHeader_Begin()
{
    if (!File__Tags_Helper::FileHeader_Begin())
        return false;

    //Synchro
    if (Buffer_Offset+4>Buffer_Size)
        return false;
    if (CC4(Buffer+Buffer_Offset)!=0x4C413034) //"LA04"
    {
        File__Tags_Helper::Reject("LA");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void File_La::FileHeader_Parse()
{
    //Parsing
    int32u SampleRate, Samples, BytesPerSecond, UnCompSize, WAVEChunk, FmtSize, FmtChunk, CRC32;
    int16u RawFormat, Channels, BytesPerSample, BitsPerSample;

    Skip_C4(                                                    "signature");
    Get_L4 (UnCompSize,                                         "uncompressed_size");
    Get_L4 (WAVEChunk,                                          "chunk");
    Skip_L4(                                                    "fmt_size");
    Get_L4 (FmtChunk,                                           "fmt_chunk");
    Get_L4 (FmtSize,                                            "fmt_size");
    Get_L2 (RawFormat,                                          "raw_format");
    Get_L2 (Channels,                                           "channels"); Param_Info(Channels, " channel(s)");
    Get_L4 (SampleRate,                                         "sample_rate");
    Get_L4 (BytesPerSecond,                                     "bytes_per_second");
    Get_L2 (BytesPerSample,                                     "bytes_per_sample");
    Get_L2 (BitsPerSample,                                      "bits_per_sample");
    Get_L4 (Samples,                                            "samples");
    Skip_L1(                                                    "flags");
    Get_L4 (CRC32,                                              "crc");

    FILLING_BEGIN()
        if (SampleRate==0)
            return;
        Duration=((int64u)Samples/Channels)*1000/SampleRate; // Seems that it's samples per channels otherwise Duration is doubled ??!!
        if (Duration==0)
            return;
        UncompressedSize=(Samples)*Channels*(BitsPerSample/8);
        if (UncompressedSize==0)
            return;

        File__Tags_Helper::Accept("LA");

        File__Tags_Helper::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "LA");
        Fill(Stream_Audio, 0, Audio_Codec, "LA");
        Fill(Stream_Audio, 0, Audio_Resolution, BitsPerSample);
        Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
        Fill(Stream_Audio, 0, Audio_SamplingRate, SampleRate);
        Fill(Stream_Audio, 0, Audio_Duration, Duration);

        //No more need data
        File__Tags_Helper::Finish("LA");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_LA_YES

