// File_Kate - Info for Kate files
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
#if defined(MEDIAINFO_KATE_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Kate.h"
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
Ztring Kate_Category(const Ztring &Category)
{
    //http://wiki.xiph.org/index.php/OggText#Categories_of_Text_Codecs
    if (Category==_T("CC"))
        return _T("Closed caption");
    if (Category==_T("SUB"))
        return _T("Subtitles");
    if (Category==_T("TAD"))
        return _T("Textual audio descriptions");
    if (Category==_T("KTV"))
        return _T("Karaoke");
    if (Category==_T("TIK"))
        return _T("Ticker text");
    if (Category==_T("AR"))
        return _T("Active regions");
    if (Category==_T("NB"))
        return _T("Semantic annotations");
    if (Category==_T("META"))
        return _T("Metadata, mostly machine-readable");
    if (Category==_T("TRX"))
        return _T("Transcript");
    if (Category==_T("LRC"))
        return _T("Lyrics");
    if (Category==_T("LIN"))
        return _T("Linguistic markup");
    if (Category==_T("CUE"))
        return _T("Cue points");

    //From Kate
    if (Category==_T("K-SLD-I"))
        return _T("Slides, as images");
    if (Category==_T("K-SLD-T"))
        return _T("Slides, as text");
    return Category;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Kate::Data_Parse()
{
    //Parsing
    Identification();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Kate::Identification()
{
    Element_Name("Identification");

    //Parsing
    Ztring Language, Category;
    int16u Width, Height;
    int8u VersionMajor, VersionMinor, NumHeaders, TextEncoding;
    Skip_B1   (                                                 "Signature");
    Skip_Local(7,                                               "Signature");
    Skip_L1(                                                    "Reserved");
    Get_L1 (VersionMajor,                                       "version major");
    Get_L1 (VersionMinor,                                       "version minor");
    Get_L1 (NumHeaders,                                         "num headers");
    Get_L1 (TextEncoding,                                       "text encoding");
    Skip_L1(                                                    "directionality");
    Skip_L1(                                                    "Reserved");
    Skip_L1(                                                    "granule shift");
    Skip_L4(                                                    "Reserved");
    Get_L2 (Width,                                              "cw sh + canvas width");
    Get_L2 (Height,                                             "ch sh + canvas height");
    /*
    BS_Begin();
    Skip_BS( 4,                                                 "cw sh");
    Get_BS (12, Width,                                          "canvas width");
    Skip_BS( 4,                                                 "ch sh");
    Get_BS (12, Height,                                         "canvas height");
    BS_End();
    */
    Skip_L4(                                                    "granule rate numerator");
    Skip_L4(                                                    "granule rate denominator");
    Get_UTF8(16, Language,                                      "Language");
    Get_UTF8(16, Category,                                      "Category");

    FILLING_BEGIN();
        Accept("Kate");

        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, Text_Format, "Kate");
        Fill(Stream_Text, 0, Text_Codec,  "Kate");
        Fill(Stream_Text, 0, Text_Language, Language);
        Fill(Stream_Text, 0, Text_Language_More, Kate_Category(Category));

        Finish("Kate");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_KATE_YES
