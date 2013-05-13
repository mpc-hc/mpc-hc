/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Source: http://trac.musepack.net/trac/wiki/SV8Specification
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
#if defined(MEDIAINFO_MPCSV8_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_MpcSv8.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
extern const int16u Mpc_SampleFreq[];

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int32u AP=0x4150;
    const int32u CT=0x4354;
    const int32u EI=0x4549;
    const int32u RG=0x5247;
    const int32u SE=0x5345;
    const int32u SH=0x5348;
    const int32u SO=0x534F;
    const int32u ST=0x5354;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_MpcSv8::File_MpcSv8()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;
}


//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_MpcSv8::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    if (CC4(Buffer)!=0x4D50434B) //"MPCK"
    {
        File__Tags_Helper::Reject("Musepack SV8");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpcSv8::FileHeader_Parse()
{
    //Parsing
    Skip_C4(                                                    "Magic Number");

    FILLING_BEGIN();
        File__Tags_Helper::Accept("MpcSv8");

        File__Tags_Helper::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "Musepack SV8");
        Fill(Stream_Audio, 0, Audio_Codec, "SV8");
    FILLING_END();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_MpcSv8::Header_Begin()
{
    //Tags
    if (!File__Tags_Helper::Header_Begin())
        return false;

    return true;
}

//---------------------------------------------------------------------------
void File_MpcSv8::Header_Parse()
{
    //Parsing
    int64u Size;
    int16u Key;
    Get_C2 (Key,                                                "Key");
    Get_VS (Size,                                               "Size");

    //Filling
    Header_Fill_Code(Key, Ztring().From_CC4(Key<<16)); //Quick filling for CC2 with text
    Header_Fill_Size(Key==Elements::AP?Element_Offset:Size); //We don't need the data of audio packet, and we will stop here...
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpcSv8::Data_Parse()
{
    #define CASE_INFO(_NAME, _DETAIL) \
        case Elements::_NAME : Element_Info1(_DETAIL); _NAME(); break;

    //Parsing
    switch (Element_Code)
    {
        CASE_INFO(AP,                                           "Audio Packet");
        CASE_INFO(CT,                                           "Chapter-Tag");
        CASE_INFO(EI,                                           "Encoder Info");
        CASE_INFO(RG,                                           "Replay Gain");
        CASE_INFO(SE,                                           "Stream End");
        CASE_INFO(SH,                                           "Stream Header");
        CASE_INFO(SO,                                           "Seek Table Offset");
        CASE_INFO(ST,                                           "Seek Table");
        default : Skip_XX(Element_Size,                         "Data");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpcSv8::AP()
{
    //No more need data
    File__Tags_Helper::Finish("MpcSv8");
}

//---------------------------------------------------------------------------
void File_MpcSv8::EI()
{
    //Parsing
    int8u  Quality, Version1, Version2, Version3;
    bool   PNS;
    BS_Begin();
    Get_S1 (7, Quality,                                         "Quality");
    Get_SB (   PNS,                                             "PNS");
    BS_End();
    Get_B1 (Version1,                                           "Major version");
    Get_B1 (Version2,                                           "Minor version");
    Get_B1 (Version3,                                           "Build");
}

//---------------------------------------------------------------------------
void File_MpcSv8::RG()
{
    //Parsing
    int16u TitleGain, AlbumGain;
    Skip_B1(                                                    "Version");
    Get_L2 (TitleGain,                                          "Title gain"); Param_Info3(((float32)((int16s)TitleGain))/1000, 2, " dB");
    Skip_L2(                                                    "Title peak");
    Get_L2 (AlbumGain,                                          "Album gain"); Param_Info3(((float32)((int16s)TitleGain))/1000, 2, " dB");
    Skip_L2(                                                    "Album peak");
}

//---------------------------------------------------------------------------
void File_MpcSv8::SH()
{
    //Parsing
    int64u SampleCount;
    int8u  Version, SampleFrequency, ChannelCount;
    bool   MidSideStereo;
    Skip_B4(                                                    "CRC32");
    Get_B1 (Version,                                            "Version");
    Get_VS (SampleCount,                                        "Sample count");
    Skip_VS(                                                    "Beginning silence");
    BS_Begin();
    Get_S1 (3, SampleFrequency,                                 "Sample frequency"); Param_Info1(Mpc_SampleFreq[SampleFrequency]);
    Skip_S1(5,                                                  "Max used bands");
    Get_S1 (4, ChannelCount,                                    "Channel count");
    Get_SB (   MidSideStereo,                                   "Mid side stereo used");
    Skip_S1(3,                                                  "Audio block frames");
    BS_End();

    //Filling
    FILLING_BEGIN();
        Fill(Stream_Audio, 0, Audio_SamplingRate, Mpc_SampleFreq[SampleFrequency]);
        if (SampleCount)
        {
            Fill(Stream_Audio, 0, Audio_SamplingCount, SampleCount);
            Fill(Stream_Audio, 0, Audio_Duration, SampleCount*1000/Mpc_SampleFreq[SampleFrequency]);
            Fill(Stream_Audio, 0, Audio_BitRate, File_Size*8*Mpc_SampleFreq[SampleFrequency]/SampleCount); //Should be more precise...
        }
        Fill(Stream_Audio, 0, Audio_BitDepth, 16); //MPC support only 16 bits
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_MpcSv8::SO()
{
    //Parsing
    Skip_VS(                                                    "Offset");
}

} //NameSpace

#endif //MEDIAINFO_MPCSV8_YES

