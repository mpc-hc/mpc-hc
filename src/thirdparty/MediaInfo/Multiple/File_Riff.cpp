// File_Riff - Info for RIFF files
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
        StreamIDs_Width[0]=17;
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
    Buffer_DataSizeToParse=0;
    avih_FrameRate=0;
    avih_TotalFrame=0;
    dmlh_TotalFrame=0;
    Idx1_Offset=(int64u)-1;
    movi_Size=0;
    TimeReference=(int64u)-1;
    SMV_BlockSize=0;
    stream_Count=0;
    rec__Present=false;
    NeedOldIndex=true;
    IsBigEndian=false;
    IsWave64=false;
    IsRIFF64=false;
    IsWaveBroken=false;
    SecondPass=false;
    DV_FromHeader=NULL;

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
    //Global
    if (IsRIFF64)
        Fill(Stream_General, 0, General_Format_Profile, "RF64");

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

        //Parser specific
        if (Temp->second.Parser)
        {
            //Finalizing and Merging (except Video codec and 120 fps hack)
            Temp->second.Parser->ShouldContinueParsing=false;

            //Hack - Before
            Ztring StreamSize, Codec_Temp;
            if (StreamKind_Last==Stream_Video)
                Codec_Temp=Retrieve(Stream_Video, StreamPos_Last, Video_Codec); //We want to keep the 4CC of AVI
            StreamSize=Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_StreamSize)); //We want to keep the 4CC of AVI

            //Merging
            Finish(Temp->second.Parser);
            Merge(*Temp->second.Parser, StreamKind_Last, 0, StreamPos_Last);
            Fill(StreamKind_Last, StreamPos_Last, General_ID, ((Temp->first>>24)-'0')*10+(((Temp->first>>16)&0xFF)-'0'));

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
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_String, MediaInfoLib::Config.Language_Get(FrameRate+_T(" (24/30)"), _T(" fps")), true);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Minimum, 24, 10, true);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Maximum, 30, 10, true);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Mode, "VFR");
                }
            }

            //Alignment
            if (StreamKind_Last==Stream_Audio && Count_Get(Stream_Video)>0) //Only if this is not a WAV file
            {
                Fill(Stream_Audio, StreamPos_Last, Audio_Alignment, Temp->second.ChunksAreComplete?"Aligned":"Split");
                Fill(Stream_Audio, StreamPos_Last, Audio_Alignment_String, MediaInfoLib::Config.Language_Get(Temp->second.ChunksAreComplete?_T("Alignment_Aligned"):_T("Alignment_Split")));
            }

            //Delay
            if (StreamKind_Last==Stream_Audio && Count_Get(Stream_Video)==1 && Temp->second.Rate!=0 && Temp->second.Parser->Status[IsAccepted])
            {
                float Delay=0;
                bool Delay_IsValid=false;

                     if (Temp->second.Parser->Buffer_TotalBytes_FirstSynched==0)
                {
                    Delay=0;
                    Delay_IsValid=true;
                }
                else if (Temp->second.Rate!=0)
                {
                    Delay=((float)Temp->second.Parser->Buffer_TotalBytes_FirstSynched)*1000/Temp->second.Rate;
                    Delay_IsValid=true;
                }
                else if (Temp->second.Parser->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int64u()!=0)
                {
                    Delay=((float)Temp->second.Parser->Buffer_TotalBytes_FirstSynched)*1000/Temp->second.Parser->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int64u();
                    Delay_IsValid=true;
                }
                else if (Temp->second.Parser->Retrieve(Stream_Audio, 0, Audio_BitRate_Nominal).To_int64u()!=0)
                {
                    Delay=((float)Temp->second.Parser->Buffer_TotalBytes_FirstSynched)*1000/Temp->second.Parser->Retrieve(Stream_Audio, 0, Audio_BitRate_Nominal).To_int64u();
                    Delay_IsValid=true;
                }

                if (Delay_IsValid)
                {
                    Delay+=((float)Temp->second.Start)*1000/Temp->second.Rate;
                    Fill(Stream_Audio, StreamPos_Last, Audio_Delay, Delay, 0, true);
                    Fill(Stream_Audio, StreamPos_Last, Audio_Delay_Source, "Container");
                    Fill(Stream_Video, 0, Video_Delay, 0, 10, true);
                }
            }

            //Special case: AAC
            if (StreamKind_Last==Stream_Audio
             && (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==_T("AAC")
              || Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==_T("MPEG Audio")
              || Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==_T("Vorbis")))
                Clear(Stream_Audio, StreamPos_Last, Audio_Resolution); //Resolution is not valid for AAC / MPEG Audio / Vorbis

            //Format specific
            #if defined(MEDIAINFO_MPEG4V_YES)
                if (StreamKind_Last==Stream_Video && MediaInfoLib::Config.Codec_Get(Ztring().From_CC4(Temp->second.Compression), InfoCodec_KindofCodec).find(_T("MPEG-4V"))==0)
                {
                    if (((File_Mpeg4v*)Temp->second.Parser)->Frame_Count_InThisBlock>1)
                    {
                        Fill(Stream_Video, StreamPos_Last, Video_MuxingMode, MediaInfoLib::Config.Language_Get("MuxingMode_PackedBitstream"));
                        Fill(Stream_Video, StreamPos_Last, Video_Codec_Settings, "Packed Bitstream");
                        Fill(Stream_Video, StreamPos_Last, Video_Codec_Settings_PacketBitStream, "Yes");
                    }
                    else
                    {
                        Fill(Stream_Video, StreamPos_Last, Video_Codec_Settings_PacketBitStream, "No");
                    }
                }
            #endif
            #if defined(MEDIAINFO_DVDIF_YES)
                if (StreamKind_Last==Stream_Video && (MediaInfoLib::Config.Codec_Get(Ztring().From_CC4(Temp->second.Compression), InfoCodec_KindofCodec).find(_T("DV"))==0
                                                   || Retrieve(Stream_Video, StreamPos_Last, Video_Format)==_T("DV")
                                                   || Retrieve(Stream_Video, StreamPos_Last, Video_Codec)==_T("DV")))
                {
                    if (Retrieve(Stream_General, 0, General_Recorded_Date).empty())
                        Fill(Stream_General, 0, General_Recorded_Date, Temp->second.Parser->Retrieve(Stream_General, 0, General_Recorded_Date));

                    //Video and Audio are together
                    size_t Audio_Count=Temp->second.Parser->Count_Get(Stream_Audio);
                    for (size_t Audio_Pos=0; Audio_Pos<Audio_Count; Audio_Pos++)
                    {
                        Fill_Flush();
                        Stream_Prepare(Stream_Audio);
                        size_t Pos=Count_Get(Stream_Audio)-1;
                        Merge(*Temp->second.Parser, Stream_Audio, Audio_Pos, StreamPos_Last);
                        Fill(Stream_Audio, Pos, Audio_MuxingMode, "DV");
                        Fill(Stream_Audio, Pos, Audio_Duration, Retrieve(Stream_Video, Temp->second.StreamPos, Video_Duration));
                        Fill(Stream_Audio, Pos, "MuxingMode_MoreInfo", _T("Muxed in Video #")+Ztring().From_Number(Temp->second.StreamPos+1));
                        Fill(Stream_Audio, Pos, Audio_StreamSize, "0"); //Included in the DV stream size
                        Ztring ID=Retrieve(Stream_Audio, Pos, Audio_ID);
                        Fill(Stream_Audio, Pos, Audio_ID, Retrieve(Stream_Video, Temp->second.StreamPos, Video_ID)+_T("-")+ID, true);
                    }

                    StreamKind_Last=Stream_Video;
                    StreamPos_Last=Temp->second.StreamPos;
                }
            #endif
        }
        else if (StreamKind_Last!=Stream_General)
            Fill(StreamKind_Last, StreamPos_Last, General_ID, ((Temp->first>>24)-'0')*10+(((Temp->first>>16)&0xFF)-'0'));

        //Duration
        if (Temp->second.PacketCount>0)
        {
            if (StreamKind_Last==Stream_Video)
            {
                //Duration in case it is missing from header (malformed header...)
                float32 FrameRate=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate).To_float32();
                if (FrameRate>0)
                    Fill(Stream_Video, StreamPos_Last, Video_Duration, Temp->second.PacketCount*1000/FrameRate, 0, true);
            }
            if (StreamKind_Last==Stream_Audio)
            {
                //Duration in case it is missing from header (malformed header...)
                int64u SamplingCount=0;
                #if defined(MEDIAINFO_MPEGA_YES)
                if (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==_T("MPEG Audio"))
                {
                    if (Temp->second.Parser && Temp->second.PacketPos==((File_Mpega*)Temp->second.Parser)->Frame_Count_Valid) //Only for stream with one frame per chunk
                    {
                        if (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format_Profile)==_T("Layer 1"))
                            SamplingCount=Temp->second.PacketCount*384;  //Layer 1
                        else
                            SamplingCount=Temp->second.PacketCount*1152; //Layer 2 and 3
                    }
                }
                #endif
                if (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==_T("PCM"))
                {
                    int64u Resolution=Retrieve(Stream_Audio, StreamPos_Last, Audio_Resolution).To_int64u();
                    int64u Channels=Retrieve(Stream_Audio, StreamPos_Last, Audio_Channel_s_).To_int64u();
                    if (Resolution>0 && Channels>0)
                        SamplingCount=Temp->second.StreamSize*8/Resolution/Channels;
                }
                if (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==_T("ADPCM"))
                {
                    int64u Resolution=Retrieve(Stream_Audio, StreamPos_Last, Audio_Resolution).To_int64u();
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

                //Interleave
                if (Stream[0x30300000].PacketCount && Temp->second.PacketCount)
                {
                    Fill(Stream_Audio, StreamPos_Last, "Interleave_VideoFrames", (float)Stream[0x30300000].PacketCount/Temp->second.PacketCount, 2);
                    if (Retrieve(Stream_Video, 0, Video_FrameRate).To_float32())
                    {
                        Fill(Stream_Audio, StreamPos_Last, "Interleave_Duration", (float)Stream[0x30300000].PacketCount/Temp->second.PacketCount*1000/Retrieve(Stream_Video, 0, Video_FrameRate).To_float32(), 0);
                        Ztring Interleave_Duration_String;
                        Interleave_Duration_String+=Retrieve(Stream_Audio, StreamPos_Last, "Interleave_Duration");
                        Interleave_Duration_String+=_T(" ");
                        Interleave_Duration_String+=MediaInfoLib::Config.Language_Get(_T("ms"));
                        if (!Retrieve(Stream_Audio, StreamPos_Last, "Interleave_VideoFrames").empty())
                        {
                            Interleave_Duration_String+=_T(" (");
                            Interleave_Duration_String+=MediaInfoLib::Config.Language_Get(Retrieve(Stream_Audio, StreamPos_Last, "Interleave_VideoFrames"), _T(" video frames"));
                            Interleave_Duration_String+=_T(")");
                        }
                        Fill(Stream_Audio, StreamPos_Last, "Interleave_Duration/String", Interleave_Duration_String);
                    }
                    int64u Audio_FirstBytes=0;
                    for (std::map<int64u, stream_structure>::iterator Stream_Structure_Temp=Stream_Structure.begin(); Stream_Structure_Temp!=Stream_Structure.end(); Stream_Structure_Temp++)
                    {
                        if (Stream_Structure_Temp->second.Name==0x30300000)
                            break;
                        if (Stream_Structure_Temp->second.Name==Temp->first)
                            Audio_FirstBytes+=Stream_Structure_Temp->second.Size;
                    }
                    if (Audio_FirstBytes && Retrieve(Stream_Audio, StreamPos_Last, Audio_BitRate).To_int32u())
                    {
                        Fill(Stream_Audio, StreamPos_Last, "Interleave_Preload", Audio_FirstBytes*1000/Temp->second.AvgBytesPerSec);
                        Fill(Stream_Audio, StreamPos_Last, "Interleave_Preload/String", Retrieve(Stream_Audio, StreamPos_Last, "Interleave_Preload")+_T(" ms"));
                    }
                }
            }
        }

        Temp++;
    }

    //Some work on the first video stream
    if (Count_Get(Stream_Video))
    {
        //ODML
        if (dmlh_TotalFrame!=0 && Retrieve(Stream_Video, 0, Video_Duration).empty())
            Fill(Stream_Video, 0, Video_FrameCount, dmlh_TotalFrame, 10, true);
    }

    //Rec
    if (rec__Present)
        Fill(Stream_General, 0, General_Format_Settings, "rec");

    //Interleaved
    if (Interleaved0_1 && Interleaved0_10 && Interleaved1_1 && Interleaved1_10)
        Fill(Stream_General, 0, General_Interleaved, (Interleaved0_1<Interleaved1_1 && Interleaved0_10>Interleaved1_1
                                                   || Interleaved1_1<Interleaved0_1 && Interleaved1_10>Interleaved0_1)?"Yes":"No");

    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
    {
        Stream.clear();
        Stream_Structure.clear();
        delete DV_FromHeader; DV_FromHeader=NULL;
    }

    //Commercial names
    if (Count_Get(Stream_Video)==1)
    {
        Streams_Finish_StreamOnly();
             if (!Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny).empty())
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny));
            Fill(Stream_General, 0, General_Format_Commercial, _T("AVI ")+Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny));
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("DV"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "DV");
            Fill(Stream_General, 0, General_Format_Commercial, "AVI DV");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("30000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "MPEG IMX 30");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "MPEG IMX 30");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("40000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "MPEG IMX 40");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "MPEG IMX 40");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("50000000"))
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
void File_Riff::Read_Buffer_Continue()
{
    if (Buffer_DataSizeToParse)
    {
        if (Buffer_Size<=Buffer_DataSizeToParse)
        {
            Element_Size=Buffer_Size; //All the buffer is used
            Buffer_DataSizeToParse-=Buffer_Size;
        }
        else
        {
            Element_Size=Buffer_DataSizeToParse;
            Buffer_DataSizeToParse=0;
        }

        Element_Begin();
        AVI__movi_xxxx();
        Element_Offset=Element_Size;
        Element_End();
    }
}

