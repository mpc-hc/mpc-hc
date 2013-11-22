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
#if defined(MEDIAINFO_LXF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Lxf.h"
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
#if defined(MEDIAINFO_VC3_YES)
    #include "MediaInfo/Video/File_Vc3.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_AAC_YES)
    #include "MediaInfo/Audio/File_Aac.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_DTS_YES)
    #include "MediaInfo/Audio/File_Dts.h"
#endif
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_ChannelGrouping.h"
#endif
#if defined(MEDIAINFO_DOLBYE_YES)
    #include "MediaInfo/Audio/File_DolbyE.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_SmpteSt0337.h"
#endif
#if defined(MEDIAINFO_ANCILLARY_YES)
    #include "MediaInfo/Multiple/File_Ancillary.h"
#endif //defined(MEDIAINFO_ANCILLARY_YES)
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include <bitset>
#include <MediaInfo/MediaInfo_Internal.h>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
const char* Lxf_Format_Video[16]=
{
    "JPEG",
    "MPEG Video", //Version 1
    "MPEG Video", //Version 2, 4:2:0
    "MPEG Video", //Version 2, 4:2:2
    "DV", //25 Mbps 4:1:1 or 4:2:0
    "DV", //DVCPRO
    "DV", //DVCPRO 50 / HD
    "RGB", //RGB uncompressed
    "Gray", //Gray uncompressed
    "MPEG Video", //Version 2, 4:2:2, GOP=9
    "AVC",
    "AVC",
    "AVC",
    "AVC",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* Lxf_PictureType[4]=
{
    "I", //Closed
    "I", //Open
    "P",
    "B",
};

//---------------------------------------------------------------------------
extern const float32 Mpegv_frame_rate[]; //In Video/File_Mpegv.cpp

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Lxf::File_Lxf()
:File__Analyze()
{
    //Configuration
    ParserName=__T("LXF");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Lxf;
        StreamIDs_Width[0]=4; //2 numbers for Code, 2 numbers for subcode
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    MustSynchronize=true;
    Buffer_TotalBytes_Fill_Max=(int64u)-1; //Disabling this feature for this format, this is done in the parser
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Streams
    #if defined(MEDIAINFO_ANCILLARY_YES)
        Ancillary=NULL;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)

    //Temp
    LookingForLastFrame=false;
    Stream_Count=0;
    Info_General_StreamSize=0;
    Video_Sizes_Pos=(size_t)-1;
    Audio_Sizes_Pos=(size_t)-1;

    //Demux
    #if MEDIAINFO_DEMUX
        DemuxParser=NULL;
    #endif //MEDIAINFO_DEMUX

    //Seek
    #if MEDIAINFO_SEEK
        SeekRequest=(int64u)-1;
    #endif //MEDIAINFO_SEEK
    FrameRate=0;
    TimeStamp_Rate=720000;
    Duration_Detected=false;
    LastAudio_BufferOffset=(int64u)-1;
}

//---------------------------------------------------------------------------
File_Lxf::~File_Lxf()
{
    for (size_t Pos=0; Pos<Videos.size(); Pos++)
        for (size_t Pos2=0; Pos2<Videos[Pos].Parsers.size(); Pos2++)
            delete Videos[Pos].Parsers[Pos2];
    for (size_t Pos=0; Pos<Audios.size(); Pos++)
        for (size_t Pos2=0; Pos2<Audios[Pos].Parsers.size(); Pos2++)
            delete Audios[Pos].Parsers[Pos2];
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lxf::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format_Version, __T("Version "+Ztring::ToZtring(Version)));

    for (size_t Pos=2; Pos<Videos.size(); Pos++) //TODO: better handling of fill/finish for Ancillary data
        if (Videos[Pos].Parsers.size()==1)
            Streams_Fill_PerStream(Videos[Pos].Parsers[0], Stream_Video, Pos, Videos[Pos].Format);
    for (size_t Pos=0; Pos<Audios.size(); Pos++)
        if (Audios[Pos].Parsers.size()==1)
            Streams_Fill_PerStream(Audios[Pos].Parsers[0], Stream_Audio, Pos, Audios[Pos].Format);

    //FrameRate
    if (FrameRate && Retrieve(Stream_Video, 0, Video_FrameRate).empty())
        Fill(Stream_Video, 0, Video_FrameRate, FrameRate, 3);
}

//---------------------------------------------------------------------------
void File_Lxf::Streams_Fill_PerStream(File__Analyze* Parser, stream_t Container_StreamKind, size_t Parser_Pos, int8u Format)
{
    if (Parser==NULL)
        return;

    Fill(Parser);
    if (Parser->Count_Get(Stream_Audio) && Config->File_Audio_MergeMonoStreams_Get() && Parser->Retrieve(Stream_Audio, 0, Audio_Format)==__T("PCM"))
    {
        if (Count_Get(Stream_Audio)==0)
        {
            Merge(*Parser);
            Fill(Stream_Audio, 0, Audio_Channel_s_, Audio_Sizes.size(), 10, true);
            int64u BitRate=Retrieve(Stream_Audio, 0, Audio_BitRate).To_int64u();
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate*Audio_Sizes.size(), 10, true);
            #if MEDIAINFO_DEMUX
                if (Config->Demux_ForceIds_Get())
                {
                    for (size_t Audio_Pos=0; Audio_Pos<Audio_Sizes.size(); Audio_Pos++)
                        Fill(StreamKind_Last, StreamPos_Last, General_ID, 0x200+Audio_Pos);
                }
            #endif //MEDIAINFO_DEMUX
        }
    }
    else
    {
        Merge(*Parser);

        Ztring LawRating=Parser->Retrieve(Stream_General, 0, General_LawRating);
        if (!LawRating.empty())
            Fill(Stream_General, 0, General_LawRating, LawRating, true);

        #if MEDIAINFO_DEMUX
            if (Config->Demux_ForceIds_Get())
                for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
                    for (size_t StreamPos=0; StreamPos<Parser->Count_Get((stream_t)StreamKind); StreamPos++)
                    {
                        Ztring ID;
                        if (Parser->Count_Get(Stream_Audio) && Parser->Retrieve(Stream_Audio, 0, Audio_MuxingMode)==__T("AES3") && Parser_Pos%2)
                            ID+=Ztring::ToZtring(0x100*Container_StreamKind+Parser_Pos-1)+__T(" / ");
                        ID+=Ztring::ToZtring(0x100*Container_StreamKind+Parser_Pos);
                        if (!Parser->Retrieve((stream_t)StreamKind, StreamPos, General_ID).empty())
                            ID+=__T('-')+Parser->Retrieve((stream_t)StreamKind, StreamPos, General_ID);
                        Fill((stream_t)StreamKind, Count_Get((stream_t)StreamKind)-Parser->Count_Get((stream_t)StreamKind)+StreamPos, General_ID, ID, true);
                    }
        #endif //MEDIAINFO_DEMUX
    }
    if (Format!=(int8u)-1)
        Fill(Container_StreamKind, Container_StreamKind==Stream_Video?0:Parser_Pos, Fill_Parameter(Container_StreamKind, Generic_CodecID), Format);
    if (Container_StreamKind==Stream_Video)
        for (size_t Pos=Count_Get(Stream_Audio)-Parser->Count_Get(Stream_Audio); Pos<Count_Get(Stream_Audio); Pos++)
            Fill(Stream_Audio, Pos, Audio_MuxingMode, Parser->Retrieve(Stream_General, 0, General_Format));
}

