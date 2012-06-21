// File_Cmml - Info for Cmml files
// Copyright (C) 2009-2011 MediaArea.net SARL, Info@MediaArea.net
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
        Value=Data.SubString(_T("<head>"), _T("</head>"));
        if (!Value.empty())
            Fill(Stream_Text, 0, Text_Title, Value.SubString(_T("<title>"), _T("</title>")));
        if (Data.find(_T("<clip"))!=string::npos)
            Finish("CMML");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_CMML_YES