//***************************************************************************
// Buffer
//***************************************************************************

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
        Get_L4 (Size,                                           "Size");

    //RF64
    int64u Size_Complete=Size;
    if (Size==0xFFFFFFFF)
    {
        if (Element_Size<0x1C)
        {
            Element_WaitForMoreData();
            return;
        }
        if (Name==Elements::RF64 && CC4(Buffer+Buffer_Offset+0x0C)==Elements::WAVE_ds64)
        {
            Size_Complete=LittleEndian2int64u(Buffer+Buffer_Offset+0x14);
            Param_Info(Size_Complete);
        }
        else if (Name==Elements::WAVE_data)
        {
            Size_Complete=WAVE_data_Size;
            Param_Info(Size_Complete);
        }
    }

    //Alignment
    if (Size_Complete%2)
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

    //Filling
    if (movi_Size && Element_Level==4 && Size_Complete+8>1024*1024)
    {
        Buffer_DataSizeToParse=Size_Complete+8;
        Size_Complete=Buffer_Size-(Buffer_Offset+8);
        Buffer_DataSizeToParse-=Size_Complete;
    }

    //Filling
    Header_Fill_Code(Name, Ztring().From_CC4(Name));
    if (Element_Level==2 && Name==Elements::WAVE && !IsRIFF64 && File_Size>0xFFFFFFFF)
        IsWaveBroken=true; //Non standard big files detection
    if (IsWaveBroken && (Name==Elements::WAVE || Name==Elements::WAVE_data))
        Size_Complete=File_Size-(File_Offset+Buffer_Offset+8); //Non standard big files detection
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
        GoTo(Stream_Structure_Temp->first);
    NeedOldIndex=false;
    SecondPass=true;
    Index_Pos.clear(); //We didn't succeed to find theses indexes :(
    return true;
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_RIFF_YES
