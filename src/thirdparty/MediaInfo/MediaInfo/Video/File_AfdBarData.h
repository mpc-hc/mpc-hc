// File_AfdBarData - Info for Bar Data Video files
// Copyright (C) 2010-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Information about AFD and Bar Data files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_AfdBarDataH
#define MediaInfo_AfdBarDataH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_AfdBarData
//***************************************************************************

class File_AfdBarData : public File__Analyze
{
public :
    //In
    enum format
    {
        Format_Unknown,
        Format_A53_4_DTG1,      //Active Format Description
        Format_A53_4_GA94_06,   //Bar Data
        Format_S2016_3,         //Active Format Description & Bar Data
    };
    format Format;
    int8u  aspect_ratio_FromContainer;  //May come from the containing parser

    //Constructor/Destructor
    File_AfdBarData();

private :
    //Streams management
    void Streams_Fill();
    
    //Buffer - Global
    void Read_Buffer_Continue();

    //Elements
    void afd_data();
    void bar_data();

    //Sream
    struct stream
    {
        int16u line_number_end_of_top_bar;
        int16u line_number_start_of_bottom_bar;
        int16u pixel_number_end_of_left_bar;
        int16u pixel_number_start_of_right_bar;
        int8u  active_format;
        int8u  aspect_ratio;

        stream()
        {
            line_number_end_of_top_bar=(int16u)-1;
            line_number_start_of_bottom_bar=(int16u)-1;
            pixel_number_end_of_left_bar=(int16u)-1;
            pixel_number_start_of_right_bar=(int16u)-1;
            active_format=(int8u)-1;
            aspect_ratio=(int8u)-1;
        }
    };
    stream Stream;

    //Temp
    int16u line_number_end_of_top_bar;
    int16u line_number_start_of_bottom_bar;
    int16u pixel_number_end_of_left_bar;
    int16u pixel_number_start_of_right_bar;
    int8u  active_format;
    int8u  aspect_ratio;
};

} //NameSpace

#endif

