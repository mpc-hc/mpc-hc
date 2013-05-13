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
    ParserName=__T("Fraps");
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
