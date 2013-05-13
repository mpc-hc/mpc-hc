/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BMP - Format
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// From http://www.onicos.com/staff/iz/formats/bmp.html
//
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
    Element_Begin1("File header");
        Skip_C2(                                                "Magic");
        Get_L4 (Size,                                           "Size");
        Skip_L2(                                                "Reserved");
        Skip_L2(                                                "Reserved");
        Get_L4 (Offset,                                         "Offset of data");
    Element_End0();

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

    Element_Begin1("DIB header");
        Peek_L4 (DIB_Size);
        switch (DIB_Size)
        {
            case  12 : Skip_XX(DIB_Size-4,                      "OS/2 v1 header"); break;
            case  40 : BitmapInfoHeader(); break;
            case  52 : Skip_XX(DIB_Size-4,                      "BitmapV2Header"); break;
            case  56 : Skip_XX(DIB_Size-4,                      "BitmapV3Header"); break;
            case  64 : Skip_XX(DIB_Size-4,                      "OS/2 v2 header"); break;
            case 108 : BitmapV4Header(); break;
            case 124 : Skip_XX(DIB_Size-4,                      "BitmapV5Header"); break;
            default  : Skip_XX(DIB_Size-4,                      "Unknown header");
            ;
        }
    Element_End0();

    Skip_XX(Offset-Element_Offset,                              "Color palette");
    Skip_XX(Element_Size-Offset,                                "Bitmap data");

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
    Element_Begin1("Bitmap Info header");
    int32u Width, Height, CompressionMethod;
    int16u BitsPerPixel;
    Skip_L4(                                                    "Size");
    Get_L4 (Width,                                              "Width");
    Get_L4 (Height,                                             "Height");
    Skip_L2(                                                    "Color planes");
    Get_L2 (BitsPerPixel,                                       "Bits per pixel");
    Get_L4 (CompressionMethod,                                  "Compression method"); Param_Info1(Bmp_CompressionMethod(CompressionMethod));
    Skip_L4(                                                    "Image size");
    Skip_L4(                                                    "Horizontal resolution");
    Skip_L4(                                                    "Vertical resolution");
    Skip_L4(                                                    "Number of colors in the color palette");
    Skip_L4(                                                    "Number of important colors used");
    Element_End0();

    FILLING_BEGIN();
        Fill(Stream_Image, 0, Image_Width, Width);
        Fill(Stream_Image, 0, Image_Height, Height);
        Fill(Stream_Image, 0, Image_BitDepth, BitsPerPixel);
        Fill(Stream_Image, 0, Image_Format, Bmp_CompressionMethod(CompressionMethod));
        Fill(Stream_Image, 0, Image_Codec, Bmp_CompressionMethod(CompressionMethod));
    FILLING_END();
}


void File_Bmp::BitmapV4Header()
{
    //Parsing
    Element_Begin1("Bitmap V4 header");
    int32u Width, Height, CompressionMethod;
    int16u BitsPerPixel;
    Skip_L4(                                                    "Size");
    Get_L4 (Width,                                              "Width");
    Get_L4 (Height,                                             "Height");
    Skip_L2(                                                    "Color planes");
    Get_L2 (BitsPerPixel,                                       "Bits per pixel");
    Get_L4 (CompressionMethod,                                  "Compression method"); Param_Info1(Bmp_CompressionMethod(CompressionMethod));
    Skip_L4(                                                    "Image size");
    Skip_L4(                                                    "Horizontal resolution");
    Skip_L4(                                                    "Vertical resolution");
    Skip_L4(                                                    "Number of colors in the color palette");
    Skip_L4(                                                    "Number of important colors used");
    Skip_L4(                                                    "Red Channel bit mask");
    Skip_L4(                                                    "Green Channel bit mask");
    Skip_L4(                                                    "Blue Channel bit mask");
    Skip_L4(                                                    "Alpha Channel bit mask");
    Skip_L4(                                                    "Color Space endpoints");
    Skip_L4(                                                    "Color Space endpoints");
    Skip_L4(                                                    "Color Space endpoints");
    Skip_L4(                                                    "Color Space endpoints");
    Skip_L4(                                                    "Color Space endpoints");
    Skip_L4(                                                    "Color Space endpoints");
    Skip_L4(                                                    "Color Space endpoints");
    Skip_L4(                                                    "Red Gamma");
    Skip_L4(                                                    "Green Gamma");
    Skip_L4(                                                    "Blue Gamma");
    Element_End0();

    FILLING_BEGIN();
        Fill(Stream_Image, 0, Image_Width, Width);
        Fill(Stream_Image, 0, Image_Height, Height);
        Fill(Stream_Image, 0, Image_BitDepth, BitsPerPixel);
        Fill(Stream_Image, 0, Image_Format, Bmp_CompressionMethod(CompressionMethod));
        Fill(Stream_Image, 0, Image_Codec, Bmp_CompressionMethod(CompressionMethod));
    FILLING_END();
}
} //NameSpace

#endif
