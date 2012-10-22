// File_Psd - Info for PSD files
// Copyright (C) 2005-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
//
// From http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm
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
#if defined(MEDIAINFO_PSD_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Psd.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Psd_ColorMode(int16u ColorMode)
{
    switch(ColorMode)
    {
        case 0 : return "Bitmap";
        case 1 : return "Grayscale";
        case 2 : return "Indexed";
        case 3 : return "RGB";
        case 4 : return "CMYK";
        case 7 : return "Multichannel";
        case 8 : return "Duotone";
        case 9 : return "Lab";
        default: return "";
    }
}

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Psd::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    if (CC4(Buffer)!=0x38425053) //"8BPS"
    {
        Reject("PSD");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// File Header Information
//***************************************************************************

//---------------------------------------------------------------------------
void File_Psd::Read_Buffer_Continue()
{
    //Parsing
    int32u Width, Height;
    int16u BitsDepth, Version, channels, ColorMode;
    Skip_C4(                                                    "Signature");
    Get_B2 (Version,                                            "Version"); //  1 = PSD, 2 = PSB
    Skip_B6(                                                    "Reserved");
    Get_B2 (channels,                                           "channels"); // 1 to 56, including alpha channel
    Get_B4 (Height,                                             "Height");
    Get_B4 (Width,                                              "Width");
    Get_B2 (BitsDepth,                                          "Depth"); // 1,8,16 or 32
    Get_B2 (ColorMode,                                          "Color Mode"); Param_Info1(Psd_ColorMode(ColorMode));

    FILLING_BEGIN();
        Accept("PSD");
        Stream_Prepare(Stream_Image);
        Fill(Stream_Image, 0, Image_Format, Version==1?"PSD":"PSB");
        Fill(Stream_Image, 0, Image_Format_Version, Version);
        Fill(Stream_Image, 0, Image_ColorSpace, Psd_ColorMode(ColorMode));
        Fill(Stream_Image, 0, Image_Width, Width);
        Fill(Stream_Image, 0, Image_Height, Height);
        Fill(Stream_Image, 0, Image_BitDepth, BitsDepth);
        Finish("PSD");
    FILLING_END();
}


} //NameSpace

#endif
