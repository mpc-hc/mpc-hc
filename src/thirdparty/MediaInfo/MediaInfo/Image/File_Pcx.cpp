/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
//
// From http://courses.engr.illinois.edu/ece390/books/labmanual/graphics-pcx.html
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
#if defined(MEDIAINFO_PCX_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Pcx.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Pcx_VersionInfo(int16u Version)
{
    switch(Version)
    {
        case 0 : return "Paintbrush v2.5";
        case 2 : return "Paintbrush v2.8 with palette information";
        case 3 : return "Paintbrush v2.8 without palette information";
        case 4 : return "Paintbrush/Windows";
        case 5 : return "Paintbrush v3.0+";
        default: return "";
    }
}

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Pcx::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<130) //Size of the header
        return false; //Must wait for more data

    if (Buffer[0]!=0x0A
     || Buffer[1]>0x05
     || Buffer[2]!=0x01
     || !(Buffer[3]==1 || Buffer[3]==4 || Buffer[3]==8 || Buffer[3]==24))
    {
        Reject("PCX");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// File Header Information
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcx::Read_Buffer_Continue()
{
    //Parsing
    int16u XMin, YMin, XMax, YMax, HorDPI, VertDPI, BytesPerLine, PaletteType, HScrSize, VScrSize;
    int8u Manufacturer, Version, EncodingScheme, BitsPerPixel, ColorPlanes;

    Get_L1 (Manufacturer,                                       "Manufacturer");
    Get_L1 (Version,                                            "Version"); // 0,2,3,4,5
    Get_L1 (EncodingScheme,                                     "EncodingScheme"); // RLE=1
    Get_L1 (BitsPerPixel,                                       "Bits Per Pixel"); // 1,4,8,24
    Get_L2 (XMin,                                               "Left margin of image");
    Get_L2 (YMin,                                               "Upper margin of image");
    Get_L2 (XMax,                                               "Right margin of image");
    Get_L2 (YMax,                                               "Lower margin of image");
    Get_L2 (HorDPI,                                             "Horizontal Resolution");
    Get_L2 (VertDPI,                                            "Vertical Resolution");
    Skip_XX(48,                                                 "Palette");
    Skip_L1(                                                    "Reserved");
    Get_L1 (ColorPlanes,                                        "ColorPlanes");
    Get_L2 (BytesPerLine,                                       "BytesPerLine");
    Get_L2 (PaletteType,                                        "PaletteType");
    Get_L2 (HScrSize,                                           "Horizontal Screen Size");
    Get_L2 (VScrSize,                                           "Vertical Screen Size");
    Skip_XX(56,                                                 "Filler");


    FILLING_BEGIN();
        //Integrity tests
        if (XMax<=XMin
         || YMax<=YMin
         || BytesPerLine<XMax-XMin)
        {
            Reject("PCX");
            return;
        }

        Accept("PCX");
        Stream_Prepare(Stream_Image);
        Fill(Stream_Image, 0, Image_Format, "PCX");
        Fill(Stream_Image, 0, Image_Format_Version, Pcx_VersionInfo(Version));
        Fill(Stream_Image, 0, Image_Width, XMax-XMin);
        Fill(Stream_Image, 0, Image_Height, YMax-YMin);
        Fill(Stream_Image, 0, Image_BitDepth, BitsPerPixel);
        Fill(Stream_Image, 0, "DPI", Ztring::ToZtring(VertDPI) + __T(" x ") + Ztring::ToZtring(HorDPI));
        Finish("PCX");
    FILLING_END();
}


} //NameSpace

#endif
