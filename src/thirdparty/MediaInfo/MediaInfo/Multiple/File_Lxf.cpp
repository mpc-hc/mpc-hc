// File_Lxf - Info for LXF files
// Copyright (C) 2006-2012 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_LXF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Lxf.h"
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
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

const char* Lxf_PictureType[4]=
{
    "I", //Closed
    "I", //Open
    "P",
    "B",
};

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

    //Temp
    LookingForLastFrame=false;
    Stream_Count=0;
    Info_General_StreamSize=0;
    Video_Sizes_Pos=(size_t)-1;
    Audio_Sizes_Pos=(size_t)-1;

    //Seek
    #if MEDIAINFO_SEEK
        SeekRequest=(int64u)-1;
    #endif //MEDIAINFO_SEEK
    FrameRate=0;
    TimeStamp_Rate=720000;
    Duration_Detected=false;
    LastAudio_BufferOffset=(int64u)-1;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lxf::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format_Version, __T("Version "+Ztring::ToZtring(Version)));

    for (size_t Pos=0; Pos<Videos.size(); Pos++)
        Streams_Fill_PerStream(Videos[Pos].Parser, 1, Pos);
    for (size_t Pos=0; Pos<Audios.size(); Pos++)
        Streams_Fill_PerStream(Audios[Pos].Parser, 2, Pos);

    if (!Videos.empty())
        Fill(Stream_Video, 0, Video_CodecID, VideoFormat);
}

//---------------------------------------------------------------------------
void File_Lxf::Streams_Fill_PerStream(File__Analyze* Parser, size_t Container_StreamKind, size_t Parser_Pos)
{
    if (Parser==NULL)
        return;

    Fill(Parser);
    if (Parser->Count_Get(Stream_Audio) && Config->File_Audio_MergeMonoStreams_Get())
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
        #if MEDIAINFO_DEMUX
            if (Config->Demux_ForceIds_Get())
                for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
                    for (size_t StreamPos=0; StreamPos<Parser->Count_Get((stream_t)StreamKind); StreamPos++)
                    {
                        Ztring ID=Ztring::ToZtring(0x100*Container_StreamKind+Parser_Pos);
                        if (!Parser->Retrieve((stream_t)StreamKind, StreamPos, General_ID).empty())
                            ID+=__T('-')+Parser->Retrieve((stream_t)StreamKind, StreamPos, General_ID);
                        Fill((stream_t)StreamKind, Count_Get((stream_t)StreamKind)-Parser->Count_Get((stream_t)StreamKind)+StreamPos, General_ID, ID, true);
                    }
        #endif //MEDIAINFO_DEMUX
    }
}

