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
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_7z::FileHeader_Begin()
{
    // Minimum buffer size
    if (Buffer_Size<6)
        return false; // Must wait for more data

    // Testing
    if (Buffer[0]!=0x37 // "7z...."
     || Buffer[1]!=0x7A
     || Buffer[2]!=0xBC
     || Buffer[3]!=0xAF
     || Buffer[4]!=0x27
     || Buffer[5]!=0x1C)
    {
        Reject("7-Zip");
        return false;
    }

    // All should be OK...
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
