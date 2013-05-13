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
#if defined(MEDIAINFO_AFDBARDATA_YES) || defined(MEDIAINFO_MXF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaInfoLib
{
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* AfdBarData_active_format[]=
{
    //1st value is for 4:3, 2nd is for 16:9
    "", //Undefined
    "Reserved",
    "Not recommended",
    "Not recommended",
    "Aspect ratio greater than 16:9", //Use GA94
    "Reserved",
    "Reserved",
    "Reserved",
    "4:3 full frame image / 16:9 full frame image",
    "4:3 full frame image / 4:3 pillarbox image",
    "16:9 letterbox image / 16:9 full frame image",
    "14:9 letterbox image / 14:9 pillarbox image",
    "Reserved",
    "4:3 full frame image, alternative 14:9 center / 4:3 pillarbox image, alternative 14:9 center",
    "16:9 letterbox image, alternative 14:9 center / 16:9 full frame image, alternative 14:9 center",
    "16:9 letterbox image, alternative 4:3 center / 16:9 full frame image, alternative 4:3 center",
};

//---------------------------------------------------------------------------
const char* AfdBarData_active_format_4_3[]=
{
    "", //Undefined
    "Reserved",
    "Letterbox 16:9 image (top)",
    "Letterbox 14:9 image (top)",
    "Letterbox image with an aspect ratio greater than 16:9",
    "Reserved",
    "Reserved",
    "Reserved",
    "Full frame 4:3 image",
    "Full frame 4:3 image",
    "Letterbox 16:9 image",
    "Letterbox 14:9 image",
    "Reserved",
    "Full frame 4:3 image, with alternative 14:9 center",
    "Letterbox 16:9 image, with alternative 14:9 center",
    "Letterbox 16:9 image, with alternative 4:3 center",
};

//---------------------------------------------------------------------------
const char* AfdBarData_active_format_16_9[]=
{
    "", //Undefined
    "Reserved",
    "Letterbox 16:9 image (top)",
    "Pillarbox 14:9 image (top)",
    "Letterbox image with an aspect ratio greater than 16:9",
    "Reserved",
    "Reserved",
    "Reserved",
    "Full frame 16:9 image",
    "Pillarbox 4:3 image",
    "Letterbox 16:9 image",
    "Pillarbox 14:9 image",
    "Reserved",
    "Full frame 4:3 image, with alternative 14:9 center",
    "Letterbox 16:9 image, with alternative 14:9 center",
    "Letterbox 16:9 image, with alternative 4:3 center",
};

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //defined(MEDIAINFO_AFDBARDATA_YES) || defined(MEDIAINFO_MXF_YES)

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AFDBARDATA_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_AfdBarData.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaInfoLib
{
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* AfdBarData_aspect_ratio[]=
{
    "4:3",
    "16:9",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_AfdBarData::File_AfdBarData()
:File__Analyze()
{
    //Configuration
    PTS_DTS_Needed=true;

    //In
    Format=Format_Unknown;
    aspect_ratio_FromContainer=(int8u)-1;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_AfdBarData::Streams_Fill()
{
    //Filling
    Stream_Prepare(Stream_Video);
    if (active_format!=(int8u)-1)
    {
        Fill(Stream_Video, 0, Video_ActiveFormatDescription, Stream.active_format);
        if (aspect_ratio==(int8u)-1)
            aspect_ratio=aspect_ratio_FromContainer;
        if (aspect_ratio!=(int8u)-1)
        {
            Fill(Stream_Video, 0, Video_ActiveFormatDescription_String, Stream.aspect_ratio?AfdBarData_active_format_16_9[Stream.active_format]:AfdBarData_active_format_4_3[Stream.active_format]);
            switch (Format)
            {
                case Format_A53_4_DTG1    : Fill(Stream_Video, 0, Video_ActiveFormatDescription_MuxingMode, "A/53"); break;
                case Format_S2016_3       : Fill(Stream_Video, 0, Video_ActiveFormatDescription_MuxingMode, "SMPTE ST 2016-3"); break;
                default                   :
                                            Skip_XX(Element_Size,       "Unknown");
                                            return;
            }
        }
    }
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_AfdBarData::Read_Buffer_Continue()
{
    //Default
    line_number_end_of_top_bar=(int16u)-1;
    line_number_start_of_bottom_bar=(int16u)-1;
    pixel_number_end_of_left_bar=(int16u)-1;
    pixel_number_start_of_right_bar=(int16u)-1;
    active_format=(int8u)-1;
    aspect_ratio=(int8u)-1;

    //Parsing
    switch (Format)
    {
        case Format_A53_4_DTG1    :
                                    afd_data();
                                    break;
        case Format_A53_4_GA94_06 :
                                    bar_data();
                                    break;
        case Format_S2016_3       :
                                    afd_data();
                                    Skip_B1(                    "Reserved");
                                    Skip_B1(                    "Reserved");
                                    bar_data();
                                    break;
        default                   :
                                    Skip_XX(Element_Size,       "Unknown");
                                    return;
    }

    FILLING_BEGIN();
        //Filling
        Stream.line_number_end_of_top_bar=line_number_end_of_top_bar;
        Stream.line_number_start_of_bottom_bar=line_number_start_of_bottom_bar;
        Stream.pixel_number_end_of_left_bar=pixel_number_end_of_left_bar;
        Stream.pixel_number_start_of_right_bar=pixel_number_start_of_right_bar;
        Stream.active_format=active_format;
        Stream.aspect_ratio=aspect_ratio;

        if (!Status[IsAccepted])
        {
            Accept("AfdBarData");
            Fill("AfdBarData");
        }
        if (MediaInfoLib::Config.ParseSpeed_Get()<1)
            Finish("AfdBarData");
    FILLING_END();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_AfdBarData::afd_data()
{
    //Parsing
    Element_Begin1("Active Format Description");
    BS_Begin();
    if (Format==Format_S2016_3)
    {
        Mark_0_NoTrustError();
        Get_S1 (4, active_format,                               "active_format"); Param_Info1(AfdBarData_active_format[active_format]);
        Get_S1 (1, aspect_ratio,                                "aspect_ratio"); Param_Info1(AfdBarData_aspect_ratio[aspect_ratio]);
        Mark_0_NoTrustError();
        Mark_0_NoTrustError();
    }
    else
    {
        bool  active_format_flag;
        Mark_0();
        Get_SB (active_format_flag,                             "active_format_flag");
        Mark_0_NoTrustError();
        Mark_0_NoTrustError();
        Mark_0_NoTrustError();
        Mark_0_NoTrustError();
        Mark_0_NoTrustError();
        Mark_1_NoTrustError();
        if (active_format_flag)
        {
            Mark_1_NoTrustError();
            Mark_1_NoTrustError();
            Mark_1_NoTrustError();
            Mark_1_NoTrustError();
            Get_S1 (4, active_format,                           "active_format"); Param_Info1(AfdBarData_active_format[active_format]);
        }
    }
    BS_End();
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AfdBarData::bar_data()
{
    //Parsing
    Element_Begin1("bar_data");
    bool   top_bar_flag, bottom_bar_flag, left_bar_flag, right_bar_flag;
    BS_Begin();
    Get_SB (top_bar_flag,                                       "top_bar_flag");
    Get_SB (bottom_bar_flag,                                    "bottom_bar_flag");
    Get_SB (left_bar_flag,                                      "left_bar_flag");
    Get_SB (right_bar_flag,                                     "right_bar_flag");
    if (Format==Format_S2016_3)
    {
        Mark_0_NoTrustError();
        Mark_0_NoTrustError();
        Mark_0_NoTrustError();
        Mark_0_NoTrustError();
    }
    else
    {
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
    }
    if (top_bar_flag)
    {
        Mark_1();
        Mark_1();
        Get_S2 (14, line_number_end_of_top_bar,                 "line_number_end_of_top_bar");
    }
    if (bottom_bar_flag)
    {
        Mark_1();
        Mark_1();
        Get_S2 (14, line_number_start_of_bottom_bar,            "line_number_start_of_bottom_bar");
    }
    if (left_bar_flag)
    {
        Mark_1();
        Mark_1();
        Get_S2 (14, pixel_number_end_of_left_bar,               "pixel_number_end_of_left_bar");
    }
    if (right_bar_flag)
    {
        Mark_1();
        Mark_1();
        Get_S2 (14, pixel_number_start_of_right_bar,            "pixel_number_start_of_right_bar");
    }
    if (!top_bar_flag && !bottom_bar_flag && !left_bar_flag && !right_bar_flag)
    {
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Skip_S2(14,                                             "reserved");
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Skip_S2(14,                                             "reserved");
    }
    BS_End();
    Element_End0();

    if (Format==Format_A53_4_DTG1)
    {
        BS_Begin();
        Mark_1();
        Mark_1();
        Mark_1();
        Mark_1();
        Mark_1();
        Mark_1();
        Mark_1();
        Mark_1();
        BS_End();

        if (Element_Size-Element_Offset)
            Skip_XX(Element_Size-Element_Offset,                "additional_bar_data");
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AFDBARDATA_YES

