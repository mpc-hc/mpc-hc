// File_Flac - Info for Flac Audio files
// Copyright (C) 2003-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
#include "MediaInfo/File__Analyze.h"
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
