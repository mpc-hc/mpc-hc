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
    if (Category==__T("CC"))
        return __T("Closed caption");
    if (Category==__T("SUB"))
        return __T("Subtitles");
    if (Category==__T("TAD"))
        return __T("Textual audio descriptions");
    if (Category==__T("KTV"))
        return __T("Karaoke");
    if (Category==__T("TIK"))
        return __T("Ticker text");
    if (Category==__T("AR"))
        return __T("Active regions");
    if (Category==__T("NB"))
        return __T("Semantic annotations");
    if (Category==__T("META"))
        return __T("Metadata, mostly machine-readable");
    if (Category==__T("TRX"))
        return __T("Transcript");
    if (Category==__T("LRC"))
        return __T("Lyrics");
    if (Category==__T("LIN"))
        return __T("Linguistic markup");
    if (Category==__T("CUE"))
        return __T("Cue points");

    //From Kate
    if (Category==__T("K-SLD-I"))
        return __T("Slides, as images");
    if (Category==__T("K-SLD-T"))
        return __T("Slides, as text");
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
