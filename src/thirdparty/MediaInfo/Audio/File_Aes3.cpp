// File_Aes3 - Info for AES3 packetized streams
// Copyright (C) 2008-2010 MediaArea.net SARL, Info@MediaArea.net
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
// AES3 PCM and non-PCM (SMPTE 337M)
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
#if defined(MEDIAINFO_AES3_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Aes3.h"
#if defined(MEDIAINFO_DOLBYE_YES)
    #include "MediaInfo/Audio/File_DolbyE.h"
#endif
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Aes3_ChannelsPositions(int8u number_channels)
{
    switch (number_channels)
    {
        case  0 : return "Front: L R";                                  //2 channels
        case  1 : return "Front: L C R, LFE";                           //4 channels
        case  2 : return "Front: L C R, Side: L R, LFE";                //6 channels
        case  3 : return "Front: L C R, Side: L R, Back: L R, LFE";     //8 channels
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Aes3_ChannelsPositions2(int8u number_channels)
{
    switch (number_channels)
    {
        case  0 : return "2/0/0.0";                                     //2 channels
        case  1 : return "3/0/0.1";                                     //4 channels
        case  2 : return "3/2/0.1";                                     //6 channels
        case  3 : return "3/2/2.1";                                     //8 channels
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Aes3_NonPCM_data_type[32]= //SMPTE 338M
{
    "",
    "AC-3",
    "Time stamp",
    "",
    "MPEG Audio",
    "MPEG Audio",
    "MPEG Audio",
    "",
    "MPEG Audio",
    "MPEG Audio",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "KLV",
    "Dolby E",
    "Captioning",
    "",
    "",
};

//---------------------------------------------------------------------------
stream_t Aes3_NonPCM_data_type_StreamKind[32]= //SMPTE 338M
{
    Stream_Max,
    Stream_Audio,
    Stream_Max,
    Stream_Max,
    Stream_Audio,
    Stream_Audio,
    Stream_Audio,
    Stream_Max,
    Stream_Audio,
    Stream_Audio,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Menu,
    Stream_Audio,
    Stream_Text,
    Stream_Max,
    Stream_Max,
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Aes3::File_Aes3()
:File__Analyze()
{
    //Configuration
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=32*1024;
    PTS_DTS_Needed=true;

    //In
    From_MpegPs=false;

    //Temp
    Frame_Count=0;
    Frame_Last_PTS=(int32u)-1;
    Frame_Last_Size=(int64u)-1;
    data_type=(int8u)-1;
    IsParsingNonPcm=false;

    //Parser
    Parser=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aes3::Streams_Fill()
{
    int64u BitRate=(int64u)-1;
    int64u SamplingRate=(int64u)-1;
    if (PTS!=(int32u)-1 && Frame_Last_PTS!=(int32u)-1 && PTS!=Frame_Last_PTS)
    {
        //Rounding
        BitRate=Frame_Last_Size*8*1000*1000000*(Frame_Count-1)/(PTS-Frame_Last_PTS);
        SamplingRate=BitRate*(4+bits_per_sample)/(5+bits_per_sample)/(2+2*number_channels)/(16+4*bits_per_sample);
        if (SamplingRate>  7840 && SamplingRate<  8160) SamplingRate=  8000;
        if (SamplingRate> 15680 && SamplingRate< 16320) SamplingRate= 16000;
        if (SamplingRate> 31360 && SamplingRate< 32640) SamplingRate= 32000;
        if (SamplingRate> 62720 && SamplingRate< 65280) SamplingRate= 64000;
        if (SamplingRate> 10804 && SamplingRate< 11246) SamplingRate= 11025;
        if (SamplingRate> 21609 && SamplingRate< 22491) SamplingRate= 22050;
        if (SamplingRate> 43218 && SamplingRate< 44982) SamplingRate= 44100;
        if (SamplingRate> 86436 && SamplingRate< 89964) SamplingRate= 88200;
        if (SamplingRate> 11760 && SamplingRate< 12240) SamplingRate= 12000;
        if (SamplingRate> 23520 && SamplingRate< 24480) SamplingRate= 24000;
        if (SamplingRate> 47040 && SamplingRate< 48960) SamplingRate= 48000;
        if (SamplingRate> 94080 && SamplingRate< 97920) SamplingRate= 96000;
        if (SamplingRate>188160 && SamplingRate<195840) SamplingRate=192000;
        BitRate=SamplingRate/(4+bits_per_sample)*(5+bits_per_sample)*(2+2*number_channels)*(16+4*bits_per_sample);
    }

    if (Count_Get(Stream_Audio))
    {
        if (Count_Get(Stream_Audio)==1 && Retrieve(Stream_Audio, 0, Audio_BitRate).empty() && BitRate!=(int64u)-1)
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
        return; //Done elsewhere
    }

    if (From_MpegPs)
    {
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "PCM");
        Fill(Stream_Audio, 0, Audio_MuxingMode, "AES3");
        Fill(Stream_Audio, 0, Audio_Codec, "AES3");
        Fill(Stream_Audio, 0, Audio_Channel_s_, 2+2*number_channels);
        Fill(Stream_Audio, 0, Audio_ChannelPositions, Aes3_ChannelsPositions(number_channels));
        Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Aes3_ChannelsPositions2(number_channels));
        Fill(Stream_Audio, 0, Audio_Resolution, 16+4*bits_per_sample);
        if (PTS!=(int32u)-1 && Frame_Last_PTS!=(int32u)-1 && PTS!=Frame_Last_PTS)
        {
            Fill(Stream_Audio, 0, Audio_SamplingRate, SamplingRate);
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
        }
    }
    else if (Retrieve(Stream_General, 0, General_Format).empty())
    {
        if (data_type!=(int8u)-1)
        {
            Fill(Stream_General, 0, General_Format, _T("AES3 / ")+Ztring().From_Local(Aes3_NonPCM_data_type[data_type]), true);
            if (Aes3_NonPCM_data_type_StreamKind[data_type]!=Stream_Max)
            {
                Stream_Prepare(Aes3_NonPCM_data_type_StreamKind[data_type]);
                Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Format), Aes3_NonPCM_data_type[data_type]);
            }
        }
        else
        {
            Fill(Stream_General, 0, General_Format, "AES3");
            Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, 0, Audio_Format, "AES3");
        }
    }
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aes3::Read_Buffer_Continue()
{
    if (From_MpegPs)
        Frame_FromMpegPs();
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Aes3::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+16<=Buffer_Size)
    {
        if (CC4(Buffer+Buffer_Offset)==0xF8724E1F) //SMPTE 337M 16-bit, BE
        {
            Container_Bits=16;
            Stream_Bits=16;
            Endianess=false; //BE
            break; //while()
        }
        if (CC4(Buffer+Buffer_Offset)==0x72F81F4E) //SMPTE 337M 16-bit, LE
        {
            Container_Bits=16;
            Stream_Bits=16;
            Endianess=true; //LE
            break; //while()
        }
        if (CC5(Buffer+Buffer_Offset)==0x6F87254E1FLL) //SMPTE 337M 20-bit, BE
        {
            Container_Bits=20;
            Stream_Bits=20;
            Endianess=false; //BE
            break; //while()
        }
        if (CC6(Buffer+Buffer_Offset)==0x96F872A54E1FLL) //SMPTE 337M 24-bit, BE
        {
            Container_Bits=24;
            Stream_Bits=24;
            Endianess=false; //BE
            break; //while()
        }
        if (CC6(Buffer+Buffer_Offset)==0x72F8961F4E5ALL) //SMPTE 337M 24-bit, LE
        {
            Container_Bits=24;
            Stream_Bits=24;
            Endianess=true; //LE
            break; //while()
        }
        if (CC6(Buffer+Buffer_Offset)==0x00F872004E1FLL) //16-bit in 24-bit, BE
        {
            Container_Bits=24;
            Stream_Bits=16;
            Endianess=false; //BE
            break; //while()
        }
        if (CC6(Buffer+Buffer_Offset)==0x0072F8001F4ELL) //16-bit in 24-bit, LE
        {
            Container_Bits=24;
            Stream_Bits=16;
            Endianess=true; //LE
            break; //while()
        }
        if (CC6(Buffer+Buffer_Offset)==0x6F872054E1F0LL) //20-bit in 24-bit, BE
        {
            Container_Bits=24;
            Stream_Bits=20;
            Endianess=false; //BE
            break; //while()
        }
        if (CC6(Buffer+Buffer_Offset)==0x20876FF0E154LL) //20-bit in 24-bit, LE
        {
            Container_Bits=24;
            Stream_Bits=20;
            Endianess=true; //LE
            break; //while()
        }
        if (CC8(Buffer+Buffer_Offset)==0x0000F87200004E1FLL) //16-bit in 32-bit, BE
        {
            Container_Bits=32;
            Stream_Bits=16;
            Endianess=false; //BE
            break; //while()
        }
        if (CC8(Buffer+Buffer_Offset)==0x000072F800001F4ELL) //16-bit in 32-bit, LE
        {
            Container_Bits=32;
            Stream_Bits=16;
            Endianess=true; //LE
            break; //while()
        }
        if (CC8(Buffer+Buffer_Offset)==0x006F87200054E1F0LL) //20-bit in 32-bit, BE
        {
            Container_Bits=32;
            Stream_Bits=20;
            Endianess=false; //BE
            break; //while()
        }
        if (CC8(Buffer+Buffer_Offset)==0x0020876F00F0E154LL) //20-bit in 32-bit, LE
        {
            Container_Bits=32;
            Stream_Bits=20;
            Endianess=true; //LE
            break; //while()
        }
        if (CC8(Buffer+Buffer_Offset)==0x0096F8720A54E1FLL) //24-bit in 32-bit, BE
        {
            Container_Bits=32;
            Stream_Bits=24;
            Endianess=false; //BE
            break; //while()
        }
        if (CC8(Buffer+Buffer_Offset)==0x0072F896001F4EA5LL) //24-bit in 32-bit, LE
        {
            Container_Bits=32;
            Stream_Bits=24;
            Endianess=true; //LE
            break; //while()
        }
        Buffer_Offset++;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+16>Buffer_Size)
    {
        return false;
    }

    //Synched
    Data_Accept("AES3");
    return true;
}

//---------------------------------------------------------------------------
bool File_Aes3::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+16>Buffer_Size)
        return false;

    //Quick test of synchro
    switch (Endianess)
    {
        case false :
                    switch (Container_Bits)
                    {
                        case 16 :   if (CC4(Buffer+Buffer_Offset)!=0xF8724E1F) {Synched=false; return true;} break;
                        case 20 :   if (CC5(Buffer+Buffer_Offset)!=0x6F87254E1FLL) {Synched=false; return true;} break;
                        case 24 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : if (CC6(Buffer+Buffer_Offset)!=0xF872004E1F00LL) {Synched=false; return true;} break;
                                        case 20 : if (CC6(Buffer+Buffer_Offset)!=0x6F872054E1F0LL) {Synched=false; return true;} break;
                                        case 24 : if (CC6(Buffer+Buffer_Offset)!=0x96F872A54E1FLL) {Synched=false; return true;} break;
                                        default : ;
                                    }
                                    break;
                        case 32 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : if (CC6(Buffer+Buffer_Offset)!=0x0000F87200004E1FLL) {Synched=false; return true;} break;
                                        case 20 : if (CC6(Buffer+Buffer_Offset)!=0x006F87200054E1F0LL) {Synched=false; return true;} break;
                                        case 24 : if (CC6(Buffer+Buffer_Offset)!=0x0096F87200A5F41FLL) {Synched=false; return true;} break;
                                        default : ;
                                    }
                                    break;
                        default : ;
                    }
                    break;
        case true  :
                    switch (Container_Bits)
                    {
                        case 16 :   if (CC4(Buffer+Buffer_Offset)!=0) {Synched=false; return true;} break;
                        case 24 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : if (CC6(Buffer+Buffer_Offset)!=0x0072F8001F4ELL) {Synched=false; return true;} break;
                                        case 20 : if (CC6(Buffer+Buffer_Offset)!=0x20876FF0E154LL) {Synched=false; return true;} break;
                                        case 24 : if (CC6(Buffer+Buffer_Offset)!=0x72F8961F4EA5LL) {Synched=false; return true;} break;
                                            default : ;
                                    }
                                    break;
                        case 32 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : if (CC8(Buffer+Buffer_Offset)!=0x000072F800001F4ELL) {Synched=false; return true;} break;
                                        case 20 : if (CC8(Buffer+Buffer_Offset)!=0x0020876F00F0E154LL) {Synched=false; return true;} break;
                                        case 24 : if (CC8(Buffer+Buffer_Offset)!=0x0072F896001F4EA5LL) {Synched=false; return true;} break;
                                        default : ;
                                    }
                                    break;
                        default : ;
                    }
                    break;
    }

    //We continue
    return true;
}

