// File_Zip - Info for NewFormat files
// Copyright (C) 2005-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Information about ZIP files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_ZipH
#define MediaInfo_File_ZipH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Zip
//***************************************************************************

class File_Zip : public File__Analyze
{
protected :
    //Buffer - File header
    bool FileHeader_Begin();
    

    //Buffer - Global
    void Read_Buffer_Continue ();
    
    int32u compressed_size;
    bool data_descriptor_set;
    int32u signature;
    int8u  local_file_Step;
    bool    end_of_central_directory_IsParsed;

    bool local_file();
    bool local_file_header();
    bool file_data();
    bool data_descriptor();
    bool archive_extra_data_record();
    bool central_directory();
    bool digital_signature();
    bool end_of_central_directory();
    bool Zip64_end_of_central_directory_record();
    bool Zip64_end_of_central_directory_locator();
};

} //NameSpace

#endif
