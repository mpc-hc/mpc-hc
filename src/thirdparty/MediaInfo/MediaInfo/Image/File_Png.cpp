/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about PNG files
//
// From http://www.fileformat.info/format/png/
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
#if defined(MEDIAINFO_PNG_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Png.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
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
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Png::File_Png()
{
    //Config
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    IsRawStream=true;

    //Temp
    Signature_Parsed=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Png::Streams_Accept()
{
    if (!IsSub)
    {
        TestContinuousFileNames();

        Stream_Prepare((Config->File_Names.size()>1 || Config->File_IsReferenced_Get())?Stream_Video:Stream_Image);
        if (File_Size!=(int64u)-1)
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_StreamSize), File_Size);
        if (StreamKind_Last==Stream_Video)
            Fill(Stream_Video, StreamPos_Last, Video_FrameCount, Config->File_Names.size());
    }
    else
        Stream_Prepare(StreamKind_Last);

    //Configuration
    Frame_Count_NotParsedIncluded=0;
}

//***************************************************************************
// Header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Png::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<8)
        return false; //Must wait for more data

    if (CC4(Buffer+4)!=0x0D0A1A0A) //Byte order
    {
        Reject("PNG");
        return false;
    }

    switch (CC4(Buffer)) //Signature
    {
        case 0x89504E47 :
            Accept("PNG");

            Fill(Stream_General, 0, General_Format, "PNG");
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Format), "PNG");
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Codec), "PNG");

            break;

        case 0x8A4E4E47 :
            Accept("PNG");

            Fill(Stream_General, 0, General_Format, "MNG");
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Format), "MNG");
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Codec), "MNG");

            Finish("PNG");
            break;

        case 0x8B4A4E47 :
            Accept("PNG");

            Fill(Stream_General, 0, General_Format, "JNG");
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Format), "JNG");
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Codec), "JNG");

            Finish("PNG");
            break;

        default:
            Reject("PNG");
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Png::Read_Buffer_Unsynched()
{
    Signature_Parsed=false;

    Read_Buffer_Unsynched_OneFramePerFile();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Png::Header_Parse()
{
    if (!Signature_Parsed)
    {
        //Filling
        Header_Fill_Size(8);
        Header_Fill_Code(0, "File header");

        return;
    }

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
    if (!Signature_Parsed)
    {
        Signature();
        return;
    }

    Element_Size-=4; //For CRC

    #define CASE_INFO(_NAME, _DETAIL) \
        case Elements::_NAME : Element_Info1(_DETAIL); _NAME(); break;

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
void File_Png::Signature()
{
    //Parsing
    Skip_B4(                                                    "Signature");
    Skip_B4(                                                    "ByteOrder");

    Frame_Count++;
    if (Frame_Count_NotParsedIncluded!=(int64u)-1)
        Frame_Count_NotParsedIncluded++;
    Signature_Parsed=true;
}

//---------------------------------------------------------------------------
void File_Png::IEND()
{
    Signature_Parsed=false;
}

//---------------------------------------------------------------------------
void File_Png::IHDR()
{
    //Parsing
    int32u Width, Height;
    int8u  Bit_depth, Colour_type, Compression_method, Interlace_method;
    Get_B4 (Width,                                              "Width");
    Get_B4 (Height,                                             "Height");
    Get_B1 (Bit_depth,                                          "Bit depth");
    Get_B1 (Colour_type,                                        "Colour type"); Param_Info1(Png_Colour_type(Colour_type));
    Get_B1 (Compression_method,                                 "Compression method");
    Skip_B1(                                                    "Filter method");
    Get_B1 (Interlace_method,                                   "Interlace method");

    FILLING_BEGIN_PRECISE();
        if (!Status[IsFilled])
        {
            Fill(StreamKind_Last, 0, "Width", Width);
            Fill(StreamKind_Last, 0, "Height", Height);
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
                Fill(StreamKind_Last, 0, "BitDepth", Resolution);
            switch (Compression_method)
            {
                case 0 :
                    Fill(StreamKind_Last, 0, "Format_Compression", "LZ77");
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

            Fill();
        }

        if (Config->ParseSpeed<1.0)
            Finish("PNG"); //No need of more
    FILLING_END();
}

} //NameSpace

#endif
