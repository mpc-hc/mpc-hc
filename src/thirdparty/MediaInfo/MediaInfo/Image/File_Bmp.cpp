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
            case  12 : BitmapCoreHeader(1); break;
            case  40 : BitmapInfoHeader(1); break;
            case  52 : BitmapInfoHeader(2); break;
            case  56 : BitmapInfoHeader(3); break;
            case  64 : BitmapCoreHeader(2); break;
            case 108 : BitmapInfoHeader(4); break;
            case 124 : BitmapInfoHeader(5); break;
            default  : if (DIB_Size>124)
                       {
                           BitmapInfoHeader((int8u)-1); //Future versions of BitmapInfoHeader (OS/2 is abandonned)
                           Skip_XX(14+124-Element_Offset,       "Unknown");
                       }
        }
    Element_End0();

    if (Element_Offset<Offset)
        Skip_XX(Offset-Element_Offset,                          "Other header data");
    Skip_XX(File_Size-Offset,                                   "Image data");

    //No need of more
    Finish("BMP");
}

//***************************************************************************
// Buffer - Elements
//***************************************************************************

void File_Bmp::BitmapCoreHeader(int8u Version)
{
    #if MEDIAINFO_TRACE
        switch (Version)
        {
            case 1 : Element_Info1("OS/2 1.x BITMAPCOREHEADER"); break;
            case 2 : Element_Info1("OS/2 2.x BITMAPCOREHEADER"); break;
            default: Element_Info1("OS/2 ? BITMAPCOREHEADER");
        }
    #endif //MEDIAINFO_TRACE

    //Parsing
    int16u Width, Height, BitsPerPixel;
    Skip_L4(                                                    "Size");
    Get_L2 (Width,                                              "Width");
    Get_L2 (Height,                                             "Height");
    Skip_L2(                                                    "Color planes");
    Get_L2 (BitsPerPixel,                                       "Bits per pixel");

    FILLING_BEGIN();
        if (BitsPerPixel<8)
            BitsPerPixel=8; //It is a palette

        Fill(Stream_Image, 0, Image_Width, Width);
        Fill(Stream_Image, 0, Image_Height, Height);
        Fill(Stream_Image, 0, Image_BitDepth, BitsPerPixel);
        Fill(Stream_Image, 0, Image_ColorSpace, "RGB");
    FILLING_END();

    if (Version>1) //V2 additional fields for information only
    {
        Skip_L4(                                                "Compression");
        Skip_L4(                                                "ImageDataSize");
        Skip_L4(                                                "XResolution");
        Skip_L4(                                                "YResolution");
        Skip_L4(                                                "ColorsUsed");
        Skip_L4(                                                "ColorsImportant");
        Skip_L2(                                                "Units");
        Skip_L2(                                                "Reserved");
        Skip_L2(                                                "Recording");
        Skip_L2(                                                "Rendering");
        Skip_L4(                                                "Size1");
        Skip_L4(                                                "Size2");
        Skip_L4(                                                "ColorEncoding");
        Skip_L4(                                                "Identifier");
    }
}

void File_Bmp::BitmapInfoHeader(int8u Version)
{
    #if MEDIAINFO_TRACE
        switch (Version)
        {
            case 1 : Element_Info1("BITMAPINFOHEADER"); break;
            case 2 : Element_Info1("BITMAPV2INFOHEADER"); break;
            case 3 : Element_Info1("BITMAPV3INFOHEADER"); break;
            case 4 : Element_Info1("BITMAPV4HEADER"); break;
            case 5 : Element_Info1("BITMAPV5HEADER"); break;
            default: Element_Info1("BITMAPV?HEADER");
        }
    #endif //MEDIAINFO_TRACE

    //Parsing
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

    FILLING_BEGIN();
        if (BitsPerPixel<8)
            BitsPerPixel=8; //It is a palette

        Fill(Stream_Image, 0, Image_Width, Width);
        Fill(Stream_Image, 0, Image_Height, Height);
        Fill(Stream_Image, 0, Image_BitDepth, BitsPerPixel);
        Fill(Stream_Image, 0, Image_Format, Bmp_CompressionMethod(CompressionMethod));
        Fill(Stream_Image, 0, Image_Codec, Bmp_CompressionMethod(CompressionMethod));
        Fill(Stream_Image, 0, Image_ColorSpace, "RGB");
    FILLING_END();

    if (Version>1)
    {
        Skip_L4(                                                "Red Channel bit mask");
        Skip_L4(                                                "Green Channel bit mask");
        Skip_L4(                                                "Blue Channel bit mask");
        if (Version>2)
        {
            Skip_L4(                                            "Alpha Channel bit mask");
            if (Version>3)
            {
                Skip_L4(                                        "Color Space endpoints");
                Skip_L4(                                        "Color Space endpoints");
                Skip_L4(                                        "Color Space endpoints");
                Skip_L4(                                        "Color Space endpoints");
                Skip_L4(                                        "Color Space endpoints");
                Skip_L4(                                        "Color Space endpoints");
                Skip_L4(                                        "Color Space endpoints");
                Skip_L4(                                        "Red Gamma");
                Skip_L4(                                        "Green Gamma");
                Skip_L4(                                        "Blue Gamma");
                if (Version>4)
                {
                    Skip_L4(                                    "Intent");
                    Skip_L4(                                    "ProfileData");
                    Skip_L4(                                    "ProfileSize");
                    Skip_L4(                                    "Reserved");
                }
            }
        }
    }
}

} //NameSpace

#endif
