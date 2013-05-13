/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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

