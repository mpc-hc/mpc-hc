/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Flac files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_FlacH
#define MediaInfo_File_FlacH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File__Tags.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Flac
//***************************************************************************

class File_Flac : public File__Analyze, public File__Tags_Helper
{
public :
    //In
    bool VorbisHeader;

    //Constructor/Destructor
    File_Flac();

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
    void STREAMINFO();
    void PADDING()          {Skip_XX(Element_Size, "Data");}
    void APPLICATION();
    void SEEKTABLE()        {Skip_XX(Element_Size, "Data");}
    void VORBIS_COMMENT();
    void CUESHEET()         {Skip_XX(Element_Size, "Data");}
    void PICTURE();

    //Temp
    bool Last_metadata_block;
};

//***************************************************************************
// Const
//***************************************************************************

namespace Flac
{
    const int16u STREAMINFO         =0x00;
    const int16u PADDING            =0x01;
    const int16u APPLICATION        =0x02;
    const int16u SEEKTABLE          =0x03;
    const int16u VORBIS_COMMENT     =0x04;
    const int16u CUESHEET           =0x05;
    const int16u PICTURE            =0x06;
}

} //NameSpace

#endif
