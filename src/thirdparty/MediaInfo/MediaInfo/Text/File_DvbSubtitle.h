/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about DVB Subtitle streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DvbSubtitleH
#define MediaInfo_File_DvbSubtitleH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_DvbSubtitle
//***************************************************************************

class File_DvbSubtitle : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;

    //Constructor/Destructor
    File_DvbSubtitle();
    ~File_DvbSubtitle();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Read_Buffer_Unsynched();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test();
    #endif //MEDIAINFO_DEMUX

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void page_composition_segment();
    void region_composition_segment();
    void CLUT_definition_segment();
    void object_data_segment();
    void display_definition_segment();
    void reserved_for_future_use();
    void end_of_display_set_segment();
    void private_data();
    void end_of_PES_data_field_marker();

    //Temp
    bool    MustFindDvbHeader;
    int16u  page_id;
    int8u   subtitle_stream_id;
    struct region_data
    {
        int16u region_horizontal_address;
        int16u region_vertical_address;

        int16u region_width;
        int16u region_height;
        int16u region_depth;

        bool   page_composition_segment;
        bool   region_composition_segment;

        region_data()
        {
            page_composition_segment=false;
            region_composition_segment=false;
        }
    };
    struct page_data
    {
        std::map<int8u, region_data> regions; //Key is region_id
    };
    struct subtitle_stream_data
    {
        std::map<int16u, page_data>  pages; //Key is page_id
    };
    std::map<int8u, subtitle_stream_data> subtitle_streams; //Key is subtitle_stream_id
};

} //NameSpace

#endif

