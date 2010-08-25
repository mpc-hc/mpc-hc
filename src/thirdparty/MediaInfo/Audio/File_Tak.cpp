// File_Tak - Info for Tak files
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
// Specifications : http://linuxstb.cream.org/tak_format.html
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
#if defined(MEDIAINFO_TAK_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Tak.h"
#if defined(MEDIAINFO_RIFF_YES)
    #include "MediaInfo/Multiple/File_Riff.h"
#endif
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Const
//***************************************************************************

namespace Elements
{
    const int16u ENDOFMETADATA      =0x00;
    const int16u STREAMINFO         =0x01;
    const int16u SEEKTABLE          =0x02;
    const int16u WAVEMETADATA       =0x03;
    const int16u ENCODERINFO        =0x04;
    const int16u PADDING            =0x05;
}

//***************************************************************************
// Const
//***************************************************************************

int8u Tak_samplesize[]=
{
     8,
    16,
    24,
     0,
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Tak::File_Tak()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Tak::FileHeader_Begin()
{
    if (!File__Tags_Helper::FileHeader_Begin())
        return false;

    //Synchro
    if (Buffer_Offset+4>Buffer_Size)
        return false;
    if (CC4(Buffer+Buffer_Offset)!=0x7442614B) //"tBaK"
    {
        File__Tags_Helper::Reject("TAK");
        return false;
    }

    return true;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tak::FileHeader_Parse()
{
    Skip_C4(                                                    "Signature");
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tak::Header_Parse()
{
    //Parsing
    int32u block_length;
    int8u block_type;
    Get_L1 (block_type,                                         "Block Type");
    Get_L3 (block_length,                                       "Block Length");

    //Filling
    Header_Fill_Code(block_type, Ztring().From_CC1(block_type));
    Header_Fill_Size(Element_Offset+block_length);
}

//---------------------------------------------------------------------------
void File_Tak::Data_Parse()
{
    #define CASE_INFO(_NAME) \
        case Elements::_NAME : Element_Info(#_NAME); _NAME(); break;

    //Parsing
    switch (Element_Code)
    {
        CASE_INFO(ENDOFMETADATA);
        CASE_INFO(STREAMINFO);
        CASE_INFO(SEEKTABLE);
        CASE_INFO(WAVEMETADATA);
        CASE_INFO(ENCODERINFO);
        CASE_INFO(PADDING);
        default : Skip_XX(Element_Size,                         "Data");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tak::ENDOFMETADATA()
{
    //Filling
    Fill(Stream_General, 0, General_StreamSize, 0); //File_Offset+Buffer_Offset+Element_Size);
    Fill(Stream_Audio, 0, Audio_StreamSize, File_Size-(File_Offset+Buffer_Offset+Element_Size));
    File__Tags_Helper::Finish("TAK");
}

//---------------------------------------------------------------------------
void File_Tak::STREAMINFO()
{
    //Parsing
    int32u num_samples_hi, samplerate;
    int8u  num_samples_lo, framesizecode, samplesize;
    bool   channels;

    Skip_L1 (                                                   "unknown");
    BS_Begin();
    Get_S1 ( 2, num_samples_lo,                                 "num_samples (lo)");
    Get_S1 ( 3, framesizecode,                                  "framesizecode");
    Skip_S1( 2,                                                 "unknown");
    BS_End();
    Get_L4 (num_samples_hi,                                     "num_samples (hi)"); Param_Info((((int64u)num_samples_hi)<<2 | num_samples_lo), " samples");
    Get_L3 (samplerate,                                         "samplerate"); Param_Info((samplerate/16)+6000, " Hz");
    BS_Begin();
    Skip_S1( 4,                                                 "unknown");
    Get_SB (    channels,                                       "channels"); Param_Info(channels?"Stereo":"Mono");
    Get_S1 ( 2, samplesize,                                     "samplesize"); Param_Info(Tak_samplesize[samplesize]);
    Skip_SB(                                                    "unknown");
    BS_End();
    Skip_L3(                                                    "crc");

    FILLING_BEGIN()
        //Coherency
        if (samplerate==0)
            return;

        //Computing
        int64u Samples=((int64u)num_samples_hi)<<2 | num_samples_lo;
        int32u SamplingRate=(samplerate/16)+6000;

        //Filling
        File__Tags_Helper::Accept("TAK");

        File__Tags_Helper::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "TAK");
        Fill(Stream_Audio, 0, Audio_Codec, "TAK");
        Fill(Stream_Audio, 0, Audio_SamplingRate, SamplingRate);
        Fill(Stream_Audio, 0, Audio_Channel_s_, channels?2:1);
        if (Tak_samplesize[samplesize])
            Fill(Stream_Audio, 0, Audio_Resolution, Tak_samplesize[samplesize]);
        Fill(Stream_Audio, 0, Audio_Duration, Samples*1000/SamplingRate);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Tak::SEEKTABLE()
{
    //Parsing
    int16u num_seekpoints;
    Get_L2 (num_seekpoints,                                     "num_seekpoints");
    Skip_L1 (                                                   "unknown");
    Skip_L1 (                                                   "seek interval");
    Element_Begin("seekpoints");
    for (int16u Pos=0; Pos<num_seekpoints; Pos++)
        Skip_L5 (                                               "seekpoint");
    Element_End();
    Skip_L3(                                                    "crc");
}

//---------------------------------------------------------------------------
void File_Tak::WAVEMETADATA()
{
    //Parsing
    int32u HeaderLength, FooterLength;
    Get_L3 (HeaderLength,                                       "HeaderLength");
    Get_L3 (FooterLength,                                       "FooterLength");
    #if defined(MEDIAINFO_RIFF_YES)
        //Creating the parser
        File_Riff MI;
        Open_Buffer_Init(&MI);

        //Parsing
        Open_Buffer_Continue(&MI, HeaderLength);
        Element_Offset+=HeaderLength;

        //Filling
        //Finish(&MI);
        //Merge(MI, StreamKind_Last, 0, StreamPos_Last);

        //The RIFF header is for PCM
        //Clear(Stream_Audio, StreamPos_Last, Audio_ID);
        //Fill(Stream_Audio, StreamPos_Last, Audio_Format, "TAK", Unlimited, true, true);
        //Fill(Stream_Audio, StreamPos_Last, Audio_Codec, "TAK", Unlimited, true, true);
        //Clear(Stream_Audio, StreamPos_Last, Audio_CodecID);
        //Clear(Stream_Audio, StreamPos_Last, Audio_CodecID_Hint);
        //Clear(Stream_Audio, StreamPos_Last, Audio_CodecID_Url);
        //Clear(Stream_Audio, StreamPos_Last, Audio_BitRate);
        //Clear(Stream_Audio, StreamPos_Last, Audio_BitRate_Mode);
        //Clear(Stream_Audio, StreamPos_Last, Audio_Codec_CC);
    #else
        Skip_XX(HeaderLength,                                   "Wave header");
    #endif
    if (FooterLength)
        Skip_XX(FooterLength,                                   "Wave footer");
    Skip_L3(                                                    "crc");
}

//---------------------------------------------------------------------------
void File_Tak::ENCODERINFO()
{
    //Parsing
    int8u Revision, Minor, Major, Preset_hi, Preset_lo;
    Get_L1 (Revision,                                           "Revision");
    Get_L1 (Minor,                                              "Minor");
    Get_L1 (Major,                                              "Major");
    BS_Begin();
    Get_S1 (4, Preset_hi,                                       "Preset (hi)");
    Get_S1 (4, Preset_lo,                                       "Preset (lo)");
    BS_End();

    FILLING_BEGIN();
        Ztring Version=Ztring::ToZtring(Major)+_T('.')+Ztring::ToZtring(Minor)+_T('.')+Ztring::ToZtring(Revision);
        Ztring Preset=_T("-p")+Ztring::ToZtring(Preset_lo);
        switch (Preset_hi)
        {
            case 0x00 :                 break;
            case 0x01 : Preset+=_T('e'); break;
            case 0x02 : Preset+=_T('m'); break;
            default   : Preset+=_T('-')+Ztring::ToZtring(Preset_hi, 16); //Unknown
        }

        Fill(Stream_Audio, 0, Audio_Encoded_Library, "TAK");
        Fill(Stream_Audio, 0, Audio_Encoded_Library_String, _T("TAK ")+Version);
        Fill(Stream_Audio, 0, Audio_Encoded_Library_Name, "TAK");
        Fill(Stream_Audio, 0, Audio_Encoded_Library_Version, Version);
        Fill(Stream_Audio, 0, Audio_Encoded_Library_Settings, Preset);
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_TAK_YES
