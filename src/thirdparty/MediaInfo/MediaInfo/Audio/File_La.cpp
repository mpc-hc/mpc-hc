/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
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
    Fill(Stream_Audio, 0, Audio_Compression_Ratio, CompressionRatio);

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
    if (Buffer_Offset+2>Buffer_Size)
        return false;
    if (CC3(Buffer+Buffer_Offset)!=0x4C4130) //"LA0"
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
    Ztring Major, Minor;
    int32u SampleRate, Samples, BytesPerSecond, UnCompSize, WAVEChunk, FmtSize, FmtChunk, CRC32;
    int16u RawFormat, Channels, BytesPerSample, BitsPerSample;

    Skip_Local(2,                                               "signature");
    Get_Local (1, Major,                                        "major_version");
    Get_Local (1, Minor,                                        "minor_version");
    Get_L4 (UnCompSize,                                         "uncompressed_size");
    Get_L4 (WAVEChunk,                                          "chunk");
    Skip_L4(                                                    "fmt_size");
    Get_L4 (FmtChunk,                                           "fmt_chunk");
    Get_L4 (FmtSize,                                            "fmt_size");
    Get_L2 (RawFormat,                                          "raw_format");
    Get_L2 (Channels,                                           "channels"); Param_Info2(Channels, " channel(s)");
    Get_L4 (SampleRate,                                         "sample_rate");
    Get_L4 (BytesPerSecond,                                     "bytes_per_second");
    Get_L2 (BytesPerSample,                                     "bytes_per_sample");
    Get_L2 (BitsPerSample,                                      "bits_per_sample");
    Get_L4 (Samples,                                            "samples");
    Skip_L1(                                                    "flags");
    Get_L4 (CRC32,                                              "crc");

    FILLING_BEGIN();
        if (SampleRate==0)
            return;
        Duration=((int64u)Samples/Channels)*1000/SampleRate; // Seems that it's samples per channels otherwise Duration is doubled ??!!
        if (Duration==0)
            return;
        UncompressedSize=((int64u)Samples)*Channels*(BitsPerSample/8);
        if (UncompressedSize==0)
            return;

        File__Tags_Helper::Accept("LA");
        Fill(Stream_General, 0, General_Format_Version, Major+__T('.')+Minor);

        File__Tags_Helper::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "LA");
        Fill(Stream_Audio, 0, Audio_Codec, "LA");
        Fill(Stream_Audio, 0, Audio_Format_Version, Major+__T('.')+Minor);
        Fill(Stream_Audio, 0, Audio_BitDepth, BitsPerSample);
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

