// File_MpegPs - Info for MPEG files
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
#if defined(MEDIAINFO_MPEGPS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_MpegPs.h"
#include "MediaInfo/Multiple/File_Mpeg_Psi.h"
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
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
#if defined(MEDIAINFO_ADTS_YES)
    #include "MediaInfo/Audio/File_Adts.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if defined(MEDIAINFO_AES3_YES)
    #include "MediaInfo/Audio/File_Aes3.h"
#endif
#if defined(MEDIAINFO_LATM_YES)
    #include "MediaInfo/Audio/File_Latm.h"
#endif
#if defined(MEDIAINFO_PS2A_YES)
    #include "MediaInfo/Audio/File_Ps2Audio.h"
#endif
#if defined(MEDIAINFO_RLE_YES)
    #include "MediaInfo/Image/File_Rle.h"
#endif
#if defined(MEDIAINFO_PGS_YES)
    #include "MediaInfo/Text/File_Pgs.h"
#endif
#include "MediaInfo/File_Unknown.h"
#include <ZenLib/Utils.h>
#include <algorithm>
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
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
    ParserName=_T("MpegPs");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_MpegPs;
        StreamIDs_Width[0]=2;
    #endif //MEDIAINFO_EVENTS
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;
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
        DecSpecificInfoTag=NULL;
        SLConfig=NULL;
    #endif
    #if MEDIAINFO_DEMUX
        SubStream_Demux=NULL;
    #endif //MEDIAINFO_DEMUX

    //Out
    HasTimeStamps=false;

    //Temp
    SizeToAnalyze=8*1024*1024;
    video_stream_Unlimited=false;
    Buffer_DataSizeToParse=0;
    Parsing_End_ForDTS=false;
    video_stream_PTS_FrameCount=0;
    video_stream_PTS_MustAddOffset=false;
    Demux_Unpacketize=MediaInfoLib::Config.Demux_Unpacketize_Get();

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
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegPs::Streams_Fill()
{
    //For each Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Fill_PerStream(StreamID, Streams[StreamID]);

    //For each private Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Fill_PerStream(StreamID, Streams_Private1[StreamID]);

    //For each extension Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
    {
        Streams_Fill_PerStream(StreamID, Streams_Extension[StreamID]);

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
    if (video_stream_PTS.size()>=2+4*2+1*2 && Retrieve(Stream_Video, 0, Video_FrameRate).To_float64()>30.000) //TODO: Handle all kind of files
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
                if (Retrieve(Stream_Video, 0, Video_ScanType)==_T("Interlaced"))
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

    if (Count_Get(Stream_Video)==1 && Retrieve(Stream_Video, 0, Video_Format_Version)==_T("Version 1"))
        Fill(Stream_General, 0, General_InternetMediaType, "video/mpeg", Unlimited, true, true);
}

//---------------------------------------------------------------------------
void File_MpegPs::Streams_Fill_PerStream(size_t StreamID, ps_stream &Temp)
{
    //By the parser
    StreamKind_Last=Stream_Max;
    if (!Temp.Parsers.empty() && Temp.Parsers[0] && Temp.Parsers[0]->Status[IsAccepted])
    {
        Fill(Temp.Parsers[0]);

        if (Temp.Parsers[0]->Count_Get(Stream_Video) && Temp.Parsers[0]->Count_Get(Stream_Text))
        {
            //Special case: Video and Text are together
            Stream_Prepare(Stream_Video);
            Merge(*Temp.Parsers[0], Stream_Video, 0, StreamPos_Last);
        }
        else
            Merge(*Temp.Parsers[0]);
    }

    //By the TS stream_type
    if (StreamKind_Last==Stream_Max)
    {
        //Disabling stream_private_1 if needed (will be done by Streams_Private1 object)
        if (Temp.stream_type!=0 && StreamID==0xBD)
        {
            bool StreamIsDetected=false;
            for (size_t Pos=0; Pos<Streams_Private1.size(); Pos++)
                if (!Streams_Private1[Pos].Parsers.empty() && Streams_Private1[Pos].Parsers[0])
                    StreamIsDetected=true;
            if (StreamIsDetected)
                Temp.stream_type=0;
        }

        if (Temp.stream_type!=0)
            Stream_Prepare(Mpeg_Psi_stream_type_StreamKind(Temp.stream_type, 0x00000000));
    }

    //By StreamIsRegistred
    if (StreamKind_Last==Stream_Max)
    {
        if (Temp.StreamIsRegistred)
        {
            if (StreamID>=0xC0 && StreamID<=0xDF)
                Stream_Prepare(Stream_Audio);
            if (StreamID>=0xE0 && StreamID<=0xEF)
                Stream_Prepare(Stream_Video);
        }
    }

    //More info
    if (StreamKind_Last!=Stream_Max) //Found
    {
        ///Saving StreamKind and Stream_Pos
        Temp.StreamKind=StreamKind_Last;
        Temp.StreamPos=StreamPos_Last;

        //Common
        Fill(StreamKind_Last, StreamPos_Last, General_ID, StreamID);
        Ztring ID_String; ID_String.From_Number(StreamID); ID_String+=_T(" (0x"); ID_String+=Ztring::ToZtring(StreamID, 16); ID_String+=_T(")");
        Fill(StreamKind_Last, StreamPos_Last, General_ID_String, ID_String, true); //TODO: merge with Decimal_Hexa in file_MpegTs
        if (Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format)).empty() && Temp.stream_type!=0)
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), Mpeg_Psi_stream_type_Format(Temp.stream_type, 0x0000));
        if (Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec)).empty() && Temp.stream_type!=0)
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), Mpeg_Psi_stream_type_Codec(Temp.stream_type, 0x0000));

        if (Temp.TimeStamp_Start.PTS.TimeStamp!=(int64u)-1)
        {
            if (StreamKind_Last==Stream_Video)
            {
                if (!Retrieve(Stream_Video, Temp.StreamPos, Video_Delay).empty())
                {
                    Ztring Delay=Retrieve(Stream_Video, Temp.StreamPos, Video_Delay);
                    Fill(Stream_Video, Temp.StreamPos, Video_Delay_Original, Delay);
                }
                if (!Retrieve(Stream_Video, Temp.StreamPos, Video_Delay_Settings).empty())
                {
                    Ztring Delay_Settings=Retrieve(Stream_Video, Temp.StreamPos, Video_Delay_Settings);
                    Fill(Stream_Video, Temp.StreamPos, Video_Delay_Original_Settings, Delay_Settings);
                }
            }
            Fill(StreamKind_Last, StreamPos_Last, "Delay", ((float64)Temp.TimeStamp_Start.PTS.TimeStamp)/90, 3, true);
            Fill(StreamKind_Last, StreamPos_Last, "Delay_Settings", "", Unlimited, true, true);
        }
        else
        {
            Clear(StreamKind_Last, StreamPos_Last, "Delay");
            Clear(StreamKind_Last, StreamPos_Last, "Delay_Settings");
        }

        //LATM
        if (StreamKind_Last==Stream_Audio && StreamID==0xFA && FromTS_stream_type==0x11)
            Fill(Stream_Audio, 0, Audio_MuxingMode, "LATM");
    }

    //Bitrate calculation
    if (PTS!=(int64u)-1 && (StreamKind_Last==Stream_Video || StreamKind_Last==Stream_Audio))
    {
        int64u BitRate=Retrieve(StreamKind_Last, StreamPos_Last, "BitRate").To_int64u();
        if (BitRate==0)
            BitRate=Retrieve(StreamKind_Last, StreamPos_Last, "BitRate_Nominal").To_int64u();
        if (BitRate==0)
            PTS=(int64u)-1;
        else
            PTS+=BitRate; //Saving global BitRate
    }
}

