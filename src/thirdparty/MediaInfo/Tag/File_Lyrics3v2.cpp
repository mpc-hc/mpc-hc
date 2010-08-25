// File_Lyrics3v2 - Info for Lyrics3v2 tagged files
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

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_LYRICS3V2_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File_Lyrics3v2.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int32u AUT=0x415554;
    const int32u CRC=0x435243;
    const int32u EAL=0x45414C;
    const int32u EAR=0x454152;
    const int32u ETT=0x455454;
    const int32u IMG=0x494D47;
    const int32u IND=0x494E44;
    const int32u INF=0x494E46;
    const int32u LYR=0x4C5952;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Lyrics3v2::File_Lyrics3v2()
:File__Analyze()
{
    //Configuration
    TotalSize=(int64u)-1;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lyrics3v2::FileHeader_Parse()
{
    if (TotalSize==(int64u)-1)
        TotalSize=Buffer_Size;

    //Parsing
    Skip_Local(11,                                              "Signature");

    FILLING_BEGIN();
        Accept("Lyrics3v2");

        TotalSize-=11;
    FILLING_END();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lyrics3v2::Header_Parse()
{
    if (TotalSize<=15) //First 10 is minimum size of a tag, Second 10 is ID3v2 header size
    {
        //Place for footer
        Header_Fill_Code((int64u)-1, "File Footer");
        Header_Fill_Size(TotalSize);
        return;
    }

    //Parsing
    Ztring SizeT;
    int64u Size;
    int32u Field;
    Get_C3 (Field,                                           "Field");
    Get_Local(5, SizeT,                                      "Size");
    Size=8+SizeT.To_int64u();

    //Filling
    if (Size+15>TotalSize)
        Size=TotalSize-15;
    Header_Fill_Code(Field, Ztring().From_CC3(Field));
    Header_Fill_Size(Size);
    TotalSize-=Size;
}

//---------------------------------------------------------------------------
void File_Lyrics3v2::Data_Parse()
{
    #define CASE_INFO(_NAME, _DETAIL) \
        case Elements::_NAME : Element_Info(_DETAIL); _NAME(); break;

    //Parsing
    switch (Element_Code)
    {
        CASE_INFO(AUT,                                          "Lyrics Author Name");
        CASE_INFO(CRC,                                          "CRC");
        CASE_INFO(EAL,                                          "Extended Album name");
        CASE_INFO(EAR,                                          "Extended Artist name");
        CASE_INFO(ETT,                                          "Extended Track Title");
        CASE_INFO(IMG,                                          "Image location");
        CASE_INFO(IND,                                          "Indications field");
        CASE_INFO(INF,                                          "Additional information");
        CASE_INFO(LYR,                                          "Lyrics");
        case (int64u)-1 : Footer(); break;
        default : Skip_XX(Element_Size,                         "Data");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lyrics3v2::Footer()
{
    //Parsing
    Skip_Local(6,                                               "Size");
    Skip_Local(9,                                               "Signature");

    Finish("Lyrics3v2");
}

//---------------------------------------------------------------------------
void File_Lyrics3v2::EAL()
{
    //Parsing
    Ztring Value;
    Get_Local(Element_Size, Value,                              "Value");

    //Filling
    Fill(Stream_General, 0, General_Album, Value);
}

//---------------------------------------------------------------------------
void File_Lyrics3v2::EAR()
{
    //Parsing
    Ztring Value;
    Get_Local(Element_Size, Value,                              "Value");

    //Filling
    Fill(Stream_General, 0, General_Performer, Value);
}

//---------------------------------------------------------------------------
void File_Lyrics3v2::ETT()
{
    //Parsing
    Ztring Value;
    Get_Local(Element_Size, Value,                              "Value");

    //Filling
    Fill(Stream_General, 0, General_Title, Value);
}

//---------------------------------------------------------------------------
void File_Lyrics3v2::IND()
{
    //Parsing
    if (Element_Size>=1)
        Skip_Local(1,                                           "lyrics present");
    if (Element_Size>=2)
        Skip_Local(1,                                           "timestamp in lyrics");
    if (Element_Size>=3)
        Skip_Local(1,                                           "inhibits tracks for random selection");
    while (Element_Offset<Element_Size)
        Skip_Local(1,                                           "unknown");
}

//---------------------------------------------------------------------------
void File_Lyrics3v2::INF()
{
    //Parsing
    Ztring Value;
    Get_Local(Element_Size, Value,                              "Value");

    //Filling
    Fill(Stream_General, 0, General_Comment, Value);
}

//---------------------------------------------------------------------------
void File_Lyrics3v2::LYR()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Value");

    //Filling
    Stream_Prepare(Stream_Text);
    Fill(Stream_Text, 0, Text_Codec, "Lyrics3v2");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_LYRICS3V2_YES