//***************************************************************************
// Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aes3::Header_Parse()
{
    //Parsing
    int32u Size=0;
    switch (Endianess)
    {
        case false :
                    switch (Container_Bits)
                    {
                        case 16 :   Size=BigEndian2int16u(Buffer+Buffer_Offset+6)         ; break;
                        case 20 :   Size=BigEndian2int24u(Buffer+Buffer_Offset+7)&0x0FFFFF; break;
                        case 24 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : Size=BigEndian2int16u(Buffer+Buffer_Offset+9)   ; break;
                                        case 20 : Size=BigEndian2int24u(Buffer+Buffer_Offset+9)>>4; break;
                                        case 24 : Size=BigEndian2int24u(Buffer+Buffer_Offset+9)   ; break;
                                        default : ;
                                    }
                                    break;
                        case 32 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : Size=BigEndian2int16u(Buffer+Buffer_Offset+0xC)   ; break;
                                        case 20 : Size=BigEndian2int24u(Buffer+Buffer_Offset+0xC)>>4; break;
                                        case 24 : Size=BigEndian2int24u(Buffer+Buffer_Offset+0xC)   ; break;
                                        default : ;
                                    }
                                    break;
                        default : ;
                    }
                    break;
        case true  :
                    switch (Container_Bits)
                    {
                        case 16 :   Size=LittleEndian2int16u(Buffer+Buffer_Offset+6)   ; break;
                        case 24 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : Size=LittleEndian2int16u(Buffer+Buffer_Offset+0xA)   ; break;
                                        case 20 : Size=LittleEndian2int24u(Buffer+Buffer_Offset+0x9)>>4; break;
                                        case 24 : Size=LittleEndian2int24u(Buffer+Buffer_Offset+0x9)   ; break;
                                        default : ;
                                    }
                                    break;
                        case 32 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : Size=LittleEndian2int16u(Buffer+Buffer_Offset+0xE)   ; break;
                                        case 20 : Size=LittleEndian2int24u(Buffer+Buffer_Offset+0xD)>>4; break;
                                        case 24 : Size=LittleEndian2int24u(Buffer+Buffer_Offset+0xD)   ; break;
                                        default : ;
                                    }
                                    break;
                        default : ;
                    }
                    break;
    }

    //Adaptation
    if (Container_Bits!=Stream_Bits)
    {
        Size*=Container_Bits; Size/=Stream_Bits;
    }

    //Filling
    Header_Fill_Size(Container_Bits*4/8+Size/8);
    Header_Fill_Code(0, "AES3");
}

