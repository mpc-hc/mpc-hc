// File_Pcm_Vob - Info for PCM (from DVD) files
// Copyright (C) 2007-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
#if defined(MEDIAINFO_PCMVOB_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Pcm_Vob.h"
#if MEDIAINFO_DEMUX
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#endif //MEDIAINFO_DEMUX
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
int32u Pcm_VOB_BitDepth[]=
{
    16,
    20,
    24,
     0,
};

//---------------------------------------------------------------------------
int32u Pcm_VOB_Frequency[]=
{
    48000,
    32000,
        0,
        0,
};

//---------------------------------------------------------------------------
const char* Pcm_VOB_ChannelsPositions(int8u channel_assignment)
{
    switch (channel_assignment)
    {
        case  1 : return "Front: C";                                    //1 channel
        case  3 : return "Front: L R";                                  //2 channels
        case  4 : return "Front: L C R";                                //3 channels
        case  5 : return "Front: L R, LFE";                             //3 channels
        case  6 : return "Front: L C R, LFE";                           //4 channels
        case  7 : return "Front: L R, Side: L R";                       //4 channels
        case  8 : return "Front: L C R, Side: L R";                     //5 channels
        case  9 : return "Front: L C R, Side: L R, LFE";                //6 channels
        case 10 : return "Front: L C R, Side: L R, Back: L R";          //7 channels
        case 11 : return "Front: L C R, Side: L R, Back: L R, LFE";     //8 channels
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Pcm_VOB_ChannelsPositions2(int8u channel_assignment)
{
    switch (channel_assignment)
    {
        case  1 : return "1/0/0.0";                                     //1 channel
        case  3 : return "2/0/0.0";                                     //2 channels
        case  4 : return "3/0/0.0";                                     //3 channels
        case  5 : return "2/0/0.1";                                     //3 channels
        case  6 : return "3/0/0.1";                                     //4 channels
        case  7 : return "2/2/0.0";                                     //4 channels
        case  8 : return "3/2/0.0";                                     //5 channels
        case  9 : return "3/2/0.1";                                     //6 channels
        case 10 : return "3/2/2.0";                                     //7 channels
        case 11 : return "3/2/2.1";                                     //8 channels
        default : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Pcm_Vob::File_Pcm_Vob()
{
    //Configuration
    ParserName=__T("PCM VOB");
    IsRawStream=true;
    PTS_DTS_Needed=true;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm_Vob::Streams_Fill()
{
    Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Format, "PCM");
    Fill(Stream_Audio, 0, Audio_Codec, "PCM");
    Fill(Stream_Audio, 0, Audio_Codec_Family, "PCM");
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");

    Fill(Stream_Audio, 0, Audio_BitDepth, Pcm_VOB_BitDepth[BitDepth]);
    Fill(Stream_Audio, 0, Audio_SamplingRate, Pcm_VOB_Frequency[Frequency]);
    Fill(Stream_Audio, 0, Audio_Channel_s_, NumberOfChannelsMinusOne+1);
    Fill(Stream_Audio, 0, Audio_ChannelPositions, Pcm_VOB_ChannelsPositions(NumberOfChannelsMinusOne+1));
    Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Pcm_VOB_ChannelsPositions2(NumberOfChannelsMinusOne+1));
    Fill(Stream_Audio, 0, Audio_BitRate, Pcm_VOB_Frequency[Frequency]*(NumberOfChannelsMinusOne+1)*16);

    //PCM Signed 16 bits Big Endian, Interleavement is for 2 samples*2 channels L0-1/L0-0/R0-1/R0-0/L1-1/L1-0/R1-1/R1-0/L0-2/R0-2/L1-2/R1-2, http://wiki.multimedia.cx/index.php?title=PCM
    Fill(Stream_Audio, 0, Audio_Format_Settings, "Big");
    Fill(Stream_Audio, 0, Audio_Format_Settings_Endianness, "Big");
    Fill(Stream_Audio, 0, Audio_Codec_Settings, "Big");
    Fill(Stream_Audio, 0, Audio_Codec_Settings_Endianness, "Big");
    Fill(Stream_Audio, 0, Audio_Format_Settings, "Signed");
    Fill(Stream_Audio, 0, Audio_Format_Settings_Sign, "Signed");
    Fill(Stream_Audio, 0, Audio_Codec_Settings, "Signed");
    Fill(Stream_Audio, 0, Audio_Codec_Settings_Sign, "Signed");
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm_Vob::Read_Buffer_Continue()
{
    if (Buffer_Size==0)
        return;

    //Parsing
    Skip_B1(                                                    "Frame number");
    Skip_B2(                                                    "Bytes to skip (+1?)");
    Skip_B1(                                                    "Unknown");
    BS_Begin();
    Get_S1 (2, BitDepth,                                        "Bit depth"); Param_Info1(Pcm_VOB_BitDepth[BitDepth]);
    Get_S1 (2, Frequency,                                       "Frequency"); Param_Info1(Pcm_VOB_Frequency[Frequency]);
    Skip_SB(                                                    "Unknown");
    Get_S1 (3, NumberOfChannelsMinusOne,                        "Number of channels (minus 1)");
    BS_End();
    Skip_B1(                                                    "Start code");

    #if MEDIAINFO_DEMUX
        if (Config->Demux_PCM_20bitTo16bit_Get() && BitDepth==1) //20-bit
        {
            int8u* Info=new int8u[(size_t)((Element_Size-6)*4/5)];
            size_t Info_Offset=0;

            while (Element_Offset+5*(NumberOfChannelsMinusOne+1)<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                std::memcpy(Info+Info_Offset, Buffer+Buffer_Pos, 4*(NumberOfChannelsMinusOne+1));

                Info_Offset+=4*(NumberOfChannelsMinusOne+1);
                Element_Offset+=5*(NumberOfChannelsMinusOne+1);
            }
            Element_Offset=6;

            FrameInfo.PTS=FrameInfo.DTS;
            if (Pcm_VOB_Frequency[Frequency])
                FrameInfo.DUR=(Element_Size-6)/5*1000000000/Pcm_VOB_Frequency[Frequency];
            Demux_random_access=true;
            Element_Code=(int64u)-1;
            Demux(Info, Info_Offset, ContentType_MainStream);

            delete[] Info;

            Skip_XX(Element_Size-6,                             "Data");
        }
        else
        {
            Demux_Offset=Buffer_Offset+(size_t)Element_Size;
            Buffer_Offset+=6; //Header is dropped
            Demux_UnpacketizeContainer_Demux();
            Buffer_Offset-=6;
        }
    #endif //MEDIAINFO_DEMUX

    Skip_XX(Element_Size-6,                                     "Data");

    FILLING_BEGIN();
        Frame_Count++;
        Frame_Count_InThisBlock++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
        if (FrameInfo.DTS!=(int64u)-1 && FrameInfo.DUR!=(int64u)-1)
        {
            FrameInfo.DTS+=FrameInfo.DUR;
            FrameInfo.PTS=FrameInfo.DTS;
        }

        if (!Status[IsAccepted])
        {
            Accept();
            Finish();
        }
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_PCMVOB_YES
