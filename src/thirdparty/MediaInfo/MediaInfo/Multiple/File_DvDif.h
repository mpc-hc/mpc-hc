/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about DV-DIF (DV Digital Interface Format)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DvDifH
#define MediaInfo_File_DvDifH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_DvDif
//***************************************************************************

class File_DvDif : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;
    int8u  AuxToAnalyze; //Only Aux must be parsed
    bool   IgnoreAudio;

    //Constructor/Destructor
    File_DvDif();
    ~File_DvDif();

protected :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Synched_Test_Reset();
    void Synched_Init();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test();
    #endif //MEDIAINFO_DEMUX

    //Buffer - Global
    #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
    void Read_Buffer_Continue();
    #endif //MEDIAINFO_DVDIF_ANALYZE_YES
    void Read_Buffer_Unsynched();
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK

    //Buffer
    void Header_Parse();
    void Data_Parse();

    //Elements - Main
    void Header();
    void Subcode();
    void Subcode_Ssyb(int8u syb_num);
    void VAUX();
    void Audio();
    void Video();

    //Elements - Sub
    void Element();
    void timecode();
    void binary_group();
    void audio_source();
    void audio_sourcecontrol();
    void audio_recdate();
    void audio_rectime();
    void video_source();
    void video_sourcecontrol();
    void video_recdate();
    void video_rectime();
    void closed_captions();
    void consumer_camera_1();
    void consumer_camera_2();

    //Helpers
    Ztring recdate();
    Ztring rectime();

    //Streams
    struct stream
    {
        std::map<std::string, Ztring> Infos;
    };
    std::vector<stream*> Streams_Audio;

    //Temp
    #if defined(MEDIAINFO_EIA608_YES)
        std::vector<File__Analyze*> CC_Parsers;
    #endif
    Ztring Recorded_Date_Date;
    Ztring Recorded_Date_Time;
    Ztring Encoded_Library_Settings;
    string TimeCode_FirstFrame;
    int64u Duration;
    int64u TimeCode_FirstFrame_ms;
    int64u FrameSize_Theory; //The size of a frame
    int8u  SCT;
    int8u  SCT_Old;
    int8u  Dseq;
    int8u  Dseq_Old;
    int8u  DBN;
    int8u  DBN_Olds[8];
    int8u  video_source_stype;
    int8u  audio_source_stype;
    bool   FSC;
    bool   FSP;
    bool   DSF;
    bool   DSF_IsValid;
    int8u  APT;
    bool   TF1;
    bool   TF2;
    bool   TF3;
    int8u  aspect;
    int8u  ssyb_AP3;
    bool   FieldOrder_FF;
    bool   FieldOrder_FS;
    bool   Interlaced;
    bool   system;
    bool   FSC_WasSet;
    bool   FSP_WasNotSet;
    bool   video_sourcecontrol_IsParsed;
    bool   audio_locked;

    #if MEDIAINFO_SEEK
        bool            Duration_Detected;
        int64u          TotalFrames;
    #endif //MEDIAINFO_SEEK

    #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
    bool Analyze_Activated;
    bool video_source_Detected;

    void Errors_Stats_Update();
    void Errors_Stats_Update_Finnish();
    Ztring Errors_Stats_03;
    Ztring Errors_Stats_05;
    Ztring Errors_Stats_09;
    Ztring Errors_Stats_10;
    Ztring Date;
    Ztring Time;
    int64u Speed_FrameCount;                            //Global    - Total
    int64u Speed_FrameCount_Video_STA_Errors;           //Global    - Error 1
    std::vector<int64u> Speed_FrameCount_Audio_Errors;  //Global    - Error 2
    int64u Speed_FrameCount_Timecode_Incoherency;       //Global    - Error 3
    int64u Speed_FrameCount_Contains_NULL;              //Global    - Error 4
    int64u Speed_Contains_NULL;                         //Per Frame - Error 4
    int64u Speed_FrameCount_Arb_Incoherency;            //Global    - Error 5
    int64u Speed_FrameCount_Stts_Fluctuation;           //Global    - Error 6
    int8u  QU;
    bool   QU_FSC; //Validity is with QU
    bool   QU_System; //Validity is with QU
    bool   REC_ST;
    bool   REC_END;
    bool   REC_IsValid;
    bool   System;
    bool   System_IsValid;
    bool   Frame_AtLeast1DIF;
    struct dvdate
    {
        int8u  Days;
        int8u  Months;
        int8u  Years;
        bool   MultipleValues;
        bool   IsValid;

        dvdate() {Clear();}

        void Clear()
        {
            MultipleValues=false;
            IsValid=false;
        }
    };
    struct dvtime
    {
        struct time
        {
            int8u  Frames;
            int8u  Seconds;
            int8u  Minutes;
            int8u  Hours;
            bool   DropFrame;

            time()
            {
                Frames=(int8u)-1;
                Seconds=(int8u)-1;
                Minutes=(int8u)-1;
                Hours=(int8u)-1;
                DropFrame=false;
            }
        };
        time    Time;
        bool    MultipleValues;
        bool    IsValid;

        dvtime() {Clear();}

        void Clear()
        {
            MultipleValues=false;
            IsValid=false;
        }
    };
    dvtime Speed_TimeCode_Last;
    dvtime Speed_TimeCode_Current;
    dvtime Speed_TimeCode_Current_Theory;
    Ztring Speed_TimeCodeZ_First;
    Ztring Speed_TimeCodeZ_Last;
    Ztring Speed_TimeCodeZ_Current;
    bool   Speed_TimeCode_IsValid;
    dvtime Speed_RecTime_Current;
    dvtime Speed_RecTime_Current_Theory;
    dvtime Speed_RecTime_Current_Theory2;
    Ztring Speed_RecTimeZ_First;
    Ztring Speed_RecTimeZ_Last;
    Ztring Speed_RecTimeZ_Current;
    dvdate Speed_RecDate_Current;
    Ztring Speed_RecDateZ_First;
    Ztring Speed_RecDateZ_Last;
    Ztring Speed_RecDateZ_Current;
    std::vector<size_t> Video_STA_Errors; //Per STA type
    std::vector<size_t> Video_STA_Errors_Total; //Per STA type
    std::vector<size_t> Audio_Errors; //Per Dseq
    std::vector<size_t> audio_source_IsPresent;
    std::vector<bool>   CH_IsPresent;
    std::vector<std::vector<size_t> > Audio_Errors_Total; //Per Channel and Dseq
    std::vector<std::vector<size_t> > Audio_Invalids; //Per Channel and Dseq
    std::vector<std::vector<size_t> > Audio_Invalids_Total; //Per Channel and Dseq
    struct recZ_Single
    {
        int64u FramePos;
        Ztring Date;
        Ztring Time;

        recZ_Single()
        {
            FramePos=(int64u)-1;
        }
    };
    struct recZ
    {
        recZ_Single First;
        recZ_Single Last;
    };
    std::vector<recZ> Speed_RecZ;
    struct timeCodeZ_Single
    {
        int64u FramePos;
        Ztring TimeCode;

        timeCodeZ_Single()
        {
            FramePos=(int64u)-1;
        }
    };
    struct timeCodeZ
    {
        timeCodeZ_Single First;
        timeCodeZ_Single Last;
    };
    std::vector<timeCodeZ> Speed_TimeCodeZ;
    struct timeStampsZ_Single
    {
        int64u FramePos;
        Ztring Time;
        Ztring TimeCode;
        Ztring Date;

        timeStampsZ_Single()
        {
            FramePos=(int64u)-1;
        }
    };
    struct timeStampsZ
    {
        timeStampsZ_Single First;
        timeStampsZ_Single Last;
    };
    std::vector<timeStampsZ> Speed_TimeStampsZ;



    struct arb
    {
        std::vector<size_t> Value_Counters;
        int8u  Value;
        bool   MultipleValues;
        bool   IsValid;

        arb() {Clear();}

        void Clear()
        {
            Value_Counters.clear();
            Value_Counters.resize(16);
            Value=0xF; //Used only when we are sure
            MultipleValues=false;
            IsValid=false;
        }
    };
    arb Speed_Arb_Last;
    arb Speed_Arb_Current;
    arb Speed_Arb_Current_Theory;
    bool   Speed_Arb_IsValid;

    //Stats
    std::vector<size_t> Stats;
    size_t              Stats_Total;
    size_t              Stats_Total_WithoutArb;
    bool                Stats_Total_AlreadyDetected;

public:
    //From MPEG-4 container
    struct stts_part
    {
        int64u Pos_Begin;
        int64u Pos_End;
        int32u Duration;
    };
    typedef std::vector<stts_part> stts;
    stts* Mpeg4_stts;
    size_t Mpeg4_stts_Pos;
    #endif //MEDIAINFO_DVDIF_ANALYZE_YES
};

} //NameSpace

#endif
