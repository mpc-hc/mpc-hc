/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about OpenMG (OMA) files
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
#if defined(MEDIAINFO_OPENMG_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_OpenMG.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
const char* OpenMG_CodecID_Format (int8u CodecID)
{
    switch (CodecID)
    {
        case  0 :
        case  1 :
                    return "Atrac3";
        case  3 :   return "MPEG Audio";
        case  4 :   return "PCM";
        case  5 :   return "WMA";
        default :   return "";
    }
}

//---------------------------------------------------------------------------
const char* OpenMG_CodecID_Encryption (int8u CodecID)
{
    switch (CodecID)
    {
        case  1 :   return "SDMI";
        default :   return "";
    }
}

//---------------------------------------------------------------------------
int32u OpenMG_SamplingRate (int8u SamplingRate_Code)
{
    switch (SamplingRate_Code)
    {
        case   0 : return 32000;
        case   1 : return 44100;
        case   2 : return 44800;
        case   3 : return 88200;
        case   4 : return 96000;
        default  : return 0;
    }
}

//---------------------------------------------------------------------------
int8u OpenMG_Channels (int8u Channels_Code)
{
    if (Channels_Code<=4)
        return Channels_Code;
    else
        return Channels_Code+1; //+LFE
}

//---------------------------------------------------------------------------
const char* OpenMG_ChannelPositions (int8u Channels_Code)
{
    switch (Channels_Code)
    {
        case  1 :   return "Front: C";
        case  2 :   return "Front: L R";
        case  3 :   return "Front: L R, Side: C";
        case  4 :   return "Front: L R, Back: L R";
        case  5 :   return "Front: L C R, Side: L R, LFE";
        case  6 :   return "Front: L C R, Side: L R, Back: C, LFE";
        case  7 :   return "Front: L C R, Side: L R, Back: L R, LFE";
        default :   return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_OpenMG::File_OpenMG()
{
    //File__Tags_Helper
    Base=this;

    //Config
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    IsRawStream=true;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_OpenMG::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format, "OpenMG");

    File__Tags_Helper::Stream_Prepare(Stream_Audio);

    File__Tags_Helper::Streams_Fill();
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_OpenMG::FileHeader_Begin()
{
    if (!File__Tags_Helper::FileHeader_Begin())
        return false;

    //Synchro
    if (Buffer_Offset+3>Buffer_Size)
        return false;

    // Testing
    if (Buffer[Buffer_Offset  ]!=0x45 // "EA3"
     || Buffer[Buffer_Offset+1]!=0x41
     || Buffer[Buffer_Offset+2]!=0x33)
    {
        File__Tags_Helper::Reject();
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void File_OpenMG::FileHeader_Parse()
{
    //Parsing
    int16u Size, FrameSize=0;
    int8u  Flags, CodecID, SamplingRate_Code=0, Channels_Code=0;
    bool  JointStereo=false;
    Skip_C3(                                                    "Code");
    Get_B1 (Flags,                                              "Flags");
    Get_B2 (Size,                                               "Size");
    Skip_XX(26,                                                 "Unknown");
    Get_B1 (CodecID,                                            "Coded ID"); Param_Info1(OpenMG_CodecID_Format(CodecID));
    if (CodecID<=1) //Atrac3
    {
        BS_Begin();
        Skip_S1(7,                                              "Unknown");
        Get_SB (   JointStereo,                                 "Joint Stereo");
        Get_S1 (3, SamplingRate_Code,                           "Sampling Rate"); Param_Info2(OpenMG_SamplingRate(SamplingRate_Code), " Hz");
        Get_S1 (3, Channels_Code,                               "Channels"); Param_Info2(OpenMG_Channels(Channels_Code), " channel(s)");
        Get_S2 (10, FrameSize,                                  "Frame size");
        BS_End();
    }
    Skip_XX(Size-Element_Offset,                                "Unknown");

    FILLING_BEGIN();
        if (!Status[IsAccepted])
        {
            File__Tags_Helper::Accept();

            Fill(Stream_Audio, 0, Audio_Format, OpenMG_CodecID_Format(CodecID));
            Fill(Stream_Audio, 0, Audio_Encryption, OpenMG_CodecID_Encryption(CodecID));
            int64u StreamSize=(int64u)-1;
            if (File_Size!=(int64u)-1)
            {
                StreamSize=File_Size-(Buffer_Offset+Element_Size);
                Fill(Stream_Audio, 0, Audio_StreamSize, StreamSize);
            }
            if (CodecID<=1) // Atrac3
            {
                Fill(Stream_Audio, 0, Audio_Channel_s_, OpenMG_Channels(Channels_Code));
                Fill(Stream_Audio, 0, Audio_ChannelPositions, OpenMG_ChannelPositions(Channels_Code));
                if (Channels_Code==1 && JointStereo)
                    Fill(Stream_Audio, 0, Audio_Format_Settings_Mode, "Joint Stereo");
                Fill(Stream_Audio, 0, Audio_SamplingRate, OpenMG_SamplingRate(SamplingRate_Code));

                if (CodecID==1) //Protected
                    FrameSize++; //Not sure
                FrameSize<<=3; //8-byte  blocks
                int64u BitRate=OpenMG_SamplingRate(SamplingRate_Code)*FrameSize/256;
                Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
                if (StreamSize!=(int64u)-1 && BitRate)
                    Fill(Stream_Audio, 0, Audio_Duration, StreamSize*8*1000/BitRate);
            }
        }
    FILLING_END();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_OpenMG::Read_Buffer_Continue()
{
    //Parsing
    Skip_XX(File_Size-Buffer_Offset,                            "Data");
    
    File__Analyze::Finish();
}

} //NameSpace

#endif
