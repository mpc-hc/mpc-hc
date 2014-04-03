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
#if defined(MEDIAINFO_ACE_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Archive/File_Ace.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ace::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<7)
        return false; //Must wait for more data

    if (Buffer[0]!=0x2A //"**ACE**"
     || Buffer[1]!=0x2A
     || Buffer[2]!=0x41
     || Buffer[3]!=0x43
     || Buffer[4]!=0x45
     || Buffer[5]!=0x2A
     || Buffer[6]!=0x2A)
    {
        Reject("Ace");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ace::Read_Buffer_Continue()
{
    //Parsing
    Skip_B7(                                                    "Magic");
    Skip_XX(File_Size-7,                                        "Data");

    FILLING_BEGIN();
        Accept("Ace");

        Fill(Stream_General, 0, General_Format, "ACE");

        Finish("Ace");
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_ACE_YES
