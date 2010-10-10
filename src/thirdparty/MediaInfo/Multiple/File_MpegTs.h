// File_Mpegts - Info for MPEG Transport Stream files
// Copyright (C) 2006-2010 MediaArea.net SARL, Info@MediaArea.net
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
#include "MediaInfo/Duplicate/File__Duplicate__Writer.h"
#include <map>
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_MpegTs
//***************************************************************************

class File_MpegTs : public File__Duplicate
{
public :
    //In
    #ifdef MEDIAINFO_BDAV_YES
        size_t BDAV_Size;
    #endif
    #ifdef MEDIAINFO_TSP_YES
        size_t TSP_Size;
    #endif

    //Constructor/Destructor
    File_MpegTs();
    ~File_MpegTs();

private :
    //Streams management
    void Streams_Fill();
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
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    void Header_Parse();
    void Header_Parse_AdaptationField();
    #ifdef MEDIAINFO_MPEGTS_PCR_YES
    void Header_Parse_AdaptationField_Duration_Update();
    #endif //MEDIAINFO_MPEGTS_PCR_YES
    void Data_Parse();

    int16u                      PID;
    bool                        payload_unit_start_indicator;

    //Global infos
    complete_stream* Complete_Stream;

    //Elements
    void PSI();
    void PSI_EPG_Update();
    void PSI_Duration_End_Update();
    void PES();

    //Helpers
    bool Header_Parser_QuickSearch();
    void Detect_EOF();

    //Temp
    #if defined(MEDIAINFO_BDAV_YES) || defined(MEDIAINFO_TSP_YES)
        size_t TS_Size;
    #endif
    int64u MpegTs_JumpTo_Begin;
    int64u MpegTs_JumpTo_End;
    bool   Searching_TimeStamp_Start;

    #if MEDIAINFO_EVENTS
        void Header_Parse_Events();
        void Header_Parse_Events_Duration(int64u program_clock_reference);
    #else //MEDIAINFO_EVENTS
        inline void Header_Parse_Events() {}
        inline void Header_Parse_Events_Duration(int64u) {}
    #endif //MEDIAINFO_EVENTS

    //Helpers
    void Streams_Fill_PerStream();
    void Streams_Finish_PerStream();

    //File__Duplicate
    void   File__Duplicate_Streams_Finish ();
    bool   File__Duplicate_Set  (const Ztring &Value); //Fill a new File__Duplicate value
    void   File__Duplicate_Write ();

    //Output buffer
    size_t Output_Buffer_Get (const String &Value);
    size_t Output_Buffer_Get (size_t Pos);
    std::vector<int16u> Output_Buffer_Get_Pos;
};

} //NameSpace

#endif
