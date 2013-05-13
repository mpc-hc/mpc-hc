/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