//---------------------------------------------------------------------------
void File_Aes3::Data_Parse()
{
    if (Container_Bits==Stream_Bits && !Endianess) //BE
        Frame();
    else
        Frame_WithPadding();
}

//---------------------------------------------------------------------------
void File_Aes3::Frame()
{
    //Parsing
    int32u  length_code;
    Element_Begin("Header");
        BS_Begin();
        Skip_S3(Stream_Bits,                                    "Pa");
        Skip_S3(Stream_Bits,                                    "Pb");
        Element_Begin("Pc");
            Skip_S1( 3,                                         "data_stream_number");
            Skip_S1( 5,                                         "data_type_dependent");
            Skip_SB(                                            "error_flag");
            Skip_S1( 2,                                         "data_mode");
            Get_S1 ( 5, data_type,                              "data_type"); Param_Info(Aes3_NonPCM_data_type[data_type]);
            if (Stream_Bits>16)
                Skip_S1( 4,                                     "reserved");
            if (Stream_Bits>20)
                Skip_S1( 4,                                     "reserved");
        Element_End();
        Get_S3 (Stream_Bits, length_code,                       "length_code");
        BS_End();
    Element_End();

    if (Parser==NULL)
    {
        switch(data_type)
        {
            case 28 : Parser=new File_DolbyE(); break;
            default : ;
        }

        if (Parser)
        {
            Open_Buffer_Init(Parser);
        }
    }
    if (Parser)
    {
        Open_Buffer_Continue(Parser, Buffer+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
        Element_Offset=Element_Size;
        if (Parser->Status[IsFinished])
        {
            if (Parser->Status[IsFilled])
            {
                Merge(*Parser);
                Fill(Stream_General, 0, General_Format, _T("AES3 / ")+Parser->Retrieve(Stream_General, 0, General_Format), true);
                int64u OverallBitRate=Parser->Retrieve(Stream_General, 0, General_OverallBitRate).To_int64u();
                OverallBitRate*=Element_Size; OverallBitRate/=Element_Size-Stream_Bits*4/8;
                Fill(Stream_General, 0, General_OverallBitRate, Ztring::ToZtring(OverallBitRate)+_T(" / ")+Parser->Retrieve(Stream_General, 0, General_OverallBitRate));
            }
            Finish("AES3");
        }
    }
    else
        Finish("AES3");
}

//---------------------------------------------------------------------------
void File_Aes3::Frame_WithPadding()
{
    int8u* Info=new int8u[(size_t)Element_Size];
    size_t Info_Offset=0;

    if (Container_Bits==24 && Stream_Bits==20 && Endianess) //LE
    {
        while (Element_Offset+6<=Element_Size)
        {
            size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

            Info[Info_Offset+0]=  Buffer[Buffer_Pos+2];
            Info[Info_Offset+1]=  Buffer[Buffer_Pos+1];
            Info[Info_Offset+2]=((Buffer[Buffer_Pos+0]&0xF0)   ) | ((Buffer[Buffer_Pos+5]&0xF0)>>4);
            Info[Info_Offset+3]=((Buffer[Buffer_Pos+5]&0x0F)<<4) | ((Buffer[Buffer_Pos+4]&0xF0)>>4);
            Info[Info_Offset+4]=((Buffer[Buffer_Pos+4]&0x0F)<<4) | ((Buffer[Buffer_Pos+3]&0xF0)>>4);
            Info_Offset+=5;
            Element_Offset+=6;
        }
    }
    if (Container_Bits==32 && Stream_Bits==16 && Endianess) //LE
    {
        while (Element_Offset+8<=Element_Size)
        {
            size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

            Info[Info_Offset+0]=  Buffer[Buffer_Pos+3];
            Info[Info_Offset+1]=  Buffer[Buffer_Pos+2];
            Info[Info_Offset+2]=  Buffer[Buffer_Pos+7];
            Info[Info_Offset+3]=  Buffer[Buffer_Pos+6];
            Info_Offset+=4;
            Element_Offset+=8;
        }
    }
    if (Container_Bits==32 && Stream_Bits==20 && Endianess) //LE
    {
        while (Element_Offset+8<=Element_Size)
        {
            size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

            Info[Info_Offset+0]=  Buffer[Buffer_Pos+3];
            Info[Info_Offset+1]=  Buffer[Buffer_Pos+2];
            Info[Info_Offset+2]=((Buffer[Buffer_Pos+1]&0xF0)   ) | ((Buffer[Buffer_Pos+7]&0xF0)>>4);
            Info[Info_Offset+3]=((Buffer[Buffer_Pos+7]&0x0F)<<4) | ((Buffer[Buffer_Pos+6]&0xF0)>>4);
            Info[Info_Offset+4]=((Buffer[Buffer_Pos+6]&0x0F)<<4) | ((Buffer[Buffer_Pos+5]&0xF0)>>4);
            Info_Offset+=5;
            Element_Offset+=8;
        }
    }
    if (Container_Bits==32 && Stream_Bits==24 && Endianess) //LE
    {
        while (Element_Offset+8<=Element_Size)
        {
            size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

            Info[Info_Offset+0]=  Buffer[Buffer_Pos+3];
            Info[Info_Offset+1]=  Buffer[Buffer_Pos+2];
            Info[Info_Offset+2]=  Buffer[Buffer_Pos+1];
            Info[Info_Offset+3]=  Buffer[Buffer_Pos+7];
            Info[Info_Offset+4]=  Buffer[Buffer_Pos+6];
            Info[Info_Offset+5]=  Buffer[Buffer_Pos+5];
            Info_Offset+=6;
            Element_Offset+=8;
        }
    }

    if (Element_Offset==0)
    {
        Skip_XX(Element_Size,                                   "Data");
        Finish();
        delete[] Info;
        return;
    }

    Parser_Parse(Info, Info_Offset);
    delete[] Info;
}

//---------------------------------------------------------------------------
static inline int8u Reverse8(int n)
{
    // Input: bit order is 76543210
    //Output: bit order is 45670123
    n=((n>>1)&0x55) | ((n<<1) & 0xaa);
    n=((n>>2)&0x33) | ((n<<2) & 0xcc);
    n=((n>>4)&0x0f) | ((n<<4) & 0xf0);
    return (int8u)n;
}

//---------------------------------------------------------------------------
void File_Aes3::Frame_FromMpegPs()
{
    //SMPTE 302M
    int16u audio_packet_size;
    Get_B2 (audio_packet_size,                              "audio_packet_size");
    BS_Begin();
    Get_S1 (2, number_channels,                             "number_channels"); Param_Info(2+2*number_channels, " channels");
    Info_S1(8, channel_identification,                      "channel_identification");
    Get_S1 (2, bits_per_sample,                             "bits_per_sample"); Param_Info(16+4*bits_per_sample, " bits");
    Info_S1(4, alignment_bits,                              "alignment_bits");
    BS_End();

    if (Element_Offset+audio_packet_size!=Element_Size || audio_packet_size<192*2*(bits_per_sample==0?2:3))
    {
        Skip_XX(Element_Size-Element_Offset,                "Error?");
        return;
    }

    if (!Status[IsAccepted])
        Accept("AES3");

    //Parsing
    switch (bits_per_sample)
    {
        case 0  : //16 bits
        {
            int8u* Info=new int8u[(size_t)Element_Size-4];
            size_t Info_Offset=0;

            while (Element_Offset<Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                //Channel 1 (16 bits, as "s16l" codec)
                Info[Info_Offset+0]= Reverse8(Buffer[Buffer_Pos+0]);
                Info[Info_Offset+1]= Reverse8(Buffer[Buffer_Pos+1]);

                //Channel 2 (16 bits, as "s16l" codec)
                Info[Info_Offset+2]=(Reverse8(Buffer[Buffer_Pos+2])>>4) | ((Reverse8(Buffer[Buffer_Pos+3])<<4)&0xF0);
                Info[Info_Offset+3]=(Reverse8(Buffer[Buffer_Pos+3])>>4) | ((Reverse8(Buffer[Buffer_Pos+4])<<4)&0xF0);

                Info_Offset+=4;
                Element_Offset+=5;
            }

            if (Info[0]==0x20 && Info[1]==0x87 && Info[2]==0x6F)
            {
                Parser_Parse(Info, Info_Offset);
            }

            delete[] Info;
        }
        break;
        case 1  : //20 bits
        {
            int8u* Info=new int8u[(size_t)Element_Size-4];
            size_t Info_Offset=0;

            while (Element_Offset<Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                //Channel 1 (24 bits, as "s24l" codec, 4 highest bits are set to 0)
                Info[Info_Offset+0]=                                       ((Reverse8(Buffer[Buffer_Pos+0])<<4)&0xF0);
                Info[Info_Offset+1]=(Reverse8(Buffer[Buffer_Pos+0])>>4 ) | ((Reverse8(Buffer[Buffer_Pos+1])<<4)&0xF0);
                Info[Info_Offset+2]=(Reverse8(Buffer[Buffer_Pos+1])>>4 ) | ((Reverse8(Buffer[Buffer_Pos+2])<<4)&0xF0);

                //Channel 2 (24 bits, as "s24l" codec, 4 highest bits are set to 0)
                Info[Info_Offset+3]=                                      ((Reverse8(Buffer[Buffer_Pos+3])<<4)&0xF0);
                Info[Info_Offset+4]=(Reverse8(Buffer[Buffer_Pos+3])>>4) | ((Reverse8(Buffer[Buffer_Pos+4])<<4)&0xF0);
                Info[Info_Offset+5]=(Reverse8(Buffer[Buffer_Pos+4])>>4) | ((Reverse8(Buffer[Buffer_Pos+5])<<4)&0xF0);

                Info_Offset+=6;
                Element_Offset+=6;
            }

            if (IsParsingNonPcm || (Info[0]==0x20 && Info[1]==0x87 && Info[2]==0x6F))
            {
                IsParsingNonPcm=true;
                Parser_Parse(Info, Info_Offset);
            }

            delete[] Info;
        }
        break;
        case 2  : //24 bits
        {
            int8u* Info=new int8u[(size_t)Element_Size-4];
            size_t Info_Offset=0;

            while (Element_Offset<Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                //Channel 1 (24 bits, as "s24l" codec)
                Info[Info_Offset+0] = Reverse8(Buffer[Buffer_Pos+0] );
                Info[Info_Offset+1] = Reverse8(Buffer[Buffer_Pos+1] );
                Info[Info_Offset+2] = Reverse8(Buffer[Buffer_Pos+2] );

                //Channel 2 (24 bits, as "s24l" codec)
                Info[Info_Offset+3] = (Reverse8(Buffer[Buffer_Pos+3])>>4) | ((Reverse8(Buffer[Buffer_Pos+4])<<4)&0xF0 );
                Info[Info_Offset+4] = (Reverse8(Buffer[Buffer_Pos+4])>>4) | ((Reverse8(Buffer[Buffer_Pos+5])<<4)&0xF0 );
                Info[Info_Offset+5] = (Reverse8(Buffer[Buffer_Pos+5])>>4) | ((Reverse8(Buffer[Buffer_Pos+6])<<4)&0xF0 );

                Info_Offset+=6;
                Element_Offset+=7;
            }

            if (Info[0]==0x20 && Info[1]==0x87 && Info[2]==0x6F)
            {
                Parser_Parse(Info, Info_Offset);
            }

            delete[] Info;
        }
        break;
        default :
            Skip_XX(Element_Size,                           "Data");
    }

    //Looking for 2 consecutive PTS
    Frame_Count++;
    if (PTS==(int64u)-1)
        Frame_Count=2; //We don't have PTS, don't need more
    else if (Frame_Count==1)
    {
        Frame_Last_PTS=PTS;
        Frame_Last_Size=Element_Size;
    }

    if ((!IsParsingNonPcm || (Parser && Parser->Status[IsFinished])) && Frame_Count>=2)
    {
        //Filling
        Finish("AES3");
    }
}

//---------------------------------------------------------------------------
void File_Aes3::Parser_Parse(const int8u* Parser_Buffer, size_t Parser_Buffer_Size)
{
    if (Parser==NULL)
    {
        Parser=new File_Aes3();
        Open_Buffer_Init(Parser);
    }
    Open_Buffer_Continue(Parser, Parser_Buffer, Parser_Buffer_Size);

    if (!From_MpegPs)
        Frame_Count++;
    if (Count_Get(Stream_Audio)==0 && Parser->Status[IsFinished])
    {
        //Filling
        Merge(*Parser);
        ZtringList OverallBitRates; OverallBitRates.Separator_Set(0, _T(" / ")); OverallBitRates.Write(Parser->Retrieve(Stream_General, 0, General_OverallBitRate));
        if (!OverallBitRates.empty())
        {
            int64u OverallBitRate=OverallBitRates[0].To_int64u();
            OverallBitRate*=Element_Offset; OverallBitRate/=Parser_Buffer_Size;
            OverallBitRates[0].From_Number(OverallBitRate);
            Fill(Stream_General, 0, General_Format, Parser->Retrieve(Stream_General, 0, General_Format), true);
            Fill(Stream_General, 0, General_OverallBitRate, OverallBitRates.Read(), true);
        }
        if (!From_MpegPs)
            Finish("AES3");
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AES3_YES