//---------------------------------------------------------------------------
void File_MpegPs::Streams_Finish()
{
    PTS=0; //Will be used for BitRate calculation
    DTS=0; //Will be used for Duration calculation

    //For each Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Finish_PerStream(StreamID, Streams[StreamID]);

    //For each private Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Finish_PerStream(StreamID, Streams_Private1[StreamID]);

    //For each extesnion Streams
    for (size_t StreamID=0; StreamID<0x100; StreamID++)
        Streams_Finish_PerStream(StreamID, Streams_Extension[StreamID]);

    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
    {
        Streams.clear();
        Streams_Private1.clear();
        Streams_Extension.clear();
    }

    //Bitrate coherancy
    if (!IsSub && PTS>0 && PTS!=(int64u)-1 && DTS!=0 && File_Size!=(int64u)-1)
    {
        int64u BitRate_FromDuration=File_Size*8000*90/DTS;
        int64u BitRate_FromBitRates=PTS;

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
}

//---------------------------------------------------------------------------
void File_MpegPs::Streams_Finish_PerStream(size_t StreamID, ps_stream &Temp)
{
    //By the parser
    if (Temp.StreamKind==Stream_Max && !Temp.Parsers.empty() && Temp.Parsers[0])
        Streams_Fill_PerStream(StreamID, Temp);

    //Init
    if (Temp.StreamKind==Stream_Max)
        return;
    StreamKind_Last=Temp.StreamKind;
    StreamPos_Last=Temp.StreamPos;

    //By the parser
    if (!Temp.Parsers.empty() && Temp.Parsers[0])
    {
        Temp.Parsers[0]->ShouldContinueParsing=false;
        Finish(Temp.Parsers[0]);
        Merge(*Temp.Parsers[0], StreamKind_Last, 0, StreamPos_Last);

        //Special cases
        if (Temp.Parsers[0]->Count_Get(Stream_Video) && Temp.Parsers[0]->Count_Get(Stream_Text))
        {
            //Video and Text are together
            size_t Text_Count=Temp.Parsers[0]->Count_Get(Stream_Text);
            for (size_t Text_Pos=0; Text_Pos<Text_Count; Text_Pos++)
            {
                Stream_Prepare(Stream_Text);
                Merge(*Temp.Parsers[0], Stream_Text, Text_Pos, StreamPos_Last);

                Ztring MuxingMode=Retrieve(Stream_Text, StreamPos_Last, "MuxingMode");
                Fill(Stream_Text, StreamPos_Last, "MuxingMode", Ztring(_T("MPEG Video / "))+MuxingMode, true);
                if (!IsSub)
                    Fill(Stream_Text, StreamPos_Last, "MuxingMode_MoreInfo", _T("Muxed in Video #")+Ztring().From_Number(Temp.StreamPos+1));
                Ztring ID=Retrieve(Stream_Text, StreamPos_Last, Text_ID);
                Fill(Stream_Text, StreamPos_Last, Text_ID, Retrieve(Stream_Video, Temp.StreamPos, Video_ID)+_T("-")+ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_ID_String, Retrieve(Stream_Video, Temp.StreamPos, Video_ID_String)+_T("-")+ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_Delay, Retrieve(Stream_Video, Temp.StreamPos, Video_Delay), true);
            }

            StreamKind_Last=Temp.StreamKind;
            StreamPos_Last=Temp.StreamPos;
        }
    }

    //More info
    if (Temp.StreamKind!=Stream_Max) //Found
    {
         StreamKind_Last=Temp.StreamKind;
         StreamPos_Last=Temp.StreamPos;

        int64u Start=(int64u)-1, End=(int64u)-1;
        if (Temp.TimeStamp_Start.DTS.TimeStamp!=(int64u)-1 && Temp.TimeStamp_End.DTS.TimeStamp!=(int64u)-1)
        {
            Start=Temp.TimeStamp_Start.DTS.TimeStamp;
            End=Temp.TimeStamp_End.DTS.TimeStamp;
        }
        else if (Temp.TimeStamp_Start.PTS.TimeStamp!=(int64u)-1 && Temp.TimeStamp_End.PTS.TimeStamp!=(int64u)-1)
        {
            Start=Temp.TimeStamp_Start.PTS.TimeStamp;
            End=Temp.TimeStamp_End.PTS.TimeStamp;
        }
        if (Start!=(int64u)-1 && End!=(int64u)-1)
        {
            //TimeStamp
            if (End<Start)
                End+=0x200000000LL; //33 bits, cyclic
            int64u Duration=End-Start;
            if (File_Size!=(int64u)-1 && File_Size>1024*1024*16 && End>=0x200000000LL)
            {
                //Testing coherancy
                if (Duration/90>16*3600*1000)
                    Duration=0; //Disabling it
            }
            if (Duration)
            {
                if (StreamKind_Last==Stream_Video)
                {
                    float64 FrameRate=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate).To_float64();
                    if (FrameRate!=0)
                        Duration+=Ztring::ToZtring(Temp.FrameCount_AfterLast_TimeStamp_End*90*1000/FrameRate, 0).To_int64u();
                }
                if (Duration)
                {
                    Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Duration), Duration/90, 10, true);
                    if (Duration>DTS)
                        DTS=Duration; //Saving maximum Duration
                }
            }
        }
    }

    //Bitrate calculation
    if (PTS!=(int64u)-1 && (StreamKind_Last==Stream_Video || StreamKind_Last==Stream_Audio))
    {
        int64u BitRate=Retrieve(StreamKind_Last, StreamPos_Last, "BitRate").To_int64u();
        if (BitRate==0)
            BitRate=Retrieve(StreamKind_Last, StreamPos_Last, "BitRate_Nominal").To_int64u();
        if (BitRate==0)
            PTS=(int64u)-1;
        else
            PTS+=BitRate; //Saving global BitRate
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
        if (Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset-1]==0x00 || Buffer_Offset>=Buffer_Size)
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
    private_stream_2_Count=true;
    extension_stream_Count=(int8u)-1;

    //From packets
    program_mux_rate=0;

    //Default values
    Streams.resize(0x100);
    Streams_Private1.resize(0x100);
    Streams_Extension.resize(0x100);
    Streams[0xBA].Searching_Payload=true;

    //Temp
    stream_id_extension=0x55; //Default is set to VC-1, should never happens, but happens sometimes

    //Case of extraction from MPEG-TS files
    if (File_Offset==0 && Buffer_Size>=4 && ((CC4(Buffer)&0xFFFFFFF0)==0x000001E0 || (CC4(Buffer)&0xFFFFFFE0)==0x000001C0 || CC4(Buffer)==0x000001BD || CC4(Buffer)==0x000001FA || CC4(Buffer)==0x000001FD))
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
    }
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

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
                Streams[StreamID].Parsers[Pos]->Open_Buffer_Unsynch();
        Streams_Private1[StreamID].TimeStamp_End.PTS.File_Pos=(int64u)-1;
        Streams_Private1[StreamID].TimeStamp_End.DTS.File_Pos=(int64u)-1;
        Streams_Private1[StreamID].TimeStamp_End.PTS.TimeStamp=(int64u)-1;
        Streams_Private1[StreamID].TimeStamp_End.DTS.TimeStamp=(int64u)-1;
        Streams_Private1[StreamID].Searching_TimeStamp_Start=false;
        for (size_t Pos=0; Pos<Streams_Private1[StreamID].Parsers.size(); Pos++)
            if (Streams_Private1[StreamID].Parsers[Pos])
                Streams_Private1[StreamID].Parsers[Pos]->Open_Buffer_Unsynch();
        Streams_Extension[StreamID].TimeStamp_End.PTS.File_Pos=(int64u)-1;
        Streams_Extension[StreamID].TimeStamp_End.DTS.File_Pos=(int64u)-1;
        Streams_Extension[StreamID].TimeStamp_End.PTS.TimeStamp=(int64u)-1;
        Streams_Extension[StreamID].TimeStamp_End.DTS.TimeStamp=(int64u)-1;
        Streams_Extension[StreamID].Searching_TimeStamp_Start=false;
        for (size_t Pos=0; Pos<Streams_Extension[StreamID].Parsers.size(); Pos++)
            if (Streams_Extension[StreamID].Parsers[Pos])
                Streams_Extension[StreamID].Parsers[Pos]->Open_Buffer_Unsynch();
    }
    video_stream_Unlimited=false;
    Buffer_DataSizeToParse=0;

    #if MEDIAINFO_EVENTS
        MpegPs_PES_FirstByte_IsAvailable=false;
    #endif //MEDIAINFO_EVENTS
}

//---------------------------------------------------------------------------
void File_MpegPs::Read_Buffer_Continue()
{
    if (Buffer_DataSizeToParse)
    {
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

        Element_Begin();
        Data_Parse();
        Element_Offset=Element_Size;
        Element_End();
    }

    //Video unlimited specific, we didn't wait for the end (because this is... unlimited)
    if (video_stream_Unlimited)
    {
        #if MEDIAINFO_EVENTS
            if (FromTS)
            {
                MpegPs_PES_FirstByte_IsAvailable=true;
                MpegPs_PES_FirstByte_Value=false;
            }
        #endif //MEDIAINFO_EVENTS

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
            if (Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp-1]==0x00 || Buffer_Offset_Temp>=Buffer_Size)
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
                Element_IsWaitingForMoreData(); //We don't know if the next bytes are a start_code or data
        }

        Element_Begin();
        Data_Parse();
        Element_Offset=Element_Size;
        Element_End();
    }
}

//***************************************************************************
// Buffer - Par element
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegPs::Header_Parse()
#if MEDIAINFO_TRACE
{
    #if MEDIAINFO_EVENTS
        if (FromTS)
        {
            MpegPs_PES_FirstByte_IsAvailable=true;
            MpegPs_PES_FirstByte_Value=true;
        }
    #endif //MEDIAINFO_EVENTS

    //Reinit
    PTS=(int64u)-1;
    DTS=(int64u)-1;

    //Parsing
    Skip_B3(                                                    "synchro");
    Get_B1 (start_code,                                         "start_code");

    if (start_code!=0xB9 && start_code!=0xBA) //MPEG_program_end or pack_start have no PES
        Header_Parse_PES_packet(start_code);
    else if (!Header_Parse_Fill_Size()) //MPEG_program_end or pack_start specific
    {
        Element_WaitForMoreData();
        return;
    }
    Header_Fill_Code(start_code);
}
#else //MEDIAINFO_TRACE
{
    #if MEDIAINFO_EVENTS
        if (FromTS)
        {
            MpegPs_PES_FirstByte_IsAvailable=true;
            MpegPs_PES_FirstByte_Value=true;
        }
    #endif //MEDIAINFO_EVENTS

    //Reinit
    PTS=(int64u)-1;
    DTS=(int64u)-1;

    //Parsing
    start_code=Buffer[Buffer_Offset+3];
    Element_Offset+=4;

    if (start_code!=0xB9 && start_code!=0xBA) //MPEG_program_end or pack_start have no PES
        Header_Parse_PES_packet(start_code);
    else if (!Header_Parse_Fill_Size()) //MPEG_program_end or pack_start specific
    {
        Element_WaitForMoreData();
        return;
    }
    Header_Fill_Code(start_code);
}
#endif //MEDIAINFO_TRACE

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
        if (Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp-1]==0x00 || Buffer_Offset_Temp>=Buffer_Size)
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
        if (File_Offset+Buffer_Size==File_Size)
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
void File_MpegPs::Header_Parse_PES_packet(int8u start_code)
{
    //Parsing
    int16u PES_packet_length;
    Get_B2 (PES_packet_length,                                  "PES_packet_length");

    //Filling
    Header_Fill_Size(6+PES_packet_length);

    //Parsing
    switch (start_code)
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
            return;

        //Element with PES Header
        default :
            switch (MPEG_Version)
            {
                case 1  : Header_Parse_PES_packet_MPEG1(start_code); break;
                case 2  : Header_Parse_PES_packet_MPEG2(start_code); break;
                default : ; //We don't know what to parse...
            }
    }

    //Video unlimited specific
    if (PES_packet_length==0 && Element_Offset<Element_Size)
        if (!Header_Parse_Fill_Size())
        {
            //Return directly if we must unpack the elementary stream;
            if (Demux_Unpacketize)
            {
                Element_WaitForMoreData();
                return;
            }

            //Next PS packet is not found, we will use all the buffer
            Header_Fill_Size(Buffer_Size-Buffer_Offset); //All the buffer is used
            video_stream_Unlimited=true;
            Buffer_Offset_Temp=0; //We use the buffer
        }

    //Can be cut in small chunks
    if (PES_packet_length!=0 && Element_Offset<Element_Size && (size_t)(6+PES_packet_length)>Buffer_Size-Buffer_Offset
     && ((start_code&0xE0)==0xC0 || (start_code&0xF0)==0xE0))
    {
        //Return directly if we must unpack the elementary stream;
        if (Demux_Unpacketize)
        {
            Element_WaitForMoreData();
            return;
        }

        Header_Fill_Size(Buffer_Size-Buffer_Offset); //All the buffer is used
        Buffer_DataSizeToParse=6+PES_packet_length-(int16u)(Buffer_Size-Buffer_Offset);
        Buffer_Offset_Temp=0; //We use the buffer
    }
}

