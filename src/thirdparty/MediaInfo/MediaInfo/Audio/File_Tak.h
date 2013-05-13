/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Tak files
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_TakH
#define MediaInfo_File_TakH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File__Tags.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Tak
//***************************************************************************

class File_Tak : public File__Analyze, public File__Tags_Helper
{
public :
    //In
    bool VorbisHeader;

    //Constructor/Destructor
    File_Tak();

private :
    //Streams management
    void Streams_Finish()                                                       {File__Tags_Helper::Streams_Finish();}

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Global
    void Read_Buffer_Continue()                                                 {File__Tags_Helper::Read_Buffer_Continue();}

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void ENDOFMETADATA();
    void STREAMINFO();
    void SEEKTABLE();
    void WAVEMETADATA();
    void ENCODERINFO();
    void PADDING()                                                              {Skip_XX(Element_Size, "Padding");}
};

} //NameSpace

#endif
