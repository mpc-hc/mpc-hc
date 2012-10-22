// File_Aes3 - Info for AES3 packetized streams
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
//
// AES3 PCM and non-PCM (SMPTE 337M)
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
#if defined(MEDIAINFO_AES3_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Aes3.h"
#if defined(MEDIAINFO_AAC_YES)
    #include "MediaInfo/Audio/File_Aac.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_DOLBYE_YES)
    #include "MediaInfo/Audio/File_DolbyE.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#if MEDIAINFO_SEEK
    #include "MediaInfo/MediaInfo_Internal.h"
#endif //MEDIAINFO_SEEK
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
    "Pause",
    "MPEG Audio",
    "MPEG Audio",
    "MPEG Audio",
    "AAC",
    "MPEG Audio",
    "MPEG Audio",
    "AAC",
    "AAC",
    "",
    "",
    "",
    "",
    "E-AC-3",
    "",
    "",
    "AAC",
    "",
    "E-AC-3",
    "",
    "",
    "",
    "",
    "Utility",
    "KLV",
    "Dolby E",
    "Captioning",
    "User defined",
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
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Aes3;
    #endif //MEDIAINFO_EVENTS
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=32*1024;
    PTS_DTS_Needed=true;
    FrameInfo.DTS=0;
    IsRawStream=true;

    //In
    SampleRate=0;
    ByteSize=0;
    QuantizationBits=0;
    ChannelCount=0;
    From_Raw=false;
    From_MpegPs=false;
    From_Aes3=false;
    IsAes3=false;
    Endianness=0x00;

    //Out
    FrameRate=0;

    //Temp
    Frame_Count=0;
    Frame_Size=(int64u)-1;
    Frame_Duration=(int64u)-1;
    IsPcm_Frame_Count=0;
    NotPCM_SizePerFrame=(int64u)-1;
    data_type=(int8u)-1;
    IsParsingNonPcm=false;
    IsPcm=false;

    //Parser
    Parser=NULL;

    #if MEDIAINFO_SEEK
        Duration_Detected=false;
    #endif //MEDIAINFO_SEEK
}

//---------------------------------------------------------------------------
File_Aes3::~File_Aes3()
{
    delete Parser; //Parser=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aes3::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, "AES3");
}

//---------------------------------------------------------------------------
void File_Aes3::Streams_Fill()
{
    int64u BitRate=(int64u)-1;
    int64u SamplingRate=(int64u)-1;
    if (Frame_Count && FrameInfo.PTS!=(int32u)-1 && PTS_Begin!=(int64u)-1 && FrameInfo.PTS!=PTS_Begin)
    {
        //Rounding
        BitRate=Frame_Size*8*1000*1000000*(Frame_Count-1)/(FrameInfo.PTS-PTS_Begin);
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
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR", Unlimited, true, true);

    if (Parser && Parser->Status[IsAccepted] && !Parser->Status[IsFilled])
    {
        Fill(Parser);
        Merge(*Parser);
    }
    else if (data_type!=(int8u)-1)
    {
        if (Retrieve(Stream_Audio, 0, Audio_Format).empty() && Aes3_NonPCM_data_type_StreamKind[data_type]!=Stream_Max)
        {
            Stream_Prepare(Aes3_NonPCM_data_type_StreamKind[data_type]);
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Format), Aes3_NonPCM_data_type[data_type]);
        }

        if (IsSub)
            Fill(Stream_Audio, 0, Audio_MuxingMode, "AES3", Unlimited, true, true);
    }
    else if (IsPcm)
    {
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "PCM");
        Fill(Stream_Audio, 0, Audio_Codec, "PCM");
        switch (Endianness)
        {
            case 'B' : Fill(Stream_Audio, 0, Audio_Format_Settings_Endianness, "Big"); break;
            case 'L' : Fill(Stream_Audio, 0, Audio_Format_Settings_Endianness, "Little"); break;
            default  : ;
        }
    }

    if (Count_Get(Stream_Audio))
    {
        if (Count_Get(Stream_Audio)==1 && Retrieve(Stream_Audio, 0, Audio_BitRate).empty() && BitRate!=(int64u)-1)
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate);

        if (!IsSub && NotPCM_SizePerFrame!=(int64u)-1 && NotPCM_SizePerFrame && FrameRate)
        {
            int64u BitRate=float64_int64s(NotPCM_SizePerFrame*8*FrameRate);
            int64u SamplingRate=Retrieve(Stream_Audio, 0, Audio_SamplingRate).To_int64u();
            if (Container_Bits && SamplingRate)
            {
                float Ratio=((float)BitRate)/(Container_Bits*SamplingRate*2);
                if (Ratio>=0.99 && Ratio<=1.01)
                    BitRate=Container_Bits*SamplingRate*2;
            }
            Fill(Stream_General, 0, General_OverallBitRate, BitRate);
        }
    }
    else if (From_MpegPs)
    {
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "PCM");
        Fill(Stream_Audio, 0, Audio_MuxingMode, "AES3");
        Fill(Stream_Audio, 0, Audio_Codec, "AES3");
        Fill(Stream_Audio, 0, Audio_Channel_s_, 2+2*number_channels);
        Fill(Stream_Audio, 0, Audio_ChannelPositions, Aes3_ChannelsPositions(number_channels));
        Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Aes3_ChannelsPositions2(number_channels));
        Fill(Stream_Audio, 0, Audio_BitDepth, 16+4*bits_per_sample);
        if (FrameInfo.PTS!=(int64u)-1 && PTS_Begin!=(int64u)-1 && FrameInfo.PTS!=PTS_Begin)
        {
            Fill(Stream_Audio, 0, Audio_SamplingRate, SamplingRate);
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
        }
    }
    else
    {
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "AES3");
    }

    if (!From_Raw && !From_Aes3 && !IsPcm && IsParsingNonPcm)
    {
        for (size_t Pos=0; Pos<Count_Get(Stream_Audio); Pos++)
        {
            switch (Endianness)
            {
                case 'B' : Fill(Stream_Audio, Pos, Audio_Format_Settings_Endianness, "Big"); break;
                case 'L' : Fill(Stream_Audio, Pos, Audio_Format_Settings_Endianness, "Little"); break;
                default  : ;
            }
            Fill(Stream_Audio, Pos, Audio_Format_Settings_Mode, Container_Bits);
            if (Retrieve(Stream_Audio, Pos, Audio_BitDepth).empty())
                Fill(Stream_Audio, Pos, Audio_BitDepth, Stream_Bits);
        }
    }

    if (From_Raw)
    {
        for (size_t Pos=0; Pos<Count_Get(Stream_Audio); Pos++)
            Fill(Stream_Audio, Pos, Audio_Format_Settings_Endianness, "Little");
    }

    if (!IsSub && File_Size!=(int64u)-1 && Frame_Size!=(int64u)-1)
    {
        for (size_t Pos=0; Pos<Count_Get(Stream_Audio); Pos++)
            Fill(Stream_Audio, Pos, Audio_FrameCount, File_Size/Frame_Size);
        Fill(Stream_General, 0, General_DataSize, ((File_Size-Buffer_TotalBytes_FirstSynched)/Frame_Size)*Frame_Size);
    }
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aes3::Read_Buffer_Init()
{
    if (From_MpegPs)
    {
        MustSynchronize=false;
    }
}