//---------------------------------------------------------------------------
// Packet header data - MPEG-1
void File_MpegPs::Header_Parse_PES_packet_MPEG1(int8u start_code)
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
        Element_Begin("PTS");
        BS_Begin();
        Mark_0();
        Mark_0();
        Mark_1();
        Mark_0();
        Get_S1 ( 3, PTS_32,                                     "PTS_32");
        Mark_1();
        Get_S2 (15, PTS_29,                                     "PTS_29");
        Mark_1();
        Get_S2 (15, PTS_14,                                     "PTS_14");
        Mark_1();
        BS_End();

        //Filling
        PTS=(((int64u)PTS_32)<<30)
          | (((int64u)PTS_29)<<15)
          | (((int64u)PTS_14));
        if (Streams[start_code].Searching_TimeStamp_End && start_code!=0xBD && start_code!=0xFD) //0xBD and 0xFD can contain multiple streams, TimeStamp management is in Streams management
        {
            Streams[start_code].TimeStamp_End.PTS.TimeStamp=PTS;
        }
        if (Searching_TimeStamp_Start && Streams[start_code].Searching_TimeStamp_Start && start_code!=0xBD && start_code!=0xFD) //0xBD and 0xFD can contain multiple streams, TimeStamp management is in Streams management
        {
            Streams[start_code].TimeStamp_Start.PTS.TimeStamp=PTS;
            Streams[start_code].Searching_TimeStamp_Start=false;
        }
        Element_Info_From_Milliseconds(PTS/90);
        Element_End();
    }
    else if ((stuffing_byte&0xF0)==0x30)
    {
        int16u PTS_29, PTS_14, DTS_29, DTS_14;
        int8u  PTS_32, DTS_32;
        Element_Begin("PTS");
        BS_Begin();
        Mark_0();
        Mark_0();
        Mark_1();
        Mark_1();
        Get_S1 ( 3, PTS_32,                                     "PTS_32");
        Mark_1();
        Get_S2 (15, PTS_29,                                     "PTS_29");
        Mark_1();
        Get_S2 (15, PTS_14,                                     "PTS_14");
        Mark_1();
        BS_End();

        //Filling
        PTS=(((int64u)PTS_32)<<30)
          | (((int64u)PTS_29)<<15)
          | (((int64u)PTS_14));
        if (Streams[start_code].Searching_TimeStamp_End)
        {
            Streams[start_code].TimeStamp_End.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[start_code].TimeStamp_End.PTS.TimeStamp=PTS;
        }
        if (Searching_TimeStamp_Start && Streams[start_code].Searching_TimeStamp_Start)
        {
            Streams[start_code].TimeStamp_Start.PTS.TimeStamp=PTS;
        }
        Element_Info_From_Milliseconds(PTS/90);
        Element_End();

        Element_Begin("DTS");
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
        DTS=(((int64u)DTS_32)<<30)
          | (((int64u)DTS_29)<<15)
          | (((int64u)DTS_14));
        if (Streams[start_code].Searching_TimeStamp_End)
        {
            Streams[start_code].TimeStamp_End.DTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[start_code].TimeStamp_End.DTS.TimeStamp=DTS;
        }
        if (Searching_TimeStamp_Start && Streams[start_code].Searching_TimeStamp_Start)
        {
            Streams[start_code].TimeStamp_Start.DTS.TimeStamp=DTS;
            Streams[start_code].Searching_TimeStamp_Start=false;
        }
        Element_Info_From_Milliseconds(Streams[start_code].TimeStamp_End.DTS.TimeStamp/90);
        Element_End();
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
    }
}

