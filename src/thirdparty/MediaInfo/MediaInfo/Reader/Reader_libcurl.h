/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Give information about a lot of media files
// Dispatch the file to be tested by all containers
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef Reader_libcurlH
#define Reader_libcurlH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Reader/Reader__Base.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Reader_libcurl
//***************************************************************************

class Reader_libcurl : public Reader__Base
{
public :
    //Constructor/Destructor
    Reader_libcurl ();
    virtual ~Reader_libcurl();

    //Format testing
    size_t Format_Test(MediaInfo_Internal* MI, String File_Name);
    size_t Format_Test_PerParser(MediaInfo_Internal* MI, const String &File_Name);
    size_t Format_Test_PerParser_Continue (MediaInfo_Internal* MI);
    size_t Format_Test_PerParser_Seek (MediaInfo_Internal* MI, size_t Method, int64u Value, int64u ID);

public:
    struct curl_data;

private :
    curl_data*          Curl_Data;
};

} //NameSpace
#endif

