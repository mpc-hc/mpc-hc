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
#if defined(MEDIAINFO_DVDIF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_DvDif.h"
#if defined(MEDIAINFO_EIA608_YES)
    #include "MediaInfo/Text/File_Eia608.h"
#endif
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#if MEDIAINFO_SEEK
    #include "MediaInfo/MediaInfo_Internal.h"
#endif //MEDIAINFO_SEEK
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
const char*  Dv_sct[]=
{
    "Header",
    "Subcode",
    "VAUX",
    "Audio",
    "Video",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* Dv_Ssyb_Pc0(int8u Pc0)
{
    switch (Pc0)
    {
        case 0x13 : return "Timecode";
        case 0x14 : return "Binary group";
        case 0x50 :
        case 0x60 : return "Source";
        case 0x51 :
        case 0x61 : return "Source control";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char*  Dv_Disp[]=
{
    "4/3",                              //S306M, S314M
    "16/9",                             //S306M
    "16/9",                             //S306M, S314M
    "4/3",                              //Which spec?
    "",
    "",
    "",
    "16/9 or 4/3 depends of ssyb AP3", //Which spec?
};

//---------------------------------------------------------------------------
const int32u  Dv_Audio_SamplingRate[]=
{
    48000,
    44100,
    32000,
    0,
    0,
    0,
    0,
    0,
};

//---------------------------------------------------------------------------
const char*  Dv_StereoMode[]=
{
    "Multi-Stero",
    "Lumped",
};

//---------------------------------------------------------------------------
const int32u  Dv_Audio_BitDepth[]=
{
    16,
    12,
    0,
    0,
    0,
    0,
    0,
    0,
};

//---------------------------------------------------------------------------
const int8u  Dv_ChannelsPerBlock[]=
{
    1,
    2,
    0,
    0,
};

//---------------------------------------------------------------------------
const char*  Dv_Pair[]=
{
    "One pair of channels",
    "Independent channels",
};

//---------------------------------------------------------------------------
const char*  Dv_CopyGenerationManagementSystem[]=
{
    "Unrestricted",
    "Not used",
    "One generation only",
    "No copy",
};

//---------------------------------------------------------------------------
const char*  Dv_InputType[]=
{
    "Analog",
    "Digital",
    "Reserved",
    "No information",
};

//---------------------------------------------------------------------------
const char*  Dv_CompressionTimes[]=
{
    "Once",
    "Twice",
    "Three+",
    "No information",
};

//---------------------------------------------------------------------------
const char*  Dv_Emphasis[]=
{
    "Enphasis off",
    "Enphasis on",
    "Reserved",
    "Reserved",
};

//---------------------------------------------------------------------------
const char*  Dv_consumer_camera_1_ae_mode[]=
{
    "full automatic",
    "gain priority mode",
    "shutter priority mode",
    "iris priority mode",
    "manual",
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
    "", //no info
};

//---------------------------------------------------------------------------
const char*  Dv_consumer_camera_1_wb_mode[]=
{
    "automatic",
    "hold",
    "one push",
    "pre-set",
    "",
    "",
    "",
    "", //no info
};

//---------------------------------------------------------------------------
const char* Dv_consumer_camera_1_white_balance(int8u white_balance)
{
    switch (white_balance)
    {
        case 0x00 : return "candle";
        case 0x01 : return "incandescent lamp";
        case 0x02 : return "low color temperature; florescent lamp";
        case 0x03 : return "high color temperature; florescent lamp";
        case 0x04 : return "sunlight";
        case 0x05 : return "cloudy weather";
        case 0x1F : return ""; //No info
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char*  Dv_consumer_camera_1_fcm[]=
{
    "auto focus",
    "manual focus",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_DvDif::File_DvDif()
:File__Analyze()
{
    //Configuration
    ParserName=__T("DV");
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;

    //In
    Frame_Count_Valid=2;
    AuxToAnalyze=0x00; //No Aux to analyze
    IgnoreAudio=false;

    //Temp
    FrameSize_Theory=0;
    Duration=0;
    TimeCode_FirstFrame_ms=(int64u)-1;
    Synched_Test_Reset();
    DSF_IsValid=false;
    APT=0xFF; //Impossible
    video_source_stype=0xFF;
    audio_source_stype=0xFF;
    ssyb_AP3=0xFF;
    TF1=false; //Valid by default, for direct analyze
    TF2=false; //Valid by default, for direct analyze
    TF3=false; //Valid by default, for direct analyze
    system=false;
    FSC_WasSet=false;
    FSP_WasNotSet=false;
    video_sourcecontrol_IsParsed=false;
    audio_locked=false;

    #if MEDIAINFO_SEEK
        Duration_Detected=false;
    #endif //MEDIAINFO_SEEK

    #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
    Analyze_Activated=false;
    video_source_Detected=false;
    Speed_FrameCount=0;
    Speed_FrameCount_Video_STA_Errors=0;
    Speed_FrameCount_Audio_Errors.resize(8);
    Speed_FrameCount_Timecode_Incoherency=0;
    Speed_FrameCount_Contains_NULL=0;
    Speed_Contains_NULL=0;
    Speed_FrameCount_Arb_Incoherency=0;
    Speed_FrameCount_Stts_Fluctuation=0;
    System_IsValid=false;
    Frame_AtLeast1DIF=false;
    QU=(int8u)-1;
    CH_IsPresent.resize(8);
    Speed_TimeCode_IsValid=false;
    Speed_Arb_IsValid=false;
    Mpeg4_stts=NULL;
    Mpeg4_stts_Pos=0;
    Stats.resize(10);
    Stats_Total=0;
    Stats_Total_WithoutArb=0;
    Stats_Total_AlreadyDetected=false;
    #endif //MEDIAINFO_DVDIF_ANALYZE_YES
}

//---------------------------------------------------------------------------
File_DvDif::~File_DvDif()
{
    for (size_t Pos=0; Pos<Streams_Audio.size(); Pos++)
        delete Streams_Audio[Pos];

    #if defined(MEDIAINFO_EIA608_YES)
        for (size_t Pos=0; Pos<CC_Parsers.size(); Pos++)
            delete CC_Parsers[Pos]; //CC_Parsers[Pos]=NULL;
    #endif
    #if defined(MEDIAINFO_DVDIF_ANALYZE_YES)
        delete Mpeg4_stts; //Mpeg4_stts=NULL;
    #endif
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_DvDif::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format, "DV", Unlimited, true, true);

    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "DV");
    Fill(Stream_Video, 0, Video_Codec, "DV");
    Fill(Stream_Video, 0, Video_Standard, system?"PAL":"NTSC");
    Fill(Stream_Video, 0, Video_BitDepth, 8);
    bool IsHd=false;
    float FrameRate_Multiplicator=1;
    switch (video_source_stype)
    {
        case 0x00 :
        case 0x04 :
                    Fill(Stream_Video, 0, Video_Width, 720);
                    Fill(Stream_Video, 0, Video_Height, system?576:480);
                    break;
        case 0x14 :
        case 0x15 :
                    Fill(Stream_Video, 0, Video_Width, system?1440:1280);
                    Fill(Stream_Video, 0, Video_Height, video_source_stype==0x14?1080:1035);
                    IsHd=true;
                    break;
        case 0x18 :
                    Fill(Stream_Video, 0, Video_Width, 960);
                    Fill(Stream_Video, 0, Video_Height, 720);
                    FrameRate_Multiplicator=2;
                    IsHd=true;
                    break;
        default   : ;
    }
    Fill(Stream_Video, 0, Video_FrameRate, (system?25.000:29.970)*FrameRate_Multiplicator);
    Fill(Stream_Video, 0, Video_FrameRate_Mode, "CFR");
    if (video_sourcecontrol_IsParsed)
    {
        if (FSC_WasSet && FSP_WasNotSet)
        {
            switch (video_source_stype)
            {
                case 0x14 :
                case 0x15 :
                            Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
                            if (FieldOrder_FF)
                                Fill(Stream_Video, 0, Video_ScanOrder, FieldOrder_FS?"TFF":"BFF");
                            else
                                Fill(Stream_Video, 0, Video_ScanOrder, FieldOrder_FS?"Top field only":"Bottom field only");
                            Fill(Stream_Video, 0, Video_Interlacement, "Interlaced");
                            break;
                case 0x18 :
                            Fill(Stream_Video, 0, Video_ScanType, "Progressive");
                            Fill(Stream_Video, 0, Video_Interlacement, "Progressive");
                            break;
                default   : ;
            }
        }
        else if (Interlaced)
        {
            Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
            if (FieldOrder_FF)
                Fill(Stream_Video, 0, Video_ScanOrder, FieldOrder_FS?"BFF":"TFF");
            else
                Fill(Stream_Video, 0, Video_ScanOrder, FieldOrder_FS?"Bottom field only":"Top field only");
            Fill(Stream_Video, 0, Video_Interlacement, "Interlaced");
        }
        else
        {
            Fill(Stream_Video, 0, Video_ScanType, Interlaced?"Interlaced":"Progressive");
            Fill(Stream_Video, 0, Video_Interlacement, Interlaced?"Interlaced":"PFF");
        }
        switch (aspect)
        {
            case 0 :
            case 4 : Fill(Stream_Video, 0, Video_DisplayAspectRatio, 4.0/3.0, 3, true); break;
            case 1 :
            case 2 : Fill(Stream_Video, 0, Video_DisplayAspectRatio, 16.0/9.0, 3, true); break;
            case 7 : switch (ssyb_AP3)
                     {
                        case 0 : Fill(Stream_Video, 0, Video_DisplayAspectRatio, 16.0/9.0, 3, true); break;
                        case 7 : Fill(Stream_Video, 0, Video_DisplayAspectRatio, 4.0/3.0, 3, true); break;
                        default: ; //No indication of aspect ratio?
                     }
            default: ;
        }
    }

    if (!FSC_WasSet) //Original DV 25 Mbps
    {
        if (system==false) //NTSC
        {
            switch (video_source_stype)
            {
                case  0 : Fill(Stream_Video, 0, Video_Colorimetry, "4:1:1"); break; //NTSC 25 Mbps
                default : ;
            }
        }
        else //PAL
        {
            switch (video_source_stype)
            {
                case  0 : if (APT==0)
                            Fill(Stream_Video, 0, Video_Colorimetry, "4:2:0");      //PAL 25 Mbps (IEC 61834)
                          else
                            Fill(Stream_Video, 0, Video_Colorimetry, "4:1:1");      //PAL 25 Mbps (SMPTE 314M)
                          break;
                default : ;
            }
        }
    }
    else //DV 50 Mbps and 100 Mbps
        Fill(Stream_Video, 0, Video_Colorimetry, "4:2:2");

    if (FrameSize_Theory && !IsHd)
    {
        float64 OverallBitRate=FrameSize_Theory*(DSF?((float64)25.000):((float64)30000/1001))*8;
        if (FSC_WasSet)
        {
            if (FSP_WasNotSet)
                OverallBitRate*=4; //DV100
            else
                OverallBitRate*=2; //DV50
        }
        if (OverallBitRate)
        {
            if (IsSub)
                Fill(Stream_Video, 0, Video_BitRate_Encoded, OverallBitRate, 0);
            else
                Fill(Stream_General, 0, General_OverallBitRate, OverallBitRate, 0);
            Fill(Stream_Video, 0, (FSC_WasSet && FSP_WasNotSet)?Video_BitRate_Maximum:Video_BitRate, OverallBitRate*134/150*76/80, 0); //134 Video DIF from 150 DIF, 76 bytes from 80 byte DIF
        }
    }

    if (!Config->File_DvDif_DisableAudioIfIsInContainer_Get())
        for (size_t Pos=0; Pos<Streams_Audio.size(); Pos++)
        {
            Stream_Prepare(Stream_Audio);
            for (std::map<std::string, Ztring>::iterator Info=Streams_Audio[Pos]->Infos.begin(); Info!=Streams_Audio[Pos]->Infos.end(); ++Info)
                Fill(Stream_Audio, StreamPos_Last, Info->first.c_str(), Info->second, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_BitRate_Encoded, 0);
        }

    if (Stream_BitRateFromContainer && Retrieve(Stream_Video, 0, Video_BitRate).empty())
    {
        if (Stream_BitRateFromContainer>=28800000*0.98 && Stream_BitRateFromContainer<=28800000*1.02)
        {
            Fill(Stream_Video, 0, Video_BitRate, ((float64)28800000)*134/150*76/80, 0);
            Fill(Stream_Video, 0, Video_BitRate_Encoded, 28800000);
        }
        if (Stream_BitRateFromContainer>=57600000*0.98 && Stream_BitRateFromContainer<=57600000*1.02)
        {
            Fill(Stream_Video, 0, Video_BitRate, ((float64)57600000)*134/150*76/80, 0);
            Fill(Stream_Video, 0, Video_BitRate_Encoded, 57600000);
        }
        if (Stream_BitRateFromContainer>=115200000*0.98 && Stream_BitRateFromContainer<=115200000*1.02)
        {
            Fill(Stream_Video, 0, Video_BitRate, ((float64)115200000)*134/150*76/80, 0);
            Fill(Stream_Video, 0, Video_BitRate_Encoded, 115200000);
        }
    }

    //Library settings
    Fill(Stream_Video, 0, Video_Encoded_Library_Settings, Encoded_Library_Settings);

    //Profile
    if (FSC_WasSet || IsHd)
    {
        if (FSP_WasNotSet || IsHd)
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "DVCPRO HD");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "DVCPRO HD");
            Clear(Stream_Video, 0, Video_BitDepth); //MXF files say that DVCPRO HD streams are 8 or 10 bits, exact?
        }
        else
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "DVCPRO 50");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "DVCPRO 50");
            Fill(Stream_Video, 0, Video_BitRate_Mode, "CBR");
        }
    }
    else if (audio_locked || (Retrieve(Stream_Video, 0, Video_Standard)==__T("PAL") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:1:1")))
    {
        Fill(Stream_General, 0, General_Format_Commercial_IfAny, "DVCPRO");
        Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "DVCPRO");
        Fill(Stream_Video, 0, Video_BitRate_Mode, "CBR");
    }
    else
        Fill(Stream_Video, 0, Video_BitRate_Mode, "CBR");

    //Delay
    if (TimeCode_FirstFrame_ms!=(int64u)-1)
    {
        Fill(Stream_Video, 0, Video_Delay, TimeCode_FirstFrame_ms);
        if (TimeCode_FirstFrame.size()==11)
            Fill(Stream_Video, 0, Video_Delay_DropFrame, TimeCode_FirstFrame[8]==';'?"Yes":"No");
        Fill(Stream_Video, 0, Video_Delay_Source, "Stream");
        Fill(Stream_Video, 0, Video_TimeCode_FirstFrame, TimeCode_FirstFrame.c_str());
        Fill(Stream_Video, 0, Video_TimeCode_Source, "Subcode time code");
        for (size_t Pos=0; Pos<Count_Get(Stream_Audio); Pos++)
        {
            Fill(Stream_Audio, Pos, Audio_Delay, TimeCode_FirstFrame_ms);
            Fill(Stream_Audio, Pos, Audio_Delay_Source, "Stream");
        }
    }

    #if defined(MEDIAINFO_EIA608_YES)
        for (size_t Pos=0; Pos<CC_Parsers.size(); Pos++)
            if (CC_Parsers[Pos] && CC_Parsers[Pos]->Status[IsAccepted])
            {
                Finish(CC_Parsers[Pos]);
                for (size_t Pos2=0; Pos2<CC_Parsers[Pos]->Count_Get(Stream_Text); Pos2++)
                {
                    Stream_Prepare(Stream_Text);
                    Merge(*CC_Parsers[Pos], Stream_Text, Pos2, StreamPos_Last);
                    Fill(Stream_Text, StreamPos_Last, Text_ID, CC_Parsers[Pos]->Retrieve(Stream_Text, Pos2, Text_ID), true);
                }
            }
    #endif //defined(MEDIAINFO_EIA608_YES)
}