//---------------------------------------------------------------------------
// Packet header data - MPEG-2
void File_MpegPs::Header_Parse_PES_packet_MPEG2(int8u start_code)
{
    //Parsing
    int8u PTS_DTS_flags, PES_header_data_length;
    bool ESCR_flag, ES_rate_flag, DSM_trick_mode_flag, additional_copy_info_flag, PES_CRC_flag, PES_extension_flag;
    #if MEDIAINFO_TRACE
    BS_Begin();
    Mark_1();
    Mark_0();
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
    #else //MEDIAINFO_TRACE
    if (Element_Offset+3>=Element_Size)
    {
        Trusted_IsNot();
        return;
    }
    size_t Buffer_Pos_Flags=Buffer_Offset+(size_t)Element_Offset;
    if ((Buffer[Buffer_Pos_Flags]&0xC0)!=0x80) //bit 6 and 7 are 01
    {
        Element_DoNotTrust(); //Mark bits are wrong
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
    #endif //MEDIAINFO_TRACE
    int64u Element_Pos_After_Data=Element_Offset+PES_header_data_length;
    
    //Options
    if (PTS_DTS_flags==0x2)
    {
        #if MEDIAINFO_TRACE
        int16u PTS_29, PTS_14;
        int8u  PTS_32;
        Element_Begin("PTS_DTS_flags");
        Element_Begin("PTS");
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
        PTS=(((int64u)PTS_32)<<30)
          | (((int64u)PTS_29)<<15)
          | (((int64u)PTS_14));
        Element_Info_From_Milliseconds(PTS/90);
        Element_End();
        Element_End();
        #else //MEDIAINFO_TRACE
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
            Element_DoNotTrust(); //Mark bits are wrong
            return;
        }
        PTS=                                            ((((int64u)Buffer[Buffer_Pos  ]&0x0E))<<29)
          | ( ((int64u)Buffer[Buffer_Pos+1]      )<<22)|((((int64u)Buffer[Buffer_Pos+2]&0xFE))<<14)
          | ( ((int64u)Buffer[Buffer_Pos+3]      )<< 7)|((((int64u)Buffer[Buffer_Pos+4]&0xFE))>> 1);
        Element_Offset+=5;
        #endif //MEDIAINFO_TRACE

        //Filling
        if (Streams[start_code].Searching_TimeStamp_End)
        {
            Streams[start_code].TimeStamp_End.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[start_code].TimeStamp_End.PTS.TimeStamp=PTS;
        }
        if (Searching_TimeStamp_Start && Streams[start_code].Searching_TimeStamp_Start)
        {
            Streams[start_code].TimeStamp_Start.PTS.TimeStamp=PTS;
            Streams[start_code].Searching_TimeStamp_Start=false;
        }
        HasTimeStamps=true;
    }
    if (PTS_DTS_flags==0x3)
    {
        #if MEDIAINFO_TRACE
        int16u PTS_29, PTS_14, DTS_29, DTS_14;
        int8u  PTS_32, DTS_32;
        Element_Begin("PTS_DTS_flags");
        Element_Begin("PTS");
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
        PTS=(((int64u)PTS_32)<<30)
          | (((int64u)PTS_29)<<15)
          | (((int64u)PTS_14));
        Element_Info_From_Milliseconds(PTS/90);
        Element_End();
        #else //MEDIAINFO_TRACE
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
            Element_DoNotTrust(); //Mark bits are wrong
            return;
        }
        PTS=                                            ((((int64u)Buffer[Buffer_Pos  ]&0x0E))<<29)
          | ( ((int64u)Buffer[Buffer_Pos+1]      )<<22)|((((int64u)Buffer[Buffer_Pos+2]&0xFE))<<14)
          | ( ((int64u)Buffer[Buffer_Pos+3]      )<< 7)|((((int64u)Buffer[Buffer_Pos+4]&0xFE))>> 1);
        Element_Offset+=5;
        #endif //MEDIAINFO_TRACE

        //Filling
        if (Streams[start_code].Searching_TimeStamp_End)
        {
            Streams[start_code].TimeStamp_End.PTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[start_code].TimeStamp_End.PTS.TimeStamp=PTS;
        }
        if (Searching_TimeStamp_Start && Streams[start_code].Searching_TimeStamp_Start)
        {
            Streams[start_code].TimeStamp_Start.PTS.TimeStamp=PTS;
            //Streams[start_code].Searching_TimeStamp_Start=false;
        }

        #if MEDIAINFO_TRACE
        Element_Begin("DTS");
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
        DTS=(((int64u)DTS_32)<<30)
          | (((int64u)DTS_29)<<15)
          | (((int64u)DTS_14));
        Element_Info_From_Milliseconds(DTS/90);
        Element_End();
        Element_End();
        #else //MEDIAINFO_TRACE
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
            Element_DoNotTrust(); //Mark bits are wrong
            return;
        }
        DTS=                                            ((((int64u)Buffer[Buffer_Pos  ]&0x0E))<<29)
          | ( ((int64u)Buffer[Buffer_Pos+1]      )<<22)|((((int64u)Buffer[Buffer_Pos+2]&0xFE))<<14)
          | ( ((int64u)Buffer[Buffer_Pos+3]      )<< 7)|((((int64u)Buffer[Buffer_Pos+4]&0xFE))>> 1);
        Element_Offset+=5;
        #endif //MEDIAINFO_TRACE

        //Filling
        if (Streams[start_code].Searching_TimeStamp_End)
        {
            Streams[start_code].TimeStamp_End.DTS.File_Pos=File_Offset+Buffer_Offset;
            Streams[start_code].TimeStamp_End.DTS.TimeStamp=DTS;
        }
        if (Searching_TimeStamp_Start && Streams[start_code].Searching_TimeStamp_Start)
        {
            Streams[start_code].TimeStamp_Start.DTS.TimeStamp=DTS;
            Streams[start_code].Searching_TimeStamp_Start=false;
        }
        HasTimeStamps=true;
    }
    if (ESCR_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin("ESCR_flag");
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
        Element_End();
    }
    if (ES_rate_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin("ES_rate_flag");
        BS_Begin();
        int32u ES_rate;
        Mark_1();
        Get_S3 (22, ES_rate,                                    "ES_rate");
        Mark_1();
        BS_End();
        Element_End();
    }
    if (DSM_trick_mode_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin("DSM_trick_mode_flag");
        BS_Begin();
        int8u trick_mode_control;
        Get_S1 (3, trick_mode_control,                         "trick_mode_control"); Param_Info(MpegPs_trick_mode_control_values[trick_mode_control]);
        switch (trick_mode_control)
        {
            case 0 :{ //fast_forward
                        Skip_S1(2,                              "field_id");
                        Skip_SB(                                "intra_slice_refresh");
                        Skip_S1(2,                              "frequency_truncation");
                    }
                    break;
            case 1 :{ //slow_motion
                        Skip_S1(5,                              "rep_cntrl");
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
                        Skip_S1(5,                              "rep_cntrl");
                    }
                    break;
            default:{
                        Skip_S1(5,                              "reserved");
                    }
        }
        BS_End();
        Element_End();
    }
    if (additional_copy_info_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin("additional_copy_info_flag");
        BS_Begin();
        Mark_1();
        Skip_S1(7,                                              "additional_copy_info");
        BS_End();
        Element_End();
    }
    if (additional_copy_info_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin("additional_copy_info_flag");
        Skip_B2(                                                "previous_PES_packet_CRC");
        Element_End();
    }
    if (PES_extension_flag && Element_Offset<Element_Pos_After_Data)
    {
        Element_Begin("PES_extension_flag");
        BS_Begin();
        bool PES_private_data_flag, pack_header_field_flag, program_packet_sequence_counter_flag, p_STD_buffer_flag, PES_extension_flag_2;
        Get_SB (PES_private_data_flag,                          "PES_private_data_flag");
        Get_SB (pack_header_field_flag,                         "pack_header_field_flag");
        Get_SB (program_packet_sequence_counter_flag,           "program_packet_sequence_counter_flag");
        Get_SB (p_STD_buffer_flag,                              "P-STD_buffer_flag");
        Skip_S1(3,                                              "reserved");
        Get_SB (PES_extension_flag_2,                           "PES_extension_flag_2");
        BS_End();
        if (PES_private_data_flag)
        {
            Element_Begin("PES_private_data_flag");
            Skip_B16(                                           "PES_private_data");
            Element_End();
        }
        if (pack_header_field_flag)
        {
            Element_Begin("pack_header_field_flag");
            int8u pack_field_length;
            Get_B1 (pack_field_length,                          "pack_field_length");
            Skip_XX(pack_field_length,                          "pack_header");
            Element_End();
        }
        if (program_packet_sequence_counter_flag)
        {
            Element_Begin("program_packet_sequence_counter_flag");
            BS_Begin();
            Mark_1();
            Skip_S1(7,                                          "program_packet_sequence_counter");
            Mark_1();
            Skip_SB(                                            "MPEG1_MPEG2_identifier");
            Skip_S1(6,                                          "original_stuff_length");
            BS_End();
            Element_End();
        }
        if (p_STD_buffer_flag)
        {
            Element_Begin("p_STD_buffer_flag");
            BS_Begin();
            Mark_0();
            Skip_SB(                                            "Should be 1"); //But I saw a file with "0"
            Skip_SB(                                            "P-STD_buffer_scale");
            Skip_S2(13,                                         "P-STD_buffer_size");
            BS_End();
            Element_End();
        }
        if (PES_extension_flag_2)
        {
            Element_Begin("PES_extension_flag_2");
            int8u PES_extension_field_length;
            bool stream_id_extension_flag;
            BS_Begin();
            Mark_1();
            Get_S1 (7, PES_extension_field_length,              "PES_extension_field_length");
            Get_SB (stream_id_extension_flag,                   "stream_id_extension_flag");
            if (stream_id_extension_flag==0)
            {
                Get_S1 (7, stream_id_extension,                 "stream_id_extension"); Param_Info(MpegPs_stream_id_extension(stream_id_extension));
            }
            else
            {
                //This should not, but I found a file with stream_id_extension_flag=1 and a real code...
                Get_S1 (7, stream_id_extension,                 "stream_id_extension"); Param_Info(MpegPs_stream_id_extension(stream_id_extension));
            }
            BS_End();
            if (PES_extension_field_length-1>0)
                Skip_XX(PES_extension_field_length-1,           "reserved");
            Element_End();
        }
        Element_End();
    }
    if (Element_Pos_After_Data>Element_Offset)
        Skip_XX(Element_Pos_After_Data-Element_Offset,          "stuffing_bytes");
}

//---------------------------------------------------------------------------
void File_MpegPs::Data_Parse()
{
    //Needed?
    if (!Streams[start_code].Searching_Payload)
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
    switch (start_code)
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
        case 0xFF : Element_Name("program_stream_directory"); Skip_XX(Element_Size, "Data"); break;
        default:
                 if ((start_code&0xE0)==0xC0) audio_stream();
            else if ((start_code&0xF0)==0xE0) video_stream();
            else
                Trusted_IsNot("Unattended element!");
    }

    //Jumping to the last DTS if needed
    //if (!Parsing_End_ForDTS && File_Offset+Buffer_Offset==File_Size)
    //    Jump_DTS();

    #if MEDIAINFO_EVENTS
        MpegPs_PES_FirstByte_IsAvailable=false;
    #endif //MEDIAINFO_EVENTS
}

//---------------------------------------------------------------------------
void File_MpegPs::Detect_EOF()
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
    }

    //Jumping only if needed
    if (Streams.empty() || video_stream_Count>0 || audio_stream_Count>0 || private_stream_1_Count>0 || private_stream_2_Count==true || extension_stream_Count>0)
        return;

    //Jumping if needed
    if (!Status[IsAccepted])
    {
        Accept("MPEG-PS");
        if (!IsSub)
            Fill(Stream_General, 0, General_Format, "MPEG-PS");
    }
    Fill("MPEG-PS");
    if (File_Size>SizeToAnalyze && File_Offset+Buffer_Size<File_Size-SizeToAnalyze)
    {
        //Jumping
        GoToFromEnd(SizeToAnalyze, "MPEG-PS");
        Read_Buffer_Unsynched();
    }
}

