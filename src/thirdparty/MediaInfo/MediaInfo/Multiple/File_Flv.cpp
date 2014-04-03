/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
//
// Examples:
// http://samples.mplayerhq.hu/FLV/
//
// Reverse engineering
// http://osflash.org/documentation/amf/astypes
//
//---------------------------------------------------------------------------

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
#if defined(MEDIAINFO_FLV_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Flv.h"
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_AAC_YES)
    #include "MediaInfo/Audio/File_Aac.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_RM_YES)
    #include "MediaInfo/Multiple/File_Rm.h"
#endif
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include <algorithm>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
const int16u  Flv_Channels[]=
{
    1,
    2,
};

const int16u  Flv_Resolution[]=
{
    8,
    16,
};

const int16u Flv_SamplingRate[]=
{
    5500,
    11025,
    22050,
    44100,
    8000, //Special case for Nellymoser 8kHz mono
};

const char* Flv_Format_Audio[16]=
{
    "PCM",
    "ADPCM",
    "MPEG Audio",
    "PCM",
    "Nellymoser", //16 KHz
    "Nellymoser", //8 KHz
    "Nellymoser",
    "ADPCM",
    "ADPCM",
    "",
    "AAC",
    "Speex",
    "",
    "",
    "MPEG Audio", //8 KHz
    "",
};

const char* Flv_Format_Profile_Audio[16]=
{
    "",
    "",
    "Layer 3",
    "",
    "",
    "",
    "",
    "A-law",
    "U-law",
    "",
    "",
    "",
    "",
    "",
    "Layer 3", //8 KHz
    "",
};

const char* Flv_Codec_Audio[16]=
{
    "Uncompressed",
    "ADPCM",
    "MPEG-1 Audio Layer 3",
    "",
    "Nellymoser 16kHz mono",
    "Nellymoser 8kHz mono",
    "Nellymoser",
    "ADPCM",
    "ADPCM",
    "",
    "AAC",
    "Speex",
    "",
    "",
    "MPEG Audio Layer 3",
    "",
};

const char* Flv_CodecID_Hint_Audio[16]=
{
    "",
    "",
    "MP3",
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
    "MP3", //8 KHz
    "",
};

const char* Flv_Format_Video[16]=
{
    "",
    "",
    "Sorenson Spark",
    "Screen video",
    "VP6",
    "VP6",
    "Screen video 2",
    "AVC",
    "",
    "",
    "",
    "",
    "HEVC",
    "",
    "",
    "",
};

const char* Flv_Format_Profile_Video[16]=
{
    "",
    "",
    "",
    "",
    "",
    "Alpha channel",
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
};

const char* Flv_Codec_Video[16]=
{
    "",
    "",
    "Sorenson H263",
    "Screen video",
    "On2 VP6",
    "On2 VP6 with alpha channel",
    "Screen video 2",
    "AVC",
    "",
    "",
    "",
    "",
    "HEVC",
    "",
    "",
    "",
};

const char* Flv_CodecID_Hint_Video[16]=
{
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
};

const char* Flv_H263_PictureSize[]=
{
    "custom, 1 byte",
    "custom, 2 bytes",
    "CIF (352x288)",
    "QCIF (176x144)",
    "SQCIF (128x96)",
    "320x240",
    "160x120",
    "",
};

const int16u Flv_H263_WidthHeight[8][2]=
{
    {  0,   0},
    {  0,   0},
    {352, 288},
    {176, 144},
    {128, 96},
    {320, 240},
    {160, 120},
    {0, 0},
};

const char* Flv_H263_PictureType[]=
{
    "IntraFrame",
    "InterFrame",
    "InterFrame (Disposable)",
    "",
};
const char* Flv_VP6_FrameMode[]=
{
    "IntraFrame",
    "",
};

const char* Flv_VP6_Marker[]=
{
    "VP6.1/6.2",
    "VP6.0",
};

const char* Flv_VP6_Version[]=
{
    "",
    "",
    "",
    "",
    "",
    "",
    "VP6.0/6.1",
    "VP6.0 (Electronic Arts)",
    "VP6.2",
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
    "",
    "",
    "",
    "",
    "",
    "",
};

const char* Flv_VP6_Version2[]=
{
    "VP6.0",
    "",
    "",
    "VP6.1/6.2",
};

const char* Flv_FrameType[]=
{
    "",
    "KeyFrame",
    "InterFrame",
    "InterFrame (Disposable)",
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
};

const char* Flv_TagType[]=
{
    "DOUBLE",
    "UI8",
    "SCRIPTDATASTRING",
    "SCRIPTDATAOBJECT[n]",
    "SCRIPTDATASTRING defining the MovieClip path",
    "Null",
    "Undefined",
    "UI16",
    "SCRIPTDATAVARIABLE[ECMAArrayLength]",
    "EndOfObject",
    "SCRIPTDATAVARIABLE[n]",
    "SCRIPTDATADATE",
    "SCRIPTDATALONGSTRING",
    "Unsupported",
    "Recordset",
    "XML",
    "TypedObject",
    "AMF3 data",
};

const char* Flv_Amf3Type[]=
{
    "Undefined",
    "Null",
    "Boolean-false",
    "Boolean-true",
    "Integer",
    "Number",
    "String",
    "XML",
    "Data",
    "Array",
    "Object",
    "XML String",
    "ByteArray",
};

const char* Flv_AVCPacketType(int8u Value)
{
    switch (Value)
    {
        case 0 : return "AVC sequence header";
        case 1 : return "NALU";
        case 2 : return "end of sequence";
        default: return "";
    }
}