//---------------------------------------------------------------------------
void File_Aes3::Read_Buffer_Continue()
{
    if (Frame_Count==0)
    {
        PTS_Begin=FrameInfo.PTS;
        Frame_Size=Element_Size;
    }

    if (IsPcm)
    {
        #if MEDIAINFO_DEMUX
            if (ByteSize)
                Element_Size=(Buffer_Size/ByteSize)*ByteSize;
            else
                Element_Size=Buffer_Size;
            if (Demux_UnpacketizeContainer && !(!IsAes3 && StreamIDs_Size>=2 && ParserIDs[StreamIDs_Size-2]==MediaInfo_Parser_ChannelGrouping))
            {
                FrameInfo.PTS=FrameInfo.DTS;
                if (SampleRate && ByteSize)
                    FrameInfo.DUR=Element_Size*1000000000/(SampleRate*ByteSize);
                Demux_random_access=true;
                Element_Code=(int64u)-1;
                Demux(Buffer, (size_t)Element_Size, ContentType_MainStream);
            }
        #endif //MEDIAINFO_DEMUX

        Skip_XX(Element_Size,                                   "Data");

        Frame_Count_InThisBlock++;
        if (IsPcm_Frame_Count)
        {
            Frame_Count_InThisBlock++; //If IsPcm_Frame_Count is set, 2 frames were needed in order to detect PCM
            IsPcm_Frame_Count=0;
        }
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
        #if MEDIAINFO_DEMUX
            if (FrameInfo.DTS!=(int64u)-1 && FrameInfo.DUR!=(int64u)-1)
            {
                FrameInfo.DTS+=FrameInfo.DUR;
                FrameInfo.PTS=FrameInfo.DTS;
            }
        #endif //MEDIAINFO_DEMUX

        return;
    }
    if (From_Raw)
    {
        Raw();
        return;
    }
    if (From_MpegPs)
    {
        Frame_FromMpegPs();
        return;
    }

    //Special cases
    if (!From_Aes3 && !Status[IsAccepted])
    {
        Synchronize();
        if (!IsParsingNonPcm)
        {
            IsPcm_Frame_Count++;
            Buffer_Offset=0;
            if ((IsSub && Buffer_Size>32*1024) || (IsSub && IsPcm_Frame_Count>1) || ChannelCount==1)
            {
                //Raw PCM
                MustSynchronize=false;
                IsPcm=true;
                Read_Buffer_Continue();

                Accept("PCM");
                Finish();

                return;
            }

            Element_WaitForMoreData();
            return;
        }
    }

    if (!From_Aes3 && Frame_Count==0)
    {
        //Guard band
        Buffer_Offset_Temp=Buffer_Offset;
        if (ByteSize==6)
        {
            while(Buffer_Offset_Temp+6<=Buffer_Size && CC3(Buffer+Buffer_Offset_Temp)==0x000000)
                Buffer_Offset_Temp+=6;

            if (Buffer_Offset_Temp+6>Buffer_Size)
            {
                Element_WaitForMoreData();
                return;
            }
        }
        else if (ByteSize==8)
        {
            while(Buffer_Offset_Temp+8<=Buffer_Size && CC4(Buffer+Buffer_Offset_Temp)==0x00000000)
                Buffer_Offset_Temp+=8;

            if (Buffer_Offset_Temp+8>Buffer_Size)
            {
                Element_WaitForMoreData();
                return;
            }
        }
        else
        {
            if (Buffer_Offset_Temp+8>Buffer_Size)
            {
                Element_WaitForMoreData();
                return;
            }

            if (CC8(Buffer+Buffer_Offset_Temp)==0x0000000000000000LL)
            {
                while(Buffer_Offset_Temp+2<=Buffer_Size && CC2(Buffer+Buffer_Offset_Temp)==0x0000)
                    Buffer_Offset_Temp+=2;

                if (Buffer_Offset_Temp+2>Buffer_Size)
                {
                    Element_WaitForMoreData();
                    return;
                }
            }
        }
        if (Buffer_Offset_Temp-Buffer_Offset)
        {
            Skip_XX(Buffer_Offset_Temp-Buffer_Offset,  "Guard band");
        }
    }
}

