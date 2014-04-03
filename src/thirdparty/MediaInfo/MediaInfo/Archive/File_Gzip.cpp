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
#if defined(MEDIAINFO_GZIP_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Archive/File_Gzip.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Gzip::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<2)
        return false; //Must wait for more data

    if (Buffer[0]!=0x1F
     || Buffer[1]!=0x8B)
    {
        Reject("Gzip");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gzip::Read_Buffer_Continue()
{
    //Parsing
    int8u CM;
    Skip_B2(                                                    "IDentification");
    Get_B1 (CM,                                                 "Compression Method");
    Skip_B1(                                                    "FLaGs");
    Skip_B4(                                                    "Modified TIME");
    Skip_XX(File_Size-10,                                       "Data");

    FILLING_BEGIN();
        //Filling
        Accept("Gzip");

        Fill(Stream_General, 0, General_Format, "GZip");
        Fill(Stream_General, 0, General_Format_Profile, "deflate");

        Finish("Gzip");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_GZIP_YES
