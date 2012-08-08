// File_Vp8 - Info for VP8 streams
// Copyright (C) 2012-2012 MediaArea.net SARL, Info@MediaArea.net
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
// Information about VP8 files
// http://datatracker.ietf.org/doc/rfc6386/?include_text=1
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Vp8H
#define MediaInfo_Vp8H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Vp8
//***************************************************************************

class File_Vp8 : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;

    //Constructor/Destructor
    File_Vp8();
    ~File_Vp8();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Update();
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - Global
    void Read_Buffer_Continue();
};

} //NameSpace

#endif