//---------------------------------------------------------------------------
void File_Lxf::Streams_Finish()
{
    if (Videos.size()>1 && Videos[1].Parsers.size()==1) //TODO: better handling of fill/finish for Ancillary data
    {
        Finish(Videos[1].Parsers[0]);
        Streams_Fill_PerStream(Videos[1].Parsers[0], Stream_Video, 1);
    }
    if (Videos.size()>2 && Videos[2].Parsers.size()==1)
    {
        Finish(Videos[2].Parsers[0]);
        Merge(*Videos[2].Parsers[0], Stream_Video, 0, 0);

        Ztring LawRating=Videos[2].Parsers[0]->Retrieve(Stream_General, 0, General_LawRating);
        if (!LawRating.empty())
            Fill(Stream_General, 0, General_LawRating, LawRating, true);
    }

    if (Audios_Header.TimeStamp_End!=(int64u)-1 && Audios_Header.TimeStamp_Begin!=(int64u)-1 && Audios_Header.Duration_First!=(int64u)-1)
    {
        int64u Duration=float64_int64s(((float64)(Audios_Header.TimeStamp_End-Audios_Header.TimeStamp_Begin))/TimeStamp_Rate*1000);
        int64u FrameCount=float64_int64s(((float64)(Audios_Header.TimeStamp_End-Audios_Header.TimeStamp_Begin))/Audios_Header.Duration_First);
        for (size_t Pos=0; Pos<Count_Get(Stream_Audio); Pos++)
        {
            if (Retrieve(Stream_Audio, Pos, Audio_Duration).empty())
                Fill(Stream_Audio, Pos, Audio_Duration, Duration);
            if (Retrieve(Stream_Audio, Pos, Audio_FrameCount).empty())
                Fill(Stream_Audio, Pos, Audio_FrameCount, FrameCount);
        }
        Info_General_StreamSize+=FrameCount*0x48;
    }
    if (Videos_Header.TimeStamp_End!=(int64u)-1 && Videos_Header.TimeStamp_Begin!=(int64u)-1)
    {
        int64u Duration=float64_int64s(((float64)(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin))/TimeStamp_Rate*1000);
        int64u FrameCount=float64_int64s(((float64)(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin))/Videos_Header.Duration);
        if (Duration)
            for (size_t Pos=0; Pos<Count_Get(Stream_Video); Pos++)
                Fill(Stream_Video, Pos, Video_Duration, Duration, 10, true);
        Info_General_StreamSize+=FrameCount*0x48;

        if (Count_Get(Stream_Video)==1 && Retrieve(Stream_Video, 0, Video_BitRate).empty())
        {
            for (size_t Pos=0; Pos<Videos.size(); Pos++)
                if (Videos[Pos].BytesPerFrame!=(int64u)-1)
                    Info_General_StreamSize+=Videos[Pos].BytesPerFrame*FrameCount;
            for (size_t Pos=0; Pos<Audios.size(); Pos++)
                if (Audios[Pos].BytesPerFrame!=(int64u)-1)
                    Info_General_StreamSize+=Audios[Pos].BytesPerFrame*Retrieve(Stream_Audio, Pos, Audio_FrameCount).To_int64u();
            Fill(Stream_General, 0, General_StreamSize, Info_General_StreamSize);
            if (Info_General_StreamSize<File_Size)
                Fill(Stream_Video, 0, Video_StreamSize, File_Size-Info_General_StreamSize);
        }
    }
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Lxf::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+20<=Buffer_Size && ( Buffer[Buffer_Offset  ]!=0x4C
                                          ||  Buffer[Buffer_Offset+ 1]!=0x45
                                          ||  Buffer[Buffer_Offset+ 2]!=0x49
                                          ||  Buffer[Buffer_Offset+ 3]!=0x54
                                          ||  Buffer[Buffer_Offset+ 4]!=0x43
                                          ||  Buffer[Buffer_Offset+ 5]!=0x48
                                          ||  Buffer[Buffer_Offset+ 6]!=0x00
                                          ||  Buffer[Buffer_Offset+ 7]!=0x00
                                         ))
    {
        Buffer_Offset+=6+2;
        while (Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x00)
            Buffer_Offset+=2;
        if (Buffer_Offset>=Buffer_Size || Buffer[Buffer_Offset-1]==0x00)
            Buffer_Offset--;
        Buffer_Offset-=6;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+20>Buffer_Size)
    {
        while (Buffer_Offset+8>Buffer_Size)
            if (Buffer_Offset+8==Buffer_Size && CC8(Buffer+Buffer_Offset)!=0x4C45495443480000LL)
                Buffer_Offset++;
            else
                break;
        if (Buffer_Offset+7==Buffer_Size && CC7(Buffer+Buffer_Offset)!=0x4C454954434800LL)
            Buffer_Offset++;
        if (Buffer_Offset+6==Buffer_Size && CC6(Buffer+Buffer_Offset)!=0x4C4549544348LL)
            Buffer_Offset++;
        if (Buffer_Offset+5==Buffer_Size && CC5(Buffer+Buffer_Offset)!=0x4C45495443LL)
            Buffer_Offset++;
        if (Buffer_Offset+4==Buffer_Size && CC4(Buffer+Buffer_Offset)!=0x4C454954)
            Buffer_Offset++;
        if (Buffer_Offset+3==Buffer_Size && CC3(Buffer+Buffer_Offset)!=0x4C4549)
            Buffer_Offset++;
        if (Buffer_Offset+2==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x4C45)
            Buffer_Offset++;
        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0x4C)
            Buffer_Offset++;
        return false;
    }

    if (!Status[IsAccepted])
    {
        Accept();

        Fill(Stream_General, 0, General_Format, "LXF");

        File_Buffer_Size_Hint_Pointer=Config->File_Buffer_Size_Hint_Pointer_Get();
    }

    #if MEDIAINFO_SEEK
        //TimeStamp
        if (SeekRequest!=(int64u)-1)
        {
            if (TimeOffsets.find(File_Offset+Buffer_Offset)==TimeOffsets.end()) //Not already saved
            {
                if (Buffer_Offset+0x48>=Buffer_Size)
                    return false;
                int32u Type       =LittleEndian2int32u(Buffer+Buffer_Offset+16);
                if (Type==0) //Video
                {
                    //Filling with the new frame
                    Version=LittleEndian2int32u(Buffer+Buffer_Offset+8);
                    int64u TimeStamp, Duration;
                    switch (Version)
                    {
                        case 0 : TimeStamp  =LittleEndian2int32u(Buffer+Buffer_Offset+24);
                                 Duration   =LittleEndian2int32u(Buffer+Buffer_Offset+28);
                                 break;
                        case 1 : TimeStamp  =LittleEndian2int64u(Buffer+Buffer_Offset+24);
                                 Duration   =LittleEndian2int64u(Buffer+Buffer_Offset+32);
                                 break;
                        default: TimeStamp=Duration=0;
                    }
                    int8u  PictureType=(LittleEndian2int8u (Buffer+Buffer_Offset+42)&0xC0)>>6;
                    TimeOffsets[File_Offset+Buffer_Offset]=stream_header(TimeStamp, TimeStamp+Duration, Duration, PictureType);
                    SeekRequest_Divider=2;
                }
            }
            if (Read_Buffer_Seek(2, (int64u)-1, (int64u)-1))
                return false;
        }
    #endif //MEDIAINFO_SEEK

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Lxf::Synched_Test()
{
    if (Video_Sizes_Pos<Video_Sizes.size())
        return true;
    if (Audio_Sizes_Pos<Audio_Sizes.size())
        return true;

    //Must have enough buffer for having header
    if (Buffer_Offset+16>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC8(Buffer+Buffer_Offset)!=0x4C45495443480000LL)
        Synched=false;

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lxf::Read_Buffer_Unsynched()
{
    Video_Sizes.clear();
    Audio_Sizes.clear();
    LastAudio_BufferOffset=(int64u)-1;
    LastAudio_TimeOffset=stream_header();
    Video_Sizes_Pos=(size_t)-1;
    Audio_Sizes_Pos=(size_t)-1;
    Videos_Header.TimeStamp_End=(int64u)-1;
    Audios_Header.TimeStamp_End=(int64u)-1;

    for (size_t Pos=0; Pos<Videos.size(); Pos++)
        for (size_t Pos2=0; Pos2<Videos[Pos].Parsers.size(); Pos2++)
            Videos[Pos].Parsers[Pos2]->Open_Buffer_Unsynch();
    for (size_t Pos=0; Pos<Audios.size(); Pos++)
        for (size_t Pos2=0; Pos2<Audios[Pos].Parsers.size(); Pos2++)
            Audios[Pos].Parsers[Pos2]->Open_Buffer_Unsynch();
}

//---------------------------------------------------------------------------
void File_Lxf::Read_Buffer_Continue()
{
    #if MEDIAINFO_DEMUX
        if (DemuxParser)
        {
            Open_Buffer_Continue(DemuxParser, Buffer+Buffer_Offset, 0, false);
            if (!Config->Demux_EventWasSent)
                DemuxParser=NULL; //No more need of it
        }
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_Lxf::Read_Buffer_Seek (size_t Method, int64u Value, int64u)
{
    //Init
    if (!Duration_Detected)
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
        if (!MiOpenResult || MI.Get(Stream_General, 0, General_Format)!=__T("LXF"))
            return 0;
        for (time_offsets::iterator TimeOffset=((File_Lxf*)MI.Info)->TimeOffsets.begin(); TimeOffset!=((File_Lxf*)MI.Info)->TimeOffsets.end(); ++TimeOffset)
            TimeOffsets[TimeOffset->first]=TimeOffset->second;
        int64u Duration=float64_int64s(Ztring(MI.Get(Stream_General, 0, __T("Duration"))).To_float64()*TimeStamp_Rate/1000);
        TimeOffsets[File_Size]=stream_header(Duration, Duration, 0, (int8u)-1);
        SeekRequest_Divider=2;
        Duration_Detected=true;
    }

    //Parsing
    switch (Method)
    {
        case 0  :   Open_Buffer_Unsynch(); GoTo(Value); return 1;
        case 1  :   Open_Buffer_Unsynch(); GoTo(File_Size*Value/10000); return 1;
        case 3  :   //Frame
                    {
                        if (FrameRate==0 && Videos_Header.TimeStamp_End!=(int64u)-1 && Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin!=0)
                            FrameRate=TimeStamp_Rate/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin);
                        if (FrameRate==0)
                            return (size_t)-1; //Not supported
                        float64 TimeStamp=((float64)Value)/FrameRate;
                        Value=float64_int64s(TimeStamp*1000000000); // In nanoseconds
                    }
        case 2  :   //Timestamp
                    {
                    if (Value!=(int64u)-1)
                    {
                        Value=float64_int64s((float64)Value*TimeStamp_Rate/1000000000); //Convert in LXF unit
                        time_offsets::iterator End=TimeOffsets.end();
                        --End;
                        if (Value>=End->second.TimeStamp_End)
                            return 2; //Higher than total size
                        SeekRequest=Value;
                    }

                    //Looking if we already have the timestamp
                    int64u SeekRequest_Mini=SeekRequest; if (SeekRequest_Mini>1000000) SeekRequest_Mini-=float64_int64s(TimeStamp_Rate/1000); //-1ms
                    int64u SeekRequest_Maxi=SeekRequest+float64_int64s(TimeStamp_Rate/1000); //+1ms
                    for (time_offsets::iterator TimeOffset=TimeOffsets.begin(); TimeOffset!=TimeOffsets.end(); ++TimeOffset)
                    {
                        if (TimeOffset->second.TimeStamp_Begin<=SeekRequest_Maxi && TimeOffset->second.TimeStamp_End>=SeekRequest_Mini) //If it is found in a frame we know
                        {
                            //Looking for the corresponding I-Frame
                            while (TimeOffset->second.PictureType&0x2 && TimeOffset!=TimeOffsets.begin()) //Not an I-Frame (and not fisrt frame)
                            {
                                time_offsets::iterator Previous=TimeOffset;
                                --Previous;
                                if (Previous->second.TimeStamp_End!=TimeOffset->second.TimeStamp_Begin) //Testing if the previous frame is not known.
                                {
                                    SeekRequest=TimeOffset->second.TimeStamp_Begin-(float64_int64s(TimeStamp_Rate/1000)+1); //1ms+1, so we are sure to not synch on the current frame again
                                    Open_Buffer_Unsynch();
                                    GoTo((Previous->first+TimeOffset->first)/2);
                                    return 1; //Looking for previous frame

                                }
                                TimeOffset=Previous;
                            }

                            //We got the right I-Frame
                            if (Value==0)
                            {
                                for (size_t Pos=0; Pos<Videos.size(); Pos++)
                                    for (size_t Pos2=0; Pos2<Videos[Pos].Parsers.size(); Pos2++)
                                        Videos[Pos].Parsers[Pos2]->Unsynch_Frame_Count=0;
                            }
                            Open_Buffer_Unsynch();
                            GoTo(TimeOffset->first);
                            SeekRequest=(int64u)-1;
                            return 1;
                        }

                        if (TimeOffset->second.TimeStamp_Begin>SeekRequest_Maxi) //Testing if too far
                        {
                            time_offsets::iterator Previous=TimeOffset; --Previous;
                            int64u ReferenceOffset;
                            if (File_Offset+Buffer_Offset==TimeOffset->first && TimeOffset->second.TimeStamp_Begin>SeekRequest) //If current frame is already too far
                                ReferenceOffset=File_Offset+Buffer_Offset;
                            else
                                ReferenceOffset=TimeOffset->first;
                            if (SeekRequest_Divider==0)
                            {
                                SeekRequest=Previous->second.TimeStamp_Begin-(float64_int64s(TimeStamp_Rate/1000)+1); //1ms+1, so we are sure to not synch on the current frame again
                                ReferenceOffset=Previous->first;
                                --Previous;
                                SeekRequest_Divider=2;
                            }
                            Open_Buffer_Unsynch();
                            GoTo(Previous->first+(ReferenceOffset-Previous->first)/SeekRequest_Divider);
                            SeekRequest_Divider*=2;
                            return 1;
                        }
                    }
                    }
                    return 0;
        default :   return (size_t)-1;
    }
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Lxf::Header_Begin()
{
    if (Buffer_Offset+16>Buffer_Size)
        return false;

    return true;
}

//---------------------------------------------------------------------------
void File_Lxf::Header_Parse()
{
    while (Video_Sizes_Pos<Video_Sizes.size() && Video_Sizes[Video_Sizes_Pos]==0)
        Video_Sizes_Pos++;
    if (Video_Sizes_Pos<Video_Sizes.size())
    {
        //Filling
        Header_Fill_Code(0x100+Video_Sizes_Pos, __T("Stream"));
        Header_Fill_Size(Video_Sizes[Video_Sizes_Pos]);
        Video_Sizes_Pos++;
        return;
    }
    while (Audio_Sizes_Pos<Audio_Sizes.size() && Audio_Sizes[Audio_Sizes_Pos]==0)
        Audio_Sizes_Pos++;
    if (Audio_Sizes_Pos<Audio_Sizes.size())
    {
        //Filling
        Header_Fill_Code(0x200+Audio_Sizes_Pos, __T("Stream"));
        Header_Fill_Size(Audio_Sizes[Audio_Sizes_Pos]);
        Audio_Sizes_Pos++;
        return;
    }

    //Parsing
    int64u BlockSize=0, TimeStamp=0, Duration=0;
    int32u HeaderSize, Type;
    Skip_C8(                                                    "Signature");
    Get_L4 (Version,                                            "Version"); //0=start and duration are in field, 1=in 27 MHz values
    Get_L4 (HeaderSize,                                         "Header size");
    if (Element_Size<HeaderSize)
    {
        Element_WaitForMoreData();
        return;
    }
    if (Version>1)
    {
        //Filling
        Header_Fill_Code(0, "Unknown");
        Header_Fill_Size(HeaderSize);
        Synched=false;
        return;
    }
    Get_L4 (Type,                                               "Type");
    Skip_L4(                                                    "Stream ID");
    switch (Version)
    {
        case 0 :
                    {
                    int32u TimeStamp4, Duration4;
                    Get_L4 (TimeStamp4,                         "TimeStamp");
                    TimeStamp=TimeStamp4;
                    Param_Info3(((float64)TimeStamp4)/TimeStamp_Rate, 3, " s");
                    FrameInfo.DTS=FrameInfo.PTS=float64_int64s(((float64)TimeStamp)*1000000000/TimeStamp_Rate);
                    Get_L4 (Duration4,                          "Duration");
                    Duration=Duration4;
                    Param_Info3(((float64)Duration)/TimeStamp_Rate, 3, " s");
                    FrameInfo.DUR=float64_int64s(((float64)Duration)*1000000000/TimeStamp_Rate);
                    }
                    break;
        case 1 :
                    Get_L8 (TimeStamp,                          "TimeStamp"); Param_Info3(((float64)TimeStamp)/720000, 3, " s"); FrameInfo.DTS=FrameInfo.PTS=float64_int64s(((float64)TimeStamp)*1000000/720);
                    Get_L8 (Duration,                           "Duration"); Param_Info3(((float64)Duration)/720000, 3, " s"); FrameInfo.DUR=float64_int64s(((float64)Duration)*1000000/720);
                    break;
        default:    ;
    }
    switch(Type)
    {
        case 0  :   //Video
                    {
                    Video_Sizes.resize(3);
                    int32u Size;
                    int8u VideoFormat, GOP_M, PictureType;
                    BS_Begin_LE();
                    Get_T1 (4, VideoFormat,                     "Format"); Param_Info1(Lxf_Format_Video[VideoFormat]);
                    Skip_T1(7,                                  "GOP (N)");
                    Get_T1 (3, GOP_M,                           "GOP (M)");
                    Info_T1(8, BitRate,                         "Bit rate"); Param_Info2((BitRate*(BitRate>60?10:(BitRate>50?5:1))-(BitRate>60?500:(BitRate>50?200:0)))*1000000, " bps");
                    Get_T1 (2, PictureType,                     "Picture type"); Param_Info1(Lxf_PictureType[PictureType]);
                    BS_End_LE();
                    Skip_L1(                                    "Reserved");
                    Get_L4(Size,                                "Video data size");
                    Skip_L4(                                    "Zero");
                    if (!Video_Sizes.empty())
                        Video_Sizes[2]=Size;
                    BlockSize+=Size;
                    Get_L4(Size,                                "VBI data size");
                    if (!Video_Sizes.empty())
                        Video_Sizes[1]=Size;
                    BlockSize+=Size;
                    Skip_L4(                                    "Zero");
                    Get_L4(Size,                                "Meta data size");
                    if (!Video_Sizes.empty())
                        Video_Sizes[0]=Size;
                    BlockSize+=Size;
                    if (Videos_Header.TimeStamp_Begin==(int64u)-1)
                        Videos_Header.TimeStamp_Begin=TimeStamp;
                    Videos_Header.TimeStamp_End=TimeStamp+Duration;
                    Videos_Header.Duration=Duration;
                    if (TimeStamp==LastAudio_TimeOffset.TimeStamp_Begin)
                        TimeOffsets[LastAudio_BufferOffset]=stream_header(TimeStamp, TimeStamp+Duration, Duration, PictureType);
                    else
                        TimeOffsets[File_Offset+Buffer_Offset]=stream_header(TimeStamp, TimeStamp+Duration, Duration, PictureType);
                    int64u PTS_Computing=TimeStamp;
                    #if MEDIAINFO_DEMUX
                        switch (PictureType)
                        {
                            case 2 :
                            case 3 : Demux_random_access=false; break; //P-Frame, B-Frame
                            default: Demux_random_access=true ;        //I-Frame
                        }
                    #endif //MEDIAINFO_DEMUX
                    if (2>Videos.size())
                        Videos.resize(2+1);
                    if (!Video_Sizes.empty())
                        Videos[2].Format=VideoFormat;
                    if (GOP_M>1) //With B-frames
                    {
                        switch (PictureType)
                        {
                            case 2 : PTS_Computing+=GOP_M*Duration; break; //P-Frame
                            case 3 :                                break; //B-Frame
                            default: PTS_Computing+=Duration;              //I-Frame
                        }
                    }
                    FrameInfo.PTS=float64_int64s(((float64)PTS_Computing)*1000000000/TimeStamp_Rate);
                    }
                    break;
        case 1  :   //Audio
                    {
                    int32u Size;
                    int8u Channels_Count=0;

                    if (Version==0)
                    {
                        Skip_L4(                                "First Active Field");
                        Skip_L4(                                "Total fields in packet");
                    }
                    BS_Begin_LE();
                    Get_T1 ( 6, SampleSize,                     "Sample size");
                    Skip_T1( 6,                                 "Sample precision");
                    Skip_T1(20,                                 "Reserved");
                    BS_End_LE();
                    Element_Begin1("Tracks mask");
                        BS_Begin_LE();
                        for (size_t Pos=0; Pos<32; Pos++)
                        {
                            bool Channel;
                            Get_TB(Channel,                     "Channel");
                            if (Channel)
                                Channels_Count++;
                        }
                        BS_End_LE();
                    Element_End0();
                    Get_L4(Size,                                "Track size");
                    Skip_L4(                                    "Zero");
                    if (Version>=1)
                    {
                        Skip_L4(                                "Zero");
                        Skip_L4(                                "Zero");
                    }
                    Audio_Sizes.resize(Channels_Count);
                    for (size_t Pos=0; Pos<Audio_Sizes.size(); Pos++)
                        Audio_Sizes[Pos]=Size;
                    BlockSize=Size*Channels_Count;
                    if (Audios_Header.TimeStamp_Begin==(int64u)-1)
                        Audios_Header.TimeStamp_Begin=TimeStamp;
                    Audios_Header.TimeStamp_End=TimeStamp+Duration;
                    Audios_Header.Duration=Duration;
                    if (Audios_Header.Duration_First==(int64u)-1 && Duration)
                        Audios_Header.Duration_First=Duration;
                    LastAudio_BufferOffset=File_Offset+Buffer_Offset;
                    LastAudio_TimeOffset=stream_header(TimeStamp, TimeStamp+Duration, Duration, (int8u)-1);
                    #if MEDIAINFO_DEMUX
                        Demux_random_access=true;
                    #endif //MEDIAINFO_DEMUX
                    }
                    break;
        case 2  :   //Header
                    {
                    Header_Sizes.resize(2);
                    int32u SegmentFormat, Size;

                    Get_L4 (SegmentFormat,                      "Segment format");
                    Get_L4 (Size,                               "Data size");
                    Header_Sizes[0]=Size;
                    BlockSize+=Size;
                    if (SegmentFormat)
                    {
                        Get_L4 (Size,                           "Extended fields size");
                        Header_Sizes[1]=Size;
                        BlockSize+=Size;
                    }
                    Skip_L4(                                    "Zero");
                    Skip_L4(                                    "Zero");
                    Skip_L4(                                    "Zero");
                    }
                    break;
        default :   BlockSize=0;
    }
    Skip_L4(                                                    "Checksum");
    if (Version)
        Skip_L4(                                                "Zero");

    if (Element_Offset<HeaderSize)
        Skip_XX(Header_Size-Element_Offset,                     "Unknown");

    if (Buffer_Offset+Element_Offset+BlockSize>Buffer_Size)
    {
        //Hints
        if (File_Buffer_Size_Hint_Pointer)
        {
            size_t Buffer_Size_Target=(size_t)(Buffer_Offset+0x48+BlockSize+0x48); //+0x48 for next packet header
            if ((*File_Buffer_Size_Hint_Pointer)<Buffer_Size_Target)
                (*File_Buffer_Size_Hint_Pointer)=Buffer_Size_Target;
        }
    }

    //Filling
    Header_Fill_Code(Type, Ztring::ToZtring(Type));
    Header_Fill_Size(HeaderSize+BlockSize);
}

//---------------------------------------------------------------------------
void File_Lxf::Data_Parse()
{
    switch(Element_Code)
    {
        case 0  : Video(); break;
        case 1  : Audio(); break;
        case 2  : Header(); break;
        default :
                    if (Element_Code&0x000100)
                        Video_Stream(Element_Code&0xFF);
                    else if (Element_Code&0x000200)
                        Audio_Stream(Element_Code&0xFF);
                    else
                        Skip_XX(Element_Size,                   "Unknown");
    }

    FILLING_BEGIN();
        if (Element_Code&0x000100 && (Element_Code&0xFF)==2) //Checking Video stream 2
        {
            Frame_Count++;
            if (!Status[IsFilled] && ((Frame_Count>6 && (Stream_Count==0 ||Config->ParseSpeed==0.0)) || Frame_Count>512)) //5 video frames for 1 Audio frame
            {
                Fill("LXF");
                if (MediaInfoLib::Config.ParseSpeed_Get()<1)
                {
                    LookingForLastFrame=true;
                    if (3*(File_Offset+Buffer_Offset)<=File_Size)
                    {
                        GoToFromEnd((File_Offset+Buffer_Offset)*2*6/Frame_Count);
                        Open_Buffer_Unsynch();
                    }
                }
            }
        }
    FILLING_END();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lxf::Header()
{
    Element_Name("Header");

    for (size_t Pos=0; Pos<Header_Sizes.size(); Pos++)
    {
        switch(Pos)
        {
            case  0 : Header_Info(); break;
            case  1 : Header_Meta(); break;
            default : Skip_XX(Header_Sizes[Pos],                       "Data");
        }
    }
    Header_Sizes.clear();

    Info_General_StreamSize=0x48+Element_Size;

    #if MEDIAINFO_DEMUX
        if (Config->NextPacket_Get() && Config->Event_CallBackFunction_IsSet())
            Config->Demux_EventWasSent=true; //First set is to indicate the user that header is parsed
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
void File_Lxf::Header_Info()
{
    Element_Begin1("Disk segment");

    //Parsing
    int64u End=Element_Offset+Header_Sizes[0];
    if (Header_Sizes[0]>=120)
    {
        Skip_L4(                                                "prev");
        Skip_L4(                                                "next");
        Skip_L4(                                                "videoClusters");
        Skip_L4(                                                "audioClusters");
        Skip_C8(                                                "ID");
        Skip_L4(                                                "minFrame");
        Skip_L4(                                                "start");
        Skip_L4(                                                "duration");
        Skip_L4(                                                "tcOffset");
        BS_Begin_LE();
        Skip_T1(4,                                              "Format");
        Skip_T1(7,                                              "GOP (N)");
        Skip_T1(3,                                              "GOP (M)");
        Skip_T1(8,                                              "Bit rate");
        Skip_TB(                                                "VBI present");
        Skip_TB(                                                "Aspect Ratio");
        BS_End_LE();
        Skip_L1(                                                "reserved");
        Skip_L4(                                                "base");
        Skip_L4(                                                "prev");
        Skip_L4(                                                "next");
        BS_Begin_LE();
        Skip_T1(7,                                              "recordDate - Year");
        Skip_T1(4,                                              "recordDate - Month");
        Skip_T1(5,                                              "recordDate - Day");
        Skip_T1(7,                                              "killDate - Year");
        Skip_T1(4,                                              "killDate - Month");
        Skip_T1(5,                                              "killDate - Day");
        BS_End_LE();
        Skip_L1(                                                "tc_type");
        Skip_L1(                                                "status");
        Skip_L1(                                                "disk");
        Skip_String(26,                                         "description");
        Skip_String(16,                                         "agency");
        Skip_String( 6,                                         "description");
        Skip_L1(                                                "videoGain");
        Skip_L1(                                                "videoSetup");
        Skip_L1(                                                "chromaGain");
        Skip_L1(                                                "hueLSB");
        Skip_L1(                                                "reserved");
        BS_Begin_LE();
        Skip_T1(2,                                              "hueMSB");
        Skip_T1(4,                                              "audioTracks");
        Skip_TB(                                                "writeProtected");
        Skip_TB(                                                "allocated");
        Skip_TB(                                                "sliding");
        Skip_TB(                                                "tcTranslate");
        Skip_TB(                                                "invisible");
        Skip_TB(                                                "macro");
        Skip_TB(                                                "alpha");
        Skip_TB(                                                "project");
        Skip_TB(                                                "purged");
        Skip_TB(                                                "reference");
        Skip_TB(                                                "looping");
        Skip_TB(                                                "notReadyToPlay");
        Skip_TB(                                                "notReadyToTransfer");
        Skip_TB(                                                "notReadyToArchive");
        Skip_TB(                                                "transferInProgress");
        Skip_T2(11,                                             "reserved");
        BS_End_LE();
    }
    if (Element_Offset<End)
        Skip_XX(End-Element_Offset,                             "Unknown");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Lxf::Header_Meta()
{
    Element_Begin1("Extended fields area");

    int64u Offset=0;
    int64u Pos=0;

    while (Offset<Header_Sizes[1])
    {
        int8u Size;
        Get_L1 (Size,                                           "Size");
        if (Size)
        {
            switch (Pos)
            {
                case  0 :   //Record Date/Time
                            {
                            Skip_XX(Size,                       "Record Date/Time");
                            }
                            break;
                case  1 :   //Codec Where Recorded
                            {
                            Ztring Library;
                            Get_UTF8(Size, Library,             "Codec Where Recorded");
                            Fill(Stream_General, 0, General_Encoded_Library, Library);
                            }
                            break;
                case  2 :   //Source Handle
                            {
                            Skip_XX(Size,                       "Source Handle");
                            }
                            break;
                case  3 :   //UMID
                            {
                            Skip_XX(Size,                       "UMID");
                            }
                            break;
                case  4 :   //Video size / rate info
                            {
                                if (Size==0x10)
                                {
                                    Element_Begin1("Video size / rate info");
                                    BS_Begin_LE();
                                    Element_Begin1("formatCode");
                                    int8u formatCode=(int8u)-1;
                                    for (int8u Pos=0; Pos<96; Pos++)
                                    {
                                        bool Temp;
                                        Get_TB (Temp,            "formatCode bit");
                                        if (Temp)
                                        {
                                            if (formatCode==(int8u)-1)
                                                formatCode=Pos;
                                            else
                                                formatCode=(int8u)-2; //problem
                                        }
                                    }
                                    if (formatCode<96)
                                    {
                                        int8u frameRateCode=formatCode%8;
                                        #if MEDIAINFO_TRACE
                                            int8u temp=formatCode/8;
                                            int8u scanType=temp%2;
                                            int8u verticalsizeCode=temp/2;
                                            Element_Info(verticalsizeCode);
                                            Element_Info(scanType);
                                        #endif //MEDIAINFO_TRACE
                                        if (frameRateCode<16-1)
                                        {
                                            FrameRate=Mpegv_frame_rate[frameRateCode+1];
                                            if (Version==0)
                                                TimeStamp_Rate=FrameRate*2; //Time stamp is in fields
                                            Element_Info3(FrameRate, 3, " fps");
                                        }
                                    }
                                    Element_End0();
                                    Skip_TB(                    "field");
                                    Skip_TB(                    "interlaced");
                                    Skip_TB(                    "progressive");
                                    Skip_TB(                    "pulldown");
                                    Skip_TB(                    "chroma 420");
                                    Skip_TB(                    "chroma 422");
                                    Skip_TB(                    "chroma 311");
                                    Skip_TB(                    "PAR 1:1");
                                    Skip_TB(                    "PAR 4:3");
                                    Skip_T4(23,                 "Zero");
                                    BS_End_LE();
                                    Element_End0();
                                }
                                else
                                    Skip_XX(Size,               "Video size / rate info");
                            }
                            break;
                case  5 :   //Source Video Info
                            {
                            Skip_XX(Size,                       "Source Video Info");
                            }
                            break;
                case  6 :   //GUID
                            {
                            Skip_XX(Size,                       "GUID");
                            }
                            break;
                case  7 :   //User Name
                            {
                            Ztring Channel;
                            Get_UTF16L(Size, Channel,           "User Name");
                            Fill(Stream_General, 0, General_EncodedBy, Channel);
                            }
                            break;
                case  8 :   //Department
                            {
                            Skip_UTF16L(Size,                   "Department");
                            }
                            break;
                case  9 :   //Reserved
                case 10 :   //Reserved
                            {
                            Skip_XX(Size,                       "Reserved");
                            }
                            break;
                case 11 :   //Link
                            {
                            Skip_XX(Size,                       "Link");
                            }
                            break;
                case 12 :   //Extended Description
                            {
                            Ztring Title;
                            Get_UTF16L(Size, Title,             "Extended Description");
                            Fill(Stream_General, 0, General_Title, Title);
                            }
                            break;
                case 13 :   //Extended Agency
                            {
                            Ztring Title;
                            Get_UTF16L(Size, Title,             "Extended Agency");
                            Fill(Stream_General, 0, General_EncodedBy, Title);
                            }
                            break;
                case 14 :   //User-definable Field
                case 15 :   //User-definable Field
                case 16 :   //User-definable Field
                case 17 :   //User-definable Field
                            {
                            Ztring Comment;
                            Get_UTF16L(Size, Comment,           "User-definable Field");
                            Fill(Stream_General, 0, General_Comment, Comment);
                            }
                            break;
                case 18 :   //External Controller UID
                            {
                            Skip_XX(Size,                       "External Controller UID");
                            }
                            break;
                case 19 :   //Video ARC
                            {
                            Skip_XX(Size,                       "Video ARC");
                            }
                            break;
                case 20 :   //Modified Timestamp
                            {
                            Skip_XX(Size,                       "Modified Timestamp");
                            }
                            break;
                case 21 :   //Video QA Status
                            {
                            Skip_XX(Size,                       "Video QA Status");
                            }
                            break;
                case 22 :   //User Segments In Use (bitmask)
                            {
                            Skip_XX(Size,                       "User Segments In Use");
                            }
                            break;
                case 23 :   //Audio Track Info
                            {
                                BS_Begin_LE();
                                for (int8u Pos=0; Pos<Size; Pos++)
                                {
                                    int8u Format;
                                    Skip_TB(                    "Group / AES pair");
                                    Skip_T1(3,                  "Channels (modulo 8)");
                                    Get_T1 (3, Format,          "Audio format");
                                    Skip_TB(                    "Metadata in ANC");

                                    if (Pos>=Audios.size())
                                        Audios.resize(Pos+1);
                                    Audios[Pos].Format=Format;
                                }
                                BS_End_LE();
                            }
                            break;
                case 24 :   //Audio Tag Info
                            {
                                for (int8u Pos=0; Pos<Size; Pos++)
                                    Skip_L1(                    "Language");
                            }
                            break;
                default : Skip_XX(Size,                         "Data");
            }
        }
        Offset+=1+Size;
        Pos++;
    }

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Lxf::Audio()
{
    Element_Name("Audio");

    #if MEDIAINFO_SEEK
        if (FrameRate==0 && Audios_Header.TimeStamp_End-Audios_Header.TimeStamp_Begin!=0)
            FrameRate=TimeStamp_Rate/(Audios_Header.TimeStamp_End-Audios_Header.TimeStamp_Begin);
    #endif //MEDIAINFO_SEEK

    Audio_Sizes_Pos=0;
    Element_ThisIsAList();
}

//---------------------------------------------------------------------------
void File_Lxf::Audio_Stream(size_t Pos)
{
    if (LookingForLastFrame || (Config->ParseSpeed<1 && Pos<Audios.size() && Audios[Pos].IsFilled))
    {
        Skip_XX(Element_Size,                                   "Data");
        return;
    }

    if (Pos>=Audios.size())
        Audios.resize(Pos+1);
    if (Audios[Pos].Parsers.empty())
    {
        //Trying to detect if this is PCM
        /*
        switch (Audios[Pos].Format)
        {
            case (int8u)-1 : //PCM without codec identifier (default)
            case  0 : //PCM
                    {
                        #ifdef MEDIAINFO_SMPTEST0337_YES
                        {
                            File_ChannelGrouping* Parser=new File_ChannelGrouping;
                            if (Pos%2 && !Audios[Pos-1].Parsers.empty())
                            {
                                Parser->Channel_Pos=1;
                                Parser->Common=((File_ChannelGrouping*)Audios[Pos-1].Parsers[0])->Common;
                                Parser->StreamID=Pos-1;
                                Element_Code--;
                            }
                            else
                            {
                                Parser->Channel_Pos=0;
                                Parser->SampleRate=48000;
                            }
                            Parser->Channel_Total=2;
                            Parser->ByteDepth=SampleSize/8;

                            Open_Buffer_Init(Parser);
                            Audios[Pos].Parsers.push_back(Parser);
                        }
                        #endif //MEDIAINFO_SMPTEST0337_YES

                        Audios[Pos].BytesPerFrame=Audio_Sizes[Pos];

                        #ifdef MEDIAINFO_PCM_YES
                            File_Pcm* Parser=new File_Pcm;
                            Parser->SamplingRate=48000;
                            Parser->Channels=1;
                            Parser->BitDepth=SampleSize;
                            Parser->Endianness='L';
                            #if !defined(MEDIAINFO_MPEGA_YES) && !defined(MEDIAINFO_SMPTEST0337_YES)
                                Parser->Frame_Count_Valid=1;
                            #else
                                Parser->Frame_Count_Valid=2;
                            #endif
                        #else //MEDIAINFO_PCM_YES
                            File__Analyze* Parser=new File__Analyze;
                        #endif //MEDIAINFO_PCM_YES
                        Open_Buffer_Init(Parser);
                        #ifndef MEDIAINFO_PCM_YES
                            Parser->Accept();
                            Parser->Stream_Prepare(Stream_Audio);
                            int64u BitRate=float64_int64s(Audio_Sizes[Pos]*TimeStamp_Rate*8/(Audios_Header.TimeStamp_End-Audios_Header.TimeStamp_Begin));
                            Parser->Fill(Stream_Audio, StreamPos_Last, Audio_BitRate, BitRate);
                            Parser->Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
                            Parser->Fill(Stream_Audio, StreamPos_Last, Audio_Format, "PCM");
                            Parser->Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, 48000);
                            Parser->Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, 1);
                            Parser->Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, SampleSize);
                            Parser->Fill();
                        #endif //MEDIAINFO_PCM_YES
                        Audios[Pos].Parsers.push_back(Parser);

                        #ifdef MEDIAINFO_MPEGA_YES
                        {
                            File_Mpega* Parser=new File_Mpega();
                            Open_Buffer_Init(Parser);
                            Audios[Pos].Parsers.push_back(Parser);
                        }
                        #endif //MEDIAINFO_MPEGA_YES
                    }
                    break;
            case  1 : //DTS
                    {
                        #ifdef MEDIAINFO_DTS_YES
                            File_Dts* Parser=new File_Dts();
                        #else //MEDIAINFO_DTS_YES
                            File__Analyze* Parser=new File__Analyze;
                        #endif //MEDIAINFO_DTS_YES
                        Open_Buffer_Init(Parser);
                        Audios[Pos].Parsers.push_back(Parser);
                    }
                    break;
            case  2 : //AC-3
            case  5 : //E-AC-3
                    {
                        #ifdef MEDIAINFO_AC3_YES
                            File_Ac3* Parser=new File_Ac3;
                        #else //MEDIAINFO_AAC_YES
                            File__Analyze* Parser=new File__Analyze;
                        #endif //MEDIAINFO_AAC_YES
                        Open_Buffer_Init(Parser);
                        Audios[Pos].Parsers.push_back(Parser);
                    }
                    break;
            case  3 : //Dolby E
                    {
                        #ifdef MEDIAINFO_SMPTEST0337_YES
                        {
                            File_ChannelGrouping* Parser=new File_ChannelGrouping;
                            if (Pos%2 && !Audios[Pos-1].Parsers.empty())
                            {
                                Parser->Channel_Pos=1;
                                Parser->Common=((File_ChannelGrouping*)Audios[Pos-1].Parsers[0])->Common;
                                Parser->StreamID=Pos-1;
                                Element_Code--;
                            }
                            else
                            {
                                Parser->Channel_Pos=0;
                                Parser->SampleRate=48000;
                            }
                            Parser->Channel_Total=2;
                            Parser->ByteDepth=SampleSize/8;

                            Open_Buffer_Init(Parser);
                            Audios[Pos].Parsers.push_back(Parser);
                        }
                        #endif //MEDIAINFO_SMPTEST0337_YES

                        #ifdef MEDIAINFO_DOLBYE_YES
                            File_DolbyE* Parser=new File_DolbyE();
                        #else //MEDIAINFO_DOLBYE_YES
                            File__Analyze* Parser=new File__Analyze;
                        #endif //MEDIAINFO_DOLBYE_YES
                        Open_Buffer_Init(Parser);
                        Audios[Pos].Parsers.push_back(Parser);
                    }
            case  4 :
                    {
                        #ifdef MEDIAINFO_MPEGA_YES
                            File_Mpega* Parser=new File_Mpega();
                        #else //MEDIAINFO_MPEGA_YES
                            File__Analyze* Parser=new File__Analyze;
                        #endif //MEDIAINFO_MPEGA_YES
                        Open_Buffer_Init(Parser);
                        Audios[Pos].Parsers.push_back(Parser);
                    }
                    break;
            case  6 :
                    {
                        #ifdef MEDIAINFO_AAC_YES
                            File_Aac* Parser=new File_Aac();
                            Parser->Mode=File_Aac::Mode_ADTS;
                        #else //MEDIAINFO_AAC_YES
                            File__Analyze* Parser=new File__Analyze;
                        #endif //MEDIAINFO_AAC_YES
                        Open_Buffer_Init(Parser);
                        Audios[Pos].Parsers.push_back(Parser);
                    }
                    break;
            default :
                    {
                        File__Analyze* Parser=new File__Analyze;
                        Open_Buffer_Init(Parser);
                        Audios[Pos].Parsers.push_back(Parser);
                    }
        }
        */

        #ifdef MEDIAINFO_SMPTEST0337_YES
        if (!(Pos%2 && Audios[Pos-1].Parsers.size()<=1)) //If the first half-stream was already rejected, don't try this one
        {
            File_ChannelGrouping* Parser=new File_ChannelGrouping;
            if (Pos%2 && !Audios[Pos-1].Parsers.empty())
            {
                Parser->Channel_Pos=1;
                Parser->Common=((File_ChannelGrouping*)Audios[Pos-1].Parsers[0])->Common;
                Parser->StreamID=Pos-1;
            }
            else
                Parser->Channel_Pos=0;
            Parser->BitDepth=SampleSize;
            Parser->Channel_Total=2;
            Parser->SamplingRate=48000;
            Parser->Endianness='L';

            Audios[Pos].Parsers.push_back(Parser);
        }
        #endif //MEDIAINFO_SMPTEST0337_YES
        #ifdef MEDIAINFO_SMPTEST0337_YES
        {
            File_SmpteSt0337* Parser=new File_SmpteSt0337;
            Parser->Container_Bits=SampleSize;
            Parser->Endianness='L';
            Parser->Aligned=true;

            Audios[Pos].Parsers.push_back(Parser);
        }
        #endif //MEDIAINFO_SMPTEST0337_YES
        #ifdef MEDIAINFO_AC3_YES
            Audios[Pos].Parsers.push_back(new File_Ac3());
        #endif //MEDIAINFO_AC3_YES
        #ifdef MEDIAINFO_DTS_YES
            Audios[Pos].Parsers.push_back(new File_Dts());
        #endif //MEDIAINFO_DTS_YES
        #ifdef MEDIAINFO_MPEGA_YES
            Audios[Pos].Parsers.push_back(new File_Mpega());
        #endif //MEDIAINFO_MPEGA_YES
        #ifdef MEDIAINFO_AAC_YES
        {
            File_Aac* Parser=new File_Aac;
            Parser->Mode=File_Aac::Mode_ADTS;

            Audios[Pos].Parsers.push_back(Parser);
        }
        #endif //MEDIAINFO_AAC_YES
        #ifdef MEDIAINFO_PCM_YES
        {
            File_Pcm* Parser=new File_Pcm;
            Parser->SamplingRate=48000;
            Parser->Channels=1;
            Parser->BitDepth=SampleSize;
            Parser->Endianness='L';
            #if !defined(MEDIAINFO_SMPTEST0337_YES) && !defined(MEDIAINFO_MPEGA_YES)
                Parser->Frame_Count_Valid=1;
            #else
                Parser->Frame_Count_Valid=2;
            #endif

            Audios[Pos].Parsers.push_back(Parser);
        }
        #endif //MEDIAINFO_PCM_YES

        for (size_t Pos2=0; Pos2<Audios[Pos].Parsers.size(); Pos2++)
        {
            Open_Buffer_Init(Audios[Pos].Parsers[Pos2]);

            #if MEDIAINFO_DEMUX
                //There are several frames in 1 block, we must rely on the stream parser
                if (Config->Demux_Unpacketize_Get())
                {
                    Audios[Pos].Parsers[Pos2]->Demux_Level=2; //Container
                    Audios[Pos].Parsers[Pos2]->Demux_UnpacketizeContainer=true;
                }
            #endif //MEDIAINFO_DEMUX
        }

        Stream_Count++;
    }

    /*
    #if MEDIAINFO_DEMUX
        #if MEDIAINFO_SEEK
            if (SeekRequest==(int64u)-1)
        #endif //MEDIAINFO_SEEK
        {
            Element_Code=0x200+Pos;
            Frame_Count_NotParsedIncluded=float64_int64s(((float64)(Audios_Header.TimeStamp_End-Audios_Header.Duration))/TimeStamp_Rate*FrameRate);
            if (SampleSize==20 && Config->Demux_PCM_20bitTo16bit_Get())
            {
                //Removing bits 3-0 (Little endian)
                int8u* SixteenBit=new int8u[(size_t)Audio_Sizes[Pos]];
                size_t SixteenBit_Pos=0;
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;
                size_t Buffer_Max=Buffer_Offset+(size_t)(Element_Offset+Audio_Sizes[Pos]);

                while (Buffer_Pos+5<=Buffer_Max)
                {
                    int64u Temp=LittleEndian2int40u(Buffer+Buffer_Pos);
                    Temp=((Temp&0xFFFF000000LL)>>8)|((Temp&0xFFFF0LL)>>4);
                    int32s2LittleEndian(SixteenBit+SixteenBit_Pos, (int32s)Temp);
                    SixteenBit_Pos+=4;
                    Buffer_Pos+=5;
                }

                Demux(SixteenBit, SixteenBit_Pos, ContentType_MainStream);

                delete[] SixteenBit;
            }
            else if (SampleSize==20 && Config->Demux_PCM_20bitTo24bit_Get())
            {
                //Padding bits 3-0 (Little endian)
                int8u* Output=new int8u[(size_t)Audio_Sizes[Pos]*24/20];
                size_t Output_Pos=0;
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;
                size_t Buffer_Max=Buffer_Offset+(size_t)(Element_Offset+Audio_Sizes[Pos]);

                while (Buffer_Pos+5<=Buffer_Max)
                {
                    Output[Output_Pos  ] =  Buffer[Buffer_Pos+0]<<4                                 ;
                    Output[Output_Pos+1] = (Buffer[Buffer_Pos+1]<<4  ) | (Buffer[Buffer_Pos+0]>>4  );
                    Output[Output_Pos+2] = (Buffer[Buffer_Pos+2]<<4  ) | (Buffer[Buffer_Pos+1]>>4  );
                    Output[Output_Pos+3] =  Buffer[Buffer_Pos+2]&0xF0                               ;
                    Output[Output_Pos+4] =  Buffer[Buffer_Pos+3]                                    ;
                    Output[Output_Pos+5] =  Buffer[Buffer_Pos+4]                                    ;

                    Buffer_Pos+=5;
                    Output_Pos+=6;
                }

                Demux(Output, Output_Pos, ContentType_MainStream);

                delete[] Output;
            }
            else
                Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)Audio_Sizes[Pos], ContentType_MainStream);
        }
    #endif //MEDIAINFO_DEMUX
    */

    #if MEDIAINFO_DEMUX
        #if MEDIAINFO_SEEK
            if (SeekRequest==(int64u)-1)
        #endif //MEDIAINFO_SEEK
        {
            Element_Code=0x200+Pos;
            Frame_Count_NotParsedIncluded=float64_int64s(((float64)(Audios_Header.TimeStamp_End-Audios_Header.Duration))/TimeStamp_Rate*FrameRate);
            Demux_Level=4; //Intermediate
            Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)Audio_Sizes[Pos], ContentType_MainStream);
        }
    #endif //MEDIAINFO_DEMUX

    //Parsing
    Frame_Count_NotParsedIncluded=float64_int64s(((float64)(Audios_Header.TimeStamp_End-Audios_Header.Duration))/TimeStamp_Rate*FrameRate);
    for (size_t Pos2=0; Pos2<Audios[Pos].Parsers.size(); Pos2++)
    {
        if (Audios[Pos].Parsers[Pos2]->FrameInfo.DTS==(int64u)-1 || !((FrameInfo.DUR/2>FrameInfo.DTS || Audios[Pos].Parsers[Pos2]->FrameInfo.DTS>=FrameInfo.DTS-FrameInfo.DUR/2) && Audios[Pos].Parsers[Pos2]->FrameInfo.DTS<FrameInfo.DTS+FrameInfo.DUR/2))
            Audios[Pos].Parsers[Pos2]->FrameInfo=FrameInfo;
        Open_Buffer_Continue(Audios[Pos].Parsers[Pos2], Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)Audio_Sizes[Pos]);

        if (Audios[Pos].Parsers.size()>1)
        {
            if (!Audios[Pos].Parsers[Pos2]->Status[IsAccepted] && Audios[Pos].Parsers[Pos2]->Status[IsFinished])
            {
                delete *(Audios[Pos].Parsers.begin()+Pos2);
                Audios[Pos].Parsers.erase(Audios[Pos].Parsers.begin()+Pos2);
                Pos2--;
            }
            else if (Audios[Pos].Parsers.size()>1 && Audios[Pos].Parsers[Pos2]->Status[IsAccepted])
            {
                File__Analyze* Parser=Audios[Pos].Parsers[Pos2];
                for (size_t Pos3=0; Pos3<Audios[Pos].Parsers.size(); Pos3++)
                {
                    if (Pos3!=Pos2)
                        delete *(Audios[Pos].Parsers.begin()+Pos3);
                }
                Audios[Pos].Parsers.clear();
                Audios[Pos].Parsers.push_back(Parser);
            }
        }

        if (Audios[Pos].Parsers.size()==1 && !Audios[Pos].IsFilled && Audios[Pos].Parsers[0]->Status[IsFilled])
        {
            if (Stream_Count>0)
                Stream_Count--;
            Audios[Pos].IsFilled=true;
        }

        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
                DemuxParser=Audios[Pos].Parsers[0];
        #endif //MEDIAINFO_DEMUX
    }
    Element_Offset+=Audio_Sizes[Pos];
}

//---------------------------------------------------------------------------
void File_Lxf::Video()
{
    Element_Name("Video");

    #if MEDIAINFO_SEEK
        if (FrameRate==0 && Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin!=0)
            FrameRate=TimeStamp_Rate/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin);
    #endif //MEDIAINFO_SEEK

    Video_Sizes_Pos=0;
    Element_ThisIsAList();
}

//---------------------------------------------------------------------------
void File_Lxf::Video_Stream(size_t Pos)
{
    if (LookingForLastFrame || (Config->ParseSpeed<1 && Pos<Videos.size() && Videos[Pos].IsFilled && Pos!=1)) //Hint: trying to catch VBI/VANC at the end of the file
    {
        Skip_XX(Element_Size,                                   "Data");
        return;
    }

    #if MEDIAINFO_DEMUX
        #if MEDIAINFO_SEEK
            if (SeekRequest==(int64u)-1)
        #endif //MEDIAINFO_SEEK
        {
            Element_Code=0x100+Pos;
            Frame_Count_NotParsedIncluded=float64_int64s(((float64)(Videos_Header.TimeStamp_End-Videos_Header.Duration))/TimeStamp_Rate*FrameRate);
            Demux_Level=2; //Container
            Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)Video_Sizes[Pos], ContentType_MainStream);
        }
    #endif //MEDIAINFO_DEMUX

    if (Pos>=Videos.size())
        Videos.resize(Pos+1);

    switch (Pos)
    {
        case 1 : Video_Stream_1(); break;
        case 2 : Video_Stream_2(); break;
        default: ;
    }
}

