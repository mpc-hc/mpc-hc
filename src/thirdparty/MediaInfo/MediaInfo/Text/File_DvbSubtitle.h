// File_DvbSubtitle - Info for DVB Subtitle streams
// Copyright (C) 2011-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Information about DVB Subtitle streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DvbSubtitleH
#define MediaInfo_File_DvbSubtitleH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
using namespace std;
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

