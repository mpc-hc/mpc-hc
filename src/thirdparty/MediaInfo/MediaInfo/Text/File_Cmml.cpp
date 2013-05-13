/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Note : the buffer must be given in ONE call
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
#if defined(MEDIAINFO_CMML_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Cmml.h"
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Cmml::Header_Parse()
{
    //Filling
    Header_Fill_Code(0, "Cmml");
    Header_Fill_Size(Element_Size);
}

//---------------------------------------------------------------------------
void File_Cmml::Data_Parse()
{
    //Parsing
    if (Status[IsAccepted])
        Configuration();
    else
        Identification();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Cmml::Identification()
{
    Element_Name("Identification");

    //Parsing
    int16u VersionMajor, VersionMinor;
    Skip_Local(8,                                               "Signature");
    Get_L2 (VersionMajor,                                       "version major");
    Get_L2 (VersionMinor,                                       "version minor");
    Skip_L8(                                                    "granule rate numerator");
    Skip_L8(                                                    "granule rate denominator");
    Skip_L1(                                                    "granule shift");

    FILLING_BEGIN();
        Accept("CMML");

        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, Text_Format, "CMML");
        Fill(Stream_Text, 0, Text_Codec,  "CMML");
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Cmml::Configuration()
{
    Element_Name("Configuration");

    //Parsing
    Ztring Data;
    Get_UTF8(Element_Size, Data,                                "Data");

    FILLING_BEGIN();
        Ztring Value;
        Value=Data.SubString(__T("<head>"), __T("</head>"));
        if (!Value.empty())
            Fill(Stream_Text, 0, Text_Title, Value.SubString(__T("<title>"), __T("</title>")));
        if (Data.find(__T("<clip"))!=string::npos)
            Finish("CMML");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_CMML_YES
