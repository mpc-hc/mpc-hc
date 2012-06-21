// File_Dirac - Info for Dirac files
// Copyright (C) 2007-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Information about Dirac files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DiracH
#define MediaInfo_File_DiracH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Dirac
//***************************************************************************

class File_Dirac : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;
    bool   Ignore_End_of_Sequence;

    //Constructor/Destructor
    File_Dirac();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin() {return FileHeader_Begin_0x000001();}

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Synched_Init();

    //Buffer - Per element
    void Header_Parse();
    bool Header_Parser_QuickSearch();
    bool Header_Parser_Fill_Size();
    void Data_Parse();

    //Elements
    void Sequence_header();
    void End_of_Sequence();
    void Auxiliary_data();
    void Padding_data();
    void Intra_Reference_Picture();
    void Intra_Non_Reference_Picture();
    void Intra_Reference_Picture_No();
    void Intra_Non_Reference_Picture_No();
    void Inter_Reference_Picture_1();
    void Inter_Reference_Picture_2();
    void Inter_Non_Reference_Picture_1();
    void Inter_Non_Reference_Picture_2();
    void Reference_Picture_Low();
    void Intra_Non_Reference_Picture_Low();
    void Reserved();
    void picture();

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

    //Temp
    int32u frame_width;
    int32u frame_height;
    int32u chroma_format;
    int32u source_sampling;
    int32u clean_width;
    int32u clean_height;
    int32u clean_left_offset;
    int32u clean_top_offset;
    float32 frame_rate;
    float32 pixel_aspect_ratio;
};

} //NameSpace

#endif
