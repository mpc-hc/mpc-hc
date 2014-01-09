/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG Transport Stream files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_MpegTsH
#define MediaInfo_MpegTsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Duplicate.h"
#include "MediaInfo/Multiple/File_Mpeg_Psi.h"
#include "MediaInfo/Duplicate/File__Duplicate_MpegTs.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_MpegTs
//***************************************************************************

class File_MpegTs :
#if MEDIAINFO_DUPLICATE
    public File__Duplicate
#else //MEDIAINFO_DUPLICATE
    public File__Analyze
#endif //MEDIAINFO_DUPLICATE
{
public :
    //In
    #ifdef MEDIAINFO_BDAV_YES
        size_t BDAV_Size;
    #endif
    #ifdef MEDIAINFO_TSP_YES
        size_t TSP_Size;
    #endif
    #ifdef MEDIAINFO_ARIBSTDB24B37_YES
        bool FromAribStdB24B37;
    #endif

    //Constructor/Destructor
    File_MpegTs();
    ~File_MpegTs();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Fill();
    void Streams_Update();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Synched_Init();

    //Options
    void Option_Manage ();

    //Buffer - Global
    void Read_Buffer_Continue();
    void Read_Buffer_AfterParsing ();
    void Read_Buffer_Unsynched();
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK

    //Buffer - Per element
    void Header_Parse();
    void Header_Parse_AdaptationField();
    void Data_Parse();

    int16u                      pid;
    int8u                       transport_scrambling_control;
    bool                        payload_unit_start_indicator;

    //Global infos
    complete_stream* Complete_Stream;

    //Elements
    void PSI();
    void PES();
    void PES_Parse_Finish();

    //Helpers
    bool Header_Parser_QuickSearch();

    //Temp
    #if defined(MEDIAINFO_BDAV_YES) || defined(MEDIAINFO_TSP_YES)
        size_t TS_Size;
    #endif
    int64u MpegTs_JumpTo_Begin;
    int64u MpegTs_JumpTo_End;
    int64u Begin_MaxDuration; //in 27 MHz
    int64u Buffer_TotalBytes_LastSynched;
    bool   ForceStreamDisplay;
    bool   Searching_TimeStamp_Start;

    #if MEDIAINFO_EVENTS
        void Header_Parse_Events();
        void Header_Parse_Events_Duration(int64u program_clock_reference);
    #else //MEDIAINFO_EVENTS
        inline void Header_Parse_Events() {}
        inline void Header_Parse_Events_Duration(int64u) {}
    #endif //MEDIAINFO_EVENTS

    //Helpers
    void Streams_Update_Programs();
    void Streams_Update_Programs_PerStream(size_t StreamID);
    void Streams_Update_EPG();
    void Streams_Update_EPG_PerProgram(complete_stream::transport_stream::programs::iterator Program);
    #ifdef MEDIAINFO_MPEGTS_PCR_YES
    void Streams_Update_Duration_Update();
    #if MEDIAINFO_ADVANCED
        float64 Config_VbrDetection_Delta;
        int64u  Config_VbrDetection_Occurences;
        bool    Config_VbrDetection_GiveUp;
    #endif // MEDIAINFO_ADVANCED
    #endif //MEDIAINFO_MPEGTS_PCR_YES
    void Streams_Update_Duration_End();
    void SetAllToPES();

    #if MEDIAINFO_DUPLICATE
        //File__Duplicate
        void   File__Duplicate_Streams_Finish ();
        bool   File__Duplicate_Set  (const Ztring &Value); //Fill a new File__Duplicate value
        void   File__Duplicate_Write ();

        //Output buffer
        size_t Output_Buffer_Get (const String &Value);
        size_t Output_Buffer_Get (size_t Pos);
        std::vector<int16u> Output_Buffer_Get_Pos;
    #endif //MEDIAINFO_DUPLICATE

    //Config
    bool Config_Trace_TimeSection_OnlyFirstOccurrence;
    bool TimeSection_FirstOccurrenceParsed;

    #if MEDIAINFO_SEEK
        std::map<int16u, int64u>    Unsynch_Frame_Counts;
        int64u                      Seek_Value;
        int64u                      Seek_Value_Maximal;
        int64u                      Seek_ID;
        size_t                      InfiniteLoop_Detect;
        bool                        Duration_Detected;
    #endif //MEDIAINFO_SEEK
};

} //NameSpace

#endif
