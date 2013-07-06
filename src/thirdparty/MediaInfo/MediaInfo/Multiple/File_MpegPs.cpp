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
#if defined(MEDIAINFO_MPEGPS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_MpegPs.h"
#include "MediaInfo/Multiple/File_Mpeg_Psi.h"
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_HEVC_YES)
    #include "MediaInfo/Video/File_Hevc.h"
#endif
#if defined(MEDIAINFO_MPEG4V_YES)
    #include "MediaInfo/Video/File_Mpeg4v.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_VC1_YES)
    #include "MediaInfo/Video/File_Vc1.h"
#endif
#if defined(MEDIAINFO_AVSV_YES)
    #include "MediaInfo/Video/File_AvsV.h"
#endif
#if defined(MEDIAINFO_DIRAC_YES)
    #include "MediaInfo/Video/File_Dirac.h"
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
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm_M2ts.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm_Vob.h"
#endif
#if defined(MEDIAINFO_SMPTEST0302_YES)
    #include "MediaInfo/Audio/File_SmpteSt0302.h"
#endif
#if defined(MEDIAINFO_PS2A_YES)
    #include "MediaInfo/Audio/File_Ps2Audio.h"
#endif
#if defined(MEDIAINFO_RLE_YES)
    #include "MediaInfo/Image/File_Rle.h"
#endif
#if defined(MEDIAINFO_ARIBSTDB24B37_YES)
    #include "MediaInfo/Text/File_AribStdB24B37.h"
#endif
#if defined(MEDIAINFO_DVBSUBTITLE_YES)
    #include "MediaInfo/Text/File_DvbSubtitle.h"
#endif
#if defined(MEDIAINFO_PGS_YES)
    #include "MediaInfo/Text/File_Pgs.h"
#endif
#if defined(MEDIAINFO_TELETEXT_YES)
    #include "MediaInfo/Text/File_Teletext.h"
#endif
#include "MediaInfo/File_Unknown.h"
#include <ZenLib/Utils.h>
#include <algorithm>
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
#if MEDIAINFO_IBI
    #if MEDIAINFO_SEEK
        #include "MediaInfo/Multiple/File_Ibi.h"
    #endif //MEDIAINFO_SEEK
#endif //MEDIAINFO_IBI
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
const char* MpegPs_System_Fixed[]=
{
    "CBR",
    "VBR",
};

//---------------------------------------------------------------------------
const char* MpegPs_stream_id(int8u Element_Name)
{
         if (Element_Name>=0xC0
          && Element_Name<=0xDF) return "MPEG Audio";
    else if (Element_Name>=0xE0
          && Element_Name<=0xEF) return "MPEG Video";
    else if (Element_Name==0xB8) return "For all MPEG Audio streams";
    else if (Element_Name==0xB9) return "For all MPEG Video streams";
    else if (Element_Name==0xBD) return "Private 1";
    else if (Element_Name==0xBF) return "Private 2";
    else if (Element_Name==0xFD) return "Private HD";
    else                         return "";
}

//---------------------------------------------------------------------------
const char* MpegPs_Codec(int8u Element_Name)
{
         if (Element_Name>=0xC0
          && Element_Name<=0xDF) return "MPEG-A";
    else if (Element_Name>=0xE0
          && Element_Name<=0xEF) return "MPEG-V";
    else                         return "";
}

//---------------------------------------------------------------------------
int32u MpegPs_Default_stream_type(int8u Element_Name, int8u Mpeg_Version)
{
         if (Element_Name>=0xC0
          && Element_Name<=0xDF) return Mpeg_Version==0x02?0x04:0x03;
    else if (Element_Name>=0xE0
          && Element_Name<=0xEF) return Mpeg_Version==0x02?0x02:0x01;
    else                         return 0x00;
}

//---------------------------------------------------------------------------
const char* MpegPs_trick_mode_control_values[8]=
{
    "Fast forward",
    "Slow motion",
    "Freeze frame",
    "Fast reverse",
    "Slow reverse",
    "Reserved",
    "Reserved",
    "Reserved"
};

//---------------------------------------------------------------------------
const char* MpegPs_stream_id_extension(int8u stream_id_extension)
{
    switch (stream_id_extension)
    {
        case 0x00 : return "IPMP Control Information Streams"; //ISO/IEC 13818-11
        case 0x01 : return "IPMP Streams";                     //ISO/IEC 13818-11
        default :
                 if (stream_id_extension>=0x02
                  && stream_id_extension<=0x11) return "ISO/IEC 14496-17 text Streams";
            else if (stream_id_extension>=0x12
                  && stream_id_extension<=0x21) return "ISO/IEC 23002-3 auxiliary video data Streams";
            else if (stream_id_extension>=0x55
                  && stream_id_extension<=0x5F) return "VC-1";
            else if (stream_id_extension>=0x60
                  && stream_id_extension<=0x6F) return "Dirac";
            else if (stream_id_extension==0x71) return "Audio";
            else if (stream_id_extension==0x72) return "Audio Ext";
            else if (stream_id_extension==0x76) return "Audio";
            else if (stream_id_extension>=0x75
                  && stream_id_extension<=0x7F) return "VC-1";
            else                                return "";
    }
}

//---------------------------------------------------------------------------
extern const char* Mpeg_Psi_stream_type_Format(int8u stream_type, int32u format_identifier);
extern const char* Mpeg_Psi_stream_type_Codec(int8u stream_type, int32u format_identifier);
extern stream_t    Mpeg_Psi_stream_type_StreamKind(int32u stream_type, int32u format_identifier);
extern const char* Mpeg_Psi_stream_type_Info(int8u stream_type, int32u format_identifier);

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_MpegPs::File_MpegPs()
:File__Analyze()
{
    //Configuration
    ParserName=__T("MpegPs");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_MpegPs;
        StreamIDs_Width[0]=2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(0); //Container1
    #endif //MEDIAINFO_TRACE
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;
    Buffer_TotalBytes_Fill_Max=(int64u)-1; //Disabling this feature for this format, this is done in the parser
    Trusted_Multiplier=2;

    //In
    FromTS=false;
    FromTS_stream_type=0x00; //No info
    FromTS_program_format_identifier=0x00000000; //No info
    FromTS_format_identifier=0x00000000; //No info
    FromTS_descriptor_tag=0x00; //No info
    MPEG_Version=0; //No info
    Searching_TimeStamp_Start=true;
    #ifdef MEDIAINFO_MPEG4_YES
        ParserFromTs=NULL;
        SLConfig=NULL;
    #endif
    #if MEDIAINFO_DEMUX
        SubStream_Demux=NULL;
        Demux_StreamIsBeingParsed_type=(int8u)-1;
    #endif //MEDIAINFO_DEMUX

    //Out
    HasTimeStamps=false;

    //Temp
    SizeToAnalyze=8*1024*1024;
    video_stream_Unlimited=false;
    Buffer_DataSizeToParse=0;
    #if MEDIAINFO_SEEK
        Seek_Value=(int64u)-1;
        Seek_ID=(int64u)-1;
        Duration_Detected=false;
    #endif //MEDIAINFO_SEEK

    //From packets
    program_mux_rate=(int32u)-1;

    BookMark_Set(); //for stream parsing in phase 2
}

//---------------------------------------------------------------------------
File_MpegPs::~File_MpegPs()
{
    #if MEDIAINFO_DEMUX
        if (FromTS_stream_type==0x20) //If SubStream, this object owns the demux handler
            delete SubStream_Demux; //SubStream_Demux=NULL;
    #endif //MEDIAINFO_DEMUX
    #ifdef MEDIAINFO_MPEG4_YES
        delete ParserFromTs; //ParserFromTs=NULL;
        delete SLConfig; //SLConfig=NULL;
    #endif
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegPs::Streams_Fill()
{
    //For each Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Fill_PerStream(StreamID, Streams[StreamID], KindOfStream_Main);

    //For each private Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Fill_PerStream(StreamID, Streams_Private1[StreamID], KindOfStream_Private);

    //For each extension Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
    {
        Streams_Fill_PerStream(StreamID, Streams_Extension[StreamID], KindOfStream_Extension);

        //Special cases
        if ((StreamID==0x71 || StreamID==0x76) && !Streams_Extension[StreamID].Parsers.empty() && Streams_Extension[0x72].StreamIsRegistred) //DTS-HD and TrueHD
        {
            Fill(Stream_Audio, StreamPos_Last, Audio_MuxingMode, "Stream extension");
            if (!IsSub)
                Fill(Stream_Audio, StreamPos_Last, Audio_MuxingMode_MoreInfo, "HD part is in stream extension 114 (0x72)");
        }
    }

    //Tags in MPEG Video
    if (Count_Get(Stream_Video)>0)
        Fill(Stream_General, 0, General_Encoded_Library, Retrieve(Stream_Video, 0, Video_Encoded_Library));

    //Special case: Video PTS
    if (video_stream_PTS.size()>=2+4*2+1*2 && Retrieve(Stream_Video, 0, Video_FrameRate).To_float64()>30.000) //TODO: Parser all kind of files
    {
        sort(video_stream_PTS.begin(), video_stream_PTS.end());
        video_stream_PTS.erase(video_stream_PTS.begin(), video_stream_PTS.begin()+4); //Removing first frames, they may lack of B/P frames
        video_stream_PTS.resize(video_stream_PTS.size()-4); //Removing last frames, they may lack of B/P frames

        //Trying to detect container FPS
        std::vector<int64u> video_stream_PTS_Between;
        for (size_t Pos=1; Pos<video_stream_PTS.size(); Pos++)
            video_stream_PTS_Between.push_back(video_stream_PTS[Pos]-video_stream_PTS[Pos-1]);
        std::sort(video_stream_PTS_Between.begin(), video_stream_PTS_Between.end());
        video_stream_PTS_Between.erase(video_stream_PTS_Between.begin(), video_stream_PTS_Between.begin()+1); //Removing first timec, they may be wrong value due to missing frame
        video_stream_PTS_Between.resize(video_stream_PTS_Between.size()-1); //Removing last frames, they may be wrong value due to missing frame
        if (video_stream_PTS_Between[0]*0.9<video_stream_PTS_Between[video_stream_PTS_Between.size()-1]
         && video_stream_PTS_Between[0]*1.1>video_stream_PTS_Between[video_stream_PTS_Between.size()-1])
        {
            float64 Time=(float)(video_stream_PTS[video_stream_PTS.size()-1]-video_stream_PTS[0])/(video_stream_PTS.size()-1)/90;
            if (Time)
            {
                float64 FrameRate_Container=1000/Time;
                if (Retrieve(Stream_Video, 0, Video_ScanType)==__T("Interlaced"))
                    FrameRate_Container/=2; //PTS is per field
                float64 FrameRate_Original=Retrieve(Stream_Video, 0, Video_FrameRate).To_float64();
                if (!(FrameRate_Original>=FrameRate_Container*0.9 && FrameRate_Original<=FrameRate_Container*1.1)
                 && !(FrameRate_Container>=FrameRate_Original*0.9 && FrameRate_Container<=FrameRate_Original*1.1))
                {
                    Clear(Stream_Video, 0, Video_FrameRate); //Or automatic filling thinks current FrameRate is the container FrameRate (usaly Conatainer FrameRate is filled first, not here)
                    Fill(Stream_Video, 0, Video_FrameRate, FrameRate_Container, 3, true);
                    if (FrameRate_Original)
                        Fill(Stream_Video, 0, Video_FrameRate_Original, FrameRate_Original);
                }
            }
        }
    }

    if (Count_Get(Stream_Video)==1 && Retrieve(Stream_Video, 0, Video_Format_Version)==__T("Version 1"))
        Fill(Stream_General, 0, General_InternetMediaType, "video/mpeg", Unlimited, true, true);
}

//---------------------------------------------------------------------------
void File_MpegPs::Streams_Fill_PerStream(size_t StreamID, ps_stream &Temp, kindofstream KindOfStream)
{
    //By the parser
    StreamKind_Last=Stream_Max;
    size_t Count=0;
    if (!Temp.Parsers.empty() && Temp.Parsers[0] && Temp.Parsers[0]->Status[IsAccepted])
    {
        Fill(Temp.Parsers[0]);

        if (Temp.Parsers[0]->Count_Get(Stream_Video) && Temp.Parsers[0]->Count_Get(Stream_Text))
        {
            //Special case: Video and Text are together
            Stream_Prepare(Stream_Video);
            Count=Merge(*Temp.Parsers[0], Stream_Video, 0, StreamPos_Last);
        }
        else
            Count=Merge(*Temp.Parsers[0]);
    }

    //By the TS stream_type
    if (StreamKind_Last==Stream_Max)
    {
        //Disabling stream_private_1 if needed (will be done by Streams_Private1 object)
        if (Temp.stream_type!=0 && (StreamID==0xBD /*|| StreamID==0xBF*/))
        {
            bool StreamIsDetected=false;
            for (size_t Pos=0; Pos<Streams_Private1.size(); Pos++)
                if (!Streams_Private1[Pos].Parsers.empty() && Streams_Private1[Pos].Parsers[0])
                    StreamIsDetected=true;
            if (StreamIsDetected)
                Temp.stream_type=0;
        }

        if (Temp.stream_type!=0)
        {
            Stream_Prepare(Mpeg_Psi_stream_type_StreamKind(Temp.stream_type, 0x00000000));
            Count=1;
        }
    }

    //By StreamIsRegistred
    if (StreamKind_Last==Stream_Max)
    {
        if (Temp.StreamIsRegistred>16)
        {
            if (StreamID>=0xC0 && StreamID<=0xDF)
            {
                Stream_Prepare(Stream_Audio);
                Count=1;
            }
            if (StreamID>=0xE0 && StreamID<=0xEF)
            {
                Stream_Prepare(Stream_Video);
                Count=1;
            }
        }
    }

    #ifdef MEDIAINFO_MPEG4_YES
        if (StreamKind_Last==Stream_Audio && SLConfig)
            Fill(Stream_Audio, StreamPos_Last, Audio_MuxingMode, "SL");
    #endif //MEDIAINFO_MPEG4_YES

    //More info
    for (size_t StreamPos=Count_Get(StreamKind_Last)-Count; StreamPos<Count_Get(StreamKind_Last); StreamPos++)
    {
        ///Saving StreamKind and Stream_Pos
        Temp.StreamKind=StreamKind_Last;
        Temp.StreamPos=Count_Get(StreamKind_Last)-Count;

        //Common
        if (KindOfStream==KindOfStream_Main)
        {
            Ztring ID; ID.From_Number(StreamID);
            Ztring ID_String; ID_String.From_Number(StreamID); ID_String+=__T(" (0x"); ID_String+=Ztring::ToZtring(StreamID, 16); ID_String+=__T(")");
            if (!Retrieve(StreamKind_Last, StreamPos, General_ID).empty())
            {
                Fill(StreamKind_Last, StreamPos, General_ID, StreamID);
                Ztring ID_String; ID_String.From_Number(StreamID); ID_String+=__T(" (0x"); ID_String+=Ztring::ToZtring(StreamID, 16); ID_String+=__T(")");
                Fill(StreamKind_Last, StreamPos, General_ID_String, ID_String, true); //TODO: merge with Decimal_Hexa in file_MpegTs
            }
            Fill(StreamKind_Last, StreamPos, General_ID, ID, true);
            Fill(StreamKind_Last, StreamPos, General_ID_String, ID_String, true); //TODO: merge with Decimal_Hexa in file_MpegTs
        }
        else if (KindOfStream==KindOfStream_Private)
        {
            Ztring ID=__T("189");
            if (StreamID)
                ID+=__T("-")+Ztring::ToZtring(StreamID);
            if (!Temp.Parsers[0]->Retrieve(StreamKind_Last, StreamPos, General_ID).empty())
                ID+=__T("-")+Temp.Parsers[0]->Retrieve(StreamKind_Last, StreamPos, General_ID);
            Fill(StreamKind_Last, StreamPos, General_ID, ID, true);
            Ztring ID_String=__T("189 (0xBD)");
            if (StreamID)
                ID_String+=__T("-")+Ztring::ToZtring(StreamID)+__T(" (0x")+Ztring::ToZtring(StreamID, 16)+__T(")");
            if (!Temp.Parsers[0]->Retrieve(StreamKind_Last, StreamPos, General_ID_String).empty())
                ID_String+=__T("-")+Temp.Parsers[0]->Retrieve(StreamKind_Last, StreamPos, General_ID_String);
            else if (!Temp.Parsers[0]->Retrieve(StreamKind_Last, StreamPos, General_ID).empty())
                ID_String+=__T("-")+Temp.Parsers[0]->Retrieve(StreamKind_Last, StreamPos, General_ID);
            Fill(StreamKind_Last, StreamPos, General_ID_String, ID_String, true); //TODO: merge with Decimal_Hexa in file_MpegTs
            if (StreamID)
                Fill(StreamKind_Last, StreamPos, "MuxingMode", "DVD-Video", Unlimited, true, true);
        }
        else if (KindOfStream==KindOfStream_Extension)
        {
            Ztring ID=__T("253");
            if (StreamID)
                ID+=__T("-")+Ztring::ToZtring(StreamID);
            Fill(StreamKind_Last, StreamPos, General_ID, ID, true);
            Ztring ID_String=__T("253 (0xFD)");
            if (StreamID)
                ID_String+=__T("-")+Ztring::ToZtring(StreamID)+__T(" (0x")+Ztring::ToZtring(StreamID, 16)+__T(")");
            Fill(StreamKind_Last, StreamPos, General_ID_String, ID_String, true); //TODO: merge with Decimal_Hexa in file_MpegTs
        }

        if (Retrieve(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Format)).empty() && Temp.stream_type!=0)
            Fill(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Format), Mpeg_Psi_stream_type_Format(Temp.stream_type, 0x0000));
        if (Retrieve(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Codec)).empty() && Temp.stream_type!=0)
            Fill(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Codec), Mpeg_Psi_stream_type_Codec(Temp.stream_type, 0x0000));

        if (Temp.TimeStamp_Start.PTS.TimeStamp!=(int64u)-1)
        {
            Fill(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay_Original), Retrieve(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay)), true);
            Clear(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay));
            Fill(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay_Original_Source), Retrieve(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay_Source)), true);
            Clear(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay_Source));
            Fill(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay_Original_Settings), Retrieve(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay_Settings)), true);
            Clear(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay_Settings));

            Fill(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay), ((float64)Temp.TimeStamp_Start.PTS.TimeStamp)/90, 3, true);
            Fill(StreamKind_Last, StreamPos, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
        }

        //Bitrate calculation
        if (FrameInfo.PTS!=(int64u)-1 && (StreamKind_Last==Stream_Video || StreamKind_Last==Stream_Audio))
        {
            int64u BitRate=Retrieve(StreamKind_Last, StreamPos, "BitRate").To_int64u();
            if (BitRate==0)
                BitRate=Retrieve(StreamKind_Last, StreamPos, "BitRate_Nominal").To_int64u();
            if (BitRate==0)
                FrameInfo.PTS=(int64u)-1;
            else
                FrameInfo.PTS+=BitRate; //Saving global BitRate
        }
    }
}

//---------------------------------------------------------------------------
void File_MpegPs::Streams_Finish()
{
    if (Streams.empty())
        return; //Parsing already done. ToDo: real time

    FrameInfo.PTS=0; //Will be used for BitRate calculation
    FrameInfo.DTS=0; //Will be used for Duration calculation

    //For each Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Finish_PerStream(StreamID, Streams[StreamID], KindOfStream_Main);

    //For each private Streams
    StreamOrder_CountOfPrivateStreams_Temp=0;
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Finish_PerStream(StreamID, Streams_Private1[StreamID], KindOfStream_Private);

    //For each extesnion Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Finish_PerStream(StreamID, Streams_Extension[StreamID], KindOfStream_Extension);

    //Bitrate coherancy
    if (!IsSub && FrameInfo.PTS>0 && FrameInfo.PTS!=(int64u)-1 && FrameInfo.DTS!=0 && File_Size!=(int64u)-1)
    {
        int64u BitRate_FromDuration=File_Size*8000*90/FrameInfo.DTS;
        int64u BitRate_FromBitRates=FrameInfo.PTS;

        if (BitRate_FromDuration>=BitRate_FromBitRates*3
         || BitRate_FromDuration<=BitRate_FromBitRates/20)
        {
            //Clearing durations
            for (size_t StreamKind=0; StreamKind<=Stream_Text; StreamKind++)
                for (size_t StreamPos=0; StreamPos<Count_Get((stream_t)StreamKind); StreamPos++)
                    Clear((stream_t)StreamKind, StreamPos, (stream_t)Fill_Parameter((stream_t)StreamKind, Generic_Duration));
            if (Count_Get(Stream_Video)==1)
                Clear(Stream_Video, 0, Video_Duration);
        }
    }

    #if MEDIAINFO_IBI
        if (!IsSub && Config_Ibi_Create)
        {
            for (ibi::streams::iterator IbiStream_Temp=Ibi.Streams.begin(); IbiStream_Temp!=Ibi.Streams.end(); ++IbiStream_Temp)
            {
                if (IbiStream_Temp->second && IbiStream_Temp->second->DtsFrequencyNumerator==1000000000 && IbiStream_Temp->second->DtsFrequencyDenominator==1)
                {
                    bool IsOk=true;
                    for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                        if (!IbiStream_Temp->second->Infos[Pos].IsContinuous && Pos+1!=IbiStream_Temp->second->Infos.size())
                            IsOk=false;
                    if (IsOk) //Only is all items are continuous (partial IBI not yet supported)
                    {
                        IbiStream_Temp->second->DtsFrequencyNumerator=90000;
                        for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                        {
                            int64u Temp=IbiStream_Temp->second->Infos[Pos].Dts*90/1000000;
                            IbiStream_Temp->second->Infos[Pos].Dts=Temp;
                        }
                    }
                }
            }
        }
    #endif //MEDIAINFO_IBI
}

