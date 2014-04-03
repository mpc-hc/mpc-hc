/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// TGA format
//
// From http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf
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
#if defined(MEDIAINFO_TGA_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Tga.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
const char* Tga_Image_Type_Compression(int8u Image_Type)
{
    switch (Image_Type)
    {
        case  1 : return "Color-mapped";
        case  2 :
        case  3 : return "Raw";
        case  9 : return "Color-mapped + RLE";
        case 10 :
        case 11 : return "RLE";
        case 32 :
        case 33 : return "Huffman";
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Tga_Image_Type_ColorSpace(int8u Image_Type)
{
    switch (Image_Type)
    {
        case  1 :
        case  2 :
        case  9 :
        case 10 :
        case 32 :
        case 33 : return "RGB";
        case  3 :
        case 11 : return "Y";
        default : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Tga::File_Tga()
{
    //Configuration
    ParserName=__T("TGA");
    Buffer_MaximumSize=64*1024*1024; //Some big frames are possible (e.g YUV 4:2:2 10 bits 1080p)
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tga::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format, "TGA");
    Fill(Stream_General, 0, General_Format_Version, __T("Version ")+Ztring::ToZtring(Version));
    Fill(Stream_General, 0, General_Title, Image_ID);

    Stream_Prepare(Stream_Image);
    Fill(Stream_Image, 0, Image_Format, Tga_Image_Type_Compression(Image_Type));
    Fill(Stream_Image, 0, Image_ColorSpace, Tga_Image_Type_ColorSpace(Image_Type));
    Fill(Stream_Image, 0, Image_CodecID, Image_Type);
    Fill(Stream_Image, 0, Image_Width, Image_Width_);
    Fill(Stream_Image, 0, Image_Height, Image_Height_);
    Fill(Stream_Image, 0, Image_BitDepth, Pixel_Depth);
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Tga::FileHeader_Begin()
{
    //Synchro
    if (18>Buffer_Size)
        return false;
    if (Buffer[2]==0x00
     || Buffer[16]>32) //bit depth
    {
        Reject();
        return false;
    }
    if (Buffer_Size<File_Size)
        return false; //Must wait for more data

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tga::Read_Buffer_Continue()
{
    //Parsing
    Tga_File_Header();
    Image_Color_Map_Data();
    Tga_File_Footer();

    FILLING_BEGIN();
        //Coherency in case of no magic value
        if (Version==1)
        {
            switch (Image_Type)
            {
                case   1 :  //Color mapped images
                case   9 :  //Color mapped images, RLE
                            if (Color_Map_Type!=1)
                            {
                                Reject();
                                return;
                            }
                            break;
                case   2 :  //True-color images
                case  10 :  //True-color images, RLE
                            if (Color_Map_Type)
                            {
                                Reject();
                                return;
                            }
                            break;
                case   3 :  //Black and White
                case  11 :  //Black and White, RLE
                            if (Color_Map_Type)
                            {
                                Reject();
                                return;
                            }
                            break;
                default  :  Reject();
                            return;
            }
            switch (Color_Map_Type)
            {
                case   0 :
                            if (First_Entry_Index || Color_map_Length || Color_map_Entry_Size)
                            {
                                Reject();
                                return;
                            }
                            break;
                case   1 :
                            switch (Color_map_Entry_Size)
                            {
                                case  15 :
                                case  16 :
                                case  24 :
                                case  32 :
                                            break;
                                default  :  Reject();
                                            return;
                                            ;
                            }
                            break;
                default  :  Reject();
                            return;
            }
            switch (Pixel_Depth)
            {
                case   8 :
                case  16 :
                case  24 :
                case  32 :
                            break;
                default  :  Reject();
                            return;
                            ;
            }
        }

        Accept();
        Fill();
        Finish();
    FILLING_END();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tga::Tga_File_Header()
{
    //Parsing
    Element_Begin1("Tga File Header");
    Get_L1 (ID_Length,                                          "ID Length");
    Get_L1 (Color_Map_Type,                                     "Color Map Type");
    Get_L1 (Image_Type,                                         "Image Type"); Param_Info1(Tga_Image_Type_Compression(Image_Type));
    Element_End0();
    Element_Begin1("Color Map Specification");
        Get_L2 (First_Entry_Index,                              "First Entry Index");
        Get_L2 (Color_map_Length,                               "Color map Length");
        Get_L1 (Color_map_Entry_Size,                           "Color map Entry Size");
    Element_End0();
    Element_Begin1("Image Specification");
        Skip_L2(                                                "X-origin of Image");
        Skip_L2(                                                "Y-origin of Image");
        Get_L2 (Image_Width_,                                   "Image Width");
        Get_L2 (Image_Height_,                                  "Image Height");
        Get_L1 (Pixel_Depth,                                    "Pixel Depth");
        Get_L1 (Image_Descriptor,                               "Image Descriptor");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Tga::Image_Color_Map_Data()
{
    Element_Begin1("Image/Color Map Data");
    Get_Local(ID_Length, Image_ID,                              "Image ID");
    if (Color_Map_Type==1)
    {
        int64u EntrySizeInBits=8;
        if (Color_map_Entry_Size<24)
            EntrySizeInBits=Color_map_Entry_Size/3;

        Skip_XX(Color_map_Length*EntrySizeInBits/8,             "Color Map Data");

    }
    if (Element_Offset+26<Element_Size
     && Buffer[Buffer_Size- 18]==0x54
     && Buffer[Buffer_Size- 17]==0x52
     && Buffer[Buffer_Size- 16]==0x55
     && Buffer[Buffer_Size- 15]==0x45
     && Buffer[Buffer_Size- 14]==0x56
     && Buffer[Buffer_Size- 13]==0x49
     && Buffer[Buffer_Size- 12]==0x53
     && Buffer[Buffer_Size- 11]==0x49
     && Buffer[Buffer_Size- 10]==0x4F
     && Buffer[Buffer_Size-  9]==0x4E
     && Buffer[Buffer_Size-  8]==0x2D
     && Buffer[Buffer_Size-  7]==0x58
     && Buffer[Buffer_Size-  6]==0x46
     && Buffer[Buffer_Size-  5]==0x49
     && Buffer[Buffer_Size-  4]==0x4C
     && Buffer[Buffer_Size-  3]==0x45
     && Buffer[Buffer_Size-  2]==0x2E
     && Buffer[Buffer_Size-  1]==0x00)
        Version=2;
     else
        Version=1;
    Skip_XX(Element_Size-Element_Offset-(Version==2?26:0),      "Image Data");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Tga::Tga_File_Footer()
{
    if (Version==1)
        return; //No footer

    Element_Begin1("Image/color Map Data");
    Skip_L4(                                                    "Extension Area Offset");
    Skip_L4(                                                    "Developer Directory Offset");
    Skip_Local(16,                                              "Signature");
    Skip_Local( 1,                                              "Reserved Character");
    Skip_L1(                                                    "Binary Zero String Terminator");
    Element_End0();
}

} //NameSpace

#endif
