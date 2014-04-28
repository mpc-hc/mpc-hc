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
#ifdef MEDIAINFO_RIFF_YES
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Riff.h"
#if defined(MEDIAINFO_MPEG4V_YES)
    #include "MediaInfo/Video/File_Mpeg4v.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_DTS_YES)
    #include "MediaInfo/Audio/File_Dts.h"
#endif
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include <ZenLib/File.h>
#include <ZenLib/Utils.h>
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Const
//***************************************************************************

namespace Elements
{
    const int32u AIFF_SSND=0x53534E44;
    const int32u AVI_=0x41564920;
    const int32u AVI__hdlr_strl_strh_txts=0x74787473;
    const int32u FORM=0x464F524D;
    const int32u LIST=0x4C495354;
    const int32u MThd=0x4D546864;
    const int32u ON2_=0x4F4E3220;
    const int32u ON2f=0x4F4E3266;
    const int32u RIFF=0x52494646;
    const int32u riff=0x72696666;
    const int32u RF64=0x52463634;
    const int32u SMV0=0x534D5630;
    const int32u SMV0_xxxx=0x534D563A;
    const int32u W3DI=0x57334449;
    const int32u WAVE=0x57415645;
    const int32u WAVE_data=0x64617461;
    const int32u WAVE_ds64=0x64733634;
}

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
File_Riff::File_Riff()
:File__Analyze()
{
    //Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Riff;
        StreamIDs_Width[0]=4;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    DataMustAlwaysBeComplete=false;

    //In/Out
    #if defined(MEDIAINFO_ANCILLARY_YES)
        Ancillary=NULL;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)

    //Data
    Interleaved0_1=0;
    Interleaved0_10=0;
    Interleaved1_1=0;
    Interleaved1_10=0;

    //Temp
    WAVE_data_Size=0xFFFFFFFF;
    WAVE_fact_samplesCount=0xFFFFFFFF;
    Buffer_DataToParse_Begin=(int64u)-1;
    Buffer_DataToParse_End=0;
    #if MEDIAINFO_DEMUX
        AvgBytesPerSec=0;
    #endif //!MEDIAINFO_DEMUX
    avih_FrameRate=0;
    avih_TotalFrame=0;
    dmlh_TotalFrame=0;
    Idx1_Offset=(int64u)-1;
    movi_Size=0;
    TimeReference=(int64u)-1;
    SMV_BlockSize=0;
    SamplesPerSec=0;
    stream_Count=0;
    rec__Present=false;
    NeedOldIndex=true;
    IsBigEndian=false;
    IsWave64=false;
    IsRIFF64=false;
    IsWaveBroken=false;
    IsNotWordAligned=false;
    IsNotWordAligned_Tested=false;
    SecondPass=false;
    DV_FromHeader=NULL;
    Kind=Kind_None;
    #if MEDIAINFO_DEMUX
    Demux_Parser=NULL;
    #endif //MEDIAINFO_DEMUX

    //Pointers
    Stream_Structure_Temp=Stream_Structure.end();
}

