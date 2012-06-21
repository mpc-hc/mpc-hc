// File_Cdxa - Info for CDXA files
// Copyright (C) 2004-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Information about CDXA files
// (like Video-CD...)
// CDXA are read by MS-Windows with CRC bytes
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_CdxaH
#define MediaInfo_File_CdxaH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class MediaInfo_Internal;

//***************************************************************************
// Class File_Cdxa
//***************************************************************************

class File_Cdxa : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Cdxa();
    ~File_Cdxa();

private :
    //Streams management
    void Streams_Finish ();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    
    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    MediaInfo_Internal* MI;
};

} //NameSpace

#endif