//---------------------------------------------------------------------------
void File_Lxf::Streams_Finish()
{
    if (Videos[1].Parser && Count_Get(Stream_Text)==0) //TODO: better handling of fill/finish
    {
        Finish(Videos[1].Parser);
        Streams_Fill_PerStream(Videos[1].Parser, Stream_Video, 1);
    }
    if (Videos[2].Parser)
    {
        Finish(Videos[2].Parser);
        Merge(*Videos[2].Parser, Stream_Video, 0, 0);
    }

    if (Audios_Header.TimeStamp_End!=(int64u)-1 && Audios_Header.TimeStamp_Begin!=(int64u)-1 && Audios_Header.Duration_First!=(int64u)-1)
    {
        int64u Duration=float64_int64s(((float64)(Audios_Header.TimeStamp_End-Audios_Header.TimeStamp_Begin))/TimeStamp_Rate*1000);
        int64u FrameCount=float64_int64s(((float64)(Audios_Header.TimeStamp_End-Audios_Header.TimeStamp_Begin))/Audios_Header.Duration_First);
        for (size_t Pos=0; Pos<Count_Get(Stream_Audio); Pos++)
        {
            Fill(Stream_Audio, Pos, Audio_Duration, Duration);
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
    #if MEDIAINFO_SEEK
                                          || (Buffer[Buffer_Offset+16]!=0x02 && Buffer[Buffer_Offset+16]!=0x00)
    #endif //MEDIAINFO_SEEK
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

    for (size_t Pos=0; Pos<Audios.size(); Pos++)
        if (Audios[Pos].Parser)
            Audios[Pos].Parser->Open_Buffer_Unsynch();
    for (size_t Pos=0; Pos<Videos.size(); Pos++)
        if (Videos[Pos].Parser)
            Videos[Pos].Parser->Open_Buffer_Unsynch();
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
                                    if (Videos[Pos].Parser)
                                        Videos[Pos].Parser->Unsynch_Frame_Count=0;
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
        Header_Fill_Code(0x010100+Video_Sizes_Pos, __T("Stream"));
        Header_Fill_Size(Video_Sizes[Video_Sizes_Pos]);
        Video_Sizes_Pos++;
        return;
    }
    while (Audio_Sizes_Pos<Audio_Sizes.size() && Audio_Sizes[Audio_Sizes_Pos]==0)
        Audio_Sizes_Pos++;
    if (Audio_Sizes_Pos<Audio_Sizes.size())
    {
        //Filling
        Header_Fill_Code(0x010200+Audio_Sizes_Pos, __T("Stream"));
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
                    FrameInfo.DTS=float64_int64s(((float64)TimeStamp)*1000000000/TimeStamp_Rate);
                    Get_L4 (Duration4,                          "Duration");
                    Duration=Duration4;
                    Param_Info3(((float64)Duration)/TimeStamp_Rate, 3, " s");
                    FrameInfo.DUR=float64_int64s(((float64)Duration)*1000000000/TimeStamp_Rate);
                    }
                    break;
        case 1 :
                    Get_L8 (TimeStamp,                          "TimeStamp"); Param_Info3(((float64)TimeStamp)/720000, 3, " s"); FrameInfo.DTS=float64_int64s(((float64)TimeStamp)*1000000/720);
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
                    int8u GOP_M, PictureType;
                    BS_Begin_LE();
                    Get_T1 (4, VideoFormat,                     "Format"); Param_Info1(Lxf_Format_Video[VideoFormat]);
                    Skip_T1(7,                                  "GOP (N)");
                    Get_T1 (3, GOP_M,                           "GOP (M)");
                    Info_T1(8, BitRate,                         "Bit rate"); Param_Info2(BitRate*1000000, " bps");
                    Get_T1 (2, PictureType,                     "Picture type"); Param_Info1(Lxf_PictureType[PictureType]);
                    BS_End_LE();
                    Skip_L1(                                    "Reserved");
                    Get_L4(Size,                                "Video data size");
                    Skip_L4(                                    "Zero");
                    Video_Sizes[2]=Size;
                    BlockSize+=Size;
                    Get_L4(Size,                                "VBI data size");
                    Video_Sizes[1]=Size;
                    BlockSize+=Size;
                    Skip_L4(                                    "Zero");
                    Get_L4(Size,                                "Meta data size");
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
                    bitset<32> Channels;

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
                            Channels[Pos]=Channel;
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
        Element_Begin1("id");
            Skip_C8(                                            "code");
            Skip_L4(                                            "minFrame");
            Skip_L4(                                            "start");
            Skip_L4(                                            "duration");
            Skip_L4(                                            "tcOffset");
            BS_Begin_LE();
            Skip_T1(4,                                          "Format");
            Skip_T1(7,                                          "GOP (N)");
            Skip_T1(3,                                          "GOP (M)");
            Skip_T1(8,                                          "Bit rate");
            Skip_TB(                                            "VBI present");
            Skip_TB(                                            "Aspect Ratio");
            BS_End_LE();
            Skip_L1(                                            "reserved");
            Skip_L4(                                            "base");
            Skip_L4(                                            "prev");
            Skip_L4(                                            "next");
            Skip_L2(                                            "recordDate");
            Skip_L2(                                            "killDate");
            Skip_L1(                                            "tc_type");
            Skip_L1(                                            "status");
            Skip_L1(                                            "disk");
            Skip_String(26,                                     "description");
            Skip_String(16,                                     "agency");
            Skip_String( 6,                                     "description");
        Element_End0();
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
                case  1 :   //Library
                            {
                            Ztring Library;
                            Get_UTF8(Size, Library,             "Library");
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
                case  7 :   //Channel?
                            {
                            Ztring Channel;
                            Get_UTF16L(Size, Channel,           "Channel?");
                            Fill(Stream_General, 0, General_ServiceName, Channel);
                            }
                            break;
                case  8 :   //Department
                            {
                            Skip_UTF16L(Size,                   "Department");
                            }
                            break;
                case 12 :   //Title
                            {
                            Ztring Title;
                            Get_UTF16L(Size, Title,             "Title");
                            Fill(Stream_General, 0, General_Title, Title);
                            }
                            break;
                case 13 :   //Extended Agency
                            {
                            Ztring Title;
                            Get_UTF16L(Size, Title,             "Extended Agency");
                            Fill(Stream_General, 0, General_Title, Title); //Note: not sure
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
                case 20 :   //Modified Timestamp
                            {
                            Skip_XX(Size,                         "Modified Timestamp");
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
    Element_Begin1("Stream");

    #if MEDIAINFO_DEMUX
        #if MEDIAINFO_SEEK
            if (SeekRequest==(int64u)-1)
        #endif //MEDIAINFO_SEEK
        {
            Element_Code=0x0200+Pos;
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
            }
            else
                Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)Audio_Sizes[Pos], ContentType_MainStream);
        }
    #endif //MEDIAINFO_DEMUX

    Skip_XX(Audio_Sizes[Pos],                                   Audio_Sizes.size()==2?"PCM":"Unknown format");

    if (Pos>=Audios.size())
        Audios.resize(Pos+1);
    if (Audios[Pos].Parser==NULL)
    {
        //Trying to detect if this is PCM
        int64u BitRate=float64_int64s(Audio_Sizes[Pos]*TimeStamp_Rate*8/(Audios_Header.TimeStamp_End-Audios_Header.TimeStamp_Begin));
        Audios[Pos].BytesPerFrame=Audio_Sizes[Pos];

        Audios[Pos].Parser=new File__Analyze;
        Open_Buffer_Init(Audios[Pos].Parser);
        Audios[Pos].Parser->Accept();
        Audios[Pos].Parser->Stream_Prepare(Stream_Audio);
        Audios[Pos].Parser->Fill(Stream_Audio, StreamPos_Last, Audio_BitRate, BitRate);
        Audios[Pos].Parser->Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
        Audios[Pos].Parser->Fill(Stream_Audio, StreamPos_Last, Audio_Format, "PCM");
        Audios[Pos].Parser->Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, 48000);
        Audios[Pos].Parser->Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, 1);
        Audios[Pos].Parser->Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, SampleSize);
        Audios[Pos].Parser->Fill();
    }

    Element_End0();
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
    if (LookingForLastFrame)
    {
        Skip_XX(Element_Size,                                   "Data");
        return;
    }

    Element_Begin1("Stream");

    #if MEDIAINFO_DEMUX
        #if MEDIAINFO_SEEK
            if (SeekRequest==(int64u)-1)
        #endif //MEDIAINFO_SEEK
        {
            Element_Code=0x0100+Pos;
            Frame_Count_NotParsedIncluded=float64_int64s(((float64)(Videos_Header.TimeStamp_End-Videos_Header.Duration))/TimeStamp_Rate*FrameRate);
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

    Element_End0();

    FILLING_BEGIN();
        if (Pos==2)
        {
            Frame_Count++;
            if (Frame_Count>6 && Stream_Count==0 && !Status[IsFilled]) //5 video frames for 1 Audio frame
            {
                Fill("LXF");
                if (MediaInfoLib::Config.ParseSpeed_Get()<1)
                {
                    LookingForLastFrame=true;
                    if (3*(File_Offset+Buffer_Offset)<=File_Size)
                    {
                        GoToFromEnd((File_Offset+Buffer_Offset)*2);
                        Open_Buffer_Unsynch();
                    }
                }
            }
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Lxf::Video_Stream_1()
{
    Videos[1].BytesPerFrame=Video_Sizes[1];

    int8u Lines_Allocated, Lines_Used, FieldNumber;
    Get_L1 (Lines_Allocated,                            "Lines allocated");
    Get_L1 (Lines_Used,                                 "Lines used");
    Get_L1 (FieldNumber,                                "Field number");

    #if defined(MEDIAINFO_CDP_YES)
        if (Videos[1].Parser==NULL)
        {
            Videos[1].Parser=new File_Ancillary;
            ((File_Ancillary*)Videos[1].Parser)->InDecodingOrder=true;
            ((File_Ancillary*)Videos[1].Parser)->WithChecksum=true;
            Videos[1].Parser->MustSynchronize=true;
            Open_Buffer_Init(Videos[1].Parser);
            Stream_Count++;
        }
        Videos[1].Parser->FrameInfo=FrameInfo;
        Open_Buffer_Continue(Videos[1].Parser);
        if (Videos[1].Parser->Status[IsFilled])
        {
            if (Stream_Count>0)
                Stream_Count--;
        }
    #else
        Skip_XX(Video_Sizes[1],                         "VBI data");
    #endif
}

//---------------------------------------------------------------------------
void File_Lxf::Video_Stream_2()
{
    switch (VideoFormat)
    {
        case 0x01 :
        case 0x02 :
        case 0x03 :
        case 0x09 :
                    Video_Stream_2_Mpegv();
                    break;
        case 0x04 :
        case 0x05 :
        case 0x06 :
                    Video_Stream_2_DvDif();
                    break;
        case 0x0A :
        case 0x0B :
        case 0x0C :
        case 0x0D :
                    Video_Stream_2_Avc();
                    break;
        default   :
                    Skip_XX(Element_Size,               "Unknown");
    }
}

//---------------------------------------------------------------------------
void File_Lxf::Video_Stream_2_Mpegv()
{
    #if defined(MEDIAINFO_MPEGV_YES)
        if (Videos[2].Parser==NULL)
        {
            Videos[2].Parser=new File_Mpegv;
            ((File_Mpegv*)Videos[2].Parser)->FrameIsAlwaysComplete=true;
            Open_Buffer_Init(Videos[2].Parser);
            Stream_Count++;
        }
        Open_Buffer_Continue(Videos[2].Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)Video_Sizes[2]);
        if (Videos[2].Parser->Status[IsFilled])
        {
            if (Stream_Count>0)
                Stream_Count--;
        }
    #else
        Skip_XX(Video_Sizes[1],                       "MPEG Video");

        if (Videos[2].Parser==NULL)
        {
            if (FrameRate==0 && Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin)
                FrameRate=((float64)1)*TimeStamp_Rate/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin);

            Videos[2].Parser=new File__Analyze;
            Open_Buffer_Init(Videos[2].Parser);
            Videos[2].Parser->Stream_Prepare(Stream_Video);
            Videos[2].Parser->Fill(Stream_Video, 0, Video_Format, "MPEG Video");
            if (FrameRate)
                Videos[2].Parser->Fill(Stream_Video, 0, Video_FrameRate, FrameRate);
        }
    #endif
}

