// File_Bmp - Info for Bitmap files
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BMP - Format
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// From http://www.onicos.com/staff/iz/formats/bmp.html
//
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
#if defined(MEDIAINFO_BMP_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Bmp.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Bmp_CompressionMethod(int32u CompressionMethod)
{
    switch(CompressionMethod)
    {
        case 0 : return "RGB";
        case 1 : return "RLE";
        case 2 : return "RLE";
        case 3 : return "Bit field";
        case 4 : return "JPEG";
        case 5 : return "PNG";
        default: return "";
    }
}

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Bmp::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<2)
        return false; //Must wait for more data

    if (CC2(Buffer)!=0x424D) //"BM"
    {
        Reject("BMP");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Bmp::Read_Buffer_Continue()
{
    //Parsing
    int32u Size, DIB_Size, Offset;
    Element_Begin("File header", 14);
        Skip_C2(                                                "Magic");
        Get_L4 (Size,                                           "Size");
        Skip_L2(                                                "Reserved");
        Skip_L2(                                                "Reserved");
        Get_L4 (Offset,                                         "Offset of data");
    Element_End();

    FILLING_BEGIN();
        if (Size!=File_Size)
        {
            Reject("BMP");
            return;
        }

        Accept("BMP");

        Fill(Stream_General, 0, General_Format, "Bitmap");

        Stream_Prepare(Stream_Image);
    FILLING_END();

    Element_Begin("DIB header");
        Peek_L4 (DIB_Size);
        switch (DIB_Size)
        {
            case  40 : BitmapInfoHeader(); break;
            case  12 : Skip_XX(DIB_Size-4,                      "OS/2 v1 header"); break;
            case  64 : Skip_XX(DIB_Size-4,                      "OS/2 v2 header"); break;
            case 108 : Skip_XX(DIB_Size-4,                      "BitmapV4Header"); break;
            case 124 : Skip_XX(DIB_Size-4,                      "BitmapV5Header"); break;
            default  : Skip_XX(DIB_Size-4,                      "Unknown header");
            ;
        }
    Element_End();

    Skip_XX(Offset-Element_Offset,                              "Color palette");
    Skip_XX(File_Size-Offset,                                   "Bitmap data");

    //No need of more
    Finish("BMP");
}

//***************************************************************************
// Buffer - Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Bmp::BitmapInfoHeader()
{
    //Parsing
    Element_Begin("Bitmap Info header", 40);
    int32u Width, Height, CompressionMethod;
    int16u BitsPerPixel;
    Skip_L4(                                                    "Size");
    Get_L4 (Width,                                              "Width");
    Get_L4 (Height,                                             "Height");
    Skip_L2(                                                    "Color planes");
    Get_L2 (BitsPerPixel,                                       "Bits per pixel");
    Get_L4 (CompressionMethod,                                  "Compression method"); Param_Info(Bmp_CompressionMethod(CompressionMethod));
    Skip_L4(                                                    "Image size");
    Skip_L4(                                                    "Horizontal resolution");
    Skip_L4(                                                    "Vertical resolution");
    Skip_L4(                                                    "Number of colors in the color palette");
    Skip_L4(                                                    "Number of important colors used");

    FILLING_BEGIN();
        Fill(Stream_Image, 0, Image_Width, Width);
        Fill(Stream_Image, 0, Image_Height, Height);
        Fill(Stream_Image, 0, Image_Resolution, BitsPerPixel);
        Fill(Stream_Image, 0, Image_Format, Bmp_CompressionMethod(CompressionMethod));
        Fill(Stream_Image, 0, Image_Codec, Bmp_CompressionMethod(CompressionMethod));
    FILLING_END();
}

} //NameSpace

#endif
