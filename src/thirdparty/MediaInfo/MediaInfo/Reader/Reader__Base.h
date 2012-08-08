// Reader_File - All information about media files
// Copyright (C) 2002-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
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
#ifndef Reader__BaseH
#define Reader__BaseH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Reader__Base
//***************************************************************************

class Reader__Base
{
public :
    //Constructor/Destructor
    virtual ~Reader__Base() {}

    //Format testing
    virtual size_t Format_Test(MediaInfo_Internal* MI, const String &File_Name)=0;
    virtual size_t Format_Test_PerParser_Continue (MediaInfo_Internal* /*MI*/) {return 0;};
    #if MEDIAINFO_SEEK
    virtual size_t Format_Test_PerParser_Seek (MediaInfo_Internal*, size_t, int64u, int64u) {return 0;};
    #endif //MEDIAINFO_SEEK
};

} //NameSpace
#endif