const char* Flv_AACPacketType(int8u Value)
{
    switch (Value)
    {
        case 0 : return "AAC sequence header";
        case 1 : return "AAC Raw";
        default: return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Flv::File_Flv()
:File__Analyze()
{
    //Configuration
    ParserName=__T("FLV");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Flv;
        StreamIDs_Width[0]=2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX

    //Internal
    Stream.resize(3); //Null, Video, Audio

    //Temp
    Searching_Duration=false;
    MetaData_NotTrustable=false;
    PreviousTagSize=(int32u)-1;
    meta_filesize=(int64u)-1;
    meta_duration=0;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Flv::Streams_Fill()
{
    //Coherency
    if (Count_Get(Stream_Video) && Count_Get(Stream_Audio) && !Retrieve(Stream_Video, 0, Video_BitRate).empty() && Retrieve(Stream_Audio, 0, Audio_BitRate).empty())
    {
        Fill(Stream_General, 0, General_OverallBitRate, Retrieve(Stream_Video, 0, Video_BitRate));
        Clear(Stream_Video, 0, Video_BitRate);
    }

    //Trying to detect VFR
    std::vector<int64u> video_stream_FrameRate_Between;
    for (size_t Pos=1; Pos<video_stream_FrameRate.size(); Pos++)
        video_stream_FrameRate_Between.push_back(video_stream_FrameRate[Pos]-video_stream_FrameRate[Pos-1]);
    std::sort(video_stream_FrameRate_Between.begin(), video_stream_FrameRate_Between.end());
    if (!video_stream_FrameRate_Between.empty())
    {
        if (video_stream_FrameRate_Between[0]*0.9<video_stream_FrameRate_Between[video_stream_FrameRate_Between.size()-1]
         && video_stream_FrameRate_Between[0]*1.1>video_stream_FrameRate_Between[video_stream_FrameRate_Between.size()-1])
        {
            float Time;
            if (video_stream_FrameRate.size()>30)
                Time=((float)(video_stream_FrameRate[30]-video_stream_FrameRate[0]))/30; //30 frames for handling 30 fps rounding problems
            else
                Time=((float)(video_stream_FrameRate[video_stream_FrameRate.size()-1]-video_stream_FrameRate[0]))/(video_stream_FrameRate.size()-1); //30 frames for handling 30 fps rounding problems
            if (Time)
            {
                Fill(Stream_Video, 0, Video_FrameRate, 1000/Time);
                Fill(Stream_Video, 0, Video_FrameRate_Mode, "CFR");
            }
        }
        else
            Fill(Stream_Video, 0, Video_FrameRate_Mode, "VFR");
    }

    //Parsers
    if (Stream[Stream_Video].Parser!=NULL)
    {
        Fill(Stream[Stream_Video].Parser);
        Merge(*Stream[Stream_Video].Parser, Stream_Video, 0, 0);
    }
    if (Stream[Stream_Audio].Parser!=NULL)
    {
        Fill(Stream[Stream_Audio].Parser);
        Merge(*Stream[Stream_Audio].Parser, Stream_Audio, 0, 0);

        //Special case: AAC
        if (Retrieve(Stream_Audio, 0, Audio_Format)==__T("AAC")
         || Retrieve(Stream_Audio, 0, Audio_Format)==__T("MPEG Audio")
         || Retrieve(Stream_Audio, 0, Audio_Format)==__T("Vorbis"))
            Clear(Stream_Audio, 0, Audio_BitDepth); //Resolution is not valid for AAC / MPEG Audio / Vorbis
    }

    //Delay
    if (Stream[Stream_Video].Delay!=(int32u)-1)
    {
        Fill(Stream_Video, 0, Video_Delay, Stream[Stream_Video].Delay+Retrieve(Stream_Video, 0, Video_Delay).To_int32u(), 10, true);
        Fill(Stream_Video, 0, Video_Delay_Source, "Container");
    }
    if (Stream[Stream_Audio].Delay!=(int32u)-1)
    {
        Fill(Stream_Audio, 0, Audio_Delay, Stream[Stream_Audio].Delay+Retrieve(Stream_Audio, 0, Audio_Delay).To_int32u(), 10, true);
        Fill(Stream_Audio, 0, Audio_Delay_Source, "Container");
    }
}

//---------------------------------------------------------------------------
void File_Flv::Streams_Finish()
{
    //Duration
    //if (meta_duration)
    //    Fill(Stream_General, 0, General_Duration, meta_duration, 10, true);
    Streams_Finish_PerStream(Stream_Video);
    Streams_Finish_PerStream(Stream_Audio);

    /*
    float64 FrameRate=Retrieve(Stream_Video, 0, Video_FrameRate).To_float64();
    if (LastFrame_Time!=(int32u)-1 && FirstFrame_Time!=(int32u)-1)
        Duration_Final=LastFrame_Time-FirstFrame_Time+((LastFrame_Type==9 && FrameRate)?((int64u)(1000/FrameRate)):0);
    if (Duration_Final)
    {
        if (Count_Get(Stream_Video))
            Fill(Stream_Video, 0, Video_Duration, Duration_Final, 10, true);
        if (Count_Get(Stream_Audio))
            Fill(Stream_Audio, 0, Audio_Duration, Duration_Final, 10, true);

        //Integrity
        if (Count_Get(Stream_Video) && File_Size!=(int64u)-1 && !Retrieve(Stream_Video, 0, Video_BitRate).empty() && !Retrieve(Stream_Video, 0, Video_Duration).empty())
        {
            int64u BitRate_Video_Meta=Retrieve(Stream_Video, 0, Video_BitRate).To_int64u();
            int64u Duration=Retrieve(Stream_Video, 0, Video_Duration).To_int64u();
            int64u BitRate_Video_Duration=File_Size*8*1000/Duration;
            if (Count_Get(Stream_Audio) && !Retrieve(Stream_Audio, 0, Audio_BitRate).empty())
            {
                int64u BitRate_Audio=Retrieve(Stream_Audio, 0, Audio_BitRate).To_int64u();
                if (BitRate_Audio<BitRate_Video_Duration)
                    BitRate_Video_Duration-=BitRate_Audio;
                else if (BitRate_Audio)
                    BitRate_Video_Duration=0; //There is a problem
            }
            if (BitRate_Video_Meta<BitRate_Video_Duration/2 || BitRate_Video_Meta>BitRate_Video_Duration*2)
                Clear(Stream_Video, 0, Video_BitRate);
        }
    }
    */

    if (Stream[Stream_Video].Parser!=NULL)
    {
        Finish(Stream[Stream_Video].Parser);
        Merge(*Stream[Stream_Video].Parser, Stream_Video, 0, 0);
    }
    if (Stream[Stream_Audio].Parser!=NULL)
    {
        Finish(Stream[Stream_Audio].Parser);
        Merge(*Stream[Stream_Audio].Parser, Stream_Audio, 0, 0);
    }

    if (Retrieve(Stream_General, 0, General_Duration).empty() && Retrieve(Stream_Video, 0, Video_Duration).empty() && meta_duration)
        Fill(Stream_General, 0, General_Duration, meta_duration, 0, true);

    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Stream.clear();
}

//---------------------------------------------------------------------------
void File_Flv::Streams_Finish_PerStream(stream_t StreamKind)
{
    if (Stream[StreamKind].TimeStamp!=(int32u)-1)
    {
        //Calculating the last timestamp (last block included)
        if (!Stream[StreamKind].Durations.empty())
        {
            int64u Durations_Total=0;
            for (size_t Pos=0; Pos<Stream[StreamKind].Durations.size(); Pos++)
                Durations_Total+=Stream[StreamKind].Durations[Pos];
            int32u Duration_Average=float32_int32s(((float32)Durations_Total)/Stream[StreamKind].Durations.size());
            Stream[StreamKind].TimeStamp+=Duration_Average;
        }

        Fill(StreamKind, 0, "Duration", Stream[StreamKind].TimeStamp, 10, true);
    }
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Flv::FileHeader_Begin()
{
    //Synchro
    if (3>Buffer_Size)
        return false;
    if (Buffer[0]!=0x46 //"FLV"
     || Buffer[1]!=0x4C
     || Buffer[2]!=0x56)
    {
        Reject();
        return false;
    }
    if (9>Buffer_Size)
        return false;

    return true;
}

//---------------------------------------------------------------------------
void File_Flv::FileHeader_Parse()
{
    //Parsing
    Element_Begin1("FLV header");
    int32u Size;
    int8u  Version, Flags;
    Skip_String(3,                                              "Signature");
    Get_B1 (Version,                                            "Version");
    Get_B1 (Flags,                                              "Flags");
        Get_Flags (Flags, 0, video_stream_Count,                "Video");
        Get_Flags (Flags, 2, audio_stream_Count,                "Audio");
    Get_B4 (Size,                                               "Size");
    if (Size>9)
        Skip_XX(Size-9,                                         "Unknown");
    Element_End0();

    FILLING_BEGIN();
        //Integrity
        if (Version==0 || Size<9)
        {
            Reject();
            return;
        }

        //Filling
        Accept();

        Fill(Stream_General, 0, General_Format, "Flash Video");
        if (!video_stream_Count && !audio_stream_Count)
        {
            //TODO: quick and awful hack for a file having both bools unset, should detect directly the streams
            video_stream_Count=true;
            audio_stream_Count=true;
        }
        if (video_stream_Count)
        {
            Stream_Prepare(Stream_Video);
            video_stream_FrameRate_Detected=false;
        }
        else
            video_stream_FrameRate_Detected=true;
        if (audio_stream_Count)
            Stream_Prepare(Stream_Audio);

        if (Version>1)
        {
            Finish();
            return; //Version more than 1 is not supported
        }
    FILLING_ELSE()
        Reject();
    FILLING_END();
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Flv::Synchronize()
{
    if (File_Offset+Buffer_Offset+4==File_Size)
        return true; // Used by seek from end

    //Synchronizing
    while (Buffer_Offset+15<=Buffer_Size)
    {
        int32u BodyLength=BigEndian2int24u(Buffer+Buffer_Offset+5);
        if ((Buffer[Buffer_Offset  ]
            || Buffer[Buffer_Offset+1]
            || Buffer[Buffer_Offset+2]
            || Buffer[Buffer_Offset+3]>=11)
            && File_Offset+Buffer_Offset+15+BodyLength==File_Size)
            break; //Last block
        if (File_Offset+Buffer_Offset+15+BodyLength<File_Size)
        {
            if (Buffer_Offset+15+BodyLength+15>Buffer_Size)
                return false; //Need more data

            if ((Buffer[Buffer_Offset  ]
              || Buffer[Buffer_Offset+1]
              || Buffer[Buffer_Offset+2]
              || Buffer[Buffer_Offset+3]>=11)
             && (BigEndian2int32u(Buffer+Buffer_Offset+15+BodyLength)==11+BodyLength // PreviousTagSize
              || BigEndian2int32u(Buffer+Buffer_Offset+15+BodyLength)==BodyLength)) // PreviousTagSize without 11, found in some buggy files
            {
                 PreviousTagSize_Add11=(BigEndian2int32u(Buffer+Buffer_Offset+15+BodyLength)==BodyLength)?0:11;
                 break;
            }
        }

        Buffer_Offset++;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+15>Buffer_Size)
        return false;

    //Synched
    return true;
}

//---------------------------------------------------------------------------
bool File_Flv::Synched_Test()
{
    if (File_Offset+Buffer_Offset+4==File_Size)
        return true; // Used by seek from end

    //Must have enough buffer for having header
    if (Buffer_Offset+15>Buffer_Size)
        return false;

    //Quick test of synchro
    if (Buffer[Buffer_Offset  ]==0
     && Buffer[Buffer_Offset+1]==0
     && Buffer[Buffer_Offset+2]==0
     && Buffer[Buffer_Offset+3]<PreviousTagSize_Add11
     && File_Offset+Buffer_Offset>9)
    {
        Synched=false;
        return true;
    }

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Flv::Read_Buffer_Unsynched()
{
    if (!Searching_Duration) //If Searching_Duration, we are looking for end in inverse order, no timestamp reset
    {
        Stream[Stream_Video].TimeStamp=(int32u)-1;
        if (Stream[Stream_Video].Parser)
            Stream[Stream_Video].Parser->Open_Buffer_Unsynch();
        Stream[Stream_Audio].TimeStamp=(int32u)-1;
        if (Stream[Stream_Audio].Parser)
            Stream[Stream_Audio].Parser->Open_Buffer_Unsynch();
    }
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Flv::Header_Parse()
{
    if (Searching_Duration && File_Offset+Buffer_Offset==File_Size-4)
    {
        Get_B4 (PreviousTagSize,                                "PreviousTagSize");

        //Filling
        Header_Fill_Code((int64u)-1, "End Of File");
        Header_Fill_Size(4);
        return;
    }

    //Parsing
    int32u BodyLength;
    int8u Type;
    Get_B4 (PreviousTagSize,                                    "PreviousTagSize");
    if (File_Offset+Buffer_Offset+4<File_Size)
    {
        int32u Timestamp_Base;
        int8u  Timestamp_Extended;
        Get_B1 (Type,                                           "Type"); //Param_Info1(Type<19?Flv_Type[Type]:__T("Unknown"));
        Get_B3 (BodyLength,                                     "BodyLength");
        Get_B3 (Timestamp_Base,                                 "Timestamp_Base"); //in ms
        Get_B1 (Timestamp_Extended,                             "Timestamp_Extended"); //TimeStamp = Timestamp_Extended*0x01000000+Timestamp_Base
        Skip_B3(                                                "StreamID");

        // For audio, check if it's just an audio config.
        bool Skip_Timestamps=false;
        if (Type==0x08)
        {
            int16u  Format_Info;
            Peek_B2(Format_Info);
            int8u   Format=(Format_Info>>12)&0x0F;
            if (Format==10 && (Format_Info&0xFF)==0) // AAC sequence header
                Skip_Timestamps=true;
        }

        //Filling
        if ((Type==0x08 && !Skip_Timestamps) || Type==0x09)
        {
            Time=(((int32u)Timestamp_Extended)<<24)|Timestamp_Base;
            stream_t StreamKind=(Type==0x08)?Stream_Audio:Stream_Video;
            if (Stream[StreamKind].Delay==(int32u)-1)
                Stream[StreamKind].Delay=Time;
            else if (Stream[StreamKind].TimeStamp!=(int32u)-1 && Time>Stream[StreamKind].TimeStamp)
                Stream[StreamKind].Durations.push_back(Time-Stream[StreamKind].TimeStamp);
            if (!Searching_Duration || Stream[StreamKind].TimeStamp==(int32u)-1)
                Stream[StreamKind].TimeStamp=Time;
        }

        if (Type==0)
            Trusted_IsNot("Wrong type");
    }
    else
    {
        Type=0;
        BodyLength=0;
    }

    //Filling
    Header_Fill_Code(Type, Ztring().From_Number(Type, 16));
    Header_Fill_Size(Element_Offset+BodyLength);
}

//---------------------------------------------------------------------------
void File_Flv::Data_Parse()
{
    switch (Element_Code)
    {
        case 0x00 : Element_Name("End Of File"); break;
        case 0x08 : audio(); break;
        case 0x09 : video(); break;
        case 0x12 : meta(); break;
        case 0xFA : Rm(); break;
        case (int64u)-1 :   //When searching the last frame
                            if (8+PreviousTagSize>File_Size)
                            {
                                Searching_Duration=false;
                                Open_Buffer_Unsynch(); //There is a problem, trying to sync
                                PreviousTagSize=65536;
                            }
                            GoTo(File_Size-PreviousTagSize-8, "FLV");
                            return;
        default : if (Searching_Duration)
                  {
                    Finish(); //This is surely a bad en of file, don't try anymore
                    return;
                  }

    }

    if (Searching_Duration)
    {
        if ((((Count_Get(Stream_Video)==0 || Stream[Stream_Video].TimeStamp!=(int32u)-1)
           && (Count_Get(Stream_Audio)==0 || Stream[Stream_Audio].TimeStamp!=(int32u)-1))
          || (File_Size>65536*2 && File_Offset+Buffer_Offset-Header_Size-PreviousTagSize-4<File_Size-65536))
         && Config->ParseSpeed<1)
            Finish();
        else if (Element_Code==0xFA) //RM metadata have a malformed PreviousTagSize, always
        {
            //Trying to sync
            Searching_Duration=false;
            Open_Buffer_Unsynch(); //There is a problem, trying to sync
            GoToFromEnd(Header_Size+Element_Size+65536);
            return;
        }
        else
            GoTo(File_Offset+Buffer_Offset-Header_Size-PreviousTagSize-4);
    }
    else if (!video_stream_Count && !audio_stream_Count && video_stream_FrameRate_Detected && File_Offset+65536*2<File_Size && MediaInfoLib::Config.ParseSpeed_Get()<1) //All streams are parsed
    {
        Fill();

        //Trying to find the last frame for duration
        Read_Buffer_Unsynched(); //This is not synched yet, so we call directly this method instead of Open_Buffer_Unsynched
        GoToFromEnd(4, "FLV");
        Searching_Duration=true;
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Flv::video()
{
    Element_Name("Video");
    Stream[Stream_Video].PacketCount++;
    Element_Info1(Stream[Stream_Video].PacketCount);

    //Handling FrameRate
    if (!video_stream_FrameRate_Detected)
    {
        if (video_stream_FrameRate.empty() || Time!=video_stream_FrameRate[video_stream_FrameRate.size()-1]) //if 2 block witht the same timestamp
            video_stream_FrameRate.push_back(Time);
        if (video_stream_FrameRate.size()>30)
            video_stream_FrameRate_Detected=true;
    }

    if (Element_Size==0) //Header says that video is present, but there is only one null packet
    {
        Element_Info1("Null");
        return;
    }

    Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset), ContentType_MainStream);

    //Needed?
    if (!video_stream_Count && Config->ParseSpeed<1)
        return; //No more need of Video stream

    //Parsing
    int8u Codec, FrameType;
    Element_Begin1("Stream header");
    BS_Begin();
    Get_S1 (4, FrameType,                                       "frameType"); Param_Info1(Flv_FrameType[FrameType]);
    Get_S1 (4, Codec,                                           "codecID"); Param_Info1(Flv_Codec_Video[Codec]); Element_Info1(Flv_Codec_Video[Codec]);
    BS_End();
    Element_End0();

    FILLING_BEGIN();
        //Filling
        if (Retrieve(Stream_Video, 0, Video_Format).empty())
        {
            if (Count_Get(Stream_Video)==0)
                Stream_Prepare(Stream_Video);
            Fill(Stream_Video, 0, Video_Format, Flv_Format_Video[Codec]);
            Fill(Stream_Video, 0, Video_Format_Profile, Flv_Format_Profile_Video[Codec]);
            Fill(Stream_Video, 0, Video_Codec, Flv_Codec_Video[Codec]);
            Fill(Stream_Video, 0, Video_CodecID, Codec);
            Fill(Stream_Video, 0, Video_CodecID_Hint, Flv_CodecID_Hint_Video[Codec]);
            Fill(Stream_Video, 0, Video_BitDepth, 8); //FLV is not known to support another bit depth

            MustSynchronize=true; // Now, synchronization test is possible
        }

        //Parsing video data
        switch (Codec)
        {
            case  2 : video_H263(); break;
            case  3 : video_ScreenVideo(1); break;
            case  4 : video_VP6(false); break;
            case  5 : video_VP6(true); break;
            case  6 : video_ScreenVideo(2); break;
            case  7 : video_AVC(); break;
            default : Skip_XX(Element_Size-Element_Offset,      "Unknown");
                      video_stream_Count=false; //No more need of Video stream;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Flv::video_H263()
{
    //Parsing
    int16u Width=0, Height=0;
    int8u  Version, PictureSize, PictureType;
    bool   ExtraInformationFlag;
    BS_Begin();
    Skip_S3(17,                                                 "PictureStartCode");
    Get_S1 ( 5, Version,                                        "Version");
    if (Version>1)
        return;
    Skip_S1( 8,                                                 "TemporalReference");
    Get_S1 ( 3, PictureSize,                                    "PictureSize"); Param_Info1(Flv_H263_PictureSize[PictureSize]);
    switch (PictureSize)
    {
        case 0 :
            Get_S2 ( 8, Width,                                  "Width");
            Get_S2 ( 8, Height,                                 "Height");
            break;
        case 1 :
            Get_S2 (16, Width,                                  "Width");
            Get_S2 (16, Height,                                 "Height");
            break;
        default :
            if (PictureSize<8)
            {
                Width=Flv_H263_WidthHeight[PictureSize][0];
                Height=Flv_H263_WidthHeight[PictureSize][1];
            }
    }
    Get_S1 ( 2, PictureType,                                    "PictureSize"); Param_Info1(Flv_H263_PictureType[PictureType]);
    Skip_SB(                                                    "DeblockingFlag");
    Skip_S1( 5,                                                 "Quantizer");
    Get_SB (    ExtraInformationFlag,                           "ExtraInformationFlag");
    while (ExtraInformationFlag)
    {
        Skip_S1( 8,                                             "ExtraInformation");
        Get_SB (    ExtraInformationFlag,                       "ExtraInformationFlag");
    }
    BS_End();

    FILLING_BEGIN();
        Fill(Stream_Video, 0, Video_Width, Width, 10, true);
        Fill(Stream_Video, 0, Video_Height, Height, 10, true);
        video_stream_Count=false; //No more need of Video stream
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Flv::video_ScreenVideo(int8u Version)
{
    //Parsing
    int16u Width, Height;
    BS_Begin();
    Info_S1( 4, BlockWidth,                                     "BlockWidth"); Param_Info1((BlockWidth+1)*16);
    Get_S2 (12, Width,                                          "ImageWidth");
    Info_S1( 4, BlockHeight,                                    "BlockHeight"); Param_Info1((BlockHeight+1)*16);
    Get_S2 (12, Height,                                         "ImageHeight");
    if (Version==2)
    {
        Skip_S1(6,                                              "Reserved");
        Skip_SB(                                                "has IFrameImage");
        Skip_SB(                                                "has PaletteInfo");
    }
    BS_End();

    FILLING_BEGIN();
        Fill(Stream_Video, 0, Video_Width, Width, 10, true);
        Fill(Stream_Video, 0, Video_Height, Height, 10, true);
        video_stream_Count=false; //No more need of Video stream
    FILLING_END();
}

//---------------------------------------------------------------------------
// From: http://wiki.multimedia.cx/index.php?title=On2_VP6
//
void File_Flv::video_VP6(bool WithAlpha)
{
    //Parsing
    int8u HorizontalAdjustment, VerticalAdjustment;
    bool  FrameMode, Marker;
    BS_Begin();
    Get_S1 ( 4, HorizontalAdjustment,                           "HorizontalAdjustment");
    Get_S1 ( 4, VerticalAdjustment,                             "VerticalAdjustment");
    if (WithAlpha)
        Skip_S3(24,                                             "OffsetToAlpha");
    Get_SB (    FrameMode,                                      "FrameMode"); Param_Info1(Flv_VP6_FrameMode[FrameMode]);
    Skip_S1( 6,                                                 "Quantization");
    Get_SB (    Marker,                                         "Marker"); Param_Info1(Flv_VP6_Marker[Marker]);
    BS_End();
    if (FrameMode)
    {
        if (Marker==1)
            Skip_B2(                                            "Offset");
    }
    else
    {
        int8u Version, Version2, Width, Height;
        BS_Begin();
        Get_S1 ( 5, Version,                                    "Version");
        Get_S1 ( 2, Version2,                                   "Version2");
        Skip_SB(                                                "Interlace");
        BS_End();
        if (Marker || Version2==0)
            Skip_B2(                                            "Offset");
        Skip_B1(                                                "MacroBlock_Height");
        Skip_B1(                                                "MacroBlock_Width");
        Get_B1 (Height,                                         "Height"); Param_Info1(Ztring::ToZtring(Height*16)+__T(" pixels"));
        Get_B1 (Width,                                          "Width"); Param_Info1(Ztring::ToZtring(Width*16)+__T(" pixels"));

        FILLING_BEGIN();
            if (Width && Height)
            {
                Fill(Stream_Video, 0, Video_Width,  Width*16-HorizontalAdjustment, 10, true);
                Fill(Stream_Video, 0, Video_Height, Height*16-VerticalAdjustment, 10, true);
            }
            video_stream_Count=false; //No more need of Video stream
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Flv::video_AVC()
{
    int8u AVCPacketType;
    Get_B1 (AVCPacketType,                                      "AVCPacketType"); Param_Info1(Flv_AVCPacketType(AVCPacketType));
    Info_B3(CompositionTime,                                    "CompositionTime"); Param_Info1(Ztring::ToZtring((int32s)(CompositionTime+0xFF000000)));

    switch (AVCPacketType)
    {
        case 0 :
                #ifdef MEDIAINFO_AVC_YES
                    if (Stream[Stream_Video].Parser==NULL)
                    {
                        Stream[Stream_Video].Parser=new File_Avc;
                        Open_Buffer_Init(Stream[Stream_Video].Parser);
                        ((File_Avc*)Stream[Stream_Video].Parser)->MustParse_SPS_PPS=true;
                        ((File_Avc*)Stream[Stream_Video].Parser)->SizedBlocks=true;
                        ((File_Avc*)Stream[Stream_Video].Parser)->MustSynchronize=false;
                    }

                    //Parsing
                    Open_Buffer_Continue(Stream[Stream_Video].Parser);
                #else
                    Skip_XX(Element_Size-Element_Offset,        "AVC Data");
                    video_stream_Count=false; //Unable to parse it
                #endif
                break;
        case 1 :
                #ifdef MEDIAINFO_AVC_YES
                    if (Stream[Stream_Video].Parser==NULL)
                    {
                        //Data before header, this is wrong
                        video_stream_Count=false;
                        break;
                    }

                    //Parsing
                    Open_Buffer_Continue(Stream[Stream_Video].Parser);

                    //Disabling this stream
                    if (Stream[Stream_Video].Parser->File_GoTo!=(int64u)-1 || Stream[Stream_Video].Parser->Count_Get(Stream_Video)>0 || (Config->ParseSpeed<1.0 && Stream[Stream_Video].PacketCount>=300))
                         video_stream_Count=false;
                #else
                    Skip_XX(Element_Size-Element_Offset,        "AVC Data");
                    video_stream_Count=false; //Unable to parse it
                #endif
                break;
        default: Skip_XX(Element_Size-Element_Offset,           "Unknown");
                 video_stream_Count=false; //Unable to parse it
    }
}
//---------------------------------------------------------------------------
void File_Flv::audio()
{
    Element_Name("Audio");
    Stream[Stream_Audio].PacketCount++;
    Element_Info1(Stream[Stream_Audio].PacketCount);

    if (Element_Size==0) //Header says that audio is present, but there is only one null packet
    {
        Element_Info1("Null");
        return;
    }

    Demux(Buffer+Buffer_Offset+(size_t)Element_Offset+1, (size_t)(Element_Size-Element_Offset-1), ContentType_MainStream);

    //Needed?
    if (!audio_stream_Count && Config->ParseSpeed<1)
        return; //No more need of Audio stream

    //Parsing
    int8u  codec, sampling_rate;
    bool   is_16bit, is_stereo;
    Element_Begin1("Stream header");
    BS_Begin();
    Get_S1 (4, codec,                                           "codec"); Param_Info1(Flv_Codec_Audio[codec]); Element_Info1(Flv_Codec_Audio[codec]);
    Get_S1 (2, sampling_rate,                                   "sampling_rate"); Param_Info1(Ztring::ToZtring(Flv_SamplingRate[sampling_rate])+__T(" Hz"));
    Get_SB (   is_16bit,                                        "is_16bit"); Param_Info1(Ztring::ToZtring(Flv_Resolution[is_16bit])+__T(" bits"));
    Get_SB (   is_stereo,                                       "is_stereo"); Param_Info1(Ztring::ToZtring(Flv_Channels[is_stereo])+__T(" channel(s)"));
    BS_End();
    Element_End0();

    //Special case
    if (codec==5) //Nellymoser 8kHz mono
    {
        sampling_rate=5; //8000 Hz forced
        is_stereo=false; //Mono forced
    }

    FILLING_BEGIN();
        if (Retrieve(Stream_Audio, 0, Audio_Format).empty())
        {
            //Filling
            if (Count_Get(Stream_Audio)==0)
                Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, 0, Audio_Channel_s_, Flv_Channels[is_stereo], 10, true);
            if (codec!=2 && codec!=10 && codec!=14) //MPEG Audio and AAC are not fixed bit depth
                Fill(Stream_Audio, 0, Audio_BitDepth, Flv_Resolution[is_16bit], 10, true);
            if (sampling_rate<4)
                Fill(Stream_Audio, 0, Audio_SamplingRate, Flv_SamplingRate[sampling_rate], 10, true);
            Fill(Stream_Audio, 0, Audio_Format, Flv_Format_Audio[codec]);
            Fill(Stream_Audio, 0, Audio_Format_Profile, Flv_Format_Profile_Audio[codec]);
            Fill(Stream_Audio, 0, Audio_Codec, Flv_Codec_Audio[codec]);
            Fill(Stream_Audio, 0, Audio_CodecID, codec);
            Fill(Stream_Audio, 0, Audio_CodecID_Hint, Flv_CodecID_Hint_Audio[codec]);
            if (codec==1)
            {
                //ADPCM
                Fill(Stream_Audio, 0, Audio_Format_Settings, "ShockWave");
                Fill(Stream_Audio, 0, Audio_Format_Settings_Firm, "ShockWave");
                Fill(Stream_Audio, 0, Audio_Codec_Settings, "SWF");
                Fill(Stream_Audio, 0, Audio_Codec_Settings_Firm, "SWF");

            }

            MustSynchronize=true; // Now, synchronization test is possible
        }

        //Parsing audio data
        switch (codec)
        {
            case  2 :
            case 14 : audio_MPEG(); break;
            case 10 : audio_AAC(); break;
            default : Skip_XX(Element_Size-Element_Offset,      "Unknown");
                      audio_stream_Count=false; //No more need of Audio stream
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Flv::audio_MPEG()
{
    #if defined(MEDIAINFO_MPEGA_YES)
        if (Stream[Stream_Audio].Parser==NULL)
        {
            Stream[Stream_Audio].Parser=new File_Mpega;
            Open_Buffer_Init(Stream[Stream_Audio].Parser);
            ((File_Mpega*)Stream[Stream_Audio].Parser)->FrameIsAlwaysComplete=true;
        }

        //Parsing
        Open_Buffer_Continue(Stream[Stream_Audio].Parser);

        //Disabling this stream
        if (Stream[Stream_Audio].Parser->File_GoTo!=(int64u)-1 || Stream[Stream_Audio].Parser->Count_Get(Stream_Audio)>0)
             audio_stream_Count=false;
    #endif
}

//---------------------------------------------------------------------------
void File_Flv::audio_AAC()
{
    int8u AACPacketType;
    Get_B1 (AACPacketType,                                      "AACPacketType"); Param_Info1(Flv_AACPacketType(AACPacketType));

    switch (AACPacketType)
    {
        case 0 :
                #if defined(MEDIAINFO_MPEG4_YES)
                    if (Stream[Stream_Audio].Parser==NULL)
                    {
                        Stream[Stream_Audio].Parser=new File_Aac;
                        ((File_Aac*)Stream[Stream_Audio].Parser)->Mode=File_Aac::Mode_AudioSpecificConfig;
                        Open_Buffer_Init(Stream[Stream_Audio].Parser);
                    }

                    //Parsing
                    Open_Buffer_Continue(Stream[Stream_Audio].Parser);
                #else
                    Skip_XX(Element_Size-Element_Offset,        "AAC Data");
                    audio_stream_Count=false; //Unable to parse it
                #endif
                break;
        case 1 :
                //Parsing
                Open_Buffer_Continue(Stream[Stream_Audio].Parser);

                audio_stream_Count=false; //No need of more
                break;
        default: Skip_XX(Element_Size-Element_Offset,           "Unknown");
                audio_stream_Count=false; //Unable to parse it
    }
}

//---------------------------------------------------------------------------
void File_Flv::meta()
{
    Element_Name("Meta");

    //Parsing
    meta_Level=0;
    meta_SCRIPTDATAOBJECT();

    if (MetaData_NotTrustable)
    {
        meta_duration=0;
        Clear(Stream_Video, 0, Video_StreamSize);
        Clear(Stream_Video, 0, Video_BitRate);
        Clear(Stream_Video, 0, Video_Bits__Pixel_Frame_);
        Clear(Stream_Audio, 0, Audio_StreamSize);
        Clear(Stream_Audio, 0, Audio_BitRate);
        Clear(Stream_General, 0, General_Duration);
        Clear(Stream_General, 0, General_OverallBitRate);
    }
}

//---------------------------------------------------------------------------
void File_Flv::meta_SCRIPTDATAOBJECT()
{
    //Parsing Value
    std::string StringData;
    meta_SCRIPTDATAVALUE(StringData);
    meta_SCRIPTDATAVALUE(StringData);
}

//---------------------------------------------------------------------------
void File_Flv::meta_SCRIPTDATAVARIABLE()
{
    std::string StringData;
    int16u StringLength;
    Element_Begin0();
    Get_B2 (StringLength,                                       "StringLength");
    Get_String(StringLength, StringData,                        "StringData");
    Element_Name(StringData.c_str());

    //Parsing Value
    meta_SCRIPTDATAVALUE(StringData);
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Flv::meta_SCRIPTDATAVALUE(const std::string &StringData)
{
    std::string StringDataModified(StringData);
    if (!StringDataModified.empty() && StringDataModified[0]==__T('_'))
        StringDataModified.erase(StringDataModified.begin());

    //Parsing
    int8u Type;
    Get_B1 (Type,                                               "Type"); Param_Info1C((Type<0x12), Flv_TagType[Type]);
    switch (Type)
    {
        case 0x00 : //DOUBLE --> 64 bits Big endian float
            {
                float64 Value;
                Get_BF8(Value,                                 "Value");
                if (Value==0)
                    break;
                std::string ToFill;
                Ztring ValueS;
                stream_t StreamKind=Stream_General;
                     if (StringDataModified=="width") {ToFill="Width"; StreamKind=Stream_Video; ValueS.From_Number(Value, 0); video_stream_Count=true;} //1 file with FrameRate tag and video stream but no video present tag
                else if (StringDataModified=="height") {ToFill="Height"; StreamKind=Stream_Video; ValueS.From_Number(Value, 0); video_stream_Count=true;} //1 file with FrameRate tag and video stream but no video present tag
                else if (StringDataModified=="duration") meta_duration=Value*1000;
                else if (StringDataModified=="audiodatarate") {ToFill="BitRate"; StreamKind=Stream_Audio; ValueS.From_Number(Value*1000, 0);}
                else if (StringDataModified=="framerate") {ToFill="FrameRate"; StreamKind=Stream_Video; ValueS.From_Number(Value, 3); video_stream_FrameRate_Detected=true; video_stream_Count=true;} //1 file with FrameRate tag and video stream but no video present tag
                else if (StringDataModified=="videoframerate") {ToFill="FrameRate"; StreamKind=Stream_Video; ValueS.From_Number(Value, 3); video_stream_FrameRate_Detected=true; video_stream_Count=true;} //1 file with FrameRate tag and video stream but no video present tag
                else if (StringDataModified=="filesize") {meta_filesize=(int64u)Value;}
                else if (StringDataModified=="audiosize") {ToFill="StreamSize"; StreamKind=Stream_Audio; ValueS.From_Number(Value, 0); if (Value>File_Size) MetaData_NotTrustable=true;}
                else if (StringDataModified=="videosize") {ToFill="StreamSize"; StreamKind=Stream_Video; ValueS.From_Number(Value, 0); if (Value>File_Size) MetaData_NotTrustable=true; video_stream_Count=true;} //1 file with FrameRate tag and video stream but no video present tag
                else if (StringDataModified=="videodatarate") {ToFill="BitRate"; StreamKind=Stream_Video; ValueS.From_Number(Value*1000, 0); video_stream_Count=true;} //1 file with FrameRate tag and video stream but no video present tag
                else if (StringDataModified=="videocodecid") {; video_stream_Count=true;} //1 file with FrameRate tag and video stream but no video present tag
                else if (StringDataModified=="audiodelay") {ToFill="Delay"; StreamKind=Stream_Audio; if (Value>0) ValueS.From_Number(Value*1000, 0);}
                else if (StringDataModified=="audiosamplerate") {ToFill="SamplingRate"; StreamKind=Stream_Audio; if (Value>0) ValueS.From_Number(Value, 0);}
                else if (StringDataModified=="audiosamplesize") {ToFill="BitDepth"; StreamKind=Stream_Audio; if (Value>0) ValueS.From_Number(Value, 0);}
                else if (StringDataModified=="totalduration") {ToFill="Duration"; StreamKind=Stream_General; ValueS.From_Number(Value*1000, 0);}
                else if (StringDataModified=="totaldatarate") {ToFill="OverallBitRate"; StreamKind=Stream_General; ValueS.From_Number(Value*1000, 0);}
                else if (StringDataModified=="totalframes") {ToFill="FrameCount"; StreamKind=Stream_Video; ValueS.From_Number(Value*1000, 0);}
                else if (StringDataModified=="bytelength") {if (File_Size!=Value) MetaData_NotTrustable=true;}
                else if (!(StringDataModified=="datasize"
                       || StringDataModified=="lasttimestamp"
                       || StringDataModified=="lastkeyframetimestamp"
                       || StringDataModified=="lastkeyframelocation"
                       || StringDataModified=="canSeekToEnd"
                       || StringDataModified=="keyframes_times"
                       || StringDataModified=="keyframes_filepositions"
                       || StringDataModified=="aacaot"
                       || StringDataModified=="audiochannels"
                       || StringDataModified=="audiocodecid"
                       || StringDataModified=="avclevel"
                       || StringDataModified=="avcprofile"
                       || StringDataModified=="moovPosition")) {StreamKind=Stream_General; ToFill=StringData; ValueS.From_Number(Value);}
                #if MEDIAINFO_TRACE
                    if (ValueS.empty())
                        ValueS.From_Number(Value, 0);
                    Element_Info1(ValueS);
                #endif //MEDIAINFO_TRACE
                if (!ToFill.empty())
                {
                    Fill(StreamKind, 0, ToFill.c_str(), ValueS, true);
                    if (ToFill=="FrameRate")
                        Fill(StreamKind, 0, "FrameRate_Mode", "CFR", Unlimited, true, true);
                }
            }
            break;
        case 0x01 : //UI8
            {
                int8u Value;
                Get_B1 (Value,                                  "Value");
                std::string ToFill;
                     if (StringDataModified=="haskeyframes") {}
                else if (StringDataModified=="hasKeyframes") {}
                else if (StringDataModified=="hasVideo") {}
                else if (StringDataModified=="stereo") {}
                else if (StringDataModified=="canSeekToEnd") {}
                else if (StringDataModified=="hasAudio") {}
                else if (StringDataModified=="hasmetadata") {}
                else if (StringDataModified=="hasMetadata") {}
                else if (StringDataModified=="hasCuePoints") {}
                else if (StringDataModified=="canseekontime") {}
                else {ToFill=StringData;}
                Element_Info1(Value);
                Fill(Stream_General, 0, ToFill.c_str(), Value?"Yes":"No", Unlimited, true, true);
            }
            break;
        case 0x02 : //SCRIPTDATASTRING
             {
                int16u Value_Size;
                Get_B2 (Value_Size,                             "Value_Size");
                if (Value_Size)
                {
                    Ztring Value;
                    Get_UTF8(Value_Size, Value,                 "Value");
                    size_t ToFill=(size_t)-1;
                    std::string ToFillS;
                         if (StringDataModified=="creator") {ToFill=General_Encoded_Application;}
                    else if (StringDataModified=="creationdate") {ToFill=General_Encoded_Date; Value.Date_From_String(Value.To_UTF8().c_str());}
                    else if (StringDataModified=="encoder") {ToFill=General_Encoded_Application;}
                    else if (StringDataModified=="Encoded_With") {ToFill=General_Encoded_Application;}
                    else if (StringDataModified=="Encoded_By") {ToFill=General_Encoded_Application;}
                    else if (StringDataModified=="metadatacreator") {ToFill=General_Tagged_Application;}
                    else if (StringDataModified=="creation_time") {ToFill=General_Encoded_Date; Value.insert(0, __T("UTC "));}
                    else if (StringDataModified=="sourcedata") {}
                    else if (StringDataModified=="audiocodecid") {}
                    else if (StringDataModified=="videocodecid") {}
                    else if (!(StringDataModified=="major_brand"
                            || StringDataModified=="minor_version"
                            || StringDataModified=="compatible_brands"))
                        ToFillS=StringData;
                    if (Value.find(__T('\r'))!=std::string::npos)
                        Value.resize(Value.find(__T('\r')));
                    if (Value.find(__T('\n'))!=std::string::npos)
                        Value.resize(Value.find(__T('\n')));
                    Element_Info1(Value);
                    if (ToFill!=(size_t)-1)
                        Fill(Stream_General, 0, ToFill, Value, true);
                    else if (!ToFillS.empty())
                        Fill(Stream_General, 0, StringData.c_str(), Value, true);
                }
            }
            break;
        case 0x03 : //SCRIPTDATAOBJECT[n]
        case 0x10 : //Typed object - SCRIPTDATAOBJECT[n]
            {
                std::string StringData2;
                int16u StringLength2;
                meta_Level++;
                meta_LevelFinished[meta_Level]=false;
                while (!meta_LevelFinished[meta_Level])
                {
                    if (Element_Offset>=Element_Size)
                        break;
                    Element_Begin0();
                    Get_B2 (StringLength2,                          "StringLength2");
                    Get_String(StringLength2, StringData2,          "StringData2");
                    Element_Name(StringData2.empty()?"EndOfObject":StringData2.c_str());
                    meta_SCRIPTDATAVALUE(StringData+'_'+StringData2);
                    Element_End0();
                }
                meta_Level--;
            }
            break;
        case 0x04 : //SCRIPTDATASTRING defining the MovieClip path
            {
                int16u Value_Size;
                Get_B2 (Value_Size,                             "Value_Size");
                if (Value_Size)
                {
                    Ztring Value;
                    Get_Local(Value_Size, Value,                "Value");
                    if (Value==__T("unknown")) Value.clear();
                    Element_Info1C((!Value.empty()), Value);
                    Fill(Stream_General, 0, StringData.c_str(), Value, true);
                }
            }
            break;
        case 0x05 : //NULL
        case 0x06 : //Undefined - NULL
        case 0x0D : //Unsupported - NULL
            break;
        case 0x07 : //UI16
            {
                int16u Value;
                Get_B2 (Value,                                  "Value");
                Element_Info1(Value);
                Fill(Stream_General, 0, StringData.c_str(), Value, true);
            }
            break;
        case 0x08 : //SCRIPTDATAVARIABLE[ECMAArrayLength]
            {
                int32u ECMAArrayLength;
                Get_B4 (ECMAArrayLength,                        "ECMAArrayLength");
                Element_Info1(Ztring::ToZtring(ECMAArrayLength)+__T(" elements"));
                for (int32u Pos=0; Pos<ECMAArrayLength; Pos++)
                {
                    meta_SCRIPTDATAVARIABLE();
                    if (meta_LevelFinished[meta_Level])
                        Pos=ECMAArrayLength; //Finished
                }
            }
            break;
        case 0x09 :
            Element_Info1("EndOfObject");
            meta_LevelFinished[meta_Level]=true;
            break;
        case 0x0A : //SCRIPTDATAVARIABLE[n]
        case 0x0E : //RecordSet - SCRIPTDATAVARIABLE[n]
            {
                int32u Count;
                Get_B4 (Count,                                  "Count");
                for (int32u Pos=0; Pos<Count; Pos++)
                    meta_SCRIPTDATAVALUE(StringData);
            }
            break;
        case 0x0B : //SCRIPTDATADATE
            {
                float64 Value;
                Get_BF8(Value,                                 "Value");
                Ztring ValueS;
                ValueS.Date_From_Seconds_1970((int32u)(Value/1000));
                Param_Info1(ValueS);
                Skip_B2(                                        "Local_Offset_Minutes");
                std::string ToFill;
                     if (StringData=="metadatadate") {ToFill="Tagged_Date";}
                else {ToFill=StringData;}
                Element_Info1(ValueS);
                Fill(Stream_General, 0, ToFill.c_str(), ValueS, true);
            }
            break;
        case 0x0C : //SCRIPTDATALONGSTRING
        case 0x0F : //XML - SCRIPTDATALONGSTRING
            {
                int32u Value_Size;
                Get_B4 (Value_Size,                             "Value_Size");
                if (Value_Size)
                {
                    Ztring Value;
                    Get_UTF16B(Value_Size, Value,               "Value");
                    std::string ToFill;
                         if (StringData=="creator") {ToFill="Encoded_Application";}
                    else if (StringData=="liveXML") {}
                    else if (StringData=="metadatacreator") {ToFill="Tagged_Application";}
                    else if (StringData=="creationdate") {ToFill="Encoded_Date"; Value.Date_From_String(Value.To_UTF8().c_str());}
                    else {ToFill=StringData;}
                    Element_Info1(Value);
                    if (!ToFill.empty())
                        Fill(Stream_General, 0, ToFill.c_str(), Value, true);
                }
            }
            break;
        case 0x11 : //AMF3 data
            {
                int32u TypeCode;
                Get_B4 (TypeCode,                               "AMF3 type code"); Param_Info1C((TypeCode<0x0D), Flv_Amf3Type[TypeCode]);
                switch (TypeCode)
                {
                    case 0x00 : //undefined
                    case 0x01 : //null
                    case 0x02 : //boolean-false
                    case 0x03 : //boolean-true
                        break;
                    default : //Not implemented or unknown
                        Element_Offset=Element_Size;
                }
            }
            break;
        default : //Unknown
            Element_Offset=Element_Size; //Forcing the end of parsing
    }
}

//---------------------------------------------------------------------------
void File_Flv::Rm()
{
    Element_Name("Real Media tags");

    //Creating the parser
    File_Rm MI;
    Open_Buffer_Init(&MI);

    //Parsing
    Open_Buffer_Continue(&MI);

    //Filling
    Finish(&MI);
    Merge(MI, Stream_General, 0, 0);
}

} //NameSpace

#endif //MEDIAINFO_FLV_YES
