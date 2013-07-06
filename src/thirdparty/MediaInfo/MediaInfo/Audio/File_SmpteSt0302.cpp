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
    {
        File_SmpteSt0337* SmpteSt0337=new File_SmpteSt0337();
        SmpteSt0337->Container_Bits=(4+bits_per_sample)*4;
        SmpteSt0337->Endianness='L';
        SmpteSt0337->Aligned=true;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_Level=4; //Intermediate
                SmpteSt0337->Demux_Level=2; //Container
                SmpteSt0337->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
        Parsers.push_back(SmpteSt0337);
    }

    // Raw PCM
    {
        File_Pcm* Pcm=new File_Pcm();
        Pcm->Codec.From_Local("SMPTE ST 302");
        Pcm->BitDepth=(4+bits_per_sample)*4;
        Pcm->Channels=(1+number_channels)*2;
        Pcm->SamplingRate=48000;
        Pcm->Endianness='L';
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_Level=4; //Intermediate
                Pcm->Demux_Level=2; //Container
                Pcm->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
        Parsers.push_back(Pcm);
    }

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
        if (Retrieve(Stream_Audio, 0, Audio_BitRate).empty())
           Fill(Stream_Audio, 0, Audio_BitRate, (4+bits_per_sample)*(1+number_channels)*8*48000);
        if (Retrieve(Stream_Audio, 0, Audio_Format)==__T("PCM"))
        {
            Fill(Stream_Audio, 0, Audio_Codec, "AES3", Unlimited, true, true);
            Fill(Stream_Audio, 0, Audio_Codec_String, "AES3", Unlimited, true, true);
            Clear(Stream_Audio, 0, Audio_Codec_Family);
        }
    }

    Fill(Stream_Audio, 0, Audio_BitRate_Encoded, (5+bits_per_sample)*(1+number_channels)*8*48000);
    for (size_t Pos=1; Pos<Count_Get(Stream_Audio); Pos++)
        Fill(Stream_Audio, Pos, Audio_BitRate_Encoded, 0);
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
    float64 Ratio=0;
    switch (bits_per_sample)
    {
        case 0 : PcmSize=audio_packet_size*4/5; Ratio=4.0/5.0; break;
        case 1 : PcmSize=audio_packet_size*5/6; Ratio=5.0/6.0; break;
        case 2 : PcmSize=audio_packet_size*6/7; Ratio=6.0/7.0; break;
        case 3 : Reject(); return;
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
                        // Source:        L1L0 L3L2 R0XX R2R1 XXR3
                        // Dest  : 16LE / L1L0 L3L2 R1R0 R3R2
                        Info[Info_Offset+0]= Reverse8(Buffer[Buffer_Pos+0]);
                        Info[Info_Offset+1]= Reverse8(Buffer[Buffer_Pos+1]);
                        Info[Info_Offset+2]=(Reverse8(Buffer[Buffer_Pos+3])<<4  ) | (Reverse8(Buffer[Buffer_Pos+2])>>4  );
                        Info[Info_Offset+3]=(Reverse8(Buffer[Buffer_Pos+4])<<4  ) | (Reverse8(Buffer[Buffer_Pos+3])>>4  );

                        Info_Offset+=4;
                        Element_Offset+=5;
                        break;

            case 1  :   //20 bits
                        // Source:        L1L0 L3L2 XXL4 R1R0 R3R2 XXR4
                        // Dest  : 20LE / L1L0 L3L2 R0L4 R2R1 R4R3
                        Info[Info_Offset+0]= Reverse8(Buffer[Buffer_Pos+0])                                               ;
                        Info[Info_Offset+1]= Reverse8(Buffer[Buffer_Pos+1])                                               ;
                        Info[Info_Offset+2]=(Reverse8(Buffer[Buffer_Pos+3])<<4  ) | (Reverse8(Buffer[Buffer_Pos+2])&0x0F);
                        Info[Info_Offset+3]=(Reverse8(Buffer[Buffer_Pos+4])<<4  ) | (Reverse8(Buffer[Buffer_Pos+3])>>4  );
                        Info[Info_Offset+4]=(Reverse8(Buffer[Buffer_Pos+5])<<4  ) | (Reverse8(Buffer[Buffer_Pos+4])>>4  );

                        Info_Offset+=5;
                        Element_Offset+=6;
                        break;

            case 2  :   //24 bits
                        // Source:        L1L0 L3L2 L5L4 R0XX R2R1 R4R3 XXR5
                        // Dest  : 16LE / L1L0 L3L2 L5L4 R1R0 R3R2 R5R4
                        Info[Info_Offset+0] = Reverse8(Buffer[Buffer_Pos+0])                                              ;
                        Info[Info_Offset+1] = Reverse8(Buffer[Buffer_Pos+1])                                              ;
                        Info[Info_Offset+2] = Reverse8(Buffer[Buffer_Pos+2])                                              ;
                        Info[Info_Offset+3] =(Reverse8(Buffer[Buffer_Pos+4])<<4  ) | (Reverse8(Buffer[Buffer_Pos+3])>>4  );
                        Info[Info_Offset+4] =(Reverse8(Buffer[Buffer_Pos+5])<<4  ) | (Reverse8(Buffer[Buffer_Pos+4])>>4  );
                        Info[Info_Offset+5] =(Reverse8(Buffer[Buffer_Pos+6])<<4  ) | (Reverse8(Buffer[Buffer_Pos+5])>>4  );

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
        Demux(Info, Info_Offset, ContentType_MainStream);
    #endif //MEDIAINFO_DEMUX

    //Parsers
    for (size_t Pos=0; Pos<Parsers.size(); Pos++)
    {
        Parsers[Pos]->FrameInfo=FrameInfo;
        Open_Buffer_Continue(Parsers[Pos], Info, Info_Offset, true, Ratio);

        if (Parsers.size()>1 && Parsers[Pos]->Status[IsAccepted])
        {
            for (size_t Pos2=0; Pos2<Pos; Pos2++)
                delete Parsers[Pos2]; //Parsers[Pos2]=NULL;
            for (size_t Pos2=Pos+1; Pos2<Parsers.size(); Pos2++)
                delete Parsers[Pos2]; //Parsers[Pos2]=NULL;
            Parsers.resize(Pos+1);
            Parsers.erase(Parsers.begin(), Parsers.begin()+Parsers.size()-1);
        }
    }
    Element_Offset=Element_Size;

    delete[] Info;

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