//---------------------------------------------------------------------------
//Jumping to the last DTS if needed
bool File_MpegPs::BookMark_Needed()
{
    if (IsSub || Streams.empty() || MediaInfoLib::Config.ParseSpeed_Get()>=1.0)
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
    if (Status[IsAccepted] && ToJump!=(int64u)-1)
    {
        Info("MPEG-PS, Jumping to nearly end of file");
        Parsing_End_ForDTS=true;
        File_GoTo=ToJump;
        return true;
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

    //Filling
    Synched=false; //We don't know what can be after
}

//---------------------------------------------------------------------------
// Packet "BA"
void File_MpegPs::pack_start()
{
    Element_Name("pack_start");

    //Parsing
    int16u SysClock_29, SysClock_14;
    int8u Version, SysClock_32, Padding;
    BS_Begin();
    Peek_S1( 2, Version);
    if (Version==1)
    {
        //MPEG-2
        MPEG_Version=2;
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
        Get_S3 (22, program_mux_rate,                           "program_mux_rate"); Param_Info(program_mux_rate*400, " bps");
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
        //MPEG-1
        MPEG_Version=1;
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
        Get_S3(22, program_mux_rate,                            "mux_rate"); Param_Info(program_mux_rate*400, " bps");
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
        }

        SizeToAnalyze=program_mux_rate*50*2; //standard delay between TimeStamps is 0.7s, we try 2s to be sure
        if (SizeToAnalyze>16*1024*1024)
            SizeToAnalyze=16*1024*1024; //Not too much
        if (SizeToAnalyze<2*1024*1024)
            SizeToAnalyze=2*1024*1024; //Not too less
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "BB"
void File_MpegPs::system_header_start()
{
    Element_Name("system_header_start");

    //If there is system_header_start, default value for private sections are false
    private_stream_1_Count=0;
    private_stream_2_Count=false;

    //Parsing
    int32u rate_bound;
    int8u  audio_bound, video_bound;
    BS_Begin();
    Mark_1();
    Get_S3 (22, rate_bound,                                     "rate_bound"); Param_Info(rate_bound*400, " bps");
    Mark_1();
    Get_S1 ( 6, audio_bound,                                    "audio_bound");
    Info_SB(    fixed_flag,                                     "fixed_flag"); Param_Info(MpegPs_System_Fixed[fixed_flag]);
    Skip_SB(                                                    "CSPS_flag");
    Skip_SB(                                                    "system_audio_lock_flag");
    Skip_SB(                                                    "system_video_lock_flag");
    Mark_1();
    Get_S1 ( 5, video_bound,                                    "video_bound");
    Skip_SB(                                                    "packet_rate_restriction_flag");
    Skip_S1( 7,                                                 "reserved_byte");
    bool one=false;
    if (Element_IsNotFinished())
        Peek_SB(one);
    while (one)
    {
        Element_Begin();
        int16u STD_buffer_size_bound;
        int8u stream_id, stream_id_extension=0;
        bool STD_buffer_bound_scale;
        Get_S1 ( 8, stream_id,                                  "stream_id"); Param_Info(MpegPs_stream_id(stream_id));
        Element_Name(Ztring().From_CC1(stream_id));
        Element_Info(MpegPs_stream_id(stream_id));
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
        Get_S2 (13, STD_buffer_size_bound,                      "STD_buffer_size_bound"); Param_Info(Ztring::ToZtring(STD_buffer_size_bound*(STD_buffer_bound_scale?1024:128)) + _T(" bytes"));
        Element_End();

        FILLING_BEGIN();
            if (stream_id==0xBD)
                private_stream_1_Count=(int8u)-1;
            if (stream_id==0xBF)
                private_stream_2_Count=true;
            if (stream_id==0xFD)
                extension_stream_Count=(int8u)-1;
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
            if (Parser.Complete_Stream->Streams[Pos].stream_type)
            {
                Streams[Pos].stream_type=Parser.Complete_Stream->Streams[Pos].stream_type;
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
        if (private_stream_1_ID)
            Element_Info(Ztring::ToZtring(private_stream_1_ID, 16));
    }

    if (!Streams_Private1[private_stream_1_ID].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            //From TS
            video_stream_Count=0;
            audio_stream_Count=0;
            private_stream_1_Count=1;
            private_stream_2_Count=false;
            extension_stream_Count=0;
            private_stream_1_ID=FromTS_stream_type;
            private_stream_1_Offset=0;
            Streams_Private1[private_stream_1_ID].stream_type=FromTS_stream_type;
        }

        //Registering
        if (!Status[IsAccepted])
        {
            Data_Accept("MPEG-PS");
            if (!IsSub)
                Fill(Stream_General, 0, General_Format, "MPEG-PS");
        }
        Streams[start_code].StreamIsRegistred=true;
        Streams_Private1[private_stream_1_ID].StreamIsRegistred=true;
        Streams_Private1[private_stream_1_ID].Searching_Payload=true;
        Streams_Private1[private_stream_1_ID].Searching_TimeStamp_Start=true;
        Streams_Private1[private_stream_1_ID].Searching_TimeStamp_End=true;

        //New parsers
        #if MEDIAINFO_EVENTS
            Element_Code=private_stream_1_ID;
        #endif //MEDIAINFO_EVENTS
        Streams_Private1[private_stream_1_ID].Parsers.push_back(private_stream_1_ChooseParser());
        if (Streams_Private1[private_stream_1_ID].Parsers[Streams_Private1[private_stream_1_ID].Parsers.size()-1])
            Open_Buffer_Init(Streams_Private1[private_stream_1_ID].Parsers[Streams_Private1[private_stream_1_ID].Parsers.size()-1]);
        else
        {
            Streams_Private1[private_stream_1_ID].Parsers.clear();
            #if defined(MEDIAINFO_AC3_YES)
            {
                File_Ac3* Parser=new File_Ac3;
                if (Streams_Private1[private_stream_1_ID].stream_type==0 || Streams_Private1[private_stream_1_ID].stream_type==0x06) //None or private
                    Parser->Frame_Count_Valid=2;
                Open_Buffer_Init(Parser);
                Streams_Private1[private_stream_1_ID].Parsers.push_back(Parser);
            }
            #endif
            #if defined(MEDIAINFO_DTS_YES)
            {
                File_Dts* Parser=new File_Dts;
                if (Streams_Private1[private_stream_1_ID].stream_type==0 || Streams_Private1[private_stream_1_ID].stream_type==0x06) //None or private
                    Parser->Frame_Count_Valid=2;
                Open_Buffer_Init(Parser);
                Streams_Private1[private_stream_1_ID].Parsers.push_back(Parser);
            }
            #endif
        }
    }

    //Specific
    #if defined(MEDIAINFO_AES3_YES)
        if (FromTS && FromTS_format_identifier==0x42535344 && PTS!=(int64u)-1) //"BSSD"
            ((File_Aes3*)Streams_Private1[private_stream_1_ID].Parsers[0])->PTS=PTS;
    #endif

    //Demux
    #if MEDIAINFO_DEMUX
        if (Streams_Private1[private_stream_1_ID].Searching_Payload)
        {
            StreamIDs[StreamIDs_Size-1]=Element_Code;
            Element_Code=private_stream_1_ID; //The upper level ID is filled by Element_Code in the common code
            StreamIDs_Width[StreamIDs_Size]=2;
            ParserIDs[StreamIDs_Size]=MediaInfo_Parser_MpegPs_Ext;
            StreamIDs_Size++;
            Demux(Buffer+Buffer_Offset+private_stream_1_Offset, (size_t)(Element_Size-private_stream_1_Offset), ContentType_MainStream);
            StreamIDs_Size--;
            Element_Code=StreamIDs[StreamIDs_Size-1];
        }
    #endif //MEDIAINFO_DEMUX

    //Parsing
    if (Element_Offset<private_stream_1_Offset)
        Skip_XX(private_stream_1_Offset-Element_Offset,         "DVD-Video data");

    xxx_stream_Parse(Streams_Private1[private_stream_1_ID], private_stream_1_Count);
}

//---------------------------------------------------------------------------
bool File_MpegPs::private_stream_1_Choose_DVD_ID()
{
    private_stream_1_IsDvdVideo=false;

    if (Element_Size<4)
        return false;

    //Parsing
    int8u  CodecID;
    Get_B1 (CodecID,                                            "CodecID");

    //Testing
    //Subtitles (CVD)
         if (CodecID>=0x00 && CodecID<=0x0F)
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
                 if (CodecID>=0x00 && CodecID<=0x0F)
                ; //Seems to not work with subtitles, to be confirmed
            //Subtitles (DVD)
                 if (CodecID>=0x20 && CodecID<=0x3F)
                ; //Seems to not work with subtitles, to be confirmed
            //Subtitles (SVCD)
                 if (CodecID>=0x70 && CodecID<=0x7F)
                ; //Seems to not work with subtitles, to be confirmed
            //AC3
            else if (CodecID>=0x80 && CodecID<=0x87)
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
            else if (CodecID>=0xA0 && CodecID<=0xAF)
                ;
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
    if (FromTS)
    {
        if (FromTS_format_identifier==0x42535344) //"BSSD"
        {
            return ChooseParser_AES3(); //AES3 (SMPTE 320M)
        }
        switch (FromTS_stream_type)
        {
            case 0x0F : return ChooseParser_Adts(); //ADTS
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
            default   : switch (FromTS_descriptor_tag)
                        {
                            case 0x56 : return ChooseParser_NULL(); //Teletext
                            case 0x59 : return ChooseParser_NULL(); //DVB Subtiles
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
                                            return ChooseParser_NULL();
                        }
        }
    }
    else if (private_stream_1_IsDvdVideo)
    {
        //Subtitles (CVD)
             if (private_stream_1_ID>=0x00 && private_stream_1_ID<=0x0F)
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
            return ChooseParser_NULL();
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
            return ChooseParser_NULL();
    }
}

//---------------------------------------------------------------------------
const ZenLib::Char* File_MpegPs::private_stream_1_ChooseExtension()
{
    if (FromTS)
    {
        switch (private_stream_1_ID)
        {
            case 0x80 : return _T(".pcm"); //PCM
            case 0x81 : return _T(".ac3"); //AC3
            case 0x83 :
            case 0x87 : return _T(".dd+"); //AC3+
            case 0x86 : return _T(".dts"); //DTS
            case 0xEA : return _T(".vc1"); //DTS
            default   : return _T(".raw");
        }
    }
    else
    {
        //Subtitles
             if (private_stream_1_ID>=0x20 && private_stream_1_ID<=0x3F)
            return _T(".sub");
        //AC3
        else if (private_stream_1_ID>=0x80 && private_stream_1_ID<=0x87)
            return _T(".ac3");
        //DTS
        else if (private_stream_1_ID>=0x88 && private_stream_1_ID<=0x8F)
            return _T(".dts");
        //SDDS
        else if (private_stream_1_ID>=0x90 && private_stream_1_ID<=0x97)
            return _T(".sdds");
        //DTS
        else if (private_stream_1_ID>=0x98 && private_stream_1_ID<=0x9F)
            return _T(".dts");
        //PCM
        else if (private_stream_1_ID>=0xA0 && private_stream_1_ID<=0xAF)
            return _T(".pcm");
        //MLP
        else if (private_stream_1_ID>=0xB0 && private_stream_1_ID<=0xBF)
            return _T(".dd+");
        //AC3+
        else if (private_stream_1_ID>=0xC0 && private_stream_1_ID<=0xCF)
            return _T(".dd+");
        else
            return _T(".raw");
    }
}

//---------------------------------------------------------------------------
void File_MpegPs::private_stream_1_Element_Info()
{
    if (FromTS)
    {
        switch (private_stream_1_ID)
        {
            case 0x80 : Element_Info("PCM"); return;
            case 0x81 : Element_Info("AC3"); return;
            case 0x83 :
            case 0x87 : Element_Info("AC3+"); return;
            case 0x86 : Element_Info("DTS"); return;
            case 0xEA : Element_Info("VC1"); return;
            default   : return;
        }
    }
    else
    {
        //Subtitles
             if (private_stream_1_ID>=0x20 && private_stream_1_ID<=0x3F)
            Element_Info("RLE");
        //AC3
        else if (private_stream_1_ID>=0x80 && private_stream_1_ID<=0x87)
            Element_Info("AC3");
        //DTS
        else if (private_stream_1_ID>=0x88 && private_stream_1_ID<=0x8F)
            Element_Info("DTS");
        //SDDS
        else if (private_stream_1_ID>=0x90 && private_stream_1_ID<=0x97)
            Element_Info("SDDS");
        //DTS
        else if (private_stream_1_ID>=0x98 && private_stream_1_ID<=0x9F)
            Element_Info("DTS");
        //PCM
        else if (private_stream_1_ID>=0xA0 && private_stream_1_ID<=0xAF)
            Element_Info("LPCM");
        //MLP
        else if (private_stream_1_ID>=0xB0 && private_stream_1_ID<=0xBF)
            Element_Info("MLP");
        //AC3+
        else if (private_stream_1_ID>=0xC0 && private_stream_1_ID<=0xCF)
            Element_Info("AC3+");
        //PS2
        else if (private_stream_1_ID>=0xC0 && private_stream_1_ID<=0xCF)
            Element_Info("PS2");
    }
}

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
        private_stream_2_Count=false;
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

    if (!Streams[start_code].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            video_stream_Count=0;
            audio_stream_Count=1;
            private_stream_1_Count=0;
            private_stream_2_Count=false;
            extension_stream_Count=0;
            Streams[start_code].stream_type=FromTS_stream_type;
        }

        //If we have no Streams map --> Registering the Streams as MPEG Audio
        if (Streams[start_code].stream_type==0)
        {
            if (MPEG_Version==2)
                Streams[start_code].stream_type=0x04; //MPEG-2 Audio
            else
                Streams[start_code].stream_type=0x03; //MPEG-1 Audio
        }

        //Registering
        if (!Status[IsAccepted])
        {
            Data_Accept("MPEG-PS");
            if (!IsSub)
                Fill(Stream_General, 0, General_Format, "MPEG-PS");
        }
        Streams[start_code].StreamIsRegistred=true;

        //New parsers
        switch (Streams[start_code].stream_type)
        {
            case 0x0F : Streams[start_code].Parsers.push_back(ChooseParser_Adts()); Open_Buffer_Init(Streams[start_code].Parsers[0]); break;
            case 0x03 :
            case 0x04 : Streams[start_code].Parsers.push_back(ChooseParser_Mpega()); Open_Buffer_Init(Streams[start_code].Parsers[0]); break;
            default   :
                        #if defined(MEDIAINFO_MPEGA_YES)
                        {
                            File_Mpega* Parser=new File_Mpega;
                            Open_Buffer_Init(Parser);
                            Parser->Frame_Count_Valid=1;
                            Streams[start_code].Parsers.push_back(Parser);
                        }
                        #endif
                        #if defined(MEDIAINFO_ADTS_YES)
                        {
                            File_Adts* Parser=new File_Adts;
                            Open_Buffer_Init(Parser);
                            Streams[start_code].Parsers.push_back(Parser);
                        }
                        #endif
        }
    }

    //Demux
    Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);

    //Parsing
    xxx_stream_Parse(Streams[start_code], audio_stream_Count);
}

