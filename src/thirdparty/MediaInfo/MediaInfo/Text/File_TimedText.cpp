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
#if defined(MEDIAINFO_TIMEDTEXT_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_TimedText.h"
#include "tinyxml2.h"
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_TimedText::File_TimedText()
{
    //Configuration
    ParserName=__T("Timed Text");

    //Temp
    #ifdef MEDIAINFO_MPEG4_YES
        IsChapter=false;
    #endif //MEDIAINFO_MPEG4_YES
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_TimedText::Header_Parse()
{
    //Parsing
    int16u Size;
    Get_B2 (Size,                                               "Size");

    //Filling
    Header_Fill_Code(0, "Block");
    Header_Fill_Size(Element_Offset+Size);

    //TODO: if IsChapter, it may be UTF-16 (with BOM), it may also be followed by an encd atom (e.g. for UTF-8 00 00 00 0C 65 6E 63 64 00 00 01 00)
}

//---------------------------------------------------------------------------
void File_TimedText::Data_Parse()
{
    //Parsing
    Ztring Value;
    Get_UTF8 (Element_Size, Value,                              "Value");

    FILLING_BEGIN();
        if (!Status[IsAccepted])
        {
            Accept();
            #ifdef MEDIAINFO_MPEG4_YES
                if (IsChapter)
                {
                    Stream_Prepare(Stream_Menu);
                }
                else
            #endif //MEDIAINFO_MPEG4_YES
                {
                    Stream_Prepare(Stream_Text);
                }
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Format), "Timed Text");
        }
        #ifdef MEDIAINFO_MPEG4_YES
            if (IsChapter)
            {
            }
            else
        #endif //MEDIAINFO_MPEG4_YES
            {
                Finish();
            }

        #ifdef MEDIAINFO_MPEG4_YES
            if (IsChapter && FrameInfo.DTS!=(int64u)-1 && Buffer_Offset==2)
                Fill(Stream_Menu, 0, Ztring().Duration_From_Milliseconds(FrameInfo.DTS/1000000).To_UTF8().c_str(), Value);
        #endif //MEDIAINFO_MPEG4_YES
    FILLING_END();

    Element_Offset=Buffer_Size-Buffer_Offset; //Buffer can also contain atoms after the text, ignoring them
}

} //NameSpace

#endif //MEDIAINFO_TIMEDTEXT_YES
