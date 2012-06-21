// File_Mpeg4_TimeCode - Info for MPEG-4 TimeCode files
// Copyright (C) 2009-2011 MediaArea.net SARL, Info@MediaArea.net
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

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Mpeg4_TimeCodeH
#define MediaInfo_File_Mpeg4_TimeCodeH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpeg4_TimeCode
//***************************************************************************

class File_Mpeg4_TimeCode : public File__Analyze
{
public :
    //In
    float64 FrameRate;
    bool    NegativeTimes;

    //Out
    int64s  Pos;

    //Constructor/Destructor
    File_Mpeg4_TimeCode();

protected :
    //Streams management
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue();
};

} //NameSpace

#endif