#if MEDIAINFO_SEEK
//---------------------------------------------------------------------------
void File_Aes3::Read_Buffer_Unsynched()
{
    if (File_GoTo==0)
        Synched_Init();

    if (Frame_Count_NotParsedIncluded!=(int64u)-1 && Frame_Duration!=(int64u)-1 && Frame_Size!=(int64u)-1)
    {
        Frame_Count_NotParsedIncluded=File_GoTo/Frame_Size;
        if (Frame_Count_NotParsedIncluded*Frame_Size<File_GoTo)
            Frame_Count_NotParsedIncluded++;
        FrameInfo.DTS=Frame_Count_NotParsedIncluded*Frame_Duration;
    }
}
#endif //MEDIAINFO_SEEK

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_Aes3::Read_Buffer_Seek (size_t Method, int64u Value, int64u /*ID*/)
{
    //Init
    if (!Duration_Detected && File_Size!=(int64u)-1 && (Frame_Size==(int64u)-1 || Frame_Duration==(int64u)-1))
    {
        MediaInfo_Internal MI;
        MI.Option(__T("File_KeepInfo"), __T("1"));
        Ztring ParseSpeed_Save=MI.Option(__T("ParseSpeed_Get"), __T(""));
        Ztring Demux_Save=MI.Option(__T("Demux_Get"), __T(""));
        MI.Option(__T("ParseSpeed"), __T("0"));
        MI.Option(__T("Demux"), Ztring());
        size_t MiOpenResult=MI.Open(File_Name);
        MI.Option(__T("ParseSpeed"), ParseSpeed_Save); //This is a global value, need to reset it. TODO: local value
        MI.Option(__T("Demux"), Demux_Save); //This is a global value, need to reset it. TODO: local value
        if (!MiOpenResult)
            return 0;

        int64u FrameCount=MI.Get(Stream_Audio, 0, __T("FrameCount")).To_int64u();
        int64u Duration=MI.Get(Stream_Audio, 0, __T("Duration")).To_int64u();
        int64u DataSize=MI.Get(Stream_General, 0, __T("DataSize")).To_int64u();
        if (FrameCount && Duration)
            Frame_Duration=Duration*1000000/FrameCount; //In nanoseconds
        if (FrameCount && DataSize)
            Frame_Size=DataSize/FrameCount;

        Duration_Detected=true;
    }

    //Parsing
    switch (Method)
    {
        case 0  :
                    GoTo(Value);
                    Open_Buffer_Unsynch();
                    return 1;
        case 1  :
                    GoTo(File_Size*Value/10000);
                    Open_Buffer_Unsynch();
                    return 1;
        case 2  :   //Timestamp
                    {
                    if (Frame_Duration==(int64u)-1 || Frame_Size==(int64u)-1)
                        return (size_t)-1; //Not supported

                    Unsynch_Frame_Count=float64_int64s(((float64)Value)/Frame_Duration);
                    GoTo(Unsynch_Frame_Count*Frame_Size);
                    Open_Buffer_Unsynch();
                    return 1;
                    }
        case 3  :   //FrameNumber
                    {
                    if (Frame_Size==(int64u)-1)
                        return (size_t)-1; //Not supported

                    Unsynch_Frame_Count=Value;
                    GoTo(Unsynch_Frame_Count*Frame_Size);
                    Open_Buffer_Unsynch();
                    return 1;
                    }
        default :   return (size_t)-1; //Not supported
    }
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Aes3::Synchronize()
{
    if (IsPcm)
        return false; //No sync with raw PCM, must return immediatly and wait for more data

    //Synchronizing
    while (Buffer_Offset+16<=Buffer_Size)
    {
        if (!Status[IsAccepted] && !IsSub && File_Offset_FirstSynched==(int64u)-1 && Buffer_TotalBytes+Buffer_Offset>=Buffer_TotalBytes_FirstSynched_Max)
        {
            Reject();
            return false;
        }

        if ((ByteSize==0 || ByteSize==4) && ((Buffer_TotalBytes+Buffer_Offset)%4)==0)
        {
            if (Buffer[Buffer_Offset  ]==0xF8
             && Buffer[Buffer_Offset+1]==0x72
             && Buffer[Buffer_Offset+2]==0x4E
             && Buffer[Buffer_Offset+3]==0x1F) //SMPTE 337M 16-bit, BE
            {
                ByteSize=4;
                Container_Bits=16;
                Stream_Bits=16;
                Endianness='B'; //BE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x72
             && Buffer[Buffer_Offset+1]==0xF8
             && Buffer[Buffer_Offset+2]==0x1F
             && Buffer[Buffer_Offset+3]==0x4E) //SMPTE 337M 16-bit, LE
            {
                ByteSize=4;
                Container_Bits=16;
                Stream_Bits=16;
                Endianness='L'; //LE
                break; //while()
            }
        }
        if ((ByteSize==0 || ByteSize==5) && ((Buffer_TotalBytes+Buffer_Offset)%5)==0)
        {
            if (Buffer[Buffer_Offset  ]==0x6F
             && Buffer[Buffer_Offset+1]==0x87
             && Buffer[Buffer_Offset+2]==0x25
             && Buffer[Buffer_Offset+3]==0x4E
             && Buffer[Buffer_Offset+4]==0x1F) //SMPTE 337M 20-bit, BE
            {
                ByteSize=5;
                Container_Bits=20;
                Stream_Bits=20;
                Endianness='B'; //BE
                break; //while()
            }
        }
        if ((ByteSize==0 || ByteSize==6) && ((Buffer_TotalBytes+Buffer_Offset)%6)==0)
        {
            if (Buffer[Buffer_Offset  ]==0x96
             && Buffer[Buffer_Offset+1]==0xF8
             && Buffer[Buffer_Offset+2]==0x72
             && Buffer[Buffer_Offset+3]==0xA5
             && Buffer[Buffer_Offset+4]==0x4E
             && Buffer[Buffer_Offset+5]==0x1F) //SMPTE 337M 24-bit, BE
            {
                ByteSize=6;
                Container_Bits=24;
                Stream_Bits=24;
                Endianness='B'; //BE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x72
             && Buffer[Buffer_Offset+1]==0xF8
             && Buffer[Buffer_Offset+2]==0x96
             && Buffer[Buffer_Offset+3]==0x1F
             && Buffer[Buffer_Offset+4]==0x4E
             && Buffer[Buffer_Offset+5]==0x5A) //SMPTE 337M 24-bit, LE
            {
                ByteSize=6;
                Container_Bits=24;
                Stream_Bits=24;
                Endianness='L'; //LE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0xF8
             && Buffer[Buffer_Offset+2]==0x72
             && Buffer[Buffer_Offset+3]==0x00
             && Buffer[Buffer_Offset+4]==0x4E
             && Buffer[Buffer_Offset+5]==0x1F) //16-bit in 24-bit, BE
            {
                ByteSize=6;
                Container_Bits=24;
                Stream_Bits=16;
                Endianness='B'; //BE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x72
             && Buffer[Buffer_Offset+2]==0xF8
             && Buffer[Buffer_Offset+3]==0x00
             && Buffer[Buffer_Offset+4]==0x1F
             && Buffer[Buffer_Offset+5]==0x4E) //16-bit in 24-bit, LE
            {
                ByteSize=6;
                Container_Bits=24;
                Stream_Bits=16;
                Endianness='L'; //LE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x6F
             && Buffer[Buffer_Offset+1]==0x87
             && Buffer[Buffer_Offset+2]==0x20
             && Buffer[Buffer_Offset+3]==0x54
             && Buffer[Buffer_Offset+4]==0xE1
             && Buffer[Buffer_Offset+5]==0xF0) //20-bit in 24-bit, BE
            {
                ByteSize=6;
                Container_Bits=24;
                Stream_Bits=20;
                Endianness='B'; //BE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x20
             && Buffer[Buffer_Offset+1]==0x87
             && Buffer[Buffer_Offset+2]==0x6F
             && Buffer[Buffer_Offset+3]==0xF0
             && Buffer[Buffer_Offset+4]==0xE1
             && Buffer[Buffer_Offset+5]==0x54) //20-bit in 24-bit, LE
            {
                ByteSize=6;
                Container_Bits=24;
                Stream_Bits=20;
                Endianness='L'; //LE
                break; //while()
            }
        }
        if ((ByteSize==0 || ByteSize==8) && ((Buffer_TotalBytes+Buffer_Offset)%8)==0)
        {
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x00
             && Buffer[Buffer_Offset+2]==0xF8
             && Buffer[Buffer_Offset+3]==0x72
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0x00
             && Buffer[Buffer_Offset+6]==0x4E
             && Buffer[Buffer_Offset+7]==0x1F) //16-bit in 32-bit, BE
            {
                ByteSize=8;
                Container_Bits=32;
                Stream_Bits=16;
                Endianness='B'; //BE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x00
             && Buffer[Buffer_Offset+2]==0x72
             && Buffer[Buffer_Offset+3]==0xF8
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0x00
             && Buffer[Buffer_Offset+6]==0x1F
             && Buffer[Buffer_Offset+7]==0x4E) //16-bit in 32-bit, LE
            {
                ByteSize=8;
                Container_Bits=32;
                Stream_Bits=16;
                Endianness='L'; //LE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x6F
             && Buffer[Buffer_Offset+2]==0x87
             && Buffer[Buffer_Offset+3]==0x20
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0x54
             && Buffer[Buffer_Offset+6]==0xE1
             && Buffer[Buffer_Offset+7]==0xF0) //20-bit in 32-bit, BE
            {
                ByteSize=8;
                Container_Bits=32;
                Stream_Bits=20;
                Endianness='B'; //BE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x20
             && Buffer[Buffer_Offset+2]==0x87
             && Buffer[Buffer_Offset+3]==0x6F
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0xF0
             && Buffer[Buffer_Offset+6]==0xE1
             && Buffer[Buffer_Offset+7]==0x54) //20-bit in 32-bit, LE
            {
                ByteSize=8;
                Container_Bits=32;
                Stream_Bits=20;
                Endianness='L'; //LE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x96
             && Buffer[Buffer_Offset+2]==0xF8
             && Buffer[Buffer_Offset+3]==0x72
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0xA5
             && Buffer[Buffer_Offset+6]==0x4E
             && Buffer[Buffer_Offset+7]==0x1F) //24-bit in 32-bit, BE
            {
                ByteSize=8;
                Container_Bits=32;
                Stream_Bits=24;
                Endianness='B'; //BE
                break; //while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x72
             && Buffer[Buffer_Offset+2]==0xF8
             && Buffer[Buffer_Offset+3]==0x96
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0x1F
             && Buffer[Buffer_Offset+6]==0x4E
             && Buffer[Buffer_Offset+7]==0xA5) //24-bit in 32-bit, LE
            {
                ByteSize=8;
                Container_Bits=32;
                Stream_Bits=24;
                Endianness='L'; //LE
                break; //while()
            }
        }

        if (ByteSize)
            Buffer_Offset+=ByteSize;
        else
            Buffer_Offset++;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+16>Buffer_Size)
    {
        return false;
    }

    //Synched
    IsParsingNonPcm=true;
    return true;
}

