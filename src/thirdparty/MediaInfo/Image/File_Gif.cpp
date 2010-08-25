// File_Gif - Info for GIF files
// Copyright (C) 2005-2010 MediaArea.net SARL, Info@MediaArea.net
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
// From http://www.onicos.com/staff/iz/formats/gif.html
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_GIF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Gif.h"
#include <cmath>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Gif::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<3)
        return false; //Must wait for more data

    if (CC3(Buffer)!=0x474946) //"GIF"
    {
        Reject("GIF");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gif::Read_Buffer_Continue()
{
    //Parsing
    Ztring Version;
    int16u Width, Height;
    int8u  BackgroundColorIndex, PixelAspectRatio, Resolution, GCT_Size;
    bool GCT_Flag, Sort;
    Skip_Local(3,                                               "Header");
    Get_Local (3, Version,                                      "Version");
    Get_L2 (Width,                                              "Logical Screen Width");
    Get_L2 (Height,                                             "Logical Screen Height");
    BS_Begin();
    Get_SB (   GCT_Flag,                                        "Global Color Table Flag");
    Get_S1 (3, Resolution,                                      "Color Resolution");
    Get_SB (   Sort,                                            "Sort Flag to Global Color Table");
    Get_S1 (3, GCT_Size,                                        "Size of Global Color Table"); Param_Info(Ztring::ToZtring((int16u)pow(2.0, 1+GCT_Size)));
    BS_End();
    Get_L1 (BackgroundColorIndex,                               "Background Color Index");
    Get_L1 (PixelAspectRatio,                                   "Pixel Aspect Ratio");
    if (GCT_Flag)
        Skip_XX((int16u)pow(2.0, 1+GCT_Size)*3,                 "Global Color Table");
    Element_End();

    FILLING_BEGIN();
        Accept("GIF");

        Stream_Prepare(Stream_Image);
        Fill(Stream_Image, 0, Image_Width, Width);
        Fill(Stream_Image, 0, Image_Height, Height);
        Fill(Stream_Image, 0, Image_Format, _T("GIF"));
        Fill(Stream_Image, 0, Image_Format_Profile, Version);
        Fill(Stream_Image, 0, Image_Codec, _T("GIF")+Version);

        Finish("GIF");
    FILLING_END();
}

} //NameSpace

#endif
