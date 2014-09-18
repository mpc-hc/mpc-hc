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
#if defined(MEDIAINFO_EXR_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Exr.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//***************************************************************************
// Const
//***************************************************************************

enum Elements
{
    Pos_FileInformation,
    Pos_GenericSection,
    Pos_IndustrySpecific,
    Pos_UserDefined,
    Pos_Padding,
    Pos_ImageData,
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Exr::File_Exr()
:File__Analyze()
{
    //Configuration
    ParserName=__T("EXR");
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Exr::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, "EXR");

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
        Stream_Prepare(Stream_Image);

    //Configuration
    Buffer_MaximumSize=64*1024*1024; //Some big frames are possible (e.g YUV 4:2:2 10 bits 1080p)
    Frame_Count_NotParsedIncluded=0;
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Exr::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    if (CC4(Buffer)!=0x762F3101) //"v/1"+1
    {
        Reject();
        return false;
    }

    //All should be OK...
    Accept();

    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Exr::Header_Begin()
{
    //Header
    if (Buffer_Offset+4>Buffer_Size)
        return false;
    if (CC4(Buffer+Buffer_Offset)==0x762F3101) //"v/1"+1
        return Buffer_Offset+12<=Buffer_Size;

    //Name
    name_End=0;
    while (Buffer_Offset+name_End<Buffer_Size)
    {
        if (Buffer[Buffer_Offset+name_End]=='\0')
            break;
        if (name_End==31)
            break;
        name_End++;
    }
    if (Buffer_Offset+name_End>=Buffer_Size)
        return false;
    if (name_End>=31)
    {
        Reject();
        return false;
    }
    if (name_End==0)
        return true;

    //Type
    type_End=0;
    while (Buffer_Offset+name_End+1+type_End<Buffer_Size)
    {
        if (Buffer[Buffer_Offset+name_End+1+type_End]=='\0')
            break;
        if (type_End==31)
            break;
        type_End++;
    }

    if (Buffer_Offset+name_End+1+type_End>=Buffer_Size)
        return false;
    if (type_End>=31)
    {
        Reject();
        return false;
    }

    if (Buffer_Offset+name_End+1+type_End+1+4>=Buffer_Size)
        return false;

    return true;
}

//---------------------------------------------------------------------------
void File_Exr::Header_Parse()
{
    //Header
    if (CC4(Buffer+Buffer_Offset)==0x762F3101) //"v/1"+1
    {
        //Filling
        Header_Fill_Code(0, "File header");
        Header_Fill_Size(12);
        return;
    }

    //Image data
    if (name_End==0)
    {
        //Filling
        Header_Fill_Code(0, "Image data");
        Header_Fill_Size(ImageData_End-(File_Offset+Buffer_Offset));
        return;
    }

    int32u size;
    Get_String(name_End, name,                                  "name");
    Element_Offset++; //Null byte
    Get_String(type_End, type,                                  "type");
    Element_Offset++; //Null byte
    Get_L4 (size,                                               "size");

    //Filling
    Header_Fill_Code(0, Ztring().From_Local(name.c_str()));
    Header_Fill_Size(name_End+1+type_End+1+4+size);
}

//---------------------------------------------------------------------------
void File_Exr::Data_Parse()
{
    if (CC4(Buffer+Buffer_Offset)==0x762F3101) //"v/1"+1 //Header
        Header();
    else if (name_End==0)
        ImageData();
    else if (name=="comments" && type=="string")
        comments();
    else if (name=="compression" && type=="compression" && Element_Size==1)
        compression();
    else if (name=="dataWindow" && type=="box2i" && Element_Size==16)
        dataWindow();
    else if (name=="displayWindow" && type=="box2i" && Element_Size==16)
        displayWindow();
    else if (name=="pixelAspectRatio" && type=="float" && Element_Size==4)
        pixelAspectRatio();
    else
        Skip_XX(Element_Size,                                   "value");
}

//---------------------------------------------------------------------------
void File_Exr::Header()
{
    //Parsing
    int32u Flags;
    int8u Version;
    Skip_L4(                                                    "Magic number");
    Get_L1 (Version,                                            "Version field");
    Get_L3 (Flags,                                              "Flags");

    //Filling
    if (Frame_Count==0)
    {
        Fill(Stream_General, 0, General_Format_Version, __T("Version ")+Ztring::ToZtring(Version));
        Fill(StreamKind_Last, 0, "Format", "EXR");
        Fill(StreamKind_Last, 0, "Format_Version", __T("Version ")+Ztring::ToZtring(Version));
        Fill(StreamKind_Last, 0, "Format_Profile", (Flags&0x02)?"Tile":"Line");
    }
    Frame_Count++;
    if (Frame_Count_NotParsedIncluded!=(int64u)-1)
        Frame_Count_NotParsedIncluded++;
    ImageData_End=Config->File_Current_Size;
}

//---------------------------------------------------------------------------
void File_Exr::ImageData()
{
    Skip_XX(Element_Size,                                       "data");

    if (!Status[IsFilled])
        Fill();
    if (Config->ParseSpeed<1.0)
        Finish();
}

//---------------------------------------------------------------------------
void File_Exr::comments ()
{
    //Parsing
    Ztring value;
    Get_Local(Element_Size, value,                              "value");

    //Filling
    if (Frame_Count==1)
        Fill(StreamKind_Last, 0, General_Comment, value);
}

//---------------------------------------------------------------------------
void File_Exr::compression ()
{
    //Parsing
    int8u value;
    Get_L1 (value,                                              "value");

    //Filling
    std::string Compression;
    switch (value)
    {
        case 0x00 : Compression="raw"; break;
        case 0x01 : Compression="RLZ"; break;
        case 0x02 : Compression="ZIPS"; break;
        case 0x03 : Compression="ZIP"; break;
        case 0x04 : Compression="PIZ"; break;
        case 0x05 : Compression="PXR24"; break;
        case 0x06 : Compression="B44"; break;
        case 0x07 : Compression="B44A"; break;
        default   : ;
    }

    if (Frame_Count==1)
        Fill(StreamKind_Last, 0, "Format_Compression", Compression.c_str());
}

//---------------------------------------------------------------------------
void File_Exr::dataWindow ()
{
    //Parsing
    int32u xMin, yMin, xMax, yMax;
    Get_L4 (xMin,                                               "xMin");
    Get_L4 (yMin,                                               "yMin");
    Get_L4 (xMax,                                               "xMax");
    Get_L4 (yMax,                                               "yMax");
}

//---------------------------------------------------------------------------
void File_Exr::displayWindow ()
{
    //Parsing
    int32u xMin, yMin, xMax, yMax;
    Get_L4 (xMin,                                               "xMin");
    Get_L4 (yMin,                                               "yMin");
    Get_L4 (xMax,                                               "xMax");
    Get_L4 (yMax,                                               "yMax");

    //Filling
    if (Frame_Count==1)
    {
        Fill(StreamKind_Last, 0, "Width", xMax-xMin+1);
        Fill(StreamKind_Last, 0, "Height", yMax-yMin+1);
    }
}

//---------------------------------------------------------------------------
void File_Exr::pixelAspectRatio ()
{
    //Parsing
    float value;
    Get_LF4(value,                                              "value");

    //Filling
    if (Frame_Count==1)
        Fill(StreamKind_Last, 0, "PixelAspectRatio", value?value:1, 3);
}

} //NameSpace

#endif //MEDIAINFO_EXR_YES