//---------------------------------------------------------------------------
File_Riff::~File_Riff()
{
    #ifdef MEDIAINFO_DVDIF_YES
        delete (File_DvDif*)DV_FromHeader; //DV_FromHeader=NULL
    #endif //MEDIAINFO_DVDIF_YES
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Riff::Streams_Finish ()
{
    //Ancillary specific
    #if defined(MEDIAINFO_ANCILLARY_YES)
        if (Ancillary && (*Ancillary))
        {
            Clear();
            Stream_Prepare(Stream_General);
            Finish(*Ancillary);
            Merge(**Ancillary);
            Ztring LawRating=(*Ancillary)->Retrieve(Stream_General, 0, General_LawRating);
            if (!LawRating.empty())
                Fill(Stream_General, 0, General_LawRating, LawRating, true);
            return;
        }
    #endif //defined(MEDIAINFO_ANCILLARY_YES)

    //Global
    if (IsRIFF64)
        Fill(Stream_General, 0, General_Format_Profile, "RF64");

    //Time codes
    TimeCode_Fill(__T("ISMP"), INFO_ISMP);
    TimeCode_Fill(__T("Adobe tc_A"), Tdat_tc_A);
    TimeCode_Fill(__T("Adobe tc_O"), Tdat_tc_O);

    //For each stream
    std::map<int32u, stream>::iterator Temp=Stream.begin();
    while (Temp!=Stream.end())
    {
        //Preparing
        StreamKind_Last=Temp->second.StreamKind;
        StreamPos_Last=Temp->second.StreamPos;

        //StreamSize
        if (Temp->second.StreamSize>0)
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_StreamSize), Temp->second.StreamSize);

        //When there are few frames, difficult to detect PCM
        if (Temp->second.IsPcm && !Temp->second.Parsers.empty() && !Temp->second.Parsers[0]->Status[IsAccepted])
        {
            for (size_t Pos=0; Pos<Temp->second.Parsers.size()-1; Pos++)
                delete Temp->second.Parsers[Pos];
            Temp->second.Parsers.erase(Temp->second.Parsers.begin(), Temp->second.Parsers.begin()+Temp->second.Parsers.size()-1);
            Temp->second.Parsers[0]->Accept();
        }

        //PAR
        if (PAR && StreamKind_Last==Stream_Video)
            Fill(Stream_Video, StreamPos_Last, Video_PixelAspectRatio, PAR);

        Ztring ID;
        if (Temp->first!=(int32u)-1)
            ID.From_Number(((Temp->first>>24)-'0')*10+(((Temp->first>>16)&0xFF)-'0'));

        //Parser specific
        if (Temp->second.Parsers.size()==1)
        {
            //Finalizing and Merging (except Video codec and 120 fps hack)
            Temp->second.Parsers[0]->ShouldContinueParsing=false;

            //Hack - Before
            Ztring StreamSize, Codec_Temp;
            if (StreamKind_Last==Stream_Video)
                Codec_Temp=Retrieve(Stream_Video, StreamPos_Last, Video_Codec); //We want to keep the 4CC of AVI
            StreamSize=Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_StreamSize)); //We want to keep the 4CC of AVI

            //Merging
            if (Config->ParseSpeed<=1.0)
            {
                Fill(Temp->second.Parsers[0]);
                Temp->second.Parsers[0]->Open_Buffer_Unsynch();
            }
            Finish(Temp->second.Parsers[0]);

            if (!Temp->second.Parsers.empty() && Temp->second.Parsers[0]->Count_Get(StreamKind_Last))
            {
                //Special case: Compressed audio hidden in PCM
                if (StreamKind_Last==Stream_Audio
                    && Temp->second.Compression==1
                    && Retrieve(Stream_General, 0, General_Format)==__T("Wave")
                    && Temp->second.Parsers[0]->Get(Stream_Audio, 0, Audio_Format)!=__T("PCM")) //Some DTS or SMPTE ST 337 streams are coded "1"
                    Clear(Stream_Audio, 0, Audio_Channel_s_);

                size_t StreamPos_Base=StreamPos_Last;
                for (size_t Pos=0; Pos<Temp->second.Parsers[0]->Count_Get(StreamKind_Last); Pos++)
                {
                    Ztring Temp_ID=ID;
                    Ztring Temp_ID_String=ID;
                    Merge(*Temp->second.Parsers[0], StreamKind_Last, Pos, StreamPos_Base+Pos);
                    if (!Retrieve(StreamKind_Last, StreamPos_Last, General_ID).empty())
                    {
                        if (!Temp_ID.empty())
                        {
                            Temp_ID+=__T('-');
                            Temp_ID_String+=__T('-');
                        }
                        Temp_ID+=Retrieve(StreamKind_Last, StreamPos_Last, General_ID);
                        Temp_ID_String+=Retrieve(StreamKind_Last, StreamPos_Last, General_ID);
                    }
                    Fill(StreamKind_Last, StreamPos_Last, General_ID, Temp_ID, true);
                    Fill(StreamKind_Last, StreamPos_Last, General_StreamOrder, Temp_ID_String, true);


                    //Special case - MPEG Video + Captions
                    if (StreamKind_Last==Stream_Video && Temp->second.Parsers[0]->Count_Get(Stream_Text))
                    {
                        //Video and Text are together
                        size_t Parser_Text_Count=Temp->second.Parsers[0]->Count_Get(Stream_Text);
                        for (size_t Parser_Text_Pos=0; Parser_Text_Pos<Parser_Text_Count; Parser_Text_Pos++)
                        {
                            size_t StreamPos_Video=StreamPos_Last;
                            Fill_Flush();
                            Stream_Prepare(Stream_Text);
                            Temp->second.Parsers[0]->Finish();
                            Merge(*Temp->second.Parsers[0], Stream_Text, Parser_Text_Pos, StreamPos_Last);
                            Fill(Stream_Text, StreamPos_Last, Text_Duration, Retrieve(Stream_Video, StreamPos_Video, Video_Duration));
                            Ztring ID=Retrieve(Stream_Text, StreamPos_Last, Text_ID);
                            Fill(Stream_Text, StreamPos_Last, Text_ID, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID)+__T("-")+ID, true);
                            Fill(Stream_Text, StreamPos_Last, Text_ID_String, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID_String)+__T("-")+ID, true);
                            Fill(Stream_Text, StreamPos_Last, Text_Title, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Title), true);
                            Fill(Stream_Text, StreamPos_Last, "MuxingMode_MoreInfo", __T("Muxed in Video #")+Ztring().From_Number(Count_Get(Stream_Video)), true);
                        }

                        StreamKind_Last=Stream_Video;
                        StreamPos_Last=Count_Get(Stream_Video)-1;
                    }
                }
            }
            else
            {
                Fill(StreamKind_Last, StreamPos_Last, General_ID, ID, true);
                Fill(StreamKind_Last, StreamPos_Last, General_StreamOrder, ID, true);
            }

            //Hacks - After
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_StreamSize), StreamSize, true);
            if (StreamKind_Last==Stream_Video)
            {
                if (!Codec_Temp.empty())
                    Fill(Stream_Video, StreamPos_Last, Video_Codec, Codec_Temp, true);

                //120 fps hack
                const Ztring &FrameRate=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate);
                if (FrameRate.To_int32u()==120)
                {
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_String, MediaInfoLib::Config.Language_Get(FrameRate+__T(" (24/30)"), __T(" fps")), true);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Minimum, 24, 10, true);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Maximum, 30, 10, true);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Mode, "VFR");
                }
            }

            //Hack - SMPTE ST 337, RIFF channels count is wrong
            if (StreamKind_Last==Stream_Audio && Retrieve(Stream_Audio, StreamPos_Last, Audio_MuxingMode)==__T("AES3") && Temp->second.Parsers[0]->Get(Stream_Audio, 0, Audio_Channel_s_).empty())
                Clear(Stream_Audio, StreamPos_Last, Audio_Channel_s_);

            //Alignment
            if (StreamKind_Last==Stream_Audio && Count_Get(Stream_Video)>0) //Only if this is not a WAV file
            {
                Fill(Stream_Audio, StreamPos_Last, Audio_Alignment, Temp->second.ChunksAreComplete?"Aligned":"Split");
                Fill(Stream_Audio, StreamPos_Last, Audio_Alignment_String, MediaInfoLib::Config.Language_Get(Temp->second.ChunksAreComplete?__T("Alignment_Aligned"):__T("Alignment_Split")));
            }

            //Delay
            if (StreamKind_Last==Stream_Audio && Count_Get(Stream_Video)==1 && Temp->second.Rate!=0 && Temp->second.Parsers[0]->Status[IsAccepted])
            {
                float Delay=0;
                bool Delay_IsValid=false;

                     if (Temp->second.Parsers[0]->Buffer_TotalBytes_FirstSynched==0)
                {
                    Delay=0;
                    Delay_IsValid=true;
                }
                else if (Temp->second.Rate!=0)
                {
                    Delay=((float)Temp->second.Parsers[0]->Buffer_TotalBytes_FirstSynched)*1000/Temp->second.Rate;
                    Delay_IsValid=true;
                }
                else if (Temp->second.Parsers[0]->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int64u()!=0)
                {
                    Delay=((float)Temp->second.Parsers[0]->Buffer_TotalBytes_FirstSynched)*1000/Temp->second.Parsers[0]->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int64u();
                    Delay_IsValid=true;
                }
                else if (Temp->second.Parsers[0]->Retrieve(Stream_Audio, 0, Audio_BitRate_Nominal).To_int64u()!=0)
                {
                    Delay=((float)Temp->second.Parsers[0]->Buffer_TotalBytes_FirstSynched)*1000/Temp->second.Parsers[0]->Retrieve(Stream_Audio, 0, Audio_BitRate_Nominal).To_int64u();
                    Delay_IsValid=true;
                }

                if (Delay_IsValid)
                {
                    Delay+=((float)Temp->second.Start)*1000/Temp->second.Rate;
                    Fill(Stream_Audio, StreamPos_Last, Audio_Delay, Delay, 0, true);
                    Fill(Stream_Audio, StreamPos_Last, Audio_Delay_Source, "Stream", Unlimited, true, true);
                    for (size_t StreamPos=0; StreamPos<Count_Get(Stream_Video); StreamPos++)
                        Fill(Stream_Video, StreamPos, Video_Delay, 0, 10, true);
                }
            }

            //Special case: AAC
            if (StreamKind_Last==Stream_Audio
             && (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("AAC")
              || Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("MPEG Audio")
              || Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("Vorbis")))
                Clear(Stream_Audio, StreamPos_Last, Audio_BitDepth); //Resolution is not valid for AAC / MPEG Audio / Vorbis

            //Format specific
            #if defined(MEDIAINFO_DVDIF_YES)
                if (StreamKind_Last==Stream_Video && (MediaInfoLib::Config.Codec_Get(Ztring().From_CC4(Temp->second.Compression), InfoCodec_KindofCodec).find(__T("DV"))==0
                                                   || Retrieve(Stream_Video, StreamPos_Last, Video_Format)==__T("DV")
                                                   || Retrieve(Stream_Video, StreamPos_Last, Video_Codec)==__T("DV")))
                {
                    if (Retrieve(Stream_General, 0, General_Recorded_Date).empty())
                        Fill(Stream_General, 0, General_Recorded_Date, Temp->second.Parsers[0]->Retrieve(Stream_General, 0, General_Recorded_Date));

                    //Video and Audio are together
                    size_t Audio_Count=Temp->second.Parsers[0]->Count_Get(Stream_Audio);
                    for (size_t Audio_Pos=0; Audio_Pos<Audio_Count; Audio_Pos++)
                    {
                        Fill_Flush();
                        Stream_Prepare(Stream_Audio);
                        size_t Pos=Count_Get(Stream_Audio)-1;
                        Merge(*Temp->second.Parsers[0], Stream_Audio, Audio_Pos, StreamPos_Last);
                        Fill(Stream_Audio, Pos, Audio_MuxingMode, "DV");
                        Fill(Stream_Audio, Pos, Audio_Duration, Retrieve(Stream_Video, Temp->second.StreamPos, Video_Duration));
                        Fill(Stream_Audio, Pos, "MuxingMode_MoreInfo", __T("Muxed in Video #")+Ztring().From_Number(Temp->second.StreamPos+1));
                        Fill(Stream_Audio, Pos, Audio_StreamSize_Encoded, 0); //Included in the DV stream size
                        Ztring ID=Retrieve(Stream_Audio, Pos, Audio_ID);
                        Fill(Stream_Audio, Pos, Audio_ID, Retrieve(Stream_Video, Temp->second.StreamPos, Video_ID)+__T("-")+ID, true);
                    }

                    StreamKind_Last=Stream_Video;
                    StreamPos_Last=Temp->second.StreamPos;
                }
            #endif
        }
        else if (StreamKind_Last!=Stream_General)
            Fill(StreamKind_Last, StreamPos_Last, General_ID, ID);

        //Duration
        if (Temp->second.PacketCount>0)
        {
            if (StreamKind_Last==Stream_Video) // && Retrieve(Stream_Video, StreamPos_Last, Video_Duration).empty())
            {
                //Duration in case it is missing from header (malformed header...)
                if (Temp->second.indx_Duration && Temp->second.Rate)
                    Fill(Stream_Video, StreamPos_Last, Video_Duration, ((float64)Temp->second.indx_Duration)*1000*Temp->second.Scale/Temp->second.Rate, 0, true);
                else
                    Fill(Stream_Video, StreamPos_Last, Video_FrameCount, Temp->second.PacketCount, 10, true);
            }
            if (StreamKind_Last==Stream_Audio)
            {
                //Duration in case it is missing from header (malformed header...)
                int64u SamplingCount=0;
                #if defined(MEDIAINFO_MPEGA_YES)
                if (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("MPEG Audio"))
                {
                    if (Temp->second.Parsers[0] && Temp->second.PacketPos==((File_Mpega*)Temp->second.Parsers[0])->Frame_Count_Valid) //Only for stream with one frame per chunk
                    {
                        Ztring Version=Retrieve(Stream_Audio, StreamPos_Last, Audio_Format_Version);
                        Ztring Layer=Retrieve(Stream_Audio, StreamPos_Last, Audio_Format_Profile);
                        if (Version==__T("Version 1") && Layer==__T("Layer 1"))
                            SamplingCount=Temp->second.PacketCount*384;  //MPEG-1 Layer 1
                        else if ((Version==__T("Version 2") || Version==__T("Version 2.5")) && Layer==__T("Layer 1"))
                            SamplingCount=Temp->second.PacketCount*192;  //MPEG-2 or 2.5 Layer 1
                        else if ((Version==__T("Version 2") || Version==__T("Version 2.5")) && Layer==__T("Layer 3"))
                            SamplingCount=Temp->second.PacketCount*576;  //MPEG-2 or 2.5 Layer 3
                        else
                            SamplingCount=Temp->second.PacketCount*1152;
                    }
                }
                #endif
                if (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("PCM"))
                {
                    int64u Resolution=Retrieve(Stream_Audio, StreamPos_Last, Audio_BitDepth).To_int64u();
                    int64u Channels=Retrieve(Stream_Audio, StreamPos_Last, Audio_Channel_s_).To_int64u();
                    if (Resolution>0 && Channels>0)
                        SamplingCount=Temp->second.StreamSize*8/Resolution/Channels;
                }
                if (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("ADPCM"))
                {
                    int64u Resolution=Retrieve(Stream_Audio, StreamPos_Last, Audio_BitDepth).To_int64u();
                    int64u Channels=Retrieve(Stream_Audio, StreamPos_Last, Audio_Channel_s_).To_int64u();
                    if (Resolution>0 && Channels>0)
                        SamplingCount=(int64u)(Temp->second.StreamSize*8/Resolution/Channels*0.98); //0.98 is not precise!

                    //ADPCM estimation is not precise, if container sampling count is around our value, using it rather than the estimation
                    float32 SamplingRate=Retrieve(Stream_Audio, StreamPos_Last, Audio_SamplingRate).To_float32();
                    if (SamplingRate>0
                     && SamplingCount*1000/SamplingRate<((float32)avih_TotalFrame)/avih_FrameRate*1000*1.10
                     && SamplingCount*1000/SamplingRate>((float32)avih_TotalFrame)/avih_FrameRate*1000*0.10)
                        SamplingCount=0; //Value disabled
                }
                //One AC-3 frame is 32 ms
                //One DTS frame is 21 ms

                float32 SamplingRate=Retrieve(Stream_Audio, StreamPos_Last, Audio_SamplingRate).To_float32();
                if (SamplingCount>0 && SamplingRate>0)
                    Fill(Stream_Audio, StreamPos_Last, Audio_Duration, SamplingCount*1000/SamplingRate, 0, true);
                else if (Temp->second.indx_Duration && Temp->second.Rate)
                    Fill(Stream_Audio, StreamPos_Last, Audio_Duration, ((float64)Temp->second.indx_Duration)*1000*Temp->second.Scale/Temp->second.Rate, 0, true);
                else if (Temp->second.Rate && Temp->second.Scale!=1) //Note: some files with Scale==1 are buggy
                    Fill(Stream_Audio, StreamPos_Last, Audio_Duration, ((float64)Temp->second.Length)*1000*Temp->second.Scale/Temp->second.Rate, 0, true);

                //Interleave
                if (Stream[0x30300000].PacketCount && Temp->second.PacketCount)
                {
                    Fill(Stream_Audio, StreamPos_Last, "Interleave_VideoFrames", (float)Stream[0x30300000].PacketCount/Temp->second.PacketCount, 2);
                    if (Retrieve(Stream_Video, 0, Video_FrameRate).To_float32())
                    {
                        Fill(Stream_Audio, StreamPos_Last, "Interleave_Duration", (float)Stream[0x30300000].PacketCount/Temp->second.PacketCount*1000/Retrieve(Stream_Video, 0, Video_FrameRate).To_float32(), 0);
                        Ztring Interleave_Duration_String;
                        Interleave_Duration_String+=Retrieve(Stream_Audio, StreamPos_Last, "Interleave_Duration");
                        Interleave_Duration_String+=__T(" ");
                        Interleave_Duration_String+=MediaInfoLib::Config.Language_Get(__T("ms"));
                        if (!Retrieve(Stream_Audio, StreamPos_Last, "Interleave_VideoFrames").empty())
                        {
                            Interleave_Duration_String+=__T(" (");
                            Interleave_Duration_String+=MediaInfoLib::Config.Language_Get(Retrieve(Stream_Audio, StreamPos_Last, "Interleave_VideoFrames"), __T(" video frames"));
                            Interleave_Duration_String+=__T(")");
                        }
                        Fill(Stream_Audio, StreamPos_Last, "Interleave_Duration/String", Interleave_Duration_String);
                    }
                    int64u Audio_FirstBytes=0;
                    for (std::map<int64u, stream_structure>::iterator Stream_Structure_Temp=Stream_Structure.begin(); Stream_Structure_Temp!=Stream_Structure.end(); ++Stream_Structure_Temp)
                    {
                        if (Stream_Structure_Temp->second.Name==0x30300000)
                            break;
                        if (Stream_Structure_Temp->second.Name==Temp->first)
                            Audio_FirstBytes+=Stream_Structure_Temp->second.Size;
                    }
                    if (Audio_FirstBytes && Temp->second.AvgBytesPerSec)
                    {
                        Fill(Stream_Audio, StreamPos_Last, "Interleave_Preload", Audio_FirstBytes*1000/Temp->second.AvgBytesPerSec);
                        Fill(Stream_Audio, StreamPos_Last, "Interleave_Preload/String", Retrieve(Stream_Audio, StreamPos_Last, "Interleave_Preload")+__T(" ")+MediaInfoLib::Config.Language_Get(__T("ms")));
                    }
                }
            }

            //Source duration
            if (Temp->second.PacketCount && Temp->second.Length!=Temp->second.PacketCount)
            {
                if (StreamKind_Last==Stream_Video && Temp->second.Rate)
                    Fill(Stream_Video, StreamPos_Last, "Source_Duration", ((float64)Temp->second.PacketCount)*1000*Temp->second.Scale/Temp->second.Rate, 0);
                if (StreamKind_Last==Stream_Audio && Temp->second.Rate)
                {
                    float64 Duration_Source=((float64)Temp->second.StreamSize)*1000/Temp->second.AvgBytesPerSec;
                    float64 Duration_Header=Retrieve(Stream_Audio, StreamPos_Last, Audio_Duration).To_float64();
                    float64 Difference=Duration_Source-Duration_Header;
                    if (Temp->second.Scale!=1 && float64_int64s(Duration_Header/Duration_Source)==Temp->second.Scale)
                        Fill(Stream_Audio, StreamPos_Last, Audio_Duration, Duration_Source, 0, true); //Found 1 stream with Scale not being right
                    else if (Difference<-2 || Difference>2) //+/- 2 ms
                        Fill(Stream_Audio, StreamPos_Last, "Source_Duration", Duration_Source, 0);
                }
            }
        }

        ++Temp;
    }

    //Some work on the first video stream
    if (Count_Get(Stream_Video))
    {
        //ODML
        if (dmlh_TotalFrame!=0 && Retrieve(Stream_Video, 0, Video_Duration).empty())
            for (size_t StreamPos=0; StreamPos<Count_Get(Stream_Video); StreamPos++)
                Fill(Stream_Video, StreamPos, Video_FrameCount, dmlh_TotalFrame, 10, true);
    }

    //Rec
    if (rec__Present)
        Fill(Stream_General, 0, General_Format_Settings, "rec");

    //Interleaved
    if (Interleaved0_1 && Interleaved0_10 && Interleaved1_1 && Interleaved1_10)
        Fill(Stream_General, 0, General_Interleaved, ((Interleaved0_1<Interleaved1_1 && Interleaved0_10>Interleaved1_1)
                                                   || (Interleaved1_1<Interleaved0_1 && Interleaved1_10>Interleaved0_1))?"Yes":"No");

    //MD5
    size_t Pos=0;
    for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
        for (size_t StreamPos=0; StreamPos<Count_Get((stream_t)StreamKind); StreamPos++)
            if (Pos<MD5s.size())
            {
                Fill((stream_t)StreamKind, StreamPos, "MD5", MD5s[Pos]);
                Pos++;
            }

    //Commercial names
    if (Count_Get(Stream_Video)==1)
    {
        Streams_Finish_StreamOnly();
             if (!Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny).empty())
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny));
            Fill(Stream_General, 0, General_Format_Commercial, __T("AVI ")+Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny));
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("DV"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "DV");
            Fill(Stream_General, 0, General_Format_Commercial, "AVI DV");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==__T("30000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "MPEG IMX 30");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "MPEG IMX 30");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==__T("40000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "MPEG IMX 40");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "MPEG IMX 40");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==__T("50000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "MPEG IMX 50");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "MPEG IMX 50");
        }
    }
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Riff::Read_Buffer_Init()
{
    #if MEDIAINFO_DEMUX
         Demux_UnpacketizeContainer=Config->Demux_Unpacketize_Get();
         Demux_Rate=Config->Demux_Rate_Get();
         if (Demux_Rate==0)
             Demux_Rate=25; //Default value
    #endif //MEDIAINFO_DEMUX

    PAR=0;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_Riff::Read_Buffer_Seek (size_t Method, int64u Value, int64u /*ID*/)
{
    //Only Wave and AIFF
    switch (Kind)
    {
        case Kind_Wave :
        case Kind_Aiff :
                         break;
        default        : return (size_t)-1;
    }

    //Parsing
    switch (Method)
    {
        case 0  :
                    if (Value<Buffer_DataToParse_Begin)
                        Value=Buffer_DataToParse_Begin;
                    if (Value>Buffer_DataToParse_End)
                        Value=Buffer_DataToParse_End;
                    GoTo(Value);
                    Open_Buffer_Unsynch();
                    return 1;
        case 1  :
                    GoTo(Buffer_DataToParse_Begin+(Buffer_DataToParse_End-Buffer_DataToParse_Begin)*Value/10000);
                    Open_Buffer_Unsynch();
                    return 1;
        case 2  :   //Timestamp
                    {
                    if (AvgBytesPerSec==0)
                        return (size_t)-1;

                    float64 ValueF=(float64)Value;
                    ValueF/=1000000000; //Value is in ns
                    ValueF*=AvgBytesPerSec;
                    GoTo(Buffer_DataToParse_Begin+float64_int64s(ValueF));
                    return 1;
                    }
        case 3  :   //FrameNumber
                    {
                    if (AvgBytesPerSec==0 || Demux_Rate==0 || BlockAlign==0)
                        return (size_t)-1;

                    float64 BytesPerFrame=AvgBytesPerSec/Demux_Rate;
                    int64u StreamOffset=(int64u)(Value*BytesPerFrame);
                    StreamOffset/=BlockAlign;
                    StreamOffset*=BlockAlign;

                    GoTo(Buffer_DataToParse_Begin+StreamOffset);
                    return 1;
                    }
        default :   return (size_t)-1; //Not supported
    }
}
#endif //MEDIAINFO_SEEK

//---------------------------------------------------------------------------
void File_Riff::Read_Buffer_Unsynched()
{
    for (std::map<int32u, stream>::iterator Stream_Item=Stream.begin(); Stream_Item!=Stream.end(); ++Stream_Item)
        for (size_t Pos=0; Pos<Stream_Item->second.Parsers.size(); Pos++)
            Stream_Item->second.Parsers[Pos]->Open_Buffer_Unsynch();

    if (IsSub)
    {
        while(Element_Level)
            Element_End0();

        #if defined(MEDIAINFO_ANCILLARY_YES)
            //Ancillary specific
            if (Ancillary && (*Ancillary))
                (*Ancillary)->Open_Buffer_Unsynch();
        #endif //defined(MEDIAINFO_ANCILLARY_YES)
    }
}

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
void File_Riff::Read_Buffer_Continue()
{
    if (Demux_Parser)
    {
        Open_Buffer_Continue(Demux_Parser, Buffer+Buffer_Offset, 0, false);
        Demux_Parser=NULL;
        //if (Config->Demux_EventWasSent)
        //    return;
    }
}
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Riff::Header_Begin()
{
    while (File_Offset+Buffer_Offset<Buffer_DataToParse_End)
    {
        #if MEDIAINFO_DEMUX
            if (AvgBytesPerSec && Demux_Rate && BlockAlign)
            {
                float64 BytesPerFrame=((float64)AvgBytesPerSec)/Demux_Rate;
                Frame_Count_NotParsedIncluded=float64_int64s(((float64)(File_Offset+Buffer_Offset-Buffer_DataToParse_Begin))/BytesPerFrame);
                Element_Size=float64_int64s(BytesPerFrame*(Frame_Count_NotParsedIncluded+1));
                Element_Size/=BlockAlign;
                Element_Size*=BlockAlign;
                Element_Size-=File_Offset+Buffer_Offset-Buffer_DataToParse_Begin;
                FrameInfo.PTS=FrameInfo.DTS=float64_int64s(((float64)Frame_Count_NotParsedIncluded)*1000000000/Demux_Rate);
                while (Element_Size && File_Offset+Buffer_Offset+Element_Size>Buffer_DataToParse_End)
                    Element_Size-=BlockAlign;
                if (Element_Size==0)
                    Element_Size=BlockAlign;
                if (Buffer_Offset+Element_Size>Buffer_Size)
                    return false;
            }
            else
        #endif //MEDIAINFO_DEMUX
        if (File_Offset+Buffer_Size<=Buffer_DataToParse_End)
            Element_Size=Buffer_Size; //All the buffer is used
        else
        {
            Element_Size=File_Offset+Buffer_Size-Buffer_DataToParse_End;
            Buffer_DataToParse_End=0;
        }

        if (Buffer_Offset+(size_t)Element_Size>Buffer_Size)
            return false;

        // Fake header
        Element_Begin0();
        Element_Begin0();
        Header_Fill_Size(Buffer_DataToParse_End-(File_Offset+Buffer_Offset));
        Element_End();

        switch (Kind)
        {
            case Kind_Wave : WAVE_data_Continue(); break;
            case Kind_Aiff : AIFF_SSND_Continue(); break;
            case Kind_Rmp3 : RMP3_data_Continue(); break;
            default        : AVI__movi_xxxx();
        }

        if (Config->ParseSpeed<1.0 && File_Offset+Buffer_Offset+Element_Offset-Buffer_DataToParse_Begin>=256*1024)
        {
            File_GoTo=Buffer_DataToParse_End;
            Buffer_Offset=Buffer_Size;
            Element_Size=0;
        }
        else
        {
            Buffer_Offset+=(size_t)Element_Size;
            Element_Size-=Element_Offset;
        }
        Element_Offset=0;
        Element_End0();

        if (Status[IsFinished] || (File_GoTo!=(int64u)-1 && (File_GoTo<=Buffer_DataToParse_Begin || File_GoTo>=Buffer_DataToParse_End)))
        {
            Buffer_DataToParse_Begin=(int64u)-1;
            Buffer_DataToParse_End=0;
            return false;
        }

        if (Buffer_Offset>=Buffer_Size)
            return false;

        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
                return false;
        #endif //MEDIAINFO_DEMUX
    }

    return true;
}

//---------------------------------------------------------------------------
void File_Riff::Header_Parse()
{
    //Special case : W3DI tags (unknown format!) are at the end of the file
    if (Element_Level==2 && File_Offset+Buffer_Size==File_Size && Buffer_Size>8)
    {
        if (CC4(Buffer+Buffer_Size-4)==Elements::W3DI)
        {
            int32u Size=LittleEndian2int32u(Buffer+Buffer_Size-8);
            if (Size>8 && Size<=Buffer_Size && Buffer_Offset+Size==Buffer_Size)
            {
                //Filling
                Header_Fill_Code(Elements::W3DI, "W3DI");
                Header_Fill_Size(Size);
                return;
            }
        }
    }

    //Special case : SMV file detected
    if (SMV_BlockSize)
    {
        //Filling
        Header_Fill_Code(Elements::SMV0_xxxx, "SMV Block");
        Header_Fill_Size(SMV_BlockSize);
        return;
    }

    //Parsing
    int32u Size, Name;
    Get_C4 (Name,                                               "Name");
    if (Name==Elements::SMV0)
    {
        //SMV specific
        //Filling
        Header_Fill_Code(Elements::SMV0, "SMV header");
        Header_Fill_Size(51);
        return;
    }
    if (Name==Elements::riff)
        IsWave64=true;
    if (IsWave64)
    {
        //Wave64 specific
        int64u Size_Complete;
        Skip_XX(12,                                             "Name (GUID)");
        Get_L8 (Size_Complete,                                  "Size");

        //Alignment
        if (Name!=Elements::riff && Size_Complete%8)
        {
            Alignement_ExtraByte=Size_Complete%8;
            Size_Complete+=Alignement_ExtraByte; //Always 8-byte aligned
        }
        else
            Alignement_ExtraByte=0;

        //Top level chunks
        if (Name==Elements::riff)
        {
            Get_C4 (Name,                                       "Real Name");
            Skip_XX(12,                                         "Real Name (GUID)");
        }

        //Filling
        Header_Fill_Code(Name, Ztring().From_CC4(Name));
        Header_Fill_Size(Size_Complete);
        return;
    }
    if (Name==Elements::FORM
     || Name==Elements::MThd)
        IsBigEndian=true; //Swap from Little to Big Endian for "FORM" files (AIFF...)
    if (IsBigEndian)
        Get_B4 (Size,                                           "Size");
    else
    {
        Get_L4 (Size,                                           "Size");

        //Testing malformed (not word aligned)
        if (!IsNotWordAligned_Tested && Size%2)
        {
            if (File_Offset+Buffer_Offset+8+Size==File_Size)
                IsNotWordAligned=true;
            else if (!File_Name.empty())
            {
                File F(File_Name);
                F.GoTo(File_Offset+Buffer_Offset+8+Size);
                int8u Temp;
                if (F.Read(&Temp, 1))
                {
                    if (!((Temp<'A' || Temp>'z') && Temp!=' '))
                        IsNotWordAligned=true;
                }
            }
            IsNotWordAligned_Tested=true;
        }
    }

    //RF64
    int64u Size_Complete=Size;
    if (Size==0 && Name==Elements::RIFF)
        Size_Complete=File_Size-8;
    else if (Size==0xFFFFFFFF)
    {
        if (Element_Size<0x1C)
        {
            Element_WaitForMoreData();
            return;
        }
        if (Name==Elements::RF64 && CC4(Buffer+Buffer_Offset+0x0C)==Elements::WAVE_ds64)
        {
            Size_Complete=LittleEndian2int64u(Buffer+Buffer_Offset+0x14);
            Param_Info1(Size_Complete);
        }
        else if (Name==Elements::WAVE_data)
        {
            Size_Complete=WAVE_data_Size;
            Param_Info1(Size_Complete);
        }
    }

    //Coherency
    if (Stream_Structure_Temp!=Stream_Structure.end() && Stream_Structure_Temp->second.Size==0)
    {
        Name=(int32u)-1;
        Size_Complete=0; //Hack in some indexes with Size==0 (why?), ignoring content of header
    }
    if (File_Offset+Buffer_Offset+8+Size_Complete>File_Size)
        Size_Complete=File_Size-(File_Offset+Buffer_Offset+8);

    //Alignment
    if (Size_Complete%2 && !IsNotWordAligned)
    {
        Size_Complete++; //Always 2-byte aligned
        Alignement_ExtraByte=1;
    }
    else
        Alignement_ExtraByte=0;

    //Top level chunks
    if (Name==Elements::LIST
     || Name==Elements::RIFF
     || Name==Elements::RF64
     || Name==Elements::ON2_
     || Name==Elements::FORM)
    {
        if (Name==Elements::RF64)
            IsRIFF64=true;
        Get_C4 (Name,                                           "Real Name");
    }

    //Integrity
    if (Name==0x00000000)
    {
        //Filling
        Header_Fill_Code(0, "Junk");
        Header_Fill_Size(File_Size-(File_Offset+Buffer_Offset));
        Alignement_ExtraByte=0;
        return;
    }

    //Specific
    if (Name==Elements::ON2f)
        Name=Elements::AVI_;

    //Tests
    if (Element_Level==2 && Name==Elements::WAVE && !IsRIFF64 && File_Size>0xFFFFFFFF)
        IsWaveBroken=true; //Non standard big files detection
    if (IsWaveBroken && (Name==Elements::WAVE || Name==Elements::WAVE_data))
        Size_Complete=File_Size-(File_Offset+Buffer_Offset+8); //Non standard big files detection
    if (movi_Size && Size_Complete>movi_Size/2 && 8+Size_Complete>1024*1024 && !((Name&0xFFFF0000)==0x69780000 || (Name&0x0000FFFF)==0x00006978) && Element_Level==(rec__Present?(size_t)5:(size_t)4) && Buffer_Offset+8+Size_Complete>Buffer_Size)
    {
        Buffer_DataToParse_End=File_Offset+Buffer_Offset+8+Size_Complete;
        Size_Complete=Buffer_Size-(Buffer_Offset+8);
    }
    if ((Name==Elements::WAVE_data || Name==Elements::AIFF_SSND))
    {
        Buffer_DataToParse_Begin=File_Offset+Buffer_Offset+8;
        if (Size_Complete)
            Buffer_DataToParse_End=File_Offset+Buffer_Offset+8+Size_Complete;
        else
            Buffer_DataToParse_End=File_Size; //Found one file with 0 as size of data part
        Size_Complete=0;
    }

    //Filling
    Header_Fill_Code(Name, Ztring().From_CC4(Name));
    Header_Fill_Size(Size_Complete+8);
}

//---------------------------------------------------------------------------
bool File_Riff::BookMark_Needed()
{
    //Go to the first usefull chunk
    if (stream_Count==0 && Stream_Structure.empty())
        return false; //No need

    Stream_Structure_Temp=Stream_Structure.begin();
    if (!Stream_Structure.empty())
    {
        #if MEDIAINFO_MD5
            if (Config->File_Md5_Get())
            {
                GoTo(0);
                Md5_ParseUpTo=Stream_Structure_Temp->first;
            }
            else
        #endif //MEDIAINFO_MD5
                GoTo(Stream_Structure_Temp->first);
    }
    NeedOldIndex=false;
    SecondPass=true;
    Index_Pos.clear(); //We didn't succeed to find theses indexes :(
    return true;
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Riff::TimeCode_Fill(const Ztring &Name, const Ztring &Value)
{
    if (Value.empty())
        return;

    for (size_t StreamPos=0; StreamPos<Count_Get(Stream_Video); StreamPos++)
    {
        Fill(Stream_Video, StreamPos, Video_TimeCode_FirstFrame, Value);
        Fill(Stream_Video, StreamPos, Video_TimeCode_Source, Name);
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_RIFF_YES