//---------------------------------------------------------------------------
bool File_Aes3::Synched_Test()
{
    //Skip NULL padding
    if (!From_Aes3)
    {
        size_t Buffer_Offset_Temp=Buffer_Offset;
        if (ByteSize==4)
        {
            while ((Buffer_TotalBytes+Buffer_Offset_Temp)%4) //Padding in part of the AES3 block
            {
                if (Buffer_Offset_Temp+1>Buffer_Size)
                {
                    Element_WaitForMoreData();
                    return false;
                }
                if (Buffer[Buffer_Offset_Temp])
                {
                    Trusted_IsNot("Bad sync");
                    return true;
                }
                Buffer_Offset_Temp++;
            }
            while(Buffer_Offset_Temp+4<=Buffer_Size && CC4(Buffer+Buffer_Offset_Temp)==0x00000000)
                Buffer_Offset_Temp+=4;
            if (Buffer_Offset_Temp+4>Buffer_Size)
            {
                Element_WaitForMoreData();
                return false;
            }
        }
        if (ByteSize==6)
        {
            while ((Buffer_TotalBytes+Buffer_Offset_Temp)%6) //Padding in part of the AES3 block
            {
                if (Buffer_Offset_Temp+1>Buffer_Size)
                {
                    Element_WaitForMoreData();
                    return false;
                }
                if (Buffer[Buffer_Offset_Temp])
                {
                    Trusted_IsNot("Bad sync");
                    return true;
                }
                Buffer_Offset_Temp++;
            }
            while(Buffer_Offset_Temp+6<=Buffer_Size && CC6(Buffer+Buffer_Offset_Temp)==0x000000000000LL)
                Buffer_Offset_Temp+=6;
            if (Buffer_Offset_Temp+6>Buffer_Size)
            {
                Element_WaitForMoreData();
                return false;
            }
        }
        else if (ByteSize==8)
        {
            while ((Buffer_TotalBytes+Buffer_Offset_Temp)%8) //Padding in part of the AES3 block
            {
                if (Buffer_Offset_Temp+1>Buffer_Size)
                {
                    Element_WaitForMoreData();
                    return false;
                }
                if (Buffer[Buffer_Offset_Temp])
                {
                    Trusted_IsNot("Bad sync");
                    return true;
                }
                Buffer_Offset_Temp++;
            }
            while(Buffer_Offset_Temp+8<=Buffer_Size && CC8(Buffer+Buffer_Offset_Temp)==0x0000000000000000LL)
                Buffer_Offset_Temp+=8;
            if (Buffer_Offset_Temp+8>Buffer_Size)
            {
                Element_WaitForMoreData();
                return false;
            }
        }

        if (Frame_Count && NotPCM_SizePerFrame==(int64u)-1)
            NotPCM_SizePerFrame=Buffer_Offset_Temp;
        #if MEDIAINFO_TRACE
            if (Buffer_Offset_Temp-Buffer_Offset)
            {
                Element_Size=Buffer_Offset_Temp-Buffer_Offset;
                Skip_XX(Buffer_Offset_Temp-Buffer_Offset,           "Guard band");
            }
        #endif //MEDIAINFO_TRACE
        Buffer_Offset=Buffer_Offset_Temp;
    }

    //Must have enough buffer for having header
    if (Buffer_Offset+16>Buffer_Size)
        return false;

    //Quick test of synchro
    switch (Endianness)
    {
        case 'B' :
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
        case 'L'  :
                    switch (Container_Bits)
                    {
                        case 16 :   if (CC4(Buffer+Buffer_Offset)!=0x72F81F4E) {Synched=false; return true;} break;
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
        default    : ; //Should never happen
    }

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_Aes3::Synched_Init()
{
    if (Frame_Count_NotParsedIncluded==(int64u)-1)
        Frame_Count_NotParsedIncluded=0;
    if (FrameInfo.DTS==(int64u)-1)
        FrameInfo.DTS=0;
}

//***************************************************************************
// Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aes3::Header_Parse()
{
    if (IsPcm)
    {
        Element_WaitForMoreData();
        return;
    }

    //Parsing
    int32u Size=0;
    switch (Endianness)
    {
        case 'B' :
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
        case 'L'  :
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
        default   : ; //Should never happen
    }

    //Adaptation
    if (Container_Bits!=Stream_Bits)
    {
        Size*=Container_Bits; Size/=Stream_Bits;
    }

    //Coherency test
    if (!IsSub && !Status[IsAccepted])
    {
        size_t Offset=Buffer_Offset+(size_t)(Container_Bits*4/8+Size/8);
        while (Offset<Buffer_Size && Buffer[Offset]==0x00)
            Offset++;
        if (Offset+ByteSize>Buffer_Size)
        {
            Element_WaitForMoreData();
            return;
        }
        Offset/=ByteSize;
        Offset*=ByteSize;
        bool IsOK=true;
        for (size_t Pos=0; Pos<ByteSize; Pos++)
            if (Buffer[Buffer_Offset+Pos]!=Buffer[Offset+Pos])
            {
                IsOK=false;
                break;
            }
        if (!IsOK)
        {
            Trusted_IsNot("Bad sync");
            Buffer_Offset++;
            return;
        }
    }

    //Filling
    Header_Fill_Size(Container_Bits*4/8+Size/8);
    Header_Fill_Code(0, "AES3");
}

//---------------------------------------------------------------------------
void File_Aes3::Data_Parse()
{
    if (!Status[IsAccepted])
        Accept("AES3");

    if (Container_Bits==Stream_Bits && Endianness=='B')
        Frame();
    else
        Frame_WithPadding();
}

//---------------------------------------------------------------------------
void File_Aes3::Raw()
{
    //SMPTE 331M
    int16u Audio_Sample_Count;
    int8u Channels_valid;
    BS_Begin();
    Skip_SB(                                                "FVUCP Valid Flag");
    Skip_S1(4,                                              "Reserved");
    Skip_S1(3,                                              "5-sequence count");
    BS_End();
    Get_L2 (Audio_Sample_Count,                             "Audio Sample Count");
    Get_B1 (Channels_valid,                                 "Channels valid");

    //Parsing
    if (QuantizationBits==16)
        bits_per_sample=0; //16 bits
    else
        bits_per_sample=2; //24 bits
    switch (bits_per_sample)
    {
        case 0  : //16 bits
        {
            int8u* Info=new int8u[(size_t)(Element_Size/2)];
            size_t Info_Offset=0;

            while (Element_Offset+8*4<=Element_Size)
            {
                for (int8u Pos=0; Pos<8; Pos++)
                {
                    if (Channels_valid&(1<<Pos))
                    {
                        size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                        Info[Info_Offset+0] = (Buffer[Buffer_Pos+1]>>4) | ((Buffer[Buffer_Pos+2]<<4)&0xF0 );
                        Info[Info_Offset+1] = (Buffer[Buffer_Pos+2]>>4) | ((Buffer[Buffer_Pos+3]<<4)&0xF0 );

                        Info_Offset+=2;
                    }
                    Element_Offset+=4;
                }
            }
            Element_Offset=4;

            #if MEDIAINFO_DEMUX
                FrameInfo.PTS=FrameInfo.DTS;
                if (SampleRate)
                    FrameInfo.DUR=(Element_Size-4)/4*1000000000/(SampleRate*8);
                Demux_random_access=true;
                Element_Code=(int64u)-1;
                Demux(Info, Info_Offset, ContentType_MainStream);
            #endif //MEDIAINFO_DEMUX

            delete[] Info;

            Skip_XX(Element_Size-4,                             "Data");

            Frame_Count_InThisBlock++;
            if (Frame_Count_NotParsedIncluded!=(int64u)-1)
                Frame_Count_NotParsedIncluded++;
            #if MEDIAINFO_DEMUX
                if (FrameInfo.DTS!=(int64u)-1 && FrameInfo.DUR!=(int64u)-1)
                {
                    FrameInfo.DTS+=FrameInfo.DUR;
                    FrameInfo.PTS=FrameInfo.DTS;
                }
            #endif //MEDIAINFO_DEMUX
        }
        break;
        case 2  : //24 bits
        {
            int8u* Info=new int8u[(size_t)(Element_Size*3/4)];
            size_t Info_Offset=0;

            while (Element_Offset+8*4<=Element_Size)
            {
                for (int8u Pos=0; Pos<8; Pos++)
                {
                    if (Channels_valid&(1<<Pos))
                    {
                        size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                        Info[Info_Offset+0] = (Buffer[Buffer_Pos+0]>>4) | ((Buffer[Buffer_Pos+1]<<4)&0xF0 );
                        Info[Info_Offset+1] = (Buffer[Buffer_Pos+1]>>4) | ((Buffer[Buffer_Pos+2]<<4)&0xF0 );
                        Info[Info_Offset+2] = (Buffer[Buffer_Pos+2]>>4) | ((Buffer[Buffer_Pos+3]<<4)&0xF0 );

                        Info_Offset+=3;
                    }
                    Element_Offset+=4;
                }
            }
            Element_Offset=4;

            #if MEDIAINFO_DEMUX
                FrameInfo.PTS=FrameInfo.DTS;
                if (SampleRate)
                    FrameInfo.DUR=(Element_Size-4)/4*1000000000/(SampleRate*8);
                Demux_random_access=true;
                Element_Code=(int64u)-1;
                Demux(Info, Info_Offset, ContentType_MainStream);
            #endif //MEDIAINFO_DEMUX

            delete[] Info;

            Skip_XX(Element_Size-4,                             "Data");

            Frame_Count_InThisBlock++;
            if (Frame_Count_NotParsedIncluded!=(int64u)-1)
                Frame_Count_NotParsedIncluded++;
            #if MEDIAINFO_DEMUX
                if (FrameInfo.DTS!=(int64u)-1 && FrameInfo.DUR!=(int64u)-1)
                {
                    FrameInfo.DTS+=FrameInfo.DUR;
                    FrameInfo.PTS=FrameInfo.DTS;
                }
            #endif //MEDIAINFO_DEMUX
        }
        break;
        default :
            Skip_XX(Element_Size-4,                             "Data");
    }

    FILLING_BEGIN();
    if (!Status[IsAccepted])
    {
        Accept("AES3");

        int8u Channels=0;
        for (int8u Pos=0; Pos<8; Pos++)
        {
            if (Channels_valid&(1<<Pos))
                Channels++;
            Element_Offset+=4;
        }

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "PCM");
        Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
    }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Aes3::Frame()
{
    //Parsing
    int32u  length_code;
    Element_Begin1("Header");
        BS_Begin();
        Skip_S3(Stream_Bits,                                    "Pa");
        Skip_S3(Stream_Bits,                                    "Pb");
        Element_Begin1("Pc");
            Skip_S1( 3,                                         "data_stream_number");
            Skip_S1( 5,                                         "data_type_dependent");
            Skip_SB(                                            "error_flag");
            Info_S1( 2, data_mode,                              "data_mode"); Param_Info2(16+4*data_mode, " bits");
            Get_S1 ( 5, data_type,                              "data_type"); Param_Info1(Aes3_NonPCM_data_type[data_type]);
            if (Stream_Bits>16)
                Skip_S1( 4,                                     "reserved");
            if (Stream_Bits>20)
                Skip_S1( 4,                                     "reserved");
        Element_End0();
        Get_S3 (Stream_Bits, length_code,                       "length_code"); Param_Info2(length_code/8, " bytes");
        BS_End();
    Element_End0();

    if (Parser==NULL)
    {
        switch(data_type)
        {
            //SMPTE ST338
            case  1 :   //AC-3
            case 16 :   //E-AC-3 (professional)
            case 21 :   //E-AC-3 (consumer)
                        Parser=new File_Ac3();
                        ((File_Ac3*)Parser)->Frame_Count_Valid=2;
                        break;
            case  4 :   //MPEG-1 Layer 1
            case  5 :   //MPEG-1 Layer 2/3, MPEG-2 Layer 1/2/3 without extension
            case  6 :   //MPEG-2 Layer 1/2/3 with extension
            case  8 :   //MPEG-2 Layer 1 low frequency
            case  9 :   //MPEG-2 Layer 2/3 low frequency
                        Parser=new File_Mpega();
                        break;
            case  7 :   //MPEG-2 AAC in ADTS
            case 19 :   //MPEG-2 AAC in ADTS low frequency
                        Parser=new File_Aac();
                        ((File_Aac*)Parser)->Mode=File_Aac::Mode_ADTS;
                        break;
            case 10 :   //MPEG-4 AAC in ADTS or LATM
            case 11 :   //MPEG-4 AAC in ADTS or LATM
                        Parser=new File_Aac();
                        break;
            case 28 :   //Dolby E
                        Parser=new File_DolbyE();
                        break;
            default : ;
        }

        if (Parser)
        {
            Open_Buffer_Init(Parser);
        }
    }
    if (Parser)
    {
        #if MEDIAINFO_DEMUX
            FrameInfo.PTS=FrameInfo.DTS;
            if (SampleRate && ByteSize)
                FrameInfo.DUR=(Element_Size-Element_Offset)*1000000000/(SampleRate*ByteSize);
            Demux_random_access=true;
            Element_Code=(int64u)-1;
            Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset), ContentType_MainStream);
        #endif //MEDIAINFO_DEMUX

        Parser->FrameInfo=FrameInfo;
        Open_Buffer_Continue(Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
        #if MEDIAINFO_DEMUX
            if (Parser->FrameInfo.DUR!=FrameInfo.DUR && Parser->FrameInfo.DUR!=(int64u)-1)
                FrameInfo.DUR=Parser->FrameInfo.DUR;
            if (FrameInfo.DUR!=(int64u)-1)
                FrameInfo.DTS+=FrameInfo.DUR;
            else if (Parser->FrameInfo.DTS!=(int64u)-1)
                FrameInfo=Parser->FrameInfo;
            else
                FrameInfo.DTS=(int64u)-1;
        #endif //MEDIAINFO_DEMUX
        Frame_Count++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
        if (!Status[IsFilled] && Parser->Status[IsFilled])
        {
            Merge(*Parser);
            int64u OverallBitRate=Parser->Retrieve(Stream_General, 0, General_OverallBitRate).To_int64u();
            if (OverallBitRate)
            {
                OverallBitRate*=Element_Size; OverallBitRate/=Element_Size-Stream_Bits*4/8;
                Fill(Stream_General, 0, General_OverallBitRate, Ztring::ToZtring(OverallBitRate)+__T(" / ")+Parser->Retrieve(Stream_General, 0, General_OverallBitRate));
            }
            int64u BitRate=Parser->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int64u();
            if (BitRate)
                FrameRate=((float64)BitRate)/((Element_Size-Element_Offset)*8);
            Fill("AES3");
        }
        if (Parser->Status[IsFinished])
            Finish("AES3");
        Element_Offset=Element_Size;
    }
    else
        Finish("AES3");
}

//---------------------------------------------------------------------------
void File_Aes3::Frame_WithPadding()
{
    #if MEDIAINFO_DEMUX
        FrameInfo.PTS=FrameInfo.DTS;
        if (SampleRate && ByteSize)
            FrameInfo.DUR=Element_Size*1000000000/(SampleRate*ByteSize);
        Demux_random_access=true;
        Element_Code=(int64u)-1;
        Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
    #endif //MEDIAINFO_DEMUX

    int8u* Info=new int8u[(size_t)Element_Size];
    size_t Info_Offset=0;

    if (Container_Bits==16 && Stream_Bits==16 && Endianness=='L')
    {
        while (Element_Offset+4<=Element_Size)
        {
            size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

            Info[Info_Offset+0]=  Buffer[Buffer_Pos+1];
            Info[Info_Offset+1]=  Buffer[Buffer_Pos+0];
            Info[Info_Offset+2]=  Buffer[Buffer_Pos+3];
            Info[Info_Offset+3]=  Buffer[Buffer_Pos+2];
            Info_Offset+=4;
            Element_Offset+=4;
        }
        if (Element_Offset+2<=Element_Size) //Only in half of the AES3 stream
        {
            size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

            Info[Info_Offset+0]=  Buffer[Buffer_Pos+1];
            Info[Info_Offset+1]=  Buffer[Buffer_Pos+0];
            Info_Offset+=2;
            Element_Offset+=2;
        }
    }
    if (Container_Bits==24 && Stream_Bits==16 && Endianness=='L')
    {
        while (Element_Offset+6<=Element_Size)
        {
            size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

            Info[Info_Offset+0]=  Buffer[Buffer_Pos+2];
            Info[Info_Offset+1]=  Buffer[Buffer_Pos+1];
            Info[Info_Offset+2]=  Buffer[Buffer_Pos+5];
            Info[Info_Offset+3]=  Buffer[Buffer_Pos+4];
            Info_Offset+=4;
            Element_Offset+=6;
        }
    }
    if (Container_Bits==24 && Stream_Bits==20 && Endianness=='L')
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
    if (Container_Bits==32 && Stream_Bits==16 && Endianness=='L')
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
    if (Container_Bits==32 && Stream_Bits==20 && Endianness=='L')
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
    if (Container_Bits==32 && Stream_Bits==24 && Endianness=='L')
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

    if (Container_Bits==24 && Stream_Bits==20 && Endianness=='B')
    {
        while (Element_Offset+6<=Element_Size)
        {
            size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

            Info[Info_Offset+0]=  Buffer[Buffer_Pos+0];
            Info[Info_Offset+1]=  Buffer[Buffer_Pos+1];
            Info[Info_Offset+2]=((Buffer[Buffer_Pos+2]&0xF0)   ) | ((Buffer[Buffer_Pos+3]&0xF0)>>4);
            Info[Info_Offset+3]=((Buffer[Buffer_Pos+3]&0x0F)<<4) | ((Buffer[Buffer_Pos+4]&0xF0)>>4);
            Info[Info_Offset+4]=((Buffer[Buffer_Pos+4]&0x0F)<<4) | ((Buffer[Buffer_Pos+5]&0xF0)>>4);
            Info_Offset+=5;
            Element_Offset+=6;
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
    int16u audio_packet_size=0;
    Get_B2 (audio_packet_size,                              "audio_packet_size");
    BS_Begin();
    Get_S1 (2, number_channels,                             "number_channels"); Param_Info2(2+2*number_channels, " channels");
    Info_S1(8, channel_identification,                      "channel_identification");
    Get_S1 (2, bits_per_sample,                             "bits_per_sample"); Param_Info2(16+4*bits_per_sample, " bits");
    Info_S1(4, alignment_bits,                              "alignment_bits");
    BS_End();

    //Enough data
    if (Element_Size<4+(int64u)audio_packet_size)
    {
        Element_WaitForMoreData();
        return;
    }
    if (Element_Size!=4+(int64u)audio_packet_size || bits_per_sample==3 || audio_packet_size%((1+number_channels)*(5+bits_per_sample)))
    {
        Trusted_IsNot("Wrong size");
        Skip_XX(Element_Size-4,                             "Problem?");
        return;
    }

    if (!Status[IsAccepted])
    {
        Accept("AES3");
        Container_Bits=Stream_Bits=16+4*bits_per_sample;
        Endianness='L';
        if (Container_Bits%8)
            Container_Bits+=4; //Rounded to next byte
    }

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
            Element_Offset=4;

            #if MEDIAINFO_DEMUX
                FrameInfo.PTS=FrameInfo.DTS;
                if (SampleRate)
                    FrameInfo.DUR=(Element_Size-4)/4*1000000000/(SampleRate*8);
                Demux_random_access=true;
                Element_Code=(int64u)-1;
                Element_Offset=0;
                Demux(Info, Info_Offset, ContentType_MainStream);
                Element_Offset=4;
            #endif //MEDIAINFO_DEMUX

            for (size_t Pos=0; Pos<Info_Offset; Pos++)
                if (Info[Pos])
                {
                    if (Pos+16<Info_Offset && Info[Pos]==0x72 && Info[Pos+1]==0xF8 && Info[Pos+2]==0x1F && Info[Pos+3]==0x4E)
                        IsParsingNonPcm=true;
                    else
                        IsParsingNonPcm=false;

                    break;
                }

            if (IsParsingNonPcm)
            {
                IsParsingNonPcm=true;
                Parser_Parse(Info, Info_Offset);
            }
            else
            {
                Skip_XX(Element_Size-Element_Offset,            "Data");

                #if MEDIAINFO_DEMUX
                    if (FrameInfo.DTS!=(int64u)-1 && FrameInfo.DUR!=(int64u)-1)
                    {
                        FrameInfo.DTS+=FrameInfo.DUR;
                        FrameInfo.PTS=FrameInfo.DTS;
                    }
                #endif //MEDIAINFO_DEMUX
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
            }
            Element_Offset=4;

            #if MEDIAINFO_DEMUX
                //if (StreamIDs_Size==0 || Config->ID_Format_Get(StreamIDs[0]==(int64u)-1?Ztring():Ztring::ToZtring(StreamIDs[0]))==__T("PCM")) //TODO: to but in an "advanced" version
                {
                    FrameInfo.PTS=FrameInfo.DTS;
                    if (SampleRate)
                        FrameInfo.DUR=(Element_Size-4)/4*1000000000/(SampleRate*8);
                    Demux_random_access=true;
                    Element_Code=(int64u)-1;
                    Element_Offset=0;
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

                        Demux(Info2, Info2_Pos, ContentType_MainStream);
                    }
                    else
                        Demux(Info, Info_Offset, ContentType_MainStream);
                    Element_Offset=4;
                }
            #endif //MEDIAINFO_DEMUX

            for (size_t Pos=0; Pos<Info_Offset; Pos++)
                if (Info[Pos])
                {
                    if (Pos+16<Info_Offset && Info[Pos]==0x20 && Info[Pos+1]==0x87 && Info[Pos+2]==0x6F && Info[Pos+3]==0xF0 && Info[Pos+4]==0xE1 && Info[Pos+5]==0x54)
                        IsParsingNonPcm=true;
                    else
                        IsParsingNonPcm=false;

                    break;
                }

            if (IsParsingNonPcm)
            {
                IsParsingNonPcm=true;
                Parser_Parse(Info, Info_Offset);
            }
            else
            {
                Skip_XX(Element_Size-Element_Offset,            "Data");

                #if MEDIAINFO_DEMUX
                    if (FrameInfo.DTS!=(int64u)-1 && FrameInfo.DUR!=(int64u)-1)
                    {
                        FrameInfo.DTS+=FrameInfo.DUR;
                        FrameInfo.PTS=FrameInfo.DTS;
                    }
                #endif //MEDIAINFO_DEMUX
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
            Element_Offset=4;

            #if MEDIAINFO_DEMUX
                FrameInfo.PTS=FrameInfo.DTS;
                if (SampleRate)
                    FrameInfo.DUR=(Element_Size-4)/4*1000000000/(SampleRate*8);
                Demux_random_access=true;
                Element_Code=(int64u)-1;
                Element_Offset=0;
                Demux(Info, Info_Offset, ContentType_MainStream);
                Element_Offset=4;
            #endif //MEDIAINFO_DEMUX

            for (size_t Pos=0; Pos<Info_Offset; Pos++)
                if (Info[Pos])
                {
                    if (Pos+16<Info_Offset && ((Info[Pos]==0x72 && Info[Pos+1]==0xF8 && Info[Pos+2]==0x96 && Info[Pos+3]==0x1F && Info[Pos+4]==0x4E && Info[Pos+5]==0xA5)   //24-bit
                                            || (Info[Pos]==0x20 && Info[Pos+1]==0x87 && Info[Pos+2]==0x6F && Info[Pos+3]==0xF0 && Info[Pos+4]==0xE1 && Info[Pos+5]==0x54))) //20-bit
                        IsParsingNonPcm=true;
                    else
                        IsParsingNonPcm=false;

                    break;
                }

            if (IsParsingNonPcm)
            {
                IsParsingNonPcm=true;
                Parser_Parse(Info, Info_Offset);
            }
            else
            {
                Skip_XX(Element_Size-Element_Offset,            "Data");

                #if MEDIAINFO_DEMUX
                    if (FrameInfo.DTS!=(int64u)-1 && FrameInfo.DUR!=(int64u)-1)
                    {
                        FrameInfo.DTS+=FrameInfo.DUR;
                        FrameInfo.PTS=FrameInfo.DTS;
                    }
                #endif //MEDIAINFO_DEMUX
            }

            delete[] Info;
        }
        break;
        default :
            Skip_XX(Element_Size,                           "Data");
    }

    //Filling
    Frame_Count++;
    if (Frame_Count_NotParsedIncluded!=(int64u)-1)
        Frame_Count_NotParsedIncluded++;
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
        ((File_Aes3*)Parser)->From_Aes3=true;
        if (SampleRate)
            ((File_Aes3*)Parser)->SampleRate=SampleRate;
        #if MEDIAINFO_DEMUX
            if (From_MpegPs) //TODO: separate AES from MPEG-PS/TS
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
        Open_Buffer_Init(Parser);
    }
    Element_Offset=0;
    Parser->FrameInfo=FrameInfo;
    Open_Buffer_Continue(Parser, Parser_Buffer, Parser_Buffer_Size);
    #if MEDIAINFO_DEMUX
        if (Parser->FrameInfo.DUR!=FrameInfo.DUR && Parser->FrameInfo.DUR!=(int64u)-1)
            FrameInfo.DUR=Parser->FrameInfo.DUR;
        if (FrameInfo.DUR!=(int64u)-1)
            FrameInfo.DTS+=FrameInfo.DUR;
        else if (Parser->FrameInfo.DTS!=(int64u)-1)
            FrameInfo=Parser->FrameInfo;
        else
            FrameInfo.DTS=(int64u)-1;
    #endif //MEDIAINFO_DEMUX
    Element_Offset=Element_Size;

    if (!From_MpegPs)
    {
        Frame_Count_InThisBlock++;
        Frame_Count++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
    }

    if (!Status[IsFilled] && Parser->Status[IsFilled])
    {
        //Filling
        Merge(*Parser);
        ZtringList OverallBitRates; OverallBitRates.Separator_Set(0, __T(" / ")); OverallBitRates.Write(Parser->Retrieve(Stream_General, 0, General_OverallBitRate));
        if (!OverallBitRates.empty())
        {
            int64u OverallBitRate=OverallBitRates[0].To_int64u();
            if (OverallBitRate)
            {
                OverallBitRate*=Element_Offset; OverallBitRate/=Parser_Buffer_Size;
                OverallBitRates[0].From_Number(OverallBitRate);
            }
            Fill(Stream_General, 0, General_OverallBitRate, OverallBitRates.Read(), true);
        }
        FrameRate=((File_Aes3*)Parser)->FrameRate;
        Fill("AES3");
        if (!From_MpegPs)
            Finish("AES3");
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AES3_YES