//---------------------------------------------------------------------------
void File_Lxf::Video_Stream_1()
{
    if (Video_Sizes[1]<2)
    {
        Skip_XX(Video_Sizes[1],                         "Unknown");
        return;
    }

    int8u Lines_Allocated, Lines_Used;
    Get_L1 (Lines_Allocated,                            "Lines allocated");
    Get_L1 (Lines_Used,                                 "Lines used");

    if (Lines_Allocated==0 || Lines_Used>Lines_Allocated || Video_Sizes[1]<2+Lines_Used)
    {
        Skip_XX(Video_Sizes[1]-2,                       "Unknown");
        return;
    }

    Videos[1].BytesPerFrame=Video_Sizes[1]-(2+Lines_Allocated);
    int64u BytesPerLine=Videos[1].BytesPerFrame/Lines_Allocated;

    std::vector<int8u> FieldNumbers;
    std::vector<bool>  FieldNumbers_IsSecondField;
    BS_Begin_LE();
    for (int8u Pos=0; Pos<Lines_Allocated; Pos++)
    {
        int8u FieldNumber;
        bool  FieldNumber_IsSecondField;
        Get_T1 (7, FieldNumber,                             "Field line");
        Get_TB (   FieldNumber_IsSecondField,               "Field");

        if (Pos<Lines_Used)
        {
            FieldNumbers.push_back(FieldNumber);
            FieldNumbers_IsSecondField.push_back(FieldNumber_IsSecondField);
        }
    }
    BS_End_LE();

    for (int8u Pos=0; Pos<Lines_Used; Pos++)
    {
        #if defined(MEDIAINFO_CDP_YES)
            Element_Begin1("VANC line");
            if (Videos[1].Parsers.empty())
            {
                Ancillary=new File_Ancillary;
                Ancillary->InDecodingOrder=true;
                Ancillary->WithChecksum=true;
                Ancillary->MustSynchronize=true;
                Open_Buffer_Init(Ancillary);
                Videos[1].Parsers.push_back(Ancillary);
                Stream_Count++;
            }
            Videos[1].Parsers[0]->FrameInfo=FrameInfo;
            ((File_Ancillary*)Videos[1].Parsers[0])->LineNumber=FieldNumbers[Pos];
            ((File_Ancillary*)Videos[1].Parsers[0])->LineNumber_IsSecondField=FieldNumbers_IsSecondField[Pos];
            Open_Buffer_Continue(Videos[1].Parsers[0], Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)BytesPerLine);
            if (Videos[1].Parsers[0]->Status[IsFilled])
            {
                if (Stream_Count>0)
                    Stream_Count--;
                Videos[1].IsFilled=true;
            }
            Element_Offset+=BytesPerLine;
            Element_End0();
        #else
            Skip_XX(BytesPerLine,                               "VBI/VANC data");
        #endif
    }
    Skip_XX((Lines_Allocated-Lines_Used)*BytesPerLine,          "Unused lines");

    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Unknown");
}

