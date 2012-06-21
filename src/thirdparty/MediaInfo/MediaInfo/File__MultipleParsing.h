// File_Mpeg - Info for MPEG files
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
// Information about MPEG files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_MpegH
#define MediaInfo_MpegH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <map>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File__MultipleParsing
//***************************************************************************

class File__MultipleParsing : public File__Analyze
{
public :
    //Out
    File__Analyze* Parser_Get();

    //Constructor
    File__MultipleParsing();
    ~File__MultipleParsing();

private :
    //Streams management
    void Streams_Finish();

    //Buffer - Global
    void Read_Buffer_Init();
    void Read_Buffer_Unsynched();
    void Read_Buffer_Continue();

    //Temp
    std::vector<File__Analyze*> Parser;
};

} //NameSpace

#endif