//---------------------------------------------------------------------------
void File_MpegPs::Streams_Finish_PerStream(size_t StreamID, ps_stream &Temp, kindofstream KindOfStream)
{
    //By the parser
    if (Temp.StreamKind==Stream_Max && !Temp.Parsers.empty() && Temp.Parsers[0])
        Streams_Fill_PerStream(StreamID, Temp, KindOfStream);

    //Init
    if (Temp.StreamKind==Stream_Max)
        return;
    StreamKind_Last=Temp.StreamKind;
    StreamPos_Last=Temp.StreamPos;

    //By the parser
    if (!Temp.Parsers.empty() && Temp.Parsers[0])
    {
        if (!Temp.Parsers[0]->Status[IsFinished])
        {
            Temp.Parsers[0]->ShouldContinueParsing=false;
            int64u File_Size_Temp=File_Size;
            File_Size=File_Offset+Buffer_Offset+Element_Offset;
            #if MEDIAINFO_EVENTS
                Temp.Parsers[0]->PES_FirstByte_IsAvailable=false;
            #endif //MEDIAINFO_EVENTS
            Open_Buffer_Continue(Temp.Parsers[0], Buffer, 0, false);
            File_Size=File_Size_Temp;
            Finish(Temp.Parsers[0]);
            #if MEDIAINFO_DEMUX
                if (Config->Demux_EventWasSent)
                    return;
            #endif //MEDIAINFO_DEMUX
        }
        Ztring ID=Retrieve(StreamKind_Last, StreamPos_Last, General_ID);
        Ztring ID_String=Retrieve(StreamKind_Last, StreamPos_Last, General_ID_String);
        Merge(*Temp.Parsers[0], StreamKind_Last, 0, StreamPos_Last);
        Fill(StreamKind_Last, StreamPos_Last, General_ID, ID, true);
        Fill(StreamKind_Last, StreamPos_Last, General_ID_String, ID_String, true);
        if (!IsSub)
        {
            switch (KindOfStream)
            {
                case KindOfStream_Private   :
                                                if (Streams[0xBD].StreamOrder!=(size_t)-1)
                                                    Fill(StreamKind_Last, StreamPos_Last, General_StreamOrder, Streams[0xBD].StreamOrder+StreamOrder_CountOfPrivateStreams_Temp);
                                                if (StreamOrder_CountOfPrivateStreams_Minus1 && StreamOrder_CountOfPrivateStreams_Temp<StreamOrder_CountOfPrivateStreams_Minus1)
                                                    StreamOrder_CountOfPrivateStreams_Temp++;
                                                break;
                case KindOfStream_Extension :
                                                if (Streams[0xFD].StreamOrder!=(size_t)-1)
                                                    Fill(StreamKind_Last, StreamPos_Last, General_StreamOrder, Streams[0xFD].StreamOrder);
                                                break;
                default                     :
                                                if (Temp.StreamOrder!=(size_t)-1)
                                                    Fill(StreamKind_Last, StreamPos_Last, General_StreamOrder, Temp.StreamOrder);
            }
            Fill(StreamKind_Last, StreamPos_Last, General_FirstPacketOrder, Temp.FirstPacketOrder);
        }

        //Special cases
        if (Temp.Parsers[0]->Count_Get(Stream_Video) && Temp.Parsers[0]->Count_Get(Stream_Text))
        {
            //Video and Text are together
            size_t Text_Count=Temp.Parsers[0]->Count_Get(Stream_Text);
            for (size_t Parser_Pos=0; Parser_Pos<Text_Count; Parser_Pos++)
            {
                Ztring ID=Retrieve(Stream_Video, Temp.StreamPos, Video_ID)+__T("-")+Temp.Parsers[0]->Retrieve(Stream_Text, Parser_Pos, Text_ID);
                StreamPos_Last=(size_t)-1;
                for (size_t Pos=0; Pos<Count_Get(Stream_Text); Pos++)
                    if (Retrieve(Stream_Text, Pos, Text_ID)==ID && Retrieve(Stream_Video, Temp.StreamPos, "MuxingMode")==Temp.Parsers[0]->Retrieve(Stream_Text, Parser_Pos, "MuxingMode"))
                    {
                        StreamPos_Last=Pos;
                        break;
                    }
                if (StreamPos_Last==(size_t)-1)
                    Stream_Prepare(Stream_Text, StreamPos_Last);
                Merge(*Temp.Parsers[0], Stream_Text, Parser_Pos, StreamPos_Last);

                if (!IsSub)
                    Fill(Stream_Text, StreamPos_Last, "MuxingMode_MoreInfo", __T("Muxed in Video #")+Ztring().From_Number(Temp.StreamPos+1));
                Fill(Stream_Text, StreamPos_Last, Text_ID, ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_ID_String, Retrieve(Stream_Video, Temp.StreamPos, Video_ID_String)+__T("-")+Temp.Parsers[0]->Retrieve(Stream_Text, Parser_Pos, Text_ID), true);
                Fill(Stream_Text, StreamPos_Last, Text_Delay, Retrieve(Stream_Video, Temp.StreamPos, Video_Delay), true);
                if (!IsSub)
                {
                    switch (KindOfStream)
                    {
                        case KindOfStream_Private   :
                                                        if (Streams[0xBD].StreamOrder!=(size_t)-1)
                                                            Fill(Stream_Text, StreamPos_Last, General_StreamOrder, Streams[0xBD].StreamOrder);
                                                        break;
                        case KindOfStream_Extension :
                                                        if (Streams[0xFD].StreamOrder!=(size_t)-1)
                                                            Fill(Stream_Text, StreamPos_Last, General_StreamOrder, Streams[0xFD].StreamOrder);
                                                        break;
                        default                     :
                                                        if (Temp.StreamOrder!=(size_t)-1)
                                                            Fill(Stream_Text, StreamPos_Last, General_StreamOrder, Temp.StreamOrder);
                    }
                    Fill(StreamKind_Last, StreamPos_Last, General_FirstPacketOrder, Temp.FirstPacketOrder);
                }
            }

            StreamKind_Last=Temp.StreamKind;
            StreamPos_Last=Temp.StreamPos;
        }
    }

    //Duration if it is missing from the parser
    if (Temp.StreamKind!=Stream_Max && Retrieve(Temp.StreamKind, Temp.StreamPos, Fill_Parameter(Temp.StreamKind, Generic_Duration)).empty())
    {
         StreamKind_Last=Temp.StreamKind;
         StreamPos_Last=Temp.StreamPos;

        int64u Start=(int64u)-1, End=(int64u)-1, ByteDifference=(int64u)-1;
        if (Temp.TimeStamp_Start.DTS.TimeStamp!=(int64u)-1 && Temp.TimeStamp_End.DTS.TimeStamp!=(int64u)-1)
        {
            Start=Temp.TimeStamp_Start.DTS.TimeStamp;
            End=Temp.TimeStamp_End.DTS.TimeStamp;
        }
        else if (Temp.TimeStamp_Start.PTS.TimeStamp!=(int64u)-1 && Temp.TimeStamp_End.PTS.TimeStamp!=(int64u)-1)
        {
            Start=Temp.TimeStamp_Start.PTS.TimeStamp;
            End=Temp.TimeStamp_End.PTS.TimeStamp;
            if (Temp.TimeStamp_Start.PTS.File_Pos<Temp.TimeStamp_End.PTS.File_Pos)
                ByteDifference=Temp.TimeStamp_End.PTS.File_Pos-Temp.TimeStamp_Start.PTS.File_Pos;
        }
        if (Start!=(int64u)-1 && End!=(int64u)-1)
        {
            //TimeStamp
            if (End<0x100000000LL && Start>0x100000000LL) //Testing coherancy: no 13 hours long files.
                End+=0x200000000LL; //33 bits, cyclic
            if (Start<End)
            {
                int64u Duration=End-Start;
                if (ByteDifference!=(int64u)-1)
                {
                    float BitRate=(ByteDifference*8)/(((float)Duration)/9000);
                    if (BitRate>10000000000LL)
                        Duration=0;
                }
                if (Duration)
                {
                    if (StreamKind_Last==Stream_Video)
                    {
                        float64 FrameRate=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate).To_float64();
                        if (FrameRate!=0)
                            Duration+=Ztring::ToZtring(90*1000/FrameRate, 0).To_int64u(); //We imagine that there is one frame in it
                    }

                    Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Duration), Duration/90, 10, true);
                }
            }
        }
    }

    //Bitrate calculation
    if (FrameInfo.PTS!=(int64u)-1 && (StreamKind_Last==Stream_Video || StreamKind_Last==Stream_Audio))
    {
        int64u BitRate=Retrieve(StreamKind_Last, StreamPos_Last, "BitRate").To_int64u();
        if (BitRate==0)
            BitRate=Retrieve(StreamKind_Last, StreamPos_Last, "BitRate_Nominal").To_int64u();
        if (BitRate==0)
            FrameInfo.PTS=(int64u)-1;
        else
            FrameInfo.PTS+=BitRate; //Saving global BitRate
    }
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_MpegPs::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+4<=Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                         || Buffer[Buffer_Offset+1]!=0x00
                                         || Buffer[Buffer_Offset+2]!=0x01
                                         || Buffer[Buffer_Offset+3]< 0xB9))
    {
        Buffer_Offset+=2;
        while(Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x00)
            Buffer_Offset+=2;
        if (Buffer_Offset>=Buffer_Size || Buffer[Buffer_Offset-1]==0x00)
            Buffer_Offset--;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+4==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00
                                      || Buffer[Buffer_Offset+2]!=0x01
                                      || Buffer[Buffer_Offset+3]< 0xB9))
        Buffer_Offset++;
    if (Buffer_Offset+3==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00
                                      || Buffer[Buffer_Offset+2]!=0x01))
        Buffer_Offset++;
    if (Buffer_Offset+2==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00))
        Buffer_Offset++;
    if (Buffer_Offset+1==Buffer_Size &&  Buffer[Buffer_Offset  ]!=0x00)
        Buffer_Offset++;

    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_MpegPs::Synched_Test()
{
    //Trailing 0xFF
    while(Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]==0xFF)
        Buffer_Offset++;

    //Trailing 0x00
    while(Buffer_Offset+3<=Buffer_Size
       && Buffer[Buffer_Offset+2]==0x00
       && Buffer[Buffer_Offset+1]==0x00
       && Buffer[Buffer_Offset  ]==0x00)
        Buffer_Offset++;

    //Must have enough buffer for having header
    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Quick test of synchro
    if (Buffer[Buffer_Offset  ]!=0x00
     || Buffer[Buffer_Offset+1]!=0x00
     || Buffer[Buffer_Offset+2]!=0x01)
        Synched=false;

    //Quick search
    if (Synched && !Header_Parser_QuickSearch())
        return false;

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_MpegPs::Synched_Init()
{
    //private_stream_1 specific
    private_stream_1_ID=0x00;
    private_stream_1_Offset=0;
    private_stream_1_IsDvdVideo=false;

    //Count
    video_stream_Count=(int8u)-1;
    audio_stream_Count=(int8u)-1;
    private_stream_1_Count=(int8u)-1;
    private_stream_2_Count=(int8u)-1;
    extension_stream_Count=(int8u)-1;
    SL_packetized_stream_Count=(int8u)-1;

    //From packets
    program_mux_rate=0;

    //Default values
    Streams.resize(0x100);
    Streams_Private1.resize(0x100);
    Streams_Extension.resize(0x100);
    Streams[0xBA].Searching_Payload=true;

    //Temp
    stream_id_extension=0x55; //Default is set to VC-1, should never happens, but happens sometimes
    FirstPacketOrder_Last=0;

    //Case of extraction from MPEG-TS files
    if (File_Offset==0 && Buffer_Size>=4 && ((CC4(Buffer)&0xFFFFFFF0)==0x000001E0 || (CC4(Buffer)&0xFFFFFFE0)==0x000001C0 || CC4(Buffer)==0x000001BD || CC4(Buffer)==0x000001FA || CC4(Buffer)==0x000001FD || CC4(Buffer)==0x000001FE))
    {
        FromTS=true; //We want to anlyze this kind of file
        MPEG_Version=2; //By default, MPEG-TS is version 2
        Streams[Buffer[3]].Searching_Payload=true; //Activating the Streams
    }

    //TS specific
    if (FromTS)
    {
        Streams[0xBD].Searching_Payload=true;            //private_stream_1
        Streams[0xBD].Searching_TimeStamp_Start=true;    //private_stream_1
        Streams[0xBD].Searching_TimeStamp_End=true;      //private_stream_1
        Streams[0xBF].Searching_Payload=true;            //private_stream_2
        Streams[0xBF].Searching_TimeStamp_Start=true;    //private_stream_2
        Streams[0xBF].Searching_TimeStamp_End=true;      //private_stream_2
        for (int8u Pos=0xC0; Pos<=0xEF; Pos++)
        {
            Streams[Pos].Searching_Payload=true;         //audio_stream or video_stream
            Streams[Pos].Searching_TimeStamp_Start=true; //audio_stream or video_stream
            Streams[Pos].Searching_TimeStamp_End=true;   //audio_stream or video_stream
        }
        Streams[0xFA].Searching_Payload=true;            //LATM
        Streams[0xFA].Searching_TimeStamp_Start=true;    //LATM
        Streams[0xFA].Searching_TimeStamp_End=true;      //LATM
        Streams[0xFD].Searching_Payload=true;            //extension_stream
        Streams[0xFD].Searching_TimeStamp_Start=true;    //extension_stream
        Streams[0xFD].Searching_TimeStamp_End=true;      //extension_stream
        Streams[0xFE].Searching_Payload=true;            //extension_stream?
        Streams[0xFE].Searching_TimeStamp_Start=true;    //extension_stream?
        Streams[0xFE].Searching_TimeStamp_End=true;      //extension_stream?
    }
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegPs::Read_Buffer_Init()
{
    #if MEDIAINFO_DEMUX
    //     Demux_UnpacketizeContainer=Config->Demux_Unpacketize_Get();
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
void File_MpegPs::Read_Buffer_Unsynched()
{
    Searching_TimeStamp_Start=false;

    if (Streams.empty())
       return;

    //No need anymore of this Streams
    Streams[0xBB].Searching_Payload=false; //system_start

    //Reactivating interessant PS streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
    {
        //End timestamp is out of date
        Streams[StreamID].TimeStamp_End.PTS.File_Pos=(int64u)-1;
        Streams[StreamID].TimeStamp_End.DTS.File_Pos=(int64u)-1;
        Streams[StreamID].TimeStamp_End.PTS.TimeStamp=(int64u)-1;
        Streams[StreamID].TimeStamp_End.DTS.TimeStamp=(int64u)-1;
        Streams[StreamID].Searching_TimeStamp_Start=false;
        for (size_t Pos=0; Pos<Streams[StreamID].Parsers.size(); Pos++)
            if (Streams[StreamID].Parsers[Pos])
            {
                #if MEDIAINFO_SEEK
                    if (IsSub)
                        Streams[StreamID].Parsers[Pos]->Unsynch_Frame_Count=Frame_Count_NotParsedIncluded;
                #endif //MEDIAINFO_SEEK
                Streams[StreamID].Parsers[Pos]->Open_Buffer_Unsynch();
            }
        Streams_Private1[StreamID].TimeStamp_End.PTS.File_Pos=(int64u)-1;
        Streams_Private1[StreamID].TimeStamp_End.DTS.File_Pos=(int64u)-1;
        Streams_Private1[StreamID].TimeStamp_End.PTS.TimeStamp=(int64u)-1;
        Streams_Private1[StreamID].TimeStamp_End.DTS.TimeStamp=(int64u)-1;
        Streams_Private1[StreamID].Searching_TimeStamp_Start=false;
        for (size_t Pos=0; Pos<Streams_Private1[StreamID].Parsers.size(); Pos++)
            if (Streams_Private1[StreamID].Parsers[Pos])
            {
                #if MEDIAINFO_SEEK
                    Streams_Private1[StreamID].Parsers[Pos]->Unsynch_Frame_Count=Unsynch_Frame_Count;
                #endif //MEDIAINFO_SEEK
                Streams_Private1[StreamID].Parsers[Pos]->Open_Buffer_Unsynch();
            }
        Streams_Extension[StreamID].TimeStamp_End.PTS.File_Pos=(int64u)-1;
        Streams_Extension[StreamID].TimeStamp_End.DTS.File_Pos=(int64u)-1;
        Streams_Extension[StreamID].TimeStamp_End.PTS.TimeStamp=(int64u)-1;
        Streams_Extension[StreamID].TimeStamp_End.DTS.TimeStamp=(int64u)-1;
        Streams_Extension[StreamID].Searching_TimeStamp_Start=false;
        for (size_t Pos=0; Pos<Streams_Extension[StreamID].Parsers.size(); Pos++)
            if (Streams_Extension[StreamID].Parsers[Pos])
            {
                #if MEDIAINFO_SEEK
                    Streams_Extension[StreamID].Parsers[Pos]->Unsynch_Frame_Count=Unsynch_Frame_Count;
                #endif //MEDIAINFO_SEEK
                Streams_Extension[StreamID].Parsers[Pos]->Open_Buffer_Unsynch();
            }
    }
    #if MEDIAINFO_SEEK
        Unsynch_Frame_Count=(int64u)-1; //We do not use it
    #endif //MEDIAINFO_SEEK
    video_stream_Unlimited=false;
    Buffer_DataSizeToParse=0;
    PES_FirstByte_IsAvailable=false;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_MpegPs::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    //Reset
    Seek_Value=(int64u)-1;
    Seek_ID=(int64u)-1;

    //Init
    if (!Duration_Detected)
    {
        //External IBI
        #if MEDIAINFO_IBI
            std::string IbiFile=Config->Ibi_Get();
            if (!IbiFile.empty())
            {
                Ibi.Streams.clear(); //TODO: support IBI data from different inputs

                File_Ibi MI;
                Open_Buffer_Init(&MI, IbiFile.size());
                MI.Ibi=&Ibi;
                MI.Open_Buffer_Continue((const int8u*)IbiFile.c_str(), IbiFile.size());
            }
            //Creating base IBI from a quick analysis of the file
            else
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
                for (ibi::streams::iterator IbiStream_Temp=((File_MpegPs*)MI.Info)->Ibi.Streams.begin(); IbiStream_Temp!=((File_MpegPs*)MI.Info)->Ibi.Streams.end(); ++IbiStream_Temp)
                {
                    if (Ibi.Streams[IbiStream_Temp->first]==NULL)
                        Ibi.Streams[IbiStream_Temp->first]=new ibi::stream(*IbiStream_Temp->second);
                    Ibi.Streams[IbiStream_Temp->first]->Unsynch();
                    for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                    {
                        Ibi.Streams[IbiStream_Temp->first]->Add(IbiStream_Temp->second->Infos[Pos]);
                        if (!IbiStream_Temp->second->Infos[Pos].IsContinuous)
                            Ibi.Streams[IbiStream_Temp->first]->Unsynch();
                    }
                    Ibi.Streams[IbiStream_Temp->first]->Unsynch();
                }
                if (Ibi.Streams.empty())
                    return 4; //Problem during IBI file parsing
            }
        #endif //#if MEDIAINFO_IBI

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
                    #if MEDIAINFO_IBI
                    {
                    ibi::streams::iterator IbiStream_Temp;
                    if (ID==(int64u)-1)
                        IbiStream_Temp=Ibi.Streams.begin();
                    else
                        IbiStream_Temp=Ibi.Streams.find(ID);
                    if (IbiStream_Temp==Ibi.Streams.end() || IbiStream_Temp->second->Infos.empty())
                        return 5; //Invalid ID

                    if (!(IbiStream_Temp->second->DtsFrequencyNumerator==1000000000 && IbiStream_Temp->second->DtsFrequencyDenominator==1))
                    {
                        float64 ValueF=(float64)Value;
                        ValueF/=1000000000; //Value is in ns
                        ValueF/=IbiStream_Temp->second->DtsFrequencyDenominator;
                        ValueF*=IbiStream_Temp->second->DtsFrequencyNumerator;
                        Value=float64_int64s(ValueF);
                    }

                    for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                    {
                        if (Value<=IbiStream_Temp->second->Infos[Pos].Dts)
                        {
                            if (Value<IbiStream_Temp->second->Infos[Pos].Dts && Pos)
                                Pos--;

                            //Checking continuity of Ibi
                            if (!IbiStream_Temp->second->Infos[Pos].IsContinuous && Pos+1<IbiStream_Temp->second->Infos.size())
                            {
                                Config->Demux_IsSeeking=true;
                                Seek_Value=Value;
                                Seek_Value_Maximal=IbiStream_Temp->second->Infos[Pos+1].StreamOffset;
                                Seek_ID=IbiStream_Temp->first;
                                GoTo((IbiStream_Temp->second->Infos[Pos].StreamOffset+IbiStream_Temp->second->Infos[Pos+1].StreamOffset)/2);
                                Open_Buffer_Unsynch();

                                return 1;
                            }

                            Config->Demux_IsSeeking=false;
                            if (!Streams[(size_t)IbiStream_Temp->first].Parsers.empty())
                                for (size_t Parser_Pos=0; Parser_Pos<Streams[(size_t)IbiStream_Temp->first].Parsers.size(); Parser_Pos++)
                                    Streams[(size_t)IbiStream_Temp->first].Parsers[Parser_Pos]->Unsynch_Frame_Count=IbiStream_Temp->second->Infos[Pos].FrameNumber;
                            else
                                Unsynch_Frame_Counts[(int16u)IbiStream_Temp->first]=IbiStream_Temp->second->Infos[Pos].FrameNumber;

                            GoTo(IbiStream_Temp->second->Infos[Pos].StreamOffset);
                            Open_Buffer_Unsynch();

                            return 1;
                        }
                    }

                    return 2; //Invalid value
                    }
                    #else //MEDIAINFO_IBI
                    return (size_t)-2; //Not supported / IBI disabled
                    #endif //MEDIAINFO_IBI
        case 3  :   //FrameNumber
                    #if MEDIAINFO_IBI
                    {
                    ibi::streams::iterator IbiStream_Temp;
                    if (ID==(int64u)-1)
                        IbiStream_Temp=Ibi.Streams.begin();
                    else
                        IbiStream_Temp=Ibi.Streams.find(ID);
                    if (IbiStream_Temp==Ibi.Streams.end() || IbiStream_Temp->second->Infos.empty())
                        return 5; //Invalid ID

                    for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                    {
                        if (Value<=IbiStream_Temp->second->Infos[Pos].FrameNumber)
                        {
                            if (Value<IbiStream_Temp->second->Infos[Pos].FrameNumber && Pos)
                                Pos--;

                            if (!Streams[(size_t)IbiStream_Temp->first].Parsers.empty())
                                for (size_t Parser_Pos=0; Parser_Pos<Streams[(size_t)IbiStream_Temp->first].Parsers.size(); Parser_Pos++)
                                    Streams[(size_t)IbiStream_Temp->first].Parsers[Parser_Pos]->Unsynch_Frame_Count=IbiStream_Temp->second->Infos[Pos].FrameNumber;
                            else
                                Unsynch_Frame_Counts[(int16u)IbiStream_Temp->first]=IbiStream_Temp->second->Infos[Pos].FrameNumber;

                            GoTo(IbiStream_Temp->second->Infos[Pos].StreamOffset);
                            Open_Buffer_Unsynch();

                            return 1;
                        }
                    }

                    return 2; //Invalid value
                    }
                    #else //MEDIAINFO_IBI
                    return (size_t)-2; //Not supported / IBI disabled
                    #endif //MEDIAINFO_IBI
        default :   return (size_t)-1; //Not supported
    }
}
#endif //MEDIAINFO_SEEK

//---------------------------------------------------------------------------
void File_MpegPs::Read_Buffer_Continue()
{
    #if MEDIAINFO_DEMUX
        if (Demux_StreamIsBeingParsed_type!=(int8u)-1)
        {
            switch (Demux_StreamIsBeingParsed_type) //TODO: transform the switch() case to a enum with a vector of streams
            {
                case 0 :    Open_Buffer_Continue(Streams[Demux_StreamIsBeingParsed_stream_id].Parsers[0], Buffer, 0, false);
                            if (IsSub && Streams[Demux_StreamIsBeingParsed_stream_id].Parsers[0]->Frame_Count_NotParsedIncluded!=(int64u)-1)
                                Frame_Count_NotParsedIncluded=Streams[Demux_StreamIsBeingParsed_stream_id].Parsers[0]->Frame_Count_NotParsedIncluded;
                            break;
                case 1 :    Open_Buffer_Continue(Streams_Private1[Demux_StreamIsBeingParsed_stream_id].Parsers[0], Buffer, 0, false);
                            if (IsSub && Streams_Private1[Demux_StreamIsBeingParsed_stream_id].Parsers[0]->Frame_Count_NotParsedIncluded!=(int64u)-1)
                                Frame_Count_NotParsedIncluded=Streams_Private1[Demux_StreamIsBeingParsed_stream_id].Parsers[0]->Frame_Count_NotParsedIncluded;
                            break;
                case 2 :    Open_Buffer_Continue(Streams_Extension[Demux_StreamIsBeingParsed_stream_id].Parsers[0], Buffer, 0, false);
                            if (IsSub && Streams_Extension[Demux_StreamIsBeingParsed_stream_id].Parsers[0]->Frame_Count_NotParsedIncluded!=(int64u)-1)
                                Frame_Count_NotParsedIncluded=Streams_Extension[Demux_StreamIsBeingParsed_stream_id].Parsers[0]->Frame_Count_NotParsedIncluded;
                            break;
                default: ;
            }
            if (Config->Demux_EventWasSent)
                return;
            Demux_StreamIsBeingParsed_type=(int8u)-1;
        }
    #endif //MEDIAINFO_DEMUX

    if (!IsSub)
    {
        if (Config->ParseSpeed>=1.0)
            Config->State_Set(((float)Buffer_TotalBytes)/File_Size);
        else if (Buffer_TotalBytes>2*SizeToAnalyze)
            Config->State_Set((float)0.99); //Nearly the end
        else
            Config->State_Set(((float)Buffer_TotalBytes)/(2*SizeToAnalyze));
    }

    if (Buffer_DataSizeToParse)
    {
        #if MEDIAINFO_EVENTS
            if (FromTS)
            {
                PES_FirstByte_IsAvailable=true;
                PES_FirstByte_Value=false;
            }
        #endif //MEDIAINFO_EVENTS

        if (Buffer_Size<=Buffer_DataSizeToParse)
        {
            Element_Size=Buffer_Size; //All the buffer is used
            Buffer_DataSizeToParse-=(int16u)Buffer_Size;
        }
        else
        {
            Element_Size=Buffer_DataSizeToParse;
            Buffer_DataSizeToParse=0;
        }

        Element_Begin0();
        Data_Parse();
        Element_Offset=Element_Size;
        Element_End0();
    }

    //Video unlimited specific, we didn't wait for the end (because this is... unlimited)
    if (video_stream_Unlimited)
    {
        PES_FirstByte_IsAvailable=true;
        PES_FirstByte_Value=false;

        //Look for next Sync word
        size_t Buffer_Offset_Temp=0;
        while (Buffer_Offset_Temp+4<=Buffer_Size
            && (Buffer[Buffer_Offset_Temp  ]!=0x00
             || Buffer[Buffer_Offset_Temp+1]!=0x00
             || Buffer[Buffer_Offset_Temp+2]!=0x01
             || Buffer[Buffer_Offset_Temp+3]< 0xB9))
        {
            Buffer_Offset_Temp+=2;
            while(Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp]!=0x00)
                Buffer_Offset_Temp+=2;
            if (Buffer_Offset_Temp>=Buffer_Size || Buffer[Buffer_Offset_Temp-1]==0x00)
                Buffer_Offset_Temp--;
        }

        //Parsing last bytes if needed
        if (Buffer_Offset_Temp+4==Buffer_Size && (Buffer[Buffer_Offset_Temp  ]!=0x00
                                               || Buffer[Buffer_Offset_Temp+1]!=0x00
                                               || Buffer[Buffer_Offset_Temp+2]!=0x01))
            Buffer_Offset_Temp++;
        if (Buffer_Offset_Temp+3==Buffer_Size && (Buffer[Buffer_Offset_Temp  ]!=0x00
                                               || Buffer[Buffer_Offset_Temp+1]!=0x00
                                               || Buffer[Buffer_Offset_Temp+2]!=0x01))
            Buffer_Offset_Temp++;
        if (Buffer_Offset_Temp+2==Buffer_Size && (Buffer[Buffer_Offset_Temp  ]!=0x00
                                               || Buffer[Buffer_Offset_Temp+1]!=0x00))
            Buffer_Offset_Temp++;
        if (Buffer_Offset_Temp+1==Buffer_Size &&  Buffer[Buffer_Offset_Temp  ]!=0x00)
            Buffer_Offset_Temp++;

        if (Buffer_Offset_Temp==Buffer_Size)
        {
            Element_Size=Buffer_Size; //All the buffer is used
        }
        else
        {
            Element_Size=Buffer_Offset_Temp;
            if (Buffer_Offset_Temp+4<=Buffer_Size)
                video_stream_Unlimited=false;
            else
                Element_IsWaitingForMoreData(); //We don't know if the next bytes are a stream_id or data
        }

        if (Element_Size)
        {
            Element_Begin0();
            Data_Parse();
            Element_Offset=Element_Size;
            Element_End0();
        }
    }
}

