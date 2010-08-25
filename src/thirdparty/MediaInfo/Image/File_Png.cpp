// File_Png - Info for PNG files
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
// PNG - Format
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// From http://www.fileformat.info/format/png/
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
#if defined(MEDIAINFO_PNG_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Png.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Png_Colour_type(int8u Colour_type)
{
    switch (Colour_type)
    {
        case 0 : return "Greyscale";
        case 2 : return "Truecolour";
        case 3 : return "Indexed-colour";
        case 4 : return "Greyscale with alpha";
        case 6 : return "Truecolour with alpha";
    default: return "";
    }
}

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int32u IDAT=0x49444154;
    const int32u IEND=0x49454E44;
    const int32u IHDR=0x49484452;
    const int32u PLTE=0x506C5445;
}

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Png::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<8)
        return false; //Must wait for more data

    if (CC4(Buffer+4)!=0x0D0A1A0A)
    {
        Reject("PNG");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
void File_Png::FileHeader_Parse()
{
    //Parsing
    int32u Signature;
    Get_B4 (Signature,                                          "Signature");
    Skip_B4(                                                    "ByteOrder");

    FILLING_BEGIN();
        switch (Signature)
        {
            case 0x89504E47 :
                Accept("PNG");

                Fill(Stream_General, 0, General_Format, "PNG");

                Stream_Prepare(Stream_Image);

                break;

            case 0x8A4E4E47 :
                Accept("PNG");

                Stream_Prepare(Stream_Image);
                Fill(Stream_Image, 0, Image_Codec, "MNG");
                Fill(Stream_Image, 0, Image_Format, "MNG");

                Finish("PNG");
                break;

            case 0x8B4A4E47 :
                Accept("PNG");

                Stream_Prepare(Stream_Image);
                Fill(Stream_Image, 0, Image_Format, "JNG");
                Fill(Stream_Image, 0, Image_Codec, "JNG");

                Finish("PNG");
                break;

            default:
                Reject("PNG");
        }
    FILLING_END();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Png::Header_Parse()
{
    //Parsing
    int32u Length, Chunk_Type;
    Get_B4 (Length,                                             "Length");
    Get_C4 (Chunk_Type,                                         "Chunk Type");

    //Filling
    Header_Fill_Size(12+Length); //+4 for CRC
    Header_Fill_Code(Chunk_Type, Ztring().From_CC4(Chunk_Type));
}

//---------------------------------------------------------------------------
void File_Png::Data_Parse()
{
    Element_Size-=4; //For CRC

    #define CASE_INFO(_NAME, _DETAIL) \
        case Elements::_NAME : Element_Info(_DETAIL); _NAME(); break;

    //Parsing
    switch (Element_Code)
    {
        CASE_INFO(IDAT,                                         "Image data");
        CASE_INFO(IEND,                                         "Image trailer");
        CASE_INFO(IHDR,                                         "Image header");
        CASE_INFO(PLTE,                                         "Palette table");
        default : Skip_XX(Element_Size,                         "Unknown");
    }

    Element_Size+=4; //For CRC
    Skip_B4(                                                    "CRC");
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Png::IHDR()
{
    //Parsing
    int32u Width, Height;
    int8u  Bit_depth, Colour_type, Compression_method, Interlace_method;
    Get_B4 (Width,                                              "Width");
    Get_B4 (Height,                                             "Height");
    Get_B1 (Bit_depth,                                          "Bit depth");
    Get_B1 (Colour_type,                                        "Colour type"); Param_Info(Png_Colour_type(Colour_type));
    Get_B1 (Compression_method,                                 "Compression method");
    Skip_B1(                                                    "Filter method");
    Get_B1 (Interlace_method,                                   "Interlace method");

    FILLING_BEGIN_PRECISE();
        Fill(Stream_Image, 0, Image_Width, Width);
        Fill(Stream_Image, 0, Image_Height, Height);
        int8u Resolution;
        switch (Colour_type)
        {
            case 0 : Resolution=Bit_depth; break;
            case 2 : Resolution=Bit_depth*3; break;
            case 3 : Resolution=Bit_depth; break;
            case 4 : Resolution=Bit_depth*2; break;
            case 6 : Resolution=Bit_depth*4; break;
            default: Resolution=0;
        }
        if (Resolution)
            Fill(Stream_Image, 0, Image_Resolution, Resolution);
        switch (Compression_method)
        {
            case 0 :
                Fill(Stream_Image, 0, Image_Format, "LZ77");
                Fill(Stream_Image, 0, Image_Codec,  "LZ77 variant");
                break;
            default: ;
        }
        switch (Interlace_method)
        {
            case 0 :
                break;
            case 1 :
                break;
            default: ;
        }

        Finish("PNG");
    FILLING_END();
}

} //NameSpace

#endif
