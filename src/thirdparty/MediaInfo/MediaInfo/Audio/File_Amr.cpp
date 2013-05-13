/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
#if defined(MEDIAINFO_AMR_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Amr.h"
#include "ZenLib/Utils.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

int16u Amr_BitRate[]=
{
     5200,
     5600,
     6400,
     7200,
     8000,
     8400,
    10800,
    12800,
     3600,
     3600,
     3600,
     3600,
        0,
        0,
        0,
      400,
};

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
File_Amr::File_Amr()
:File__Analyze()
{
    //Temp
    Header_Size=(int64u)-1;
    Frame_Number=0;
    FrameType=(int8u)-1;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Amr::FileHeader_Begin()
{
    if (!Codec.empty()) //Test of header only if it is a file --> The codec field is empty
        return true;

    //Testing
    if (Buffer_Size<5)
        return false; //Must wait for more data
    if (CC5(Buffer)!=0x2321414D52LL) //"#!AMR"
    {
        Reject("AMR");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Amr::Streams_Fill()
{
    Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Format, "AMR");
    Fill(Stream_Audio, 0, Audio_Codec, "AMR");
    if (!Codec.empty())
    {
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Codec, InfoCodecID_Profile)==__T("Narrow band"))
            IsWB=false;
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Codec, InfoCodecID_Profile)==__T("Wide band"))
            IsWB=true;
        Channels=1;
    }
    if (Channels==0)
        return; //No more info

    if (Header_Size!=(int64u)-1)
        Fill(Stream_General, 0, General_StreamSize, Header_Size);
    Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
    if (IsWB)
    {
        Fill(Stream_Audio, 0, Audio_Format_Profile, "Wide band");
        Fill(Stream_Audio, 0, Audio_Codec, "sawb", Unlimited, true, true);
        if (Codec.empty()) //If there is a container, trusting the container sampling rate
            Fill(Stream_Audio, 0, Audio_SamplingRate, 16000);
        Fill(Stream_Audio, 0, Audio_BitDepth, 14);
        //Fill(Stream_Audio, 0, Audio_InternetMediaType, "audio/AMR-WB", Unlimited, true, true);
    }
    else
    {
        Fill(Stream_Audio, 0, Audio_Format_Profile, "Narrow band");
        Fill(Stream_Audio, 0, Audio_Codec, "samr", Unlimited, true, true);
        if (Codec.empty()) //If there is a container, trusting the container sampling rate
            Fill(Stream_Audio, 0, Audio_SamplingRate, 8000);
        Fill(Stream_Audio, 0, Audio_BitDepth, 13);
        if (FrameType!=(int8u)-1 && Amr_BitRate[FrameType] && FrameTypes.size()==1)
        {
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
            Fill(Stream_Audio, 0, Audio_BitRate, Amr_BitRate[FrameType]);
            Fill(Stream_General, 0, General_OverallBitRate, Amr_BitRate[FrameType]);
            if (File_Size!=(int64u)-1)
            {
                Fill(Stream_Audio, 0, Audio_Duration, ((float32)(File_Size-Header_Size))*8*1000/Amr_BitRate[FrameType]);
            }
        }
    }
}

//---------------------------------------------------------------------------
void File_Amr::Streams_Finish()
{
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Amr::FileHeader_Parse()
{
    //From a container
    if (!Codec.empty())
    {
        Accept("AMR");
        Finish("AMR");
        return;
    }

    //Parsing
    int64u Signature;
    Skip_C5(                                                    "Signature (Common)");
    Peek_B8(Signature);
    if ((Signature&0xFF00000000000000LL)==0x0A00000000000000LL) //\n
    {
        IsWB=false;
        Channels=1;
    }
    else if ((Signature&0xFFFFFFFFFFFFFF00LL)==0x5F4D43312E300A00LL) //_MC1.0\n
    {
        IsWB=false;
        Channels=2; //Or more, see later
    }
    else if ((Signature&0xFFFFFF0000000000LL)==0x2D57420000000000LL) //-WB
    {
        Skip_C3(                                              "Signature (WB)");
        IsWB=true;

        Peek_B8(Signature);
        if ((Signature&0xFF00000000000000LL)==0x0A00000000000000LL) //\n
        {
            Channels=1;
        }
        else if ((Signature&0xFFFFFFFFFFFFFF00LL)==0x5F4D43312E300A00LL) //_MC1.0\n
        {
            Channels=2; //Or more, see later
        }
    }
    else
        Channels=0;
    Skip_B1(                                                    "Signature (Carriage return)");
    /*
    if (Channels==2) //Mutli-channels
    {
        BS_Begin();
        Skip_S4(28,                                             "Reserved");
        Get_S1 ( 4, Channels,                                   "Channels");
    }
    */
    Header_Size=(int8u)Element_Offset;


    FILLING_BEGIN();
        Accept("AMR");

        if (Channels!=1 || IsWB)
            Finish("AMR");
    FILLING_END();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Amr::Header_Parse()
{
    BS_Begin();
    Skip_SB(                                                "Frame Following");
    Get_S1 ( 4, FrameType,                                  "Frame Type");
    Skip_SB(                                                "Frame Quality");
    Skip_SB(                                                "Unknown");
    Skip_SB(                                                "Unknown");
    BS_End();

    //Filling
    if (Amr_BitRate[FrameType]==0)
    {
        Finish("AMR");
        return;
    }
    Header_Fill_Size(Amr_BitRate[FrameType]/400);
    Header_Fill_Code(0, "frame");
}

//---------------------------------------------------------------------------
void File_Amr::Data_Parse()
{
    Element_Info1(Frame_Number);

    //Parsing
    Skip_XX(Element_Size,                                       "Data");


    FILLING_BEGIN();
        Frame_Number++;
        FrameTypes[FrameType]++;
        if (Frame_Number>=32)
            Finish("AMR");
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_AMR_YES

