// Reader_Directory - All information about media files
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
#ifndef Reader_DirectoryH
#define Reader_DirectoryH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Reader/Reader__Base.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Reader_Directory
//***************************************************************************

class Reader_Directory : public Reader__Base
{
public :
    //Constructor/Destructor
    virtual ~Reader_Directory() {}

    //Format testing
    size_t Format_Test(MediaInfo_Internal* MI, const String &File_Name);

    //For the list
    void Directory_Cleanup(ZtringList &List);

private :
    //Bdmv
    int  Bdmv_Format_Test(MediaInfo_Internal* MI, const String &File_Name);
    void Bdmv_Directory_Cleanup(ZtringList &List);

    //P2
    int  P2_Format_Test(MediaInfo_Internal* MI, const String &File_Name);
    void P2_Directory_Cleanup(ZtringList &List);

    //XDCAM
    int  Xdcam_Format_Test(MediaInfo_Internal* MI, const String &File_Name);
    void Xdcam_Directory_Cleanup(ZtringList &List);
};

} //NameSpace
#endif