//---------------------------------------------------------------------------
void File_MpegPs::video_stream()
{
    Element_Name("Video");

    if (!Streams[start_code].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            video_stream_Count=1;
            audio_stream_Count=0;
            private_stream_1_Count=0;
            private_stream_2_Count=false;
            extension_stream_Count=0;
            Streams[start_code].stream_type=FromTS_stream_type;
        }

        //Registering
        if (!Status[IsAccepted])
        {
            Data_Accept("MPEG-PS");
            if (!IsSub)
                Fill(Stream_General, 0, General_Format, "MPEG-PS");
        }
        Streams[start_code].StreamIsRegistred=true;

        //New parsers
        switch (Streams[start_code].stream_type)
        {
            case 0x10 : Streams[start_code].Parsers.push_back(ChooseParser_Mpeg4v()); Open_Buffer_Init(Streams[start_code].Parsers[0]); break;
            case 0x1B : Streams[start_code].Parsers.push_back(ChooseParser_Avc()   ); Open_Buffer_Init(Streams[start_code].Parsers[0]); break;
            case 0x01 :
            case 0x02 :
            case 0x80 : Streams[start_code].Parsers.push_back(ChooseParser_Mpegv() ); Open_Buffer_Init(Streams[start_code].Parsers[0]); break;
            default   :
                        #if defined(MEDIAINFO_MPEGV_YES)
                        {
                            File_Mpegv* Parser=new File_Mpegv;
                            Open_Buffer_Init(Parser);
                            Parser->MPEG_Version=MPEG_Version;
                            Parser->ShouldContinueParsing=true;
                            Streams[start_code].Parsers.push_back(Parser);
                        }
                        #endif
                        #if defined(MEDIAINFO_AVC_YES)
                        {
                            File_Avc* Parser=new File_Avc;
                            Open_Buffer_Init(Parser);
                            Parser->ShouldContinueParsing=true;
                            Streams[start_code].Parsers.push_back(Parser);
                        }
                        #endif
                        #if defined(MEDIAINFO_MPEG4V_YES)
                        {
                            File_Mpeg4v* Parser=new File_Mpeg4v;
                            Open_Buffer_Init(Parser);
                            Parser->ShouldContinueParsing=true;
                            Streams[start_code].Parsers.push_back(Parser);
                        }
                        #endif
                        #if defined(MEDIAINFO_AVSV_YES)
                        {
                            File_AvsV* Parser=new File_AvsV;
                            Open_Buffer_Init(Parser);
                            Parser->ShouldContinueParsing=true;
                            Streams[start_code].Parsers.push_back(Parser);
                        }
                        #endif
        }
    }

    //Demux
    if (!(FromTS_stream_type==0x20
        #if MEDIAINFO_DEMUX
             && SubStream_Demux
        #endif //MEDIAINFO_DEMUX
        ))
        Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);

    //Parsing
    xxx_stream_Parse(Streams[start_code], video_stream_Count);

    //Saving PTS
    bool video_stream_IsFilled=false; //If parser is filled, frame count is more update, we stop testing PTS.
    bool video_stream_IsInterlaced=false; //If interlaced, we must count the number of fields, not frames
    for (size_t Pos=0; Pos<Streams[start_code].Parsers.size(); Pos++)
    {
        if (Streams[start_code].Parsers[Pos]->Status[IsFilled])
            video_stream_IsFilled=true;
        if (!video_stream_IsFilled && Streams[start_code].Parsers[Pos]->Frame_Count_InThisBlock)
        {
            video_stream_PTS_FrameCount+=Streams[start_code].Parsers[Pos]->Frame_Count_InThisBlock; //TODO: check if there are more than 1 parser with Frame_Count_InThisBlock>0
            video_stream_IsInterlaced=Streams[start_code].Parsers[Pos]->Retrieve(Stream_Video, 0, Video_ScanType)==_T("Intelaced"); //TODO: SanType is written too late, not available on time, wrong data
        }
    }
    if (!video_stream_IsFilled && !video_stream_Unlimited && PTS!=(int64u)-1)
    {
        if (PTS>=0x100000000LL)
            video_stream_PTS_MustAddOffset=true;
        if (video_stream_IsInterlaced)
            video_stream_PTS_FrameCount*=2; //Count fields, not frames
        int64u PTS_Calculated=((video_stream_PTS_MustAddOffset && PTS<0x100000000LL)?0x200000000LL:0)+PTS; //With Offset if needed
        if (video_stream_PTS_FrameCount<=1)
            video_stream_PTS.push_back(PTS_Calculated);
        else if (!video_stream_PTS.empty())
        {
            //Calculating the average PTS per frame if there is more than 1 frame between PTS
            int64u PTS_Calculated_Base=video_stream_PTS[video_stream_PTS.size()-1];
            int64u PTS_Calculated_PerFrame=(PTS_Calculated-PTS_Calculated_Base)/video_stream_PTS_FrameCount;
            for (size_t Pos=0; Pos<video_stream_PTS_FrameCount; Pos++)
                video_stream_PTS.push_back(PTS_Calculated_Base+(1+Pos)*PTS_Calculated_PerFrame);
        }
        video_stream_PTS_FrameCount=0;
    }
}

