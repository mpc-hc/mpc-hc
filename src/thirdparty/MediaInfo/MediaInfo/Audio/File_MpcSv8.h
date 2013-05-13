/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Musepack files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_MpcSv8H
#define MediaInfo_File_MpcSv8H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File__Tags.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_MpcSv8
//***************************************************************************

class File_MpcSv8 : public File__Analyze, public File__Tags_Helper
{
public :
    //Constructor/Destructor
    File_MpcSv8();

private :
    //Streams management
    void Streams_Finish()                                                       {File__Tags_Helper::Streams_Finish();}

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Global
    void Read_Buffer_Continue()                                                 {File__Tags_Helper::Read_Buffer_Continue();}

    //Buffer - Per element
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();

    //Elements
    void AP();
    void CT() {Skip_XX(Element_Size, "Data");}
    void EI();
    void RG();
    void SE() {Skip_XX(Element_Size, "Data");}
    void SH();
    void SO();
    void ST() {Skip_XX(Element_Size, "Data");}
};

} //NameSpace

#endif

