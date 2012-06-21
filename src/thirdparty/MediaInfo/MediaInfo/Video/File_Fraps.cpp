// File_Fraps - Info for Fraps  files
// Copyright (C) 2010-2011 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_FRAPS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Fraps.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Fraps::File_Fraps()
:File__Analyze()
{
    //Configuration
    ParserName=_T("Fraps");
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Fraps::Streams_Fill()
{
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "Fraps");
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Fraps::Read_Buffer_Continue()
{
    //Parsing
    int8u version, flags;
    Get_L1 (version,                                            "version");
    Skip_L2(                                                    "unknown");
    Get_L1 (flags,                                              "flags");
    if (flags&0x40)
        Skip_L4(                                                "unknown");
    switch (version)
    {
        case 0x00 :
                    Version0(); break;
        case 0x01 :
                    Version1(); break;
        case 0x02 :
        case 0x04 :
                    Version2(); break;
        default   : Skip_XX(Element_Size-Element_Offset,         "data");
    }

    Finish();
}

//---------------------------------------------------------------------------
void File_Fraps::Version0()
{
    //Parsing
    Skip_XX(Element_Size-Element_Offset,                        "data");

    FILLING_BEGIN();
        Accept();
        Fill();
        Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
        Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:2:0");
        Fill(Stream_Video, 0, Video_BitDepth, 8);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Fraps::Version1()
{
    //Parsing
    Skip_XX(Element_Size-Element_Offset,                        "data");

    FILLING_BEGIN();
        Accept();
        Fill();
        Fill(Stream_Video, 0, Video_ColorSpace, "RGB");
        Fill(Stream_Video, 0, Video_BitDepth, 8);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Fraps::Version2()
{
    //Parsing
    if (Element_Size>8) //Else this is a repeat frame
    {
        Skip_C4(                                                "FPSx");
        Skip_L4(                                                "offset to the Y plane (minus 8)");
        Skip_L4(                                                "offset to the U plane (minus 8)");
        Skip_L4(                                                "offset to the V plane (minus 8)");
        Skip_XX(Element_Size-Element_Offset,                    "data");
    }

    FILLING_BEGIN();
        Accept();
        Fill();
        Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
        Fill(Stream_Video, 0, Video_BitDepth, 8);
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_FRAPS_YES