//---------------------------------------------------------------------------
// Packet "FA"
void File_MpegPs::SL_packetized_stream()
{
    Element_Name("SL-packetized_stream");

    if (!Streams[start_code].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            video_stream_Count=0;
            audio_stream_Count=1;
            private_stream_1_Count=0;
            private_stream_2_Count=false;
            extension_stream_Count=0;
            Streams[start_code].stream_type=FromTS_stream_type;
        }

        //Registering
        Streams[start_code].StreamIsRegistred=true;
        if (!Status[IsAccepted])
            Data_Accept("MPEG-PS");
        Streams[start_code].Searching_TimeStamp_Start=true;

        //New parsers
        if (FromTS_stream_type)
            switch (FromTS_stream_type)
            {
                case 0x0F :
                            #if defined(MEDIAINFO_ADTS_YES)
                            {
                                File_Adts* Parser=new File_Adts;
                                Parser->Frame_Count_Valid=1;
                                Open_Buffer_Init(Parser);
                                Streams[start_code].Parsers.push_back(Parser);
                            }
                            #endif
                            break;

                case 0x11 :
                            #if defined(MEDIAINFO_MPEG4_YES)
                            {
                                File_Aac* Parser=new File_Aac;
                                Parser->DecSpecificInfoTag=DecSpecificInfoTag;
                                Parser->SLConfig=SLConfig;
                                Open_Buffer_Init(Parser);
                                Streams[start_code].Parsers.push_back(Parser);
                            }
                            #endif
                            break;
                default   : ;
            }
        else
        {
            #if defined(MEDIAINFO_ADTS_YES)
            {
                File_Adts* Parser=new File_Adts;
                Parser->Frame_Count_Valid=1;
                Open_Buffer_Init(Parser);
                Streams[start_code].Parsers.push_back(Parser);
            }
            #endif
            #if defined(MEDIAINFO_MPEG4_YES)
            {
                File_Aac* Parser=new File_Aac;
                Parser->DecSpecificInfoTag=DecSpecificInfoTag;
                Parser->SLConfig=SLConfig;
                Open_Buffer_Init(Parser);
                Streams[start_code].Parsers.push_back(Parser);
            }
            #endif
        }
    }

    //Parsing
    if (SLConfig) //LATM
    {
        BS_Begin();
        int8u paddingBits=0;
        bool paddingFlag=false, idleFlag=false/*not in spec*/, DegPrioflag=false/*not ins specs*/, OCRflag=false,
             accessUnitStartFlag=false/*should be "previous-SL packet has accessUnitEndFlag"*/, accessUnitEndFlag=false/*Should be "subsequent-SL packet has accessUnitStartFlag"*/,
             decodingTimeStampFlag=false/*not in spec*/, compositionTimeStampFlag=false/*not in spec*/,
             instantBitrateFlag=false/*not in spec*/;
        if (SLConfig->useAccessUnitStartFlag)
            Get_SB (accessUnitStartFlag,                        "accessUnitStartFlag");
        if (SLConfig->useAccessUnitEndFlag)
            Get_SB (accessUnitEndFlag,                          "accessUnitEndFlag");
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
        Skip_XX(Element_Size-Element_Offset,                    "AAC (raw)");
    }

    //Demux
    if (MediaInfoLib::Config.Demux_Get())
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

    //Parsing
    xxx_stream_Parse(Streams[start_code], audio_stream_Count);
}

//---------------------------------------------------------------------------
// Packet "FD"
void File_MpegPs::extension_stream()
{
    Element_Name("With Extension");
    Element_Info(MpegPs_stream_id_extension(stream_id_extension));

    if (!Streams_Extension[stream_id_extension].StreamIsRegistred)
    {
        //For TS streams, which does not have Start chunk
        if (FromTS)
        {
            video_stream_Count=0;
            audio_stream_Count=0;
            private_stream_1_Count=0;
            private_stream_2_Count=false;
            extension_stream_Count=1;
        }

        //Registering
        if (!Status[IsAccepted])
            Data_Accept("MPEG-PS");
        Streams[start_code].StreamIsRegistred=true;
        Streams_Extension[stream_id_extension].StreamIsRegistred=true;
        Streams_Extension[stream_id_extension].Searching_Payload=true;
        Streams_Extension[stream_id_extension].Searching_TimeStamp_Start=true;
        Streams_Extension[stream_id_extension].Searching_TimeStamp_End=true;

        //New parsers
            if ((stream_id_extension>=0x55 && stream_id_extension<=0x5F)
             || (stream_id_extension==0x75 && stream_id_extension<=0x7F))
             Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_VC1());
        else if (stream_id_extension>=0x60 && stream_id_extension<=0x6F)
             Streams_Extension[stream_id_extension].Parsers.push_back(ChooseParser_Dirac());
        else if (stream_id_extension==0x71)
        {
            Streams_Extension[0x72].Parsers.clear(); //In case of HD part before Core part
            Streams_Extension[0x71].Parsers.push_back(ChooseParser_DTS());
            Streams_Extension[0x71].Parsers.push_back(ChooseParser_AC3());
        }
        else if (stream_id_extension==0x76)
        {
            Streams_Extension[0x72].Parsers.clear(); //In case of HD part before Core part
            Streams_Extension[0x76].Parsers.push_back(ChooseParser_AC3());
        }
        else if (stream_id_extension==0x72)
        {
            if (Streams_Extension[0x71].Parsers.empty() && Streams_Extension[0x76].Parsers.empty())
            {
                Streams_Extension[0x72].Parsers.push_back(ChooseParser_DTS());
                Streams_Extension[0x72].Parsers.push_back(ChooseParser_AC3());
            }
            /*
                 if (!Streams_Extension[0x71].Parsers.empty())
                ; //Streams_Extension[0x72].Parsers.push_back(Streams_Extension[0x71].Parsers[0]); //Binding 0x72 to 0x71 (DTS-HD)
            else if (!Streams_Extension[0x76].Parsers.empty())
                ; //Streams_Extension[0x72].Parsers.push_back(Streams_Extension[0x76].Parsers[0]); //Binding 0x72 to 0x76 (TrueHD)
            else
            {
                //Audio core is not yet ready, waiting
                Skip_XX(Element_Size,                           "Waiting for core data...");
                return;
            }
            */
        }
        for (size_t Pos=0; Pos<Streams_Extension[stream_id_extension].Parsers.size(); Pos++)
            Open_Buffer_Init(Streams_Extension[stream_id_extension].Parsers[Pos]);
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
            xxx_stream_Parse(Streams_Extension[0x71], extension_stream_Count);
        if (!Streams_Extension[0x76].Parsers.empty())
            xxx_stream_Parse(Streams_Extension[0x76], extension_stream_Count);
    }
    else
        xxx_stream_Parse(Streams_Extension[stream_id_extension], extension_stream_Count);
}

//---------------------------------------------------------------------------
const ZenLib::Char* File_MpegPs::extension_stream_ChooseExtension()
{
    //AC3
        if ((stream_id_extension>=0x55 && stream_id_extension<=0x5F)
         || (stream_id_extension==0x75 && stream_id_extension<=0x7F))
        return _T(".vc1");
    //AC3+
    else if (stream_id_extension>=0x60 && stream_id_extension<=0x6F)
        return _T(".dirac");
    else if (stream_id_extension==0x71)
        return private_stream_1_ChooseExtension();
    else
        return _T(".raw");
}

