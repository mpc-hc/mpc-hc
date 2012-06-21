// Reader_libcurl - All information about media files
// Copyright (C) 2002-2011 MediaArea.net SARL, Info@MediaArea.net
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
    size_t Format_Test(MediaInfo_Internal* MI, const String &File_Name);
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

