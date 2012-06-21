// File_AvsV - Info for AVS Video files
// Copyright (C) 2008-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Information about AVS Video files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_AvsVH
#define MediaInfo_AvsVH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/Multiple/File_Mpeg4.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Avs
//***************************************************************************

class File_AvsV : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;
    bool   FrameIsAlwaysComplete;

    //constructor/Destructor
    File_AvsV();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin() {return FileHeader_Begin_0x000001();}

    //Buffer - Synchro
    bool Synchronize() {return Synchronize_0x000001();}
    bool Synched_Test();
    void Synched_Init();
    
    //Buffer - Per element
    void Header_Parse();
    bool Header_Parser_QuickSearch();
    bool Header_Parser_Fill_Size();
    void Data_Parse();

    //Elements
    void slice();
    void video_sequence_start();
    void video_sequence_end();
    void user_data_start();
    void extension_start();
    void picture_start();
    void video_edit();
    void reserved();
    
    //Count of a Packets
    size_t progressive_frame_Count;
    size_t Interlaced_Top;
    size_t Interlaced_Bottom;

    //From user_data
    Ztring Library;
    Ztring Library_Name;
    Ztring Library_Version;
    Ztring Library_Date;

    //Temp
    int32u  bit_rate;                           //From video_sequence_start
    int16u  horizontal_size;                    //From video_sequence_start
    int16u  vertical_size;                      //From video_sequence_start
    int16u  display_horizontal_size;            //From sequence_display
    int16u  display_vertical_size;              //From sequence_display
    int8u   profile_id;                         //From video_sequence_start
    int8u   level_id;                           //From video_sequence_start
    int8u   chroma_format;                      //From video_sequence_start
    int8u   aspect_ratio;                       //From video_sequence_start
    int8u   frame_rate_code;                    //From video_sequence_start
    int8u   video_format;                       //From sequence_display
    bool    progressive_sequence;               //From video_sequence_start
    bool    low_delay;                          //From video_sequence_start
    bool    video_sequence_start_IsParsed;      //From video_sequence_start

    //Streams
    struct stream
    {
        bool   Searching_Payload;

        stream()
        {
            Searching_Payload=false;
        }
    };
    std::vector<stream> Streams;
};

} //NameSpace

#endif