//***************************************************************************
// xxx_stream helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegPs::xxx_stream_Parse(ps_stream &Temp, int8u &xxx_Count)
{
    switch (start_code)
    {
        case 0xBD :
        case 0xFD :
            //PTS
            if (PTS!=(int64u)-1)
            {
                if (Streams[start_code].Searching_TimeStamp_End)
                {
                    Temp.TimeStamp_End.PTS.File_Pos=File_Offset+Buffer_Offset;
                    Temp.TimeStamp_End.PTS.TimeStamp=PTS;
                }
                if (Searching_TimeStamp_Start && Temp.Searching_TimeStamp_Start)
                {
                    Temp.TimeStamp_Start.PTS.TimeStamp=PTS;
                    Temp.Searching_TimeStamp_Start=false;
                }
            }

            //DTS
            if (DTS!=(int64u)-1)
            {
                if (Streams[start_code].Searching_TimeStamp_End)
                {
                    Temp.TimeStamp_End.DTS.File_Pos=File_Offset+Buffer_Offset;
                    Temp.TimeStamp_End.DTS.TimeStamp=DTS;
                }
                if (Searching_TimeStamp_Start && DTS!=(int64u)-1 && Temp.Searching_TimeStamp_Start)
                {
                    Temp.TimeStamp_Start.DTS.TimeStamp=DTS;
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
        if (start_code==0xBD)
            private_stream_1_Element_Info();
    #endif //MEDIAINFO_TRACE

    if (1 //Temp.StreamKind==Stream_Video
    && ((DTS!=(int64u)-1 && Temp.TimeStamp_End.DTS.TimeStamp!=(int64u)-1)
      || (PTS!=(int64u)-1 && Temp.TimeStamp_End.PTS.TimeStamp!=(int64u)-1)))
        Temp.FrameCount_AfterLast_TimeStamp_End=0;

    for (size_t Pos=0; Pos<Temp.Parsers.size(); Pos++)
        if (Temp.Parsers[Pos] && !Temp.Parsers[Pos]->Status[IsFinished])
        {
            //PTS/DTS
            if (Temp.Parsers[Pos]->PTS_DTS_Needed)
            {
                if (PCR!=(int64u)-1)
                    Temp.Parsers[Pos]->PCR=PCR;
                if (PTS!=(int64u)-1)
                    Temp.Parsers[Pos]->PTS=PTS*1000000/90;
                if (DTS!=(int64u)-1)
                    Temp.Parsers[Pos]->DTS=DTS*1000000/90;
            }

            #if MEDIAINFO_TRACE
                if (Temp.Parsers.size()>1)
                    Element_Begin("Test");
            #endif //MEDIAINFO_TRACE
            Open_Buffer_Continue(Temp.Parsers[Pos], Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
            #if MEDIAINFO_TRACE
                if (Temp.Parsers.size()>1)
                    Element_End();
            #endif //MEDIAINFO_TRACE

            if ((Temp.Parsers[Pos]->Status[IsAccepted] && Temp.Parsers[Pos]->Status[IsFinished])
             || (!Parsing_End_ForDTS && Temp.Parsers[Pos]->Status[IsFilled]))
            {
                if (MediaInfoLib::Config.ParseSpeed_Get()<1 && Temp.Parsers[Pos]->Count_Get(Stream_Video)==0) //TODO: speed improvement, we do this only for CC
                    Temp.Searching_Payload=false;
                if (xxx_Count>0)
                    xxx_Count--;
            }
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
        }

    if (!Temp.Parsers.empty())
        Temp.FrameCount_AfterLast_TimeStamp_End+=Temp.Parsers[0]->Frame_Count_InThisBlock;

    Element_Show();

    #if MEDIAINFO_EVENTS
        if (DTS==(int64u)-1)
            DTS=PTS;

        //New PES
        if (MpegPs_PES_FirstByte_IsAvailable && MpegPs_PES_FirstByte_Value)
        {
            //Demux of substream data
            if (FromTS_stream_type==0x1B && SubStream_Demux)
            {
                if (!SubStream_Demux->Buffers.empty() && SubStream_Demux->Buffers[0]->DTS<DTS)
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
            if (SubStream_Demux->Buffers.empty() || SubStream_Demux->Buffers[SubStream_Demux->Buffers.size()-1]->DTS!=DTS)
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
                SubStream_Demux->Buffers[Buffers_Pos]->DTS=DTS;
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
    #endif //MEDIAINFO_EVENTS
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
        //Getting start_code
        int8u start_code=Buffer[Buffer_Offset+3];

        //Searching start
        if (Streams[start_code].Searching_Payload)
        {
            if (start_code!=0xBD || !private_stream_1_IsDvdVideo) //Not (private_stream_1 and IsDvdVideo)
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
        if (Streams[start_code].Searching_TimeStamp_End)
        {
            switch(start_code)
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
        switch(start_code)
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
                    if (Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset-1]==0x00 || Buffer_Offset>=Buffer_Size)
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
        return false; //Sync is OK, but start_code is not available
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
        File_Mpegv* Handle=new File_Mpegv;
        Handle->MPEG_Version=MPEG_Version;
        Handle->ShouldContinueParsing=true;
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Video);
        Handle->Fill(Stream_Video, 0, Video_Format, "MPEG Video");
        switch (FromTS_stream_type)
        {
            case 0x01 : Handle->Fill(Stream_Video, 0, Video_Codec, "MPEG-1V");
                        Handle->Fill(Stream_Video, 0, Video_Format_Version, "Version 1"); break;
            case 0x02 : Handle->Fill(Stream_Video, 0, Video_Codec, "MPEG-2V");
                        Handle->Fill(Stream_Video, 0, Video_Format_Version, "Version 2"); break;
            default   : Handle->Fill(Stream_Video, 0, Video_Codec, "MPEG-V");
        }
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Mpeg4v()
{
    //Filling
    #if defined(MEDIAINFO_MPEG4V_YES)
        File_Mpeg4v* Handle=new File_Mpeg4v;
        Handle->Frame_Count_Valid=1;
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Video);
        Handle->Fill(Stream_Video, 0, Video_Codec, "MPEG-4V");
        Handle->Fill(Stream_Video, 0, Video_Format, "MPEG-4 Visual");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Avc()
{
    //Filling
    #if defined(MEDIAINFO_AVC_YES)
        File_Avc* Handle=new File_Avc;
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Video);
        Handle->Fill(Stream_Video, 0, Video_Codec,  "AVC");
        Handle->Fill(Stream_Video, 0, Video_Format, "AVC");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_VC1()
{
    //Filling
    #if defined(MEDIAINFO_VC1_YES)
        File__Analyze* Handle=new File_Vc1;
        ((File_Vc1*)Handle)->Frame_Count_Valid=30;
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Video);
        Handle->Fill(Stream_Video, 0, Video_Codec,  "VC-1");
        Handle->Fill(Stream_Video, 0, Video_Format, "VC-1");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Dirac()
{
    //Filling
    #if defined(MEDIAINFO_DIRAC_YES)
        File__Analyze* Handle=new File_Dirac;
        ((File_Dirac*)Handle)->Frame_Count_Valid=1;
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Video);
        Handle->Fill(Stream_Video, 0, Video_Codec,  "Dirac");
        Handle->Fill(Stream_Video, 0, Video_Format, "Dirac");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Mpega()
{
    //Filling
    #if defined(MEDIAINFO_MPEGA_YES)
        File_Mpega* Handle=new File_Mpega;
        Handle->Frame_Count_Valid=1;
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Audio);
        Handle->Fill(Stream_Audio, 0, Audio_Format, "MPEG Audio");
        switch (FromTS_stream_type)
        {
            case 0x03 : Handle->Fill(Stream_Audio, 0, Audio_Codec,  "MPEG-1A");
                        Handle->Fill(Stream_Audio, 0, Audio_Format_Version, "Version 1"); break;
            case 0x04 : Handle->Fill(Stream_Audio, 0, Audio_Codec,  "MPEG-2A");
                        Handle->Fill(Stream_Audio, 0, Audio_Format_Version, "Version 2"); break;
            default   : Handle->Fill(Stream_Audio, 0, Audio_Codec,  "MPEG-A"); break;
        }
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_Adts()
{
    //Filling
    #if defined(MEDIAINFO_ADTS_YES)
        File__Analyze* Handle=new File_Adts;
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Audio);
        Handle->Fill(Stream_Audio, 0, Audio_Codec,  "AAC");
        Handle->Fill(Stream_Audio, 0, Audio_Format, "AAC");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_AC3()
{
    //Filling
    #if defined(MEDIAINFO_AC3_YES)
        File_Ac3* Handle=new File_Ac3();
        Handle->Frame_Count_Valid=2; //2 frames to be sure
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Audio);
        Handle->Fill(Stream_Audio, 0, Audio_Format, private_stream_1_ID==0x83?"E-AC-3":"AC-3");
        Handle->Fill(Stream_Audio, 0, Audio_Codec, private_stream_1_ID==0x83?"AC3+":"AC3");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_DTS()
{
    //Filling
    #if defined(MEDIAINFO_DTS_YES)
        File__Analyze* Handle=new File_Dts();
        ((File_Dts*)Handle)->Frame_Count_Valid=2;
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Audio);
        Handle->Fill(Stream_Audio, 0, Audio_Format, "DTS");
        Handle->Fill(Stream_Audio, 0, Audio_Codec, "DTS");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_SDDS()
{
    //Filling
    #if defined(MEDIAINFO_SDDS_YES)
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Audio);
        Handle->Fill(Stream_Audio, StreamPos_Last, Audio_Format, "SDDS");
        Handle->Fill(Stream_Audio, StreamPos_Last, Audio_Codec,  "SDDS");
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Handle->Stream_Prepare(Stream_Audio);
        Handle->Fill(Stream_Audio, 0, Audio_Format, "SDDS");
        Handle->Fill(Stream_Audio, 0, Audio_Codec,  "SDDS");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_AAC()
{
    //Filling
    #if defined(MEDIAINFO_ADTS_YES)
        //Filling
        File__Analyze* Handle=new File_Aac();
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Audio);
        Handle->Fill(Stream_Audio, 0, Audio_Format, "AAC");
        Handle->Fill(Stream_Audio, 0, Audio_Codec,  "AAC");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_PCM()
{
    //Filling
    #if defined(MEDIAINFO_PCM_YES)
        File__Analyze* Handle=new File_Pcm();
        Ztring Codec;
        switch (private_stream_1_ID)
        {
            case 0x80 : Codec=_T("M2TS"); break;
            case 0x83 : Codec=_T("EVOB"); break;
            default   : Codec=_T("VOB");
        }
        ((File_Pcm*)Handle)->Codec=Codec;
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Audio);
        Handle->Fill(Stream_Audio, 0, Audio_Format, "PCM");
        Handle->Fill(Stream_Audio, 0, Audio_Codec,  "PCM");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_RLE()
{
    //Filling
    #if defined(MEDIAINFO_RLE_YES)
        File__Analyze* Handle=new File_Rle();
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Text);
        Handle->Fill(Stream_Text, 0, Text_Format, "RLE");
        Handle->Fill(Stream_Text, 0, Text_Codec,  "RLE");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_PGS()
{
    //Filling
    #if defined(MEDIAINFO_PGS_YES)
        File__Analyze* Handle=new File_Pgs();
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Text);
        Handle->Fill(Stream_Text, 0, Text_Format, "PGS");
        Handle->Fill(Stream_Text, 0, Text_Codec,  "PGS");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_AES3()
{
    //Filling
    #if defined(MEDIAINFO_AES3_YES)
        File__Analyze* Handle=new File_Aes3();
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Audio);
        Handle->Fill(Stream_Audio, 0, Audio_Format, "AES3");
        Handle->Fill(Stream_Audio, 0, Audio_Codec,  "AES3");
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_PS2()
{
    //Filling
    #if defined(MEDIAINFO_PS2A_YES)
        File__Analyze* Handle=new File_Ps2Audio();
    #else
        //Filling
        File__Analyze* Handle=new File_Unknown();
        Open_Buffer_Init(Handle);
        Handle->Stream_Prepare(Stream_Audio);
    #endif
    return Handle;
}

//---------------------------------------------------------------------------
File__Analyze* File_MpegPs::ChooseParser_NULL()
{
    //Filling
    //File__Analyze* Handle=new File__Analyze();
    //return Handle;
    return NULL;
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



