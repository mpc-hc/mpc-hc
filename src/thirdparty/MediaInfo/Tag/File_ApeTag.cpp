// File_ApeTag - Info for ApeTag tagged files
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
#if defined(MEDIAINFO_APETAG_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File_ApeTag.h"
#include <algorithm>
#include <ctime>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
void File_ApeTag::FileHeader_Parse()
{
    //Parsing
    int64u Signature;
    Peek_B8(Signature);
    if (Signature==0x4150455441474558LL) //"APETAGEX"
        HeaderFooter(); //v2

    FILLING_BEGIN();
        Accept("ApeTag");

        Stream_Prepare(Stream_General);

        Stream_Prepare(Stream_Audio);
    FILLING_END();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_ApeTag::Header_Begin()
{
    if (Buffer_Size<0x20)
        return false; //At least 32 bytes are needed for footer

    return true;
}

//---------------------------------------------------------------------------
void File_ApeTag::Header_Parse()
{
    //Testing if begin or end of tags
    if (CC8(Buffer+Buffer_Offset)==0x4150455441474558LL) //"APETAGEX"
    {
        //Filling
        Header_Fill_Code((int64u)-1, "File Footer");
        Header_Fill_Size(0x20);
        return;
    }

    //Parsing
    Ztring Value;
    int32u Flags, Length;
    Get_L4 (Length,                                         "Length");
    Get_L4 (Flags,                                          "Flags");
        Skip_Flags(Flags,  0,                               "Read Only");
        Skip_Flags(Flags,  1,                               "Binary");
        Skip_Flags(Flags,  2,                               "Locator of external stored information");
        Skip_Flags(Flags, 29,                               "Is the header");
        Skip_Flags(Flags, 30,                               "Contains a footer");
        Skip_Flags(Flags, 31,                               "Contains a header");
    size_t Pos=(size_t)Element_Offset;
    for (; Pos<Element_Size; Pos++)
        if (Buffer[Buffer_Offset+Pos]==0x00)
            break;
    if (Pos==Element_Size)
    {
        Element_WaitForMoreData();
        return;
    }
    Get_String(Pos-Element_Offset, Key,                     "Key");
    Skip_L1(                                                "0x00");

    //Filling
    Header_Fill_Code(0, Key.c_str());
    Header_Fill_Size(Element_Offset+Length);
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_ApeTag::HeaderFooter()
{
    //Parsing
    int32u Flags;
    Skip_C8(                                                    "Preamble");
    Skip_L4(                                                    "Version");
    Skip_L4(                                                    "Size");
    Skip_L4(                                                    "Count");
    Get_L4 (Flags,                                              "Flags");
        Skip_Flags(Flags,  0,                                   "Read Only");
        Skip_Flags(Flags,  1,                                   "Binary");
        Skip_Flags(Flags,  2,                                   "Locator of external stored information");
        Skip_Flags(Flags, 29,                                   "Is the header");
        Skip_Flags(Flags, 30,                                   "Contains a footer");
        Skip_Flags(Flags, 31,                                   "Contains a header");
    Skip_L8(                                                    "Reserved");
}

//---------------------------------------------------------------------------
void File_ApeTag::Data_Parse()
{
    //If footer
    if (Element_Code==(int64u)-1)
    {
        HeaderFooter();
        Finish("ApeTag");
        return;
    }

    //Parsing
    Ztring Value;
    Get_UTF8(Element_Size, Value,                               "Value"); Element_Info(Value);

    //Filling
    transform(Key.begin(), Key.end(), Key.begin(), (int(*)(int))toupper); //(int(*)(int)) is a patch for unix
         if (Key=="ALBUM")          Fill(Stream_General, 0, General_Album, Value);
    else if (Key=="ARTIST")         Fill(Stream_General, 0, General_Performer, Value);
    else if (Key=="AUTHOR")         Fill(Stream_General, 0, General_WrittenBy, Value);
    else if (Key=="BAND")           Fill(Stream_General, 0, General_Performer, Value);
    else if (Key=="COMMENT")        Fill(Stream_General, 0, General_Comment, Value);
    else if (Key=="COMMENTS")       Fill(Stream_General, 0, General_Comment, Value);
    else if (Key=="COMPOSER")       Fill(Stream_General, 0, General_Composer, Value);
    else if (Key=="CONTENTGROUP")   Fill(Stream_General, 0, General_Genre, Value);
    else if (Key=="COPYRIGHT")      Fill(Stream_General, 0, General_Copyright, Value);
    else if (Key=="DISK")
    {
                                    if (Value.find(_T("/"))!=Error)
                                    {
                                        Fill(Stream_General, 0, General_Part_Position_Total, Value.SubString(_T("/"), _T("")));
                                        Fill(Stream_General, 0, General_Part_Position, Value.SubString(_T(""), _T("/")));
                                    }
                                    else
                                        Fill(Stream_General, 0, General_Track_Position, Value);
    }
    else if (Key=="ENCODEDBY")      Fill(Stream_General, 0, General_EncodedBy, Value);
    else if (Key=="GENRE")          Fill(Stream_General, 0, General_Genre, Value);
    else if (Key=="ORIGARTIST")     Fill(Stream_General, 0, General_Original_Performer, Value);
    else if (Key=="TITLE")          Fill(Stream_General, 0, General_Title, Value);
    else if (Key=="TRACK")
    {
                                    if (Value.find(_T("/"))!=Error)
                                    {
                                        Fill(Stream_General, 0, General_Track_Position_Total, Value.SubString(_T("/"), _T("")));
                                        Fill(Stream_General, 0, General_Track_Position, Value.SubString(_T(""), _T("/")));
                                    }
                                    else
                                        Fill(Stream_General, 0, General_Track_Position, Value);
    }
    else if (Key=="UNSYNCEDLYRICS") Fill(Stream_General, 0, General_Lyrics, Value);
    else if (Key=="WWW")            Fill(Stream_General, 0, General_Title_Url, Value);
    else if (Key=="YEAR")           Fill(Stream_General, 0, General_Recorded_Date, Value);
    else if (Key=="CONTENT GROUP DESCRIPTION") Fill(Stream_General, 0, General_Title, Value);
    else if (Key=="ORIGINAL ALBUM/MOVIE/SHOW TITLE") Fill(Stream_General, 0, General_Original_Album, Value);
    else if (Key=="ORIGINAL ARTIST(S)/PERFORMER(S)") Fill(Stream_General, 0, General_Original_Performer, Value);
    else if (Key=="MP3GAIN_MINMAX") Fill(Stream_Audio, 0, "MP3Gain, Min/Max", Value);
    else if (Key=="MP3GAIN_UNDO") Fill(Stream_Audio, 0, "MP3Gain, Undo", Value);
    else if (Key=="REPLAYGAIN_TRACK_GAIN") Fill(Stream_Audio, 0, Audio_ReplayGain_Gain, Value.To_float64(), 2, true);
    else if (Key=="REPLAYGAIN_TRACK_PEAK") Fill(Stream_Audio, 0, Audio_ReplayGain_Peak, Value.To_float64(), 6, true);
    else                            Fill(Stream_General, 0, Key.c_str(), Value);
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_APETAG_YES

