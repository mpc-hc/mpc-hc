// File_DtvccTransport - Info for DTVCC Transport strams
// Copyright (C) 2010-2012 MediaArea.net SARL, Info@MediaArea.net
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
// Information about Bar Data files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_DtvccTransportH
#define MediaInfo_DtvccTransportH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_DtvccTransport
//***************************************************************************

class File_DtvccTransport : public File__Analyze
{
public :
    //In
    enum format
    {
        Format_Unknown,
        Format_A53_4_GA94_03,   //MPEG_cc_data
        Format_DVD,             //Unknown standard
    };
    format Format;
    float64 AspectRatio;

    //Constructor/Destructor
    File_DtvccTransport();
    ~File_DtvccTransport();

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

        stream()
        {
            Parser=NULL;
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
        }
    };
    std::vector<stream*> Streams;
};

} //NameSpace

#endif

