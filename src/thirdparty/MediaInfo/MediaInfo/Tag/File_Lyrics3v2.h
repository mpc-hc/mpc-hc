/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Lyrics3v2 tagged files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Lyrics3v2H
#define MediaInfo_File_Lyrics3v2H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Lyrics3v2
//***************************************************************************

class File_Lyrics3v2 : public File__Analyze
{
public :
    //In
    int64u TotalSize;

    //Constructor/Destructor
    File_Lyrics3v2();

private :
    //Buffer - File header
    void FileHeader_Parse();

    //Buffer - Per element
    void Header_Parse ();
    void Data_Parse ();

    //Elements
    void Header();
    void Footer();
    void AUT() {Skip_Local(Element_Size, "Value");}
    void CRC() {Skip_Local(Element_Size, "Value");}
    void EAL();
    void EAR();
    void ETT();
    void IMG() {Skip_Local(Element_Size, "Value");}
    void IND();
    void INF();
    void LYR();
};

} //NameSpace

#endif
