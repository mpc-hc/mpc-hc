// File_VorbisCom - Info for VorbisComments tagged files
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
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
// Information about Vorbis comments
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_VorbisComH
#define MediaInfo_File_VorbisComH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_VorbisCom
//***************************************************************************

class File_VorbisCom : public File__Analyze
{
public :
    //In
    stream_t StreamKind_Specific; //Always in this stream kind whatever is the configuration
    stream_t StreamKind_Multiple; //Specific stream kind depend if there is multiple streams or not
    stream_t StreamKind_Common;   //Stream kind for common values

    //Constructor/Destructor
    File_VorbisCom();

private :
    //Buffer - File header
    void FileHeader_Parse();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    int32u user_comment_list_length;
    Ztring Chapter_Pos;
    Ztring Chapter_Time;
};

} //NameSpace

#endif