//---------------------------------------------------------------------------
void File_MpegPs::Read_Buffer_AfterParsing()
{
    if (!Status[IsFilled])
    {
        //In case of problem with some streams
        if (Buffer_TotalBytes>Buffer_TotalBytes_FirstSynched+SizeToAnalyze)
        {
            if (!Status[IsAccepted])
            {
                Reject("MPEG-PS");
                return;
            }

            video_stream_Count=0;
            audio_stream_Count=0;
            private_stream_1_Count=0;
            private_stream_2_Count=false;
            extension_stream_Count=0;
            SL_packetized_stream_Count=false;
        }

        //Jumping only if needed
        if (Streams.empty() || video_stream_Count || audio_stream_Count || private_stream_1_Count || private_stream_2_Count || extension_stream_Count || SL_packetized_stream_Count)
            return;

        //Jumping if needed
        if (!Status[IsAccepted])
        {
            Accept("MPEG-PS");
            if (!IsSub)
                Fill(Stream_General, 0, General_Format, "MPEG-PS");
        }
        Fill("MPEG-PS");
        if (!ShouldContinueParsing && File_Offset+Buffer_Size+SizeToAnalyze<File_Size && Config->ParseSpeed<1.0)
        {
            //Jumping
            GoToFromEnd(SizeToAnalyze, "MPEG-PS");
            Open_Buffer_Unsynch();
        }
    }
}

//***************************************************************************
// Buffer - Par element
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegPs::Header_Parse()
{
    PES_FirstByte_IsAvailable=true;
    PES_FirstByte_Value=true;

    //Reinit
    FrameInfo.PTS=(int64u)-1;
    FrameInfo.DTS=(int64u)-1;

    #if MEDIAINFO_TRACE
    if (Trace_Activated)
    {
        //Parsing
        Skip_B3(                                                    "synchro");
        Get_B1 (stream_id,                                         "stream_id");
    }
    else
    {
    #endif //MEDIAINFO_TRACE
        //Parsing
        stream_id=Buffer[Buffer_Offset+3];
        Element_Offset+=4;
    #if MEDIAINFO_TRACE
    }
    #endif //MEDIAINFO_TRACE

    if (stream_id!=0xB9 && stream_id!=0xBA) //MPEG_program_end or pack_start have no PES
    {
        if (!Header_Parse_PES_packet(stream_id))
        {
            Element_WaitForMoreData();
            return;
        }
    }
    else if (!Header_Parse_Fill_Size()) //MPEG_program_end or pack_start specific
    {
        Element_WaitForMoreData();
        return;
    }
    Header_Fill_Code(stream_id);
}

//---------------------------------------------------------------------------
bool File_MpegPs::Header_Parse_Fill_Size()
{
    //Look for next Sync word
    if (Buffer_Offset_Temp==0) //Buffer_Offset_Temp is not 0 if Header_Parse_Fill_Size() has already parsed first frames
        Buffer_Offset_Temp=Buffer_Offset+4;
    while (Buffer_Offset_Temp+4<=Buffer_Size
        && (Buffer[Buffer_Offset_Temp  ]!=0x00
         || Buffer[Buffer_Offset_Temp+1]!=0x00
         || Buffer[Buffer_Offset_Temp+2]!=0x01
         || Buffer[Buffer_Offset_Temp+3]< 0xB9))
    {
        Buffer_Offset_Temp+=2;
        while(Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp]!=0x00)
            Buffer_Offset_Temp+=2;
        if (Buffer_Offset_Temp>=Buffer_Size || Buffer[Buffer_Offset_Temp-1]==0x00)
            Buffer_Offset_Temp--;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset_Temp+4==Buffer_Size && (Buffer[Buffer_Offset_Temp  ]!=0x00
                                           || Buffer[Buffer_Offset_Temp+1]!=0x00
                                           || Buffer[Buffer_Offset_Temp+2]!=0x01))
        Buffer_Offset_Temp++;
    if (Buffer_Offset_Temp+3==Buffer_Size && (Buffer[Buffer_Offset_Temp  ]!=0x00
                                           || Buffer[Buffer_Offset_Temp+1]!=0x00
                                           || Buffer[Buffer_Offset_Temp+2]!=0x01))
        Buffer_Offset_Temp++;
    if (Buffer_Offset_Temp+2==Buffer_Size && (Buffer[Buffer_Offset_Temp  ]!=0x00
                                           || Buffer[Buffer_Offset_Temp+1]!=0x00))
        Buffer_Offset_Temp++;
    if (Buffer_Offset_Temp+1==Buffer_Size &&  Buffer[Buffer_Offset_Temp  ]!=0x00)
        Buffer_Offset_Temp++;

    if (Buffer_Offset_Temp+4>Buffer_Size)
    {
        if (File_Offset+Buffer_Size>=File_Size)
            Buffer_Offset_Temp=Buffer_Size; //We are sure that the next bytes are a start
        else
            return false;
    }

    //OK, we continue
    Header_Fill_Size(Buffer_Offset_Temp-Buffer_Offset);
    Buffer_Offset_Temp=0;
    return true;
}

//---------------------------------------------------------------------------
bool File_MpegPs::Header_Parse_PES_packet(int8u stream_id)
{
    //Parsing
    int16u PES_packet_length;
    Get_B2 (PES_packet_length,                                  "PES_packet_length");
    #if MEDIAINFO_DEMUX
        if (Demux_UnpacketizeContainer && Buffer_Offset+6+PES_packet_length>Buffer_Size)
            return false;
    #endif //MEDIAINFO_DEMUX
    if (PES_packet_length && File_Offset+Buffer_Offset+6+PES_packet_length>=File_Size)
        PES_packet_length=(int16u)(File_Size-(File_Offset+Buffer_Offset+6));

    //Parsing
    switch (stream_id)
    {
        //Header is only Size
        case 0xBB : //system_header_start
        case 0xBC : //program_stream_map
        case 0xBE : //padding_stream
        case 0xBF : //private_stream_2
        case 0xF0 : //ECM
        case 0xF1 : //EMM
        case 0xF2 : //DSMCC Streams
        case 0xF8 : //ITU-T Rec. H .222.1 type E
        case 0xFF : //Program Streams directory
            break;

        //Element with PES Header
        default :
            switch (MPEG_Version)
            {
                case 1  : Header_Parse_PES_packet_MPEG1(stream_id); break;
                case 2  : Header_Parse_PES_packet_MPEG2(stream_id); break;
                default : ; //We don't know what to parse...
            }
    }

    //Video unlimited specific
    if (PES_packet_length==0)
    {
        if (!Header_Parse_Fill_Size())
        {
            //Return directly if we must unpack the elementary stream;
            #if MEDIAINFO_DEMUX
                if (Demux_UnpacketizeContainer)
                    return false;
            #endif //MEDIAINFO_DEMUX

            //Next PS packet is not found, we will use all the buffer
            Header_Fill_Size(Buffer_Size-Buffer_Offset); //All the buffer is used
            video_stream_Unlimited=true;
            Buffer_Offset_Temp=0; //We use the buffer
        }
    }
    else
        //Filling
        Header_Fill_Size(6+PES_packet_length);

    //Can be cut in small chunks
    if (Element_IsWaitingForMoreData())
        return false;
    if (PES_packet_length!=0 && Element_Offset<Element_Size && (size_t)(6+PES_packet_length)>Buffer_Size-Buffer_Offset
     && ((stream_id&0xE0)==0xC0 || (stream_id&0xF0)==0xE0))
    {
        //Return directly if we must unpack the elementary stream;
        #if MEDIAINFO_DEMUX
            if (Demux_UnpacketizeContainer)
                return false;
        #endif //MEDIAINFO_DEMUX

        Header_Fill_Size(Buffer_Size-Buffer_Offset); //All the buffer is used
        Buffer_DataSizeToParse=6+PES_packet_length-(int16u)(Buffer_Size-Buffer_Offset);
        Buffer_Offset_Temp=0; //We use the buffer
    }

    return true;
}

//---------------------------------------------------------------------------
// Packet header data - MPEG-1
void File_MpegPs::Header_Parse_PES_packet_MPEG1(int8u stream_id)
{
    int8u stuffing_byte;
    do
    {
        Peek_B1(stuffing_byte);
        if (stuffing_byte==0xFF)
            Skip_B1(                                            "stuffing_byte");
    }
    while(stuffing_byte==0xFF);

    if ((stuffing_byte&0xC0)==0x40)
    {
        BS_Begin();
        Mark_0();
        Mark_1();
        Skip_SB(                                                "STD_buffer_scale");
        Skip_S2(13,                                             "STD_buffer_size");
        BS_End();
        Peek_B1(stuffing_byte);
    }
    if ((stuffing_byte&0xF0)==0x20)
    {
        int16u PTS_29, PTS_14;
        int8u  PTS_32;
        Element_Begin1("PTS");
        BS_Begin();
        Mark_0();
        Mark_0();
        Mark_1();
        Mark_0();
        Get_S1 ( 3, PTS_32,                                     "PTS_32");
        Mark_1_NoTrustError(); //Found 0 in one file
        Get_S2 (15, PTS_29,                                     "PTS_29");
        Mark_1();
        Get_S2 (15, PTS_14,                                     "PTS_14");
        Mark_1();
        BS_End();

        //Filling
        FrameInfo.PTS=(((int64u)PTS_32)<<30)
                    | (((int64u)PTS_29)<<15)
                    | (((int64u)PTS_14));
        if (Streams[stream_id].Searching_TimeStamp_End && stream_id!=0xBD && stream_id!=0xFD) //0xBD and 0xFD can contain multiple streams, TimeStamp management is in Streams management
        {
            if (Streams[stream_id].TimeStamp_End.PTS.TimeStamp==(int64u)-1)
                Streams[stream_id].TimeStamp_End.PTS.TimeStamp=FrameInfo.PTS;
            while (FrameInfo.PTS+0x100000000LL<Streams[stream_id].TimeStamp_End.PTS.TimeStamp)
                FrameInfo.PTS+=0x200000000LL;
            Streams[stream_id].TimeStamp_End.DTS.File_Pos=Streams[stream_id].TimeStamp_End.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_End.DTS.TimeStamp=Streams[stream_id].TimeStamp_End.PTS.TimeStamp=FrameInfo.PTS;
        }
        if (Searching_TimeStamp_Start && Streams[stream_id].Searching_TimeStamp_Start && stream_id!=0xBD && stream_id!=0xFD) //0xBD and 0xFD can contain multiple streams, TimeStamp management is in Streams management
        {
            Streams[stream_id].TimeStamp_Start.DTS.File_Pos=Streams[stream_id].TimeStamp_Start.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_Start.DTS.TimeStamp=Streams[stream_id].TimeStamp_Start.PTS.TimeStamp=FrameInfo.PTS;
            Streams[stream_id].Searching_TimeStamp_Start=false;
        }
        Element_Info_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/90));
        FrameInfo.DTS=FrameInfo.PTS=FrameInfo.PTS*1000000/90; //In ns
        HasTimeStamps=true;
        Element_End0();
    }
    else if ((stuffing_byte&0xF0)==0x30)
    {
        int16u PTS_29, PTS_14, DTS_29, DTS_14;
        int8u  PTS_32, DTS_32;
        Element_Begin1("PTS");
        BS_Begin();
        Mark_0();
        Mark_0();
        Mark_1();
        Mark_1();
        Get_S1 ( 3, PTS_32,                                     "PTS_32");
        Mark_1_NoTrustError(); //Found 0 in one file
        Get_S2 (15, PTS_29,                                     "PTS_29");
        Mark_1();
        Get_S2 (15, PTS_14,                                     "PTS_14");
        Mark_1();
        BS_End();

        //Filling
        FrameInfo.PTS=(((int64u)PTS_32)<<30)
                    | (((int64u)PTS_29)<<15)
                    | (((int64u)PTS_14));
        if (Streams[stream_id].Searching_TimeStamp_End)
        {
            if (Streams[stream_id].TimeStamp_End.PTS.TimeStamp==(int64u)-1)
                Streams[stream_id].TimeStamp_End.PTS.TimeStamp=FrameInfo.PTS;
            while (FrameInfo.PTS+0x100000000LL<Streams[stream_id].TimeStamp_End.PTS.TimeStamp)
                FrameInfo.PTS+=0x200000000LL;
            Streams[stream_id].TimeStamp_End.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_End.PTS.TimeStamp=FrameInfo.PTS;
        }
        if (Searching_TimeStamp_Start && Streams[stream_id].Searching_TimeStamp_Start)
        {
            Streams[stream_id].TimeStamp_Start.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_Start.PTS.TimeStamp=FrameInfo.PTS;
        }
        Element_Info_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/90));
        FrameInfo.PTS=FrameInfo.PTS*1000000/90; //In ns
        Element_End0();

        Element_Begin1("DTS");
        BS_Begin();
        Mark_0();
        Mark_0();
        Mark_0();
        Mark_1_NoTrustError(); //Is "0" in one sample
        Get_S1 ( 3, DTS_32,                                     "DTS_32");
        Mark_1();
        Get_S2 (15, DTS_29,                                     "DTS_29");
        Mark_1();
        Get_S2 (15, DTS_14,                                     "DTS_14");
        Mark_1();
        BS_End();

        //Filling
        FrameInfo.DTS=(((int64u)DTS_32)<<30)
                    | (((int64u)DTS_29)<<15)
                    | (((int64u)DTS_14));
        if (Streams[stream_id].Searching_TimeStamp_End)
        {
            if (Streams[stream_id].TimeStamp_End.DTS.TimeStamp==(int64u)-1)
                Streams[stream_id].TimeStamp_End.DTS.TimeStamp=FrameInfo.DTS;
            while (FrameInfo.DTS+0x100000000LL<Streams[stream_id].TimeStamp_End.DTS.TimeStamp)
                FrameInfo.DTS+=0x200000000LL;
            Streams[stream_id].TimeStamp_End.DTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_End.DTS.TimeStamp=FrameInfo.DTS;
        }
        if (Searching_TimeStamp_Start && Streams[stream_id].Searching_TimeStamp_Start)
        {
            Streams[stream_id].TimeStamp_Start.DTS.TimeStamp=FrameInfo.DTS;
            Streams[stream_id].Searching_TimeStamp_Start=false;
        }
        Element_Info_From_Milliseconds(float64_int64s(((float64)FrameInfo.DTS)/90));
        FrameInfo.DTS=FrameInfo.DTS*1000000/90; //In ns
        Element_End0();
    }
    else
    {
        BS_Begin();
        Mark_0();
        Mark_0();
        Mark_0();
        Mark_0();
        Mark_1();
        Mark_1();
        Mark_1();
        Mark_1();
        BS_End();

        if (!FromTS)
            PES_FirstByte_Value=false;
    }
}

