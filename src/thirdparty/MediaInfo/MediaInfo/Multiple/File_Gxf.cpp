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
#if defined(MEDIAINFO_GXF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Gxf.h"
#include "MediaInfo/Multiple/File_Gxf_TimeCode.h"
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
#if defined(MEDIAINFO_RIFF_YES)
    #include "MediaInfo/Multiple/File_Riff.h"
#endif
#if defined(MEDIAINFO_GXF_YES)
    #include "MediaInfo/Multiple/File_Umf.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_ChannelGrouping.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/Utils.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
const char* Gxf_Tag_Name(int8u Tag)
{
    switch (Tag)
    {
        case 0x40 : return "Media file name of material";
        case 0x41 : return "First field of material in stream";
        case 0x42 : return "Last field of material in stream";
        case 0x43 : return "Mark in for the stream";
        case 0x44 : return "Mark out for the stream";
        case 0x45 : return "Estimated size of stream in 1024 byte units";
        case 0x46 :
        case 0x47 :
        case 0x48 :
        case 0x49 :
        case 0x4A :
        case 0x4B : return "Reserved";
        case 0x4C : return "Media file name";
        case 0x4D : return "Auxiliary Information";
        case 0x4E : return "Media file system version";
        case 0x4F : return "MPEG auxiliary information";
        case 0x50 : return "Frame rate";
        case 0x51 : return "Lines per frame";
        case 0x52 : return "Fields per frame";
        default   : return "Unknown";
    }
}

//---------------------------------------------------------------------------
const char* Gxf_MediaTypes(int8u Type)
{
    switch (Type)
    {
        case  3 : return "JPEG"; //525 lines
        case  4 : return "JPEG"; //625 lines
        case  7 : return "SMPTE 12M"; //525 lines
        case  8 : return "SMPTE 12M"; //625 lines
        case  9 : return "PCM"; //24-bit
        case 10 : return "PCM"; //16-bit
        case 11 : return "MPEG-2 Video"; //525 lines
        case 12 : return "MPEG-2 Video"; //625 lines
        case 13 : return "DV"; //25 Mbps, 525 lines
        case 14 : return "DV"; //25 Mbps, 625 lines
        case 15 : return "DV"; //50 Mbps, 525 lines
        case 16 : return "DV"; //50 Mbps, 625 lines
        case 17 : return "AC-3"; //16-bit
        case 18 : return "AES"; //non-PCM
        case 19 : return "Reserved";
        case 20 : return "MPEG-2 Video"; //HD, Main Profile at High Level
        case 21 : return "Ancillary data"; //SMPTE 291M 10-bit type 2 component ancillary data
        case 22 : return "MPEG-1 Video"; //525 lines
        case 23 : return "MPEG-1 Video"; //625 lines
        case 24 : return "SMPTE 12M"; //HD
        case 25 : return "DV"; //DVCPRO HD
        default : return "Unknown";
    }
}

//---------------------------------------------------------------------------
stream_t Gxf_MediaTypes_StreamKind(int8u Type)
{
    switch (Type)
    {
        case  3 : return Stream_Video;
        case  4 : return Stream_Video;
        case  7 : return Stream_Max;
        case  8 : return Stream_Max;
        case  9 : return Stream_Audio;
        case 10 : return Stream_Audio;
        case 11 : return Stream_Video;
        case 12 : return Stream_Video;
        case 13 : return Stream_Video;
        case 14 : return Stream_Video;
        case 15 : return Stream_Video;
        case 16 : return Stream_Video;
        case 17 : return Stream_Audio;
        case 18 : return Stream_Audio;
        case 19 : return Stream_Max;
        case 20 : return Stream_Video;
        case 21 : return Stream_Max;
        case 22 : return Stream_Video;
        case 23 : return Stream_Video;
        case 24 : return Stream_Max;
        case 25 : return Stream_Video;
        default : return Stream_Max;
    }
}

//---------------------------------------------------------------------------
const char* Gxf_MediaTypes_Format(int8u Type)
{
    switch (Type)
    {
        case  3 : return "JPEG"; //525 lines
        case  4 : return "JPEG"; //625 lines
        case  9 : return "PCM"; //24-bit
        case 10 : return "PCM"; //16-bit
        case 11 : return "MPEG Video"; //525 lines
        case 12 : return "MPEG Video"; //625 lines
        case 13 : return "DV"; //25 Mbps, 525 lines
        case 14 : return "DV"; //25 Mbps, 625 lines
        case 15 : return "DV"; //50 Mbps, 525 lines
        case 16 : return "DV"; //50 Mbps, 625 lines
        case 17 : return "AC-3"; //16-bit
        case 18 : return "SMPTE 338M, table 1, data type 28"; //SMPTE 338M, table 1, data type 28
        case 20 : return "MPEG Video"; //HD, Main Profile at High Level
        case 22 : return "MPEG Video"; //525 lines
        case 23 : return "MPEG Video"; //625 lines
        case 25 : return "DV"; //DVCPRO HD
        default : return "";
    }
}

//---------------------------------------------------------------------------
double Gxf_FrameRate(int32u Content)
{
    switch (Content)
    {
        case 1 : return 60.000;
        case 2 : return 59.940;
        case 3 : return 50.000;
        case 4 : return 30.000;
        case 5 : return 29.970;
        case 6 : return 25.000;
        case 7 : return 24.000;
        case 8 : return 23.976;
        default: return  0.000;
    }
}

//---------------------------------------------------------------------------
int32u Gxf_LinesPerFrame_Height(int32u Content)
{
    switch (Content)
    {
        case 1 : return  480;
        case 2 : return  576;
        case 4 : return 1080;
        case 6 : return  720;
        default: return    0;
    }
}

//---------------------------------------------------------------------------
int32u Gxf_LinesPerFrame_Width(int32u Content)
{
    switch (Content)
    {
        case 1 : return  720;
        case 2 : return  720;
        case 4 : return 1920;
        case 6 : return 1080;
        default: return    0;
    }
}

