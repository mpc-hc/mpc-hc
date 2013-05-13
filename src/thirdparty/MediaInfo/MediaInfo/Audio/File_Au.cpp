/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Source: http://wiki.multimedia.cx/index.php?title=Au/snd
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
#if defined(MEDIAINFO_AU_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Au.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Au_Format(int32u sample_format)
{
    switch (sample_format)
    {
        case  1 : return "ADPCM";
        case  2 : return "PCM";
        case  3 : return "PCM";
        case  4 : return "PCM";
        case  5 : return "PCM";
        case  6 : return "PCM";
        case  7 : return "PCM";
        case  8 : return "fragmented sampled data";
        case 10 : return "DSP program";
        case 11 : return "PCM";
        case 12 : return "PCM";
        case 13 : return "PCM";
        case 14 : return "PCM";
        case 17 : return "ADPCM";
        case 18 : return "PCM";
        case 19 : return "PCM";
        case 20 : return "PCM";
        case 21 : return "Music Kit DSP commands";
        case 22 : return "Music Kit DSP samples";
        case 23 : return "ADPCM";
        case 24 : return "ADPCM";
        case 25 : return "ADPCM";
        case 26 : return "ADPCM";
        case 27 : return "ADPCM";
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Au_sample_format(int32u sample_format)
{
    switch (sample_format)
    {
        case  1 : return "8-bit mu-law";
        case  2 : return "8-bit signed linear";
        case  3 : return "16-bit signed linear";
        case  4 : return "24-bit signed linear";
        case  5 : return "32-bit signed linear";
        case  6 : return "floating-point";
        case  7 : return "double precision float";
        case  8 : return "fragmented sampled data";
        case 10 : return "DSP program";
        case 11 : return "8-bit fixed-point";
        case 12 : return "16-bit fixed-point";
        case 13 : return "24-bit fixed-point";
        case 14 : return "32-bit fixed-point";
        case 17 : return "mu-law squelch";
        case 18 : return "16-bit linear with emphasis";
        case 19 : return "16-bit linear with compression";
        case 20 : return "16-bit linear with emphasis and compression";
        case 21 : return "Music Kit DSP commands";
        case 22 : return "Music Kit DSP samples";
        case 23 : return "G.721 ADPCM";
        case 24 : return "G.722 ADPCM";
        case 25 : return "G.723 ADPCM";
        case 26 : return "5-bit G.723 ADPCM";
        case 27 : return "8-bit a-law";
        default : return "";
    }
}

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Au::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    if (CC4(Buffer)!=0x2E736E64) //".snd"
    {
        Reject("AU");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Au::FileHeader_Parse()
{
    //Parsing
    Ztring arbitrary;
    int32u data_start, data_size, sample_format, sample_rate, channels;
    Skip_B4(                                                    "Magic");
    Get_B4 (data_start,                                         "data_start");
    Get_B4 (data_size,                                          "data_size");
    Get_B4 (sample_format,                                      "sample_format");
    Get_B4 (sample_rate,                                        "sample_rate");
    Get_B4 (channels,                                           "channels");
    if (data_start>24)
        Get_Local(data_start-24, arbitrary,                     "arbitrary data");

    FILLING_BEGIN();
        Accept("AU");

        Fill(Stream_General, 0, General_Format, "AU");

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, Au_Format(sample_format));
        Fill(Stream_Audio, 0, Audio_CodecID, Au_sample_format(sample_format));
        Fill(Stream_Audio, 0, Audio_Codec, Au_sample_format(sample_format));
        Fill(Stream_Audio, 0, Audio_Channel_s_, channels);
        Fill(Stream_Audio, 0, Audio_SamplingRate, sample_rate);
        if (File_Size!=(int64u)-1)
            data_size=(int32u)File_Size-data_start; //Priority for File size
        if (sample_rate && data_size!=0 && data_size!=0xFFFFFFFF)
            Fill(Stream_Audio, 0, Audio_Duration, ((int64u)data_size)*1000/sample_rate);
        Fill(Stream_Audio, 0, Audio_StreamSize, File_Size-Element_Offset);
        Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
        Fill(Stream_General, 0, General_Comment, arbitrary);

        //No more need data
        Finish("AU");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AU_YES