//---------------------------------------------------------------------------
void File_Lxf::Video_Stream_2()
{
    if (Videos[2].Parsers.empty())
    {
        /*
        //Trying to detect if this is PCM
        switch (Videos[2].Format)
        {
            case 0x01 :
            case 0x02 :
            case 0x03 :
            case 0x09 :
                        {
                            #ifdef MEDIAINFO_MPEGV_YES
                                File_Mpegv* Parser=new File_Mpegv();
                            #else //MEDIAINFO_MPEGV_YES
                                File__Analyze* Parser=new File__Analyze;
                            #endif //MEDIAINFO_MPEGV_YES
                            Open_Buffer_Init(Parser);
                            #ifndef MEDIAINFO_MPEGV_YES
                                Parser->Stream_Prepare(Stream_Video);
                                Parser->Fill(Stream_Video, 0, Video_Format, "MPEG Video");
                                if (Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin)
                                {
                                    FrameRate=((float64)1)*TimeStamp_Rate/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin);
                                    if (FrameRate)
                                        Parser->Fill(Stream_Video, 0, Video_FrameRate, FrameRate);
                                }
                            #endif //MEDIAINFO_MPEGV_YES
                            Videos[2].Parsers.push_back(Parser);
                        }
                        break;
            case 0x04 :
            case 0x05 :
            case 0x06 :
                        {
                            #ifdef MEDIAINFO_DVDIF_YES
                                File_DvDif* Parser=new File_DvDif();
                            #else //MEDIAINFO_DVDIF_YES
                                File__Analyze* Parser=new File__Analyze;
                            #endif //MEDIAINFO_DVDIF_YES
                            Open_Buffer_Init(Parser);
                            #ifndef MEDIAINFO_DVDIF_YES
                                Parser->Stream_Prepare(Stream_Video);
                                Parser->Fill(Stream_Video, 0, Video_Format, "DV");
                                if (Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin)
                                {
                                    FrameRate=((float64)1)*TimeStamp_Rate/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin);
                                    if (FrameRate)
                                        Parser->Fill(Stream_Video, 0, Video_FrameRate, FrameRate);
                                    int64u BitRate=float64_int64s(((float64)Video_Sizes[1])*TimeStamp_Rate*8/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin));
                                    if (BitRate)
                                        Parser->Fill(Stream_Video, 0, Video_BitRate, BitRate);
                                    Parser->Fill(Stream_Video, 0, Video_BitRate_Mode, "CBR");
                                }
                            #endif //MEDIAINFO_DVDIF_YES
                            Videos[2].Parsers.push_back(Parser);
                        }
                        #ifdef MEDIAINFO_VC3_YES
                        if (Videos[2].Format==0x06) // One file with VideoFormat = 6 has VC-3
                        {
                            File_Vc3* Parser=new File_Vc3();
                            Open_Buffer_Init(Parser);
                            Videos[2].Parsers.push_back(Parser);
                        }
                        #endif //MEDIAINFO_VC3_YES
                        break;
            case 0x0A :
            case 0x0B :
            case 0x0C :
            case 0x0D :
                        {
                            #ifdef MEDIAINFO_AVC_YES
                                File_Avc* Parser=new File_Avc();
                            #else //MEDIAINFO_AVC_YES
                                File__Analyze* Parser=new File__Analyze;
                            #endif //MEDIAINFO_AVC_YES
                            Open_Buffer_Init(Parser);
                            #ifndef MEDIAINFO_AVC_YES
                                Parser->Accept();
                                Parser->Stream_Prepare(Stream_Video);
                                Parser->Fill(Stream_Video, 0, Video_Format, "AVC");
                                if (Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin)
                                {
                                    FrameRate=((float64)1)*TimeStamp_Rate/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin);
                                    if (FrameRate)
                                        Parser->Fill(Stream_Video, 0, Video_FrameRate, FrameRate);
                                }
                            #endif //MEDIAINFO_AVC_YES
                            Videos[2].Parsers.push_back(Parser);
                        }
                        break;
            default :
                    {
                        File__Analyze* Parser=new File__Analyze;
                        Open_Buffer_Init(Parser);
                        Videos[2].Parsers.push_back(Parser);
                    }
        }
        */

        #ifdef MEDIAINFO_DVDIF_YES
            Videos[2].Parsers.push_back(new File_DvDif());
        #endif //MEDIAINFO_DVDIF_YES
        #ifdef MEDIAINFO_MPEGV_YES
        {
            File_Mpegv* Parser=new File_Mpegv();
            #if defined(MEDIAINFO_ANCILLARY_YES)
                Parser->Ancillary=&Ancillary;
            #endif //defined(MEDIAINFO_ANCILLARY_YES)
            Videos[2].Parsers.push_back(Parser);
        }
        #endif //MEDIAINFO_MPEGV_YES
        #ifdef MEDIAINFO_AVC_YES
            Videos[2].Parsers.push_back(new File_Avc());
        #endif //MEDIAINFO_AVC_YES
        #ifdef MEDIAINFO_VC3_YES
            Videos[2].Parsers.push_back(new File_Vc3());
        #endif //MEDIAINFO_VC3_YES
        for (size_t Pos2=0; Pos2<Videos[2].Parsers.size(); Pos2++)
            Open_Buffer_Init(Videos[2].Parsers[Pos2]);

        Stream_Count++;
    }

    //Parsing
    for (size_t Pos2=0; Pos2<Videos[2].Parsers.size(); Pos2++)
    {
        Videos[2].Parsers[Pos2]->FrameInfo=FrameInfo;
        Open_Buffer_Continue(Videos[2].Parsers[Pos2], Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)Video_Sizes[2]);
        Element_Show();

        if (Videos[2].Parsers.size()>1)
        {
            if (!Videos[2].Parsers[Pos2]->Status[IsAccepted] && Videos[2].Parsers[Pos2]->Status[IsFinished])
            {
                delete *(Videos[2].Parsers.begin()+Pos2);
                Videos[2].Parsers.erase(Videos[2].Parsers.begin()+Pos2);
                Pos2--;
            }
            else if (Videos[2].Parsers.size()>1 && Videos[2].Parsers[Pos2]->Status[IsAccepted])
            {
                File__Analyze* Parser=Videos[2].Parsers[Pos2];
                for (size_t Pos3=0; Pos3<Videos[2].Parsers.size(); Pos3++)
                {
                    if (Pos3!=Pos2)
                        delete *(Videos[2].Parsers.begin()+Pos3);
                }
                Videos[2].Parsers.clear();
                Videos[2].Parsers.push_back(Parser);
            }
        }

        if (Videos[2].Parsers.size()==1 && !Videos[2].IsFilled && Videos[2].Parsers[0]->Status[IsFilled])
        {
            if (Stream_Count>0)
                Stream_Count--;
            Videos[2].IsFilled=true;
        }
    }
    Element_Offset+=Video_Sizes[2];
}

} //NameSpace

#endif //MEDIAINFO_LXF_YES
