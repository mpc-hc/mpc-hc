// File_Scte20 - Info for SCTE 20 streams
// Copyright (C) 2010-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Information about SCTE 20 streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Scte20H
#define MediaInfo_Scte20H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Scte20
//***************************************************************************

class File_Scte20 : public File__Analyze
{
public :
    //In
    int8u picture_structure;
    bool  progressive_sequence;
    bool  progressive_frame;
    bool  top_field_first;
    bool  repeat_first_field;

    //Constructor/Destructor
    File_Scte20();
    ~File_Scte20();

private :
    //Streams management
    void Streams_Update();
    void Streams_Update_PerStream(size_t Pos);
    void Streams_Finish();

    //Synchro
    void Read_Buffer_Unsynched();

    //Buffer - Global
    void Read_Buffer_Continue();

    //Temp
    struct stream
    {
        File__Analyze*  Parser;
        size_t          StreamPos;
        bool            IsFilled;

        stream()
        {
            Parser=NULL;
            StreamPos=(size_t)-1;
            IsFilled=false;
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
        }
    };
    std::vector<stream*> Streams;
    size_t               Streams_Count;
};

} //NameSpace

#endif

