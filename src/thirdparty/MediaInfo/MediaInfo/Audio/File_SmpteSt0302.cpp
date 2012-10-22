// File_SmpteSt0302 - Info for SMPTE ST0302
// Copyright (C) 2008-2012 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_SMPTEST0302_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_SmpteSt0302.h"
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_SmpteSt0337.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#if MEDIAINFO_DEMUX
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#endif //MEDIAINFO_EVENTS
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_SmpteSt0302::File_SmpteSt0302()
:File__Analyze()
{
    //Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Aes3;
    #endif //MEDIAINFO_EVENTS
    PTS_DTS_Needed=true;
    IsRawStream=true;
}

//---------------------------------------------------------------------------
File_SmpteSt0302::~File_SmpteSt0302()
{
    for (size_t Pos=0; Pos<Parsers.size(); Pos++)
        delete Parsers[Pos]; //Parsers[Pos]=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_SmpteSt0302::Streams_Accept()
{
    // SMPTE ST 337
    File_SmpteSt0337* SmpteSt0337=new File_SmpteSt0337();
    SmpteSt0337->Container_Bits_Original=(4+bits_per_sample)*4;
    SmpteSt0337->Container_Bits=SmpteSt0337->Container_Bits_Original==20?24:SmpteSt0337->Container_Bits_Original;
    Parsers.push_back(SmpteSt0337);

    // Raw PCM
    File_Pcm* Pcm=new File_Pcm();
    Pcm->Codec.From_Local("SMPTE ST 337");
    Pcm->BitDepth=(4+bits_per_sample)*4;
    Pcm->Channels=(1+number_channels)*2;
    Pcm->SamplingRate=48000;
    Parsers.push_back(Pcm);

    // Init
    for (size_t Pos=0; Pos<Parsers.size(); Pos++)
        Open_Buffer_Init(Parsers[Pos]);
}

//---------------------------------------------------------------------------
void File_SmpteSt0302::Streams_Fill()
{
    if (Parsers.size()==1 && Parsers[0]->Status[IsAccepted])
    {
        Fill(Parsers[0]);
        Merge(*Parsers[0]);
    }

    for (size_t Pos=0; Pos<Count_Get(Stream_Audio); Pos++)
        if (Retrieve(Stream_Audio, Pos, Audio_MuxingMode).empty()) //TODO: put "SMPTE ST 302" in this field, the current name is there only for legacy
            Fill(Stream_Audio, 0, Audio_MuxingMode, "AES3");

    if (Count_Get(Stream_Audio)==1)
    {
        Fill(Stream_Audio, 0, Audio_BitRate, (5+bits_per_sample)*(1+number_channels)*8*48000);
        if (Retrieve(Stream_Audio, 0, Audio_Format)==__T("PCM"))
        {
            Fill(Stream_Audio, 0, Audio_Codec, "AES3", Unlimited, true, true);
            Fill(Stream_Audio, 0, Audio_Codec_String, "AES3", Unlimited, true, true);
            Clear(Stream_Audio, 0, Audio_Codec_Family);
        }
    }
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

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
void File_SmpteSt0302::Read_Buffer_Continue()
{
    //Parsing
    Get_B2 (audio_packet_size,                              "audio_packet_size");
    BS_Begin();
    Get_S1 (2, number_channels,                             "number_channels"); Param_Info2((1+number_channels)*2, " channels");
    Info_S1(8, channel_identification,                      "channel_identification");
    Get_S1 (2, bits_per_sample,                             "bits_per_sample"); Param_Info2((4+bits_per_sample)*4, " bits");
    Info_S1(4, alignment_bits,                              "alignment_bits");
    BS_End();

    //Enough data
    if (Element_Size<4+(int64u)audio_packet_size)
    {
        Element_Offset=0;
        Element_WaitForMoreData();
        return;
    }

    //Cohenrancy test
    if (Element_Size!=4+(int64u)audio_packet_size || bits_per_sample==3 || audio_packet_size%((1+number_channels)*(5+bits_per_sample)))
    {
        Trusted_IsNot("Wrong size");
        Skip_XX(Element_Size-4,                             "Problem?");
        return;
    }

    if (!Status[IsAccepted])
        Accept("SMPTE ST 302");

    //Decyphering
    size_t PcmSize=0;
    switch (bits_per_sample)
    {
        case 0 : PcmSize=audio_packet_size*4/5; break;
        case 1 : PcmSize=audio_packet_size    ; break; //Should be 5/6, but we pad 20-bit to 24-bit so (5+1)/6
        case 2 : PcmSize=audio_packet_size*6/7; break;
        default: ;
    }

    int8u* Info=new int8u[PcmSize];
    size_t Info_Offset=0;

    while (Element_Offset<Element_Size)
    {
        size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

        switch (bits_per_sample)
        {
            case 0  :   //16 bits
                        //Channel 1 (16 bits, as "s16l" codec)
                        Info[Info_Offset+0]= Reverse8(Buffer[Buffer_Pos+0]);
                        Info[Info_Offset+1]= Reverse8(Buffer[Buffer_Pos+1]);

                        //Channel 2 (16 bits, as "s16l" codec)
                        Info[Info_Offset+2]=(Reverse8(Buffer[Buffer_Pos+2])>>4) | ((Reverse8(Buffer[Buffer_Pos+3])<<4)&0xF0);
                        Info[Info_Offset+3]=(Reverse8(Buffer[Buffer_Pos+3])>>4) | ((Reverse8(Buffer[Buffer_Pos+4])<<4)&0xF0);

                        Info_Offset+=4;
                        Element_Offset+=5;
                        break;
            case 1  :   //20 bits
                        //Channel 1 (24 bits, as "s24l" codec, 4 lowest bits are set to 0)
                        Info[Info_Offset+0]=                                       ((Reverse8(Buffer[Buffer_Pos+0])<<4)&0xF0);
                        Info[Info_Offset+1]=(Reverse8(Buffer[Buffer_Pos+0])>>4 ) | ((Reverse8(Buffer[Buffer_Pos+1])<<4)&0xF0);
                        Info[Info_Offset+2]=(Reverse8(Buffer[Buffer_Pos+1])>>4 ) | ((Reverse8(Buffer[Buffer_Pos+2])<<4)&0xF0);

                        //Channel 2 (24 bits, as "s24l" codec, 4 lowest bits are set to 0)
                        Info[Info_Offset+3]=                                      ((Reverse8(Buffer[Buffer_Pos+3])<<4)&0xF0);
                        Info[Info_Offset+4]=(Reverse8(Buffer[Buffer_Pos+3])>>4) | ((Reverse8(Buffer[Buffer_Pos+4])<<4)&0xF0);
                        Info[Info_Offset+5]=(Reverse8(Buffer[Buffer_Pos+4])>>4) | ((Reverse8(Buffer[Buffer_Pos+5])<<4)&0xF0);

                        Info_Offset+=6;
                        Element_Offset+=6;
                        break;
                case 2  : //24 bits
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
                        break;
                default : ;
        }
    }
    Element_Offset=4;

    FrameInfo.PTS=FrameInfo.DTS;
    FrameInfo.DUR=((int64u)audio_packet_size)*1000000000/((1+number_channels)*(5+bits_per_sample)*48000);

    #if MEDIAINFO_DEMUX
        Demux_random_access=true;

        if (Config->Demux_PCM_20bitTo16bit_Get())
        {
            size_t Info2_Size=((size_t)Element_Size-4)*2/3;
            int8u* Info2=new int8u[Info2_Size];
            size_t Info2_Pos=0;
            size_t Info_Pos=0;

            while (Info_Pos<Info_Offset)
            {
                Info2[Info2_Pos+0]=Info[Info_Pos+1];
                Info2[Info2_Pos+1]=Info[Info_Pos+2];
                Info2[Info2_Pos+2]=Info[Info_Pos+4];
                Info2[Info2_Pos+3]=Info[Info_Pos+5];

                Info2_Pos+=4;
                Info_Pos+=6;
            }

            Demux(Info2, Info2_Pos, ContentType_MainStream, Buffer+Buffer_Offset+4, (size_t)Element_Size-4);
        }
        else
            Demux(Info, Info_Offset, ContentType_MainStream, Buffer+Buffer_Offset+4, (size_t)Element_Size-4);
    #endif //MEDIAINFO_DEMUX

    //Parsers
    for (size_t Pos=0; Pos<Parsers.size(); Pos++)
    {
        Parsers[Pos]->FrameInfo=FrameInfo;
        Open_Buffer_Continue(Parsers[Pos], Info, Info_Offset);
        Element_Offset=Element_Size;

        if (Parsers.size()>1 && Parsers[Pos]->Status[IsAccepted])
        {
            for (size_t Pos2=0; Pos2<Pos; Pos2++)
                delete Parsers[Pos2]; //Parsers[Pos2]=NULL;
            for (size_t Pos2=Pos+1; Pos2<Parsers.size()-1; Pos2++)
                delete Parsers[Pos2]; //Parsers[Pos2]=NULL;
            Parsers.resize(Pos+1);
            Parsers.erase(Parsers.begin(), Parsers.begin()+Parsers.size()-1);
        }
    }

    FrameInfo.DTS+=FrameInfo.DUR;

    //Filling
    Frame_Count++;
    if (Frame_Count_NotParsedIncluded!=(int64u)-1)
        Frame_Count_NotParsedIncluded++;
    if (Parsers.size()>1 && Frame_Count>=2)
    {
        for (size_t Pos=0; Pos<Parsers.size()-1; Pos++)
            delete Parsers[Pos]; //Parsers[Pos]=NULL;
        Parsers.erase(Parsers.begin(), Parsers.begin()+Parsers.size()-1);
    }

    if (!Status[IsFilled] && Parsers.size()==1 && Parsers[0]->Status[IsFinished])
    {
        //Filling
        Finish("SMPTE ST 302");
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_SMPTEST0302_YES
