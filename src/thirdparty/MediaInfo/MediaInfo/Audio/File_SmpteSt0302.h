// File_SmpteSt0302 - Info for SMPTE ST0302
/// Copyright (C) 2008-2012 MediaArea.net SARL, Info@MediaArea.net
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
// Information about SMPTE ST0302
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_SmpteSt0302H
#define MediaInfo_File_SmpteSt0302H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_SmpteSt0302
//***************************************************************************

class File_SmpteSt0302 : public File__Analyze
{
public :
    //Constructor/Destructor
    File_SmpteSt0302();
    ~File_SmpteSt0302();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Temp
    int16u  audio_packet_size;
    int8u   number_channels;
    int8u   bits_per_sample;

    //Parsers
    std::vector<File__Analyze*> Parsers;
    void            Parsers_Init();
    void            Parsers_Parse(const int8u* Parser_Buffer, size_t Parser_Buffer_Size);
};

} //NameSpace

#endif