//---------------------------------------------------------------------------
void File_Lxf::Video_Stream_2_DvDif()
{
    #if defined(MEDIAINFO_DVDIF_YES)
        if (Videos[2].Parser==NULL)
        {
            Videos[2].Parser=new File_DvDif;
            ((File_DvDif*)Videos[2].Parser)->IgnoreAudio=true;
            Open_Buffer_Init(Videos[2].Parser);
            Stream_Count++;
        }
        Open_Buffer_Continue(Videos[2].Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)Video_Sizes[2]);
        if (Videos[2].Parser->Status[IsFilled])
        {
            if (Stream_Count>0)
                Stream_Count--;
        }
    #else
        Skip_XX(Video_Sizes[1],                           "DV");

        if (Videos[2].Parser==NULL)
        {
            int64u BitRate=float64_int64s(((float64)Video_Sizes[1])*TimeStamp_Rate*8/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin));
            if (FrameRate==0 && Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin)
                FrameRate=((float64)1)*TimeStamp_Rate/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin);

            Videos[2].Parser=new File__Analyze;
            Open_Buffer_Init(Videos[2].Parser);
            Videos[2].Parser->Accept();
            Videos[2].Parser->Stream_Prepare(Stream_Video);
            Videos[2].Parser->Fill(Stream_Video, 0, Video_Format, "DV");
            Videos[2].Parser->Fill(Stream_Video, 0, Video_BitRate, BitRate);
            Videos[2].Parser->Fill(Stream_Video, 0, Video_BitRate_Mode, "CBR");
            if (FrameRate)
                Videos[2].Parser->Fill(Stream_Video, 0, Video_FrameRate, FrameRate);
            Videos[2].Parser->Fill();
        }
    #endif
}

