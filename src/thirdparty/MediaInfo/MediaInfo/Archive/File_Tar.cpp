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
#if defined(MEDIAINFO_TAR_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Archive/File_Tar.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tar::Read_Buffer_Continue()
{
    if (File_Size<257)
    {
        Reject();
        return;
    }
    if (Buffer_Size<257)
        return; //Wait for more data

    //Parsing
    Ztring ChecksumO;
    Skip_Local(100,                                             "File name");
    Skip_Local(  8,                                             "File mode");
    Skip_Local(  8,                                             "Owner's numeric user ID");
    Skip_Local( 12,                                             "Group's numeric user ID");
    Skip_Local( 12,                                             "File size in bytes");
    Skip_Local(  8,                                             "Last modification time in numeric Unix time format");
    Get_Local (  8, ChecksumO,                                  "Checksum for header block");
    Skip_B1(                                                    "Link indicator (file type)");
    Skip_Local(100,                                             "Name of linked file");
    Skip_XX(File_Size-257,                                      "Data");

    FILLING_BEGIN();
        //Handling Checksum
        int32u Checksum=ChecksumO.To_int32u(8);
        int32u ChecksumU=0;
        int32u ChecksumS=0;
        for (size_t Pos=0; Pos<257; Pos++)
        {
            if (Pos==148)
            {
                ChecksumU+=32*8; //8 spaces
                ChecksumS+=32*8; //8 spaces
                Pos+=7; //Skiping Checksum
            }
            ChecksumU+=(int8u)Buffer[Pos];
            ChecksumS+=(int8s)Buffer[Pos];
        }

        if (ChecksumU!=Checksum && ChecksumS!=Checksum)
        {
            Reject("Tar");
            return;
        }

        //Filling
        Accept("Tar");

        Fill(Stream_General, 0, General_Format, "Tar");

        Reject("Tar");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_TAR_YES
