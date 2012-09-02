// File_Vp8 - Info for MPEG Video files
// Copyright (C) 2004-2012 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_VP8_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Vp8.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/BitStream.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaInfoLib
{
//---------------------------------------------------------------------------

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Vp8::File_Vp8()
:File__Analyze()
{
    //Configuration
    ParserName=__T("VP8");
    IsRawStream=true;

    //In
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.3?32:4;
}

//---------------------------------------------------------------------------
File_Vp8::~File_Vp8()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vp8::Streams_Accept()
{
    Stream_Prepare(Stream_Video);
}

//---------------------------------------------------------------------------
void File_Vp8::Streams_Update()
{
}

//---------------------------------------------------------------------------
void File_Vp8::Streams_Fill()
{
    Fill(Stream_Video, 0, Video_Format, "VP8");
    Fill(Stream_Video, 0, Video_Codec, "VP8");
}

//---------------------------------------------------------------------------
void File_Vp8::Streams_Finish()
{
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vp8::Read_Buffer_Continue()
{
    Accept();

    BS_Begin_LE(); //VP8 bitstream is Little Endian
    bool frame_type;
    Get_TB (    frame_type,                                     "frame type");
    Skip_T1( 3,                                                 "version number");
    Skip_TB(                                                    "show_frame flag");
    Skip_T3(19,                                                 "size of the first data partition");
    BS_End();

    if (!frame_type) //I-Frame
    {
        Skip_B3(                                                "0x9D012A");
        Skip_L2(                                                "Width");
        Skip_L2(                                                "Height");
    }
    Skip_XX(Element_Size-Element_Offset,                        "Other data");

    Frame_Count++;
    if (Frame_Count>=Frame_Count_Valid)
        Finish();
}

} //NameSpace

#endif //MEDIAINFO_VP8_YES