//---------------------------------------------------------------------------
// Packet header data - MPEG-2
void File_MpegPs::Header_Parse_PES_packet_MPEG2(int8u stream_id)
{
    //Parsing
    int8u PTS_DTS_flags, PES_header_data_length;
    bool ESCR_flag, ES_rate_flag, DSM_trick_mode_flag, additional_copy_info_flag, PES_CRC_flag, PES_extension_flag;
    #if MEDIAINFO_TRACE
    if (Trace_Activated)
    {
        BS_Begin();
        Mark_1_NoTrustError();
        Mark_0_NoTrustError();
        Skip_S1(2,                                                  "PES_scrambling_control");
        Skip_SB(                                                    "PES_priority");
        Skip_SB(                                                    "data_alignment_indicator");
        Skip_SB(                                                    "copyright");
        Skip_SB(                                                    "original_or_copy");
        Get_S1 (2, PTS_DTS_flags,                                   "PTS_DTS_flags");
        Get_SB (ESCR_flag,                                          "ESCR_flag");
        Get_SB (ES_rate_flag,                                       "ES_rate_flag");
        Get_SB (DSM_trick_mode_flag,                                "DSM_trick_mode_flag");
        Get_SB (additional_copy_info_flag,                          "additional_copy_info_flag");
        Get_SB (PES_CRC_flag,                                       "PES_CRC_flag");
        Get_SB (PES_extension_flag,                                 "PES_extension_flag");
        BS_End();
        Get_B1 (PES_header_data_length,                             "PES_header_data_length");
    }
    else
    {
    #endif //MEDIAINFO_TRACE
        if (Element_Offset+3>=Element_Size)
        {
            Trusted_IsNot("");
            return;
        }
        size_t Buffer_Pos_Flags=Buffer_Offset+(size_t)Element_Offset;
        if ((Buffer[Buffer_Pos_Flags]&0xC0)!=0x80) //bit 6 and 7 are 01
        {
            Element_DoNotTrust(""); //Mark bits are wrong
            return;
        }
        Buffer_Pos_Flags++;
        PTS_DTS_flags               =Buffer[Buffer_Pos_Flags]>>6;
        ESCR_flag                   =Buffer[Buffer_Pos_Flags]&0x20?true:false;
        ES_rate_flag                =Buffer[Buffer_Pos_Flags]&0x10?true:false;
        DSM_trick_mode_flag         =Buffer[Buffer_Pos_Flags]&0x08?true:false;
        additional_copy_info_flag   =Buffer[Buffer_Pos_Flags]&0x04?true:false;
        PES_CRC_flag                =Buffer[Buffer_Pos_Flags]&0x02?true:false;
        PES_extension_flag          =Buffer[Buffer_Pos_Flags]&0x01?true:false;
        Buffer_Pos_Flags++;
        PES_header_data_length      =Buffer[Buffer_Pos_Flags];
        Element_Offset+=3;
    #if MEDIAINFO_TRACE
    }
    #endif //MEDIAINFO_TRACE
    int64u Element_Pos_After_Data=Element_Offset+PES_header_data_length;
    if (Element_Pos_After_Data>Element_Size)
    {
        Element_WaitForMoreData();
        return;
    }

    //Options
    if (PTS_DTS_flags==0x2)
    {
        #if MEDIAINFO_TRACE
        if (Trace_Activated)
        {
            int16u PTS_29, PTS_14;
            int8u  PTS_32;
            Element_Begin1("PTS_DTS_flags");
            Element_Begin1("PTS");
            BS_Begin();
            Mark_0();
            Mark_0();
            Mark_1_NoTrustError(); //Is "0" in one sample
            Mark_0_NoTrustError(); //Is "1" in one sample
            Get_S1 ( 3, PTS_32,                                     "PTS_32");
            Mark_1();
            Get_S2 (15, PTS_29,                                     "PTS_29");
            Mark_1();
            Get_S2 (15, PTS_14,                                     "PTS_14");
            Mark_1();
            BS_End();
            FrameInfo.PTS=(((int64u)PTS_32)<<30)
                        | (((int64u)PTS_29)<<15)
                        | (((int64u)PTS_14));
            Element_Info_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/90));
            Element_End0();
            Element_End0();
        }
        else
        {
        #endif //MEDIAINFO_TRACE
            if (Element_Offset+5>Element_Size)
            {
                Element_WaitForMoreData();
                return;
            }
            size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;
            if ((Buffer[Buffer_Pos  ]&0xC1)!=0x01 //bit 5 and 4 are not tested because of one sample with wrong mark bits
             || (Buffer[Buffer_Pos+2]&0x01)!=0x01
             || (Buffer[Buffer_Pos+4]&0x01)!=0x01)
            {
                Element_DoNotTrust(""); //Mark bits are wrong
                return;
            }
            FrameInfo.PTS=                                  ((((int64u)Buffer[Buffer_Pos  ]&0x0E))<<29)
              | ( ((int64u)Buffer[Buffer_Pos+1]      )<<22)|((((int64u)Buffer[Buffer_Pos+2]&0xFE))<<14)
              | ( ((int64u)Buffer[Buffer_Pos+3]      )<< 7)|((((int64u)Buffer[Buffer_Pos+4]&0xFE))>> 1);
            Element_Offset+=5;
        #if MEDIAINFO_TRACE
        }
        #endif //MEDIAINFO_TRACE

        //Filling
        if (Streams[stream_id].Searching_TimeStamp_End)
        {
            if (Streams[stream_id].TimeStamp_End.PTS.TimeStamp==(int64u)-1)
                Streams[stream_id].TimeStamp_End.PTS.TimeStamp=FrameInfo.PTS;
            while (FrameInfo.PTS+0x100000000LL<Streams[stream_id].TimeStamp_End.PTS.TimeStamp)
                FrameInfo.PTS+=0x200000000LL;
            Streams[stream_id].TimeStamp_End.DTS.File_Pos=Streams[stream_id].TimeStamp_End.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_End.DTS.TimeStamp=Streams[stream_id].TimeStamp_End.PTS.TimeStamp=FrameInfo.PTS;
        }
        if (Searching_TimeStamp_Start && Streams[stream_id].Searching_TimeStamp_Start)
        {
            Streams[stream_id].TimeStamp_Start.DTS.File_Pos=Streams[stream_id].TimeStamp_Start.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_Start.DTS.TimeStamp=Streams[stream_id].TimeStamp_Start.PTS.TimeStamp=FrameInfo.PTS;
            Streams[stream_id].Searching_TimeStamp_Start=false;
        }
        FrameInfo.DTS=FrameInfo.PTS=FrameInfo.PTS*1000000/90; //In ns
        HasTimeStamps=true;
    }
    else if (PTS_DTS_flags==0x3)
    {
        size_t Buffer_Pos;
        #if MEDIAINFO_TRACE
        int16u PTS_29, PTS_14, DTS_29, DTS_14;
        int8u  PTS_32, DTS_32;
        if (Trace_Activated)
        {
            Element_Begin1("PTS_DTS_flags");
            Element_Begin1("PTS");
            BS_Begin();
            Mark_0();
            Mark_0();
            Mark_1_NoTrustError(); //Is "0" in one sample
            Mark_0_NoTrustError(); //Is "1" in one sample
            Get_S1 ( 3, PTS_32,                                     "PTS_32");
            Mark_1();
            Get_S2 (15, PTS_29,                                     "PTS_29");
            Mark_1();
            Get_S2 (15, PTS_14,                                     "PTS_14");
            Mark_1();
            BS_End();
            FrameInfo.PTS=(((int64u)PTS_32)<<30)
                        | (((int64u)PTS_29)<<15)
                        | (((int64u)PTS_14));
            Element_Info_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/90));
            Element_End0();
        }
        else
        {
        #endif //MEDIAINFO_TRACE
            if (Element_Offset+5>Element_Size)
            {
                Element_WaitForMoreData();
                return;
            }
            Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;
            if ((Buffer[Buffer_Pos  ]&0xC1)!=0x01 //bit 5 and 4 are not tested because of one sample with wrong mark bits
             || (Buffer[Buffer_Pos+2]&0x01)!=0x01
             || (Buffer[Buffer_Pos+4]&0x01)!=0x01)
            {
                Element_DoNotTrust(""); //Mark bits are wrong
                return;
            }
            FrameInfo.PTS=                                  ((((int64u)Buffer[Buffer_Pos  ]&0x0E))<<29)
              | ( ((int64u)Buffer[Buffer_Pos+1]      )<<22)|((((int64u)Buffer[Buffer_Pos+2]&0xFE))<<14)
              | ( ((int64u)Buffer[Buffer_Pos+3]      )<< 7)|((((int64u)Buffer[Buffer_Pos+4]&0xFE))>> 1);
            Element_Offset+=5;
        #if MEDIAINFO_TRACE
        }
        #endif //MEDIAINFO_TRACE

        //Filling
        if (Streams[stream_id].Searching_TimeStamp_End)
        {
            if (Streams[stream_id].TimeStamp_End.PTS.TimeStamp==(int64u)-1)
                Streams[stream_id].TimeStamp_End.PTS.TimeStamp=FrameInfo.PTS;
            while (FrameInfo.PTS+0x100000000LL<Streams[stream_id].TimeStamp_End.PTS.TimeStamp)
                FrameInfo.PTS+=0x200000000LL;
            Streams[stream_id].TimeStamp_End.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_End.PTS.TimeStamp=FrameInfo.PTS;
        }
        if (Searching_TimeStamp_Start && Streams[stream_id].Searching_TimeStamp_Start)
        {
            Streams[stream_id].TimeStamp_Start.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_Start.PTS.TimeStamp=FrameInfo.PTS;
            //Streams[stream_id].Searching_TimeStamp_Start=false; //Done with DTS
        }
        FrameInfo.PTS=FrameInfo.PTS*1000000/90; //In ns

        #if MEDIAINFO_TRACE
        if (Trace_Activated)
        {
            Element_Begin1("DTS");
            BS_Begin();
            Mark_0();
            Mark_0();
            Mark_1_NoTrustError(); //Is "0" in one sample
            Mark_0_NoTrustError(); //Is "1" in one sample
            Get_S1 ( 3, DTS_32,                                     "DTS_32");
            Mark_1();
            Get_S2 (15, DTS_29,                                     "DTS_29");
            Mark_1();
            Get_S2 (15, DTS_14,                                     "DTS_14");
            Mark_1();
            BS_End();
            FrameInfo.DTS=(((int64u)DTS_32)<<30)
                        | (((int64u)DTS_29)<<15)
                        | (((int64u)DTS_14));
            Element_Info_From_Milliseconds(float64_int64s(((float64)FrameInfo.DTS)/90));
            Element_End0();
            Element_End0();
        }
        else
        {
        #endif //MEDIAINFO_TRACE
            if (Element_Offset+5>Element_Size)
            {
                Element_WaitForMoreData();
                return;
            }
            Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;
            if ((Buffer[Buffer_Pos  ]&0xC1)!=0x01 //bit 5 and 4 are not tested because of one sample with wrong mark bits
             || (Buffer[Buffer_Pos+2]&0x01)!=0x01
             || (Buffer[Buffer_Pos+4]&0x01)!=0x01)
            {
                Element_DoNotTrust(""); //Mark bits are wrong
                return;
            }
            FrameInfo.DTS=                                  ((((int64u)Buffer[Buffer_Pos  ]&0x0E))<<29)
              | ( ((int64u)Buffer[Buffer_Pos+1]      )<<22)|((((int64u)Buffer[Buffer_Pos+2]&0xFE))<<14)
              | ( ((int64u)Buffer[Buffer_Pos+3]      )<< 7)|((((int64u)Buffer[Buffer_Pos+4]&0xFE))>> 1);
            Element_Offset+=5;
        #if MEDIAINFO_TRACE
        }
        #endif //MEDIAINFO_TRACE

        //Filling
        if (Streams[stream_id].Searching_TimeStamp_End)
        {
            if (Streams[stream_id].TimeStamp_End.DTS.TimeStamp==(int64u)-1)
                Streams[stream_id].TimeStamp_End.DTS.TimeStamp=FrameInfo.DTS;
            while (FrameInfo.DTS+0x100000000LL<Streams[stream_id].TimeStamp_End.DTS.TimeStamp)
                FrameInfo.DTS+=0x200000000LL;
            Streams[stream_id].TimeStamp_End.DTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[stream_id].TimeStamp_End.DTS.TimeStamp=FrameInfo.DTS;
        }
        if (Searching_TimeStamp_Start && Streams[stream_id].Searching_TimeStamp_Start)
        {
            Streams[stream_id].TimeStamp_Start.DTS.TimeStamp=FrameInfo.DTS;
            Streams[stream_id].Searching_TimeStamp_Start=false;
        }
        FrameInfo.DTS=FrameInfo.DTS*1000000/90; //In ns
        HasTimeStamps=true;
    }
    else if (!FromTS)
        PES_FirstByte_Value=false;
    if (ESCR_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin1("ESCR_flag");
        BS_Begin();
        int16u ESCR_29, ESCR_14, ESCR_extension;
        int8u  ESCR_32;
        Skip_S1( 2,                                             "reserved");
        Get_S1 ( 3, ESCR_32,                                    "PTS_32");
        Mark_1();
        Get_S2 (15, ESCR_29,                                    "PTS_29");
        Mark_1();
        Get_S2 (15, ESCR_14,                                    "PTS_14");
        Mark_1();
        Get_S2 (15, ESCR_extension,                             "ESCR_extension");
        Mark_1();
        BS_End();
        Element_End0();
    }
    if (ES_rate_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin1("ES_rate_flag");
        BS_Begin();
        int32u ES_rate;
        Mark_1();
        Get_S3 (22, ES_rate,                                    "ES_rate");
        Mark_1();
        BS_End();
        Element_End0();
    }
    if (DSM_trick_mode_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin1("DSM_trick_mode_flag");
        BS_Begin();
        int8u trick_mode_control;
        Get_S1 (3, trick_mode_control,                         "trick_mode_control"); Param_Info1(MpegPs_trick_mode_control_values[trick_mode_control]);
        switch (trick_mode_control)
        {
            case 0 :{ //fast_forward
                        Skip_S1(2,                              "field_id");
                        Skip_SB(                                "intra_slice_refresh");
                        Skip_S1(2,                              "frequency_truncation");
                    }
                    break;
            case 1 :{ //slow_motion
                        int8u rep_cntrl;
                        Get_S1 (5, rep_cntrl,                   "rep_cntrl");
                    }
                    break;
            case 2 :{ //freeze_frame
                        Skip_S1(2,                              "field_id");
                        Skip_S1(3,                              "reserved");
                    }
                    break;
            case 3 :{ //fast_reverse
                        Skip_S1(2,                              "field_id");
                        Skip_SB(                                "intra_slice_refresh");
                        Skip_S1(2,                              "frequency_truncation");
                    }
                    break;
            case 4 :{ //slow_reverse
                        int8u rep_cntrl;
                        Get_S1 (5, rep_cntrl,                   "rep_cntrl");
                    }
                    break;
            default:{
                        Skip_S1(5,                              "reserved");
                    }
        }
        BS_End();
        Element_End0();
    }
    if (additional_copy_info_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin1("additional_copy_info_flag");
        BS_Begin();
        Mark_1();
        Skip_S1(7,                                              "additional_copy_info");
        BS_End();
        Element_End0();
    }
    if (PES_CRC_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin1("PES_CRC_flag");
        Skip_B2(                                                "previous_PES_packet_CRC");
        Element_End0();
    }
    if (PES_extension_flag && Element_Offset<Element_Pos_After_Data)
    {
        bool PES_private_data_flag=false, pack_header_field_flag=false, program_packet_sequence_counter_flag=false, p_STD_buffer_flag=false, PES_extension_flag_2=false;
        Element_Begin1("PES_extension_flag");
        BS_Begin();
        Get_SB (PES_private_data_flag,                          "PES_private_data_flag");
        Get_SB (pack_header_field_flag,                         "pack_header_field_flag");
        Get_SB (program_packet_sequence_counter_flag,           "program_packet_sequence_counter_flag");
        Get_SB (p_STD_buffer_flag,                              "P-STD_buffer_flag");
        Skip_S1(3,                                              "reserved");
        Get_SB (PES_extension_flag_2,                           "PES_extension_flag_2");
        BS_End();

        //Integrity test
        if (Element_Offset+(PES_private_data_flag?16:0)+(pack_header_field_flag?1:0)+(program_packet_sequence_counter_flag?2:0)+(p_STD_buffer_flag?2:0)+(PES_extension_flag_2?2:0)>Element_Pos_After_Data)
        {
            //There is a problem
            PES_private_data_flag=false;
            pack_header_field_flag=false;
            program_packet_sequence_counter_flag=false;
            p_STD_buffer_flag=false;
            PES_extension_flag_2=false;
        }

        if (PES_private_data_flag)
        {
            Element_Begin1("PES_private_data");
            int32u Code;
            Peek_B4(Code);
            if (Code==0x43434953) // "CCIS"
            {
                if (Streams_Private1[private_stream_1_ID].Parsers.size()>1)
                {
                    //Should not happen, this is only in case the previous packet was without CCIS
                    Streams_Private1[private_stream_1_ID].Parsers.clear();
                    Streams_Private1[private_stream_1_ID].StreamIsRegistred=false;
                }
                if (!Streams_Private1[private_stream_1_ID].StreamIsRegistred)
                {
                    Streams_Private1[private_stream_1_ID].Parsers.push_back(ChooseParser_AribStdB24B37(true));
                    Open_Buffer_Init(Streams_Private1[private_stream_1_ID].Parsers[0]);
                    Streams_Private1[private_stream_1_ID].StreamIsRegistred=true;
                }

                if (Streams_Private1[private_stream_1_ID].Parsers.size()==1)
                {
                    File_AribStdB24B37* Parser=(File_AribStdB24B37*)Streams_Private1[private_stream_1_ID].Parsers[0];
                    Parser->ParseCcis=true;
                    Parser->Open_Buffer_Continue(Buffer+Buffer_Offset+(size_t)Element_Offset, 16);
                }
                else
                    Skip_B16(                                   "PES_private_data");
           }
            else
                Skip_B16(                                       "PES_private_data");
            Element_End0();
        }
        if (pack_header_field_flag)
        {
            Element_Begin1("pack_header_field_flag");
            int8u pack_field_length;
            Get_B1 (pack_field_length,                          "pack_field_length");
            Skip_XX(pack_field_length,                          "pack_header");
            Element_End0();
        }
        if (program_packet_sequence_counter_flag)
        {
            Element_Begin1("program_packet_sequence_counter_flag");
            int8u   program_packet_sequence_counter, original_stuff_length;
            bool    MPEG1_MPEG2_identifier;
            BS_Begin();
            Mark_1();
            Get_S1 (7, program_packet_sequence_counter,         "program_packet_sequence_counter");
            Mark_1();
            Get_SB (   MPEG1_MPEG2_identifier,                  "MPEG1_MPEG2_identifier");
            Get_S1 (6, original_stuff_length,                   "original_stuff_length");
            BS_End();
            Element_End0();
        }
        if (p_STD_buffer_flag)
        {
            Element_Begin1("p_STD_buffer_flag");
            bool P_STD_buffer_scale;
            BS_Begin();
            Mark_0();
            Skip_SB(                                            "Should be 1"); //But I saw a file with "0"
            Get_SB (    P_STD_buffer_scale,                     "P-STD_buffer_scale");
            Skip_S2(13,                                         "P-STD_buffer_size");
            BS_End();
            Element_End0();
        }
        if (PES_extension_flag_2)
        {
            Element_Begin1("PES_extension_flag_2");
            int8u PES_extension_field_length;
            bool stream_id_extension_flag;
            BS_Begin();
            Mark_1();
            Get_S1 (7, PES_extension_field_length,              "PES_extension_field_length");
            Get_SB (stream_id_extension_flag,                   "stream_id_extension_flag");
            if (stream_id_extension_flag==0) //This should be limited to stream_id_extension_flag==0, but I found a file with stream_id_extension_flag=1 and a real code...
            {
                Get_S1 (7, stream_id_extension,                 "stream_id_extension"); Param_Info1(MpegPs_stream_id_extension(stream_id_extension));
            }
            BS_End();
            if (PES_extension_field_length-1>0)
                Skip_XX(PES_extension_field_length-1,           "reserved");
            Element_End0();
        }
        Element_End0();
    }
    if (Element_Pos_After_Data>Element_Offset)
        Skip_XX(Element_Pos_After_Data-Element_Offset,          "stuffing_bytes");
}

//---------------------------------------------------------------------------
void File_MpegPs::Data_Parse()
{
    //Counting
    Frame_Count++;

    //Needed?
    if (!Streams[stream_id].Searching_Payload)
    {
        Skip_XX(Element_Size,                                   "data");
        Element_DoNotShow();
        return;
    }

    //From TS
    if (FromTS && !Status[IsAccepted])
    {
        Data_Accept("MPEG-PS");
        if (!IsSub)
            Fill(Stream_General, 0, General_Format, "MPEG-PS");
    }

    //Parsing
    switch (stream_id)
    {
        case 0xB9 : MPEG_program_end(); break;
        case 0xBA : pack_start(); break;
        case 0xBB : system_header_start(); break;
        case 0xBC : program_stream_map(); break;
        case 0xBD : private_stream_1(); break;
        case 0xBE : padding_stream(); break;
        case 0xBF : private_stream_2(); break;
        case 0xF0 : Element_Name("ECM_Stream"); Skip_XX(Element_Size, "Data"); break;
        case 0xF1 : Element_Name("EMM_Stream"); Skip_XX(Element_Size, "Data"); break;
        case 0xF2 : Element_Name("DSMCC_stream"); Skip_XX(Element_Size, "Data"); break;
        case 0xF3 : Element_Name("ISO/IEC_13522_stream"); Skip_XX(Element_Size, "Data"); break;
        case 0xF4 : Element_Name("ITU-T Rec. H.222.1 type A"); Skip_XX(Element_Size, "Data"); break;
        case 0xF5 : Element_Name("ITU-T Rec. H.222.1 type B"); Skip_XX(Element_Size, "Data"); break;
        case 0xF6 : Element_Name("ITU-T Rec. H.222.1 type C"); Skip_XX(Element_Size, "Data"); break;
        case 0xF7 : Element_Name("ITU-T Rec. H.222.1 type D"); Skip_XX(Element_Size, "Data"); break;
        case 0xF8 : Element_Name("ITU-T Rec. H.222.1 type E"); Skip_XX(Element_Size, "Data"); break;
        case 0xF9 : Element_Name("ancillary_stream"); Skip_XX(Element_Size, "Data"); break;
        case 0xFA : SL_packetized_stream(); break;
        case 0xFB : Element_Name("FlexMux_stream"); Skip_XX(Element_Size, "Data"); break;
        case 0xFC : Element_Name("descriptive data stream"); Skip_XX(Element_Size, "Data"); break;
        case 0xFD : extension_stream(); break;
        case 0xFE : video_stream(); break;
        case 0xFF : Element_Name("program_stream_directory"); Skip_XX(Element_Size, "Data"); break;
        default:
                 if ((stream_id&0xE0)==0xC0) audio_stream();
            else if ((stream_id&0xF0)==0xE0) video_stream();
            else
                Trusted_IsNot("Unattended element!");
    }

    #if MEDIAINFO_EVENTS
        PES_FirstByte_IsAvailable=false;
    #endif //MEDIAINFO_EVENTS
}