//---------------------------------------------------------------------------
const char* Gxf_FieldsPerFrame(int32u Tag)
{
    switch (Tag)
    {
        case 1 : return "Progressive";
        case 2 : return "Interlaced";
        default: return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Gxf::File_Gxf()
:File__Analyze()
{
    //Configuration
    ParserName=__T("GXF");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Gxf;
        StreamIDs_Width[0]=2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;
    Buffer_TotalBytes_Fill_Max=(int64u)-1; //Disabling this feature for this format, this is done in the parser
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Temp
    Material_Fields_FieldsPerFrame=1; //Progressive by default
    Parsers_Count=0;
    AncillaryData_StreamID=(int8u)-1;
    Material_Fields_First_IsValid=false;
    Material_Fields_Last_IsValid=false;
    Material_File_Size_IsValid=false;
    UMF_File=NULL;
    #if defined(MEDIAINFO_ANCILLARY_YES)
        Ancillary=NULL;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)
    SizeToAnalyze=16*1024*1024;
    IsParsingMiddle_MaxOffset=(int64u)-1;
    Audio_Count=0;
    Element_Code=0x00; //Element_Code is used as a test for pre-existing parsing, it must be initialized

    #if MEDIAINFO_DEMUX
        Demux_HeaderParsed=false;
    #endif //MEDIAINFO_DEMUX
    #if MEDIAINFO_SEEK
        Flt_FieldPerEntry=(int32u)-1;
        IFrame_IsParsed=false;
    #endif //MEDIAINFO_SEEK
}

//---------------------------------------------------------------------------
File_Gxf::~File_Gxf()
{
    //Temp
    delete Ancillary; //Ancillary=NULL;
    delete UMF_File; //UMF_File=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gxf::Streams_Finish()
{
    //Merging audio if Title are same
    for (size_t StreamID=0; StreamID<Streams.size(); StreamID++)
    {
        if (Gxf_MediaTypes_StreamKind(Streams[StreamID].MediaType)==Stream_Video)
        {
            Ztring Title=Streams[StreamID].MediaName;
            size_t Title_Extension_Offset=Title.find(__T(".M0"));
            if (Title_Extension_Offset==std::string::npos || Title_Extension_Offset!=Title.size()-3)
                Title_Extension_Offset=Title.find(__T(".H0"));
            if (Title_Extension_Offset!=std::string::npos && Title_Extension_Offset==Title.size()-3)
            {
                Title.resize(Title.size()-3);
                Streams[StreamID].MediaName=Title;
            }
        }
        if (Gxf_MediaTypes_StreamKind(Streams[StreamID].MediaType)==Stream_Audio && Config->File_Audio_MergeMonoStreams_Get())
        {
            Ztring Title=Streams[StreamID].MediaName;
            size_t Title_Extension_Offset=Title.find(__T(".A0"));
            if (Title_Extension_Offset!=std::string::npos && Title_Extension_Offset==Title.size()-3)
            {
                Title.resize(Title.size()-3);
                for (size_t StreamID2=StreamID+1; StreamID2<Streams.size(); StreamID2++)
                {
                    if (Streams[StreamID2].MediaName==Title+__T(".A")+Ztring::ToZtring(StreamID2-StreamID))
                    {
                        Streams[StreamID].MediaName=Title;
                        if (Streams[StreamID].Parsers.size()==1 && Streams[StreamID2].Parsers.size()==1)
                        {
                            int32u Channels=Streams[StreamID].Parsers[0]->Retrieve(Stream_Audio, 0, Audio_Channel_s_).To_int32u()+Streams[StreamID2].Parsers[0]->Retrieve(Stream_Audio, 0, Audio_Channel_s_).To_int32u();
                            Streams[StreamID].Parsers[0]->Fill(Stream_Audio, 0, Audio_Channel_s_, Channels, 10, true);
                            int32u BitRate=Streams[StreamID].Parsers[0]->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int32u()+Streams[StreamID2].Parsers[0]->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int32u();
                            Streams[StreamID].Parsers[0]->Fill(Stream_Audio, 0, Audio_BitRate, BitRate, 10, true);
                        }
                        Streams[StreamID2].MediaType=(int8u)-1;
                    }
                }
            }
        }
    }

    //For each Streams
    for (size_t StreamID=0; StreamID<Streams.size(); StreamID++)
        Streams_Finish_PerStream(StreamID, Streams[StreamID]);

    //Global
    if (Material_Fields_First_IsValid && Material_Fields_Last_IsValid && Material_Fields_Last-Material_Fields_First)
    {
        int64u FrameCount=(Material_Fields_Last+1-Material_Fields_First)/Material_Fields_FieldsPerFrame;
        Fill(Stream_Video, 0, Video_FrameCount, FrameCount);
        if (Gxf_FrameRate(Streams[0x00].FrameRate_Code))
            Fill(Stream_Video, 0, Video_Duration, ((float64)FrameCount)/Gxf_FrameRate(Streams[0x00].FrameRate_Code)*1000, 0); //In milliseconds

        //We trust more the MPEG Video bitrate thant the rest
        //TODO: Chech why there is incohenrency (mainly about Material File size info in the sample)
        if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video"))
            Fill(Stream_Video, 0, Video_BitRate, Retrieve(Stream_Video, 0, Video_BitRate_Nominal));
    }
    if (Material_File_Size_IsValid)
    {
        //Fill(Stream_General, 0, General_OverallBitRate, ((int64u)Material_File_Size)*1024*8/???);
    }

    //Time code tracks
    for (std::map<int8u, tc>::iterator TimeCode=TimeCodes.begin(); TimeCode!=TimeCodes.end(); ++TimeCode)
    {
        int64u TimeCode_FirstFrame_ms=((File_Gxf_TimeCode*)Streams[TimeCode->first].Parsers[0])->TimeCode_FirstFrame_ms;
        string TimeCode_FirstFrame=((File_Gxf_TimeCode*)Streams[TimeCode->first].Parsers[0])->TimeCode_FirstFrame;
        bool   TimeCode_FirstFrame_Striped=false;
        if (TimeCode_FirstFrame_ms==(int64u)-1)
        {
            TimeCode_FirstFrame_ms=TimeCode->second.Milliseconds;
            TimeCode_FirstFrame=TimeCode->second.String;
            TimeCode_FirstFrame_Striped=true;
        }
        if (TimeCode_FirstFrame_ms!=(int64u)-1)
        {
            Stream_Prepare(Stream_Other);
            Fill(Stream_Other, StreamPos_Last, Other_CodecID, Streams[TimeCode->first].MediaType);
            Fill(Stream_Other, StreamPos_Last, Other_ID, TimeCode->first);
            Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
            Fill(Stream_Other, StreamPos_Last, Other_Format, "SMPTE TC");
            //Fill(Stream_Other, StreamPos_Last, Other_MuxingMode, "Time code track");
            Fill(Stream_Other, StreamPos_Last, Other_TimeCode_FirstFrame, TimeCode_FirstFrame.c_str());
            if (TimeCode_FirstFrame_Striped)
                Fill(Stream_Other, StreamPos_Last, Other_TimeCode_Settings, "Striped");
            if (TimeCode->first<Streams.size())
                Fill(Stream_Other, StreamPos_Last, Other_Title, Streams[TimeCode->first].MediaName);
        }
    }
}

//---------------------------------------------------------------------------
void File_Gxf::Streams_Finish_PerStream(size_t StreamID, stream &Temp)
{
    if (Temp.MediaType==(int8u)-1)
        return;

    //By the parser
    if (Temp.Parsers.size()==1 && Temp.Parsers[0]->Status[IsAccepted])
    {
        StreamKind_Last=Stream_Max;
        StreamPos_Last=(size_t)-1;
        if (Config->ParseSpeed<=1.0)
        {
            Fill(Temp.Parsers[0]);
            Temp.Parsers[0]->Open_Buffer_Unsynch();
        }
        Finish(Temp.Parsers[0]);

        //Video
        bool IsTimeCode=false;
        for (std::map<int8u, tc>::iterator TimeCode=TimeCodes.begin(); TimeCode!=TimeCodes.end(); ++TimeCode)
            if (StreamID==TimeCode->first)
                IsTimeCode=true;
        if (!IsTimeCode && Temp.DisplayInfo)
        {
            if (Temp.Parsers[0]->Count_Get(Stream_Video))
            {
                Stream_Prepare(Stream_Video);

                if (TimeCodes.empty())
                {
                    Fill(Stream_Video, StreamPos_Last, Video_Delay, ((float64)(Material_Fields_First/Material_Fields_FieldsPerFrame))/Gxf_FrameRate(Streams[0x00].FrameRate_Code)*1000, 0);
                    Fill(Stream_Video, StreamPos_Last, Video_Delay_Source, "Container");
                }
                else
                    for (std::map<int8u, tc>::iterator TimeCode=TimeCodes.begin(); TimeCode!=TimeCodes.end(); ++TimeCode)
                    {
                        int64u TimeCode_FirstFrame_ms=((File_Gxf_TimeCode*)Streams[TimeCode->first].Parsers[0])->TimeCode_FirstFrame_ms;
                        string TimeCode_FirstFrame=((File_Gxf_TimeCode*)Streams[TimeCode->first].Parsers[0])->TimeCode_FirstFrame;
                        if (TimeCode_FirstFrame_ms==(int64u)-1)
                        {
                            TimeCode_FirstFrame_ms=TimeCode->second.Milliseconds;
                            TimeCode_FirstFrame=TimeCode->second.String;
                        }
                        if (TimeCode_FirstFrame_ms!=(int64u)-1)
                        {
                            Fill(Stream_Video, StreamPos_Last, Video_Delay, TimeCode_FirstFrame_ms, 0);
                            if (TimeCode_FirstFrame.size()==11)
                                Fill(Stream_Video, StreamPos_Last, Video_Delay_DropFrame, TimeCode_FirstFrame[8]==';'?"Yes":"No");
                            Fill(Stream_Video, StreamPos_Last, Video_Delay_Source, "Container");

                            //Fill(Stream_Video, StreamPos_Last, Video_TimeCode_FirstFrame, TimeCode_FirstFrame.c_str());
                            //Fill(Stream_Video, StreamPos_Last, Video_TimeCode_Source, "Time code track");
                        }
                    }

                Merge(*Temp.Parsers[0], Stream_Video, 0, StreamPos_Last);
                Fill(Stream_Video, StreamPos_Last, Video_CodecID, Temp.MediaType);

                Fill(Stream_Video, StreamPos_Last, Video_ID, StreamID, 10, true);
                Fill(Stream_Video, StreamPos_Last, "Title", Temp.MediaName);

                Ztring LawRating=Temp.Parsers[0]->Retrieve(Stream_General, 0, General_LawRating);
                if (!LawRating.empty())
                    Fill(Stream_General, 0, General_LawRating, LawRating, true);
                Ztring Title=Temp.Parsers[0]->Retrieve(Stream_General, 0, General_Title);
                if (!Title.empty() && Retrieve(Stream_General, 0, General_Title).empty())
                    Fill(Stream_General, 0, General_Title, Title);

                //Special cases
                if (Temp.Parsers[0]->Count_Get(Stream_Text))
                {
                    //Video and Text are together
                    size_t Parser_Text_Count=Temp.Parsers[0]->Count_Get(Stream_Text);
                    for (size_t Parser_Text_Pos=0; Parser_Text_Pos<Parser_Text_Count; Parser_Text_Pos++)
                    {
                        Stream_Prepare(Stream_Text);
                        Merge(*Temp.Parsers[0], Stream_Text, Parser_Text_Pos, StreamPos_Last);
                        Ztring ID=Retrieve(Stream_Text, StreamPos_Last, Text_ID);
                        Fill(Stream_Text, StreamPos_Last, Text_ID, Ztring::ToZtring(AncillaryData_StreamID)+__T("-")+ID, true);
                        Fill(Stream_Text, StreamPos_Last, Text_ID_String, Ztring::ToZtring(AncillaryData_StreamID)+__T("-")+ID, true);
                        Fill(Stream_Text, StreamPos_Last, Text_Delay, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay), true);
                        Fill(Stream_Text, StreamPos_Last, Text_Delay_Source, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Source), true);
                        Fill(Stream_Text, StreamPos_Last, Text_Delay_Original, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Original), true);
                        Fill(Stream_Text, StreamPos_Last, Text_Delay_Original_Source, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Original_Source), true);
                    }

                    StreamKind_Last=Stream_Video;
                    StreamPos_Last=Count_Get(Stream_Video)-1;
                }
            }

            //Audio
            for (size_t Pos=0; Pos<Temp.Parsers[0]->Count_Get(Stream_Audio); Pos++)
            {
                Stream_Prepare(Stream_Audio);
                Fill(Stream_Audio, StreamPos_Last, Audio_CodecID, Temp.MediaType);

                if (TimeCodes.empty())
                {
                    Fill(Stream_Audio, StreamPos_Last, Audio_Delay, ((float64)(Material_Fields_First/Material_Fields_FieldsPerFrame))/Gxf_FrameRate(Streams[0x00].FrameRate_Code)*1000, 0);
                    Fill(Stream_Audio, StreamPos_Last, Audio_Delay_Source, "Container");
                }
                else
                    for (std::map<int8u, tc>::iterator TimeCode=TimeCodes.begin(); TimeCode!=TimeCodes.end(); ++TimeCode)
                    {
                        int64u TimeCode_FirstFrame_ms=((File_Gxf_TimeCode*)Streams[TimeCode->first].Parsers[0])->TimeCode_FirstFrame_ms;
                        string TimeCode_FirstFrame=((File_Gxf_TimeCode*)Streams[TimeCode->first].Parsers[0])->TimeCode_FirstFrame;
                        if (TimeCode_FirstFrame_ms==(int64u)-1)
                        {
                            TimeCode_FirstFrame_ms=TimeCode->second.Milliseconds;
                            TimeCode_FirstFrame=TimeCode->second.String;
                        }
                        if (TimeCode_FirstFrame_ms!=(int64u)-1)
                        {
                            Fill(Stream_Audio, StreamPos_Last, Audio_Delay, TimeCode_FirstFrame_ms, 0);
                            if (TimeCode_FirstFrame.size()==11)
                                Fill(Stream_Audio, StreamPos_Last, Audio_Delay_DropFrame, TimeCode_FirstFrame[8]==';'?"Yes":"No");
                            Fill(Stream_Audio, StreamPos_Last, Audio_Delay_Source, "Container");

                            //Fill(Stream_Audio, StreamPos_Last, Audio_TimeCode_FirstFrame, TimeCode_FirstFrame.c_str());
                            //Fill(Stream_Audio, StreamPos_Last, Audio_TimeCode_Source, "Time code track");
                        }
                    }

                Merge(*Temp.Parsers[0], Stream_Audio, Pos, StreamPos_Last, false);

                Ztring ID;
                if (Temp.IsChannelGrouping)
                    ID=Ztring::ToZtring(StreamID-1)+__T(" / "); //First half of the channel grouping
                ID+=Ztring::ToZtring(StreamID); //Second half of the channel grouping or standalone
                Ztring ID_String=ID;
                if (!Retrieve(Stream_Audio, StreamPos_Last, Audio_ID).empty())
                {
                    ID+=__T('-')+Retrieve(Stream_Audio, StreamPos_Last, Audio_ID);
                    ID_String+=__T('-')+Retrieve(Stream_Audio, StreamPos_Last, Audio_ID_String);
                }
                Fill(Stream_Audio, StreamPos_Last, Audio_ID, ID, true);
                Fill(Stream_Audio, StreamPos_Last, Audio_ID_String, ID_String, true);
                if (Temp.IsChannelGrouping)
                    Fill(Stream_Audio, StreamPos_Last, "Title", Streams[StreamID-1].MediaName); //First half of the channel grouping
                Fill(StreamKind_Last, StreamPos_Last, "Title", Temp.MediaName); //Second half of the channel grouping or standalone

                for (std::map<std::string, Ztring>::iterator Info=Temp.Infos.begin(); Info!=Temp.Infos.end(); ++Info)
                    if (Info->first=="BitRate" && Temp.Parsers[0]->Count_Get(Stream_Audio)>1)
                        Fill(Stream_Audio, StreamPos_Last, Audio_BitRate_Encoded, Pos?Ztring(__T("0")):Info->second); // In case of more than 1 audio sub-stream Encoded bit rate is the bit rate of all streams + overhead
                    else if (Retrieve(Stream_Audio, StreamPos_Last, Info->first.c_str()).empty())
                        Fill(Stream_Audio, StreamPos_Last, Info->first.c_str(), Info->second);
            }

            //Text
            if (Temp.Parsers[0]->Count_Get(Stream_Text))
            {
                size_t Parser_Text_Count=Temp.Parsers[0]->Count_Get(Stream_Text);
                for (size_t Parser_Text_Pos=0; Parser_Text_Pos<Parser_Text_Count; Parser_Text_Pos++)
                {
                    Stream_Prepare(Stream_Text);
                    Merge(*Temp.Parsers[0], Stream_Text, Parser_Text_Pos, StreamPos_Last);
                    Ztring ID=Retrieve(Stream_Text, StreamPos_Last, Text_ID);
                    Fill(Stream_Text, StreamPos_Last, Text_ID, Ztring::ToZtring(AncillaryData_StreamID)+__T("-")+ID, true);
                    Fill(Stream_Text, StreamPos_Last, Text_ID_String, Ztring::ToZtring(AncillaryData_StreamID)+__T("-")+ID, true);
                    Fill(Stream_Text, StreamPos_Last, Text_Delay, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay), true);
                    Fill(Stream_Text, StreamPos_Last, Text_Delay_Source, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Source), true);
                    Fill(Stream_Text, StreamPos_Last, Text_Delay_Original, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Original), true);
                    Fill(Stream_Text, StreamPos_Last, Text_Delay_Original_Source, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Original_Source), true);
                    Fill(Stream_Text, StreamPos_Last, "Title", Temp.MediaName);
                }

                Ztring LawRating=Temp.Parsers[0]->Retrieve(Stream_General, 0, General_LawRating);
                if (!LawRating.empty())
                    Fill(Stream_General, 0, General_LawRating, LawRating, true);
                Ztring Title=Temp.Parsers[0]->Retrieve(Stream_General, 0, General_Title);
                if (!Title.empty() && Retrieve(Stream_General, 0, General_Title).empty())
                    Fill(Stream_General, 0, General_Title, Title);

                StreamKind_Last=Stream_Max;
                StreamPos_Last=(size_t)-1;
            }

            //Other
            if (Temp.Parsers[0]->Count_Get(Stream_Other))
            {
                size_t Parser_Other_Count=Temp.Parsers[0]->Count_Get(Stream_Other);
                for (size_t Parser_Other_Pos=0; Parser_Other_Pos<Parser_Other_Count; Parser_Other_Pos++)
                {
                    Stream_Prepare(Stream_Other);
                    Fill(Stream_Other, StreamPos_Last, Other_CodecID, Temp.MediaType);
                    Merge(*Temp.Parsers[0], Stream_Other, Parser_Other_Pos, StreamPos_Last);
                    Ztring ID=Retrieve(Stream_Other, StreamPos_Last, Other_ID);
                    Fill(Stream_Other, StreamPos_Last, Other_ID, Ztring::ToZtring(AncillaryData_StreamID)+__T("-")+ID, true);
                    Fill(Stream_Other, StreamPos_Last, Other_ID_String, Ztring::ToZtring(AncillaryData_StreamID)+__T("-")+ID, true);
                    /*
                    Fill(Stream_Other, StreamPos_Last, Other_Delay, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay), true);
                    Fill(Stream_Other, StreamPos_Last, Other_Delay_Source, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Source), true);
                    Fill(Stream_Other, StreamPos_Last, Other_Delay_Original, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Original), true);
                    Fill(Stream_Other, StreamPos_Last, Other_Delay_Original_Source, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Original_Source), true);
                    */
                    Fill(Stream_Other, StreamPos_Last, "Title", Temp.MediaName);
                }

                StreamKind_Last=Stream_Max;
                StreamPos_Last=(size_t)-1;
            }
        }
    }
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Gxf::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+16<=Buffer_Size)
    {
        while (Buffer_Offset+16<=Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                              || Buffer[Buffer_Offset+1]!=0x00
                                              || Buffer[Buffer_Offset+2]!=0x00
                                              || Buffer[Buffer_Offset+3]!=0x00
                                              || Buffer[Buffer_Offset+4]!=0x01
                                              || Buffer[Buffer_Offset+14]!=0xE1
                                              || Buffer[Buffer_Offset+15]!=0xE2))
        {
            Buffer_Offset+=4;
            while (Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x00)
                Buffer_Offset+=4;
            for (int8u Pos=0; Pos<3; Pos++)
                if (Buffer_Offset>=Buffer_Size || Buffer[Buffer_Offset-1]==0x00)
                    Buffer_Offset--;
        }

        if (Buffer_Offset+16<=Buffer_Size) //Testing if size is coherant
        {
            //Retrieving some info
            int32u Size=BigEndian2int32u(Buffer+Buffer_Offset+6);

            //Testing
            if (Buffer_Offset+Size+16>Buffer_Size)
                return false; //Need more data
            if (Buffer[Buffer_Offset+Size  ]!=0x00
             || Buffer[Buffer_Offset+Size+1]!=0x00
             || Buffer[Buffer_Offset+Size+2]!=0x00
             || Buffer[Buffer_Offset+Size+3]!=0x00
             || Buffer[Buffer_Offset+Size+4]!=0x01
             || Buffer[Buffer_Offset+Size+14]!=0xE1
             || Buffer[Buffer_Offset+Size+15]!=0xE2)
                 Buffer_Offset++;
            else
                break;
        }
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+16>Buffer_Size)
    {
        return false;
    }

    if (!Status[IsAccepted])
    {
        Accept("GXF");
        Fill(Stream_General, 0, General_Format, "GXF");
        Streams.resize(0x40);
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Gxf::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+16>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC5(Buffer+Buffer_Offset   )!=0x0000000001
     || CC2(Buffer+Buffer_Offset+14)!=0xE1E2)
        Synched=false;

    //Test if the next synchro is available
    int32u PacketLength=BigEndian2int32u(Buffer+Buffer_Offset+6);
    if (File_Offset+Buffer_Offset+PacketLength+16<=File_Size)
    {
        if (Buffer_Offset+PacketLength+16>Buffer_Size)
            return false;
        if (CC5(Buffer+Buffer_Offset+PacketLength   )!=0x0000000001
         || CC2(Buffer+Buffer_Offset+PacketLength+14)!=0xE1E2)
            Synched=false;
    }

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gxf::Read_Buffer_Unsynched()
{
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        for (size_t Parser_Pos=0; Parser_Pos<Streams[Pos].Parsers.size(); Parser_Pos++)
            Streams[Pos].Parsers[Parser_Pos]->Open_Buffer_Unsynch();

    #if MEDIAINFO_SEEK
        IFrame_IsParsed=false;
    #endif //MEDIAINFO_SEEK
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_Gxf::Read_Buffer_Seek (size_t Method, int64u Value, int64u)
{
    //Parsing
    switch (Method)
    {
        case 0  :   Open_Buffer_Unsynch(); GoTo(Value); return 1;
        case 1  :   Open_Buffer_Unsynch(); GoTo(File_Size*Value/10000); return 1;
        case 2  :   //Timestamp
                    {
                        //We transform TimeStamp to a frame number
                        if (Streams.empty() || Gxf_FrameRate(Streams[0x00].FrameRate_Code)==0)
                            return (size_t)-1; //Not supported

                        int64u Delay=0;
                        if (TimeCodes.empty())
                        {
                            if (Material_Fields_First_IsValid)
                                Delay=float64_int64s(((float64)(Material_Fields_First/Material_Fields_FieldsPerFrame))/Gxf_FrameRate(Streams[0x00].FrameRate_Code)*1000000000);
                            else
                                Delay=0;
                        }
                        else
                        {
                            for (std::map<int8u, tc>::iterator TimeCode=TimeCodes.begin(); TimeCode!=TimeCodes.end(); ++TimeCode)
                            {
                                int64u TimeCode_First=((File_Gxf_TimeCode*)Streams[TimeCode->first].Parsers[0])->TimeCode_FirstFrame_ms;
                                if (TimeCode_First==(int64u)-1)
                                    TimeCode_First=TimeCode->second.Milliseconds;
                                if (TimeCode_First==(int64u)-1)
                                    Delay=0;
                                else
                                {
                                    Delay=TimeCode_First*1000000;
                                    break;
                                }
                            }
                        }

                        if (Value<Delay)
                            Value=0;
                        else
                            Value=float64_int64s(((float64)(Value-Delay))/1000000000*Gxf_FrameRate(Streams[0x00].FrameRate_Code));
                    }
                    //No break;
        case 3  :   //FrameNumber
                    {
                    if (Seeks.empty())
                        return (size_t)-1; //Not supported

                    //Search previous I-Frame
                    if (UMF_File && ((File_Umf*)UMF_File)->GopSize!=(int64u)-1)
                    {
                        Value/=((File_Umf*)UMF_File)->GopSize;
                        Value*=((File_Umf*)UMF_File)->GopSize;
                    }
                    Value*=Material_Fields_FieldsPerFrame;

                    for (size_t Pos=0; Pos<Seeks.size(); Pos++)
                    {
                        if (Material_Fields_First+Value<=Seeks[Pos].FrameNumber)
                        {
                            if (Material_Fields_First+Value<Seeks[Pos].FrameNumber && Pos)
                                Pos--;
                            Open_Buffer_Unsynch();
                            GoTo(((int64u)Seeks[Pos].StreamOffset)*1024);

                            return 1;
                        }
                    }

                    return 2; //Invalid value
                    }
        default :   return (size_t)-1; //Not supported
    }
}
#endif //MEDIAINFO_SEEK

//---------------------------------------------------------------------------
void File_Gxf::Read_Buffer_AfterParsing()
{
    if (File_GoTo==(int64u)-1 && File_Offset+Buffer_Offset>=IsParsingMiddle_MaxOffset)
    {
        Fill();
        Open_Buffer_Unsynch();
        Finish();
        return;
    }
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Gxf::Header_Begin()
{
    #if MEDIAINFO_DEMUX
        //Handling of multiple frames in one block
        if (Element_Code==0xBF && Config->Demux_Unpacketize_Get() && Streams[TrackNumber].Demux_EventWasSent) //media block
        {
            Open_Buffer_Continue(Streams[TrackNumber].Parsers[0], Buffer+Buffer_Offset, 0);
            if (Config->Demux_EventWasSent)
                return false;
            Streams[TrackNumber].Demux_EventWasSent=false;
        }
    #endif //MEDIAINFO_DEMUX

    return true;
}

//---------------------------------------------------------------------------
void File_Gxf::Header_Parse()
{
    //Parsing
    int32u PacketLength;
    int8u  PacketType;
    Skip_B5(                                                    "Packet leader");
    Get_B1 (PacketType,                                         "Packet type");
    Get_B4 (PacketLength,                                       "Packet length");
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Packet trailer");

    //Filling
    Header_Fill_Size(PacketLength);
    Header_Fill_Code(PacketType);

    #if MEDIAINFO_DEMUX
        if (!Demux_HeaderParsed)
        {
            if (PacketType==0xBF) //media
            {
                if (Config->NextPacket_Get() && Config->Event_CallBackFunction_IsSet())
                    Config->Demux_EventWasSent=true; //First set is to indicate the user that header is parsed
                Demux_HeaderParsed=true;
            }
        }
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
void File_Gxf::Data_Parse()
{
    //Counting
    Frame_Count++;

    switch (Element_Code)
    {
        case 0x00 : Finish("GXF"); break;
        case 0xBC : map(); break;
        case 0xBF : media(); break;
        case 0xFB : end_of_stream(); break;
        case 0xFC : field_locator_table(); break;
        case 0xFD : UMF_file(); break;
        default: ;
    }
}

//---------------------------------------------------------------------------
void File_Gxf::map()
{
    Element_Name("map");

    //Parsing
    int8u Version;
    Element_Begin1("Preamble");
        BS_Begin();
        Mark_1();
        Mark_1();
        Mark_1();
        Get_S1(5, Version,                                      "Version");
        BS_End();
        Skip_B1(                                                "Reserved");
    Element_End0();

    Element_Begin1("Material Data");
        int16u SectionLength;
        Get_B2 (SectionLength,                                  "Section Length");
        if (Element_Offset+SectionLength>=Element_Size)
            SectionLength=(int16u)(Element_Size-Element_Offset);
        int64u Material_Data_End=Element_Offset+SectionLength;
        while (Element_Offset<Material_Data_End)
        {
            Element_Begin1("Tag");
            int8u Tag, DataLength;
            Get_B1(Tag,                                         "Tag");
            Get_B1(DataLength,                                  "Data Length");
            Element_Name(Gxf_Tag_Name(Tag));
            switch (Tag)
            {
                case 0x40 : //Media file name of material
                            {
                            Ztring MediaFileName;
                            Get_Local(DataLength, MediaFileName, "Content");
                            Fill(Stream_General, 0, General_Title, MediaFileName, true);
                            }
                            break;
                case 0x41 : //First field of material in stream
                            if (DataLength==4)
                            {
                                if (Material_Fields_First_IsValid)
                                    Skip_B4(                    "Content");
                                else
                                {
                                    Get_B4 (Material_Fields_First, "Content");
                                    Material_Fields_First_IsValid=true;
                                }
                            }
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x42 : //Last field of material in stream
                            if (DataLength==4)
                            {
                                if (Material_Fields_Last_IsValid)
                                    Skip_B4(                    "Content");
                                else
                                {
                                    Get_B4 (Material_Fields_Last, "Content");
                                    Material_Fields_Last_IsValid=true;
                                }
                            }
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x43 : //Mark in for the stream
                            if (DataLength==4)
                                Skip_B4(                        "Content");
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x44 : //Mark out for the stream
                            if (DataLength==4)
                                Skip_B4(                        "Content");
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x45 : //Estimated size of stream in 1024 byte units
                            if (DataLength==4)
                            {
                                Get_B4 (Material_File_Size  ,   "Content");
                                Material_File_Size_IsValid=true;
                            }
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x46 : //Reserved
                            if (DataLength==4)
                                Skip_B4(                        "Content");
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x47 : //Reserved
                            if (DataLength==8)
                                Skip_B8(                        "Content");
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x48 : //Reserved
                            Skip_String(DataLength,             "Content");
                            break;
                case 0x49 : //Reserved
                            Skip_String(DataLength,             "Content");
                            break;
                case 0x4A : //Reserved
                            Skip_String(DataLength,             "Content");
                            break;
                case 0x4B : //Reserved
                            Skip_String(DataLength,             "Content");
                            break;
                default   : Skip_XX(DataLength,                 "Unknown");
            }
            Element_End0();
        }
    Element_End0();

    Element_Begin1("Track Description");
        int32u Stream_Video_FrameRate_Code=(int32u)-1, Stream_Video_FieldsPerFrame_Code=(int32u)-1;
        Get_B2 (SectionLength,                                  "Section Length");
        if (Element_Offset+SectionLength>=Element_Size)
            SectionLength=(int16u)(Element_Size-Element_Offset);
        int64u Track_Data_End=Element_Offset+SectionLength;
        while (Element_Offset<Track_Data_End)
        {
            Element_Begin1("Track");
            int16u TrackLength;
            int8u  MediaType, TrackID;
            Get_B1 (MediaType,                                  "Media type"); Param_Info1(Gxf_MediaTypes(MediaType&0x7F));
            Get_B1 (TrackID,                                    "Track ID");
            Get_B2 (TrackLength,                                "Track Length");
            if (Element_Offset+TrackLength>=Track_Data_End)
                TrackLength=(int16u)(Track_Data_End-Element_Offset);
            int64u Track_End=Element_Offset+TrackLength;
            Element_Info1(TrackID&0x3F);
            Element_Info1(Gxf_MediaTypes(MediaType&0x7F));

            FILLING_BEGIN();
                MediaType&=0x7F; //Remove the last bit
                TrackID&=0x3F; //Remove the 2 last bits
                Streams[TrackID].MediaType=MediaType;
                Streams[TrackID].TrackID=TrackID;
                if (Streams[TrackID].Parsers.empty())
                {
                    Streams[TrackID].MediaType=MediaType;
                    Streams[TrackID].TrackID=TrackID;

                    //Parsers
                    #if MEDIAINFO_DEMUX
                        Element_Code=TrackID;
                    #endif //MEDIAINFO_DEMUX
                    switch (MediaType)
                    {
                        case  3 :
                        case  4 :  //JPEG
                                    {
                                        File__Analyze* Parser=new File__Analyze;
                                        Open_Buffer_Init(Parser);
                                        Parser->Accept();
                                        Parser->Fill();
                                        Parser->Stream_Prepare(Stream_Video);
                                        Parser->Fill(Stream_Video, 0, Video_Format, "JPEG");
                                        Streams[TrackID].Parsers.push_back(Parser);
                                    }
                                    break;
                        case  7 :
                        case  8 :
                        case 24 :  //TimeCode
                                    {
                                        File__Analyze* Parser=new File_Gxf_TimeCode;
                                        Open_Buffer_Init(Parser);
                                        Streams[TrackID].Parsers.push_back(Parser);

                                        Parsers_Count++;
                                        Streams[TrackID].Searching_Payload=true;
                                        TimeCodes[TrackID].Milliseconds=(int64u)-1;
                                    }
                                    break;
                        case  9 :
                        case 10 :
                                    {
                                        File__Analyze* Parser=new File__Analyze;
                                        Open_Buffer_Init(Parser);
                                        Parser->Accept();
                                        Parser->Fill();
                                        Parser->Stream_Prepare(Stream_Audio);
                                        Parser->Fill(Stream_Audio, 0, Audio_Format, "PCM");
                                        Parser->Fill(Stream_Audio, 0, Audio_Format_Settings_Endianness, "Little");
                                        Streams[TrackID].Parsers.push_back(Parser);

                                        Audio_Count++;
                                    }
                                    break;
                        case 11 :
                        case 12 :
                        case 20 :
                        case 22 :
                        case 23 :   //MPEG Video
                                     {
                                        File__Analyze* Parser=new File_Mpegv();
                                        ((File_Mpegv*)Parser)->FrameIsAlwaysComplete=true;
                                        ((File_Mpegv*)Parser)->Ancillary=&Ancillary;
                                        Open_Buffer_Init(Parser);
                                        Streams[TrackID].Parsers.push_back(Parser);

                                        Parsers_Count++;
                                        Streams[TrackID].Searching_Payload=true;
                                     }
                                    break;
                        case 13 :
                        case 14 :
                        case 15 :
                        case 16 :
                        case 25 :   // was found for DVCPro HD in some files (not in SMPTE ST 360-2009, maybe it is present in a later version)
                                    //DV
                                    {
                                        File__Analyze* Parser=new File_DvDif();
                                        Open_Buffer_Init(Parser);
                                        Streams[TrackID].Parsers.push_back(Parser);

                                        Parsers_Count++;
                                        Streams[TrackID].Searching_Payload=true;
                                    }
                                    break;
                        case 17 :   //AC-3 in AES3 (half)
                        case 18 :   //Dolby E in AES3 (half)
                                    {
                                        File__Analyze* Parser=ChooseParser_ChannelGrouping(TrackID);
                                        if (Parser)
                                        {
                                            Open_Buffer_Init(Parser);
                                            Streams[TrackID].Parsers.push_back(Parser);

                                            Parsers_Count++;
                                            Audio_Count++;
                                            Streams[TrackID].Searching_Payload=true;
                                        }
                                    }
                                    break;
                        case 21 :   //Ancillary Metadata
                                    {
                                        File__Analyze* Parser=new File_Riff();
                                        ((File_Riff*)Parser)->Ancillary=&Ancillary;
                                        Open_Buffer_Init(Parser);
                                        Streams[TrackID].Parsers.push_back(Parser);

                                        Parsers_Count++;
                                        Streams[TrackID].Searching_Payload=true;

                                        Ancillary=new File_Ancillary;
                                        Ancillary->WithTenBit=true;
                                        Ancillary->WithChecksum=true;
                                        Open_Buffer_Init(Ancillary);
                                        AncillaryData_StreamID=TrackID;
                                        if (SizeToAnalyze<8*16*1024*1024)
                                            SizeToAnalyze*=8; //10x more, to be sure to find captions
                                    }
                                    break;
                        default :   ;
                    }

                    if (Gxf_MediaTypes_StreamKind(MediaType)==Stream_Audio)
                    {
                        //Resolution
                        switch (MediaType)
                        {
                            case  9 :
                            case 18 :   //24-bit
                                        Streams[TrackID].Infos["BitDepth"].From_Number(24);
                                        break;
                            case 10 :
                            case 17 :   //16-bit
                                        Streams[TrackID].Infos["BitDepth"].From_Number(16);
                                        break;
                            default : ;
                        }

                        //Channels
                        switch (MediaType)
                        {
                            case  9 :
                            case 10 :   //Mono
                                        Streams[TrackID].Infos["Channel(s)"].From_Number(1);
                                        break;
                            case 18 :   //Stereo
                                        Streams[TrackID].Infos["Channel(s)"].From_Number(2);
                                        break;
                            default : ;
                        }

                        //Sampling rate
                        switch (MediaType)
                        {
                            case  9 :
                            case 10 :
                            case 17 :
                            case 18 :   //48000
                                        Streams[TrackID].Infos["SamplingRate"].From_Number(48000);
                                        break;
                            default : ;
                        }

                        //Bit rate
                        switch (MediaType)
                        {
                            case  9 :   //Mono, 48 KHz, 24-bit (or padded up to 24-bit)
                                        Streams[TrackID].Infos["BitRate"].From_Number(1*48000*24);
                                        break;
                            case 10 :   //Mono, 48 KHz, 16-bit
                                        Streams[TrackID].Infos["BitRate"].From_Number(1*48000*16);
                                        break;
                            case 18 :   //Stereo, 48 KHz, 24-bit (or padded up to 24-bit)
                                        Streams[TrackID].Infos["BitRate"].From_Number(2*48000*24);
                                        break;
                            default : ;
                        }
                    }
                }
            FILLING_END();

            int8u Hours=(int8u)-1, Minutes=(int8u)-1, Seconds=(int8u)-1, Fields=(int8u)-1;
            bool  Invalid=true, DropFrame=true;
            bool  TimeCode_Parsed=false;

            while (Element_Offset<Track_End)
            {
                Element_Begin1("Tag");
                int8u Tag, DataLength;
                Get_B1(Tag,                                     "Tag");
                Get_B1(DataLength,                              "Data Length");
                Element_Name(Gxf_Tag_Name(Tag));
                switch (Tag)
                {
                    case 0x4C : //Media name
                                {
                                    Get_Local(DataLength, Streams[TrackID].MediaName, "Content");
                                }
                                break;
                    case 0x4D : //Auxiliary Information
                                if (DataLength==8)
                                {
                                    if (MediaType==21)
                                    {
                                        //Ancillary
                                        Skip_B1(                "Reserved");
                                        Skip_B1(                "Reserved");
                                        Skip_B1(                "Ancillary data presentation format");
                                        Skip_B1(                "Number of ancillary data fields per ancillary data media packet");
                                        Skip_B2(                "Byte size of each ancillary data field");
                                        Skip_B2(                "Byte size of the ancillary data media packet in 256 byte units");
                                    }
                                    else if (MediaType==7 || MediaType==8 || MediaType==24)
                                    {
                                        //TimeCode
                                        Get_B1 (Fields,         "Fields");
                                        Get_B1 (Seconds,        "Second");
                                        Get_B1 (Minutes,        "Minute");
                                        BS_Begin();
                                        Get_SB (   Invalid,     "Invalid");
                                        Skip_SB(                "Color frame");
                                        Get_SB (   DropFrame,   "Drop frame");
                                        Get_S1 (5, Hours,       "Hour");
                                        BS_End();
                                        Skip_B1(                "User bits");
                                        Skip_B1(                "User bits");
                                        Skip_B1(                "User bits");
                                        Skip_B1(                "User bits");
                                        if (!Invalid)
                                            TimeCode_Parsed=true;
                                    }
                                    else
                                        Skip_B8(                "Content");
                                }
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    case 0x4E : //Media file system version
                                if (DataLength==4)
                                    Skip_B4(                    "Content");
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    case 0x4F : //MPEG auxiliary information
                                Skip_String(DataLength,         "Content");
                                break;
                    case 0x50 : //Frame rate
                                if (DataLength==4)
                                {
                                    Get_B4 (Streams[TrackID].FrameRate_Code, "Content"); Param_Info1(Gxf_FrameRate(Streams[TrackID].FrameRate_Code)); Element_Info1(Gxf_FrameRate(Streams[TrackID].FrameRate_Code));
                                    for (std::map<int8u, tc>::iterator TimeCode=TimeCodes.begin(); TimeCode!=TimeCodes.end(); ++TimeCode)
                                        if (TrackID==TimeCode->first)
                                            for (size_t Pos=0; Pos<Streams[TrackID].Parsers.size(); Pos++)
                                                ((File_Gxf_TimeCode*)Streams[TrackID].Parsers[Pos])->FrameRate_Code=Streams[0x00].FrameRate_Code;
                                    if (Gxf_MediaTypes_StreamKind(MediaType)==Stream_Video)
                                        Stream_Video_FrameRate_Code=Streams[TrackID].FrameRate_Code;
                                }
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    case 0x51 : //Lines per frame
                                if (DataLength==4)
                                {
                                    Get_B4 (Streams[TrackID].LinesPerFrame_Code, "Content"); Param_Info1(Gxf_LinesPerFrame_Height(Streams[TrackID].LinesPerFrame_Code)); Element_Info1(Gxf_LinesPerFrame_Height(Streams[TrackID].LinesPerFrame_Code));
                                }
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    case 0x52 : //Fields per frame
                                if (DataLength==4)
                                {
                                    Get_B4 (Streams[TrackID].FieldsPerFrame_Code, "Content"); Param_Info1(Gxf_FieldsPerFrame(Streams[TrackID].FieldsPerFrame_Code)); Element_Info1(Gxf_FieldsPerFrame(Streams[TrackID].FieldsPerFrame_Code));
                                    if (Gxf_MediaTypes_StreamKind(MediaType)==Stream_Video)
                                    {
                                        Stream_Video_FieldsPerFrame_Code=Streams[TrackID].FieldsPerFrame_Code;
                                        Material_Fields_FieldsPerFrame=Streams[TrackID].FieldsPerFrame_Code;
                                    }
                                    for (std::map<int8u, tc>::iterator TimeCode=TimeCodes.begin(); TimeCode!=TimeCodes.end(); ++TimeCode)
                                        if (TrackID==TimeCode->first)
                                            for (size_t Pos=0; Pos<Streams[TrackID].Parsers.size(); Pos++)
                                                ((File_Gxf_TimeCode*)Streams[TrackID].Parsers[Pos])->FieldsPerFrame_Code=Streams[0x00].FieldsPerFrame_Code;
                                }
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    default   : Skip_XX(DataLength,             "Unknown");
                }
                Element_End0();
            }
            Element_End0();

            //Test on TimeCode
            if (TimeCode_Parsed && !Invalid)
            {
                std::map<int8u, tc>::iterator TimeCode=TimeCodes.find(TrackID);
                if (TimeCode==TimeCodes.end() || TimeCode->second.Milliseconds==(int64u)-1)
                {
                    float64 FrameRate=Gxf_FrameRate(Streams[TrackID].FrameRate_Code);
                    TimeCodes[TrackID].Milliseconds=Hours  *60*60*1000
                                                   +Minutes   *60*1000
                                                   +Seconds      *1000;
                    MediaInfoLib::TimeCode TC;
                    TC.Hours=Hours;
                    TC.Minutes=Minutes;
                    TC.Seconds=Seconds;
                    TC.Frames=Fields/2;
                    TC.DropFrame=DropFrame;
                    TimeCodes[TrackID].String=TC.ToString();

                    if (!FrameRate)
                    {
                        //Time code frame rate is missing, using the video frame rate
                        for (size_t Pos=0; Pos<Streams.size(); Pos++)
                            if (Streams[Pos].FrameRate_Code!=(int32u)-1)
                            {
                                FrameRate=Gxf_FrameRate(Streams[Pos].FrameRate_Code);
                                break;
                            }
                    }
                    if (FrameRate)
                        float64_int64s(Fields*1000/(FrameRate*2));
                }
            }
        }
    Element_End0();
    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Padding");

    //Filling missing frame rates for PCM
    for (size_t TrackID=0; TrackID<Streams.size(); TrackID++)
    {
        if (Gxf_FrameRate(Streams[TrackID].FrameRate_Code)==0)
        {
            Streams[TrackID].FrameRate_Code=Stream_Video_FrameRate_Code;
            Streams[TrackID].FieldsPerFrame_Code=Stream_Video_FieldsPerFrame_Code;
        }
        if (Material_Fields_First_IsValid && Gxf_MediaTypes_StreamKind(Streams[TrackID].MediaType)==Stream_Audio) //In case of offset, MediaFieldNumber-Material_Fields_First is not well rounded
        {
            float64 Temp=((float64)Material_Fields_First/Streams[TrackID].FieldsPerFrame_Code)/Gxf_FrameRate(Streams[TrackID].FrameRate_Code);
            Temp*=48000; //TODO: find where this piece of info is available
            Temp/=32768;
            Temp-=(int64u)(Temp);
            if (Temp)
            {
                Temp=((float64)32768)/48000*(1-Temp); //Duration of the first frame not counted
                Streams[TrackID].FirstFrameDuration=float64_int64s(Temp*1000000000);
            }
        }
    }
}

//---------------------------------------------------------------------------
void File_Gxf::media()
{
    Element_Name("media");

    //Parsing
    int32u  MediaFieldNumber;
    int8u   MediaType;
    Element_Begin1("Preamble");
        Get_B1 (MediaType,                                      "Media type");
        Get_B1 (TrackNumber,                                    "Track number");
        Get_B4 (MediaFieldNumber,                               "Media field number");
        Skip_B1(                                                "Field information");
        Skip_B1(                                                "Field information");
        Skip_B1(                                                "Field information");
        Skip_B1(                                                "Field information");
        Skip_B4(                                                "Time line field number");
        Skip_B1(                                                "Flags");
        Skip_B1(                                                "Reserved");
        TrackNumber&=0x3F;
    Element_End0();
    Element_Info1(TrackNumber);

    //Managing audio 32768-sample DTS synchro
    if (Gxf_MediaTypes_StreamKind(MediaType)==Stream_Audio && MediaFieldNumber==Material_Fields_First && Gxf_FrameRate(Streams[TrackNumber].FrameRate_Code) && Streams[TrackNumber].FirstFrameDuration)
    {
        float64 Temp=((float64)MediaFieldNumber/Material_Fields_FieldsPerFrame)/Gxf_FrameRate(Streams[TrackNumber].FrameRate_Code);
        Temp*=48000; //TODO: find where this piece of info is available
        Temp/=32768;
        Temp-=(int64u)(Temp);
        Temp*=(Element_Size-Element_Offset);
        int64u ByteOffset=(int64u)Temp;
        int64u SampleSize=(Element_Size-Element_Offset)/32768;
        ByteOffset/=SampleSize; //Need to be in sync with sample size
        ByteOffset*=SampleSize;
        Element_Offset+=ByteOffset;
    }

    #if MEDIAINFO_SEEK
        if (!IFrame_IsParsed)
        {
            if (UMF_File && ((File_Umf*)UMF_File)->GopSize!=(int64u)-1)
                IFrame_IsParsed=(((MediaFieldNumber-(Material_Fields_First_IsValid?Material_Fields_First:0))/Material_Fields_FieldsPerFrame)%((File_Umf*)UMF_File)->GopSize)==0;
            else if (Gxf_MediaTypes_StreamKind(Streams[TrackNumber].MediaType)==Stream_Video)
                IFrame_IsParsed=true;
        }
    #endif //MEDIAINFO_SEEK

    #if MEDIAINFO_DEMUX
            if (Streams[TrackNumber].MediaType!=8) //Not a TimeCode stream
            {
                Element_Code=TrackNumber;
                int64u TimeCode_First=0;
                if (!TimeCodes.empty())
                {
                    TimeCode_First=((File_Gxf_TimeCode*)Streams[TimeCodes.begin()->first].Parsers[0])->TimeCode_FirstFrame_ms;
                    if (TimeCode_First==(int64u)-1)
                        TimeCode_First=TimeCodes.begin()->second.Milliseconds;
                }
                if (TimeCode_First!=(int64u)-1)
                    TimeCode_First*=1000000;
                if (Gxf_MediaTypes_StreamKind(Streams[TrackNumber].MediaType)==Stream_Video)
                {
                    if (Gxf_FrameRate(Streams[TrackNumber].FrameRate_Code))
                    {
                        FrameInfo.DTS=TimeCode_First+float64_int64s(((float64)MediaFieldNumber-(Material_Fields_First_IsValid?Material_Fields_First:0))/Material_Fields_FieldsPerFrame*1000000000/Gxf_FrameRate(Streams[TrackNumber].FrameRate_Code));
                        FrameInfo.PTS=(int64u)-1;
                        FrameInfo.DUR=float64_int64s(((float64)1000000000)/Gxf_FrameRate(Streams[TrackNumber].FrameRate_Code));
                    }
                    else
                        FrameInfo.DTS=FrameInfo.PTS=FrameInfo.DUR=(int64u)-1;
                    if (MediaFieldNumber-(Material_Fields_First_IsValid?Material_Fields_First:0)==0)
                        Demux_random_access=true;
                    else
                    {
                        if (UMF_File && ((File_Umf*)UMF_File)->GopSize!=(int64u)-1)
                            Demux_random_access=(((MediaFieldNumber-(Material_Fields_First_IsValid?Material_Fields_First:0))/Material_Fields_FieldsPerFrame)%((File_Umf*)UMF_File)->GopSize)==0;
                        else
                            Demux_random_access=false;
                    }
                }
                else if (Gxf_MediaTypes_StreamKind(Streams[TrackNumber].MediaType)==Stream_Audio)
                {
                    if (Gxf_FrameRate(Streams[TrackNumber].FrameRate_Code))
                    {
                        Frame_Count_NotParsedIncluded=(int64u)((MediaFieldNumber-(Material_Fields_First_IsValid?Material_Fields_First:0))/Gxf_FrameRate(Streams[TrackNumber].FrameRate_Code)*48000/32768/Material_Fields_FieldsPerFrame); //A block is 32768 samples at 48 KHz
                        FrameInfo.PTS=TimeCode_First+Frame_Count_NotParsedIncluded*1000000000*32768/48000; //A block is 32768 samples at 48 KHz
                        if (Material_Fields_First_IsValid && MediaFieldNumber!=Material_Fields_First && Streams[TrackNumber].FirstFrameDuration) //In case of offset, MediaFieldNumber-Material_Fields_First is not well rounded
                        {
                            FrameInfo.PTS+=Streams[TrackNumber].FirstFrameDuration;
                            Frame_Count_NotParsedIncluded++;
                        }
                        FrameInfo.DTS=FrameInfo.PTS;
                    }
                    else
                        FrameInfo.DTS=FrameInfo.PTS=(int64u)-1;
                    FrameInfo.DUR=float64_int64s(((float64)1000000000)*32768/48000);
                    Demux_random_access=true;
                }
                else
                {
                    if (Gxf_FrameRate(Streams[0x00].FrameRate_Code))
                        FrameInfo.DTS=FrameInfo.PTS=TimeCode_First+float64_int64s(((float64)(MediaFieldNumber-(Material_Fields_First_IsValid?Material_Fields_First:0)))*1000000000/Gxf_FrameRate(Streams[0x00].FrameRate_Code)/Material_Fields_FieldsPerFrame);
                    else
                        FrameInfo.DTS=FrameInfo.PTS=(int64u)-1;
                    FrameInfo.DUR=(int64u)-1;
                    Demux_random_access=true;
                }
                #if MEDIAINFO_SEEK
                    if (Gxf_MediaTypes_StreamKind(Streams[TrackNumber].MediaType)!=Stream_Video || IFrame_IsParsed)
                #endif //MEDIAINFO_SEEK
                {
                    if (Gxf_MediaTypes_StreamKind(Streams[TrackNumber].MediaType)!=Stream_Audio)
                        Frame_Count_NotParsedIncluded=(MediaFieldNumber-(Material_Fields_First_IsValid?Material_Fields_First:0))/Material_Fields_FieldsPerFrame;
                    Demux_Level=(!Streams[TrackNumber].Parsers.empty() && (Streams[TrackNumber].Parsers[0]->Demux_UnpacketizeContainer || Streams[TrackNumber].Parsers[0]->Demux_Level==2))?4:2; //Intermediate (D-10 Audio) / Container
                    Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset), ContentType_MainStream);
                }
                Element_Code=0xBF; //media
            }
    #endif //MEDIAINFO_DEMUX

    //Needed?
    if (!Streams[TrackNumber].Searching_Payload && IsParsingMiddle_MaxOffset==(int64u)-1)
    {
        Skip_XX(Element_Size-Element_Offset,                    "data");
        //Element_DoNotShow();
        return;
    }

    for (size_t Pos=0; Pos<Streams[TrackNumber].Parsers.size(); Pos++)
    {
        Streams[TrackNumber].Parsers[Pos]->FrameInfo.DTS=FrameInfo.DTS;
        Open_Buffer_Continue(Streams[TrackNumber].Parsers[Pos], Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent && Config->Demux_Unpacketize_Get())
                Streams[TrackNumber].Demux_EventWasSent=true;
        #endif //MEDIAINFO_DEMUX

        //Multiple parsers
        if (Streams[TrackNumber].Parsers.size()>1)
        {
            if (!Streams[TrackNumber].Parsers[Pos]->Status[IsAccepted] && Streams[TrackNumber].Parsers[Pos]->Status[IsFinished])
            {
                delete *(Streams[TrackNumber].Parsers.begin()+Pos);
                Streams[TrackNumber].Parsers.erase(Streams[TrackNumber].Parsers.begin()+Pos);
                Pos--;
            }
            else if (Streams[TrackNumber].Parsers.size()>1 && Streams[TrackNumber].Parsers[Pos]->Status[IsAccepted])
            {
                File__Analyze* Parser=Streams[TrackNumber].Parsers[Pos];
                for (size_t Pos2=0; Pos2<Streams[TrackNumber].Parsers.size(); Pos2++)
                {
                    if (Pos2!=Pos)
                        delete *(Streams[TrackNumber].Parsers.begin()+Pos2);
                }
                Streams[TrackNumber].Parsers.clear();
                Streams[TrackNumber].Parsers.push_back(Parser);
            }
        }
    }

    Element_Offset=Element_Size;

    if (IsParsingMiddle_MaxOffset!=(int64u)-1 && Config->ParseSpeed<1 && Streams[TrackNumber].Parsers.size()==1 && Streams[TrackNumber].Parsers[0]->Status[IsFilled])
    {
        Streams[TrackNumber].Searching_Payload=false;

        if (Parsers_Count>0)
            Parsers_Count--;
        if (Parsers_Count==0)
        {
            TryToFinish();
        }
    }
}

//---------------------------------------------------------------------------
void File_Gxf::end_of_stream()
{
    Element_Name("end of stream");
}

//---------------------------------------------------------------------------
void File_Gxf::field_locator_table()
{
    Element_Name("field locator table");

    //Parsing
    int32u Entries;
    #if MEDIAINFO_SEEK
        Get_L4 (Flt_FieldPerEntry,                              "Number of fields per FLT entry");
    #else //MEDIAINFO_SEEK
        Skip_L4(                                                "Number of fields per FLT entry");
    #endif //MEDIAINFO_SEEK
    Get_L4 (Entries,                                            "Number of FLT entries");
    for (size_t Pos=0; Pos<Entries; Pos++)
    {
        #if MEDIAINFO_SEEK
            int32u Offset;
            Get_L4 (Offset,                                     "Offset to fields");
            Flt_Offsets.push_back(Offset);
        #else //MEDIAINFO_SEEK
            Skip_L4(                                            "Offset to fields");
        #endif //MEDIAINFO_SEEK
        if (Element_Offset==Element_Size)
            break;
    }
}

//---------------------------------------------------------------------------
void File_Gxf::UMF_file()
{
    Element_Name("UMF file");

    //Parsing
    int32u PayloadDataLength;
    Element_Begin1("Preamble");
        Skip_B1(                                                "First/last packet flag");
        Get_B4 (PayloadDataLength,                              "Payload data length");
    Element_End0();

    if (UMF_File==NULL)
    {
        UMF_File=new File_Umf();
        Open_Buffer_Init(UMF_File);
    }
    Open_Buffer_Continue(UMF_File, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));

    #if MEDIAINFO_SEEK
    if (Seeks.empty() && Flt_FieldPerEntry!=(int32u)-1 && ((File_Umf*)UMF_File)->GopSize!=(int64u)-1)
        {
            size_t NextIFrame=0;
            for (size_t Pos=0; Pos<Flt_Offsets.size(); Pos++)
                if (Pos*Flt_FieldPerEntry>=NextIFrame)
                {
                    seek Seek;
                    Seek.FrameNumber=(Material_Fields_First_IsValid?Material_Fields_First:0)+Pos*Flt_FieldPerEntry;
                    Seek.StreamOffset=Flt_Offsets[Pos];
                    Seeks.push_back(Seek);
                    NextIFrame+=(size_t)((File_Umf*)UMF_File)->GopSize*Material_Fields_FieldsPerFrame;
                }
            Flt_Offsets.clear();
        }
    #endif //MEDIAINFO_SEEK
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gxf::Detect_EOF()
{
    if (File_Offset+Buffer_Size>=SizeToAnalyze)
    {
        TryToFinish();
    }
}

//---------------------------------------------------------------------------
File__Analyze* File_Gxf::ChooseParser_ChannelGrouping(int8u TrackID)
{
    File_ChannelGrouping* Parser;
    if (Audio_Count%2)
    {
        if (!Streams[TrackID-1].IsChannelGrouping)
            return NULL; //Not a channel grouping

        Parser=new File_ChannelGrouping;
        Parser->CanBePcm=true;
        Parser->Channel_Pos=1;
        Parser->Common=((File_ChannelGrouping*)Streams[TrackID-1].Parsers[0])->Common;
        Parser->StreamID=TrackID-1;
        Streams[TrackID].IsChannelGrouping=true;
    }
    else
    {
        Parser=new File_ChannelGrouping;
        Parser->CanBePcm=true;
        Parser->Channel_Pos=0;
        //if (Descriptor->second.Infos.find("SamplingRate")!=Descriptor->second.Infos.end())
        Streams[TrackID].IsChannelGrouping=true;
        Streams[TrackID].DisplayInfo=false;
    }
    Parser->BitDepth=24;
    Parser->Channel_Total=2;
    Parser->SamplingRate=48000; //TODO: find where this piece of info is avaialble
    Parser->Endianness='L';

    #if MEDIAINFO_DEMUX
        if (Demux_UnpacketizeContainer)
        {
            Parser->Demux_Level=2; //Container
            Parser->Demux_UnpacketizeContainer=true;
        }
    #endif //MEDIAINFO_DEMUX

    return Parser;
}

//---------------------------------------------------------------------------
void File_Gxf::TryToFinish()
{
    if (!IsSub && File_Size!=(int64u)-1 && Config->ParseSpeed<1 && IsParsingMiddle_MaxOffset==(int64u)-1 && File_Size/2>SizeToAnalyze*4)
    {
        IsParsingMiddle_MaxOffset=File_Size/2+SizeToAnalyze*4;
        GoTo(File_Size/2);
        Open_Buffer_Unsynch();
        Parsers_Count=(int8u)-1;
        return;
    }

    Finish();
}

} //NameSpace

#endif //MEDIAINFO_GXF_YES
