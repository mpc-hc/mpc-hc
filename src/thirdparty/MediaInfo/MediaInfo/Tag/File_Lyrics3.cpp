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
#if defined(MEDIAINFO_LYRICS3_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File_Lyrics3.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Lyrics3::File_Lyrics3()
:File__Analyze()
{
    //Configuration
    TotalSize=(int64u)-1;
}

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lyrics3::Read_Buffer_Continue()
{
    if (TotalSize==(int64u)-1)
        TotalSize=Buffer_Size;

    //Coherency
    if (TotalSize<20)
    {
        Reject("Lyrics3");
        return;
    }

    //Buffer size
    if (Buffer_Size<TotalSize)
        return;

    //Parsing
    Element_Offset=0;
    Element_Size=TotalSize;
    Skip_Local(11,                                              "Signature");
    Skip_Local(TotalSize-20,                                    "Lyrics");
    Skip_Local(9,                                               "Signature");

    //Filling
    Accept("Lyric3");

    Stream_Prepare(Stream_Text);
    Fill(Stream_Text, 0, Text_Codec, "Lyrics3");

    Finish("Lyrics3");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_LYRICS3_YES