//---------------------------------------------------------------------------
//Jumping to the last DTS if needed
bool File_MpegPs::BookMark_Needed()
{
    if (IsSub || Streams.empty() || Config->ParseSpeed>=1.0)
        return false;

    int64u ToJump=(int64u)-1;
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
    {
        //End timestamp is out of date
        if (Streams[StreamID].TimeStamp_End.PTS.File_Pos!=(int64u)-1)
        {
            if (Streams[StreamID].TimeStamp_End.PTS.File_Pos<ToJump)
                ToJump=Streams[StreamID].TimeStamp_End.PTS.File_Pos;
            Streams[StreamID].Searching_Payload=true;
        }
        if (Streams[StreamID].TimeStamp_End.DTS.File_Pos!=(int64u)-1)
        {
            if (Streams[StreamID].TimeStamp_End.DTS.File_Pos<ToJump)
                ToJump=Streams[StreamID].TimeStamp_End.DTS.File_Pos;
            Streams[StreamID].Searching_Payload=true;
        }
        if (Streams_Private1[StreamID].TimeStamp_End.PTS.File_Pos!=(int64u)-1)
        {
            if (Streams_Private1[StreamID].TimeStamp_End.PTS.File_Pos<ToJump)
                ToJump=Streams_Private1[StreamID].TimeStamp_End.PTS.File_Pos;
            Streams_Private1[StreamID].Searching_Payload=true;
        }
        if (Streams_Private1[StreamID].TimeStamp_End.DTS.File_Pos!=(int64u)-1)
        {
            if (Streams_Private1[StreamID].TimeStamp_End.DTS.File_Pos<ToJump)
                ToJump=Streams_Private1[StreamID].TimeStamp_End.DTS.File_Pos;
            Streams_Private1[StreamID].Searching_Payload=true;
        }
        if (Streams_Extension[StreamID].TimeStamp_End.PTS.File_Pos!=(int64u)-1)
        {
            if (Streams_Extension[StreamID].TimeStamp_End.PTS.File_Pos<ToJump)
                ToJump=Streams_Extension[StreamID].TimeStamp_End.PTS.File_Pos;
            Streams_Extension[StreamID].Searching_Payload=true;
        }
        if (Streams_Extension[StreamID].TimeStamp_End.DTS.File_Pos!=(int64u)-1)
        {
            if (Streams_Extension[StreamID].TimeStamp_End.DTS.File_Pos<ToJump)
                ToJump=Streams_Extension[StreamID].TimeStamp_End.DTS.File_Pos;
            Streams_Extension[StreamID].Searching_Payload=true;
        }
    }

    return false;
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
// Packet "B9"
void File_MpegPs::MPEG_program_end()
{
    Element_Name("MPEG_program_end");
}

//---------------------------------------------------------------------------
// Packet "BA"
void File_MpegPs::pack_start()
{
    Element_Name("pack_start");

    //Parsing
    int16u SysClock_29, SysClock_14;
    int8u Version, SysClock_32;
    size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;
    #if MEDIAINFO_TRACE
    if (Trace_Activated)
    {
        //Parsing
        BS_Begin();
        Peek_S1( 2, Version);
    }
    else
    {
    #endif //MEDIAINFO_TRACE
        //Parsing
        Version=Buffer[Buffer_Pos]>>6;
    #if MEDIAINFO_TRACE
    }
    #endif //MEDIAINFO_TRACE
    if (Version==1)
    {
        //MPEG-2
        #if MEDIAINFO_TRACE
        if (Trace_Activated)
        {
            //Parsing
            int8u Padding;
            Mark_0();
            Mark_1();
            Get_S1 ( 3, SysClock_32,                                "system_clock_reference_base32");
            Mark_1();
            Get_S2 (15, SysClock_29,                                "system_clock_reference_base29");
            Mark_1();
            Get_S2 (15, SysClock_14,                                "system_clock_reference_base14");

            //Filling
            Streams[0xBA].TimeStamp_End.PTS.TimeStamp=(((int64u)SysClock_32)<<30)
                                                    | (((int64u)SysClock_29)<<15)
                                                    | (((int64u)SysClock_14));
            if (Searching_TimeStamp_Start && Streams[0xBA].Searching_TimeStamp_Start)
            {
                Streams[0xBA].TimeStamp_Start=Streams[0xBA].TimeStamp_End;
                Streams[0xBA].Searching_TimeStamp_Start=false;
            }
            Param_Info_From_Milliseconds(Streams[0xBA].TimeStamp_End.PTS.TimeStamp/90);

            Mark_1();
            Skip_S2( 9,                                             "system_clock_reference_extension");
            Mark_1();
            Get_S3 (22, program_mux_rate,                           "program_mux_rate"); Param_Info2(program_mux_rate*400, " bps");
            Mark_1();
            Mark_1();
            Skip_S1( 5,                                             "reserved");
            Get_S1 ( 3, Padding,                                    "pack_stuffing_length");
            BS_End();
            if (Padding>0)
                Skip_XX(Padding,                                    "padding");
        }
        else
        {
        #endif //MEDIAINFO_TRACE
            //Parsing
            Streams[0xBA].TimeStamp_End.PTS.TimeStamp=((Buffer[Buffer_Pos  ]&0x38)<<30)
                                                    | ((Buffer[Buffer_Pos  ]&0x03)<<28)
                                                    | ((Buffer[Buffer_Pos+1]     )<<20)
                                                    | ((Buffer[Buffer_Pos+2]&0xF8)<<15)
                                                    | ((Buffer[Buffer_Pos+2]&0x03)<<13)
                                                    | ((Buffer[Buffer_Pos+3]     )<< 5)
                                                    | ((Buffer[Buffer_Pos+4]&0xF8)>> 3);
            if (!Status[IsAccepted])
            {
                program_mux_rate                     =((Buffer[Buffer_Pos+6]     )<<14)
                                                    | ((Buffer[Buffer_Pos+7]     )<< 6)
                                                    | ((Buffer[Buffer_Pos+8]     )>> 2);
            }
            int8u Padding                            =  Buffer[Buffer_Pos+9]&0x07;
            Element_Offset=10+Padding;
        #if MEDIAINFO_TRACE
        }
        #endif //MEDIAINFO_TRACE
    }
    else
    {
        BS_Begin();
        Mark_0();
        Mark_0();
        Mark_1();
        Mark_0();
        Get_S1 ( 3, SysClock_32,                                "system_clock_reference_base32");
        Mark_1();
        Get_S2 (15, SysClock_29,                                "system_clock_reference_base29");
        Mark_1();
        Get_S2 (15, SysClock_14,                                "system_clock_reference_base14");

        //Filling
        Streams[0xBA].TimeStamp_End.PTS.TimeStamp=(((int64u)SysClock_32)<<30)
                                                | (((int64u)SysClock_29)<<15)
                                                | (((int64u)SysClock_14));
        if (Searching_TimeStamp_Start && Streams[0xBA].Searching_TimeStamp_Start)
        {
            Streams[0xBA].TimeStamp_Start=Streams[0xBA].TimeStamp_End;
            Streams[0xBA].Searching_TimeStamp_Start=false;
        }
        Param_Info_From_Milliseconds(Streams[0xBA].TimeStamp_End.PTS.TimeStamp/90);

        Mark_1();
        Mark_1();
        Get_S3(22, program_mux_rate,                            "mux_rate"); Param_Info2(program_mux_rate*400, " bps");
        Mark_1();
        BS_End();
    }


    //Filling
    FILLING_BEGIN_PRECISE();
        if (!Status[IsAccepted])
        {
            Data_Accept("MPEG-PS");
            if (!IsSub)
                Fill(Stream_General, 0, General_Format, "MPEG-PS");

            //Autorisation of other streams
            Streams[0xB9].Searching_Payload=true;            //MPEG_program_end
            Streams[0xBB].Searching_Payload=true;            //system_header_start
            Streams[0xBD].Searching_Payload=true;            //private_stream_1
            Streams[0xBD].Searching_TimeStamp_Start=true;    //private_stream_1
            Streams[0xBD].Searching_TimeStamp_End=true;      //private_stream_1
            Streams[0xBF].Searching_Payload=true;            //private_stream_2
            Streams[0xFD].Searching_Payload=true;            //private_stream_1 or video_stream
            Streams[0xFD].Searching_TimeStamp_Start=true;    //private_stream_1 or video_stream
            Streams[0xFD].Searching_TimeStamp_End=true;      //private_stream_1 or video_stream
            for (int8u Pos=0xC0; Pos<=0xEF; Pos++)
            {
                Streams[Pos].Searching_Payload=true;         //audio_stream or video_stream
                Streams[Pos].Searching_TimeStamp_Start=true; //audio_stream or video_stream
                Streams[Pos].Searching_TimeStamp_End=true;   //audio_stream or video_stream
            }

            MPEG_Version=Version==1?2:1;

            SizeToAnalyze=program_mux_rate*50*4*(MustExtendParsingDuration?4:1); //standard delay between TimeStamps is 0.7s, we try 4s to be sure
            if (SizeToAnalyze>16*1024*1024)
                SizeToAnalyze=16*1024*1024; //Not too much
            if (SizeToAnalyze<2*1024*1024)
                SizeToAnalyze=2*1024*1024; //Not too less
        }

        #if MEDIAINFO_IBI
            if (!IsSub)
                Ibi_SynchronizationOffset_Current=File_Offset+Buffer_Offset-Header_Size;
        #endif //MEDIAINFO_IBI
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "BB"
void File_MpegPs::system_header_start()
{
    Element_Name("system_header_start");

    //If there is system_header_start, default value for private sections are false
    private_stream_1_Count=0;
    private_stream_2_Count=0;
    SL_packetized_stream_Count=0;

    //StreamOrder
    StreamOrder_CountOfPrivateStreams_Minus1=0;

    //Parsing
    int32u rate_bound;
    int8u  audio_bound, video_bound;
    BS_Begin();
    Mark_1();
    Get_S3 (22, rate_bound,                                     "rate_bound"); Param_Info2(rate_bound*400, " bps");
    Mark_1();
    Get_S1 ( 6, audio_bound,                                    "audio_bound");
    Info_SB(    fixed_flag,                                     "fixed_flag"); Param_Info1(MpegPs_System_Fixed[fixed_flag]);
    Skip_SB(                                                    "CSPS_flag");
    Skip_SB(                                                    "system_audio_lock_flag");
    Skip_SB(                                                    "system_video_lock_flag");
    Mark_1();
    Get_S1 ( 5, video_bound,                                    "video_bound");
    Skip_SB(                                                    "packet_rate_restriction_flag");
    Skip_S1( 7,                                                 "reserved_byte");
    bool one=false;
    size_t StreamOrder=0;
    if (Element_IsNotFinished())
        Peek_SB(one);
    while (one)
    {
        Element_Begin0();
        int16u STD_buffer_size_bound;
        int8u stream_id, stream_id_extension=0;
        bool STD_buffer_bound_scale;
        Get_S1 ( 8, stream_id,                                  "stream_id"); Param_Info1(MpegPs_stream_id(stream_id));
        Element_Name(Ztring().From_CC1(stream_id));
        Element_Info1(MpegPs_stream_id(stream_id));
        if (stream_id==0xB7)
        {
            Mark_1();
            Mark_1();
            Mark_0();
            Mark_0();
            Mark_0();
            Mark_0();
            Mark_0();
            Mark_0();
            Mark_0();
            Get_S1 (8, stream_id_extension,                     "stream_id_extension");
            Mark_1();
            Mark_0();
            Mark_1();
            Mark_1();
            Mark_0();
            Mark_1();
            Mark_1();
            Mark_0();
        }
        Mark_1();
        Mark_1();
        Get_SB (    STD_buffer_bound_scale,                     "STD_buffer_bound_scale");
        Get_S2 (13, STD_buffer_size_bound,                      "STD_buffer_size_bound"); Param_Info1(Ztring::ToZtring(STD_buffer_size_bound*(STD_buffer_bound_scale?1024:128)) + __T(" bytes"));
        Element_End0();

        FILLING_BEGIN();
            switch (stream_id)
            {
                case 0xBD : private_stream_1_Count=(int8u)-1; break;
                case 0xBF : private_stream_2_Count=(int8u)-1; break;
                case 0xFA : SL_packetized_stream_Count=(int8u)-1; break;
                case 0xFD : extension_stream_Count=(int8u)-1; break;
                default   : ;
            }

            if (stream_id==0xBD && Streams[stream_id].StreamOrder!=(size_t)-1)
                StreamOrder_CountOfPrivateStreams_Minus1++;
            else if (stream_id>0xB9)
            {
                Streams[stream_id].StreamOrder=StreamOrder;
                StreamOrder++;
            }
        FILLING_END();

        if (Element_IsNotFinished())
            Peek_SB(one);
        else
            one=false;
    }
    BS_End();

    //Filling
    if (audio_stream_Count==(int8u)-1) //0xBB may be multipart
        audio_stream_Count=0;
    audio_stream_Count+=audio_bound;
    if (video_stream_Count==(int8u)-1) //0xBB may be multipart
        video_stream_Count=0;
    video_stream_Count+=video_bound;
    if (private_stream_1_Count>0 && program_mux_rate*50==SizeToAnalyze)
        SizeToAnalyze*=32; //If there is a private section, this may be DVD, with late data --> 10s minimum
    if (SizeToAnalyze>8*1024*1024)
        SizeToAnalyze=8*1024*1024;

    //Autorisation of other streams
    if ((private_stream_1_Count>0 || audio_stream_Count>0) && video_stream_Count>0) //0xBB may be multipart
        Streams[0xBB].Searching_Payload=false;
    Streams[0xBC].Searching_Payload=true;            //program_stream_map
}

//---------------------------------------------------------------------------
// Packet "BC"
void File_MpegPs::program_stream_map()
{
    Element_Name("program_stream_map");
    MPEG_Version=2; //program_stream_map does NOT exist in MPEG-1 specs

    File_Mpeg_Psi Parser;
    Parser.From_TS=false;
    Parser.Complete_Stream=new complete_stream;
    Parser.Complete_Stream->Streams.resize(0x100);
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Parser.Complete_Stream->Streams[StreamID]=new complete_stream::stream;
    Open_Buffer_Init(&Parser);
    Open_Buffer_Continue(&Parser);
    Finish(&Parser);

    FILLING_BEGIN();
        //Time stamps
        Streams[0xBC].TimeStamp_End=Streams[0xBA].TimeStamp_End;
        if (Streams[0xBC].TimeStamp_Start.PTS.TimeStamp==(int64u)-1)
            Streams[0xBC].TimeStamp_Start=Streams[0xBC].TimeStamp_End;

        //Registering the streams
        for (int8u Pos=0; Pos<0xFF; Pos++)
            if (Parser.Complete_Stream->Streams[Pos]->stream_type!=(int8u)-1)
            {
                if (!Parser.Complete_Stream->Transport_Streams.empty() && !Parser.Complete_Stream->Transport_Streams.begin()->second.Programs.empty())
                    Streams[Pos].program_format_identifier=Parser.Complete_Stream->Transport_Streams.begin()->second.Programs.begin()->second.registration_format_identifier;
                Streams[Pos].format_identifier=Parser.Complete_Stream->Streams[Pos]->registration_format_identifier;
                Streams[Pos].stream_type=Parser.Complete_Stream->Streams[Pos]->stream_type;
            }
            else
            {
            }
    FILLING_END();

    delete Parser.Complete_Stream; //Parser.Complete_Stream=NULL;
}

//---------------------------------------------------------------------------
// Packet "BD"
void File_MpegPs::private_stream_1()
{
    Element_Name("private_stream_1");

    if (!FromTS)
    {
        //From PS, trying DVD system
        private_stream_1_ID=0;
        private_stream_1_Offset=0;
        if (!private_stream_1_Choose_DVD_ID())
        {
            Skip_XX(Element_Size-Element_Offset,                "Unknown");
            return;
        }
        Element_Info1C(private_stream_1_ID, Ztring::ToZtring(private_stream_1_ID, 16));
    }

    if (!Streams_Private1[private_stream_1_ID].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            if (video_stream_Count==(int8u)-1 && audio_stream_Count==(int8u)-1)
            {
                video_stream_Count=0;
                audio_stream_Count=0;
                private_stream_1_Count=1;
                private_stream_2_Count=0;
                extension_stream_Count=0;
                SL_packetized_stream_Count=0;
                private_stream_1_ID=0;
                private_stream_1_Offset=0;
                Streams_Private1[private_stream_1_ID].stream_type=FromTS_stream_type;
            }
            else if (!IsSub)
            {
                //2 streams in the file, this can not be From TS, we have no idea of the count of streams
                video_stream_Count=(int8u)-1;
                audio_stream_Count=(int8u)-1;
                private_stream_1_Count=(int8u)-1;
                private_stream_2_Count=(int8u)-1;
                extension_stream_Count=(int8u)-1;
                SL_packetized_stream_Count=(int8u)-1;
                FromTS=false;
            }
        }

        //Registering
        if (!Status[IsAccepted])
        {
            Data_Accept("MPEG-PS");
            if (!IsSub)
                Fill(Stream_General, 0, General_Format, "MPEG-PS");
        }
        Streams[stream_id].StreamIsRegistred++;
        Streams_Private1[private_stream_1_ID].StreamIsRegistred++;
        Streams_Private1[private_stream_1_ID].Searching_Payload=true;
        Streams_Private1[private_stream_1_ID].Searching_TimeStamp_Start=true;
        Streams_Private1[private_stream_1_ID].Searching_TimeStamp_End=true;
        Streams_Private1[private_stream_1_ID].FirstPacketOrder=FirstPacketOrder_Last;
        FirstPacketOrder_Last++;

        //New parsers
        Streams_Private1[private_stream_1_ID].Parsers.push_back(private_stream_1_ChooseParser());
        if (Streams_Private1[private_stream_1_ID].Parsers[Streams_Private1[private_stream_1_ID].Parsers.size()-1]==NULL)
        {
            Streams_Private1[private_stream_1_ID].Parsers.clear();
            #if defined(MEDIAINFO_AC3_YES)
                Streams_Private1[private_stream_1_ID].Parsers.push_back(ChooseParser_AC3());
            #endif
            #if defined(MEDIAINFO_DTS_YES)
                Streams_Private1[private_stream_1_ID].Parsers.push_back(ChooseParser_DTS());
            #endif
            #if defined(MEDIAINFO_SMPTEST0337_YES)
                Streams_Private1[private_stream_1_ID].Parsers.push_back(ChooseParser_SmpteSt0302());
            #endif
            #if defined(MEDIAINFO_ARIBSTDB24B37_YES)
                Streams_Private1[private_stream_1_ID].Parsers.push_back(ChooseParser_AribStdB24B37());
            #endif
        }
        #if MEDIAINFO_EVENTS
            if (private_stream_1_Offset)
            {
                //Multiple substreams in 1 stream
                StreamIDs[StreamIDs_Size-1]=Element_Code;
                Element_Code=private_stream_1_ID; //The upper level ID is filled by Element_Code in the common code
                StreamIDs_Width[StreamIDs_Size]=2;
                ParserIDs[StreamIDs_Size]=MediaInfo_Parser_MpegPs_Ext;
                StreamIDs_Size++;
            }
        #endif //MEDIAINFO_EVENTS
        for (size_t Pos=0; Pos<Streams_Private1[private_stream_1_ID].Parsers.size(); Pos++)
        {
            Streams_Private1[private_stream_1_ID].Parsers[Pos]->CA_system_ID_MustSkipSlices=CA_system_ID_MustSkipSlices;
            Open_Buffer_Init(Streams_Private1[private_stream_1_ID].Parsers[Pos]);
        }
        #if MEDIAINFO_EVENTS
            if (private_stream_1_Offset)
            {
                StreamIDs_Size--;
                Element_Code=StreamIDs[StreamIDs_Size-1];
            }
        #endif //MEDIAINFO_EVENTS
    }

    //Demux
    #if MEDIAINFO_DEMUX
        if (Streams_Private1[private_stream_1_ID].Searching_Payload)
        {
            if (private_stream_1_Offset)
            {
                //Multiple substreams in 1 stream
                StreamIDs[StreamIDs_Size-1]=Element_Code;
                Element_Code=private_stream_1_ID; //The upper level ID is filled by Element_Code in the common code
                StreamIDs_Width[StreamIDs_Size]=2;
                ParserIDs[StreamIDs_Size]=MediaInfo_Parser_MpegPs_Ext;
                StreamIDs_Size++;
                Demux(Buffer+Buffer_Offset+private_stream_1_Offset, (size_t)(Element_Size-private_stream_1_Offset), ContentType_MainStream);
                StreamIDs_Size--;
                Element_Code=StreamIDs[StreamIDs_Size-1];
            }
            else
                Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
        }
    #endif //MEDIAINFO_DEMUX

    //Parsing
    if (Element_Offset<private_stream_1_Offset)
        Skip_XX(private_stream_1_Offset-Element_Offset,         "DVD-Video data");

    #if MEDIAINFO_EVENTS
        StreamIDs[StreamIDs_Size-1]=Element_Code;
        if (private_stream_1_Offset)
        {
            //Multiple substreams in 1 stream
            StreamIDs[StreamIDs_Size]=Element_Code=private_stream_1_ID;
            StreamIDs_Width[StreamIDs_Size]=2;
            ParserIDs[StreamIDs_Size]=MediaInfo_Parser_MpegPs_Ext;
            StreamIDs_Size++;
        }
    #endif //MEDIAINFO_EVENTS
    xxx_stream_Parse(Streams_Private1[private_stream_1_ID], private_stream_1_Count);
    #if MEDIAINFO_EVENTS
        if (private_stream_1_Offset)
        {
            StreamIDs_Size--;
            Element_Code=StreamIDs[StreamIDs_Size-1];
        }
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        if (Config->Demux_EventWasSent)
        {
            Demux_StreamIsBeingParsed_type=1;
            Demux_StreamIsBeingParsed_stream_id=private_stream_1_ID;
        }
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
bool File_MpegPs::private_stream_1_Choose_DVD_ID()
{
    private_stream_1_IsDvdVideo=false;

    if (Element_Size<4)
        return false;

    //Testing false-positives
    if (CC2(Buffer+Buffer_Offset+(size_t)Element_Offset)==0x0B77)
        return true;

    //Parsing
    int8u  CodecID;
    Get_B1 (CodecID,                                            "CodecID");

    //Testing
    //Subtitles (CVD)
         if (CodecID<=0x0F)
    {
        private_stream_1_IsDvdVideo=true;
        private_stream_1_Offset=1;
    }
    //Subtitles (DVD)
    else if (CodecID>=0x20 && CodecID<=0x3F)
    {
        private_stream_1_IsDvdVideo=true;
        private_stream_1_Offset=1;
    }
    //Subtitles (SVCD)
    else if (CodecID>=0x70 && CodecID<=0x7F)
    {
        private_stream_1_IsDvdVideo=true;
        private_stream_1_Offset=1;
    }
    //AC-3 (OTA?)
    else if (CodecID==0x80 && CC3(Buffer+Buffer_Offset+1)==0x000000)
    {
        private_stream_1_IsDvdVideo=true; //Not sure
        private_stream_1_Offset=4;
    }
    //PCM
    else if (CodecID>=0xA0 && CodecID<=0xAF && Element_Size>=7 && Buffer[Buffer_Offset+6]==0x80)
    {
        private_stream_1_IsDvdVideo=true;
        private_stream_1_Offset=1;
    }
    //PS2-MPG
    else if (CodecID==0xFF)
    {
        int16u StreamID;
        int8u  SubID;
        Get_B1 (SubID,                                          "CodecID (part 2)");
        Get_B2 (StreamID,                                       "Stream ID");

             if ((SubID&0xFE)==0xA0) //0xFFA0 or 0xFFA1
        {
            //PS2-MPG PCM/ADPCM
            private_stream_1_Offset=4;
            private_stream_1_ID=(int8u)StreamID; //ID is maybe 2 byte long, but private_stream_1_ID is an int8u
            return true;
        }
        else if (SubID==0x90) //0xFF90
        {
            //PS2-MPG AC-3 or subtitles
            private_stream_1_Offset=4;
            private_stream_1_ID=(int8u)StreamID; //ID is maybe 2 byte long, but private_stream_1_ID is an int8u
            return true;
        }
        else
            return false;
    }
    else
    {
        int16u Next;
        int8u  Count;
        Get_B1 (Count,                                          "Count of next frame headers");
        Get_B2 (Next,                                           "Next frame offset minus 1");

        if (Count>0 && 4+(int64u)Next+4<=Element_Size)
        {
            //Subtitles (CVD)
            //     if (CodecID>=0x00 && CodecID<=0x0F)
            //    ; //Seems to not work with subtitles, to be confirmed
            //Subtitles (DVD)
            //     if (CodecID>=0x20 && CodecID<=0x3F)
            //    ; //Seems to not work with subtitles, to be confirmed
            //Subtitles (SVCD)
            //     if (CodecID>=0x70 && CodecID<=0x7F)
            //    ; //Seems to not work with subtitles, to be confirmed
            //AC3
                if (CodecID>=0x80 && CodecID<=0x87)
            {
                if (CC2(Buffer+Buffer_Offset+4+Next)!=0x0B77 && CC2(Buffer+Buffer_Offset+3+Next)!=0x0B77 && CC2(Buffer+Buffer_Offset+2+Next)!=0x0B77)
                    return false;
            }
            //DTS
            else if (CodecID>=0x88 && CodecID<=0x8F)
            {
                if (CC4(Buffer+Buffer_Offset+4+Next)!=0x7FFE8001 && CC4(Buffer+Buffer_Offset+3+Next)!=0x7FFE8001 && CC4(Buffer+Buffer_Offset+2+Next)!=0x7FFE8001)
                    return false;
            }
            //DTS
            else if (CodecID>=0x98 && CodecID<=0x9F)
            {
                if (CC4(Buffer+Buffer_Offset+4+Next)!=0x7FFE8001 && CC4(Buffer+Buffer_Offset+3+Next)!=0x7FFE8001 && CC4(Buffer+Buffer_Offset+2+Next)!=0x7FFE8001)
                    return false;
            }
            //PCM
            //else if (CodecID>=0xA0 && CodecID<=0xAF)
            //    ;
            //MLP
            else if (CodecID>=0xB0 && CodecID<=0xBF)
            {
                if (CC2(Buffer+Buffer_Offset+4+Next)!=0x0B77 && CC2(Buffer+Buffer_Offset+3+Next)!=0x0B77 && CC2(Buffer+Buffer_Offset+2+Next)!=0x0B77)
                    return false;
            }
            //AC3+
            else if (CodecID>=0xC0 && CodecID<=0xCF)
            {
                if (CC2(Buffer+Buffer_Offset+4+Next)!=0x0B77 && CC2(Buffer+Buffer_Offset+3+Next)!=0x0B77 && CC2(Buffer+Buffer_Offset+2+Next)!=0x0B77)
                    return false;
            }
            else
                return false;

            private_stream_1_IsDvdVideo=true;
            private_stream_1_Offset=4;
        }
    }

    //Filling
    private_stream_1_ID=CodecID;
    return true;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::private_stream_1_ChooseParser()
{
    if (FromTS || Streams[stream_id].program_format_identifier || Streams[stream_id].format_identifier || Streams[stream_id].descriptor_tag)
    {
        int32u format_identifier=FromTS?FromTS_format_identifier:Streams[stream_id].format_identifier;
        if (format_identifier==0x42535344) //"BSSD"
        {
            return ChooseParser_SmpteSt0302(); //AES3 (SMPTE 302M)
        }
        int32u stream_type=FromTS?FromTS_stream_type:Streams[stream_id].stream_type;
        switch (stream_type)
        {
            case 0x03 :
            case 0x04 : return ChooseParser_Mpega(); //MPEG Audio
            case 0x0F : return ChooseParser_Adts(); //ADTS
            case 0x11 : return ChooseParser_Latm(); //LATM
            case 0x80 : return ChooseParser_PCM(); //PCM
            case 0x81 :
            case 0x83 :
            case 0x84 :
            case 0x87 :
            case 0xA1 : return ChooseParser_AC3(); //AC3/AC3+
            case 0x82 :
            case 0x85 :
            case 0x86 :
            case 0xA2 : return ChooseParser_DTS(); //DTS
            case 0x90 : return ChooseParser_PGS(); //PGS from Bluray
            case 0xEA : return ChooseParser_NULL(); //VC1()
            default   :
                        {
                        int8u descriptor_tag=FromTS?FromTS_descriptor_tag:Streams[stream_id].descriptor_tag;
                        switch (descriptor_tag)
                        {
                            case 0x56 : return ChooseParser_Teletext(); //Teletext
                            case 0x59 : return ChooseParser_DvbSubtitle(); //DVB Subtiles
                            case 0x6A :
                            case 0x7A :
                            case 0x81 : return ChooseParser_AC3(); //AC3/AC3+
                            case 0x7B : return ChooseParser_DTS(); //DTS
                            case 0x7C : return ChooseParser_AAC(); //AAC
                            default   :      if (Element_Size>2 && CC2(Buffer+Buffer_Offset)==0x0B77)
                                            return ChooseParser_AC3(); //AC3/AC3+
                                        else if (Element_Size>4 && CC4(Buffer+Buffer_Offset)==0x7FFE8001)
                                            return ChooseParser_DTS(); //DTS
                                        else
                                            return NULL;
                        }
                        }
        }
    }
    else if (Element_Code==0xBD && private_stream_1_IsDvdVideo)
    {
        //Subtitles (CVD)
             if (private_stream_1_ID<=0x0F)
            return ChooseParser_RLE();
        //Subtitles (DVD)
             if (private_stream_1_ID>=0x20 && private_stream_1_ID<=0x3F)
            return ChooseParser_RLE();
        //Subtitles (SVCD)
             if (private_stream_1_ID>=0x70 && private_stream_1_ID<=0x7F)
            return ChooseParser_RLE();
        //AC3
        else if (private_stream_1_ID>=0x80 && private_stream_1_ID<=0x87)
            return ChooseParser_AC3();
        //DTS
        else if (private_stream_1_ID>=0x88 && private_stream_1_ID<=0x8F)
            return ChooseParser_DTS();
        //SDDS
        else if (private_stream_1_ID>=0x90 && private_stream_1_ID<=0x97)
            return ChooseParser_DTS();
        //DTS
        else if (private_stream_1_ID>=0x98 && private_stream_1_ID<=0x9F)
            return ChooseParser_DTS();
        //PCM
        else if (private_stream_1_ID>=0xA0 && private_stream_1_ID<=0xAF)
            return ChooseParser_PCM();
        //AC3+
        else if (private_stream_1_ID>=0xC0 && private_stream_1_ID<=0xCF)
            return ChooseParser_AC3();
        else
            return NULL;
    }
    else
    {
             if (Element_Size>2 && CC2(Buffer+Buffer_Offset)==0x0B77)
            return ChooseParser_AC3(); //AC3/AC3+
        else if (Element_Size>4 && CC4(Buffer+Buffer_Offset)==0x7FFE8001)
            return ChooseParser_DTS(); //DTS
        else if (Element_Size>2 && (CC2(Buffer+Buffer_Offset)&0xFFFE)==0xFFA0) //0xFFA0 or 0xFFA1
            return ChooseParser_PS2(); //PS2-MPG PCM/ADPCM
        else if (Element_Size>6 && CC2(Buffer+Buffer_Offset)==0xFF90 && CC2(Buffer+Buffer_Offset+4)==0x0B77)
            return ChooseParser_AC3(); //PS2-MPG AC-3
        else if (Element_Size>6 && CC2(Buffer+Buffer_Offset)==0xFF90 && CC2(Buffer+Buffer_Offset+4)==0x0000)
            return ChooseParser_RLE(); //PS2-MPG Subtitles
        else
            return NULL;
    }
}

//---------------------------------------------------------------------------
const ZenLib::Char* File_MpegPs::private_stream_1_ChooseExtension()
{
    if (FromTS)
    {
        switch (private_stream_1_ID)
        {
            case 0x80 : return __T(".pcm"); //PCM
            case 0x81 : return __T(".ac3"); //AC3
            case 0x83 :
            case 0x87 : return __T(".dd+"); //AC3+
            case 0x86 : return __T(".dts"); //DTS
            case 0xEA : return __T(".vc1"); //DTS
            default   : return __T(".raw");
        }
    }
    else
    {
        //Subtitles
             if (private_stream_1_ID>=0x20 && private_stream_1_ID<=0x3F)
            return __T(".sub");
        //AC3
        else if (private_stream_1_ID>=0x80 && private_stream_1_ID<=0x87)
            return __T(".ac3");
        //DTS
        else if (private_stream_1_ID>=0x88 && private_stream_1_ID<=0x8F)
            return __T(".dts");
        //SDDS
        else if (private_stream_1_ID>=0x90 && private_stream_1_ID<=0x97)
            return __T(".sdds");
        //DTS
        else if (private_stream_1_ID>=0x98 && private_stream_1_ID<=0x9F)
            return __T(".dts");
        //PCM
        else if (private_stream_1_ID>=0xA0 && private_stream_1_ID<=0xAF)
            return __T(".pcm");
        //MLP
        else if (private_stream_1_ID>=0xB0 && private_stream_1_ID<=0xBF)
            return __T(".dd+");
        //AC3+
        else if (private_stream_1_ID>=0xC0 && private_stream_1_ID<=0xCF)
            return __T(".dd+");
        else
            return __T(".raw");
    }
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File_MpegPs::private_stream_1_Element_Info1()
{
    if (FromTS)
    {
        switch (private_stream_1_ID)
        {
            case 0x80 : Element_Info1("PCM"); return;
            case 0x81 : Element_Info1("AC3"); return;
            case 0x83 :
            case 0x87 : Element_Info1("AC3+"); return;
            case 0x86 : Element_Info1("DTS"); return;
            case 0xEA : Element_Info1("VC1"); return;
            default   : return;
        }
    }
    else
    {
        //Subtitles
             if (private_stream_1_ID>=0x20 && private_stream_1_ID<=0x3F)
            Element_Info1("RLE");
        //AC3
        else if (private_stream_1_ID>=0x80 && private_stream_1_ID<=0x87)
            Element_Info1("AC3");
        //DTS
        else if (private_stream_1_ID>=0x88 && private_stream_1_ID<=0x8F)
            Element_Info1("DTS");
        //SDDS
        else if (private_stream_1_ID>=0x90 && private_stream_1_ID<=0x97)
            Element_Info1("SDDS");
        //DTS
        else if (private_stream_1_ID>=0x98 && private_stream_1_ID<=0x9F)
            Element_Info1("DTS");
        //PCM
        else if (private_stream_1_ID>=0xA0 && private_stream_1_ID<=0xAF)
            Element_Info1("LPCM");
        //MLP
        else if (private_stream_1_ID>=0xB0 && private_stream_1_ID<=0xBF)
            Element_Info1("MLP");
        //AC3+
        else if (private_stream_1_ID>=0xC0 && private_stream_1_ID<=0xCF)
            Element_Info1("AC3+");
    }
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
// Packet "BE"
void File_MpegPs::padding_stream()
{
    Element_Name("padding_stream");

    Skip_XX(Element_Size,                                       "stuffing_bytes");
}

//---------------------------------------------------------------------------
// Packet "BF"
void File_MpegPs::private_stream_2()
{
    Element_Name("private_stream_2");

    //Filling
    if (FromTS)
    {
        switch (FromTS_program_format_identifier)
        {
            case 0x54534856 : //TSHV
                                switch (FromTS_stream_type)
                                {
                                    case 0xA0 : private_stream_2_TSHV_A0(); break;
                                    case 0xA1 : private_stream_2_TSHV_A1(); break;
                                    default   : Skip_XX(Element_Size, "Unknown");
                                }
            default         : Skip_XX(Element_Size,             "Unknown");
        }

        //Disabling the program
        if (!Status[IsAccepted])
            Data_Accept("MPEG-PS");
    }
    else //DVD?
    {
        Stream_Prepare(Stream_Menu);
        Fill(Stream_Menu, StreamPos_Last, Menu_Format, "DVD-Video");
        Fill(Stream_Menu, StreamPos_Last, Menu_Codec, "DVD-Video");

        //Disabling this Stream
        Streams[0xBF].Searching_Payload=false;
        private_stream_2_Count=0;
    }
}

//---------------------------------------------------------------------------
void File_MpegPs::private_stream_2_TSHV_A0()
{
    Element_Name("DV A0");

    //Parsing
    Skip_XX(Element_Size,                                       "Unknown");

    //Filling
    Data_Accept("MPEG-PS");
    Finish("MPEG-PS");
}

//---------------------------------------------------------------------------
void File_MpegPs::private_stream_2_TSHV_A1()
{
    Element_Name("DV A1");

    //Parsing
    int8u day, month, year, second, minute, hour;
    Skip_XX(31,                                                 "Unknown");
    BS_Begin();
    Skip_S1(2,                                                  "Unknown");
    Skip_S1(6,                                                  "timecode_frame");
    Skip_S1(1,                                                  "Unknown");
    Skip_S1(7,                                                  "timecode_second");
    Skip_S1(1,                                                  "Unknown");
    Skip_S1(7,                                                  "timecode_minute");
    Skip_S1(2,                                                  "Unknown");
    Skip_S1(6,                                                  "timecode_hour");
    Skip_S1(8,                                                  "Unknown");
    Skip_S1(2,                                                  "Unknown");
    Get_S1 (6, day,                                             "day");
    Skip_S1(3,                                                  "Unknown");
    Get_S1 (5, month,                                           "month");
    Get_S1 (8, year,                                            "year");
    Skip_S1(8,                                                  "Unknown");
    Skip_S1(1,                                                  "Unknown");
    Get_S1 (7, second,                                          "second");
    Skip_S1(1,                                                  "Unknown");
    Get_S1 (7, minute,                                          "minute");
    Skip_S1(2,                                                  "Unknown");
    Get_S1 (6, hour,                                            "hour");
    Skip_S1(2,                                                  "Unknown");
    Skip_S1(1,                                                  "scene_start");
    Skip_S1(5,                                                  "Unknown");
    BS_End();
    Skip_XX(Element_Size-Element_Offset,                        "Unknown");

    FILLING_BEGIN();
        Ztring Date_Time=Ztring().Date_From_Numbers(year/0x10*10+year%0x10, month/0x10*10+month%0x10, day/0x10*10+day%0x10, hour/0x10*10+hour%0x10, minute/0x10*10+minute%0x10, second/0x10*10+second%0x10);
        if (Retrieve(Stream_General, 0, General_Encoded_Date).empty())
        {
            Fill(Stream_General, 0, General_Encoded_Date, Date_Time);
            Fill(Stream_General, 0, General_Duration_Start, Date_Time);
        }
        Fill(Stream_General, 0, General_Duration_End, Date_Time, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_MpegPs::audio_stream()
{
    Element_Name("Audio");

    if (!Streams[stream_id].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            if (video_stream_Count==(int8u)-1 && audio_stream_Count==(int8u)-1)
            {
                video_stream_Count=0;
                audio_stream_Count=1;
                private_stream_1_Count=0;
                private_stream_2_Count=0;
                extension_stream_Count=0;
                SL_packetized_stream_Count=0;
                Streams[stream_id].stream_type=FromTS_stream_type;
            }
            else if (!IsSub)
            {
                //2 streams in the file, this can not be From TS, we have no idea of the count of streams
                video_stream_Count=(int8u)-1;
                audio_stream_Count=(int8u)-1;
                private_stream_1_Count=(int8u)-1;
                private_stream_2_Count=(int8u)-1;
                extension_stream_Count=(int8u)-1;
                SL_packetized_stream_Count=(int8u)-1;
                FromTS=false;
            }
        }

        //If we have no Streams map --> Registering the Streams as MPEG Audio
        if (Streams[stream_id].stream_type==0 && !FromTS)
        {
            if (MPEG_Version==2)
                Streams[stream_id].stream_type=0x04; //MPEG-2 Audio
            else
                Streams[stream_id].stream_type=0x03; //MPEG-1 Audio
        }

        //Registering
        if (!Status[IsAccepted])
        {
            Data_Accept("MPEG-PS");
            if (!IsSub)
                Fill(Stream_General, 0, General_Format, "MPEG-PS");
        }
        Streams[stream_id].StreamIsRegistred++;
        Streams[stream_id].FirstPacketOrder=FirstPacketOrder_Last;
        FirstPacketOrder_Last++;

        //New parsers
        Streams[stream_id].Parsers.push_back(private_stream_1_ChooseParser());
        if (Streams[stream_id].Parsers[Streams[stream_id].Parsers.size()-1]==NULL)
        {
            Streams[stream_id].Parsers.clear();
            #if defined(MEDIAINFO_MPEGA_YES)
                Streams[stream_id].Parsers.push_back(ChooseParser_Mpega());
            #endif
            #if defined(MEDIAINFO_AC3_YES)
                Streams[stream_id].Parsers.push_back(ChooseParser_AC3());
            #endif
            #if defined(MEDIAINFO_DTS_YES)
                Streams[stream_id].Parsers.push_back(ChooseParser_DTS());
            #endif
            #if defined(MEDIAINFO_AAC_YES)
                Streams[stream_id].Parsers.push_back(ChooseParser_Adts());
            #endif
            #if defined(MEDIAINFO_AAC_YES)
                Streams[stream_id].Parsers.push_back(ChooseParser_Latm());
            #endif
        }
        for (size_t Pos=0; Pos<Streams[stream_id].Parsers.size(); Pos++)
        {
            Streams[stream_id].Parsers[Pos]->CA_system_ID_MustSkipSlices=CA_system_ID_MustSkipSlices;
            Open_Buffer_Init(Streams[stream_id].Parsers[Pos]);
        }
    }

    //Demux
    #if MEDIAINFO_DEMUX
        if (Streams[stream_id].Parsers.empty() || !Streams[stream_id].Parsers[0]->Demux_UnpacketizeContainer)
            Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
    #endif //MEDIAINFO_DEMUX

    //Parsing
    #if MEDIAINFO_EVENTS
        StreamIDs[StreamIDs_Size-1]=Element_Code;
    #endif //MEDIAINFO_EVENTS
    xxx_stream_Parse(Streams[stream_id], audio_stream_Count);
    #if MEDIAINFO_DEMUX
        if (Config->Demux_EventWasSent)
        {
            Demux_StreamIsBeingParsed_type=0;
            Demux_StreamIsBeingParsed_stream_id=stream_id;
        }
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
void File_MpegPs::video_stream()
{
    Element_Name("Video");

    if (!Streams[stream_id].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            if (video_stream_Count==(int8u)-1 && audio_stream_Count==(int8u)-1)
            {
                video_stream_Count=1;
                audio_stream_Count=0;
                private_stream_1_Count=0;
                private_stream_2_Count=0;
                extension_stream_Count=0;
                SL_packetized_stream_Count=0;
                Streams[stream_id].stream_type=FromTS_stream_type;
            }
            else if (!IsSub)
            {
                //2 streams in the file, this can not be From TS, we have no idea of the count of streams
                video_stream_Count=(int8u)-1;
                audio_stream_Count=(int8u)-1;
                private_stream_1_Count=(int8u)-1;
                private_stream_2_Count=(int8u)-1;
                extension_stream_Count=(int8u)-1;
                SL_packetized_stream_Count=(int8u)-1;
                FromTS=false;
            }
        }

        //Registering
        if (!Status[IsAccepted])
        {
            Data_Accept("MPEG-PS");
            if (!IsSub)
                Fill(Stream_General, 0, General_Format, "MPEG-PS");
        }
        Streams[stream_id].StreamIsRegistred++;
        Streams[stream_id].FirstPacketOrder=FirstPacketOrder_Last;
        FirstPacketOrder_Last++;

        //New parsers
        switch (Streams[stream_id].stream_type)
        {
            case 0x10 : Streams[stream_id].Parsers.push_back(ChooseParser_Mpeg4v()); break;
            case 0x1B : Streams[stream_id].Parsers.push_back(ChooseParser_Avc()   ); break;
            case 0x27 : Streams[stream_id].Parsers.push_back(ChooseParser_Hevc()  ); break;
            case 0x01 :
            case 0x02 :
            case 0x80 : Streams[stream_id].Parsers.push_back(ChooseParser_Mpegv() ); break;
            default   :
                        #if defined(MEDIAINFO_MPEGV_YES)
                            Streams[stream_id].Parsers.push_back(ChooseParser_Mpegv());
                        #endif
                        #if defined(MEDIAINFO_AVC_YES)
                            Streams[stream_id].Parsers.push_back(ChooseParser_Avc());
                        #endif
                        #if defined(MEDIAINFO_HEVC_YES)
                            Streams[stream_id].Parsers.push_back(ChooseParser_Hevc());
                        #endif
                        #if defined(MEDIAINFO_MPEG4V_YES)
                            Streams[stream_id].Parsers.push_back(ChooseParser_Mpeg4v());
                        #endif
                        #if defined(MEDIAINFO_AVSV_YES)
                        {
                            File_AvsV* Parser=new File_AvsV;
                            Streams[stream_id].Parsers.push_back(Parser);
                        }
                        #endif
        }
        for (size_t Pos=0; Pos<Streams[stream_id].Parsers.size(); Pos++)
        {
            Streams[stream_id].Parsers[Pos]->CA_system_ID_MustSkipSlices=CA_system_ID_MustSkipSlices;
            Open_Buffer_Init(Streams[stream_id].Parsers[Pos]);
            #if MEDIAINFO_IBI
                if (FromTS)
                    Streams[stream_id].Parsers[Pos]->IbiStream=IbiStream;
                else
                {
                    if (Ibi.Streams[stream_id]==NULL)
                        Ibi.Streams[stream_id]=new ibi::stream;
                    Streams[stream_id].Parsers[Pos]->IbiStream=Ibi.Streams[stream_id];
                }
            #endif //MEDIAINFO_IBI
            #if MEDIAINFO_SEEK
                if (Unsynch_Frame_Counts.find(stream_id)!=Unsynch_Frame_Counts.end())
                    Streams[stream_id].Parsers[Pos]->Frame_Count_NotParsedIncluded=Unsynch_Frame_Counts[stream_id];
            #endif //MEDIAINFO_SEEK
        }
        #if MEDIAINFO_SEEK
            if (Unsynch_Frame_Counts.find(stream_id)!=Unsynch_Frame_Counts.end())
                Unsynch_Frame_Counts.erase(stream_id);
        #endif //MEDIAINFO_SEEK
    }

    //Demux
    #if MEDIAINFO_DEMUX
        if (!(FromTS_stream_type==0x20 && SubStream_Demux) && (Streams[stream_id].Parsers.empty() || !Streams[stream_id].Parsers[0]->Demux_UnpacketizeContainer))
            Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
    #endif //MEDIAINFO_DEMUX

    //Parsing
    #if MEDIAINFO_EVENTS
        StreamIDs[StreamIDs_Size-1]=Element_Code;
    #endif //MEDIAINFO_EVENTS
    xxx_stream_Parse(Streams[stream_id], video_stream_Count);
    #if MEDIAINFO_DEMUX
        if (Config->Demux_EventWasSent)
        {
            Demux_StreamIsBeingParsed_type=0;
            Demux_StreamIsBeingParsed_stream_id=stream_id;
        }
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
// Packet "FA"
void File_MpegPs::SL_packetized_stream()
{
    Element_Name("SL-packetized_stream");

    if (!Streams[stream_id].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            if (video_stream_Count==(int8u)-1 && audio_stream_Count==(int8u)-1)
            {
                video_stream_Count=0;
                audio_stream_Count=0;
                private_stream_1_Count=0;
                private_stream_2_Count=0;
                extension_stream_Count=0;
                SL_packetized_stream_Count=1;
                Streams[stream_id].stream_type=FromTS_stream_type;
            }
            else if (!IsSub)
            {
                //2 streams in the file, this can not be From TS, we have no idea of the count of streams
                video_stream_Count=(int8u)-1;
                audio_stream_Count=(int8u)-1;
                private_stream_1_Count=(int8u)-1;
                private_stream_2_Count=(int8u)-1;
                extension_stream_Count=(int8u)-1;
                SL_packetized_stream_Count=(int8u)-1;
                FromTS=false;
            }
        }

        //Registering
        Streams[stream_id].StreamIsRegistred++;
        Streams[stream_id].FirstPacketOrder=FirstPacketOrder_Last;
        FirstPacketOrder_Last++;
        if (!Status[IsAccepted])
            Data_Accept("MPEG-PS");
        Streams[stream_id].Searching_TimeStamp_Start=true;

        //New parsers
        #ifdef MEDIAINFO_MPEG4_YES
            if (ParserFromTs)
            {
                Streams[stream_id].Parsers.push_back(ParserFromTs); ParserFromTs=NULL;
            }
            else
        #endif
        if (FromTS_stream_type)
            switch (FromTS_stream_type)
            {
                case 0x0F :
                            Streams[stream_id].Parsers.push_back(ChooseParser_Adts());
                            break;

                case 0x11 :
                            Streams[stream_id].Parsers.push_back(ChooseParser_Latm());
                            break;
                default   : ;
            }
        else
        {
            #if defined(MEDIAINFO_AAC_YES)
                Streams[stream_id].Parsers.push_back(ChooseParser_Adts());
            #endif
            #if defined(MEDIAINFO_AAC_YES)
                Streams[stream_id].Parsers.push_back(ChooseParser_Latm());
            #endif
        }
        for (size_t Pos=0; Pos<Streams[stream_id].Parsers.size(); Pos++)
        {
            Streams[stream_id].Parsers[Pos]->CA_system_ID_MustSkipSlices=CA_system_ID_MustSkipSlices;
            Open_Buffer_Init(Streams[stream_id].Parsers[Pos]);
        }
    }

    //Parsing
    #ifdef MEDIAINFO_MPEG4_YES
        if (SLConfig) //SL
        {
            BS_Begin();
            int8u paddingBits=0;
            bool paddingFlag=false, idleFlag=false, OCRflag=false, accessUnitStartFlag=false;
            if (SLConfig->useAccessUnitStartFlag)
                Get_SB (accessUnitStartFlag,                        "accessUnitStartFlag");
            if (SLConfig->useAccessUnitEndFlag)
                Skip_SB(                                            "accessUnitEndFlag");
            if (SLConfig->OCRLength>0)
                Get_SB (OCRflag,                                    "OCRflag");
            if (SLConfig->useIdleFlag)
                Get_SB (idleFlag,                                   "idleFlag");
            if (SLConfig->usePaddingFlag)
                Get_SB (paddingFlag,                                "paddingFlag");
            if (paddingFlag)
                Get_S1(3, paddingBits,                              "paddingBits");
            if (!idleFlag && (!paddingFlag || paddingBits!=0))
            {
                bool DegPrioflag=false;
                if (SLConfig->packetSeqNumLength>0)
                    Skip_S2(SLConfig->packetSeqNumLength,           "packetSequenceNumber");
                if (SLConfig->degradationPriorityLength>0)
                    Get_SB (DegPrioflag,                            "DegPrioflag");
                if (DegPrioflag)
                    Skip_S2(SLConfig->degradationPriorityLength,    "degradationPriority");
                if (OCRflag)
                    Skip_S8(SLConfig->OCRLength,                    "objectClockReference");
                if (accessUnitStartFlag)
                {
                    bool decodingTimeStampFlag=false, compositionTimeStampFlag=false, instantBitrateFlag=false;
                    if (SLConfig->useRandomAccessPointFlag)
                        Skip_SB(                                    "randomAccessPointFlag");
                    if (SLConfig->AU_seqNumLength >0)
                        Skip_S2(SLConfig->AU_seqNumLength,          "AU_sequenceNumber");
                    if (SLConfig->useTimeStampsFlag)
                    {
                        Get_SB (decodingTimeStampFlag,              "decodingTimeStampFlag");
                        Get_SB (compositionTimeStampFlag,           "compositionTimeStampFlag");
                    }
                    if (SLConfig->instantBitrateLength>0)
                        Get_SB (instantBitrateFlag,                 "instantBitrateFlag");
                    if (decodingTimeStampFlag)
                        Skip_S2(SLConfig->timeStampLength,          "decodingTimeStamp");
                    if (compositionTimeStampFlag)
                        Skip_S2(SLConfig->timeStampLength,          "compositionTimeStamp");
                    if (SLConfig->AU_Length > 0)
                        Skip_S2(SLConfig->AU_Length,                "accessUnitLength");
                    if (instantBitrateFlag)
                        Skip_S2(SLConfig->instantBitrateLength,     "instantBitrate");
                }
            }
            BS_End();
        }
    #else //MEDIAINFO_MPEG4_YES
        Skip_XX(Element_Size,                                       "LATM (not decoded)");
    #endif //MEDIAINFO_MPEG4_YES

    //Demux
    /*
    if (Config_Demux)
    {
        int8u A[7];
        //TODO: Only for 24KHz stuff, should be modified... output is ADTS
        A[0]=0xFF;
        A[1]=0xF9;
        A[2]=0x58;
        A[3]=0x80;
        A[4]=0x00;
        A[5]=0x1F;
        A[6]=0xFC;

        int32u Size=(int32u)(Element_Size+7);
        Size=Size<<13;
        A[3]=A[3]|((int8u)(Size>>24));
        A[4]=A[4]|((int8u)(Size>>16));
        A[5]=A[5]|((int8u)(Size>>8));

        //Demux
        Demux(A, 7, ContentType_Header);
        Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
    }
    */
    #if MEDIAINFO_DEMUX
        Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
    #endif //MEDIAINFO_DEMUX

    //Parsing
    #if MEDIAINFO_EVENTS
        StreamIDs[StreamIDs_Size-1]=Element_Code;
    #endif //MEDIAINFO_EVENTS
    xxx_stream_Parse(Streams[stream_id], SL_packetized_stream_Count);
    #if MEDIAINFO_DEMUX
        if (Config->Demux_EventWasSent)
        {
            Demux_StreamIsBeingParsed_type=0;
            Demux_StreamIsBeingParsed_stream_id=stream_id;
        }
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
// Packet "FD"
void File_MpegPs::extension_stream()
{
    Element_Name("With Extension");
    Element_Info1(MpegPs_stream_id_extension(stream_id_extension));

    if (!Streams_Extension[stream_id_extension].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            if (video_stream_Count==(int8u)-1 && audio_stream_Count==(int8u)-1)
            {
                video_stream_Count=0;
                audio_stream_Count=0;
                private_stream_1_Count=0;
                private_stream_2_Count=0;
                extension_stream_Count=1;
                SL_packetized_stream_Count=0;
                Streams_Extension[stream_id_extension].stream_type=FromTS_stream_type;
            }
            else if (!IsSub)
            {
                //2 streams in the file, this can not be From TS, we have no idea of the count of streams
                video_stream_Count=(int8u)-1;
                audio_stream_Count=(int8u)-1;
                private_stream_1_Count=(int8u)-1;
                private_stream_2_Count=(int8u)-1;
                extension_stream_Count=(int8u)-1;
                SL_packetized_stream_Count=(int8u)-1;
                FromTS=false;
            }
        }

        //Registering
        if (!Status[IsAccepted])
            Data_Accept("MPEG-PS");
        Streams[stream_id].StreamIsRegistred++;
        Streams_Extension[stream_id_extension].StreamIsRegistred++;
        Streams_Extension[stream_id_extension].Searching_Payload=true;
        Streams_Extension[stream_id_extension].Searching_TimeStamp_Start=true;
        Streams_Extension[stream_id_extension].Searching_TimeStamp_End=true;
        Streams_Extension[stream_id_extension].FirstPacketOrder=FirstPacketOrder_Last;
        FirstPacketOrder_Last++;

        //New parsers
        if (Streams_Extension[stream_id_extension].stream_type && Streams_Extension[stream_id_extension].stream_type<0x80) //Standard
            switch (Streams_Extension[stream_id_extension].stream_type)
            {
                case 0x0F : Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_Adts()); break;
                default   : ;
            }
        else
            switch (FromTS_format_identifier)
            {
                case 0x41432D33 :
                                    Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_AC3());
                                    break;
                case 0x44545331 :
                case 0x44545332 :
                case 0x44545333 :
                                    Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_DTS());
                                    break;
                case 0x56432D31 :
                                    Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_VC1());
                                    break;
                case 0x64726163 :
                                    Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_Dirac());
                                    break;
                default           :
                                    switch (FromTS_program_format_identifier)
                                    {
                                        case 0x48444D56 :   //HDMV (BluRay)
                                                            switch (Streams_Extension[stream_id_extension].stream_type)
                                                            {
                                                                case 0x81 :
                                                                case 0x83 :
                                                                case 0x84 :
                                                                case 0xA1 :
                                                                            Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_AC3());
                                                                            break;
                                                                case 0x82 :
                                                                case 0x85 :
                                                                case 0x86 :
                                                                case 0xA2 :
                                                                            Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_DTS());
                                                                            break;
                                                                case 0xEA :
                                                                            Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_VC1());
                                                                            break;
                                                                default   : ;
                                                            }
                                                            break;
                                        default           : ;
                                                                 if (stream_id_extension==0x00)
                                                                {} //IPMP Control Information stream
                                                            else if (stream_id_extension==0x01)
                                                                {} //IPMP stream
                                                            else if (stream_id_extension>=0x55 && stream_id_extension<=0x5F)
                                                                 Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_VC1());
                                                            else if (stream_id_extension>=0x60 && stream_id_extension<=0x6F)
                                                                 Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_Dirac());
                                                            else if (stream_id_extension==0x71 || stream_id_extension==0x72 || stream_id_extension==0x76)
                                                            {
                                                                Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_DTS());
                                                                Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_AC3());
                                                            }
                                                            else if (stream_id_extension==0x75 && stream_id_extension<=0x7F)
                                                                 Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_VC1());
                                                      }
            }

        if (Streams_Extension[stream_id_extension].Parsers.empty())
        {
            #if defined(MEDIAINFO_DIRAC_YES)
                Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_Dirac());
            #endif
            #if defined(MEDIAINFO_VC1_YES)
                Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_VC1());
            #endif
            #if defined(MEDIAINFO_AC3_YES)
                Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_AC3());
            #endif
            #if defined(MEDIAINFO_DTS_YES)
                Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_DTS());
            #endif
        }

        //In case of HD part before Core part
        switch (stream_id_extension)
        {
            case 0x71 :
            case 0x76 :
                        for (size_t Pos=0; Pos<Streams_Extension[0x72].Parsers.size(); Pos++)
                            delete Streams_Extension[0x72].Parsers[Pos]; //Streams_Extension[0x72].Parsers[Pos]=NULL;
                        Streams_Extension[0x72].Parsers.clear();
                        break;
        }

        //Init
        for (size_t Pos=0; Pos<Streams_Extension[stream_id_extension].Parsers.size(); Pos++)
        {
            Streams_Extension[stream_id_extension].Parsers[Pos]->CA_system_ID_MustSkipSlices=CA_system_ID_MustSkipSlices;
            Open_Buffer_Init(Streams_Extension[stream_id_extension].Parsers[Pos]);
        }
    }

    //Demux
    #if MEDIAINFO_DEMUX
        if (Streams_Extension[stream_id_extension].Searching_Payload)
        {
            StreamIDs[StreamIDs_Size-1]=Element_Code;
            if (stream_id_extension==0x72 && !(Streams_Extension[0x71].Parsers.empty() && Streams_Extension[0x76].Parsers.empty()))
            {
                if (!Streams_Extension[0x71].Parsers.empty())
                    Element_Code=0x71;
                if (!Streams_Extension[0x76].Parsers.empty())
                    Element_Code=0x76;
            }
            else
                Element_Code=stream_id_extension; //The upper level ID is filled by Element_Code in the common code
            StreamIDs_Width[StreamIDs_Size]=2;
            ParserIDs[StreamIDs_Size]=MediaInfo_Parser_MpegPs_Ext;
            StreamIDs_Size++;
            if (stream_id_extension==0x72 && !(Streams_Extension[0x71].Parsers.empty() && Streams_Extension[0x76].Parsers.empty()))
                Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_SubStream);
            else
                Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
            StreamIDs_Size--;
            Element_Code=StreamIDs[StreamIDs_Size-1];
        }
    #endif //MEDIAINFO_DEMUX

    //Parsing
    if (stream_id_extension==0x72 && !(Streams_Extension[0x71].Parsers.empty() && Streams_Extension[0x76].Parsers.empty()))
    {
        if (!Streams_Extension[0x71].Parsers.empty())
        {
            #if MEDIAINFO_EVENTS
                //Multiple substreams in 1 stream
                StreamIDs[StreamIDs_Size-1]=Element_Code;
                StreamIDs[StreamIDs_Size]=Element_Code=0x71;
                StreamIDs_Width[StreamIDs_Size]=2;
                ParserIDs[StreamIDs_Size]=MediaInfo_Parser_MpegPs_Ext;
                StreamIDs_Size++;
            #endif //MEDIAINFO_EVENTS
            xxx_stream_Parse(Streams_Extension[0x71], extension_stream_Count);
            #if MEDIAINFO_EVENTS
                StreamIDs_Size--;
                Element_Code=StreamIDs[StreamIDs_Size-1];
            #endif //MEDIAINFO_EVENTS
            #if MEDIAINFO_DEMUX
                if (Config->Demux_EventWasSent)
                {
                    Demux_StreamIsBeingParsed_type=2;
                    Demux_StreamIsBeingParsed_stream_id=0x71;
                }
            #endif //MEDIAINFO_DEMUX
        }
        if (!Streams_Extension[0x76].Parsers.empty())
        {
            #if MEDIAINFO_EVENTS
                //Multiple substreams in 1 stream
                StreamIDs[StreamIDs_Size-1]=Element_Code;
                StreamIDs[StreamIDs_Size]=Element_Code=0x76;
                StreamIDs_Width[StreamIDs_Size]=2;
                ParserIDs[StreamIDs_Size]=MediaInfo_Parser_MpegPs_Ext;
                StreamIDs_Size++;
            #endif //MEDIAINFO_EVENTS
            xxx_stream_Parse(Streams_Extension[0x76], extension_stream_Count);
            #if MEDIAINFO_EVENTS
                StreamIDs_Size--;
                Element_Code=StreamIDs[StreamIDs_Size-1];
            #endif //MEDIAINFO_EVENTS
            #if MEDIAINFO_DEMUX
                if (Config->Demux_EventWasSent)
                {
                    Demux_StreamIsBeingParsed_type=2;
                    Demux_StreamIsBeingParsed_stream_id=0x76;
                }
            #endif //MEDIAINFO_DEMUX
        }
    }
    else
    {
        #if MEDIAINFO_EVENTS
            //Multiple substreams in 1 stream
            StreamIDs[StreamIDs_Size-1]=Element_Code;
            StreamIDs[StreamIDs_Size]=Element_Code=stream_id_extension;
            StreamIDs_Width[StreamIDs_Size]=2;
            ParserIDs[StreamIDs_Size]=MediaInfo_Parser_MpegPs_Ext;
            StreamIDs_Size++;
        #endif //MEDIAINFO_EVENTS
        xxx_stream_Parse(Streams_Extension[stream_id_extension], extension_stream_Count);
        #if MEDIAINFO_EVENTS
            StreamIDs_Size--;
            Element_Code=StreamIDs[StreamIDs_Size-1];
        #endif //MEDIAINFO_EVENTS
        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
            {
                Demux_StreamIsBeingParsed_type=2;
                Demux_StreamIsBeingParsed_stream_id=stream_id_extension;
            }
        #endif //MEDIAINFO_DEMUX
    }
}

