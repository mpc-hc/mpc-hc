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
#if defined(MEDIAINFO_RKAU_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Rkau.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Rkau::File_Rkau()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Rkau::Streams_Finish()
{
    //Filling
    int64u CompressedSize=File_Size-TagsSize;
    float32 CompressionRatio=((float32)UncompressedSize)/CompressedSize;

    Fill(Stream_Audio, 0, Audio_StreamSize, CompressedSize);
    Fill(Stream_Audio, 0, Audio_Compression_Ratio, CompressionRatio);
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, "VBR");

    File__Tags_Helper::Streams_Finish();
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Rkau::FileHeader_Begin()
{
    if (!File__Tags_Helper::FileHeader_Begin())
        return false;

    //Synchro
    if (Buffer_Offset+3>Buffer_Size)
        return false;
    if (CC3(Buffer+Buffer_Offset)!=0x524B41) //"RKA"
    {
        File__Tags_Helper::Reject("RKAU");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void File_Rkau::FileHeader_Parse()
{
    //Parsing
    Ztring version;
    int32u SampleRate, source_bytes;
    int8u Channels, BitsPerSample, Quality, Flags;
    bool joint_stereo, streaming, vrq_lossy_mode;

    Skip_Local(3,                                               "Signature");
    Get_Local (1, version,                                      "Version");
    Get_L4 (source_bytes,                                       "SourceBytes");
    Get_L4 (SampleRate,                                         "SampleRate");
    Get_L1 (Channels,                                           "Channels");
    Get_L1 (BitsPerSample,                                      "BitsPerSample");
    Get_L1 (Quality,                                            "Quality");
    Get_L1 (Flags,                                              "Flags");
    Get_Flags (Flags, 0, joint_stereo,                          "JointStereo");
    Get_Flags (Flags, 1, streaming,                             "Streaming");
    Get_Flags (Flags, 2, vrq_lossy_mode,                        "VRQLossyMode");

    FILLING_BEGIN();
        if (SampleRate==0)
            return;
        Duration=(((int64u)source_bytes*1000)/4)/SampleRate;
        if (Duration==0)
            return;
        UncompressedSize = ((int64u)Channels) * ((int64u)BitsPerSample / 8);
        if (UncompressedSize==0)
            return;

        //Filling data
        File__Tags_Helper::Accept("RKAU");

        File__Tags_Helper::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "RK Audio");
        Fill(Stream_Audio, 0, Audio_Codec, "Rkau");
        Fill(Stream_Audio, 0, Audio_Encoded_Library, __T("1.0") + version);
        Fill(Stream_Audio, 0, Audio_Compression_Mode, (Quality==0)?"Lossless":"Lossy");
        Fill(Stream_Audio, 0, Audio_BitDepth, BitsPerSample);
        Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
        Fill(Stream_Audio, 0, Audio_SamplingRate, SampleRate);
        Fill(Stream_Audio, 0, Audio_Duration, Duration);

    FILLING_END();

    //No more needed data
    File__Tags_Helper::Finish("RKAU");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_RKAU_YES
