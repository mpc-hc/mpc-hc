// File_Adts - Info for AAC (ADTS) files
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
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

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_ADTS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Adts.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const int32u ADTS_SamplingRate[]=
{96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
 16000, 12000, 11025,  8000,  7350,     0,     0,     0,};

//---------------------------------------------------------------------------
const char* ADTS_ID[]=
{
    "MPEG-4",
    "MPEG-2",
};

//---------------------------------------------------------------------------
const char* ADTS_Format_Profile[]=
{
    "Main",
    "LC",
    "SSR",
    "LTP",
};

//---------------------------------------------------------------------------
const char* ADTS_Profile[]=
{
    "A_AAC/MPEG4/MAIN",
    "A_AAC/MPEG4/LC",
    "A_AAC/MPEG4/SSR",
    "A_AAC/MPEG4/LTP",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Adts::File_Adts()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;

    //Configuration
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;

    //In
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.5?128:(MediaInfoLib::Config.ParseSpeed_Get()>=0.3?32:2);

    //Temp
    Frame_Count=0;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Adts::Streams_Fill()
{
    //Calculating
    bool sbrPresentFlag=ADTS_SamplingRate[sampling_frequency_index]<=24000;
    bool psPresentFlag=channel_configuration<=1; //1 channel
    int64u BitRate=0;
    if (!aac_frame_lengths.empty())
    {
        int64u aac_frame_length_Total=0;
        for (size_t Pos=0; Pos<aac_frame_lengths.size(); Pos++)
            aac_frame_length_Total+=aac_frame_lengths[Pos];

        BitRate=(ADTS_SamplingRate[sampling_frequency_index]/1024);
        BitRate*=aac_frame_length_Total*8;
        BitRate/=aac_frame_lengths.size();
    }

    if (!IsSub)
        Fill(Stream_General, 0, General_Format, "ADTS");

    File__Tags_Helper::Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Format, "AAC");
    Fill(Stream_Audio, 0, Audio_Format_Version, id?"Version 2":"Version 4");
    Fill(Stream_Audio, 0, Audio_Format_Profile, ADTS_Format_Profile[profile_ObjectType]);
    if (!sbrPresentFlag && !psPresentFlag)
        Fill(Stream_Audio, 0, Audio_Codec, ADTS_Profile[profile_ObjectType]);
    Fill(Stream_Audio, 0, Audio_SamplingRate, ADTS_SamplingRate[sampling_frequency_index]*(sbrPresentFlag?2:1));
    Fill(Stream_Audio, 0, Audio_Channel_s_, psPresentFlag?2:channel_configuration);
    Fill(Stream_Audio, 0, Audio_MuxingMode, "ADTS");
    if (adts_buffer_fullness==0x7FF)
        Fill(Stream_Audio, 0, Audio_BitRate_Mode, "VBR");
    else
    {
        Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
        if (BitRate)
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
    }
    if (sbrPresentFlag)
    {
        Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings, "SBR");
        Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_SBR, "Yes", Unlimited, true, true);
        Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_PS, "No");
        Ztring Codec=Ztring().From_Local(ADTS_Profile[profile_ObjectType])+_T("/SBR");
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, Codec, true);
    }
    if (psPresentFlag)
    {
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, 2, 10, true);
        Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings, "PS");
        Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_PS, "Yes", Unlimited, true, true);
        Ztring Codec=Ztring().From_Local(ADTS_Profile[profile_ObjectType])+(sbrPresentFlag?_T("/SBR"):_T(""))+_T("/PS");
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, Codec, true);
    }

    File__Tags_Helper::Streams_Fill();
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Adts::Synchronize()
{
    //Tags
    bool Tag_Found;
    if (!File__Tags_Helper::Synchronize(Tag_Found))
        return false;
    if (Tag_Found)
        return true;

    //Synchronizing
    while (Buffer_Offset+6<=Buffer_Size)
    {
         while (Buffer_Offset+6<=Buffer_Size
             && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;

        if (Buffer_Offset+6<=Buffer_Size)//Testing if size is coherant
        {
            //Testing next start, to be sure
            int16u aac_frame_length=(CC3(Buffer+Buffer_Offset+3)>>5)&0x1FFF;
            if (IsSub && Buffer_Offset+aac_frame_length==Buffer_Size)
                break;
            if (File_Offset+Buffer_Offset+aac_frame_length!=File_Size-File_EndTagSize)
            {
                if (Buffer_Offset+aac_frame_length+2>Buffer_Size)
                    return false; //Need more data

                //Testing
                if (aac_frame_length<=7 || (CC2(Buffer+Buffer_Offset+aac_frame_length)&0xFFF6)!=0xFFF0)
                    Buffer_Offset++;
                else
                {
                    //Testing next start, to be sure
                    int16u aac_frame_length2=(CC3(Buffer+Buffer_Offset+aac_frame_length+3)>>5)&0x1FFF;
                    if (File_Offset+Buffer_Offset+aac_frame_length+aac_frame_length2!=File_Size-File_EndTagSize)
                    {
                        if (Buffer_Offset+aac_frame_length+aac_frame_length2+2>Buffer_Size)
                            return false; //Need more data

                        //Testing
                        if (aac_frame_length<=7 || (CC2(Buffer+Buffer_Offset+aac_frame_length+aac_frame_length2)&0xFFF6)!=0xFFF0)
                            Buffer_Offset++;
                        else
                            break; //while()
                    }
                    else
                        break; //while()
                }
            }
            else
                break; //while()
        }
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+6>Buffer_Size)
    {
        if (Buffer_Offset+5==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;
        if (Buffer_Offset+4==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;
        if (Buffer_Offset+3==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;
        if (Buffer_Offset+2==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;
        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0xFF)
            Buffer_Offset++;
        return false;
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Adts::Synched_Test()
{
    //Tags
    if (!File__Tags_Helper::Synched_Test())
        return false;

    //Must have enough buffer for having header
    if (Buffer_Offset+2>Buffer_Size)
        return false;

    //Quick test of synchro
    if ((CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
        Synched=false;

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Adts::Header_Parse()
{
    //Parsing
    bool protection_absent;
    BS_Begin();
    Skip_BS(12,                                                 "syncword");
    Get_SB (    id,                                             "id"); Param_Info(ADTS_ID[id]);
    Skip_BS( 2,                                                 "layer");
    Get_SB (    protection_absent,                              "protection_absent");
    Get_S1 ( 2, profile_ObjectType,                             "profile_ObjectType"); Param_Info(ADTS_Profile[profile_ObjectType]);
    Get_S1 ( 4, sampling_frequency_index,                       "sampling_frequency_index"); Param_Info(ADTS_SamplingRate[sampling_frequency_index], " Hz");
    Skip_SB(                                                    "private");
    Get_S1 ( 3, channel_configuration,                          "channel_configuration");
    Skip_SB(                                                    "original");
    Skip_SB(                                                    "home");
    Skip_SB(                                                    "copyright_id");
    Skip_SB(                                                    "copyright_id_start");
    Get_S2 (13, aac_frame_length,                               "aac_frame_length");
    Get_S2 (11, adts_buffer_fullness,                           "adts_buffer_fullness"); Param_Info(adts_buffer_fullness==0x7FF?"VBR":"CBR");
    Skip_BS( 2,                                                 "num_raw_data_blocks");
    BS_End();

    //Filling
    Header_Fill_Size(aac_frame_length);
    Header_Fill_Code(0, "Frame");
}

//---------------------------------------------------------------------------
void File_Adts::Data_Parse()
{
    //Counting
    if (File_Offset+Buffer_Offset+Element_Size==File_Size)
        Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames
    Frame_Count++;

    //Name
    Element_Info(Ztring::ToZtring(Frame_Count));

    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    //Filling
    aac_frame_lengths.push_back(aac_frame_length);
    if (!Status[IsAccepted] && Frame_Count>=Frame_Count_Valid)
    {
        //Filling
        File__Tags_Helper::Accept("ADTS");

        Fill("ADTS");

        //No more need data
        File__Tags_Helper::Finish("ADTS");
    }
}

} //NameSpace

#endif //MEDIAINFO_ADTS_YES