//---------------------------------------------------------------------------
void File_DvDif::Streams_Finish()
{
    if (!Recorded_Date_Date.empty())
    {
        Ztring Recorded_Date(Recorded_Date_Date);
        if (Recorded_Date_Time.size()>4)
        {
            Recorded_Date+=__T(" ");
            Recorded_Date+=Recorded_Date_Time;
        }
        if (Count_Get(Stream_General)==0)
            Stream_Prepare(Stream_General);
        Fill(Stream_General, 0, General_Recorded_Date, Recorded_Date, true);
    }
    if (!IsSub && Duration)
        Fill(Stream_General, 0, General_Duration, Duration);

    #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
        if (Config->File_DvDif_Analysis_Get())
        {
            //Errors stats
            Status[IsFinished]=true; //We need to fill it before the call to Errors_Stats_Update
            Errors_Stats_Update();
            Errors_Stats_Update_Finnish();
        }
    #endif //MEDIAINFO_DVDIF_ANALYZE_YES
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_DvDif::FileHeader_Begin()
{
    //Must have enough buffer for having header
    if (Buffer_Size<8)
        return false; //Must wait for more data

    //False positives detection: detect some headers from other files, DV parser is not smart enough
    if (CC4(Buffer)==0x52494646  //RIFF
     || CC4(Buffer+4)==0x66747970  //ftyp
     || CC4(Buffer+4)==0x66726565  //free
     || CC4(Buffer+4)==0x6D646174  //mdat
     || CC4(Buffer+4)==0x6D6F6F76  //moov
     || CC4(Buffer+4)==0x736B6970  //skip
     || CC4(Buffer+4)==0x77696465  //wide
     || CC4(Buffer)==0x060E2B34) //MXF begin
    {
        Finish();
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_DvDif::Synchronize()
{
    if (AuxToAnalyze)
    {
        Accept();
        return true;
    }

    while (Buffer_Offset+8*80<=Buffer_Size //8 blocks
        && !((Buffer[Buffer_Offset+0*80]&0xE0)==0x00 && (Buffer[Buffer_Offset+0*80+1]&0xF0)==0x00 && Buffer[Buffer_Offset+0*80+2]==0x00   //Header 0
          && (Buffer[Buffer_Offset+1*80]&0xE0)==0x20 && (Buffer[Buffer_Offset+1*80+1]&0xF0)==0x00 && Buffer[Buffer_Offset+1*80+2]==0x00   //Subcode 0
          && (Buffer[Buffer_Offset+2*80]&0xE0)==0x20 && (Buffer[Buffer_Offset+2*80+1]&0xF0)==0x00 && Buffer[Buffer_Offset+2*80+2]==0x01   //Subcode 1
          && (Buffer[Buffer_Offset+3*80]&0xE0)==0x40 && (Buffer[Buffer_Offset+3*80+1]&0xF0)==0x00 && Buffer[Buffer_Offset+3*80+2]==0x00   //VAUX 0
          && (Buffer[Buffer_Offset+4*80]&0xE0)==0x40 && (Buffer[Buffer_Offset+4*80+1]&0xF0)==0x00 && Buffer[Buffer_Offset+4*80+2]==0x01   //VAUX 1
          && (Buffer[Buffer_Offset+5*80]&0xE0)==0x40 && (Buffer[Buffer_Offset+5*80+1]&0xF0)==0x00 && Buffer[Buffer_Offset+5*80+2]==0x02   //VAUX 2
          && (Buffer[Buffer_Offset+6*80]&0xE0)==0x60 && (Buffer[Buffer_Offset+6*80+1]&0xF0)==0x00 && Buffer[Buffer_Offset+6*80+2]==0x00   //Audio 0
          && (Buffer[Buffer_Offset+7*80]&0xE0)==0x80 && (Buffer[Buffer_Offset+7*80+1]&0xF0)==0x00 && Buffer[Buffer_Offset+7*80+2]==0x00)) //Video 0
            Buffer_Offset++;

    if (Buffer_Offset+8*80>Buffer_Size)
        return false;

    if (!Status[IsAccepted])
        Accept();

    return true;
}

//---------------------------------------------------------------------------
bool File_DvDif::Synched_Test()
{
    if (AuxToAnalyze)
        return true;

    //Must have enough buffer for having header
    if (Buffer_Offset+80>Buffer_Size)
        return false;

    //NULL blocks
    if (Buffer[Buffer_Offset  ]==0x00
     && Buffer[Buffer_Offset+1]==0x00
     && Buffer[Buffer_Offset+2]==0x00)
        return true;

    SCT =(Buffer[Buffer_Offset  ]&0xE0)>>5;
    DBN = Buffer[Buffer_Offset+2];

    //DIF Sequence Numbers
    if (DSF_IsValid)
    {
        if (Dseq_Old!=Dseq)
        {
            if (Dseq==0
             && !(!DSF && Dseq_Old==9)
             && !( DSF && Dseq_Old==11))
            {
                if (!Status[IsAccepted])
                    Trusted_IsNot("Wrong order");
                else
                    Synched_Test_Reset();
            }
            Dseq_Old=Dseq;
        }
    }

    //DIF Block Numbers
    if (SCT!=(int8u)-1)
    {
        int8u Number=DBN_Olds[SCT]+1;
        switch (SCT)
        {
            case 0 : //Header
                        if (SCT_Old!=4
                         || DBN!=0)
                        {
                            if (!Status[IsAccepted])
                                Trusted_IsNot("Wrong order");
                            else
                                Synched_Test_Reset();
                        }
                        break;
            case 1 : //Subcode
                        if (!((DBN==0 && SCT_Old==0) || (DBN!=0 && SCT_Old==1))
                         || (Number!=DBN && !(Number==2 && DBN==0)))
                        {
                            if (!Status[IsAccepted])
                                Trusted_IsNot("Wrong order");
                            else
                                Synched_Test_Reset();
                        }
                        break;
            case 2 : //VAUX
                        if (!((DBN==0 && SCT_Old==1) || (DBN!=0 && SCT_Old==2))
                         || (Number!=DBN && !(Number==3 && DBN==0)))
                        {
                            if (!Status[IsAccepted])
                                Trusted_IsNot("Wrong order");
                            else
                                Synched_Test_Reset();
                        }
                        break;
            case 3 : //Audio
                        if (!((DBN==0 && SCT_Old==2) || (DBN!=0 && SCT_Old==4))
                         || (Number!=DBN && !(Number==9 && DBN==0)))
                        {
                            if (!Status[IsAccepted])
                                Trusted_IsNot("Wrong order");
                            else
                                Synched_Test_Reset();
                        }
                        break;
            case 4 : //Video
                        if (!(SCT_Old==3 || SCT_Old==4)
                         || (Number!=DBN && !(Number==135 && DBN==0)))
                        {
                            if (!Status[IsAccepted])
                                Trusted_IsNot("Wrong order");
                            else
                                Synched_Test_Reset();
                        }
                        break;
            default: ;
        }

        if (SCT!=(int8u)-1)
        {
            SCT_Old=SCT;
            DBN_Olds[SCT]=DBN;
        }
    }

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_DvDif::Synched_Test_Reset()
{
    //Temp
    SCT=(int8u)-1;
    SCT_Old=4; //Video
    DBN_Olds[0]=0;
    DBN_Olds[1]=1; //SubCode
    DBN_Olds[2]=2; //Vaux
    DBN_Olds[3]=8; //Audio
    DBN_Olds[4]=134; //Video
    DBN_Olds[5]=0;
    DBN_Olds[6]=0;
    DBN_Olds[7]=0;

    //Synch
    Synched=false;
}

//---------------------------------------------------------------------------
void File_DvDif::Synched_Init()
{
    //FrameInfo
    if (FrameInfo.DTS==(int64u)-1)
        FrameInfo.DTS=0; //No DTS in container
    if (FrameInfo.PTS==(int64u)-1)
        FrameInfo.PTS=0; //No PTS in container
    if (Frame_Count_NotParsedIncluded==(int64u)-1)
        Frame_Count_NotParsedIncluded=0; //No Frame_Count_NotParsedIncluded in the container
}

//***************************************************************************
// Buffer - Demux
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
bool File_DvDif::Demux_UnpacketizeContainer_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+8*80>Buffer_Size)
        return false;

    if ((Buffer[Buffer_Offset]&0xE0)==0x00   //Speed up the parsing
     && (CC3(Buffer+Buffer_Offset+0*80)&0xE0FCFF)==0x000400   //Header 0 (with FSC==false and FSP==true)
     && (CC3(Buffer+Buffer_Offset+1*80)&0xE0F0FF)==0x200000   //Subcode 0
     && (CC3(Buffer+Buffer_Offset+2*80)&0xE0F0FF)==0x200001   //Subcode 1
     && (CC3(Buffer+Buffer_Offset+3*80)&0xE0F0FF)==0x400000   //VAUX 0
     && (CC3(Buffer+Buffer_Offset+4*80)&0xE0F0FF)==0x400001   //VAUX 1
     && (CC3(Buffer+Buffer_Offset+5*80)&0xE0F0FF)==0x400002   //VAUX 2
     && (CC3(Buffer+Buffer_Offset+6*80)&0xE0F0FF)==0x600000   //Audio 0
     && (CC3(Buffer+Buffer_Offset+7*80)&0xE0F0FF)==0x800000)  //Video 0
    {
        if (Demux_Offset==0)
        {
            Demux_Offset=Buffer_Offset+1;
        }

        while (Demux_Offset+8*80<=Buffer_Size //8 blocks
            && !((Buffer[Demux_Offset]&0xE0)==0x00   //Speed up the parsing
              && (CC3(Buffer+Demux_Offset+0*80)&0xE0FCFF)==0x000400   //Header 0 (with FSC==false and FSP==true)
              && (CC3(Buffer+Demux_Offset+1*80)&0xE0F0FF)==0x200000   //Subcode 0
              && (CC3(Buffer+Demux_Offset+2*80)&0xE0F0FF)==0x200001   //Subcode 1
              && (CC3(Buffer+Demux_Offset+3*80)&0xE0F0FF)==0x400000   //VAUX 0
              && (CC3(Buffer+Demux_Offset+4*80)&0xE0F0FF)==0x400001   //VAUX 1
              && (CC3(Buffer+Demux_Offset+5*80)&0xE0F0FF)==0x400002   //VAUX 2
              && (CC3(Buffer+Demux_Offset+6*80)&0xE0F0FF)==0x600000   //Audio 0
              && (CC3(Buffer+Demux_Offset+7*80)&0xE0F0FF)==0x800000)) //Video 0
                Demux_Offset++;

        if (Demux_Offset+8*80>Buffer_Size && File_Offset+Buffer_Size!=File_Size)
            return false; //No complete frame
        if (Demux_Offset+8*80>Buffer_Size && File_Offset+Buffer_Size==File_Size)
            Demux_Offset=(size_t)(File_Size-File_Offset); //Using the complete buffer (no next sync)

        Demux_UnpacketizeContainer_Demux();
    }

    return true;
}
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_DvDif::Read_Buffer_Unsynched()
{
    Synched_Test_Reset();
        if (!IsSub && File_GoTo!=(int64u)-1 && (Frame_Count
    #if MEDIAINFO_SEEK
          || Duration_Detected
    #endif //MEDIAINFO_SEEK
          ) && !FSP_WasNotSet)
    {
        int64u BytesPerFrame=12000*(DSF?12:10);
        if (FSC_WasSet)
            BytesPerFrame*=2;
        Frame_Count_NotParsedIncluded=File_GoTo/BytesPerFrame;
        FrameInfo.PTS=FrameInfo.DTS=float64_int64s(Frame_Count_NotParsedIncluded/(DSF?25.000:(30.000*1000/1001))*1000000000);
    }
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_DvDif::Read_Buffer_Seek (size_t Method, int64u Value, int64u /*ID*/)
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
        if (!MiOpenResult || MI.Get(Stream_General, 0, General_Format)!=__T("DV"))
            return 0;

        TotalFrames=Ztring(MI.Get(Stream_Video, 0, Video_FrameCount)).To_int64u();
        int64u VideoBitRate=Ztring(MI.Get(Stream_Video, 0, Video_BitRate)).To_int64u();
        if (VideoBitRate==0 || VideoBitRate>=50000000)
        {
            FSC_WasSet=true;
            FSP_WasNotSet=true;
        }
        else if (VideoBitRate>=30000000)
            FSC_WasSet=true;
        float32 FrameRate=Ztring(MI.Get(Stream_Video, 0, Video_FrameRate)).To_float32();
        if (FrameRate>=24.0 && FrameRate<26.0)
            DSF=system=true;
        if (FrameRate>=29.0 && FrameRate<31.0)
            DSF=system=false;
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
                        //We transform TimeStamp to a frame number
                        Value=float64_int64s(((float64)Value)*(DSF?25.000:(30.000*1000/1001))/1000000000);
                    }
                    //No break;
        case 3  :   //FrameNumber
                    if (!FSP_WasNotSet)
                    {
                        int64u BytesPerFrame=12000*(DSF?12:10);
                        if (FSC_WasSet)
                            BytesPerFrame*=2;
                        GoTo(BytesPerFrame*Value);
                        Open_Buffer_Unsynch();
                        Frame_Count_NotParsedIncluded=Value;
                        FrameInfo.PTS=FrameInfo.DTS=float64_int64s(Frame_Count_NotParsedIncluded/(DSF?25.000:(30.000*1000/1001))*1000000000);
                        return 1;
                    }
                    else
                        return (size_t)-1; //Not supported
        default :   return (size_t)-1; //Not supported
    }
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_DvDif::Header_Parse()
#if MEDIAINFO_TRACE
{
    if (AuxToAnalyze!=0x00)
    {
        SCT=(int8u)-1;
        Header_Fill_Code(AuxToAnalyze, Ztring::ToZtring(AuxToAnalyze, 16));
        Header_Fill_Size(4);
        return;
    }

    //Unsynch problems
    if (Element_Size<80)
    {
        Element_WaitForMoreData();
        return;
    }
    if (Buffer[Buffer_Offset  ]==0x00
     && Buffer[Buffer_Offset+1]==0x00
     && Buffer[Buffer_Offset+2]==0x00)
    {
        SCT=(int8u)-1;
        Header_Fill_Code((int64u)-1);
        Header_Fill_Size(80);
        return;
    }

    //Parsing
    BS_Begin();
    //0
    Get_S1 (3, SCT,                                             "SCT - Section Type"); Param_Info1(Dv_sct[SCT]);
    Skip_SB(                                                    "Res - Reserved");
    Skip_S1(4,                                                  "Arb - Arbitrary bits");
    //1
    Get_S1 (4, Dseq,                                            "Dseq - DIF sequence number"); //0-9 for 525/60; 0-11 for 625/50
    Get_SB (   FSC,                                             "FSC - Channel number");
    Get_SB (   FSP,                                             "FSP - Channel number"); //SMPTE 370M only
    Skip_S1(2,                                                  "Res - Reserved");
    BS_End();
    //2
    Get_B1 (DBN,                                                "DBN - DIF block number"); //Video: 0-134, Audio: 0-8

    Header_Fill_Code(SCT, Dv_sct[SCT]);
    Header_Fill_Size(80);
}
#else //MEDIAINFO_TRACE
{
    if (AuxToAnalyze!=0x00)
    {
        SCT=(int8u)-1;
        Header_Fill_Code(AuxToAnalyze);
        Header_Fill_Size(4);
        return;
    }

    //Unsynch problems
    if (Element_Size<80)
    {
        Element_WaitForMoreData();
        return;
    }
    if (Buffer[Buffer_Offset  ]==0x00
     && Buffer[Buffer_Offset+1]==0x00
     && Buffer[Buffer_Offset+2]==0x00)
    {
        SCT=(int8u)-1;
        Header_Fill_Code((int64u)-1);
        Header_Fill_Size(80);
        return;
    }

    //Parsing
    SCT =(Buffer[Buffer_Offset  ]&0xE0)>>5;
    Dseq=(Buffer[Buffer_Offset+1]&0xF0)>>4;
    FSC =(Buffer[Buffer_Offset+1]&0x08)==0x08;
    FSP =(Buffer[Buffer_Offset+1]&0x04)==0x04;
    DBN = Buffer[Buffer_Offset+2];
    Element_Offset+=3;

    Header_Fill_Code(SCT);
    Header_Fill_Size(80);
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
void File_DvDif::Data_Parse()
{
    if (Element_Code==(int64u)-1)
    {
        Skip_XX(Element_Size,                                   "Junk");
        return;
    }

    //Config
    if (SCT!=(int8u)-1)
    {
        if (!FSC_WasSet && FSC)
            FSC_WasSet=true;

        if (!FSP_WasNotSet && !FSP)
            FSP_WasNotSet=true;
    }

    if (AuxToAnalyze!=0x00)
    {
        Element();
        return;
    }

    Element_Info1(DBN);

    switch (SCT)
    {
        case 0 : Header(); break;
        case 1 : Subcode(); break;
        case 2 : VAUX(); break;
        case 3 : Audio(); break;
        case 4 : Video(); break;
        default: Skip_XX(Element_Size,                          "Unknown");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_DvDif::Header()
#if MEDIAINFO_TRACE
{
    BS_Begin();
    //3
    Get_SB (   DSF,                                             "DSF - DIF Sequence Flag"); //0=NTSC, 1=PAL
    Skip_SB(                                                    "Zero");
    Skip_S1(6,                                                  "Reserved");

    //4
    Skip_S1(5,                                                  "Reserved");
    Get_S1 (3, APT,                                             "APT"); //Track application ID, 0=4:2:0, 1=not 4:2:0

    //5
    Get_SB (   TF1,                                             "TF1 - Audio data is not valid");
    Skip_S1(4,                                                  "Reserved");
    Skip_S1(3,                                                  "AP1 - Audio application ID");

    //6
    Get_SB (  TF2,                                              "TF2 - Video data is not valid");
    Skip_S1(4,                                                  "Reserved");
    Skip_S1(3,                                                  "AP2 - Video application ID");

    //7
    Get_SB (  TF3,                                              "TF3 - Subcode is not valid");
    Skip_S1(4,                                                  "Reserved");
    Skip_S1(3,                                                  "AP3 - Subcode application ID");

    //8-79
    BS_End();
    Skip_XX(72,                                                 "Reserved"); //Should be filled with 0xFF

    if (Config->File_DvDif_IgnoreTransmittingFlags_Get())
    {
        TF1=false;
        TF2=false;
        TF3=false;
    }

    FILLING_BEGIN();
        DSF_IsValid=true;
        Dseq_Old=DSF?11:9;
        FrameSize_Theory=(DSF?12:10)*150*80; //12 DIF sequences for PAL, 10 for NTSC

        if (TF1 && TF2)
        {
            //This is not logic, the header says no audio and no video! We do not trust the header, resetting all
            TF1=false;
            TF2=false;
            TF3=false;
        }
    FILLING_END();
}
#else //MEDIAINFO_TRACE
{
    if (Element_Size<77)
    {
        Trusted_IsNot("Size is wrong");
        return;
    }

    DSF=(Buffer[Buffer_Offset  ]&0x80)?true:false;
    APT=(Buffer[Buffer_Offset+1]&0x07);
    TF1=(Buffer[Buffer_Offset+2]&0x80)?true:false;
    TF2=(Buffer[Buffer_Offset+3]&0x80)?true:false;
    TF3=(Buffer[Buffer_Offset+4]&0x80)?true:false;

    if (Config->File_DvDif_IgnoreTransmittingFlags_Get())
    {
        TF1=false;
        TF2=false;
        TF3=false;
    }

    FILLING_BEGIN();
        DSF_IsValid=true;
        Dseq_Old=DSF?11:9;
        FrameSize_Theory=(DSF?12:10)*150*80; //12 DIF sequences for PAL, 10 for NTSC

        if (TF1 && TF2)
        {
            //This is not logic, the header says no audio and no video! We do not trust the header, resetting all
            TF1=false;
            TF2=false;
            TF3=false;
        }
    FILLING_END();
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
void File_DvDif::Subcode()
#if MEDIAINFO_TRACE
{
    //Present?
    if (TF3)
    {
        Skip_XX(Element_Size,                                   "Unused");
        return;
    }

    //Parsing
    for (int8u syb_num=0; syb_num<6; syb_num++)
        Subcode_Ssyb(syb_num);
    Skip_XX(29,                                                 "Unused");
}
#else //MEDIAINFO_TRACE
{
    if (TF3)
        return;

    for (int8u syb_num=0; syb_num<6; syb_num++)
        Subcode_Ssyb(syb_num);
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
void File_DvDif::Subcode_Ssyb(int8u syb_num)
{
    Element_Begin1("ssyb");

    //Parsing
    BS_Begin();
    //ID0-ID1
    Skip_SB(                                                    "FR - Identification of half of channel"); //1=first half, 0=second
    if (syb_num==0)
    {
        if (FSC==false)
            Get_S1 ( 3, ssyb_AP3,                               "AP3 - Subcode application ID");
        else
            Skip_S1( 3,                                         "AP3 - Subcode application ID");
    }
    else if (DBN==1 && syb_num==5)
        Skip_S1(3,                                              "APT - track application ID");
    else
        Skip_S1(3,                                              "Res - Reserved");
    Skip_S1(8,                                                  "Arb - Arbitrary bits");
    Skip_S1(4,                                                  "Syb - SSYSB number");
    BS_End();
    //FFh
    Skip_B1(                                                    "0xFF");
    //PC0-PC4
    Element();

    Element_End0();
}

//---------------------------------------------------------------------------
void File_DvDif::VAUX()
{
    //Present?
    if (TF2)
    {
        Skip_XX(Element_Size,                                   "Unused");
        return;
    }

    //Parsing
    for (int8u i=0; i<15; i++)
        Element();
    Skip_XX(2,                                                  "Unused");
}

//---------------------------------------------------------------------------
void File_DvDif::Audio()
{
    //Present?
    if (TF1)
    {
        Skip_XX(Element_Size,                                   "Unused");
        return;
    }

    Element_Name("Audio");

    Element(); //First 5 bytes
    Skip_XX(Element_Size-Element_Offset,                        "Unknown");
}

//---------------------------------------------------------------------------
void File_DvDif::Video()
{
    #if MEDIAINFO_TRACE
    //Present?
    if (TF2)
    {
        Skip_XX(Element_Size,                                   "Unused");
        return;
    }

    Element_Name("Video");

    //Parsing
    BS_Begin();
    Skip_S1(4,                                                  "STA");
    Skip_S1(4,                                                  "QNO");
    BS_End();
    Skip_XX(Element_Size-Element_Offset,                        "Unknown");
    #endif //MEDIAINFO_TRACE

    FILLING_BEGIN();
        if (DBN==134 && video_source_stype!=(int8u)-1)
        {
            if (!Status[IsAccepted])
            {
                Accept("DV DIF");

                if (!IsSub)
                    Fill(Stream_General, 0, General_Format, "DV");
            }
            if (!Status[IsFilled] && Frame_Count>=Frame_Count_Valid)
                #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
                {
                    if (Config->File_DvDif_Analysis_Get())
                        Fill("DV DIF");
                    else
                        Finish("DV DIF");
                }
                #else //MEDIAINFO_DVDIF_ANALYZE_YES
                    Finish("DV DIF");
                #endif //MEDIAINFO_DVDIF_ANALYZE_YES
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_DvDif::Element()
{
    Element_Begin0();
    int8u PackType;
    if (AuxToAnalyze==0x00)
        Get_B1 (PackType,                                       "Pack Type");
    else
        PackType=AuxToAnalyze; //Forced by parser

    switch(PackType)
    {
        case 0x13 : timecode(); break;
        case 0x14 : binary_group(); break;
        case 0x50 : audio_source(); break;
        case 0x51 : audio_sourcecontrol(); break;
        case 0x52 : audio_recdate(); break;
        case 0x53 : audio_rectime(); break;
        case 0x60 : video_source(); break;
        case 0x61 : video_sourcecontrol(); break;
        case 0x62 : video_recdate(); break;
        case 0x63 : video_rectime(); break;
        case 0x65 : closed_captions(); break;
        case 0x70 : consumer_camera_1(); break;
        case 0x71 : consumer_camera_2(); break;
        case 0xFF : Element_Name(Ztring().From_Number(PackType, 16));
                    Skip_B4(                                    "Unused"); break;
        default   : Element_Name(Ztring().From_Number(PackType, 16));
                    Skip_B4(                                    "Unknown");
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_DvDif::binary_group()
{
    Element_Name("binary_group");

    if (Buffer[Buffer_Offset+(size_t)Element_Offset  ]==0xFF
     && Buffer[Buffer_Offset+(size_t)Element_Offset+1]==0xFF
     && Buffer[Buffer_Offset+(size_t)Element_Offset+2]==0xFF
     && Buffer[Buffer_Offset+(size_t)Element_Offset+3]==0xFF
    )
    {
        Skip_XX(4,                                              "All one");
        return;
    }

    //Parsing
    BS_Begin();
    Skip_S1(4,                                                  "Binary group 2");
    Skip_S1(4,                                                  "Binary group 1");
    Skip_S1(4,                                                  "Binary group 4");
    Skip_S1(4,                                                  "Binary group 3");
    Skip_S1(4,                                                  "Binary group 6");
    Skip_S1(4,                                                  "Binary group 5");
    Skip_S1(4,                                                  "Binary group 8");
    Skip_S1(4,                                                  "Binary group 7");
    BS_End();
}


//---------------------------------------------------------------------------
void File_DvDif::timecode()
{
    Element_Name("timecode");

    if (Buffer[Buffer_Offset+(size_t)Element_Offset  ]==0x00
     && Buffer[Buffer_Offset+(size_t)Element_Offset+1]==0x00
     && Buffer[Buffer_Offset+(size_t)Element_Offset+2]==0x00
     && Buffer[Buffer_Offset+(size_t)Element_Offset+3]==0x00
    )
    {
        Skip_XX(4,                                              "All zero");
        return;
    }

    //Parsing
    int8u Frames_Units, Frames_Tens, Seconds_Units, Seconds_Tens, Minutes_Units, Minutes_Tens, Hours_Units, Hours_Tens;
    int64u MilliSeconds=0;
    int8u  Frames=0;
    bool   DropFrame=false;
    BS_Begin();
    Skip_SB(                                                    "CF - Color fame");
    if (!DSF_IsValid)
        Skip_SB(                                                "Arbitrary bit or DP");
    else if (DSF)    //625/50
        Skip_SB(                                                "Arbitrary bit");
    else        //525/60
        Get_SB (DropFrame,                                      "DP - Drop frame"); //525/60
    Get_S1 (2, Frames_Tens,                                     "Frames (Tens)");
    Frames+=Frames_Tens*10;
    Get_S1 (4, Frames_Units,                                    "Frames (Units)");
    Frames+=Frames_Units;

    if (!DSF_IsValid)
        Skip_SB(                                                "BGF0 or PC");
    else if (DSF)    //625/50
        Skip_SB(                                                "BGF0 - Binary group flag");
    else        //525/60
        Skip_SB(                                                "PC - Biphase mark polarity correction"); //0=even; 1=odd
    Get_S1 (3, Seconds_Tens,                                    "Seconds (Tens)");
    MilliSeconds+=Seconds_Tens*10*1000;
    Get_S1 (4, Seconds_Units,                                   "Seconds (Units)");
    MilliSeconds+=Seconds_Units*1000;

    if (!DSF_IsValid)
        Skip_SB(                                                "BGF2 or BGF0");
    else if (DSF)    //625/50
        Skip_SB(                                                "BGF2 - Binary group flag");
    else        //525/60
        Skip_SB(                                                "BGF0 - Binary group flag");
    Get_S1 (3, Minutes_Tens,                                    "Minutes (Tens)");
    MilliSeconds+=Minutes_Tens*10*60*1000;
    Get_S1 (4, Minutes_Units,                                   "Minutes (Units)");
    MilliSeconds+=Minutes_Units*60*1000;

    if (!DSF_IsValid)
        Skip_SB(                                                "PC or BGF1");
    else if (DSF)    //625/50
        Skip_SB(                                                "PC - Biphase mark polarity correction"); //0=even; 1=odd
    else        //525/60
        Skip_SB(                                                "BGF1 - Binary group flag");
    Skip_SB(                                                    "BGF2 - Binary group flag");
    Get_S1 (2, Hours_Tens,                                      "Hours (Tens)");
    MilliSeconds+=Hours_Tens*10*60*60*1000;
    Get_S1 (4, Hours_Units,                                     "Hours (Units)");
    MilliSeconds+=Hours_Units*60*60*1000;
    Element_Info1(Ztring().Duration_From_Milliseconds(MilliSeconds+((DSF_IsValid && Frames!=45)?((int64u)(Frames/(DSF?25.000:29.970)*1000)):0)));
    BS_End();

    if (TimeCode_FirstFrame_ms==(int64u)-1 && MilliSeconds!=167185000) //if all bits are set to 1, this is not a valid timestamp
    {
        TimeCode_FirstFrame_ms=MilliSeconds;
        if (DSF_IsValid && Frames!=45) //all bits are set to 1

        TimeCode_FirstFrame_ms+=(int64u)(Frames/(DSF?25.000:29.970)*1000);

        TimeCode_FirstFrame+=('0'+Hours_Tens);
        TimeCode_FirstFrame+=('0'+Hours_Units);
        TimeCode_FirstFrame+=':';
        TimeCode_FirstFrame+=('0'+Minutes_Tens);
        TimeCode_FirstFrame+=('0'+Minutes_Units);
        TimeCode_FirstFrame+=':';
        TimeCode_FirstFrame+=('0'+Seconds_Tens);
        TimeCode_FirstFrame+=('0'+Seconds_Units);
        TimeCode_FirstFrame+=DropFrame?';':':';
        TimeCode_FirstFrame+=('0'+Frames_Tens);
        TimeCode_FirstFrame+=('0'+Frames_Units);
    }
}

//---------------------------------------------------------------------------
void File_DvDif::audio_source()
{
    if (TF1)
    {
        Skip_XX(4,                                              "Unused");
        return;
    }

    Element_Name("audio_source");

    int8u SamplingRate, Resolution;
    BS_Begin();
    //PC1
    Get_SB (   audio_locked,                                    "LF - Locked mode");
    Skip_SB(                                                    "Reserved");
    Skip_S1(6,                                                  "AF - Samples in this frame");

    //PC2
    Info_S1(1, StereoMode,                                      "SM - Stereo mode"); Param_Info1(Dv_StereoMode[StereoMode]);
    Info_S1(2, ChannelsPerBlock,                                "CHN - Channels per block"); Param_Info1(Dv_ChannelsPerBlock[ChannelsPerBlock]);
    Info_S1(1, Pair,                                            "PA - Pair"); Param_Info1(Dv_Pair[Pair]);
    Skip_S1(4,                                                  "AM - Audio mode");

    Skip_SB(                                                    "Reserved");
    Skip_SB(                                                    "ML - Multi-language");
    Skip_SB(                                                    "50/60");
    Get_S1 (5, audio_source_stype,                              "STYPE - audio blocks per video frame"); Param_Info1(audio_source_stype==0?"2 channels":(audio_source_stype==2?"4 channels":"Unknown")); //0=25 Mbps, 2=50 Mbps

    Skip_SB(                                                    "EF - Emphasis off");
    Skip_SB(                                                    "TC - Time constant of emphasis");
    Get_S1 (3, SamplingRate,                                    "SMP - Sampling rate"); Param_Info1(Dv_Audio_SamplingRate[SamplingRate]);
    Get_S1 (3, Resolution,                                      "QU - Resolution"); Param_Info1(Dv_Audio_BitDepth[Resolution]);
    BS_End();

    FILLING_BEGIN();
        if (!IgnoreAudio && Streams_Audio.empty() && Dv_Audio_SamplingRate[SamplingRate] && Dv_Audio_BitDepth[Resolution])
        {
            //Calculating the count of audio
            size_t Audio_Count=1;
            if (audio_source_stype==2 || (Resolution==1 && SamplingRate==2)) //stype=2 or (Resolution=12 bits and SamplingRate=32 KHz)
                Audio_Count=2;
            if (audio_source_stype==3)
                Audio_Count=4;

            //Filling
            if (Streams_Audio.size()<Audio_Count)
                Streams_Audio.resize(Audio_Count);
            for (size_t Pos=0; Pos<Audio_Count; Pos++)
            {
                if (Streams_Audio[Pos]==NULL)
                    Streams_Audio[Pos]=new stream;
                Streams_Audio[Pos]->Infos["ID"].From_Number(Pos);
                Streams_Audio[Pos]->Infos["Format"]=__T("PCM");
                Streams_Audio[Pos]->Infos["Codec"]=__T("PCM");
                Streams_Audio[Pos]->Infos["BitRate_Mode"]=__T("CBR");
                Streams_Audio[Pos]->Infos["Channel(s)"].From_Number(audio_source_stype==3?1:2);
                Streams_Audio[Pos]->Infos["SamplingRate"].From_Number(Dv_Audio_SamplingRate[SamplingRate]);
                Streams_Audio[Pos]->Infos["BitDepth"].From_Number(Dv_Audio_BitDepth[Resolution]);
                Streams_Audio[Pos]->Infos["BitRate"].From_Number((audio_source_stype==3?1:2)*Dv_Audio_SamplingRate[SamplingRate]*Dv_Audio_BitDepth[Resolution]);
            }
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_DvDif::audio_sourcecontrol()
{
    if (TF1)
    {
        Skip_XX(4,                                              "Unused");
        return;
    }

    Element_Name("audio_control");

    BS_Begin();

    //PC1
    Info_S1(2, CopyGenerationManagementSystem,                  "CGMS - Copy generation management system"); Param_Info1(Dv_CopyGenerationManagementSystem[CopyGenerationManagementSystem]);
    Info_S1(2, InputType,                                       "ISR - Input type"); Param_Info1(Dv_InputType[InputType]);
    Info_S1(2, CompressionTimes,                                "CMP - Compression times"); Param_Info1(Dv_CompressionTimes[CompressionTimes]);
    Info_S1(2, Emphasis,                                        "EFC - Emphasis"); Param_Info1(Dv_Emphasis[Emphasis]);

    //PC2
    Skip_SB(                                                    "REC S Non-recording start point");
    Skip_SB(                                                    "REC E - Non-recording end point");
    Skip_SB(                                                    "FADE S - Recording mode"); //1=Original
    Skip_SB(                                                    "FADE E - Unknown");
    Skip_SB(                                                    "Reserved");
    Skip_SB(                                                    "Reserved");
    Skip_SB(                                                    "Reserved");
    Skip_SB(                                                    "Reserved");

    //PC3
    Skip_SB(                                                    "DRF - Direction"); //1=Forward
    Skip_S1(7,                                                  "SPD - Speed");

    //PC4
    Skip_SB(                                                    "Reserved");
    Skip_S1(7,                                                  "GEN - Category");

    BS_End();
}

//---------------------------------------------------------------------------
void File_DvDif::audio_recdate()
{
    if (TF1)
    {
        Skip_XX(4,                                              "Unused");
        return;
    }

    Element_Name("audio_recdate");

    recdate();
}

//---------------------------------------------------------------------------
void File_DvDif::audio_rectime()
{
    if (TF1)
    {
        Skip_XX(4,                                              "Unused");
        return;
    }

    Element_Name("audio_rectime");

    rectime();
}

//---------------------------------------------------------------------------
void File_DvDif::video_source()
{
    if (TF2)
    {
        Skip_XX(4,                                              "Unused");
        return;
    }

    Element_Name("video_source");

    BS_Begin();
    //PC1
    Skip_S1(4,                                                  "TVCH (tens of units, 0–9)");
    Skip_S1(4,                                                  "TVCH (units, 0–9)");

    //PC2
    Skip_SB(                                                    "B/W - Black and White"); //0=Black and White, 1=Color
    Skip_SB(                                                    "EN - Color Frames is not valid");
    Skip_S1(2,                                                  "CLF - Color frames id");
    Skip_S1(4,                                                  "TVCH (hundreds of units, 0–9)");

    //PC3
    Skip_S1(2,                                                  "SRC");
    Get_SB (   system,                                          "50/60 - System");
    Get_S1 (5, video_source_stype,                              "STYPE - Signal type of video signal"); //0=not 4:2:2, 4=4:2:2

    //PC4
    BS_End();
    Skip_B1(                                                    "TUN/VISC");

    FILLING_BEGIN();
        if (FSC==false && FSP==true && Dseq==0)
        {
            Frame_Count++;
            if (Frame_Count_NotParsedIncluded!=(int64u)-1)
                Frame_Count_NotParsedIncluded++;
            FrameInfo.DUR=float64_int64s(((float64)1000000000)/(DSF?25.000:29.970));
            if (FrameInfo.DTS!=(int64u)-1)
                FrameInfo.DTS+=FrameInfo.DUR;
            if (FrameInfo.PTS!=(int64u)-1)
                FrameInfo.PTS+=FrameInfo.DUR;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_DvDif::video_sourcecontrol()
{
    if (TF2)
    {
        Skip_XX(4,                                              "Unused");
        return;
    }

    Element_Name("video_control");

    BS_Begin();
    //PC1
    Info_S1(2, CopyGenerationManagementSystem,                  "CGMS - Copy generation management system"); Param_Info1(Dv_CopyGenerationManagementSystem[CopyGenerationManagementSystem]);
    Skip_S1(2,                                                  "ISR");
    Skip_S1(2,                                                  "CMP");
    Skip_S2(2,                                                  "SS");

    //PC2
    Skip_SB(                                                    "REC S");
    Skip_SB(                                                    "Reserved");
    Skip_S1(2,                                                  "REC M");
    Skip_SB(                                                    "Reserved");
    Get_S1 (3, aspect,                                          "DISP - Aspect ratio"); Param_Info1(Dv_Disp[aspect]);

    //PC3
    Get_SB (   FieldOrder_FF,                                   "FF - Frame/Field"); //1=Frame, 0=Field
    Get_SB (   FieldOrder_FS,                                   "FS - First/second field"); //0=Field 2, 1=Field 1, if FF=0 x is output twice, if FF=1, Field x fisrst, other second
    Skip_SB(                                                    "FC - Frame Change"); //0=Same picture as before
    Get_SB (   Interlaced,                                      "IL - Interlaced"); //1=Interlaced
    Skip_SB(                                                    "SF");
    Skip_SB(                                                    "SC");
    Skip_S1(2,                                                  "BCS");

    //PC4
    Skip_SB(                                                    "Reserved");
    Skip_S1(7,                                                  "GEN - Category");

    BS_End();

    FILLING_BEGIN();
        video_sourcecontrol_IsParsed=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_DvDif::video_recdate()
{
    if (TF2)
    {
        Skip_XX(4,                                              "Unused");
        return;
    }

    Element_Name("video_recdate");

    Ztring Date=recdate();
    if (Recorded_Date_Date.empty())
        Recorded_Date_Date=Date;
}

//---------------------------------------------------------------------------
void File_DvDif::video_rectime()
{
    if (TF2)
    {
        Skip_XX(4,                                              "Unused");
        return;
    }

    Element_Name("video_rectime");

    Ztring Date=rectime();
    if (Recorded_Date_Time.empty())
        Recorded_Date_Time=Date;
}

//---------------------------------------------------------------------------
void File_DvDif::closed_captions()
{
    Element_Name("closed_captions");

    #if defined(MEDIAINFO_EIA608_YES)
        if (CC_Parsers.empty())
        {
            CC_Parsers.resize(2);
            for (int8u Pos=0; Pos<2; Pos++)
            {
                CC_Parsers[Pos]=new File_Eia608();
                ((File_Eia608*)CC_Parsers[Pos])->cc_type=Pos;
            }
            Frame_Count_Valid*=10; //More frames
        }
        if (Dseq==0) //CC are duplicated for each DIF sequence!
        {
            for (size_t Pos=0; Pos<2; Pos++)
            {
                Open_Buffer_Init(CC_Parsers[Pos]);
                Open_Buffer_Continue(CC_Parsers[Pos], 2);
            }
        }

    #else
        Skip_XX(4,                                              "Captions");
    #endif
}

//---------------------------------------------------------------------------
void File_DvDif::consumer_camera_1()
{
    Element_Name("consumer_camera_1");

    //Parsing
    BS_Begin();
    int8u ae_mode, wb_mode, white_balance, fcm;
    Mark_1_NoTrustError();
    Mark_1_NoTrustError();
    Skip_S1(6,                                                  "iris");
    Get_S1 (4, ae_mode,                                         "ae mode"); Param_Info1(Dv_consumer_camera_1_ae_mode[ae_mode]);
    Skip_S1(4,                                                  "agc(Automatic Gain Control)");
    Get_S1 (3, wb_mode,                                         "wb mode (white balance mode)"); Param_Info1(Dv_consumer_camera_1_wb_mode[wb_mode]);
    Get_S1 (5, white_balance,                                   "white balance"); Param_Info1(Dv_consumer_camera_1_white_balance(white_balance));
    Get_S1 (1, fcm,                                             "fcm (Focus mode)"); Param_Info1(Dv_consumer_camera_1_fcm[fcm]);
    Skip_S1(7,                                                  "focus (focal point)");
    BS_End();

    if (Encoded_Library_Settings.empty())
    {
        if (ae_mode<0x0F) Encoded_Library_Settings+=__T("ae mode=")+Ztring(Dv_consumer_camera_1_ae_mode[ae_mode])+__T(" / ");
        if (wb_mode<0x08) Encoded_Library_Settings+=__T("wb mode=")+Ztring(Dv_consumer_camera_1_wb_mode[wb_mode])+__T(" / ");
        if (wb_mode<0x1F) Encoded_Library_Settings+=__T("white balance=")+Ztring(Dv_consumer_camera_1_white_balance(white_balance))+__T(" / ");
                          Encoded_Library_Settings+=__T("fcm=")+Ztring(Dv_consumer_camera_1_fcm[fcm]);
    }
}

//---------------------------------------------------------------------------
void File_DvDif::consumer_camera_2()
{
    Element_Name("consumer_camera_2");

    //Parsing
    BS_Begin();
    Mark_1_NoTrustError();
    Mark_1_NoTrustError();
    Skip_S1(1,                                                  "vpd");
    Skip_S1(5,                                                  "vertical panning speed");
    Skip_S1(1,                                                  "is");
    Skip_S1(1,                                                  "hpd");
    Skip_S1(6,                                                  "horizontal panning speed");
    Skip_S1(8,                                                  "focal length");
    Skip_S1(1,                                                  "zen");
    Info_S1(3, zoom_U,                                          "units of e-zoom");
    Info_S1(4, zoom_D,                                          "1/10 of e-zoom"); /*if (zoom_D!=0xF)*/ Param_Info1(__T("zoom=")+Ztring().From_Number(zoom_U+((float32)zoom_U)/10, 2));
    BS_End();
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
Ztring File_DvDif::recdate()
{
    BS_Begin();

    int8u Temp;
    int16u Year=0;
    int8u  Month=0, Day=0;
    Skip_S1(8,                                                  "Time zone specific"); //ds, tm, tens of time zone, units of time zone, 0xFF for Unknwon
    Skip_SB(                                                    "1");
    Skip_SB(                                                    "1");
    Get_S1 (2, Temp,                                            "Days (Tens)");
    Day+=Temp*10;
    Get_S1 (4, Temp,                                            "Days (Units)");
    Day+=Temp;
    Skip_SB(                                                    "1");
    Skip_SB(                                                    "1");
    Skip_SB(                                                    "1");
    Get_S1 (1, Temp,                                            "Month (Tens)");
    Month+=Temp*10;
    Get_S1 (4, Temp,                                            "Month (Units)");
    Month+=Temp;
    Get_S1 (4, Temp,                                            "Year (Tens)");
    Year+=Temp*10;
    Get_S1 (4, Temp,                                            "Year (Units)");
    Year+=Temp;
    Year+=Year<25?2000:1900;
    Element_Info1(Ztring::ToZtring(Year)+__T("-")+Ztring::ToZtring(Month)+__T("-")+Ztring::ToZtring(Day));

    BS_End();

    if (Month>12 || Day>31)
        return Ztring(); //If all bits are set to 1, this is invalid
    Ztring MonthString;
    if (Month<10)
        MonthString=__T("0");
    MonthString+=Ztring::ToZtring(Month);
    Ztring DayString;
    if (Day<10)
        DayString=__T("0");
    DayString+=Ztring::ToZtring(Day);
    return Ztring::ToZtring(Year)+__T("-")+MonthString+__T("-")+DayString;
}

//---------------------------------------------------------------------------
Ztring File_DvDif::rectime()
{
    if (!DSF_IsValid)
    {
        Trusted_IsNot("Not in right order");
        return Ztring();
    }

    BS_Begin();

    if (Buffer[Buffer_Offset+(size_t)Element_Offset  ]==0x00
     && Buffer[Buffer_Offset+(size_t)Element_Offset+1]==0x00
     && Buffer[Buffer_Offset+(size_t)Element_Offset+2]==0x00
     && Buffer[Buffer_Offset+(size_t)Element_Offset+3]==0x00
    )
    {
        Skip_XX(4,                                              "All zero");
        return Ztring();
    }

    int8u Temp;
    int64u Time=0;
    int8u Frames=0;
    Skip_SB(                                                    "Unknown");
    Skip_SB(                                                    "1");
    Get_S1 (2, Temp,                                            "Frames (Tens)");
    Frames+=Temp*10;
    Get_S1 (4, Temp,                                            "Frames (Units)");
    Frames+=Temp;
    if (Temp!=0xF && DSF_IsValid)
        Time+=(int64u)(Frames/(DSF?25.000:29.970));
    Skip_SB(                                                    "1");
    Get_S1 (3, Temp,                                            "Seconds (Tens)");
    Time+=Temp*10*1000;
    Get_S1 (4, Temp,                                            "Seconds (Units)");
    Time+=Temp*1000;
    Skip_SB(                                                    "1");
    Get_S1 (3, Temp,                                            "Minutes (Tens)");
    Time+=Temp*10*60*1000;
    Get_S1 (4, Temp,                                            "Minutes (Units)");
    Time+=Temp*60*1000;
    Skip_SB(                                                    "1");
    Skip_SB(                                                    "1");
    Get_S1 (2, Temp,                                            "Hours (Tens)");
    Time+=Temp*10*60*60*1000;
    Get_S1 (4, Temp,                                            "Hours (Units)");
    Time+=Temp*60*60*1000;
    Element_Info1(Ztring().Duration_From_Milliseconds(Time));

    BS_End();

    if (Time!=167185000)
        return Ztring().Duration_From_Milliseconds(Time);
    else
        return Ztring(); //If all bits are set to 1, this is invalid
}

} //NameSpace

#endif //MEDIAINFO_DV_YES

