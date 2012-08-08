// File_Pcm - Info for PCM files
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
#if defined(MEDIAINFO_PCM_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Pcm.h"
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

//---------------------------------------------------------------------------
int8u Pcm_M2TS_channel_assignment[16]=
{
        0,
        1,
        0,
        2,
        3,
        3,
        4,
        4,
        5,
        6,
        7,
        8,
        0,
        0,
        0,
        0,
};

//---------------------------------------------------------------------------
int32u Pcm_M2TS_sampling_frequency[16]=
{
        0,
    48000,
        0,
        0,
    96000,
   192000,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
};

//---------------------------------------------------------------------------
int8u Pcm_M2TS_bits_per_sample[4]=
{
        0,
       16,
       20,
       24,
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Pcm::File_Pcm()
{
    //Configuration
    ParserName=__T("PCM");
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    IsRawStream=true;
    PTS_DTS_Needed=true;

    //In
    BitDepth=0;
    Channels=0;
    IsRawPcm=true;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm::Streams_Fill()
{
    if (Count_Get(Stream_Audio)==0)
    {
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "PCM");
        Fill(Stream_Audio, 0, Audio_Codec, "PCM");
    }

    //Filling
    Ztring Firm, Endianness, Sign, ITU, Resolution;
         if (Codec==__T("EVOB"))             {Firm=__T("");       Endianness=__T("Big");    Sign=__T("Signed");}                        //PCM Signed 16 bits Big Endian, Interleavement is for 2 samples*2 channels L0-1/L0-0/R0-1/R0-0/L1-1/L1-0/R1-1/R1-0/L0-2/R0-2/L1-2/R1-2, http://wiki.multimedia.cx/index.php?title=PCM
    else if (Codec==__T("VOB"))              {Firm=__T("");       Endianness=__T("Big");    Sign=__T("Signed");}                        //PCM Signed 16 bits Big Endian, Interleavement is for 2 samples*2 channels L0-1/L0-0/R0-1/R0-0/L1-1/L1-0/R1-1/R1-0/L0-2/R0-2/L1-2/R1-2, http://wiki.multimedia.cx/index.php?title=PCM
    else if (Codec==__T("M2TS"))             {Firm=__T("");       Endianness=__T("Big");    Sign=__T("Signed");}                        //PCM Signed         Big Endian
    else if (Codec==__T("A_PCM/INT/BIG"))    {Firm=__T("");       Endianness=__T("Big");}
    else if (Codec==__T("A_PCM/INT/LITTLE")) {Firm=__T("");       Endianness=__T("Little");}
    else if (Codec==__T("A_PCM/INT/FLOAT"))  {Firm=__T("");       Endianness=__T("Big");    Sign=__T("Float");}
    else if (Codec==__T("fl32"))             {                   Endianness=__T("Big");    Sign=__T("Float");    Resolution=__T("32");}
    else if (Codec==__T("fl64"))             {                   Endianness=__T("Big");    Sign=__T("Float");    Resolution=__T("64");}
    else if (Codec==__T("in24"))             {                   Endianness=__T("Big");    Sign=__T("Unsigned"); Resolution=__T("24");}
    else if (Codec==__T("in32"))             {                   Endianness=__T("Big");    Sign=__T("Unsigned"); Resolution=__T("32");}
    else if (Codec==__T("lpcm"))             {                   Endianness=__T("Big");    Sign=__T("Unsigned");}
    else if (Codec==__T("raw "))             {                   Endianness=__T("Little"); Sign=__T("Unsigned");}
    else if (Codec==__T("twos"))             {                   Endianness=__T("Big");    Sign=__T("Signed");}
    else if (Codec==__T("sowt"))             {                   Endianness=__T("Little"); Sign=__T("Signed");}
    else if (Codec==__T("SWF ADPCM"))        {Firm=__T("SWF");}
    else if (Codec==__T("1"))                {   if (BitDepth)
                                                {
                                                    if (BitDepth>8)
                                                    {           Endianness=__T("Little"); Sign=__T("Signed");}
                                                    else
                                                    {                                    Sign=__T("Unsigned");}
                                                }
                                            }
    else if (Codec==__T("2"))                {Firm=__T("Microsoft");}
    else if (Codec==__T("3"))                {                   Endianness=__T("Float");}
    else if (Codec==__T("10"))               {Firm=__T("OKI");}
    else if (Codec==__T("11"))               {Firm=__T("Intel");}
    else if (Codec==__T("12"))               {Firm=__T("Mediaspace");}
    else if (Codec==__T("13"))               {Firm=__T("Sierra");}
    else if (Codec==__T("14"))               {Firm=__T("Antex");}
    else if (Codec==__T("17"))               {Firm=__T("Dialogic");}
    else if (Codec==__T("18"))               {Firm=__T("Mediavision");}
    else if (Codec==__T("20"))               {Firm=__T("Yamaha");}
    else if (Codec==__T("33"))               {Firm=__T("Antex");}
    else if (Codec==__T("36"))               {Firm=__T("DSP Solution");}
    else if (Codec==__T("38"))               {Firm=__T("Natural MicroSystems");}
    else if (Codec==__T("39"))               {Firm=__T("Crystal Semiconductor");}
    else if (Codec==__T("3B"))               {Firm=__T("Rockwell");}
    else if (Codec==__T("40"))               {Firm=__T("Antex Electronics");}
    else if (Codec==__T("42"))               {Firm=__T("IBM");}
    else if (Codec==__T("45"))               {Firm=__T("Microsoft"); ITU=__T("G.726");}
    else if (Codec==__T("64"))               {Firm=__T("Apicom"); ITU=__T("G.726");}
    else if (Codec==__T("65"))               {Firm=__T("Apicom"); ITU=__T("G.722");}
    else if (Codec==__T("85"))               {Firm=__T("DataFusion Systems"); ITU=__T("G.726");}
    else if (Codec==__T("8B"))               {Firm=__T("Infocom"); ITU=__T("G.721");}
    else if (Codec==__T("97"))               {Firm=__T("ZyXEL");}
    else if (Codec==__T("100"))              {Firm=__T("Rhetorex");}
    else if (Codec==__T("125"))              {Firm=__T("Sanyo");}
    else if (Codec==__T("140"))              {Firm=__T("Dictaphone"); ITU=__T("G.726");}
    else if (Codec==__T("170"))              {Firm=__T("Unisys");}
    else if (Codec==__T("175"))              {Firm=__T("SyCom"); ITU=__T("G.726");}
    else if (Codec==__T("178"))              {Firm=__T("Knownledge");}
    else if (Codec==__T("200"))              {Firm=__T("Creative");}
    else if (Codec==__T("210"))              {Firm=__T("Uher");}
    else if (Codec==__T("285"))              {Firm=__T("Norcom Voice Systems");}
    else if (Codec==__T("1001"))             {Firm=__T("Olivetti");}
    else if (Codec==__T("1C03"))             {Firm=__T("Lucent"); ITU=__T("G.723");}
    else if (Codec==__T("1C0C"))             {Firm=__T("Lucent"); ITU=__T("G.723");}
    else if (Codec==__T("4243"))             {ITU=__T("G.726");}
    else if (Codec==__T("A105"))             {ITU=__T("G.726");}
    else if (Codec==__T("A107"))             {ITU=__T("G.726");}

    Fill(Stream_Audio, 0, Audio_Codec_String, "PCM");
    Fill(Stream_Audio, 0, Audio_Codec_Family, "PCM");
    if (!Firm.empty())
    {
        Fill(Stream_Audio, 0, Audio_Format_Settings, Firm);
        Fill(Stream_Audio, 0, Audio_Format_Settings_Firm, Firm);
        Fill(Stream_Audio, 0, Audio_Codec_Settings, Firm);
        Fill(Stream_Audio, 0, Audio_Codec_Settings_Firm, Firm);
    }
    if (!Endianness.empty())
    {
        Fill(Stream_Audio, 0, Audio_Format_Settings, Endianness);
        Fill(Stream_Audio, 0, Audio_Format_Settings_Endianness, Endianness);
        Fill(Stream_Audio, 0, Audio_Codec_Settings, Endianness);
        Fill(Stream_Audio, 0, Audio_Codec_Settings_Endianness, Endianness);
    }
    if (!Sign.empty())
    {
        Fill(Stream_Audio, 0, Audio_Format_Settings, Sign);
        Fill(Stream_Audio, 0, Audio_Format_Settings_Sign, Sign);
        Fill(Stream_Audio, 0, Audio_Codec_Settings, Sign);
        Fill(Stream_Audio, 0, Audio_Codec_Settings_Sign, Sign);
    }
    if (!ITU.empty())
    {
        Fill(Stream_Audio, 0, Audio_Format_Settings, ITU);
        Fill(Stream_Audio, 0, Audio_Format_Settings_ITU, ITU);
        Fill(Stream_Audio, 0, Audio_Codec_Settings, ITU);
        Fill(Stream_Audio, 0, Audio_Codec_Settings_ITU, ITU);
    }
    if (!Resolution.empty())
        Fill(Stream_Audio, 0, Audio_BitDepth, Resolution);
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR", Unlimited, true, true);
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm::Read_Buffer_Continue()
{
    if (!Status[IsAccepted] && IsRawPcm) //No need of data
    {
        Accept();
        Finish();
    }
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm::Header_Parse()
{
    //Filling
    Header_Fill_Code(0, "Block");
    #if MEDIAINFO_DEMUX
        if (Demux_UnpacketizeContainer && IsRawPcm && BitDepth && Channels)
            Header_Fill_Size((Element_Size/(BitDepth*Channels/8))*(BitDepth*Channels/8)); //A complete sample
        else
    #endif //MEDIAINFO_DEMUX
            Header_Fill_Size(Element_Size);
}

//---------------------------------------------------------------------------
void File_Pcm::Data_Parse()
{
    Accept();

    //Parsing
    if (Codec==__T("VOB") || Codec==__T("EVOB"))
        VOB();
    else if (Codec==__T("M2TS"))
        M2TS();
    else
    {
        #if MEDIAINFO_DEMUX
            if (Demux_UnpacketizeContainer && Demux_TotalBytes<=Buffer_TotalBytes+Buffer_Offset)
            {
                Demux_Offset=(size_t)Element_Size;
                Demux_UnpacketizeContainer_Demux();
            }
        #endif //MEDIAINFO_DEMUX
        Skip_XX(Element_Size,                                   "Data"); //It is impossible to detect... Default is no detection, only filling

        Accept();
    }

    Finish();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm::VOB()
{
    //Parsing
    int8u BitDepth, Frequency, NumberOfChannelsMinusOne;
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
        if (Count_Get(Stream_Audio)==0)
        {
            Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, 0, Audio_Format, "PCM");
            Fill(Stream_Audio, 0, Audio_Codec, "PCM");
            Fill(Stream_Audio, 0, Audio_BitDepth, Pcm_VOB_BitDepth[BitDepth]);
            Fill(Stream_Audio, 0, Audio_SamplingRate, Pcm_VOB_Frequency[Frequency]);
            Fill(Stream_Audio, 0, Audio_Channel_s_, NumberOfChannelsMinusOne+1);
            Fill(Stream_Audio, 0, Audio_ChannelPositions, Pcm_VOB_ChannelsPositions(NumberOfChannelsMinusOne+1));
            Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Pcm_VOB_ChannelsPositions2(NumberOfChannelsMinusOne+1));
            Fill(Stream_Audio, 0, Audio_BitRate, Pcm_VOB_Frequency[Frequency]*(NumberOfChannelsMinusOne+1)*16);
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR", Unlimited, true, true);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Pcm::M2TS()
{
    //Parsing
    int16u  audio_data_payload_size;
    int8u   channel_assignment, sampling_frequency, bits_per_sample;
    Get_B2 (   audio_data_payload_size,                         "audio_data_payload_size");
    BS_Begin();
    Get_S1 (4, channel_assignment,                              "channel_assignment"); Param_Info2(Pcm_M2TS_channel_assignment[channel_assignment], " channel(s)");
    Get_S1 (4, sampling_frequency,                              "sampling_frequency"); Param_Info2(Pcm_M2TS_sampling_frequency[sampling_frequency], " Hz");
    Get_S1 (2, bits_per_sample,                                 "bits_per_sample"); Param_Info2(Pcm_M2TS_bits_per_sample[bits_per_sample], " bits");
    Skip_SB(                                                    "start_flag");
    Skip_S1(5,                                                  "reserved");
    BS_End();
    Skip_XX(audio_data_payload_size,                            "audio_data_payload");

    FILLING_BEGIN_PRECISE();
        Accept("PCM");

        if (Count_Get(Stream_Audio)==0)
        {
            int8u Channels=Pcm_M2TS_channel_assignment[channel_assignment];
            Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, 0, Audio_Format, "PCM");
            Fill(Stream_Audio, 0, Audio_Codec, "PCM");
            Fill(Stream_Audio, 0, Audio_MuxingMode, "Blu-ray");
            if (Channels)
            {
                if (Pcm_M2TS_sampling_frequency[sampling_frequency])
                    Fill(Stream_Audio, 0, Audio_SamplingRate, Pcm_M2TS_sampling_frequency[sampling_frequency]);
                if (Pcm_M2TS_bits_per_sample[bits_per_sample])
                    Fill(Stream_Audio, 0, Audio_BitDepth, Pcm_M2TS_bits_per_sample[bits_per_sample]);
                Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
                Fill(Stream_Audio, 0, Audio_ChannelPositions, Pcm_VOB_ChannelsPositions(channel_assignment));
                Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Pcm_VOB_ChannelsPositions2(channel_assignment));
                if (Pcm_M2TS_sampling_frequency[sampling_frequency] && Pcm_M2TS_bits_per_sample[bits_per_sample])
                {
                    if (Channels%2)
                        Fill(Stream_Audio, 0, Audio_BitRate_Encoded, Pcm_M2TS_sampling_frequency[sampling_frequency]*(Channels+1)*Pcm_M2TS_bits_per_sample[bits_per_sample]); //Always by pair
                    Fill(Stream_Audio, 0, Audio_BitRate, Pcm_M2TS_sampling_frequency[sampling_frequency]*Channels*Pcm_M2TS_bits_per_sample[bits_per_sample]);
                }
            }
        }

        Finish("PCM");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_PCM_YES