//---------------------------------------------------------------------------
const ZenLib::Char* File_MpegPs::extension_stream_ChooseExtension()
{
    //AC3
        if ((stream_id_extension>=0x55 && stream_id_extension<=0x5F)
         || (stream_id_extension==0x75 && stream_id_extension<=0x7F))
        return __T(".vc1");
    //AC3+
    else if (stream_id_extension>=0x60 && stream_id_extension<=0x6F)
        return __T(".dirac");
    else if (stream_id_extension==0x71)
        return private_stream_1_ChooseExtension();
    else
        return __T(".raw");
}

//***************************************************************************
// xxx_stream helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegPs::xxx_stream_Parse(ps_stream &Temp, int8u &stream_Count)
{
    switch (stream_id)
    {
        case 0xBD :
        //case 0xBF :
        case 0xFD :
            //PTS
            if (Streams[stream_id].TimeStamp_End.PTS.TimeStamp!=(int64u)-1)
            {
                if (Streams[stream_id].Searching_TimeStamp_End)
                {
                    Temp.TimeStamp_End.PTS.File_Pos=File_Offset+Buffer_Offset;
                    Temp.TimeStamp_End.PTS.TimeStamp=Streams[stream_id].TimeStamp_End.PTS.TimeStamp;
                }
                if (Searching_TimeStamp_Start && Temp.Searching_TimeStamp_Start)
                {
                    Temp.TimeStamp_Start.PTS.File_Pos=File_Offset+Buffer_Offset;
                    Temp.TimeStamp_Start.PTS.TimeStamp=Streams[stream_id].TimeStamp_End.PTS.TimeStamp;
                    Temp.Searching_TimeStamp_Start=false;
                }
            }

            //DTS
            if (Streams[stream_id].TimeStamp_End.DTS.TimeStamp!=(int64u)-1)
            {
                if (Streams[stream_id].Searching_TimeStamp_End)
                {
                    Temp.TimeStamp_End.DTS.File_Pos=File_Offset+Buffer_Offset;
                    Temp.TimeStamp_End.DTS.TimeStamp=Streams[stream_id].TimeStamp_End.DTS.TimeStamp;
                }
                if (Searching_TimeStamp_Start && Streams[stream_id].TimeStamp_End.DTS.TimeStamp!=(int64u)-1 && Temp.Searching_TimeStamp_Start)
                {
                    Temp.TimeStamp_Start.DTS.TimeStamp=Streams[stream_id].TimeStamp_End.DTS.TimeStamp;
                    Temp.Searching_TimeStamp_Start=false;
                }
            }
        default : ;
    }

    //Needed?
    if (Temp.Parsers.size()==1 && Temp.Parsers[0]->Status[IsFinished])
    {
        Skip_XX(Element_Size-Element_Offset,                    "data");
        return;
    }

    #if MEDIAINFO_TRACE
        if (stream_id==0xBD /*|| stream_id==0xBF*/)
            private_stream_1_Element_Info1();
    #endif //MEDIAINFO_TRACE

    for (size_t Pos=0; Pos<Temp.Parsers.size(); Pos++)
        if (Temp.Parsers[Pos] && !Temp.Parsers[Pos]->Status[IsFinished])
        {
            //PTS/DTS
            if (Temp.Parsers[Pos]->PTS_DTS_Needed)
            {
                if (FrameInfo.PCR!=(int64u)-1)
                    Temp.Parsers[Pos]->FrameInfo.PCR=FrameInfo.PCR;
                if (FrameInfo.PTS!=(int64u)-1)
                    Temp.Parsers[Pos]->FrameInfo.PTS=FrameInfo.PTS;
                if (FrameInfo.DTS!=(int64u)-1)
                    Temp.Parsers[Pos]->FrameInfo.DTS=FrameInfo.DTS;
            }

            #if MEDIAINFO_TRACE
                if (Temp.Parsers.size()>1)
                    Element_Begin1("Test");
            #endif //MEDIAINFO_TRACE
            #if MEDIAINFO_IBI
                Temp.Parsers[Pos]->Ibi_SynchronizationOffset_Current=Ibi_SynchronizationOffset_Current;
            #endif //MEDIAINFO_IBI
            Open_Buffer_Continue(Temp.Parsers[Pos], Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
            if (IsSub && Temp.Parsers[Pos]->Frame_Count_NotParsedIncluded!=(int64u)-1)
                Frame_Count_NotParsedIncluded=Temp.Parsers[Pos]->Frame_Count_NotParsedIncluded;
            if (!MustExtendParsingDuration && Temp.Parsers[Pos]->MustExtendParsingDuration)
            {
                SizeToAnalyze*=4; //Normally 4 seconds, now 16 seconds
                MustExtendParsingDuration=true;
            }
            #if MEDIAINFO_TRACE
                if (Temp.Parsers.size()>1)
                    Element_End0();
            #endif //MEDIAINFO_TRACE

            if (Temp.Parsers.size()>1)
            {
                if (!Temp.Parsers[Pos]->Status[IsAccepted] && Temp.Parsers[Pos]->Status[IsFinished])
                {
                    delete *(Temp.Parsers.begin()+Pos);
                    Temp.Parsers.erase(Temp.Parsers.begin()+Pos);
                    Pos--;
                }
                else if (Temp.Parsers.size()>1 && Temp.Parsers[Pos]->Status[IsAccepted])
                {
                    File__Analyze* Parser=Temp.Parsers[Pos];
                    for (size_t Pos2=0; Pos2<Temp.Parsers.size(); Pos2++)
                    {
                        if (Pos2!=Pos)
                            delete *(Temp.Parsers.begin()+Pos2);
                    }
                    Temp.Parsers.clear();
                    Temp.Parsers.push_back(Parser);
                }
            }

            if (Temp.Parsers.size()==1 && !Temp.IsFilled && Temp.Parsers[0]->Status[IsFilled])
            {
                stream_Count--;
                Temp.IsFilled=true;
            }
        }
    //FrameInfo.PCR=(int64u)-1;
    FrameInfo.DTS=(int64u)-1;
    FrameInfo.PTS=(int64u)-1;
    Element_Show();

    #if MEDIAINFO_EVENTS
        if (FrameInfo.DTS==(int64u)-1)
            FrameInfo.DTS=FrameInfo.PTS;

        //New PES
        #if MEDIAINFO_DEMUX
            if (PES_FirstByte_IsAvailable && PES_FirstByte_Value)
            {
                //Demux of substream data
                if (FromTS_stream_type==0x1B && SubStream_Demux)
                {
                    if (!SubStream_Demux->Buffers.empty() && !SubStream_Demux->Buffers.empty() && SubStream_Demux->Buffers[0] && SubStream_Demux->Buffers[0]->DTS<FrameInfo.DTS)
                    {
                        Demux(SubStream_Demux->Buffers[0]->Buffer, SubStream_Demux->Buffers[0]->Buffer_Size, ContentType_SubStream);
                        delete SubStream_Demux->Buffers[0]->Buffer; SubStream_Demux->Buffers[0]->Buffer=NULL;
                        SubStream_Demux->Buffers.erase(SubStream_Demux->Buffers.begin()); //Moving 2nd Buffer to 1st position
                    }
                }
            }

            //Demux of SubStream
            if (FromTS_stream_type==0x20 && SubStream_Demux)
            {
                //Searching an available slot
                size_t Buffers_Pos;
                if (SubStream_Demux->Buffers.empty() || SubStream_Demux->Buffers[SubStream_Demux->Buffers.size()-1]->DTS!=FrameInfo.DTS)
                {
                    Buffers_Pos=SubStream_Demux->Buffers.size();
                    SubStream_Demux->Buffers.push_back(new demux::buffer);
                }
                else
                {
                    Buffers_Pos=SubStream_Demux->Buffers.size()-1;
                }

                //Filling buffer
                if (SubStream_Demux->Buffers[Buffers_Pos]->Buffer==NULL)
                {
                    SubStream_Demux->Buffers[Buffers_Pos]->DTS=FrameInfo.DTS;
                    SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size_Max=128*1024;
                    SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size=0;
                    SubStream_Demux->Buffers[Buffers_Pos]->Buffer=new int8u[SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size_Max];
                }
                if (SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size_Max>SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size+(size_t)(Element_Size-Element_Offset) && SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size_Max<=16*1024*1024)
                {
                    SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size_Max*=2;
                    int8u* Buffer_Demux=SubStream_Demux->Buffers[Buffers_Pos]->Buffer;
                    SubStream_Demux->Buffers[Buffers_Pos]->Buffer=new int8u[SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size_Max];
                    std::memcpy(SubStream_Demux->Buffers[Buffers_Pos]->Buffer, Buffer_Demux, SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size);
                    delete[] Buffer_Demux; //Buffer_Demux=NULL;
                }
                if (SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size+(size_t)(Element_Size-Element_Offset)<=SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size_Max)
                {
                    std::memcpy(SubStream_Demux->Buffers[Buffers_Pos]->Buffer+SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
                    SubStream_Demux->Buffers[Buffers_Pos]->Buffer_Size+=(size_t)(Element_Size-Element_Offset);
                }
            }
        #endif //MEDIAINFO_DEMUX
    #endif //MEDIAINFO_EVENTS

    #if MEDIAINFO_SEEK && MEDIAINFO_IBI
        if (Seek_ID!=(int64u)-1)
        {
            if (Ibi.Streams[Seek_ID]->IsModified)
            {
                Read_Buffer_Seek(2, Seek_Value, Seek_ID);
            }
            else if (File_Offset+Buffer_Offset>=Seek_Value_Maximal)
            {
                //No intermediate seek point found, going to previous seek point
                for (size_t Pos=1; Pos<Ibi.Streams[Seek_ID]->Infos.size(); Pos++)
                    if (Ibi.Streams[Seek_ID]->Infos[Pos].StreamOffset>=Seek_Value_Maximal)
                    {
                        if (Ibi.Streams[Seek_ID]->IsSynchronized)
                        {
                            //No intermediate point is possible
                            Ibi.Streams[Seek_ID]->Infos[Pos-1].IsContinuous=true;
                            Read_Buffer_Seek(2, Seek_Value, Seek_ID);
                        }
                        else
                        {
                            //Going to last known seek point
                            GoTo(Ibi.Streams[Seek_ID]->Infos[Pos-1].StreamOffset);
                            Open_Buffer_Unsynch();
                        }
                        break;
                    }
            }
        }
    #endif //MEDIAINFO_SEEK && MEDIAINFO_IBI
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
bool File_MpegPs::Header_Parser_QuickSearch()
{
    while (           Buffer_Offset+4<=Buffer_Size
      &&   CC3(Buffer+Buffer_Offset)==0x000001)
    {
        //Getting stream_id
        int8u stream_id=Buffer[Buffer_Offset+3];

        //Trace config
        #if MEDIAINFO_TRACE
            if (Config_Trace_Level)
            {
                if (stream_id==0xC0 || stream_id==0xE0)
                {
                    if (!Trace_Layers[8])
                        Trace_Layers_Update(8); //Stream
                }
                else
                    Trace_Layers_Update(IsSub?1:0);
            }
        #endif //MEDIAINFO_TRACE

        //Searching start
        if (Streams[stream_id].Searching_Payload)
        {
            if (stream_id!=0xBD /*&& stream_id!=0xBF)*/ || !private_stream_1_IsDvdVideo) //Not (private_stream_1 and IsDvdVideo)
                return true;

            //private_stream_1 and IsDvdVideo, looking for substream ID
            if (Buffer_Offset+9>=Buffer_Size)
                return false; //Need more data
            size_t Data_Offset=Buffer[Buffer_Offset+8];
            if (Buffer_Offset+9+Data_Offset>=Buffer_Size)
                return false; //Need more data
            int8u  private_stream_1_ID=Buffer[Buffer_Offset+9+Data_Offset];
            if (!Streams_Private1[private_stream_1_ID].StreamIsRegistred || Streams_Private1[private_stream_1_ID].Searching_Payload)
                return true;
        }

        //Searching TimeStamp_End
        if (Streams[stream_id].Searching_TimeStamp_End)
        {
            switch(stream_id)
            {
                //Element with no PES Header
                case 0xB9 : //MPEG_program_end
                case 0xBA : //pack_start
                case 0xBB : //system_header_start
                case 0xBC : //program_stream_map
                case 0xBE : //padding_stream
                case 0xBF : //private_stream_2
                case 0xF0 : //ECM
                case 0xF1 : //EMM
                case 0xF2 : //DSMCC Streams
                case 0xF8 : //ITU-T Rec. H .222.1 type E
                case 0xFF : //Program Streams directory
                    break;

                //Element with PES Header
                default :
                    if (MPEG_Version==1)
                    {
                        size_t Buffer_Offset_Temp=Buffer_Offset+6;
                        while(Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp]==0xFF)
                        {
                            Buffer_Offset_Temp++;
                            if (Buffer_Offset_Temp+1>=Buffer_Size)
                                return false; //Not enough data
                        }
                        if (Buffer_Offset_Temp+1>=Buffer_Size)
                            return false; //Not enough data
                        if (Buffer_Offset_Temp<Buffer_Size && (Buffer[Buffer_Offset_Temp]&0xF0)!=0x00)
                            return true; //With a PTS
                    }
                    if (MPEG_Version==2)
                    {
                        if (Buffer_Offset+8>Buffer_Size)
                            return false; //Not enough buffer
                        if ((Buffer[Buffer_Offset+7]&0xC0)!=0x00)
                            return true; //With a PTS
                    }
            }
        }

        //Getting size
        switch(stream_id)
        {
            //No size
            case 0xB9 : //MPEG_program_end
            case 0xBA : //pack_start
                Buffer_Offset+=4;
                while(Buffer_Offset+4<=Buffer_Size && !(CC3(Buffer+Buffer_Offset)==0x000001 && Buffer[Buffer_Offset+3]>=0xB9))
                {
                    Buffer_Offset+=2;
                    while(Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x00)
                        Buffer_Offset+=2;
                    if (Buffer_Offset>=Buffer_Size || Buffer[Buffer_Offset-1]==0x00)
                        Buffer_Offset--;
                }
                //Parsing last bytes if needed
                if (Buffer_Offset+4>Buffer_Size)
                {
                    if (Buffer_Offset+3==Buffer_Size && CC3(Buffer+Buffer_Offset)!=0x000001)
                        Buffer_Offset++;
                    if (Buffer_Offset+2==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x0000)
                        Buffer_Offset++;
                    if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0x00)
                        Buffer_Offset++;
                }
                break;

            //Element with size
            default :
                if (Buffer_Offset+6>=Buffer_Size)
                    return false; //Not enough data
                int16u Size=CC2(Buffer+Buffer_Offset+4);
                if (Size>0)
                {
                    Buffer_Offset+=6+Size;

                    //Trailing 0xFF
                    while(Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]==0xFF)
                        Buffer_Offset++;

                    //Trailing 0x00
                    while(Buffer_Offset+3<=Buffer_Size
                       && Buffer[Buffer_Offset+2]==0x00
                       && Buffer[Buffer_Offset+1]==0x00
                       && Buffer[Buffer_Offset  ]==0x00)
                        Buffer_Offset++;
                }
                else
                {
                    Buffer_Offset+=6;
                    while(Buffer_Offset+4<=Buffer_Size && !(CC3(Buffer+Buffer_Offset)==0x000001 && Buffer[Buffer_Offset+3]>=0xB9))
                        Buffer_Offset++;
                    if (Buffer_Offset+4>Buffer_Size)
                    {
                        if (Buffer_Offset+3==Buffer_Size && CC3(Buffer+Buffer_Offset)!=0x000001)
                            Buffer_Offset++;
                        if (Buffer_Offset+2==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x0000)
                            Buffer_Offset++;
                        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0x00)
                            Buffer_Offset++;
                    }
                }
        }
    }

    if (Buffer_Offset+3==Buffer_Size)
        return false; //Sync is OK, but stream_id is not available
    if (Buffer_Offset+4<=Buffer_Size)
        Trusted_IsNot("MPEG-PS, Synchronisation lost");
    Synched=false;
    return Synchronize();
}

