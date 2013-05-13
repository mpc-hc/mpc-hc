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
#if defined(MEDIAINFO_7Z_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Archive/File_7z.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_7z::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<6)
        return false; //Must wait for more data

    if (CC6(Buffer)!=0x377ABCAF271CLL) //"7z...."
    {
        Reject("7-Zip");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_7z::Read_Buffer_Continue()
{
    Skip_B6(                                                    "Magic");
    Skip_XX(File_Size-6,                                        "Data");

    FILLING_BEGIN();
        Accept("7-Zip");

        Fill(Stream_General, 0, General_Format, "7-Zip");

        Finish("7-Zip");
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_7Z_YES