//---------------------------------------------------------------------------
void File_Lxf::Video_Stream_2_Avc()
{
    #if defined(MEDIAINFO_AVC_YES)
        if (Videos[2].Parser==NULL)
        {
            Videos[2].Parser=new File_Avc;
            ((File_Avc*)Videos[2].Parser)->FrameIsAlwaysComplete=true;
            Open_Buffer_Init(Videos[2].Parser);
            Stream_Count++;
        }
        Open_Buffer_Continue(Videos[2].Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)Video_Sizes[2]);
        if (Videos[2].Parser->Status[IsFilled])
        {
            if (Stream_Count>0)
                Stream_Count--;
        }
    #else
        Skip_XX(Video_Sizes[1],                       "AVC");

        if (Videos[2].Parser==NULL)
        {
            if (FrameRate==0 && Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin)
                FrameRate=((float64)1)*TimeStamp_Rate/(Videos_Header.TimeStamp_End-Videos_Header.TimeStamp_Begin);

            Videos[2].Parser=new File__Analyze;
            Open_Buffer_Init(Videos[2].Parser);
            Videos[2].Parser->Stream_Prepare(Stream_Video);
            Videos[2].Parser->Fill(Stream_Video, 0, Video_Format, "AVC");
            if (FrameRate)
                Videos[2].Parser->Fill(Stream_Video, 0, Video_FrameRate, FrameRate);
        }
    #endif
}

} //NameSpace

#endif //MEDIAINFO_LXF_*