//***************************************************************************
// Parsers
//***************************************************************************

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Mpegv()
{
    //Filling
    #if defined(MEDIAINFO_MPEGV_YES)
        File_Mpegv* Parser=new File_Mpegv;
        Parser->MPEG_Version=MPEG_Version;
        Parser->ShouldContinueParsing=true;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Format, "MPEG Video");
        switch (FromTS_stream_type)
        {
            case 0x01 : Parser->Fill(Stream_Video, 0, Video_Codec, "MPEG-1V");
                        Parser->Fill(Stream_Video, 0, Video_Format_Version, "Version 1"); break;
            case 0x02 : Parser->Fill(Stream_Video, 0, Video_Codec, "MPEG-2V");
                        Parser->Fill(Stream_Video, 0, Video_Format_Version, "Version 2"); break;
            default   : Parser->Fill(Stream_Video, 0, Video_Codec, "MPEG-V");
        }
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Mpeg4v()
{
    //Filling
    #if defined(MEDIAINFO_MPEG4V_YES)
        File_Mpeg4v* Parser=new File_Mpeg4v;
        Parser->Frame_Count_Valid=1;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Codec, "MPEG-4V");
        Parser->Fill(Stream_Video, 0, Video_Format, "MPEG-4 Visual");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Avc()
{
    //Filling
    #if defined(MEDIAINFO_AVC_YES)
        File_Avc* Parser=new File_Avc;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Codec,  "AVC");
        Parser->Fill(Stream_Video, 0, Video_Format, "AVC");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Hevc()
{
    //Filling
    #if defined(MEDIAINFO_HEVC_YES)
        File_Hevc* Parser=new File_Hevc;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Codec,  "HEVC");
        Parser->Fill(Stream_Video, 0, Video_Format, "HEVC");
    #endif
    return Parser;
}
//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_VC1()
{
    //Filling
    #if defined(MEDIAINFO_VC1_YES)
        File_Vc1* Parser=new File_Vc1;
        Parser->Frame_Count_Valid=30;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Codec,  "VC-1");
        Parser->Fill(Stream_Video, 0, Video_Format, "VC-1");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Dirac()
{
    //Filling
    #if defined(MEDIAINFO_DIRAC_YES)
        File__Analyze* Parser=new File_Dirac;
        ((File_Dirac*)Parser)->Frame_Count_Valid=1;
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Codec,  "Dirac");
        Parser->Fill(Stream_Video, 0, Video_Format, "Dirac");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Mpega()
{
    //Filling
    #if defined(MEDIAINFO_MPEGA_YES)
        File_Mpega* Parser=new File_Mpega;
        Parser->Frame_Count_Valid=1;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "MPEG Audio");
        switch (FromTS_stream_type)
        {
            case 0x03 : Parser->Fill(Stream_Audio, 0, Audio_Codec,  "MPEG-1A");
                        Parser->Fill(Stream_Audio, 0, Audio_Format_Version, "Version 1"); break;
            case 0x04 : Parser->Fill(Stream_Audio, 0, Audio_Codec,  "MPEG-2A");
                        Parser->Fill(Stream_Audio, 0, Audio_Format_Version, "Version 2"); break;
            default   : Parser->Fill(Stream_Audio, 0, Audio_Codec,  "MPEG-A"); break;
        }
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Adts()
{
    //Filling
    #if defined(MEDIAINFO_AAC_YES)
        File_Aac* Parser=new File_Aac;
        Parser->Mode=File_Aac::Mode_ADTS;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Codec,  "AAC");
        Parser->Fill(Stream_Audio, 0, Audio_Format, "AAC");
        Parser->Fill(Stream_Audio, 0, Audio_MuxingMode, "ADTS");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Latm()
{
    //Filling
    #if defined(MEDIAINFO_AAC_YES)
        File_Aac* Parser=new File_Aac;
        Parser->Mode=File_Aac::Mode_LATM;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Codec,  "AAC");
        Parser->Fill(Stream_Audio, 0, Audio_Format, "AAC");
        Parser->Fill(Stream_Audio, 0, Audio_MuxingMode, "LATM");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_AC3()
{
    //Filling
    #if defined(MEDIAINFO_AC3_YES)
        File_Ac3* Parser=new File_Ac3();
        Parser->Frame_Count_Valid=2; //2 frames to be sure
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, private_stream_1_ID==0x83?"E-AC-3":"AC-3");
        Parser->Fill(Stream_Audio, 0, Audio_Codec, private_stream_1_ID==0x83?"AC3+":"AC3");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_DTS()
{
    //Filling
    #if defined(MEDIAINFO_DTS_YES)
        File__Analyze* Parser=new File_Dts();
        ((File_Dts*)Parser)->Frame_Count_Valid=2;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "DTS");
        Parser->Fill(Stream_Audio, 0, Audio_Codec, "DTS");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_SDDS()
{
    //Filling
    #if defined(MEDIAINFO_SDDS_YES)
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, StreamPos_Last, Audio_Format, "SDDS");
        Parser->Fill(Stream_Audio, StreamPos_Last, Audio_Codec,  "SDDS");
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "SDDS");
        Parser->Fill(Stream_Audio, 0, Audio_Codec,  "SDDS");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_AAC()
{
    //Filling
    #if defined(MEDIAINFO_AAC_YES)
        File_Aac* Parser=new File_Aac;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "AAC");
        Parser->Fill(Stream_Audio, 0, Audio_Codec,  "AAC");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_PCM()
{
    //Filling
    #if defined(MEDIAINFO_PCM_YES)
        File__Analyze* Parser;
        switch (FromTS_stream_type)
        {
            case 0x80 :
                        Parser=new File_Pcm_M2ts();
                        break;
            default   :
                        Parser=new File_Pcm_Vob();
        }
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "PCM");
        Parser->Fill(Stream_Audio, 0, Audio_Codec,  "PCM");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_RLE()
{
    //Filling
    #if defined(MEDIAINFO_RLE_YES)
        File__Analyze* Parser=new File_Rle();
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Text);
        Parser->Fill(Stream_Text, 0, Text_Format, "RLE");
        Parser->Fill(Stream_Text, 0, Text_Codec,  "RLE");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_AribStdB24B37(bool HasCcis)
{
    //Filling
    #if defined(MEDIAINFO_ARIBSTDB24B37_YES)
        File_AribStdB24B37* Parser=new File_AribStdB24B37();
        Parser->HasCcis=HasCcis;
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Text);
        Parser->Fill(Stream_Text, 0, Text_Format, "ARIB STD B24/B37");
        Parser->Fill(Stream_Text, 0, Text_Codec,  "ARIB STD B24/B37");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_DvbSubtitle()
{
    //Filling
    #if defined(MEDIAINFO_DVBSUBTITLE_YES)
        File__Analyze* Parser=new File_DvbSubtitle();
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Text);
        Parser->Fill(Stream_Text, 0, Text_Format, "DVB Subtitle");
        Parser->Fill(Stream_Text, 0, Text_Codec,  "DVB Subtitle");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Teletext()
{
    //Filling
    #if defined(MEDIAINFO_TELETEXT_YES)
        File__Analyze* Parser=new File_Teletext();
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Text);
        Parser->Fill(Stream_Text, 0, Text_Format, "Teletext");
        Parser->Fill(Stream_Text, 0, Text_Codec,  "Teletext");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_PGS()
{
    //Filling
    #if defined(MEDIAINFO_PGS_YES)
        File__Analyze* Parser=new File_Pgs();
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Text);
        Parser->Fill(Stream_Text, 0, Text_Format, "PGS");
        Parser->Fill(Stream_Text, 0, Text_Codec,  "PGS");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_SmpteSt0302()
{
    //Filling
    #if defined(MEDIAINFO_SMPTEST0302_YES)
        File_SmpteSt0302* Parser=new File_SmpteSt0302();
        #if MEDIAINFO_DEMUX
            if (Config->Demux_Unpacketize_Get())
            {
                Demux_UnpacketizeContainer=false; //No demux from this parser
                Demux_Level=4; //Intermediate
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "AES3");
        Parser->Fill(Stream_Audio, 0, Audio_Codec,  "AES3");
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_PS2()
{
    //Filling
    #if defined(MEDIAINFO_PS2A_YES)
        File__Analyze* Parser=new File_Ps2Audio();
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
    #endif
    return Parser;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_NULL()
{
    //Filling
    File__Analyze* Parser=new File_Unknown();
    Open_Buffer_Init(Parser);
    return Parser;
}

//***************************************************************************
// Output_Buffer
//***************************************************************************

//---------------------------------------------------------------------------
size_t File_MpegPs::Output_Buffer_Get (const String &Code)
{
    //Parsing Parsers
    for (size_t Streams_Pos=0; Streams_Pos<Streams.size(); Streams_Pos++)
        for (size_t Pos=0; Pos<Streams[Streams_Pos].Parsers.size(); Pos++)
            if (Streams[Streams_Pos].Parsers[Pos])
                if (size_t Size=Streams[Streams_Pos].Parsers[Pos]->Output_Buffer_Get(Code))
                    return Size;

    return 0;
}

//---------------------------------------------------------------------------
size_t File_MpegPs::Output_Buffer_Get (size_t Pos_)
{
    //Parsing Parsers
    for (size_t Streams_Pos=0; Streams_Pos<Streams.size(); Streams_Pos++)
        for (size_t Pos=0; Pos<Streams[Streams_Pos].Parsers.size(); Pos++)
            if (Streams[Streams_Pos].Parsers[Pos])
                if (size_t Size=Streams[Streams_Pos].Parsers[Pos]->Output_Buffer_Get(Pos_))
                    return Size;

    return 0;
}

} //Namespace

#endif //MEDIAINFO_MPEGPS_YES
