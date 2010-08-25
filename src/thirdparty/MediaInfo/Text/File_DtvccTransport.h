// File_DtvccTransport - Info for DTVCC Transport strams
// Copyright (C) 2010-2010 MediaArea.net SARL, Info@MediaArea.net
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
    float32 AspectRatio;

    //Constructor/Destructor
    File_DtvccTransport();
    ~File_DtvccTransport();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Synchro
    void Read_Buffer_Unsynched();

    //Buffer - Global
    void Read_Buffer_Continue();

    //Temp
    struct stream
    {
        File__Analyze*  Parser;
        bool            IsFilled;

        stream()
        {
            Parser=NULL;
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

